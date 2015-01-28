/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: libferrispostgresqlshared.hh,v 1.2 2006/12/07 06:49:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_SHARED_KIO_H_
#define _ALREADY_INCLUDED_FERRIS_SHARED_KIO_H_

#include <FerrisContextPlugin.hh>
#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/FerrisQt_private.hh>

#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCoreApplication>

#include <kio/job.h>
using namespace KIO;


namespace Ferris
{
    
    class FERRISEXP_CTXPLUGIN KioBaseContext
        :
        public QObject,
    //        public FakeInternalContext
        public StateLessEAHolder< KioBaseContext, FakeInternalContext >
    {
        Q_OBJECT;
//        typedef FakeInternalContext  _Base;
        typedef StateLessEAHolder< KioBaseContext, FakeInternalContext > _Base;

        bool m_isDir;
        std::streamsize m_sz;
        fh_stringstream m_fromBufferTarget;

        std::string getKIOPath();
        
      public:

        KioBaseContext( Context* parent, const std::string& rdn );
        void setup( const KIO::UDSEntry& e );
        fh_istream
            priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception);
        
        void createStateLessAttributes( bool force = false );
        static fh_istream SL_getSizeIStream( KioBaseContext* c, const std::string& rdn, EA_Atom* atom );

        
        void enter_loop();
        void priv_read();
    public slots:
        void slotResult( KJob* job );
        void entries (KIO::Job *job, const KIO::UDSEntryList &list);
        void data_fromBuffer(KIO::Job *job, const QByteArray &data);
        
      Q_SIGNALS:
        void leaveModality();
    };

    class FERRISEXP_CTXPLUGIN KioRootContext
        :
        public KioBaseContext
    {
        typedef KioBaseContext _Base;
        
      public:
        KioRootContext( Context* parent, const std::string& rdn );
        void priv_read();

      protected:

        fh_context priv_getSubContext( const std::string& rdn )
            throw( NoSuchSubContext );
        
    };
    
    
    namespace Kio
    {
    };
    
    /****************************************/
    /****************************************/
    /****************************************/
    
    namespace Factory
    {
    };
};

#endif
