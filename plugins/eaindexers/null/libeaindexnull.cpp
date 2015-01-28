/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libeaindexnull.cpp,v 1.2 2007/09/17 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include "EAIndexerMetaInterface.hh"
#include "EAIndexer.hh"

using namespace std;

namespace Ferris
{
    namespace EAIndex 
    {
        static const char* CFG_EAIDX_READVALUES_K = "cfg-eaidx-readvalues";
        
        class FERRISEXP_DLLLOCAL EAIndexerNULL
            :
            public MetaEAIndexerInterface
        {
            bool m_readValues;
            
        protected:
            virtual void Setup();
            virtual void CreateIndex( fh_context c,
                                      fh_context md );
            virtual void CommonConstruction();

        public:
            EAIndexerNULL();
            static MetaEAIndexerInterface* Create();
            virtual ~EAIndexerNULL();

            virtual void sync();

            virtual void reindexingDocument( fh_context c, docid_t docid );
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 );
            virtual std::string resolveDocumentID( docid_t );


            bool readValues();
        };
        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        EAIndexerNULL::EAIndexerNULL()
        {
        }

        MetaEAIndexerInterface*
        EAIndexerNULL::Create()
        {
            return new EAIndexerNULL();
        }
        

        EAIndexerNULL::~EAIndexerNULL()
        {
        }
        
        void
        EAIndexerNULL::Setup()
        {
            m_readValues = toType<bool>(getConfig( CFG_EAIDX_READVALUES_K, "1" ));
        }

        bool
        EAIndexerNULL::readValues()
        {
            return m_readValues;
        }
        
        void
        EAIndexerNULL::CreateIndex( fh_context c,
                                    fh_context md )
        {
            string readValues = getStrSubCtx( md, "read-values", "1" );
            setConfig( CFG_EAIDX_READVALUES_K, readValues );
            m_readValues = toType<bool>(getConfig( CFG_EAIDX_READVALUES_K, "1" ));
        }
        
        void
        EAIndexerNULL::CommonConstruction()
        {
        }
        
        
        void
        EAIndexerNULL::sync()
        {
        }
        
        void
        EAIndexerNULL::reindexingDocument( fh_context c, docid_t docid )
        {
        }
        
        void
        EAIndexerNULL::addToIndex( fh_context c,
                                   fh_docindexer di )
        {
            string earl   = c->getURL();
            
            int attributesDone = 0;
            int signalWindow   = 0;
            stringlist_t slist;
            getEANamesToIndex( c, slist );
//            stringlist_t slist = Util::parseCommaSeperatedList( getStrAttr( c, "ea-names", "" ));
            int totalAttributes = slist.size();

            Time::Benchmark bm( "EAIndexerNULL::addToIndex() earl:" + earl );
            bm.start();
            
            for( stringlist_t::iterator si = slist.begin(); si != slist.end(); ++si )
            {
                try
                {
                    string attributeName = *si;

                    bool valueIsNull = false;
                    string k = attributeName;
                    string v = "";
                    if( !obtainValueIfShouldIndex( c, di,
                                                   attributeName, v,
                                                   true, valueIsNull ))
                        continue;
                    IndexableValue iv  = getIndexableValue( c, k, v );

                    
//                     if( attributeName == "width" )
//                     {
//                         cerr << "WIDTH:" << v << " for file:" << c->getURL() << endl;
//                         if( valueIsNull )
//                         {
//                             cerr << "Null w for file:" << c->getURL() << endl;
//                         }
//                         if( toint( v ) > 200 )
//                         {
//                             cerr << "WIDTH>200 for file:" << c->getURL() << endl;
//                         }
//                     }
                }
                catch( exception& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                }
                
                if( signalWindow > 5 )
                {
                    signalWindow = 0;
                    di->getProgressSig().emit( c, attributesDone, totalAttributes );
                }
                ++attributesDone;
                ++signalWindow;
            }
        }

        
        
        docNumSet_t&
        EAIndexerNULL::ExecuteQuery( fh_context q,
                                     docNumSet_t& output,
                                     fh_eaquery qobj,
                                     int limit )
        {
            return output;
        }
        
        std::string
        EAIndexerNULL::resolveDocumentID( docid_t id )
        {
            return "";
        }

        
        
    };
};


extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::EAIndexerNULL();
    }
};
