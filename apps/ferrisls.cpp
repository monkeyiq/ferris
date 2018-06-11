/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris ls
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

    $Id: ls.cpp,v 1.16 2010/09/24 21:31:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
/*


FIXME: -a and -A are the same as there are no . and .. fake contexts created
       at the moment.



Note that this command will show you what types of filesystem your Ferris (TM)
can access.

./tests/ls/ls --root-context-class=Context  / --show-columns=name --record-seperator="   " --seperate-each-context-with-new-line --hide-headings




The following are some fileutils 'ls' to Ferrisls conversions.


ls /tmp/test
 ./tests/ls/ls --show-columns=name /tmp/test --field-seperator=" " --record-seperator=" " --seperate-each-context-with-new-line --hide-headings


ls -l /tmp/test
./tests/ls/ls --show-columns=protection-raw,block-count,user-owner-number,group-owner-number,size,atime-ctime,name /tmp/test --field-seperator="   "
 

./tests/ls/ls --show-columns=size,name,sha1 /tmp/test --field-seperator="        "  --columns-widths=8,15,40,15 --fill-char='x' --seperate-each-context-with-new-line



*/



#include <config.h>

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <Ferrisls.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/EAIndexerMetaInterface.hh>

#include <popt.h>
#include <unistd.h>


using namespace std;
using namespace Ferris;

#define DEBUG 0

const string PROGRAM_NAME = "ls";

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


