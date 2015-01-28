/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2006 Ben Martin

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

    $Id: libferrisxapianeashared.hh,v 1.2 2010/09/24 21:31:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/version.hpp>

#include <xapian.h>
#include <xapian/queryparser.h>

#include "Ferris/Ferris.hh"
#include "Ferris/Ferris_private.hh"

namespace Ferris
{
    using namespace Xapian;
    using namespace std;

    string makeURLTerm( string earl );
    string makeURLTerm( fh_context c );
    string makeTerm( const string& k, const string& v );

    /////////////////////////////////////////

    stringmap_t& getData( stringmap_t& ret, const Xapian::Document& doc );
    typedef guint32 docid_t;
    typedef std::set< docid_t > docNumSet_t;

    typedef map< string, int > m_revdocidmap_t;
    typedef map< int, string > m_docidmap_t;
    typedef Loki::Functor< void, TYPELIST_2( docNumSet_t&, docid_t ) > addDocIDFunctor;

    docNumSet_t& common_addAllDocumentsMatchingMSet( MSet& matches, docNumSet_t& output,
                                                     m_docidmap_t& m_docidmap,
                                                     m_revdocidmap_t& m_revdocidmap,
                                                     int& m_docidmap_nextid,
                                                     addDocIDFunctor addDocID );
    bool common_isFileNewerThanIndexedVersion( WritableDatabase& m_database, const fh_context& c );
    void setupNewDocument( Xapian::Document& doc, fh_context c, string docidtime );
    

    
};

