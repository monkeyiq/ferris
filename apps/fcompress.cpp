/******************************************************************************
*******************************************************************************
*******************************************************************************

    fcompress command line client
    Copyright (C) 2003 Ben Martin

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

    $Id: fcompress.cpp,v 1.4 2010/09/24 21:31:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris.hh>
#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "fcompress";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        const char* CreateTypeName_CSTR    = 0;
        unsigned long Compress             = 0;
        unsigned long Decompress           = 0;
        unsigned long UseGZip              = 0;
        unsigned long UseBZip2             = 0;
        unsigned long UseNoCompression     = 0;
        unsigned long Verbose              = 0;
        unsigned long NumberOf512ByteBlocksPerChunk = 0;
        unsigned long CompressionLevel              = 1;
        unsigned long UseFastestCompression         = 0;
        unsigned long UseSlowestCompression         = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "compress", 'c', POPT_ARG_NONE, &Compress, 0,
                  "compress given files", "" },

                { "decompress", 'x', POPT_ARG_NONE, &Decompress, 0,
                  "decompress given files", "" },

                { "use-gzip", 'z', POPT_ARG_NONE, &UseGZip, 0,
                  "use gzip when selecting compression", "" },

                { "use-bzip2", 'j', POPT_ARG_NONE, &UseBZip2, 0,
                  "use bzip2 when selecting compression", "" },

                { "use-no-compression", 0, POPT_ARG_NONE, &UseNoCompression, 0,
                  "just chunk the file and dont compress the chunks (for testing mainly)", "" },

                { "block-size", 'b', POPT_ARG_INT, &NumberOf512ByteBlocksPerChunk, 0,
                  "block size of Nx512 bytes used for chunking (default varies)", "" },

                { "compression-level", 0, POPT_ARG_INT, &CompressionLevel, 0,
                  "use compression level (1-9 for gzip, 1==speed) (1-9 for bzip2 means 100-900k blocks", "1" },

                { 0, '1', POPT_ARG_NONE, &UseFastestCompression, 0,
                  "use best compression viz CPU time required", "" },

                { 0, '9', POPT_ARG_NONE, &UseSlowestCompression, 0,
                  "use best compression viz size of output", "" },

                { "create-type", 0, POPT_ARG_STRING, &CreateTypeName_CSTR, 0,
                  "what form of context to store the chunks in (dir/db4/gdbm/xml/etc)", 0 },
                

//                 { "src-attr", 'a', POPT_ARG_STRING,
//                   &SourceAttrName, 0,
//                   "cat an EA rather than the content itself", "" },
                
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

        if (argc < 2 || (!Compress && !Decompress) )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        if( UseFastestCompression ) CompressionLevel = 1;
        if( UseSlowestCompression ) CompressionLevel = 9;
        
        if( !NumberOf512ByteBlocksPerChunk )
        {
            if( UseBZip2 )
                NumberOf512ByteBlocksPerChunk = CompressionLevel * 200;
            else
                NumberOf512ByteBlocksPerChunk = 512;
        }
        
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            try
            {
                string srcURL = tmpCSTR;
                string extension = "";

                int compress_mode = Factory::COMPRESS_INVALID;

                if( UseNoCompression ) compress_mode = Factory::COMPRESS_NONE;
                if( UseGZip )          compress_mode = Factory::COMPRESS_GZIP;
                if( UseBZip2 )         compress_mode = Factory::COMPRESS_BZIP2;

                fh_context c = Resolve( srcURL );
                fh_context target = 0;

                string CreateTypeName;
                if( CreateTypeName_CSTR )
                    CreateTypeName = CreateTypeName_CSTR;
                
                if( Compress && CreateTypeName_CSTR )
                {
                    fh_mdcontext md = new f_mdcontext();
                    fh_mdcontext child = md->setChild( CreateTypeName, "" );

                    // FIXME: This is very hacky, but we need to be fairly sure
                    // that overmounting will work for target.
                    if( CreateTypeName == "db4" )  extension = ".db";
                    if( CreateTypeName == "gdbm" ) extension = ".gdbm";
                    if( CreateTypeName == "tdb" )  extension = ".tdb";
                    if( CreateTypeName == "xml" )  extension = ".xml";
                    
                    string rdn = c->getDirName() + ".compressed" + extension;
                    cerr << "Creating new context at path:" << c->getURL()
                         << " rdn:" << rdn
                         << " type:" << CreateTypeName 
                         << " extension:" << extension
                         << endl;
                    child->setChild( "name", rdn );
                    target = c->getParent()->createSubContext( "", md );
                }
                
                if( Compress )
                {
                    string rdn   = c->getDirName();
                    fh_context p = c->getParent();

                    if( !CreateTypeName.empty() )
                    {
//                        cerr << "Setting ferris-type:" << CreateTypeName << endl;
                        setStrAttr( target, "ferris-type", CreateTypeName,1,1);
                    }

                    Factory::ConvertToCompressedChunkContext( c,
                                                              target,
                                                              NumberOf512ByteBlocksPerChunk*512,
                                                              compress_mode,
                                                              CompressionLevel );
//                     if( !extension.empty() )
//                     {
//                         p->rename( rdn, rdn + extension );
//                         Shell::CreateLink( p->getSubContext( rdn + extension ), p, rdn );
//                     }

                }
                if( Decompress )
                    Factory::ConvertFromCompressedChunkContext( c );
            }
            catch( exception& e )
            {
                cerr << "error:" << e.what() << endl;
                exit_status = 1;
            }
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


