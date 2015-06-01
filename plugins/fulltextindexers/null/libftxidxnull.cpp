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

    $Id: libftxidxnull.cpp,v 1.2 2008/12/19 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include "config.h"

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FullTextIndexer_private.hh>

#include <string>
using namespace std;


namespace Ferris
{
    namespace FullTextIndex 
    {

        class FERRISEXP_DLLLOCAL FullTextIndexerNull
            :
            public MetaFullTextIndexerInterface
        {
        protected:

            FullTextIndexerNull();
            
            virtual void Setup();
            virtual void CreateIndexBeforeConfig( fh_context c,
                                                  bool caseSensitive,
                                                  bool dropStopWords,
                                                  StemMode stemMode,
                                                  const std::string& lex_class,
                                                  fh_context md );
            virtual void CreateIndex( fh_context c,
                                      bool caseSensitive,
                                      bool dropStopWords,
                                      StemMode stemMode,
                                      const std::string& lex_class,
                                      fh_context md );
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );

            virtual docNumSet_t& addAllDocumentsMatchingTerm(
                const std::string& term,
                docNumSet_t& output,
                int limit );
            virtual std::string resolveDocumentID( docid_t );

        public:

            static MetaFullTextIndexerInterface* Create();

            template <class DocTermsClass>
            void
            addToIndexDocTermsClass( fh_context c,
                                     fh_docindexer di );
            
        };

        

        /************************************************************/
        /************************************************************/
        /************************************************************/

        FullTextIndexerNull::FullTextIndexerNull()
        {
        }

        MetaFullTextIndexerInterface*
        FullTextIndexerNull::Create()
        {
            return new FullTextIndexerNull();
        }
        void
        FullTextIndexerNull::Setup()
        {
        }

        void
        FullTextIndexerNull::CreateIndexBeforeConfig( fh_context c,
                                                      bool caseSensitive,
                                                      bool dropStopWords,
                                                      StemMode stemMode,
                                                      const std::string& lex_class,
                                                      fh_context md )
        {
        }
        
        
        void
        FullTextIndexerNull::CreateIndex( fh_context c,
                                          bool caseSensitive,
                                          bool dropStopWords,
                                          StemMode stemMode,
                                          const std::string& lex_class,
                                          fh_context md )
        {
        }

        void
        FullTextIndexerNull::addToIndex( fh_context c,
                                         fh_docindexer di )
        {
            string s;
            fh_istream iss;
            try
            {
                fh_attribute a = c->getAttribute( "as-text" );
                iss = a->getIStream();
            }
            catch( exception& e )
            {
                iss = c->getIStream();
            }
            streamsize bytesDone    = 0;
            streamsize signalWindow = 0;
            streamsize totalBytes   = toType<streamsize>(getStrAttr( c, "size", "0" ));
            

            bool isCaseSensitive = this->isCaseSensitive();
            bool DropStopWords   = getDropStopWords();
            StemMode stemmer     = getStemMode();
            
//            cerr << "Starting to parse tokens" << endl;
            
            while( !(s = di->getToken( iss )).empty() )
            {
//                cerr << "got token:" << s << endl;

                if( !isCaseSensitive )
                {
                    s = foldcase( s );
//                    cerr << "case folded:" << s << endl;
                }

                s = stem( s, stemmer );

                if( DropStopWords )
                {
                    if( getStopWords().count( s ) )
                    {
                        continue;
                    }
                }

//                cerr << "s:" << s << endl;
            }
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        std::string
        FullTextIndexerNull::resolveDocumentID( docid_t id )
        {
            fh_stringstream ess;
            ess << "NULLFTX: Failed to resolve document ID:" << id << endl;
            Throw_FullTextIndexException( tostr( ess ), 0 );
        }
        
        docNumSet_t&
        FullTextIndexerNull::addAllDocumentsMatchingTerm(
            const std::string& term,
            docNumSet_t& output,
            int limit )
        {
            return output;
        }
        
        /**************************************************/
        /**************************************************/

        
    };
};
