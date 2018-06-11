/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisxwin_klipper.cpp,v 1.7 2010/09/24 21:31:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "libferrisxwin_klipper.hh"
#include <qstringlist.h>
#include "klipper_interface_public.hh"

#include <FerrisQt_private.hh>

namespace Ferris
{
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

//     static std::string tostr( QString q )
//     {
//         return q.toUtf8().data();
//     }
//     inline fh_stringstream& operator<<( fh_stringstream& S, const QString& q )
//     {
//         S << tostr(q);
//         return S;
//     }
    
    
    klipper* getKlipper()
    {
        static klipper* m_klip = 0;
        if( !m_klip )
        {
            m_klip = new klipper( "org.kde.klipper", "/klipper",
                                  QDBusConnection::sessionBus(), 0 );
        }
        return m_klip;
    }
    
    
    /**
     */
    class FERRISEXP_CTXPLUGIN klipContext
        :
        public StateLessEAHolder< klipContext, leafContext >
    {
        typedef klipContext                                   _Self;
        typedef StateLessEAHolder< klipContext, leafContext > _Base;

    protected:
        virtual std::string priv_getRecommendedEA()
            {
                return adjustRecommendedEAForDotFiles(this, "name,content");
            }

        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    _Base::createStateLessAttributes( true );
                }
            }

        ferris_ios::openmode getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::out       |
                    ios_base::trunc     |
                    ios_base::binary    ;
            }

        

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {
                fh_stringstream ret;

                int i = toint( getDirName() );

                klipper* k = getKlipper();
                QString qs = k->getClipboardHistoryItem( i );
                ret << qs;
                return ret;
            }

        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ret;
                int i = toint( getDirName() );
                if( i )
                {
                    stringstream ss;
                    ss << "Can only write to most recent clipboard entry (having name == 0)" << endl;
                    Throw_CanNotGetStream( ss.str(), this );
                }

                if( !(m & ios_base::trunc) )
                {
                    klipper* k = getKlipper();
                    QString qs = k->getClipboardHistoryItem( i );
                    ret << qs;
                }
                ret->getCloseSig().connect(
                    sigc::bind(
                        sigc::mem_fun( *this, &_Self::OnStreamClosed ), m ));
                return ret;
            }
        
        virtual void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                AdjustForOpenMode_Closing( ss, m, tellp );
                const std::string s = StreamToString(ss);

                cerr << "OnStreamClosed() s:" << s << endl;

                QString qs = s.c_str();
                klipper* k = getKlipper();
                k->setClipboardContents( qs );
            }
        
        
    public:
        
        klipContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        
        virtual ~klipContext()
            {
            }
    };
    FERRIS_SMARTPTR( klipContext, fh_klipContext );

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    klipperTopDirectoryContext::priv_read()
    {
        emitExistsEventForEachItemRAII _raii1( this );
        clearContext();

        klipper* k = getKlipper();
        QStringList qsl = k->getClipboardHistoryMenu();
        int i=0;
        for( QStringList::const_iterator qsi = qsl.begin(); qsi!=qsl.end(); ++qsi, ++i )
        {
            string rdn = tostr(i);
            klipContext* cc = 0;
            cc = priv_ensureSubContext( rdn, cc );
        }
    }
        
    klipperTopDirectoryContext::klipperTopDirectoryContext( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn )
    {
        createStateLessAttributes();
    }
    klipperTopDirectoryContext::klipperTopDirectoryContext()
    {
        createStateLessAttributes();
        createAttributes();
    }
    
    klipperTopDirectoryContext::~klipperTopDirectoryContext()
    {
    }
    std::string
    klipperTopDirectoryContext::priv_getRecommendedEA()
    {
        return adjustRecommendedEAForDotFiles(this, "name");
    }
    std::string
    klipperTopDirectoryContext::getRecommendedEA()
    {
        return adjustRecommendedEAForDotFiles(this, "name");
    }
    
    klipperTopDirectoryContext*
    klipperTopDirectoryContext::priv_CreateContext( Context* parent, string rdn )
    {
        klipperTopDirectoryContext* ret = new klipperTopDirectoryContext();
        ret->setContext( parent, rdn );
        return ret;
    }
        
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
};
