/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: FerrisCLucene.hh,v 1.2 2010/09/24 21:30:35 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CLUCENE_COMMON_H_
#define _ALREADY_INCLUDED_FERRIS_CLUCENE_COMMON_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>

namespace lucene
{
    namespace debug
    {
        class LuceneBase;
    };
    namespace index
    {
        class IndexWriter;
        class IndexReader;
        class Term;
    };
    namespace search
    {
        class Query;
        class TermQuery;
        class BooleanQuery;
        class Hits;
    };
    typedef search::Hits* l_Hits;
    typedef Loki::SmartPtr< search::Query, 
                            Loki::RefCounted, 
                            Loki::DisallowConversion, 
                            FerrisLoki::FerrisExSmartPointerChecker, 
                            Loki::DefaultSPStorage >  l_Query;
typedef Loki::SmartPtr< search::TermQuery, 
                        Loki::RefCounted, 
                        Loki::AllowConversion, 
                        FerrisLoki::FerrisExSmartPointerChecker, 
                        Loki::DefaultSPStorage >  l_TermQuery;
typedef Loki::SmartPtr< search::BooleanQuery, 
                        Loki::RefCounted, 
                        Loki::DisallowConversion, 
                        FerrisLoki::FerrisExSmartPointerChecker, 
                        Loki::DefaultSPStorage >  l_BooleanQuery;
typedef Loki::SmartPtr< index::IndexWriter, 
                        Loki::RefCounted, 
                        Loki::AllowConversion, 
                        FerrisLoki::FerrisExSmartPointerChecker, 
                        Loki::DefaultSPStorage >  l_IndexWriter;
typedef Loki::SmartPtr< index::IndexReader, 
                        Loki::RefCounted, 
                        Loki::AllowConversion, 
                        FerrisLoki::FerrisExSmartPointerChecker, 
                        Loki::DefaultSPStorage >  l_IndexReader;
typedef Loki::SmartPtr< index::Term, 
                        Loki::RefCounted, 
                        Loki::AllowConversion, 
                        FerrisLoki::FerrisExSmartPointerChecker, 
                        Loki::DefaultSPStorage >  l_Term;


};


#endif
