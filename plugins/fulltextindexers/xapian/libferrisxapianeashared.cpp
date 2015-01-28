/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2006 Ben Martin

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

    $Id: libferrisxapianeashared.cpp,v 1.1 2006/12/07 06:49:43 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "libferrisxapianeashared.hh"


namespace Ferris
{

    //
    // This could be a hash if the URL is too long for xapian terms.
    //
    string makeURLTerm( fh_context c )
    {
        return makeURLTerm( c->getURL() );
    }

    string makeURLTerm( string earl )
    {
        const unsigned int MAX_SAFE_TERM_LENGTH = 240;

        string urlterm("U");
        urlterm += earl;
        if (urlterm.length() > MAX_SAFE_TERM_LENGTH)
            urlterm = std::hash<const char*>()( urlterm.c_str() );

        return urlterm;
    }

    string makeTerm( const string& k, const string& v )
    {
        const unsigned int MAX_SAFE_TERM_LENGTH = 240;

        string ret("");
        ret += k;
        ret += (string)"=";
        ret += v;
        if (ret.length() > MAX_SAFE_TERM_LENGTH)
            ret = std::hash<const char*>()( ret.c_str() );

        return ret;
    }


    /////////////////////////////////////////
    /////////////////////////////////////////
    /////////////////////////////////////////

    stringmap_t& getData( stringmap_t& ret, const Xapian::Document& doc )
    {
        stringstream ifs;
        ifs << doc.get_data();
        boost::archive::binary_iarchive archive( ifs );
        archive >> ret;
    }
    

    docNumSet_t& common_addAllDocumentsMatchingMSet( MSet& matches, docNumSet_t& output,
                                                     m_docidmap_t& m_docidmap,
                                                     m_revdocidmap_t& m_revdocidmap,
                                                     int& m_docidmap_nextid,
                                                     addDocIDFunctor addDocID )
    {
        // Display the results
        LG_IDX_D << matches.get_matches_estimated() << " results found" << endl;

        for (MSetIterator i = matches.begin(); i != matches.end(); ++i)
        {
            stringmap_t data;
            getData( data, i.get_document() );
                
//                 cout << "ID " << *i << " " << i.get_percent() << "% ["
//                      << data["url"] << "]" << endl;

            string earl = data["url"];
            m_revdocidmap_t::const_iterator reviter = m_revdocidmap.find( earl );
            if( reviter != m_revdocidmap.end() )
            {
                addDocID( output, reviter->second );
            }
            else
            {
                m_revdocidmap.insert( make_pair( earl, m_docidmap_nextid ) );
                m_docidmap[ m_docidmap_nextid ] = earl;
                addDocID( output, m_docidmap_nextid );
                ++m_docidmap_nextid;
            }
        }
    }


    bool
    common_isFileNewerThanIndexedVersion( WritableDatabase& m_database, const fh_context& c )
    {
        bool ret = true;

        time_t ct = getTimeAttr( c, "mtime", 0 );
        if( !ct )
            return ret;
            
        string earl = c->getURL();
        string urlterm = makeURLTerm( c );
            
        if ( m_database.term_exists(urlterm) )
        {
            // check the docidtime for the match.

            Enquire enquire( m_database );
            Query query( urlterm );
            enquire.set_query(query);
            MSet matches = enquire.get_mset(0, 500);

            for (MSetIterator i = matches.begin(); i != matches.end(); ++i)
            {
                stringmap_t data;
                getData( data, i.get_document() );

                if( data["url"] == earl )
                {
                    return toType<time_t>(data["docidtime"]) < ct;
                }
            }
        }

        return ret;
    }

    void setupNewDocument( Xapian::Document& doc, fh_context c, string docidtime )
    {
        stringmap_t data;
        data["url"] = c->getURL();
        data["docidtime"] = docidtime;
        stringstream ofs;
        boost::archive::binary_oarchive archive( ofs );
        const stringmap_t& d = data;
        archive << d;
        doc.set_data( ofs.str() );

        string urlterm = makeURLTerm( c );
        // Add some 'term's for later probing.
        doc.add_term(urlterm); 
        string docidtimeterm = "docidtime-";
        docidtimeterm += docidtime;
        doc.add_term(docidtimeterm);
    }
    
    
    
};
