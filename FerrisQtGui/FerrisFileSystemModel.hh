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

#ifndef _ALREADY_INCLUDED_FERRISQTGUI_FERRISFILESYSTEMMODEL_H_
#define _ALREADY_INCLUDED_FERRISQTGUI_FERRISFILESYSTEMMODEL_H_

#include <HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

#include <QAbstractTableModel>

#include "Ferris/FerrisQt_private.hh"

namespace Ferris
{
    class FERRISEXP_API FerrisFileSystemModel
        :
        public QAbstractTableModel
    {
        Q_OBJECT;

        fh_context m_root;
        typedef std::vector< std::string > columnNames_t;
        mutable columnNames_t m_columnNames;
        columnNames_t m_staticColumnNames;

      protected:

        void updateColumnNames() const;
        
      public:

        int getColumnNumber( const std::string& s );
        virtual QVariant::Type getColumnType( int index ) const;
        std::string getColumnName( int index ) const;
        fh_context toContext( const QModelIndex& index ) const;

        QModelIndex setRootPath( const QString& newPath );
        QModelIndex setRootPath( const std::string& newPath );

        
        FerrisFileSystemModel();
        virtual ~FerrisFileSystemModel();

        virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;
        virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;
        virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

        // virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
        // virtual Qt::ItemFlags flags( const QModelIndex & index ) const;
        
    };
    
};

#endif
