/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2002 Ben Martin

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

    $Id: libcreationfulltextindex.cpp,v 1.3 2010/09/24 21:31:52 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>

#include <FullTextIndexer.hh>
#include <FullTextIndexer_private.hh>

#include "fulltextindexers_custom_plugin/libftxcustomferris.hh"

using namespace std;

namespace Ferris
{
    using namespace FullTextIndex;
    
    class CreationStatelessFunctorFullTextIndex
        :
        public CreationStatelessFunctor
    {
    public:
        virtual fh_context create( fh_context c, fh_context md );
    };

    extern "C"
    {
        fh_CreationStatelessFunctor
        Create()
        {
            return new CreationStatelessFunctorFullTextIndex();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    fh_context
    CreationStatelessFunctorFullTextIndex::create( fh_context c, fh_context md )
    {
        string stemModeString = getStrSubCtx( md, "stemmer", "J B Lovins 68" );
        string lex_class      = getStrSubCtx( md, "lexicon-class", IDXMGR_LEXICON_CLASS_DEFAULT );
        string dgap_code      = getStrSubCtx( md, "document-number-gap-code",
                                              IDXMGR_DGAP_CODE_DEFAULT );
        string f_d_t_code     = getStrSubCtx( md, "frequency-of-term-in-document-code",
                                              IDXMGR_FDT_CODE_DEFAULT );
        bool caseSensitive = isTrue( getStrSubCtx( md, "case-sensitive", "0" ) );
        bool dropStopWords = isTrue( getStrSubCtx( md, "drop-stop-words", "0" ) );
        FullTextIndex::StemMode stemMode  = FullTextIndex::STEM_J_B_LOVINS_68;
        int skiplistMaxSize = toint( getStrSubCtx( md, "inverted-skiplist-max-size",
                                                   IDXMGR_MAXPTRS_PER_SKIPLISTCHUNK_DEFAULT ) );
        try
        {
            if( stemModeString == "J B Lovins 68" )
                stemMode = FullTextIndex::STEM_J_B_LOVINS_68;
            else if( stemModeString == "none" )
                stemMode = FullTextIndex::STEM_NONE;

//            cerr << "About to create FullTextIndex object" << endl;
            
            FullTextIndex::fh_idx baseidx
                = FullTextIndex::createFullTextIndex( "native",
                                                      c,
                                                      caseSensitive,
                                                      dropStopWords,
                                                      stemMode,
                                                      lex_class );
            fh_nidx idx = dynamic_cast< FullTextIndexManagerNative* >( GetImpl( baseidx ));
//            cerr << "created FullTextIndex object" << endl;

            idx->setInvertedSkiplistMaxSize( skiplistMaxSize );
            idx->setDocumentNumberGapCode( dgap_code );
            if( !f_d_t_code.empty() )
                idx->setFrequencyOfTermInDocumentCode( f_d_t_code );
//            cerr << "about to drop FullTextIndex object" << endl;
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "SL_SubCreate_fulltextindex() error creating index"
               << " URL:" << c->getURL()
               << " error:" << e.what()
               << endl;
            cerr << "ftx module error:" << e.what() << endl;
            Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
        }
//        cerr << "about to dropped FullTextIndex object" << endl;
        return c;
    }
};