string subst( string s, const string& oldv, const string& newv )
{
    string::size_type loc = s.find( oldv );
    while( loc != string::npos )
    {
        s = s.replace( loc, oldv.length(), newv );
        loc = s.find( oldv );
    }
    return s;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


#include <Attribute.hh>
//#include <Medallion.hh>

int main( int argc, const char** argv )
{
    LG_JOURNAL_D << " debug! " << endl;
    LG_JOURNAL_I << " info! " << endl;
    
    Ferrisls ls;

//     fh_etagere et = Factory::getEtagere();
// cerr << "calling et sync!" << endl;
// et->sync();
// cerr << "DONE calling et sync!" << endl;
// cerr << "------------------------------------" << endl;

// cerr << "Loading smushes" << endl;
// redlandea::fh_TreeSmushing sm = redlandea::getDefaultImplicitTreeSmushing();
// cerr << "Saving smushes" << endl;
// sm->sync();
// cerr << "syncing done!" << endl;
    
//     {
//         string earl = "~/.ego/history.db/ea-dir";
//         string rdn = "cursor-object-always-remake";

//         fh_context c = Resolve( earl );
//         fh_attribute a = c->getAttribute( rdn );
//         fh_iostream ss = a->getIOStream();
//         cerr << StreamToString(ss) << endl;
//         cerr << "ok" << endl;
//         exit(0);
//     }
    


    const char* ShowColumnsDefault= "name";
    const char* ShowColumnsCSTR  = ShowColumnsDefault;
    const char* AppendShowColumnsRegexCSTR  = 0;
    const char* ColumnsWidthsCSTR= 0;
    const char* FieldSeperator   = "	";
    const char* RecordSeperator  = "\n";
    const char* RootNameCSTR     = 0;
    const char* NoSuchAttributeIndicator = " ";
    const char* FilterStringCSTR = 0;
    const char* SortStringDefault= "name";
    const char* SortStringCSTR   = SortStringDefault;
    unsigned long HideHeadings   = 1;
    unsigned long ShowHeadings   = 0;
    const char* ContextSeperator = "";
    unsigned long SeperateEachContextWithNewLine = 0;
    const char* FillString = " ";
    unsigned long ShowContextNameInHeading = 0;
    unsigned long HideContextNameInHeading = 0;
    unsigned long ShowAllAttributes = 0;
    unsigned long ListDirectoryNodeOnly = 0;
    unsigned long NoImplicitListDirectoryNodeOnly = 0;
    unsigned long ForceReadRootDirectoryNodes = 0;
    unsigned long RecursiveList   = 0;
    unsigned long MonitorCreate   = 0;
    unsigned long MonitorDelete   = 0;
    unsigned long MonitorChange   = 0;
    unsigned long MonitorAll      = 0;
    unsigned long ShowRecommendedEA = 0;
    unsigned long ShowNameAndEANamesOnly = 0;

    /*
     * Fileutils ls like options
     */
    unsigned long SortByMTime        = 0;
    unsigned long LongList           = 0;
    unsigned long XMLList            = 0;
    unsigned long XMLListElements    = 0;
    unsigned long XMLListRAW         = 0;
    unsigned long XMLListXSLTFSDebug = 0;
    unsigned long JSONList           = 0;
    unsigned long RDFList            = 0;
    unsigned long DiredList          = 0;
    unsigned long CSVList            = 0;
    unsigned long HorizList          = 0;
    unsigned long VerticalList       = 0;
    unsigned long LongListNoGroup    = 0;
    unsigned long HideGroupInfo      = 0;
    unsigned long NoSort             = 0;
    unsigned long SortByVersion      = 0;
    unsigned long SortByExtension    = 0;
    unsigned long SortByATime        = 0;
    unsigned long SortByCTime        = 0;
    unsigned long HumanReadableSizes = 0;
    unsigned long SortByFileSize     = 0;
    unsigned long ReverseSort        = 0;
    unsigned long ShowAllFiles       = 0;
    unsigned long ShowAlmostAllFiles = 0;
    unsigned long IgnoreBackupFiles  = 0;
    unsigned long OptionMinusF       = 0;
    unsigned long ShowFullTime       = 0;
    unsigned long ShowINode          = 0;
    unsigned long ShowNumericUIDGID  = 0;
    unsigned long FullClassify       = 0;
    unsigned long Classify           = 0;
    unsigned long ClassifyDirs       = 0;
    unsigned long QuoteName          = 0;
    unsigned long ShowNonPrintableNameAsOctal = 0;
    unsigned long ShowNonPrintableNameAsQ = 0;
    unsigned long ShowNonPrintableName = 0;
    unsigned long LongListNameOnly   = 0;

    unsigned long ShowSELinuxContext = 0;
    unsigned long ShowSELinuxContextLong = 0;
    
    unsigned long ConsoleWidth       = 0;
    unsigned long HideXMLDeclaration = 0;
    
    const char*   QuoteNameSelectCSTR= 0;
    const char*   IndicatorStyleCSTR = 0;
    const char*   SortByCSTR         = 0;
    const char*   ShowTimeCSTR       = 0;
    const char*   FormatDisplayCSTR  = 0;
    const char*   DontDescendRegexCSTR = 0;
    
    unsigned long ShowVersion        = 0;

    const char*   EAIndexPath_CSTR = 0;
    
    unsigned long OutputPrecision = 0;
    
    struct poptOption optionsTable[] = {

        { "show-columns", 0, POPT_ARG_STRING, &ShowColumnsCSTR, 0,
          "Same as --show-ea",
          "size,mimetype,name" },

        { "append-ea-regex", 0, POPT_ARG_STRING, &AppendShowColumnsRegexCSTR, 0,
          "regex for attributes to also show. handy for showing a namespace.",
          "" },
        
        
        { "show-ea", 0, POPT_ARG_STRING, &ShowColumnsCSTR, 0,
          "A comma seperated list of EA to show in the order listed",
          "size,mimetype,name" },
        
        { "columns-widths", 0, POPT_ARG_STRING, &ColumnsWidthsCSTR, 0,
          "A comma seperated list of widths of each column in show-columns."
          "0 is unlimited length", "6,6,8,15,0" },
        
        { "field-seperator", 0, POPT_ARG_STRING, &FieldSeperator, 0,
          "The seperator to use between attributes", "	" },
        
        { "record-seperator", 0, POPT_ARG_STRING, &RecordSeperator, 0,
          "The seperator to use between contexts", "\\n" },
        
        { "hide-headings", 0, POPT_ARG_NONE, &HideHeadings, 0,
          "Prohibit the display of column headings", 0 },

        { "show-headings", 0, POPT_ARG_NONE, &ShowHeadings, 0,
          "Display column headings", 0 },
        
        { "show-context-name-in-heading", 0, POPT_ARG_NONE, &ShowContextNameInHeading, 0,
          "Show the context name that is being listed in the heading", 0 },

        { "hide-context-name-in-heading", 0, POPT_ARG_NONE, &HideContextNameInHeading, 0,
          "Hide the context name from heading even in -R mode", 0 },
        
        { "seperate-each-context-with-new-line", 0, POPT_ARG_NONE,
          &SeperateEachContextWithNewLine, 0,
          "Seperate each context with an extra newline", 0 },
        
        { "context-seperator", 0, POPT_ARG_STRING, &ContextSeperator, 0,
          "Data to be printed after the display of each context", "" },
        
        { "fill-char", 0, POPT_ARG_STRING, &FillString, 0,
          "Padding char to fill out small columns with", "' '" },

        { "force-read-root-dir-nodes", 0, POPT_ARG_NONE,
          &ForceReadRootDirectoryNodes, 0,
          "Always read the context given on cmd line."
          "Handy in use with -d to force EA generation", 0 },

        { "no-such-attribute-indicator", 0, POPT_ARG_STRING,
          &NoSuchAttributeIndicator, 0,
          "Item to print when EA is not available for a context", " " },

        { "ferris-filter", 0, POPT_ARG_STRING, &FilterStringCSTR, 0,
          "an LDAP filter string to apply to contexts to match",
          "(name=fred*)" },

        { "ferris-sort", 0, POPT_ARG_STRING, &SortStringCSTR, 0,
          "a sorting predicate",
          "name" },
        
        { "monitor-create", 0, POPT_ARG_NONE, &MonitorCreate, 0,
          "Monitor the given context for changes.", 0 },

        { "monitor-delete", 0, POPT_ARG_NONE, &MonitorDelete, 0,
          "Monitor the given context for changes.", 0 },

        { "monitor-change", 0, POPT_ARG_NONE, &MonitorChange, 0,
          "Monitor the given context for changes.", 0 },

        { "monitor-all", 0, POPT_ARG_NONE, &MonitorAll, 0,
          "Monitor the given context for changes on all event types.", 0 },

        { "show-recommended-ea", '0', POPT_ARG_NONE, &ShowRecommendedEA, 0,
          "Show the EA that is recommended by the context itself.", 0 },

        { "show-name-and-ea-names", '7', POPT_ARG_NONE, &ShowNameAndEANamesOnly, 0,
          "equal to --show-columns=name,ea-names", 0 },

        { "xml", '2', POPT_ARG_NONE, &XMLList, 0,
          "Show output as XML", 0 },

        { "rdf", '3', POPT_ARG_NONE, &RDFList, 0,
          "Show output as RDF/XML", 0 },

        { "xmle", '4', POPT_ARG_NONE, &XMLListElements, 0,
          "Show output as XML with full data in elements", 0 },

        { "json", '5', POPT_ARG_NONE, &JSONList, 0,
          "Show output as JSON", 0 },
        
        { "xmlraw", 0, POPT_ARG_NONE, &XMLListRAW, 0,
          "Show output as XML using specialized adhoc serialization", 0 },

        { "xml-xsltfs-debug", 0, POPT_ARG_NONE, &XMLListXSLTFSDebug, 0,
          "Show output as XML in the style that xsltfs:// would expect an input filesystem", 0 },

        { "hide-xml-declaration", 0, POPT_ARG_NONE, &HideXMLDeclaration, 0,
          "do not show initial <?xml...?> declaration", 0 },
        
        
        /*
         * Fileutils ls like options
         */
        { 0, '1', POPT_ARG_NONE, &LongListNameOnly, 0,
          "list one file per line", 0 },

        { "all", 'a', POPT_ARG_NONE, &ShowAllFiles, 0,
          "do not hide entries starting with .", 0 },

        { "almost-all", 'A', POPT_ARG_NONE, &ShowAlmostAllFiles, 0,
          "do not list implied . and ..", 0 },

        { "escape", 'b', POPT_ARG_NONE, &ShowNonPrintableNameAsOctal, 0,
          "print octal escapes for nongraphic characters", 0 },

        { "ignore-backups", 'B', POPT_ARG_NONE, &IgnoreBackupFiles, 0,
          "do not list implied entries ending with ~", 0 },

        { 0, 'C', POPT_ARG_NONE, &VerticalList, 0,
          "list entries by columns", 0 },
        
        { "sort-by-ctime", 'c', POPT_ARG_NONE, &SortByCTime, 0,
          "with -lt: sort by, and show, ctime (time of last "
          "modification of file status information). "
          "with -l: show ctime and sort by name"
          "otherwise: sort by ctime", 0 },

        { "directory", 'd', POPT_ARG_NONE,
          &ListDirectoryNodeOnly, 0,
          "list directory entries instead of contents", 0 },

        { "no-auto-directory-mode", 0, POPT_ARG_NONE,
          &NoImplicitListDirectoryNodeOnly, 0,
          "Don't imply -d for files", 0 },
        
        { "dired", 'D', POPT_ARG_NONE, &DiredList, 0,
          "generate output designed for Emacs' dired mode", 0 },
        
        { 0, 'f', POPT_ARG_NONE, &OptionMinusF, 0,
          "do not sort, enable -aU, disable -lst", 0 },

        { "classify", 'F', POPT_ARG_NONE, &FullClassify, 0,
          "append indicator (one of */=@|) to entries", 0 },

        { "classify-directories", 0, POPT_ARG_NONE, &ClassifyDirs, 0,
          "append '/' indicator to directories", 0 },
        
        { "format", 0, POPT_ARG_NONE, &FormatDisplayCSTR, 0,
          "across -x, commas -m, horizontal -x, long -l, "
          "single-column -1, verbose -l, vertical -C, xml --xml", 0 },
        
        { "full-time", 0, POPT_ARG_NONE, &ShowFullTime, 0,
          "list both full date and full time", 0 },

        { "no-group", 'G', POPT_ARG_NONE, &HideGroupInfo, 0,
          "inhibit display of group information", 0 },
        
        { "human-readable", 'h', POPT_ARG_NONE, &HumanReadableSizes, 0,
          "print sizes in human readable format (e.g., 1K 234M 2G)", 0 },

        { "indicator-style", 0, POPT_ARG_STRING, &IndicatorStyleCSTR, 0,
          "append indicator with style STRING to entry names: "
          "none (default), classify (-F), file-type (-p)", 0 },
        
        { "inode", 'i', POPT_ARG_NONE, &ShowINode, 0,
          "print inode number of each file", 0 },

        { "long-list", 'l', POPT_ARG_NONE, &LongList, 0,
          "use a long listing format", 0 },

        { 0, 'm', POPT_ARG_NONE, &CSVList, 0,
          "fill width with a comma separated list of entries", 0 },
        
        { "numeric-uid-gid", 'n', POPT_ARG_NONE, &ShowNumericUIDGID, 0,
          "list numeric UIDs and GIDs instead of names", 0 },

        { "literal", 'N', POPT_ARG_NONE, &ShowNonPrintableName, 0,
          "print raw entry names (don't treat e.g. control "
          "characters specially)" , 0 },
        
        { 0, 'o', POPT_ARG_NONE, &LongListNoGroup, 0,
          "use long listing format without group info", 0 },

        { "file-type", 'p', POPT_ARG_NONE, &Classify, 0,
          "append indicator (one of /=@|) to entries", 0 },

        { "hide-control-chars", 'q', POPT_ARG_NONE, &ShowNonPrintableNameAsQ, 0,
          "print ? instead of non graphic characters", 0 },

        { "show-control-chars", 0, POPT_ARG_NONE, &ShowNonPrintableName, 0,
          "show non graphic characters as-is (default "
          "unless program is `ls' and output is a terminal)", 0 },

        { "quote-name", 'Q', POPT_ARG_NONE, &QuoteName, 0,
          "enclose entry names in double quotes", 0 },

        { "quoting-style", 0, POPT_ARG_STRING, &QuoteNameSelectCSTR, 0,
          "use quoting style STRING for entry names: "
          "literal, locale, shell, shell-always, c, escape", 0 },

        { "recursive", 'R', POPT_ARG_NONE, &RecursiveList, 0,
          "list directorys recursively", 0 },

        { "reverse", 'r', POPT_ARG_NONE, &ReverseSort, 0,
          "reverse order while sorting", 0 },

        { 0, 'S', POPT_ARG_NONE, &SortByFileSize, 0,
          "sort by file size", 0 },

        { "sort", 0, POPT_ARG_STRING, &SortByCSTR, 0,
          "extension -X, none -U, size -S, time -t, version -v "
          "status -c, time -t, atime -u, access -u, use -u", 0 },

        { "time", 0, POPT_ARG_STRING, &ShowTimeCSTR, 0,
          "show time as STRING instead of modification time: atime, access, use,"
          "ctime or status; use specified time as sort key if --sort=time", 0 },
        
        { "sort-by-time", 't', POPT_ARG_NONE, &SortByMTime, 0,
          "sort by modification time unless -c or -u are also used", 0 },

        { "no-sort", 'U', POPT_ARG_NONE, &NoSort, 0,
          "do not sort; list entries in hashed order", 0 },

        { "sort-by-atime", 'u', POPT_ARG_NONE, &SortByATime, 0,
          "with -lt: sort by, and show, access time. "
          "with -l: show access time and sort by name. "
          "otherwise: sort by access time", 0 },
        
        { "sort-by-version", 'v', POPT_ARG_NONE, &SortByVersion, 0,
          "sort by version", 0 },

        { "width", 'w', POPT_ARG_INT, &ConsoleWidth, 0,
          "assume screen width instead of current value", 0 },
        
        { "sort-by-extension", 'X', POPT_ARG_NONE, &SortByExtension, 0,
          "sort alphabetically by entry extension", 0 },

        { 0, 'x', POPT_ARG_NONE, &HorizList, 0,
          "list entries by lines instead of by columns", 0 },

        { "context", 'Z', POPT_ARG_NONE, &ShowSELinuxContext, 0,
          "Display SELinux security context so it fits on most displays.", 0 },

        { "lcontext", 0, POPT_ARG_NONE, &ShowSELinuxContextLong, 0,
          "Display SELinux security context in wide display mode", 0 },

        { "dont-descend-regex", 0, POPT_ARG_STRING, &DontDescendRegexCSTR, 0,
          "Don't descend into urls which match this regex in -R mode", 0 },

        { "ea-index-path", 0, POPT_ARG_STRING, &EAIndexPath_CSTR, 0,
          "which EA Index to use", "" },

//         { "output-precision", 0, POPT_ARG_INT, &OutputPrecision, 0,
//           "precision of numerical output", "" },
        
        /*
         * Other handy stuff
         */

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        

        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS

        /**
         * Expansion of strange-url://foo*
         */
        FERRIS_SHELL_GLOB_POPT_OPTIONS
        
//         { "show-all-attributes", 0, POPT_ARG_NONE,
//           &ShowAllAttributes, 0,
//           "Show every available bit of metadata about each object in context",
//           0 },
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
        cout << "ferrisls version: $Id: ls.cpp,v 1.16 2010/09/24 21:31:18 ben Exp $\n"
             << "ferris   version: " << VERSION << nl
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2001 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    if( EAIndexPath_CSTR )
    {
        ::Ferris::EAIndex::Factory::setDefaultEAIndexPath( EAIndexPath_CSTR );
    }
    
    /* Having these both on gives side effects */
    if( ShowRecommendedEA )
    {
        LongList = 0;
    }
    if( ShowNameAndEANamesOnly )
    {
        ShowColumnsCSTR = "name,ea-names";
        LongList = 1;
    }
    
    if( ShowHeadings )
    {
        HideHeadings = !ShowHeadings;
    }
    
    if( RecursiveList )
    {
        ShowContextNameInHeading = true;
        if( HideContextNameInHeading )
            ShowContextNameInHeading = false;
    }

    if( FormatDisplayCSTR )
    {
        string s = FormatDisplayCSTR;

        if( s == "across" || s == "horizontal" )
        {
            HorizList = 1;
        }
        else if( s == "commas" )
        {
            CSVList = 1;
        }
        else if( s == "long" || s == "verbose" )
        {
            LongList = 1;
        }
        else if( s == "single-column" )
        {
            LongListNameOnly = 1;
        }
        else if( s == "vertical" )
        {
            VerticalList = 1;
        }
        else if( s == "xml" )
        {
            XMLList = 1;
        }
        else if( s == "json" )
        {
            JSONList = 1;
        }
        else if( s == "xmle" )
        {
            XMLListElements = 1;
        }
        else if( s == "xmlraw" )
        {
            XMLListRAW = 1;
        }
        else if( s == "xml-xsltfs-debug" )
        {
            XMLListXSLTFSDebug = 1;
        }
        else if( s == "rdf" )
        {
            RDFList = 1;
        }
    }
    
    
    if( IndicatorStyleCSTR )
    {
        string IndicatorStyle = IndicatorStyleCSTR;
        
        if( IndicatorStyle == "classify" )
        {
            FullClassify = 1;
        }
        else if( IndicatorStyle == "file-type" )
        {
            Classify = 1;
        }
        else if( IndicatorStyle == "directories" )
        {
           ClassifyDirs = 1;
        }
    }
    
    if( SortByCSTR )
    {
        string s = SortByCSTR;

        if( s == "extension" )     { SortByExtension = 1; }
        else if( s == "none" )     { NoSort = 1; }
        else if( s == "size" )     { SortByFileSize = 1; }
        else if( s == "time" )     { SortByMTime = 1; }
        else if( s == "version" )  { SortByVersion = 1; }
        else if( s == "status" )   { SortByCTime = 1; }
        else if( s == "time" )     { SortByMTime = 1; }
        else if( s == "atime" )    { SortByATime = 1; }
        else if( s == "use" )      { SortByATime = 1; }
    }
    
    /*
     * NOTE ABOUT SORTING
     * 
     * `-u' alone means sort by atime, -lu means show atime and sort by name,
     * -lut means show atime and sort by atime.
     */
    if( ShowTimeCSTR )
    {
        string s = ShowTimeCSTR;

        if( s == "atime" || s == "access" || s == "use" )
        {
            SortByATime = 1;
        }
        else if( s == "ctime" || s == "status" )
        {
            SortByCTime = 1;
        }
    }
    
    

    
    /* FIXME disable '-s' when that option is added */
    if( OptionMinusF )
    {
        ShowAllFiles = 1;
        NoSort       = 1;

        LongList     = 0;
        SortByMTime  = 0;
    }
    
    
    try
    {
        bool ShowColumnsNotSetByUser   = ShowColumnsCSTR==ShowColumnsDefault
            && AppendShowColumnsRegexCSTR == 0;
        bool SortStringNotSetByUser    = SortStringCSTR==SortStringDefault;
        bool FilterStringNotSetByUser  = FilterStringCSTR==0;
        bool ColumnsWidthsNotSetByUser = ColumnsWidthsCSTR==0;

        if( !ShowColumnsNotSetByUser )
        {
            LongList          = 1;
            ShowRecommendedEA = 0;
        }
        
        string ShowColumns   = ShowColumnsCSTR;
        string SortString    = SortStringCSTR;
        string FilterString  = FilterStringCSTR ? FilterStringCSTR : "";
        string ColumnsWidths = ColumnsWidthsCSTR ? ColumnsWidthsCSTR : "";

        /*
         * If they want a long list and have not overriden the column names
         */
        if( ShowColumnsNotSetByUser )
        {
            /* Implicitly full time enables long list mode */
            if( ShowFullTime )
            {
                LongList = 1;
            }
            
            if( LongList )
            {
                ShowColumns = 
                    "protection-ls,user-owner-name,"
                    "group-owner-name,size,mtime-display,name";

                if( ColumnsWidthsNotSetByUser )
                {
                    ColumnsWidths  = "10,10,10,7,12,0";
                    FieldSeperator = " ";
                }
            }
            else if( LongListNoGroup )
            {
                ShowColumns = 
                    "protection-ls,user-owner-name,"
                    "size,mtime-display,name";
                
                if( ColumnsWidthsNotSetByUser )
                {
                    ColumnsWidths  = "10,10,7,12,0";
                    FieldSeperator = " ";
                }
            }
            else if( LongListNameOnly )
            {
                ShowColumns    = "name";
                ColumnsWidths  = "";
                FieldSeperator = "";
            }
            else if( XMLList || XMLListElements || XMLListRAW || XMLListXSLTFSDebug || RDFList )
            {
                ShowColumns = "recommended-ea";
            }
            if( ShowSELinuxContext )
            {
                ShowColumns = "protection-ls,user-owner-name,group-owner-name,selinux-identity,selinux-type,name";
                LongList = 1;
            }
            else if( ShowSELinuxContextLong )
            {
                ShowColumns = "protection-ls,selinux-identity,selinux-type,user-owner-name,group-owner-name,size,mtime-display,name";
                LongList = 1;
            }
            
            if( HumanReadableSizes )
            {
                ShowColumns = subst( ShowColumns, "size,", "size-human-readable," );
            }
        }

        /*
         * Only allow override of sorting string if the user has not set it explicitly.
         *
         * SEE ALSO: NOTE ABOUT SORTING (comment above)
         */
        if( SortStringNotSetByUser )
        {
            /*
             * Optionally show the ctime/atime given the right option combination.
             */
            if( SortByATime && LongList && ShowColumnsNotSetByUser )
            {
                ShowColumns = subst( ShowColumns, "mtime-display", "atime-display" );
            }
            else if( SortByCTime && LongList && ShowColumnsNotSetByUser )
            {
                ShowColumns = subst( ShowColumns, "mtime-display", "ctime-display" );
            }

            
            /*
             * Sort by time
             */
            if( !LongList || (LongList && SortByMTime) )
            {
                /*
                 * Check which time to sort by
                 */
                if( SortByATime )
                {
                    SortString = ":!#:atime";
                }
                else if( SortByCTime )
                {
                    SortString = ":!#:ctime";
                }
                else if( SortByMTime )
                {
                    SortString = ":!#:mtime";
                }
            }
        }
        
        /*
         * Sorting by other properties.
         */
        if( SortStringNotSetByUser )
        {
            if( SortByExtension )
            {
                SortString = "name-extension";
            }
            else if( SortByVersion )
            {
                SortString = ":VER:name";
            }
            else if( SortByFileSize )
            {
                SortString = ":!#:size";
            }
            
        }


        if( ReverseSort && SortString.length() )
        {
            SortString = ::Ferris::Factory::ReverseSortStringOrder( SortString );
        }
        

        if( FilterStringNotSetByUser )
        {
            FilterString = "(!(name=~^\\..*))";
            
            if( ShowAllFiles )
            {
                FilterString = "";
            }
            else if( ShowAlmostAllFiles )
            {
                FilterString = "";
//                FilterString = "(!(|(name=.)(name=..)))";
            }
            else if( IgnoreBackupFiles )
            {
                FilterString = "(!(name=~^.*~$))";
            }
        }

        

        if( ShowFullTime )
        {
            ShowColumns = subst( ShowColumns, "atime-display,", "atime-ctime," );
            ShowColumns = subst( ShowColumns, "ctime-display,", "ctime-ctime," );
            ShowColumns = subst( ShowColumns, "mtime-display,", "mtime-ctime," );
        }
        if( ShowINode )
        {
            string tmp    = "inode,";
            tmp          += ShowColumns;
            ShowColumns   = tmp;

            tmp           = "7,";
            tmp          += ColumnsWidths;
            ColumnsWidths = tmp;
        }
        if( HideGroupInfo )
        {
            ShowColumns = subst( ShowColumns, "group-owner-name,", "" );
            ShowColumns = subst( ShowColumns, "group-owner-number,", "" );
        }
        if( ShowNumericUIDGID )
        {
            ShowColumns = subst( ShowColumns, "group-owner-name,", "group-owner-number," );
            ShowColumns = subst( ShowColumns, "user-owner-name,",  "user-owner-number," );
        }
        
        
//        ls.setContextClass( RootContextClass );
    
        ls.setListDirectoryNodeOnly( ListDirectoryNodeOnly );
        ls.setForceReadRootDirectoryNodes( ForceReadRootDirectoryNodes );
        ls.setRecursiveList( RecursiveList );
        if( DontDescendRegexCSTR )
            ls.setDontDescendRegex( DontDescendRegexCSTR );
        
//         Ferrisls_long_display d = new Ferrisls_long_display();
//         fh_lsdisplay d_cleaner = d;


        fh_lsdisplay d;

        if( CSVList )
        {
            fh_lsdisplay_csv ld = new Ferrisls_csv_display();
            d = ld;

            if( ConsoleWidth )
            {
                ld->setConsoleWidth( ConsoleWidth );
            }
            
            FieldSeperator   = ", ";
            RecordSeperator  = "";
        }
        else if( JSONList )
        {
            fh_lsdisplay_json ld = new Ferrisls_json_display();
            d = ld;

            ld->setColumns( ShowColumns );
            ColumnsWidths    = "";
            RecordSeperator  = "";
            FieldSeperator   = "";

            if( AppendShowColumnsRegexCSTR )
            {
                string AppendShowColumnsRegex = AppendShowColumnsRegexCSTR;
                ld->appendShowColumnsRegex( AppendShowColumnsRegex );
            }
            if( OutputPrecision )
                ld->setOutputPrecision( OutputPrecision );
        }
        else if( XMLList )
        {
            fh_lsdisplay_xml ld = new Ferrisls_xml_display();
            d = ld;

            ld->setHideXMLDeclaration( HideXMLDeclaration );
            ld->setColumns( ShowColumns );
            ColumnsWidths    = "";
            RecordSeperator  = "";
            FieldSeperator   = "";

            if( AppendShowColumnsRegexCSTR )
            {
                string AppendShowColumnsRegex = AppendShowColumnsRegexCSTR;
                ld->appendShowColumnsRegex( AppendShowColumnsRegex );
            }
            if( OutputPrecision )
                ld->setOutputPrecision( OutputPrecision );
        }
        else if( XMLListElements )
        {
            fh_lsdisplay_xmle ld = new Ferrisls_xmle_display();
            d = ld;

            ld->setHideXMLDeclaration( HideXMLDeclaration );
            ld->setColumns( ShowColumns );
            ColumnsWidths    = "";
            RecordSeperator  = "";
            FieldSeperator   = "";

            if( AppendShowColumnsRegexCSTR )
            {
                string AppendShowColumnsRegex = AppendShowColumnsRegexCSTR;
                ld->appendShowColumnsRegex( AppendShowColumnsRegex );
            }
            if( OutputPrecision )
                ld->setOutputPrecision( OutputPrecision );
        }
        else if( XMLListRAW )
        {
            fh_lsdisplay_xmlraw ld = new Ferrisls_xmlraw_display();
            d = ld;
            
            ld->setColumns( ShowColumns );
            ColumnsWidths    = "";
            RecordSeperator  = "";
            FieldSeperator   = "";

            if( AppendShowColumnsRegexCSTR )
            {
                string AppendShowColumnsRegex = AppendShowColumnsRegexCSTR;
                ld->appendShowColumnsRegex( AppendShowColumnsRegex );
            }
        }
        else if( XMLListXSLTFSDebug )
        {
            fh_lsdisplay_xml_xsltfs_debug ld = new Ferrisls_xml_xsltfs_debug_display();
            d = ld;
            
            ld->setColumns( ShowColumns );
            ColumnsWidths    = "";
            RecordSeperator  = "";
            FieldSeperator   = "";

            if( AppendShowColumnsRegexCSTR )
            {
                string AppendShowColumnsRegex = AppendShowColumnsRegexCSTR;
                ld->appendShowColumnsRegex( AppendShowColumnsRegex );
            }
        }
        else if( RDFList )
        {
            fh_lsdisplay_rdf ld = new Ferrisls_rdf_display();
            d = ld;
            
            ld->setColumns( ShowColumns );
            ColumnsWidths    = "";
            RecordSeperator  = "";
            FieldSeperator   = "";

            if( AppendShowColumnsRegexCSTR )
            {
                string AppendShowColumnsRegex = AppendShowColumnsRegexCSTR;
                ld->appendShowColumnsRegex( AppendShowColumnsRegex );
            }
        }
        else if( HorizList )
        {
            fh_lsdisplay_horiz ld = new Ferrisls_horiz_display();
            d = ld;
            

            if( ConsoleWidth )
            {
                ld->setConsoleWidth( ConsoleWidth );
            }
            RecordSeperator  = "";
            FieldSeperator   = "  ";
        }
        else if( LongList || LongListNoGroup || LongListNameOnly
                 || DiredList || ShowRecommendedEA )
        {
            fh_lsdisplay_long ld;

            if( DiredList )
            {
                ld = new Ferrisls_dired_display();
            }
            else
            {
                ld = new Ferrisls_long_display();
            }
            d = ld;

            if( ColumnsWidths.length() && !ld->setColumnWidths( ColumnsWidths ))
            {
                usage(optCon, 1, "Column widths are wrong", ".e.g. 10,12,12,0");
                exit(1);
            }

            if( ShowRecommendedEA )
            {
                ShowColumns = "recommended-ea";
            }
            
            ld->setColumns( ShowColumns );
            ld->setShowAllAttributes( ShowAllAttributes );

            if( AppendShowColumnsRegexCSTR )
            {
                string AppendShowColumnsRegex = AppendShowColumnsRegexCSTR;
                ld->appendShowColumnsRegex( AppendShowColumnsRegex );
            }
            if( OutputPrecision )
                ld->setOutputPrecision( OutputPrecision );
        }
        else if( VerticalList || !isBound(d) )
        {
            fh_lsdisplay_vert ld = new Ferrisls_vert_display();
            d = ld;

            if( ConsoleWidth )
            {
                ld->setConsoleWidth( ConsoleWidth );
            }
            RecordSeperator  = "";
            FieldSeperator   = "  ";
        }

        if( MonitorAll )
        {
            MonitorCreate = MonitorDelete = MonitorChange = true;
        }
        d->setMonitorCreate( MonitorCreate );
        d->setMonitorDelete( MonitorDelete );
        d->setMonitorChanged( MonitorChange );

        d->setFillChar( FillString[0] );
        d->setShowHeadings( !HideHeadings );
        d->setShowContextNameInHeading( ShowContextNameInHeading );
        d->setFieldSeperator( FieldSeperator );
        d->setRecordSeperator( RecordSeperator );
        d->setNoSuchAttributeIndicator( NoSuchAttributeIndicator );

        /*
         * Filename quoting
         */ 
        if( QuoteNameSelectCSTR )
        {
            string s = QuoteNameSelectCSTR;

            if( s == "c" )
            {
                d->setQuoteStyle( StringQuote::C_QUOTING );
            }
            else if( s == "literal" )
            {
                d->setQuoteStyle( StringQuote::LITERAL_QUOTING );
            }
            else if( s == "shell" )
            {
                d->setQuoteStyle( StringQuote::SHELL_QUOTING );
            }
            else if( s == "shell-always" )
            {
                d->setQuoteStyle( StringQuote::SHELL_ALWAYS_QUOTING );
            }
            else if( s == "escape" )
            {
                d->setQuoteStyle( StringQuote::ESCAPE_QUOTING );
            }
        }
        else if( QuoteName )
        {
            d->setQuoteStyle( StringQuote::C_QUOTING );
        }
        else if( ShowNonPrintableNameAsOctal )
        {
            d->setQuoteStyle( StringQuote::ESCAPE_QUOTING );
        }
        else if( ShowNonPrintableNameAsQ )
        {
            d->setQuoteStyle( StringQuote::ESCAPE_QUOTING );
        }
        else if( ShowNonPrintableName )
        {
            d->setQuoteStyle( StringQuote::LITERAL_QUOTING );
        }

        if( FullClassify )
        {
            d->setNameClassification(
                Ferrisls_display::CLASSIFY_EXE     |
                Ferrisls_display::CLASSIFY_DIR     |
                Ferrisls_display::CLASSIFY_SOCKET  |
                Ferrisls_display::CLASSIFY_FIFO    |
                Ferrisls_display::CLASSIFY_SYMLINK );
        }
        else if( Classify )
        {
            d->setNameClassification(
                Ferrisls_display::CLASSIFY_DIR     |
                Ferrisls_display::CLASSIFY_SOCKET  |
                Ferrisls_display::CLASSIFY_FIFO    |
                Ferrisls_display::CLASSIFY_SYMLINK );
        }
        if( ClassifyDirs )
        {
           d->setNameClassification(
              d->getNameClassification() |
              Ferrisls_display::CLASSIFY_DIR );
        }
        
        
        ls.setDisplay( d );

        
//        cout << "NoSort:" << NoSort << " SortString:" << SortString << endl;
        if( !NoSort )
        {
            ls.setSortString( SortString );
        }
        
        if( FilterString.length() )
        {
            ls.setFilterString( FilterString );
        }

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


            if( NoImplicitListDirectoryNodeOnly )
            {
                ls.setImplyListDirectoryNodeOnly( !NoImplicitListDirectoryNodeOnly );
            }

//             /*
//              * If there are no dirs specified we use "." and are done after this dir.
//              */
//             if( !RootName && First_Time )
//             {
//                 Done      = true;
//                 RootName  = ".";
//             }
//             else if (c < -1 && First_Time )
//             {
//                 /* an error occurred during option processing */
//                 fprintf(stderr, "%s: %s\n", 
//                         poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
//                         poptStrerror(c));
//                 return 1;
//             }
//             else if( !RootName )
//             {
//                 break;
//             }
        

//        cout << "About to read dir:" << RootName << endl;
        
        
            First_Time = 0;
//        ls.setRoot( RootName );

            // imply -d for directories.
            try
            {
                static const fh_rex rex = toregexh( "^branch(es|fs-[a-z]+):" );
                
                if( regex_search( RootName, rex, boost::match_any ) )
                {
                    ls.setImplyListDirectoryNodeOnly( false );
                }
                else
                {
                    if( !NoImplicitListDirectoryNodeOnly )
                    {
                        fh_context c = Resolve( RootName );
                        if( !ListDirectoryNodeOnly
                            && !isTrue( getStrAttr( c, "is-dir-try-automounting", "1" ) ) )
                        {
                            ListDirectoryNodeOnly = true;
                            ls.setListDirectoryNodeOnly( ListDirectoryNodeOnly );
                        }
                    }
                }
            }
            catch( exception& e )
            {}
            
            
            ls.setURL( RootName );
            ls();

//             // FIXME!
//             cerr << "entering main loop" << endl;
//             Main::mainLoop();

            d->DetachAllSignals();
            

//         CacheManager* c =  CacheManager::getCacheManager();
//         cerr << "==========================================================" << endl;
//         cerr << "=================== Dump and Clean =======================" << endl;
//         cerr << "==========================================================" << endl;
//         c->dumpFreeListTo( cerr );
//         c->cleanUp();
//         c->dumpFreeListTo( cerr );
//         cerr << "==========================================================" << endl;
//         cerr << "==================== ls again ============================" << endl;
//         cerr << "==========================================================" << endl;
//         ls();
//         cerr << "==========================================================" << endl;
//         cerr << "=================== Dump and Clean =======================" << endl;
//         cerr << "==========================================================" << endl;
//         c->dumpFreeListTo( cerr );
//         c->cleanUp();
//         c->dumpFreeListTo( cerr );
//         cerr << "==========================================================" << endl;

        
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

#ifdef FERRIS_DEBUG_LS_VM
    {
        CacheManager* cc = getCacheManager();
        DEBUG_dumpcl( "about to cleanup sweep 1" );
        cc->cleanUp( true );
        DEBUG_dumpcl( "about to cleanup sweep 2" );
        cc->cleanUp( true );
        DEBUG_dumpcl("exiting");
    }
#endif
    
// #ifdef FERRIS_DEBUG_VM    
//     CacheManager* cc = getCacheManager();
//     DEBUG_dumpcl( "about to cleanup sweep 1" );
//     cc->cleanUp( true );
//     DEBUG_dumpcl( "about to cleanup sweep 2" );
//     cc->cleanUp( true );
//     DEBUG_dumpcl( "about to cleanup sweep 3" );
//     cc->cleanUp( true );
//     DEBUG_dumpcl("exiting");
// #endif

    cout << flush;
    return ls.hadErrors();
}
