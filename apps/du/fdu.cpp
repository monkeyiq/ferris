/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris fdu
    Copyright (C) 2008 Ben Martin

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

    $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris/Ferris.hh>
#include <Ferrisls.hh>
#include <Ferris/Ferrisls_AggregateData.hh>
#include <Ferris/FerrisBoost.hh>

#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;

#define DEBUG 0

const string PROGRAM_NAME = "fdu";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

fh_display_aggdata  d = 0;

unsigned long HumanReadable = 0;
unsigned long ShowCountsForAll = 0;
unsigned long ShowApparentSize = 0;
const char*   BlockSize_CSTR = 0;
unsigned long ShowBytes = 0;
unsigned long ShowTotal = 0;
unsigned long DereferenceArgs = 0;
const char*   Files0From= 0;
unsigned long BlockSize1K = 0;
unsigned long BlockSize1M = 0;
unsigned long CountLinks = 0;
unsigned long DereferenceAllLinks = 0;
unsigned long NullTerminateLines = 0;
unsigned long SeparateDirs = 0;
unsigned long Summarize = 0;
unsigned long OneFileSystem = 0;
const char*   ExcludeFrom = 0;
const char*   Exclude = 0;
unsigned long MaxDepth = 0;
unsigned long ShowMTime = 0;
const char*   ShowXTime_CSTR = "none";
const char*   TimeStyle_CSTR = 0;
unsigned long ShowVersion = 0;
unsigned long Debug = 0;
unsigned long XML = 0;

    
const char*   DontDescendRegexCSTR = 0;
const char* ContextSeperator = "";
unsigned long SeperateEachContextWithNewLine = 0;
const char* FieldSeperator   = "\t";


bool ShowCTime = false;
bool ShowATime = false;
string time_format = "";


string showByteCount( guint64 v )
{
    if( HumanReadable )
    {
        return Util::convertByteString( v );
    }
    return tostr( v );
}

void presentData( const Ferrisls_aggregate_t& data, const std::string& earl )
{
    guint64 sizein1kblocks = data.sizeIn1KBlocks();
    guint64 sz = sizein1kblocks;
    if( HumanReadable )
        sz = data.byteSizeOnDisk();
    else
    {
        if( BlockSize_CSTR )
        {
            guint64 v = Util::convertByteString( (string)BlockSize_CSTR );
            sz = sizein1kblocks * 1024.0 / v;
            cerr << "blocksz given v:" << v << " total blocks:" << sizein1kblocks << endl;
        }
    }
                
    cout << showByteCount(sz);
                    
    if( ShowMTime )
        cout << FieldSeperator << Time::toTimeString( data.newestmtime, time_format );
    if( ShowCTime )
        cout << FieldSeperator << Time::toTimeString( data.newestmtime, time_format );
    if( ShowATime )
        cout << FieldSeperator << Time::toTimeString( data.newestmtime, time_format );

    cout << FieldSeperator << earl << endl;
}



typedef list< Ferrisls_aggregate_t > aggregateStack_t;
aggregateStack_t aggregateStack;
void EnteringContext( Ferrisls& ls, fh_context c )
{
//    cerr << "EnteringContext() c:" << c->getURL() << endl;
    
    static Util::SingleShot virgin;
    if( virgin )
    {
//        cerr << "EnteringContext(IGNORING FIRST) c:" << c->getURL() << endl;
        Ferrisls_aggregate_t a;
        a.reset();
        aggregateStack.push_back( a );
        return;
    }
    Ferrisls_aggregate_t a = d->getData( AGGDATA_RECURSIVE );
    aggregateStack.push_back( d->getData( AGGDATA_RECURSIVE ) );
    d->getData( AGGDATA_RECURSIVE ).reset();
}

void LeavingContext( Ferrisls& ls, fh_context c )
{
//    cerr << "LeavingContext() c:" << c->getURL() << endl;
    presentData( d->getData( AGGDATA_RECURSIVE ), c->getURL() );
        
    Ferrisls_aggregate_t a = aggregateStack.back();
    aggregateStack.pop_back();

    presentData( d->getData( AGGDATA_RECURSIVE ), c->getURL() );
//    presentData( a, c->getURL() );
    d->setData( d->getData( AGGDATA_RECURSIVE ) + a, AGGDATA_RECURSIVE );

}

