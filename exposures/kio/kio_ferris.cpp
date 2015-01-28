/*
 *   Copyright 2012 Ben Martin <monkeyiq@example.com>
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

#include <Ferris/Ferris.hh>
#include <Ferris/Runner.hh>
using namespace Ferris;
//#include <gtk/gtk.h>

#include "kio_ferris.hh"

#include <kdebug.h>
#include <kcomponentdata.h>
#include <QProcess>
#include <QtXml>

#include <iostream>
#include <fstream>
using namespace std;


ofstream& getlog() 
{
    static ofstream log;
    static bool v = true;
    if(v) 
    {
        v = false;
        log.open("/tmp/log.txt", ios_base::app );
    }
    return log;
}


std::string tostr( const QString& q )
{
    return q.toUtf8().data();
}
std::string tostr( const QByteArray& q )
{
    std::stringstream ret;
    ret.write( q.data(), q.size() );
    return ret.str();
}

int toint( const QString& s )
{
    stringstream ss;
    ss << tostr(s);
    int ret;
    ss >> ret;
    return ret;
}


extern "C" int KDE_EXPORT kdemain( int argc, char **argv )
{
  kDebug(7000) << "Entering function";
  getlog() << "starting" << endl;
//  gtk_init( &argc, &argv );
  GMainLoop* ml = g_main_loop_new( 0, 0 );
  
  fh_context c = Resolve("/");
  getlog() << "starting, have ferris /..." << endl;
  
  KComponentData instance( "kio_ferris" );
 
  if (argc != 4)
  {
    fprintf( stderr, "Usage: kio_ferris protocol domain-socket1 domain-socket2\n");
    exit( -1 );
  }
  KIOFerris slave( argv[2], argv[3] );
  slave.dispatchLoop();
  return 0;
}

// int main( int argc, char **argv )
// {
//     cerr << "Entering function" << endl;
//     return kdemain( argc, argv );
// }

KIOFerris::KIOFerris( const QByteArray &pool, const QByteArray &app )
  : SlaveBase( "ferris", pool, app )
{
}

void
KIOFerris::open( const KUrl& url, QIODevice::OpenMode mode )
{
    m_openURL = url.path();
    getlog() << "open(xxx)... url:" << tostr(m_openURL) << endl;
     
    opened();
    finished();
}

void
KIOFerris::write( const QByteArray& data ) 	
{
    getlog() << "write(xxx)... url:" << tostr(m_openURL) << endl;
    finished();
}

void
KIOFerris::seek( KIO::filesize_t offset )
{
    getlog() << "seek(xxx)... offset:" << offset << endl;
    finished();
}

    
void
KIOFerris::put( const KUrl& url, int permissions, KIO::JobFlags flags )
{
    getlog() << "put(xxx)... url:" << tostr(url.path()) << endl;

    // Loop until we got 0 (end of data)
    QByteArray allData;
    while( true )
    {
        QByteArray buffer;
        dataReq(); // Request for data
        int result = readData( buffer );
        if( !result )
            break;
        allData += buffer;
    }

    QProcess p;
    p.start("ferris-redirect",
            QStringList()
            << "--trunc"
            << url.path() );
    p.waitForStarted();
    p.write( allData );
    p.closeWriteChannel();
    p.waitForFinished();
    QByteArray t = p.readAll();
    finished();
}


void
KIOFerris::get( const KUrl &url )
{
    getlog() << "get(xxx) url:" << tostr(url.path()) << endl;
    kDebug() << " : " << url.path();

  // mimeType("xdirectory/normal");
  // finished();
  // return;
  
  // error( KIO::ERR_DOES_NOT_EXIST, "can't get there from here" );
  // return;
  
  mimeType( "text/plain" );
//  fh_context c = Resolve( tostr(url.path()) );
  getlog() << "get(xxx2) url:" << tostr(url.path()) << endl;
//  QByteArray str( getStrAttr(c,"content","",true,true).c_str() );
//  QByteArray str( getStrAttr(c,"annotation","",true,true).c_str() );
//  data( str );

  /*
  fh_runner r = new Runner();
  r->pushCommandLineArg( "fcat" );
//  r->pushCommandLineArg( "-a" );
//  r->pushCommandLineArg( "annotation" );
  r->pushCommandLineArg( tostr(url.path()) );
  r->Run();
  fh_istream outss = r->getStdOut();
  data( StreamToString(outss).c_str() );
  */

  QProcess p;
  p.start("fcat", QStringList() << url.path() );
  p.waitForFinished();
  data(p.readAll());

  
  getlog() << "get(xxx3) url:" << tostr(url.path()) << endl;
  finished();
  kDebug(7000) << "Leaving function";
}

