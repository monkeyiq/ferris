/*
 *   Copyright 2010 Ben Martin <monkeyiq@example.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  For more details see the COPYING file in the root directory of this
 *  distribution.
 */
#include <config.h>
 
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisQt_private.hh>

#include "plasma_engine_libferris.hh"

#include <Plasma/DataContainer>
#include <QDate>
#include <QTime>
 
#include <KSystemTimeZones>
#include <KDateTime>
#include <QProcess>


#define DEBUG kDebug(500115)


using namespace Ferris;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FerrisService::FerrisService( FerrisEngine* engine, QString src )
    : Plasma::Service()
    , m_engine(engine)
    , m_source(src)
{
    cerr << "FerrisService::FerrisService src:" << tostr(src) << endl;
    setName("libferris");
    
}

Plasma::ServiceJob*
FerrisService::createJob(const QString &operation,
                         QMap<QString, QVariant> &parameters)
{
    cerr << "createJob()" << endl;
    cerr << "createJob() src" << tostr(m_source) << endl;
    if (operation == "write")
    {
        cerr << "WRITE service..." << endl;
        cerr << "WRITE  src:" << tostr(m_source) << endl;
        std::string content = tostr(parameters["content"].toString());
        cerr << "WRITE data:" << content << endl;

        fh_context c;
        string eaname = "content";
        
        if( m_source.contains("@") )
        {
            DEBUG << "FerrisEngine::update() has EA name:" << tostr(m_source).c_str();
            string key = tostr(m_source);
            int ampos = key.rfind("@");
            eaname = key.substr(ampos+1);
            string earl = key.substr(0,ampos);

            c = Resolve( earl );
        }
        else
        {
            c = Resolve(tostr(m_source));
        }
    
        setStrAttr(c,eaname,content,true,true);
    }
    if (operation == "readURL")
    {
        cerr << "READURL src:" << tostr(m_source) << endl;
        fh_context c = Resolve(tostr(m_source));
        c->read();
    }
    return 0;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FerrisEngine::FerrisEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
    Q_UNUSED(args)
    setMinimumPollingInterval(200);
    setenv("LIBFERRIS_ENABLE_ATTRIBUTE_RESOLUTION","1",1);
}


void
FerrisEngine::init()
{
//        m_sources << "foo";
}

bool
FerrisEngine::sourceRequestEvent(const QString &name)
{
    // one time setup...
    DEBUG << "FerrisEngine::sourceRequestEvent() name:" << name;
    
    // forward...
    return updateSourceEvent(name);
}



bool
FerrisEngine::updateSourceEvent(const QString &name_const)
{
    QString name = name_const;

    if( name.isEmpty() )
        return false;
    
//    if( name.startsWith("file://"))
    {
        try
        {            
            DEBUG << "FerrisEngine::update() name:" << name_const;
            if( name.contains("/*@") )
            {
                cerr << "has /*@" << endl;
                
                string key = tostr(name);
                int ampos = key.rfind("*");
                string eaname = key.substr(ampos+1);
                string earl = key.substr(0,ampos);

                cerr << "earl:" << earl << endl;
                cerr << "ea:" << eaname << endl;
                
                fh_context c = Resolve( earl );
                c->read();
                if( c->getSubContextCount() )
                {
                    fh_context child = *(c->begin());
                    c = child;
                    cerr << "child:" << child->getURL() << endl;
                    name = c->getURL().c_str();
                    name.append( eaname.c_str() );
                    cerr << "new name:" << tostr(name).c_str() << endl;
                    
                }
            }
            if( name.contains("@") )
            {
                DEBUG << "FerrisEngine::update() has EA name:" << tostr(name).c_str();
                string key = tostr(name);
                int ampos = key.rfind("@");
                string eaname = key.substr(ampos+1);
                string earl = key.substr(0,ampos);

                cerr << "update() has @... earl:" << earl << endl;
                cerr << "update() has @... eaname:" << eaname << endl;
                DEBUG << "FerrisEngine::update() earl:" << earl.c_str();
                DEBUG << "FerrisEngine::update() eaname:" << eaname.c_str();
                DEBUG << "FerrisEngine::update(2) eaname:" << eaname.c_str();

                fh_context c = Resolve( earl );
                string v = getStrAttr( c, eaname, "", true, true );
                cerr << "v:" << v << endl;
                string scv = getStrAttr( c, "schema:" + eaname, "" );
//                cerr << "schema:" << scv << endl;
                QVariant qv = v.c_str();
                if( starts_with( scv, "schema://xsd/attributes/decimal/integer" ))
                    qv = toint(v);

                cerr << "qv:" << tostr(qv.toString()).c_str() << endl;
                setData(name_const, "content", qv );
            }
            else
            {
                DEBUG << "FerrisEngine::update() RAW";
                fh_context c = Resolve( tostr(name) );
                string v = getStrAttr( c, "content", "", true, true );
                setData(name, "content", v.c_str() );
            }
        }
        catch( std::exception& e )
        {
            setData(name, "content", e.what() );
            setData(name, "error", e.what() );
        }
    }
    // else
    // {
    //     setData(name, "id", "2");
    // }
    return true;
}

QStringList
FerrisEngine::sources() const
{
    return m_sources;
}

Plasma::Service*
FerrisEngine::serviceForSource(const QString &source)
{
    cerr << "FerrisEngine::serviceForSource() src:" << tostr(source) << endl;
    FerrisService *service = new FerrisService(this,source);
    service->setParent(this);
    return service;
}


K_EXPORT_PLASMA_DATAENGINE(libferris, FerrisEngine)
#include "plasma_engine_libferris_moc.cpp"
