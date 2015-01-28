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

    $Id: libcreationfulltextindexgeneric.cpp,v 1.2 2010/09/24 21:31:52 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>

#include <FullTextIndexer.hh>
#include <FullTextIndexer_private.hh>

using namespace std;

namespace Ferris
{
    using namespace FullTextIndex;
    
    class CreationStatelessFunctorFullTextIndexLucene
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
            return new CreationStatelessFunctorFullTextIndexLucene();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    fh_context
    CreationStatelessFunctorFullTextIndexLucene::create( fh_context c, fh_context md )
    {
        string stemModeString = getStrSubCtx( md, "stemmer", "Porter" );
        string lex_class      = getStrSubCtx( md, "lexicon-class", IDXMGR_LEXICON_CLASS_DEFAULT );
        bool   caseSensitive = isTrue( getStrSubCtx( md, "case-sensitive", "0" ) );
        bool   dropStopWords = isTrue( getStrSubCtx( md, "drop-stop-words", "0" ) );
        FullTextIndex::StemMode stemMode  = FullTextIndex::STEM_NONE;
        
        try
        {
            if( stemModeString == "Porter" )
                stemMode = FullTextIndex::STEM_PORTER;
            if( stemModeString == "J B Lovins 68" )
                stemMode = FullTextIndex::STEM_J_B_LOVINS_68;
            if( stemModeString == "porter" )
                stemMode = FullTextIndex::STEM_PORTER;
            if( stemModeString == "None" )
                stemMode = FullTextIndex::STEM_NONE;
            else if( stemModeString == "none" )
                stemMode = FullTextIndex::STEM_NONE;
            
            FullTextIndex::fh_idx idx
                = FullTextIndex::createFullTextIndex( md->getDirName(),
                                                      c,
                                                      caseSensitive,
                                                      dropStopWords,
                                                      stemMode,
                                                      lex_class,
                                                      md );

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
        return c;
    }
};
