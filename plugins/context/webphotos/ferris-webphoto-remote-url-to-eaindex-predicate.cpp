/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-webphoto-upload
    Copyright (C) 2007 Ben Martin

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

    $Id: ferris-webphoto-remote-url-to-eaindex-predicate.cpp,v 1.6 2009/08/07 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * return 5 for not modified
 */

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisBoost.hh>
#include "plugins/context/webphotos/libferriswebphotos_shared.hh"


#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-webphoto-remote-url-to-eaindex-predicate";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

typedef set< string > photoidset_t;
void dumpSet( const string& serverName, photoidset_t& col, fh_ostream oss )
{
    stringstream qss;
    bool compositeQ = false;
    if( distance( col.begin(), col.end() ) > 1 )
    {
        compositeQ = true;
        oss << "(|";
    }

    photoidset_t::iterator coliter = col.begin();
    photoidset_t::iterator colend  = col.end();
    for( ; coliter != colend ; ++coliter )
    {
        string photoid = *coliter;
        oss << "(" << serverName << "-photo-id" << "==" << photoid << ")";
    }
    
    if( compositeQ )
        oss << ")";
    oss << flush;
}


string getServerName( const std::string& earl )
{
    string ret = "";

    if( contains( earl, "flickr.com" ) )
        ret = "flickr";

    if( contains( earl, "23hq.com" ) )
        ret = "23hq";

    if( ret.empty() )
    {
        cerr << "Can not work out what server your photos are on..." << endl;
        cerr << "URL:" << earl << endl;
        exit(1);
    }
    
    return ret;
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose              = 0;
        unsigned long PhotoPageURLs        = 0;
        unsigned long AlbumURL             = 0;
//        unsigned long UseDefaultFlickrAccount = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "photo-url", 'p', POPT_ARG_NONE, &PhotoPageURLs, 0,
                  "urls are to webphoto photo page", "" },

                { "album-url", 'a', POPT_ARG_NONE, &AlbumURL, 0,
                  "urls are to webphoto photo page", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* url1 url2 ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
//         switch (c) {
//         }
        }
        LG_WEBPHOTO_D << "AlbumURL:" << AlbumURL << endl;

        if (argc < 2)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
                
        
        stringlist_t srcs;
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string RootName = RootNameCSTR;
            srcs.push_back( RootName );
        }

        // http://www.flickr.com/photos/foo/123456789/
        // http://www.23hq.com/monkeyiq/photo/1815816
        if( PhotoPageURLs )
        {
            const int r_expected_matches = 2;
//            boost::regex r( "^.*\\((flickr|23hq)\\.com\\)/.*/([0-9]+)/*$" );
            boost::regex r( "^.*/([0-9]+)[/]*$" );

            string serverName = "";
            photoidset_t col;
            
            stringlist_t::iterator srcsiter = srcs.begin();
            stringlist_t::iterator srcsend  = srcs.end();
            for( ; srcsiter != srcsend ; ++srcsiter )
            {
                string earl = *srcsiter;

                if( serverName.empty() )
                    serverName = getServerName(earl);
                LG_WEBPHOTO_D << "PhotoPageURLs, serv:" << serverName << "  earl:" << earl << endl;
                
                boost::smatch matches;
                if(boost::regex_match( earl, matches, r ))
                {
                    LG_WEBPHOTO_D << "matches.size():" << matches.size() << endl;
                    
                    if( matches.size() == r_expected_matches )
                    {
                        string photoid = matches[1];
                        col.insert( photoid );
                        LG_WEBPHOTO_D << "photoid:" << photoid << endl;
                    }
                }
            }

            dumpSet( serverName, col, Factory::fcout() );
        }
        else if( AlbumURL )
        {
            // URL is like.
            // http://www.flickr.com/photos/foo/

            // regex seeks.
            // <a href="/photos/foo/123456789/">
            // <a href="/photos/foo/123456789/in/set-12345678901234567/" title="foo" ...
            // 23hq
            // <a href="http://www.23hq.com/monkeyiq/photo/1815816">
            const int r_expected_matches = 3;

            fh_rex flickr_rex = toregexh(
                "<a href=\"/photos/([^/]+)/([0-9]+)/" );
            fh_rex twothreehq_rex = toregexh(
                "<a href=\"http://www.23hq.com/([^/]+)/photo/([0-9]+)[/]*" );
            fh_rex r = flickr_rex;
            
            photoidset_t photoidset;
            string serverName = "";
            
            stringlist_t::iterator srcsiter = srcs.begin();
            stringlist_t::iterator srcsend  = srcs.end();
            for( ; srcsiter != srcsend ; ++srcsiter )
            {
                string earl = *srcsiter;
                LG_WEBPHOTO_D << "earl:" << earl << endl;
                if( serverName.empty() )
                    serverName = getServerName(earl);

                string sn = getServerName(earl);
                if( sn == "23hq" )
                    r = twothreehq_rex;
                
                fh_context c = Resolve( earl );
                string content = getStrAttr( c, "content", "", true, true );
                std::string::const_iterator cstart = content.begin();
                std::string::const_iterator cend = content.end();
                boost::match_flag_type flags = boost::match_default; 
                
//                LG_WEBPHOTO_D << "content:" << content << endl;
                LG_WEBPHOTO_D << "content.sz:" << content.length() << endl;
                boost::smatch matches;
                while(boost::regex_search( cstart, cend, matches, *r ))
                {
                    LG_WEBPHOTO_D << "matches.size():" << matches.size() << endl;
                    
                    if( matches.size() == r_expected_matches )
                    {
                        string userName = matches[1];
                        string photoid  = matches[2];
                        LG_WEBPHOTO_D << "photoid:" << photoid << endl;

                        photoidset.insert( photoid );
                        cstart = matches[0].second; 
                        // update flags: 
                        flags |= boost::match_prev_avail; 
                        flags |= boost::match_not_bob; 
                    }
                }
            }

            dumpSet( serverName, photoidset, Factory::fcout() );
        }
        else
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        
//        fh_webPhotos wf = Factory::getDefaultFlickrWebPhotos();
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


        
                
        
