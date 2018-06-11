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

    $Id: ferris-webphoto-upload.cpp,v 1.7 2009/08/07 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

/*
 * return 0 for success
 * return 1 for generic error
 * return 5 for not modified
 */

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisBoost.hh>
#include "plugins/context/libferriswebphotos_shared.hh"


#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferris-webphoto-upload";

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


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
//         const char* SourceAttrName         = "content";
//         const char* DownloadIfMtimeSinceStr= 0;
        const char* IncludeEAIsPresentRegex_CSTR = 0;
        const char* IncludeEAandValueRegex_CSTR = 0;
        const char* Desc_CSTR = 0;
        const char* DescFromEA_CSTR = 0;
        const char* ServiceName_CSTR = 0;
        unsigned long DebugVerbose         = 0;
        unsigned long Verbose              = 0;
//        unsigned long isPublic              = 0;
        unsigned long isFriend              = 0;
        unsigned long isFamily              = 0;
        long largestDimensionExplicit = -1;

        struct poptOption optionsTable[] =
            {
                { "service-name", 'n', POPT_ARG_STRING, &ServiceName_CSTR, 0,
                  "name of webphoto service, eg. 23hq, flickr", "" },
                
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "debug-verbose", 0, POPT_ARG_NONE, &DebugVerbose, 0,
                  "show in depth details of what is happening. debug option", "" },

                { "scale", 0, POPT_ARG_INT, &largestDimensionExplicit, 0,
                  "scale largest side to this size preserving aspect ratio", "" },
                
                { "description", 'd', POPT_ARG_STRING, &Desc_CSTR, 0,
                  "description for uploaded image(s)", "" },

                { "description-from-ea", 'f', POPT_ARG_STRING, &DescFromEA_CSTR, 0,
                  "for each image, try to read this EA and use the value as the description", "" },
                

                { "include-ea-is-present-regex", 'p', POPT_ARG_STRING, &IncludeEAIsPresentRegex_CSTR, 0,
                  "If these EA are present on the uploading files then place tags on the uploaded file with the EA name", "" },

                { "include-ea-and-value-regex", 'a', POPT_ARG_STRING, &IncludeEAandValueRegex_CSTR, 0,
                  "If these EA are present on the uploading files then place tags on the uploaded file in the form eaname=eavalue taken from the source", "" },


//                { "public", 0, POPT_ARG_NONE, &isPublic, 0, "image are public viewable", "" },
                { "friend", 0, POPT_ARG_NONE, &isFriend, 0, "image are friend viewable", "" },
                { "family", 0, POPT_ARG_NONE, &isFamily, 0, "image are family viewable", "" },
                
                
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

        if (argc < 2 || !ServiceName_CSTR)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        stringlist_t srcs;
        srcs = expandShellGlobs( srcs, optCon );

        fh_webPhotos wf = Factory::getDefaultWebPhotosForShortName( ServiceName_CSTR );
        string token = wf->getToken();
        if( token.empty() )
        {
            cerr << "default user:" << wf->getDefaultUsername() << endl;
            cerr << "selected user:" << wf->getUserName() << endl;
            cerr << "AUTH token is empty! exiting..." << endl;
            exit(1);
        }

        fh_webPhotoUpload wu = new WebPhotoUpload( wf );

        if( Verbose )
            wu->setVerboseStream( Factory::fcout() );
        wu->setDebugVerbose( DebugVerbose );
        if( IncludeEAIsPresentRegex_CSTR )
        {
            fh_rex r = toregexh( IncludeEAIsPresentRegex_CSTR );
            wu->setIncludeEAisPresentRegex( r );
        }
        if( IncludeEAandValueRegex_CSTR )
        {
            fh_rex r = toregexh( IncludeEAandValueRegex_CSTR );
            wu->setIncludeEAandValueRegex( r );
        }
        if( !isFriend && !isFamily )
            wu->setPublicViewable( true );
        wu->setFriendViewable( isFriend );
        wu->setFamilyViewable( isFamily );
        if( largestDimensionExplicit != -1 )
        {
            wu->setLargestDimensionExplicit( largestDimensionExplicit );
        }
        string Desc = Desc_CSTR ? Desc_CSTR : "";
        if( DescFromEA_CSTR )
        {
            wu->setDescriptionFromEA( DescFromEA_CSTR );
        }
        
        
        stringlist_t::iterator srcsiter = srcs.begin();
        stringlist_t::iterator srcsend  = srcs.end();
        for( ; srcsiter != srcsend ; ++srcsiter )
        {
            string srcURL = *srcsiter;

            try
            {
                fh_context c = Resolve( srcURL );
                wu->upload( c, "", Desc );
            }
            catch( exception& e )
            {
                cerr << "processing file:" << srcURL << endl;
                cerr << "error:" << e.what() << endl;
                exit_status = 1;
            }
        }

        cout << "See uploads here...  " << wu->getPostUploadURL() << endl;
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


        
                