int main( int argc, const char** argv )
{
    Ferrisls ls;

    
    struct poptOption optionsTable[] = {

//         { 0, 'x', POPT_ARG_NONE, &HorizList, 0,
//           "list entries by lines instead of by columns", 0 },

//         { "context", 'Z', POPT_ARG_NONE, &ShowSELinuxContext, 0,
//           "Display SELinux security context so it fits on most displays.", 0 },

//         { "lcontext", 0, POPT_ARG_NONE, &ShowSELinuxContextLong, 0,
//           "Display SELinux security context in wide display mode", 0 },

//         { "dont-descend-regex", 0, POPT_ARG_STRING, &DontDescendRegexCSTR, 0,
//           "Don't descend into urls which match this regex in -R mode", 0 },

//         { "ea-index-path", 0, POPT_ARG_STRING, &EAIndexPath_CSTR, 0,
//           "which EA Index to use", "" },

//         { "output-precision", 0, POPT_ARG_INT, &OutputPrecision, 0,
//           "precision of numerical output", "" },

        { "summarize", 's', POPT_ARG_NONE, &Summarize, 0,
          "display only a total for each argument", "" },

        { "human-readable", 'h', POPT_ARG_NONE, &HumanReadable, 0,
          "print sizes in human readable format (e.g., 1K 234M 2G)", "" },

        { "block-size", 'h', POPT_ARG_STRING, &BlockSize_CSTR, 0,
          "Scale sizes by SIZE before printing them", "" },

        { "time", '1', POPT_ARG_STRING, &ShowXTime_CSTR, 0,
          "show time as WORD instead of modification time: atime, access, use, ctime or status", "" },

        { "mtime", 0, POPT_ARG_NONE, &ShowMTime, 0,
          "show mtime", "" },

        { "time-style", 0, POPT_ARG_STRING, &TimeStyle_CSTR, 0,
          "List timestamps in style STYLE.", "" },
        
        /*
         * ferrisls type options
         */
        { "dont-descend-regex", 0, POPT_ARG_STRING, &DontDescendRegexCSTR, 0,
          "Don't descend into urls which match this regex in -R mode", 0 },

        { "context-seperator", 0, POPT_ARG_STRING, &ContextSeperator, 0,
          "Data to be printed after the display of each context", "" },

        { "seperate-each-context-with-new-line", 0, POPT_ARG_NONE,
          &SeperateEachContextWithNewLine, 0,
          "Seperate each context with an extra newline", 0 },

        { "field-seperator", 0, POPT_ARG_STRING, &FieldSeperator, 0,
          "The seperator to use between attributes", "	" },
        
        /*
         * Other handy stuff
         */

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },

        { "debug", 0, POPT_ARG_NONE, &Debug, 0,
          "show debugging output", 0 },

        { "xml", 0, POPT_ARG_NONE, &XML, 0,
          "show output in XML format", 0 },
        

        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS

        /**
         * Expansion of strange-url://foo*
         */
        FERRIS_SHELL_GLOB_POPT_OPTIONS
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;


    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* url1 url2 ...");

    if (argc < 1) {
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }

    if( ShowVersion )
    {
        cout << "ferrisls version: $Id: ls.cpp,v 1.12 2008/04/27 21:30:11 ben Exp $\n"
             << "ferris   version: " << VERSION << nl
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2001 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    d = createDisplayAggregateData( &ls );
    
    if( DontDescendRegexCSTR )
        ls.setDontDescendRegex( DontDescendRegexCSTR );

    if( !Summarize )
    {
        ls.getEnteringContext_Sig().connect( sigc::ptr_fun( EnteringContext ));
        ls.getLeavingContext_Sig().connect( sigc::ptr_fun( LeavingContext ));
    }
    
    

//    cerr << "ShowMTime:" << ShowMTime << endl;
//    cerr << "ShowXTime_CSTR:" << ShowXTime_CSTR << endl;
    if( ShowXTime_CSTR )
    {
        string ShowXTime = ShowXTime_CSTR;
        ShowCTime = ShowXTime == "ctime" || ShowXTime == "status" || ShowXTime == "use";
        ShowATime = ShowXTime == "atime" || ShowXTime == "access";
        ShowMTime = ShowMTime || ShowXTime == "mtime" || ShowXTime == "modification";
    }
    time_format = TimeStyle_CSTR ? TimeStyle_CSTR : "";

    try
    {
        

        stringlist_t srcs;
        srcs = expandShellGlobs( srcs, optCon );
        if( srcs.empty() )
        {
            srcs.push_back(".");
        }
        stringlist_t::iterator srcsiter = srcs.begin();
        stringlist_t::iterator srcsend  = srcs.end();
        
        bool Done = false;
        for( int  First_Time = 1; !Done && srcsiter != srcsend ; ++srcsiter )
        {
//            RootName = poptGetArg(optCon);
            string RootName = *srcsiter;


            First_Time = 0;

            fh_context c = Resolve( RootName );
//            d->setRootContext( c );
//            ls.setURL( c->getURL() );
            
            if( isTrue( getStrAttr( c, "is-dir", "0" )))
                d->ShowAttributes( c );
            ls.setURL( RootName );
            ls();
            d->DetachAllSignals();
//    d->UpdateAggregateData( d->getData( m ), c );

            string earl = RootName;
            const Ferrisls_aggregate_t& data = d->getData( AGGDATA_RECURSIVE );

            if( Debug )
            {
                cout << "              count:" << data.count << endl;
                cout << "           maxdepth:" << data.maxdepth << endl;
                cout << "               size:" << data.size << endl;
                cout << "size human readable:" << Util::convertByteString(data.size) << endl;
                cout << "     size in blocks:" << data.sizeinblocks << endl;
                cout << endl;
                cout << "              files:" << data.filecount << endl;
                cout << "               dirs:" << data.dircount << endl;
            }
            else
            {
                if( Summarize )
                    presentData( data, earl );
            }
            
            cout << ContextSeperator;
            if(SeperateEachContextWithNewLine)
                cout << endl;
        }
    }
    catch( NoSuchContextClass& e )
    {
//        cerr << "Invalid context class given:" << RootContextClass << endl;
        cerr << "e:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        cerr << "ls.cpp cought:" << e.what() << endl;
        exit(1);
    }


        
poptFreeContext(optCon);

cout << flush;
return ls.hadErrors();
}