static void setupEntry( KIO::UDSEntry& e, QDomNamedNodeMap a )
{
    e.insert( KIO::UDSEntry::UDS_NAME, a.namedItem("name").nodeValue() );

    getlog() << "is-dir:" << tostr(a.namedItem("is-dir").nodeValue()) << endl;
    
    if( a.namedItem("is-dir").nodeValue() == "1")
        e.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
    else
        e.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG );

    e.insert( KIO::UDSEntry::UDS_ACCESS,            a.namedItem("mode").nodeValue());
    e.insert( KIO::UDSEntry::UDS_ACCESS_TIME,       toint(a.namedItem("atime").nodeValue()));
    e.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, toint(a.namedItem("mtime").nodeValue()));
    e.insert( KIO::UDSEntry::UDS_SIZE,       toint(a.namedItem("size").nodeValue()));
    e.insert( KIO::UDSEntry::UDS_SIZE_LARGE, toint(a.namedItem("size").nodeValue()));
    e.insert( KIO::UDSEntry::UDS_GROUP,      toint(a.namedItem("group-owner-number").nodeValue()));
    e.insert( KIO::UDSEntry::UDS_LINK_DEST,  a.namedItem("link-target").nodeValue());
    e.insert( KIO::UDSEntry::UDS_GUESSED_MIME_TYPE, a.namedItem("mimetype").nodeValue());
    e.insert( KIO::UDSEntry::UDS_COMMENT,    a.namedItem("annotation").nodeValue());
}

    

void
KIOFerris::stat(const KUrl& url)
{
    getlog() << "stat(xxx) url:" << tostr(url.path()) <<  endl;
    kDebug() << " : " << url.path();

    KIO::UDSEntry e;
    QProcess p;
    p.start("ferrisls",
            QStringList()
            << "--xml"
            << "-lhd"
            << "--show-ea=name,is-dir,size,atime,mtime,ctime,group-owner-number,annotation"
            << url.path() );
    p.waitForFinished();
    QByteArray ba = p.readAll();

    QDomDocument doc;
    doc.setContent(ba);
    QDomElement top = doc.documentElement();
    QDomNodeList nl = doc.elementsByTagName("context");
    getlog() << "nl.sz:" << nl.length() << endl;
    for( int i = 0; i<nl.length(); i++ )
    {
        QDomNode n = nl.item(i);
        QDomNamedNodeMap a = n.attributes();
        setupEntry( e, a );
        break;
    }
    statEntry( e );
    finished();
}

void
KIOFerris::listDir(const KUrl& url)
{
    getlog() << "listDir(xxx) url:" << tostr(url.path()) << endl;
    kDebug() << " listDir(xxx) : " << url.path();

    KIO::UDSEntry dirEntry;

    QProcess p;
    p.start("ferrisls",
            QStringList()
            << "--xml"
            << "--show-ea=name,is-dir,size,atime,mtime,ctime,group-owner-number,annotation"
            << url.path() );
    p.waitForFinished();
    QByteArray ba = p.readAll();
    getlog() << tostr(ba) << endl;

    QDomDocument doc;
    doc.setContent(ba);
    QDomElement top = doc.documentElement();
    QDomNodeList nl = doc.elementsByTagName("context");
    getlog() << "nl.sz:" << nl.length() << endl;
    for( int i = 0; i<nl.length(); i++ )
    {
        QDomNode n = nl.item(i);
        QDomNamedNodeMap a = n.attributes();
        KIO::UDSEntry e;
        setupEntry( e, a );
        
        // emit a signal to list this entry
        listEntry( e, false );
    }
    
/*        
    fh_context c = Resolve( tostr(url.path()) );
//    getlog() << "listDir(xxx2) url:" << tostr(url.path()) << endl;
//    c->read(true);
    getlog() << "listDir(xxx3) url:" << tostr(url.path()) << endl;
    for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
    {
        getlog() << "listDir(xxx) name:" << (*ci)->getDirName() << endl;
        
        dirEntry.insert( KIO::UDSEntry::UDS_NAME, getStrAttr( *ci, "name", "" ).c_str() );
        stringstream ss;
        if( toint(getStrAttr( *ci, "is-dir", "0" )))
            dirEntry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
        else
            dirEntry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG );

        
        dirEntry.insert( KIO::UDSEntry::UDS_ACCESS,            toint(getStrAttr( *ci, "mode",  "0" )));
        dirEntry.insert( KIO::UDSEntry::UDS_ACCESS_TIME,       toint(getStrAttr( *ci, "atime", "0" )));
        dirEntry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, toint(getStrAttr( *ci, "mtime", "0" )));
        dirEntry.insert( KIO::UDSEntry::UDS_SIZE,       toint(getStrAttr( *ci, "size",  "0" )));
        dirEntry.insert( KIO::UDSEntry::UDS_SIZE_LARGE, toint(getStrAttr( *ci, "size",  "0" )));
        dirEntry.insert( KIO::UDSEntry::UDS_GROUP,      toint(getStrAttr( *ci, "group-owner-number",  "0" )));
        dirEntry.insert( KIO::UDSEntry::UDS_LINK_DEST,  getStrAttr( *ci, "link-target",  "" ).c_str());
        dirEntry.insert( KIO::UDSEntry::UDS_GUESSED_MIME_TYPE, getStrAttr( *ci, "mimetype", "" ).c_str());
        dirEntry.insert( KIO::UDSEntry::UDS_COMMENT,    getStrAttr( *ci, "annotation", "" ).c_str());

        // emit a signal to list this entry
        listEntry( dirEntry, false );
    }
*/
        
    // all done. dirEntry is not touched, the second argument
    // is used only to say we are all done. Yay for modal API
    // designs.
    listEntry( dirEntry, true );
    finished();
}

KConfigGroup*
KIOFerris::config()
{
    KConfigGroup* ret = new KConfigGroup();
    ret->writeEntry("comment", "Fdsfsdfsd");
    return ret;
}

bool
KIOFerris::hasMetaData( const QString& key ) const
{
    getlog() << "hasMetaData(xxx) url:" << tostr(key) << endl;
    return true;
}

