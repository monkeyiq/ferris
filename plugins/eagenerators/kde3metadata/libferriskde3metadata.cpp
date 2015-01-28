/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

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

    $Id: libferriskde3metadata.cpp,v 1.2 2010/09/24 21:31:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <Ferris/Trimming.hh>


// #include <qapplication.h>
#include <kapplication.h>
#include <kio/metainfojob.h>
#include <kcmdlineargs.h>
#include <kfilemetainfo.h>
using namespace KIO;


using namespace std;

namespace Ferris
{
    XSDBasic_t QTypeToFerrisType( QVariant::Type t )
    {
        switch( t )
        {
        case QVariant::Invalid:     return XSD_UNKNOWN;
        case QVariant::Map:         return XSD_UNKNOWN;
        case QVariant::List:        return FXD_XLIST;
        case QVariant::CString:
        case QVariant::String:      return XSD_BASIC_STRING;
        case QVariant::StringList:  return FXD_STRINGLIST;
        case QVariant::Font:
        case QVariant::Pixmap:
        case QVariant::Brush:
        case QVariant::Rect:
        case QVariant::Size:
        case QVariant::Color:
        case QVariant::Palette:
        case QVariant::ColorGroup:
        case QVariant::IconSet:
        case QVariant::Point:
        case QVariant::Image:
            return XSD_UNKNOWN;
        case QVariant::Int:
            return XSD_BASIC_INTEGER;
        case QVariant::UInt:
            return FXD_UINT32;
        case QVariant::LongLong:
            return FXD_INT64;
        case QVariant::ULongLong:
            return FXD_UINT64;
        case QVariant::Bool:
            return XSD_BASIC_BOOL;
        case QVariant::Double:
            return XSD_BASIC_DOUBLE;
        case QVariant::PointArray:
        case QVariant::Region:
        case QVariant::Bitmap:
        case QVariant::Cursor:
        case QVariant::SizePolicy:
        case QVariant::Date:
        case QVariant::Time:
            return XSD_UNKNOWN;
        case QVariant::DateTime: // these seem to be in a strange format at times?
//            return XSD_BASIC_STRING;
            return FXD_UNIXEPOCH_T;
        case QVariant::ByteArray:
        case QVariant::BitArray:
        case QVariant::KeySequence:
        case QVariant::Pen:
        default:
            return XSD_UNKNOWN;
        }
        return XSD_UNKNOWN;
    }

    string KDEAttributeNameToFerrisAttributeName( const std::string& input_string )
    {
        stringstream retss;
        stringstream iss;
        iss << input_string;
        char ch = 0;
        bool lastWasUpper = true;
        
        while( iss >> noskipws >> ch )
        {
            if( ch == '.' || ch == ',' )
            {
                lastWasUpper = false;
            }
            else if( ch == '/' || ch == '&' )
            {
                lastWasUpper = false;
                retss << "-and-";
            }
            else if( ch == ' ' || ch == '_' || ch == '@' || ch == '%' || ch == '*' || ch == '?' )
            {
                lastWasUpper = false;
                retss << "-";
            }
            else if( ::isupper( ch ) )
            {
                if( !lastWasUpper )
                    retss << "-";
                retss << (char)(::tolower(ch));
                lastWasUpper = true;
            }
            else
            {
                lastWasUpper = false;
                retss << ch;
            }
        }
        
        return retss.str();
    }
    
    
    
