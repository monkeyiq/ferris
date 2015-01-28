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

#include "FerrisFileSystemModel.hh"
#include <QApplication>
#include <QDeclarativeView>
#include <qdeclarative.h>
#include <QDeclarativeExtensionPlugin>

using Ferris::FerrisFileSystemModel;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
 
    qmlRegisterType<FerrisFileSystemModel>("Ferris", 1, 0, "FerrisFileSystemModel");
 
    QDeclarativeView view;
    view.setSource(QUrl("./modeltestqml.qml"));
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
 
#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    view.showMaximized();
#else
    view.setGeometry(100,100, 800, 480);
    view.show();
#endif
    return a.exec();
}


