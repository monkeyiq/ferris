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
#include <QtCore/QDateTime>

#define DEBUG LG_FERRISQTGUI_D

static const char* const treeicon_pixbuf_cn = "treeicon-pixbuf";
static const char* const emblems_pixbuf_cn  = "emblem:emblems-pixbuf";

namespace Ferris
{
    using std::endl;
    using std::string;
    using std::exception;

    void
    FerrisFileSystemModel::updateColumnNames() const
    {
        if( !isBound( m_root ) )
        {
            DEBUG << "updateColumnNames(returning) m_root not set." << endl;
            return;
        }

        if( !m_columnNames.empty() )
        {
            DEBUG << "updateColumnNames() m_columnNames already populated." << endl;
            return;
        }

        if( !m_staticColumnNames.empty() )
        {
            DEBUG << "updateColumnNames() using a static column name list.." << endl;
            m_columnNames = m_staticColumnNames;
            return;
        }

        DEBUG << "updateColumnNames(starting)" << endl;
        DEBUG << "updateColumnNames() m_root:" << m_root->getURL() << endl;

        fh_context rc = m_root;
        try
        {
            std::set< std::string > unionset;
            
            DEBUG << "updateColumnNames(getting ea-names)" << endl;
            addEAToSet( unionset,  getStrAttr( rc, "ea-names", "" ));
            DEBUG << "updateColumnNames(getting ea-names-union-view)" << endl;
            addEAToSet( unionset,  getStrAttr( rc, "recommended-ea-union-view", "" ));
            DEBUG << "updateColumnNames(getting fixed additions)" << endl;
            addEAToSet( unionset,  getEDBString( FDB_GENERAL,
                                                 CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_K,
                                                 CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_DEFAULT ));
            unionset.insert( treeicon_pixbuf_cn );
            unionset.insert( emblems_pixbuf_cn );
            m_columnNames.clear();
            m_columnNames.reserve( unionset.size() );
            copy( unionset.begin(), unionset.end(), back_inserter( m_columnNames ));
        }
        catch( exception& e )
        {
            DEBUG << "error adding column data for ctx:" << rc->getURL() << endl;
        }

        DEBUG << "UpdateColumnNames() m_columnNames.size:" << m_columnNames.size() << endl;
    }

    QVariant::Type
    FerrisFileSystemModel::getColumnType( int index ) const
    {
        // FIXME
        
        return QVariant::String;
    }
    
    std::string
    FerrisFileSystemModel::getColumnName( int index ) const
    {
        return m_columnNames[index];
    }
    
    int
    FerrisFileSystemModel::getColumnNumber( const std::string& s )
    {
        updateColumnNames();
        columnNames_t an = m_columnNames;
        int i=0;

        for( columnNames_t::const_iterator iter = an.begin(); iter != an.end(); ++iter, ++i )
        {
            if( *iter == s )
            {
                return i;
            }
        }

        DEBUG << "getColumnNumber() HAS FAILED TO FIND COLUMN:" << s << endl;
        for( columnNames_t::const_iterator iter = an.begin(); iter != an.end(); ++iter, ++i )
        {
            DEBUG << "  iter:  " << *iter << endl;
        }
        
        return -1;
    }
    
    fh_context
    FerrisFileSystemModel::toContext( const QModelIndex& index ) const
    {
        int r = index.row();
        if( r > rowCount() )
        {
            DEBUG << "row is too large. r:" << r << endl;
            return 0;
        }

        Context::iterator ci = m_root->begin();
        std::advance( ci, r );
        return *ci;
    }
    

    
    FerrisFileSystemModel::FerrisFileSystemModel()
        :
        m_root(0)
    {
    }
    
    FerrisFileSystemModel::~FerrisFileSystemModel()
    {
    }

    QModelIndex
    FerrisFileSystemModel::setRootPath( const QString & newPath )
    {
        beginResetModel();
        
        m_root = Resolve( tostr(newPath) );
        m_root->read();

        endResetModel();
        
    }
    QModelIndex FerrisFileSystemModel::setRootPath( const std::string& newPath )
    {
        QString s = newPath.c_str();
        return setRootPath( s );
    }
    

    
    int
    FerrisFileSystemModel::rowCount( const QModelIndex & parent ) const
    {
        if( !m_root )
            return 0;
        
        return m_root->getSubContextCount();
    }

    
    
    int
    FerrisFileSystemModel::columnCount( const QModelIndex & parent ) const
    {
        if( !m_root )
            return 0;

        updateColumnNames();
        return m_columnNames.size();
        
    }
    
    QVariant
    FerrisFileSystemModel::data( const QModelIndex & index, int role ) const
    {
        // FIXME
        DEBUG << "data() role:" << role << " row:" << index.row() << " col:" << index.column() << endl;
        
        if( role == Qt::DisplayRole )
        {
            if( fh_context c = toContext( index ) )
            {
                std::string eaname = getColumnName( index.column() );
                QVariant ret;
                
                std::string v  = getStrAttr( c, eaname, "", true, false );
                fh_context schema = c->getSchema( eaname );
                string schema_url = schema->getURL();
                
                DEBUG << "eaname:" << eaname << " schema:" << schema->getURL() << endl;

                if( ends_with( schema_url, "schema.xml/xsd/attributes/decimal/integer/long/fs/time" ))
                {
                    QDateTime qv;
                    qv.setTime_t( toType<uint>(v) );
                    ret = qv;
                }
                else if( contains( schema_url, "schema.xml/xsd/attributes/decimal/integer" ))
                {
                    qlonglong qv = ::Ferris::toType<qlonglong>(v);
                    ret = qv;
                }
                else
                {
                    QString qv = v.c_str();
                    ret = qv;
                }
                
                return ret;
            }
        }
        
        return QVariant();
    }
    
    QVariant
    FerrisFileSystemModel::headerData( int section, Qt::Orientation orientation, int role ) const
    {
        if( orientation == Qt::Horizontal )
        {
            if( role == Qt::DisplayRole )
            {
                std::string cn = getColumnName( section );
                QString qcn = cn.c_str();
                return qcn;
            }
        }
        if( orientation == Qt::Vertical )
        {
            if( role == Qt::DisplayRole )
            {
                std::string cn = tostr( section );
                QString qcn = cn.c_str();
                return qcn;
            }
        }
        
        return QVariant();
    }

};

#include "FerrisFileSystemModel_moc.cpp"
