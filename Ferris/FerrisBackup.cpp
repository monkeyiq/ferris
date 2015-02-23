/******************************************************************************
*******************************************************************************
*******************************************************************************

    backup handler

    Copyright (C) 2002 Ben Martin

    libferris is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libferris is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libferris.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: FerrisBackup.cpp,v 1.2 2010/09/24 21:30:34 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <FerrisBackup.hh>
#include <Ferris.hh>

using namespace std;

namespace Ferris
{
    namespace Util
    {
   
        BackupMaker::BackupMaker()
            :
            Suffix(""),
            ModeString("existing"),
            Mode( MODE_EXISTING )
        {
        }

        void
        BackupMaker::setSuffix( const std::string& s )
        {
            Suffix = s;
        }
        
        string
        BackupMaker::getSuffix()
        {
            if( Suffix.length() )
            {
                return Suffix;
            }
            else if( const gchar* p = g_getenv ("SIMPLE_BACKUP_SUFFIX") )
            {
                Suffix = p;
            }
            else
            {
                Suffix = "~";
            }
            return Suffix;
        }
        
        void
        BackupMaker::setMode( Mode_t m )
        {
            if( m == MODE_NONE )
            {
                ModeString = "none";
            }
            else if( m == MODE_NUMBERED )
            {
                ModeString = "numbered";
            }
            else if( m == MODE_EXISTING )
            {
                ModeString = "existing";
            }
            else if( m == MODE_SIMPLE )
            {
                ModeString = "simple";
            }
            else
            {
                fh_stringstream ss;
                ss << "unknown backup mode requested:" << m << endl;
                Throw_UnknownBackupMode( tostr(ss), 0 );
            }

            Mode = m;
        }
        
        void
        BackupMaker::setMode( const std::string& s )
        {
            if( s == "none" || s == "off" )
            {
                Mode = MODE_NONE;
            }
            else if( s == "numbered" || s == "t" )
            {
                Mode = MODE_NUMBERED;
            }
            else if( s == "existing" || s == "nil" )
            {
                Mode = MODE_EXISTING;
            }
            else if( s == "simple" || s == "never" )
            {
                Mode = MODE_SIMPLE;
            }
            else
            {
                fh_stringstream ss;
                ss << "unknown backup mode requested:" << s << endl;
                Throw_UnknownBackupMode( tostr(ss), 0 );
            }
            
            ModeString = Mode;
        }

        bool
        BackupMaker::impotent()
        {
            return Mode == MODE_NONE;
        }
        
        

        std::string
        BackupMaker::getBackupName_Simple( fh_context c )
        {
            string rdn = c->getDirName();
            rdn += getSuffix();
            return rdn;
        }

        int
        BackupMaker::getHighestBackupNumber( fh_context c )
        {
            typedef Context::SubContextNames_t SubContextNames_t;
            typedef SubContextNames_t::iterator iter_t;

            SubContextNames_t scn = c->getParent()->getSubContextNames();
            int ret = 0;
            string versioned_rdn_prefix = c->getDirName();
            versioned_rdn_prefix += ".~";
            
            for( iter_t iter = scn.begin(); iter != scn.end(); ++iter )
            {
                string n = *iter;
                
                if( starts_with( n, versioned_rdn_prefix ) )
                {
                    string ending = n.substr( versioned_rdn_prefix.length() );
                    int revision = 0;

                    fh_stringstream ss;
                    ss << ending;
                    ss >> revision;

                    if( revision > ret )
                    {
                        ret = revision;
                    }
                }
            }
                
            return ret;
        }
        
        std::string
        BackupMaker::getBackupName_Numbered( fh_context c )
        {
            int n = 1 + getHighestBackupNumber( c );
            
            fh_stringstream ss;
            ss << c->getDirName() << ".~" << n << "~" << flush;
//            cerr << "getBackupName_Numbered() ret:" << tostr(ss) << endl;
            return tostr(ss);
        }
        
        std::string
        BackupMaker::getBackupName( fh_context c )
        {
            string rdn = "";
            
            switch( Mode )
            {
            case MODE_NONE:     rdn = c->getDirName();             break;
            case MODE_SIMPLE:   rdn = getBackupName_Simple( c );   break;
            case MODE_NUMBERED: rdn = getBackupName_Numbered( c ); break;
            case MODE_EXISTING:
                if( getHighestBackupNumber( c ) > 0 )
                    rdn = getBackupName_Numbered( c );
                else
                    rdn = getBackupName_Simple( c );
                break;
            }

            if( !rdn.length() )
            {
                fh_stringstream ss;
                ss << "unknown backup mode requested. Mode" << Mode
                   << " ModeString:" << ModeString
                   << endl;
                Throw_UnknownBackupMode( tostr(ss), 0 );
            }

            int maxlen = toType<int>(getStrAttr( c, "fs-file-name-length-maximum", "0" ));
            if( maxlen && maxlen < rdn.length() )
            {
                fh_stringstream ss;
                ss << "Backup name:" << rdn
                   << " would exceed the maximum filename length of:" << maxlen
                   << " for url:" << c->getURL();
                Throw_BackupNameWouldBeTooLong( tostr(ss), 0 );
            }
            
            return rdn;
        }
        
        void
        BackupMaker::operator()( fh_context c )
        {
            bool isDir  = isTrue(getStrAttr( c, "is-dir", "0" ));
            int  mode   = toint(getStrAttr( c, "mode", "0" ));
            fh_context parent   = c->getParent();
            string BackupName   = getBackupName( c );
            string OriginalName = c->getDirName();
            
            
            if( Mode == MODE_SIMPLE || Mode == MODE_EXISTING )
            {
//                 cerr << "parent:" << parent->getDirPath() << endl;
//                 cerr << "BackupName:" << BackupName << endl;
//                 cerr << "isbound:" << parent->isSubContextBound( BackupName ) << endl;
                if( parent->isSubContextBound( BackupName ) )
                {
                    parent->remove( BackupName );
                }
                
//                 string fqfn = c->appendToPath( parent->getDirPath(), BackupName );
//                 cerr << "fqfn:" << fqfn << endl;
//                 ::remove( fqfn.c_str() );
            }

//             cerr << "BackupMaker::operator() calling rename()"
//                  << " c->url:" << c->getURL()
//                  << " c->name:" << c->getDirName()
//                  << " OriginalName:" << OriginalName
//                  << " BackupName:" << BackupName
//                  << endl;
            parent->rename( OriginalName, BackupName );
//            cerr << "BackupMaker::operator() calling touch()" << endl;
            Shell::touch( parent->getDirPath() + "/" + OriginalName, true, isDir, mode );
//            cerr << "BackupMaker::operator() returning" << endl;
    }

        void
        BackupMaker::operator()( fh_attribute a )
        {
            fh_context   c    = a->getParent();
            string rdn        = getBackupName( c );
            fh_attribute newa = c->createAttribute( rdn );
            fh_iostream  oss  = newa->getIOStream();
            
            oss = a->copyTo( oss );
        }

        void
        BackupMaker::perform( fh_context c )
        {
            return operator()( c );
        }
        
        void
        BackupMaker::perform( const std::string& path )
        {
//            cerr << "BackupMaker::perform() path:" << path << endl;
            return operator()( Resolve( path ) );
        }
        
    };
};
