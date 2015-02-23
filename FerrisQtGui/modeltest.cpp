/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2010 Ben Martin

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

    $Id: FerrisWebServices_private.hh,v 1.4 2010/11/15 21:30:33 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "FerrisFileSystemModel.hh"
#include "FerrisSortFilterProxyModel.hh"

#include <qmainwindow.h>
#include <qapplication.h>

#include <QtGui/QTableView>
#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QTimer>

using Ferris::FerrisFileSystemModel;
using Ferris::FerrisSortFilterProxyModel;

std::string ROOTPATH = "/tmp/qtmodel";

class ApplicationWindow: public QMainWindow
{
    Q_OBJECT;
    FerrisFileSystemModel* m;
    QTableView* v;
    QSortFilterProxyModel* p;
    
public:
    ApplicationWindow()
    {
        m = new FerrisFileSystemModel();
        m->setRootPath( ROOTPATH );
        v = new QTableView( this );

//        v->setModel(m);
        v->setSortingEnabled(true);
//        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        QSortFilterProxyModel *proxyModel = new FerrisSortFilterProxyModel( this );
        p = proxyModel;
        proxyModel->setSourceModel( m );
        v->setModel( proxyModel );
        
        for( int i = m->columnCount(); i>=0; i-- )
            v->setColumnHidden( i, true );
        v->setColumnHidden( m->getColumnNumber("name"), false );
        v->setColumnHidden( m->getColumnNumber("size-human-readable"), false );
        v->setColumnHidden( m->getColumnNumber("size"), false );
        v->setColumnHidden( m->getColumnNumber("mtime-display"), false );
        v->setColumnHidden( m->getColumnNumber("mtime"), false );
//        v->setColumnHidden( m->getColumnNumber("sha1"), false );

         
        v->setFocus();
        setCentralWidget( v );

        QTimer::singleShot(3000, this, SLOT(timer()));
    }

    void read( const std::string& url )
    {
        m->setRootPath( url );
    }
    
    ~ApplicationWindow()
    {
    }
    

protected:
//    void closeEvent( QCloseEvent* );

private slots:
    void timer()
    {
//        read( "/tmp/qtmodel" );
    }
    
    // void newDoc();
    // void load();
    // void load( const char *fileName );
    // void save();
    // void saveAs();
    // void print();

    // void about();
    // void aboutQt();

private:
    QToolBar *fileTools;
    QString filename;
};

int main( int argc, char** argv )
{
    QApplication a( argc, argv );

    if( argc > 1 )
        ROOTPATH = argv[1];
    
    ApplicationWindow * mw = new ApplicationWindow();
    mw->show();
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}

#include "modeltest_moc_impl.cpp"