    /************************************************************/
    /************************************************************/
    /************************************************************/

    
    class FERRISEXP_DLLLOCAL EAGenerator_KDE3Metadata
        :
        public MatchedEAGeneratorFactory
    {
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_KDE3Metadata();

        fh_attribute CreateAttr(
            const fh_context& a,
            const string& rdn,
            fh_context md = 0 )
            throw(
                FerrisCreateAttributeFailed,
                FerrisCreateAttributeNotSupported
                );
        
        virtual bool isDynamic()
            {
                return false;
            }
    
        bool
        supportsCreateForContext( fh_context c )
            {
                return false;
            }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    EAGenerator_KDE3Metadata::EAGenerator_KDE3Metadata()
        :
        MatchedEAGeneratorFactory()
    {
    }

    static void my_init_kde()
    {
        static int virgin = true;
        if( virgin )
        {
            virgin = false;
        
            int argc = 1;
            char* argv[2] = {"libferris",0};
            KCmdLineArgs::init( argc, argv, "ferris", "ferris", "ferris", "1.0" );
            KApplication* myapp = new KApplication( false, false );
        }
    }
    
    void
    EAGenerator_KDE3Metadata::Brew( const fh_context& a )
    {
        my_init_kde();
        
        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );

        LG_KDE_D << "EAGenerator_KDE3Metadata::Brew() a:'" << a->getDirPath() << "'" << endl;
        
        try
        {
            string earl = a->getURL();
            LG_KDE_D << "...earl0:" << earl << endl;
            if( starts_with( earl, "x-ferris" ) )
                return;
            
            KURL kurl( earl.c_str() );
            QString mT = KMimeType::findByURL(kurl)->name();
            if( mT == "image/x-xpm" )
            {
                LG_KDE_D << "Skipping XPM file. url:" << earl << endl;
                return;
            }
            
            KFileMetaInfo mi( kurl );
            if( !mi.isValid() )
                return;
            
            LG_KDE_D << "looking at data for earl:" << earl << endl;

            QStringList ql = mi.supportedKeys();
            LG_KDE_D << "EAGenerator_KDE3Metadata::Brew() kv.size:" << ql.size() << endl;
            for( QStringList::const_iterator iter = ql.begin(); iter != ql.end(); ++iter )
            {
                KFileMetaInfoItem mii = mi.item( *iter );
                QString qs = mii.string();

                if( !qs.length() )
                    continue;

//                cerr << " iter:" << *iter << " qs:" << qs.utf8() << endl;
//                cerr << " iter:" << *iter << " qs:" << qs.latin1() << endl;
                
                string k = KDEAttributeNameToFerrisAttributeName( *iter );
                string v( qs.utf8() );
                const QVariant& qvar = mii.value();
//                cerr << "k:" << k << " v:" << v << endl;
                
                
                XSDBasic_t xsd = QTypeToFerrisType( qvar.type() );
                LG_KDE_D << "EAGenerator_KDE3Metadata::Brew() k:" << k << " v:" << v << " xsd:" << xsd << endl;
                if( qvar.type() == QVariant::DateTime )
                {
                    const QDateTime qdt = qvar.toDateTime();
                    time_t tt = qdt.toTime_t();
//                    cerr << "qdt:" << k << " tt:" << tt << endl;
                    v = tostr(tt);
                }

                switch( qvar.type() )
                {
                case QVariant::Int:
                {
                    v = tostr( qvar.toInt() );
                    break;
                }
                case QVariant::UInt:
                {
                    v = tostr( qvar.toUInt() );
                    break;
                }
                case QVariant::LongLong:
                {
                    v = tostr( qvar.toLongLong() );
                    break;
                }
                case QVariant::ULongLong:
                {
                    v = tostr( qvar.toULongLong() );
                    break;
                }
                case QVariant::Bool:
                {
                    v = tostr( qvar.toBool() );
                    break;
                }
                case QVariant::Double:
                {
                    v = tostr( qvar.toDouble() );
                    break;
                }
//                 {
//                     cerr << "is-int conversion..." << endl;
//                     stringstream ss;
//                     ss << v;
//                     v = "";
//                     ss >> v;
// //                     PostfixTrimmer ptrimmer;
// //                     ptrimmer.push_back( " " );
// //                     v = ptrimmer( v );
//                 }
                }
//                cerr << " iter:" << *iter << " qs:" << qs.utf8() << " v:" << v << endl;
                
                bool rc = a->addAttribute( k, v, xsd );
            }
        }
        catch( exception& e )
        {
            LG_KDE_D << "Failed to load EAs, error:" << e.what() << endl;
        }
    }
    
    fh_attribute
    EAGenerator_KDE3Metadata::CreateAttr( const fh_context& a,
                                 const string& rdn,
                                 fh_context md )
        throw(
            FerrisCreateAttributeFailed,
            FerrisCreateAttributeNotSupported
            )
    {
        fh_stringstream ss;
        ss << "Creating EA is not supported for KDE3 metadata ferris plugin."
           << " context:" << a->getURL()
           << " rdn:" << rdn
           << endl;
        Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
    }

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            return new EAGenerator_KDE3Metadata();
        }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

};
