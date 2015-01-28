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

#ifndef _ALREADY_INCLUDED_FERRISQTGUI_FERRISSORTFILTERPROXYMODEL_H_
#define _ALREADY_INCLUDED_FERRISQTGUI_FERRISSORTFILTERPROXYMODEL_H_

#include <HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

#include <QSortFilterProxyModel>

#include "Ferris/FerrisQt_private.hh"
#include "FerrisFileSystemModel.hh"

namespace Ferris
{
    class FERRISEXP_API FerrisSortFilterProxyModel
        :
        public QSortFilterProxyModel
    {
        Q_OBJECT;
        typedef QSortFilterProxyModel _Base;

        FerrisFileSystemModel* m;
        
      public:
        FerrisSortFilterProxyModel( QObject * parent = 0 );
        virtual ~FerrisSortFilterProxyModel();
        
        virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );
        virtual void setSourceModel( QAbstractItemModel * sourceModel );
    };
};

#endif
