/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2002 Ben Martin

    This file is part of libferris.

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

    $Id: fcat.cpp,v 1.11 2010/09/24 21:31:08 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * return 5 for not modified
 * 
 *
 *
 *
 *
bash -c '/ferris/apps/cat/fcat Makefile 2>/dev/null | wc -l'



*/
#include <config.h>


#include <Ferris.hh>
#include <FerrisDOM.hh>
#include <popt.h>
#include <unistd.h>

#include <Ferris/EAIndexerMetaInterface.hh>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "fcat";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

unsigned long OutputHeadersOnFD = 0;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void OutputHeadersOnFD_cb( fh_context c, const stringset_t& strset )
{
    try
    {
        cerr << "OutputHeadersOnFD_cb()" << endl;
        fh_stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> " << endl;
        ss << "<headerinfo> " << endl;
        for( stringset_t::const_iterator iter = strset.begin(); iter != strset.end(); ++iter )
        {
            string k = *iter;
            cerr << "OutputHeadersOnFD_cb() k:" << k <<  endl;
            string v = getStrAttr( c, k, "" );
            ss << "<" << k << ">";
            ss << v;
            ss << "</" << k << ">" << endl;
        }
        ss << "</headerinfo> " << endl;

        fh_ostream oss = Factory::MakeFdOStream( OutputHeadersOnFD );
        oss << tostr(ss) << flush;
    }
    catch( exception& e )
    {
        cerr << "OutputHeadersOnFD_cb() e:" << e.what() << endl;
    }
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    unsigned long FerrisInternalAsyncMessageSlave      = 0;
    const char*   FerrisInternalAsyncMessageSlaveAttrs = 0;
    const char*   SourceAttrName         = "content";
    const char*   EAIndexPath_CSTR = 0;

    try
    {
        const char* DownloadIfMtimeSinceStr= 0;
        unsigned long DownloadIfMtimeSince = 0;
        unsigned long Verbose              = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "download-if-mtime-since", 0, POPT_ARG_INT, &DownloadIfMtimeSince, 0,
                  "only transfer file content if it has been modified after given long time", "" },

                { "output-headers-on-fd", 0, POPT_ARG_INT, &OutputHeadersOnFD, 0,
                  "when header info is received output an XML docuemnt on the given already open fd.",
                  "" },
                
                { "download-if-mtime-since-string", 0, POPT_ARG_STRING,
                  &DownloadIfMtimeSinceStr, 0,
                  "only transfer file content if it has been modified after given string time", "" },

                { "src-attr", 'a', POPT_ARG_STRING,
                  &SourceAttrName, 0,
                  "cat an EA rather than the content itself", "" },

                { "ea", 0, POPT_ARG_STRING,
                  &SourceAttrName, 0,
                  "cat an EA rather than the content itself", "" },
                
                { "ferris-internal-async-message-slave", 0, POPT_ARG_NONE, &FerrisInternalAsyncMessageSlave, 0,
                  "used by libferris itself to perform async queries through a slave process", "" },

                { "ferris-internal-async-message-slave-attrs", 0, POPT_ARG_STRING,
                  &FerrisInternalAsyncMessageSlaveAttrs, 0,
                  "used by libferris itself to perform async queries through a slave process", "" },

                { "ea-index-path", 0, POPT_ARG_STRING, &EAIndexPath_CSTR, 0,
                  "which EA Index to use", "" },

                
//             { "target-directory", 0, POPT_ARG_STRING, &DstNameCSTR, 0,
//               "Specify destination explicity, all remaining URLs are assumed to be source files",
//               "DIR" },

                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
//         switch (c) {
//         }
        }

        if (argc < 2)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        if( EAIndexPath_CSTR )
        {
            ::Ferris::EAIndex::Factory::setDefaultEAIndexPath( EAIndexPath_CSTR );
        }
        

        stringlist_t srcs;
        srcs = expandShellGlobs( srcs, optCon );

        if( FerrisInternalAsyncMessageSlave )
        {
            fh_ostream mss = ::Ferris::Factory::fcout();
            stringlist_t rdnlist;
            Util::parseSeperatedList( FerrisInternalAsyncMessageSlaveAttrs,
                                rdnlist, back_inserter( rdnlist ));
            stringlist_t::iterator srcsiter = srcs.begin();
            stringlist_t::iterator srcsend  = srcs.end();
            for( ; srcsiter != srcsend ; ++srcsiter )
            {
                stringlist_t::iterator rdniter = rdnlist.begin();
                stringlist_t::iterator  rdnend = rdnlist.end();
                for( ; rdniter != rdnend; ++rdniter )
                {
                    string rdn = *rdniter;
                    
                    try
                    {
                        string srcURL = *srcsiter;
                        fh_context c = Resolve( srcURL );
                        string v = getStrAttr( c, rdn, "", true, true );
                    
                        stringmap_t m;
                        m["v"] = v;
                        m["eaname"] = rdn;
                        XML::writeMessage( mss, m );
                        mss << flush;
                    }
                    catch( exception& e )
                    {
                        string emsg = e.what();
                        if( FerrisInternalAsyncMessageSlave )
                        {
                            fh_ostream oss = ::Ferris::Factory::fcout();
                            stringmap_t m;
                            m["outofband-error"] = emsg;
                            m["eaname"] = rdn;
                            XML::writeMessage( mss, m );
                            exit_status = 1;
                            mss << flush;
                        }
                    }
                }
            }
            return exit_status;
        }




            
        stringlist_t::iterator srcsiter = srcs.begin();
        stringlist_t::iterator srcsend  = srcs.end();
        for( ; srcsiter != srcsend ; ++srcsiter )
        {
            string srcURL = *srcsiter;
            fh_istream iss;
            fh_ostream oss = Factory::MakeFdOStream( STDOUT_FILENO );
            
            try
            {
//                cerr << "srcURL :" << srcURL << endl;
                
                if( srcURL == "-" )
                {
                    iss = Factory::MakeFdIStream( STDIN_FILENO );
                }
                else
                {
                    fh_context c = Resolve( srcURL );
                    if( DownloadIfMtimeSince )
                        setStrAttr( c, "download-if-mtime-since",
                                    tostr(DownloadIfMtimeSince),
                                    false, false );
                    if( DownloadIfMtimeSinceStr )
                    {
                        try
                        {
                            cerr << "DownloadIfMtimeSinceStr:" << DownloadIfMtimeSinceStr << endl;
                            cerr << "c:" << c->getURL() << endl;
                            setStrAttr( c, "download-if-mtime-since-display",
                                        DownloadIfMtimeSinceStr,
                                        false, true );
                        }
                        catch( exception& e )
                        {
                            cerr << "DownloadIfMtimeSinceStr e:" << e.what() << endl;
                        }
                    }

                    sigc::connection hdrcon;
                    
                    if( OutputHeadersOnFD )
                    {
                        hdrcon = c->getContextEvent_Headers_Received_Sig().connect(
                            sigc::ptr_fun( OutputHeadersOnFD_cb ) );
                    }

                    /*
                     * Either cat an EA or the file itself,
                     * SourceAttrName==content by default
                     */
                    fh_attribute a = c->getAttribute( SourceAttrName );
                    iss = a->getIStream();
                    
                    if( OutputHeadersOnFD )
                    {
                        hdrcon.disconnect();
                    }
                    
                }

//                 iss.exceptions( ios_base::badbit | ios_base::failbit );
//                 cerr << "set exceptions to on...." << endl;

                copy( istreambuf_iterator<char>(iss), istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(oss));
                oss << flush;
                
//                 cerr << " iss is good():"<< iss.good() << endl;
//                 cerr << " iss is eof():"<< iss.eof() << endl;
//                 cerr << " iss is state:"<< iss.rdstate() << endl;
//                 if ( iss.rdstate() & ifstream::failbit )
//                     cerr << " iss has fail bit set." << endl;
//                 if ( iss.rdstate() & ifstream::badbit )
//                     cerr << " iss has bad bit set." << endl;

                string emsg = "";
                
                if( !iss.good() )
                {
                    emsg = getIOErrorDescription( iss, srcURL );
                }
//                 else if( haveIOError( iss ) )
//                 {
//                     emsg = getIOErrorDescription( iss, srcURL );
//                 }
                else if( haveIOError( oss ) )
                {
                    emsg = getIOErrorDescription( oss, "<STDOUT>" );
                }

                if( !emsg.empty() )
                {
                    cerr << emsg << endl;
                    exit_status = 1;
                }
            }
            catch( ContentNotModified& e )
            {
                cerr << "not modified:" << srcURL << endl;
                return 5;
            }
            catch( exception& e )
            {
                string emsg = e.what();
                cerr << "error:" << emsg << endl;
                exit_status = 1;
            }
        }
    }
    catch( exception& e )
    {
        string emsg = e.what();
        cerr << "error:" << emsg << endl;
        exit(1);
    }
    return exit_status;
}


