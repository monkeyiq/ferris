/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-find command line client
    Copyright (C) 2005 Ben Martin

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

    $Id: ferris-find.cpp,v 1.5 2010/09/24 21:31:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * More like locate than find, though with the full power of find.
 * The paths themselves become index filters.
 *
 * FIXME: sizef() needs to consider the postfixes that find uses.
 */

/*
 * return 0 for success
 * return 1 for generic error
 * return 2 for parse error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/Chmod.hh>
#include <Ferris/Runner.hh>
#include <Ferris/EAIndexer.hh>
#include <Ferris/EAQuery.hh>
#include <Ferris/FullTextIndexer.hh>
#include <Ferris/FullTextQuery.hh>
#include <Ferris/Trimming.hh>

#include <popt.h>
#include <unistd.h>

#define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 3

#include <boost/spirit.hpp>
#include <boost/spirit/home/classic/utility/regex.hpp>
using namespace boost::spirit;

#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
using namespace boost::lambda;

typedef scanner_list<scanner<>, phrase_scanner_t> scanners;
typedef rule< scanners > R;

using namespace std;
using namespace Ferris;
using namespace Ferris::EAIndex;

const string PROGRAM_NAME = "ferris-find";

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

string globToRegex( const std::string& s )
{
    cerr << "Warning: globToRegex() is very trivial. Special regex chars are not escaped for you!" << endl;
    stringstream idata;
    stringstream ret;
    idata << s;
    char ch;

    while( idata >> noskipws >> ch )
    {
        if( ch == '*' )
            ret << ".*";
        else if( ch == '?' )
            ret << '.';
        else
            ret << ch;
    }
    
    return ret.str();
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

const char* EAIndexPath_CSTR = 0;

fh_stringstream fss;
int SearchPathRestrictionCount = 0;
stringlist_t filter_earls;

bool action_print  = false;
bool action_print0 = false;
bool action_print_count = false;
bool action_exec = false;
bool action_exec_many = false;

unsigned long BenchQueryResolution = 0;
unsigned long BenchDocIDResolution = 0;


void add_filter_earl( const char* beg, const char* end )
{
    string str( beg, end );
    LG_FFIND_D << "add_filter_earl() str:" << str << endl;
    filter_earls.push_back( str );

    if( !SearchPathRestrictionCount )
        fss << "(&(|";
    SearchPathRestrictionCount=2;
            
    try
    {
        fh_context c = Resolve( str );
        fss << "(url=~" << c->getURL() << ")";
    }
    catch( exception& e )
    {
        fss << "(url=~" << str << ")";
//        cerr << "Can't resolve one of the paths you gave. e:" << e.what() << endl;
//        exit(1);
    }
}

void add_filter_earl_complete( const char* beg, const char* end )
{
    if( SearchPathRestrictionCount )
    {
        --SearchPathRestrictionCount;
        fss << ")";
    }
}

string tostr( const char* beg, const char* end )
{
    return string( beg, end );
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

struct NumericSpec
{
    string opcode;
    string v;
    
    NumericSpec( const std::string& s )
        :
        opcode("=="),
        v("0")
        {
            stringstream tmp;
            
            if( starts_with( s, "+" ) )
            { opcode = "<="; tmp << s.substr(1); }
            else if( starts_with( s, "-" ) )
            { opcode = ">="; tmp << s.substr(1); }
            else
                tmp << s;

            tmp >> v;
        }
};


void indexpathf( const char* beg, const char* end )
{
    string v( beg, end );
    static string cache;
    cache = v;
    EAIndexPath_CSTR = cache.c_str();
}

void wholename( const char* beg, const char* end )
{
    fss << "(url=~" << globToRegex(string( beg, end )) << ")";
}
void iwholename( const char* beg, const char* end )
{
    fss << "(url=~" << globToRegex(tolowerstr()( string( beg, end ) )) << ")";
}

void name( const char* beg, const char* end )
{
    fss << "(name=~" << globToRegex(string( beg, end )) << ")";
}
void iname( const char* beg, const char* end )
{
    fss << "(name=~" << globToRegex(tolowerstr()( string( beg, end ) )) << ")";
}

void regex( const char* beg, const char* end )
{
    fss << "(url=~" << (string( beg, end )) << ")";
}
void iregex( const char* beg, const char* end )
{
    fss << "(url=~" << (tolowerstr()( string( beg, end ) )) << ")";
}

void lname( const char* beg, const char* end )
{
    fss << "(link-target=~" << globToRegex(string( beg, end )) << ")";
}
void ilname( const char* beg, const char* end )
{
    fss << "(link-target=~" << globToRegex(tolowerstr()( string( beg, end ) )) << ")";
}

void lregex( const char* beg, const char* end )
{
    fss << "(link-target=~" << (string( beg, end )) << ")";
}
void ilregex( const char* beg, const char* end )
{
    fss << "(link-target=~" << (tolowerstr()( string( beg, end ) )) << ")";
}

void samefile( const char* beg, const char* end )
{
    string filename = string( beg, end );
    fh_context c = Resolve( filename );
    string inode = getStrAttr( c, "inode", "", true, true );
    fss << "(inode==" << inode << ")";
}

void inode( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(inode" << spec.opcode << spec.v << ")";
}

void links( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(hard-link-count" << spec.opcode << spec.v << ")";
}

time_t day_offset = 24*3600;
static time_t RoundXTime( NumericSpec& spec, long offset = 24*3600, long period = 24*3600 )
{
    long v = toType<long>( spec.v );
    time_t tt = Time::getTime() - offset - v * period;

    time_t nowt = Time::getTime();
    LG_FFIND_D << "RoundXTime() v:" << v << endl
               << " now:" << nowt << " now.disp:" << Time::toTimeString(nowt) << endl
               << "  tt:" << tt << "  tt.disp:" << Time::toTimeString(tt)
               << endl;
    return tt;
}

void daystart(  const char* beg, const char* end )
{
    time_t tt = Time::getTime();
    day_offset = tt - (tt % 24*3600);
}

void atimef( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(atime" << spec.opcode << RoundXTime( spec, day_offset ) << ")";
}
void ctimef( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(ctime" << spec.opcode << RoundXTime( spec, day_offset ) << ")";
}
void mtimef( const char* beg, const char* end )
{
    LG_FFIND_D << "mtimef:" << string( beg, end ) << endl;
    NumericSpec spec( string( beg, end ) );
    fss << "(mtime" << spec.opcode << RoundXTime( spec, day_offset ) << ")";
}


void aminf( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(atime" << spec.opcode << RoundXTime( spec, 0, 60 ) << ")";
}
void cminf( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(ctime" << spec.opcode << RoundXTime( spec, 0, 60 ) << ")";
}
void mminf( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(mtime" << spec.opcode << RoundXTime( spec, 0, 60 ) << ")";
}

void newerf( const std::string& attr, const std::string& v )
{
    string filename = v;
    fh_context c = Resolve( filename );
    fss << "(" << attr << ">=" << getStrAttr( c, attr, "0", true, true ) << ")";
}
void cnewerf( const char* beg, const char* end )
{
    newerf( "ctime", string( beg, end ) );
}
void anewerf( const char* beg, const char* end )
{
    newerf( "atime", string( beg, end ) );
}
void mnewerf( const char* beg, const char* end )
{
    newerf( "mtime", string( beg, end ) );
}

void sizef( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(size" << spec.opcode << spec.v << ")";
}

void emptyf( const char* beg, const char* end )
{
    fss << "(&(|(is-dir==1)(is-file==1))(size==0))";
}

static void TypeFunc( const std::string& attr, const std::string& v )
{
    fss << "(" << attr << "==";
    if( v == "b" )       fss << "block device";
    else if( v == "c" )  fss << "character device";
    else if( v == "d" )  fss << "directory";
    else if( v == "p" )  fss << "fifo";
    else if( v == "f" )  fss << "regular file";
    else if( v == "l" )  fss << "symbolic link";
    else if( v == "s" )  fss << "socket";
    else if( v == "D" )  fss << "door";
    else                 fss << "unknown";
    fss << ")";
}    
void typef( const char* beg, const char* end )
{
    TypeFunc( "dontfollow-filesystem-filetype", string( beg, end ) );
}
void xtypef( const char* beg, const char* end )
{
    TypeFunc( "filesystem-filetype", string( beg, end ) );
}


void uidf( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(user-owner-number" << spec.opcode << spec.v << ")";
}
void gidf( const char* beg, const char* end )
{
    NumericSpec spec( string( beg, end ) );
    fss << "(group-owner-number" << spec.opcode << spec.v << ")";
}
void userf( const char* beg, const char* end )
{
    string v( beg, end );
    if( !v.empty() && v[0] >= '0' && v[0] <= '9' )
        uidf( beg, end );
    else 
        fss << "(user-owner==" << v << ")";
}

void groupf( const char* beg, const char* end )
{
    string v( beg, end );
    if( !v.empty() && v[0] >= '0' && v[0] <= '9' )
        gidf( beg, end );
    else 
        fss << "(group-owner==" << v << ")";
}

void permf( const char* beg, const char* end )
{
    string v( beg, end );
    if( !v.empty() && v[0]=='-' || v[0]=='+' )
    {
        if( v[0] == '-' )
            fss << "(|";
        else
            fss << "(&";

        ::mode_t m = ::Ferris::Factory::MakeInitializationMode( &v[1] );
        LG_FFIND_D << "nested mode:" << oct << m << endl;

        if     ( m == S_IFLNK  )  TypeFunc( "dontfollow-filesystem-filetype", "l" );
        else if( m == S_IFSOCK )  TypeFunc( "dontfollow-filesystem-filetype", "s" );
        else if( m == S_IFBLK  )  TypeFunc( "dontfollow-filesystem-filetype", "b" );
        else if( m == S_IFDIR  )  TypeFunc( "dontfollow-filesystem-filetype", "d" );
        else if( m == S_IFCHR  )  TypeFunc( "dontfollow-filesystem-filetype", "c" );
        else if( m == S_IFIFO  )  TypeFunc( "dontfollow-filesystem-filetype", "p" );
        else if( m == S_IFREG  )  TypeFunc( "dontfollow-filesystem-filetype", "f" );

        if( m & S_IRUSR )      fss << "(user-readable==1)";
        if( m & S_IWUSR )      fss << "(user-writable==1)";
        if( m & S_ISUID )      fss << "(is-setuid==1)";
        else if( m & S_IXUSR ) fss << "(user-executable==1)";
    
        if( m & S_IRGRP )      fss << "(group-readable==1)";
        if( m & S_IWGRP )      fss << "(group-writable==1)";
        if( m & S_ISGID )      fss << "(is-setgid==1)";
        else if( m & S_IXGRP ) fss << "(group-executable==1)";
    
        if( m & S_IROTH )      fss << "(other-readable==1)";
        if( m & S_IWOTH )      fss << "(other-writable==1)";
        if( m & S_ISVTX )      fss << "(is-sticky==1)";
        else if( m & S_IXOTH ) fss << "(other-executable==1)";

        
        fss << ")";
        
//         R plus_p  = (str_p("+"));
//         R minus_p = (str_p("-"));
//         R number_p = (uint_p);
        
//         R chmod_p =
//             (
//                 plus_p[ ((_1, var( fss << "(&" ) )) ] |
//                 minus_p[ ((_1, var( fss << "(|" ) )) ]
//                 )
//             >> ( number_p )
//             ;
        
//         parse_info<> info = parse(
//             v.c_str(),
//             chmod_p,
//             space_p );

//         if (info.full)
//         {
//         }
//         else
//         {
//             fh_stringstream ss;
//             ss << "Parsing find chmod specification failed" << nl
//                << "input:" << v << nl
//                << "stopped at: \": " << info.stop << "\"" << nl
//                << "char offset:" << ( info.stop - v.c_str() ) << nl;
//             cerr << tostr(ss) << endl;
//             exit(2);
//         }
//         fss << ")";
    }
    else
    {
        ::mode_t m = ::Ferris::Factory::MakeInitializationMode( v );
        fss << "(mode==" << m << ")";
    }
}

void contentsf( const char* beg, const char* end )
{
    string v( beg, end );
    fss << "(ferris-fulltext-search==" << v << ")";
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void a_printf( const char* beg, const char* end )
{
    string v( beg, end );
    action_print = 1;
}
void a_print0f( const char* beg, const char* end )
{
    string v( beg, end );
    action_print0 = 1;
}
void a_printcountf( const char* beg, const char* end )
{
    string v( beg, end );
    action_print_count = 1;
}

string a_collect_command_string_cache = "";
void a_collect_command_string( const char* beg, const char* end )
{
    string v( beg, end );
    a_collect_command_string_cache = v;
    
}
void a_exec( const char* beg, const char* end )
{
    action_exec = 1;
}
void a_exec_many( const char* beg, const char* end )
{
    action_exec_many = 1;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void start_not_pred( const char* beg, const char* end )
{
    fss << "(!(&";
}
void end_not_pred( const char* beg, const char* end )
{
    fss << "))";
}

//
// We have to strip off the previous ffilter chunk and move
// that into the OR predicate.
//
void start_or_pred( const char* beg, const char* end )
{
    string prevchunk = "";
    string tmp = fss.str();

    int char_count = 0;
    int paren_count = 0;
    string::reverse_iterator iter = tmp.rbegin();
    while( iter != tmp.rend() )
    {
        LG_FFIND_D << "iter:" << *iter << endl;
        
        if( *iter == ')' )
            ++paren_count;
        if( *iter == '(' )
            --paren_count;
        ++iter;
        ++char_count;
        
        if( !paren_count )
        {
            prevchunk = tmp.substr( tmp.length() - char_count );
            fss.seekp( -char_count, ios_base::cur );
            break;
        }
    }
    
    LG_FFIND_D << "prevchunk:" << prevchunk << endl;
    fss << "(|" << prevchunk;
    
}
void end_or_pred( const char* beg, const char* end )
{
    fss << ")";
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void parseCommand( const std::string& cmdstr )
{
    LG_FFIND_D << "parseCommand(top) cmdstr:" << cmdstr << endl;

    R string_p      = regex_p("[^ ]+");
    R commandstring_p = regex_p("[^;+]+");
    R earl_p        = regex_p("[^-][^ ]*");
    R filter_earl_p = earl_p[ (&add_filter_earl) ];
    R wholename_p   = (str_p("-wholename") | str_p("-path"));
    R iwholename_p  = (str_p("-iwholename") | str_p("-ipath"));
    R name_p        = (str_p("-name"));
    R iname_p       = (str_p("-iname"));
    R regex_p       = (str_p("-regex"));
    R iregex_p      = (str_p("-iregex"));
    R lname_p       = (str_p("-lname"));
    R ilname_p      = (str_p("-ilname"));
    R lregex_p       = (str_p("-lregex"));   // added
    R ilregex_p      = (str_p("-ilregex"));  // added
    R samefile_p      = (str_p("-samefile"));
    R inode_p         = (str_p("-inum") | str_p("-inode"));
    R links_p         = (str_p("-links") );
    R atime_p         = (str_p("-atime") );
    R ctime_p         = (str_p("-ctime") );
    R mtime_p         = (str_p("-mtime") );
    R amin_p         = (str_p("-amin") );
    R cmin_p         = (str_p("-cmin") );
    R mmin_p         = (str_p("-mmin") );
    R daystart_p         = (str_p("-daystart") );
    R anewer_p         = (str_p("-anewer") );
    R cnewer_p         = (str_p("-cnewer") );
    R mnewer_p         = (str_p("-newer") | str_p("-mnewer") );
//    R used_p         = (str_p("-used") );
    R size_p         = (str_p("-size") );
    R empty_p        = (str_p("-empty") );
    R type_p         = (str_p("-type") );
    R xtype_p        = (str_p("-xtype") );
    R user_p         = (str_p("-user") );
    R group_p        = (str_p("-group") );
    R uid_p          = (str_p("-uid") );
    R gid_p          = (str_p("-gid") );
//     R nouser_p       = (str_p("-nouser") );
//     R nogroup_p      = (str_p("-nogroup") );
    R perm_p      = (str_p("-perm") );
    R contents_p      = (str_p("-contents") );  // added.
    R indexpath_p        = (str_p("-index-path"));


    R a_print_p       = (str_p("-print") );
    R a_print0_p      = (str_p("-print0") );
    R a_print_count_p = (str_p("-print-count") );

    R a_exec_p       = (str_p("-exec") );
    
    R name_pred_p =
            ( wholename_p >> string_p[ &wholename ] )
            |
            ( iwholename_p >> string_p[ &iwholename ] )
            |
            ( name_p >> string_p[ &name ] )
            |
            ( iname_p >> string_p[ &iname ] )
            |
            ( regex_p >> string_p[ &regex ] )
            |
            ( iregex_p >> string_p[ &iregex ] )
            |
            ( lname_p >> string_p[ &lname ] )
            |
            ( ilname_p >> string_p[ &ilname ] )
            |
            ( lregex_p >> string_p[ &lregex ] )
            |
            ( ilregex_p >> string_p[ &ilregex ] )
        ;
    
    R time_pred_p =
            daystart_p[ &daystart ]
            |
            ( atime_p >> string_p[ &atimef ] )
            |
            ( ctime_p >> string_p[ &ctimef ] )
            |
            ( mtime_p >> string_p[ &mtimef ] )
            |
            ( amin_p >> string_p[ &aminf ] )
            |
            ( cmin_p >> string_p[ &cminf ] )
            |
            ( mmin_p >> string_p[ &mminf ] )
            |
            ( anewer_p >> string_p[ &anewerf ] )
            |
            ( cnewer_p >> string_p[ &cnewerf ] )
            |
            ( mnewer_p >> string_p[ &mnewerf ] )
        ;

    R ea_pred_p =
        ( samefile_p >> string_p[ &samefile ] )
        |
        ( inode_p >> string_p[ &inode ] )
        |
        ( links_p >> string_p[ &links ] )
        |
        ( size_p >> string_p[ &sizef ] )
        |
        ( empty_p >> string_p[ &emptyf ] )
        |
        ( type_p >> string_p[ &typef ] )
        |
        ( xtype_p >> string_p[ &xtypef ] )
        |
        ( user_p >> string_p[ &userf ] )
        |
        ( group_p >> string_p[ &groupf ] )
        |
        ( uid_p >> string_p[ &uidf ] )
        |
        ( gid_p >> string_p[ &gidf ] )
        |
        ( perm_p >> string_p[ &permf ] )
        ;

    R ferris_options_p =
        ( indexpath_p >> string_p[ &indexpathf ] );
    
    R pred_p =
        +(
            name_pred_p
            |
            ferris_options_p
            |
            ea_pred_p
            |
            time_pred_p
            |
            ( contents_p >> str_p("\"") >> string_p[ &contentsf ] >> str_p("\"")  )
            |
            (((str_p("-not") | str_p("!"))[ &start_not_pred ])
             >> pred_p)[ &end_not_pred ]
            |
            (str_p("-and") | str_p("-a"))
            |
            (((str_p("-or") | str_p("-o"))[ &start_or_pred ])
             >> pred_p)[ &end_or_pred ]
            );
    R find_p = (+(filter_earl_p))[ &add_filter_earl_complete ]
        >> *( pred_p )
        >>
        *( (a_print_count_p[ &a_printcountf ])
           | (a_print0_p[ &a_print0f ])
           | (a_print_p[ &a_printf ])
           | (a_exec_p >> commandstring_p[ &a_collect_command_string ]
              >> ( str_p(";")[ &a_exec ] | str_p("+")[ &a_exec_many ] ))
            )
        ;
        
    
    
    parse_info<> info = parse(
        cmdstr.c_str(),
        find_p,
        space_p );

    if (info.full)
    {
    }
    else
    {
        fh_stringstream ss;
        ss << "Parsing find specification failed" << nl
           << "input:" << cmdstr << nl
           << "stopped at: \": " << info.stop << "\"" << nl
           << "char offset:" << ( info.stop - cmdstr.c_str() ) << nl;
        cerr << tostr(ss) << endl;
        exit(2);
    }
    
    ////////////////////////////////////////////////////////////
    //
    // Close the path filter predicate off
    //
    ////////////////////////////////////////////////////////////
    for( int i=0; i<SearchPathRestrictionCount; ++i )
        fss << ")";
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void spawn( const std::string& cmd )
{
    fh_runner r = new Runner();
    r->setCommandLine( cmd );
    r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags() | (G_SPAWN_DO_NOT_REAP_CHILD)));
    r->setConnect_ChildStdOut_To_ParentStdOut( true );
    r->setConnect_ChildStdErr_To_ParentStdErr( true );

//    cerr << "spawn() cmd:" << cmd << endl;
    r->Run();
    r->getExitStatus();
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        {
            stringstream ss;
            for( int i=1; i<argc; ++i )
            {
                ss << argv[i] << " ";
            }
            string t = ss.str();
            PostfixTrimmer trimmer;
            trimmer.push_back( " " );
            t = trimmer( t );
            parseCommand( t );
        }
        EAIndex::fh_idx idx;
        if( EAIndexPath_CSTR )
        {
            idx = EAIndex::Factory::getEAIndex( EAIndexPath_CSTR );
        }
        else
        {
            idx = EAIndex::Factory::getDefaultEAIndex();
        }

        LG_FFIND_D << "ferris-filter:" << fss.str() << endl;

        string qs = fss.str();
        fh_eaquery q = EAIndex::Factory::makeEAQuery( qs, idx );
        EAIndex::docNumSet_t docset;
        {
            Time::fh_benchmark bm = BenchQueryResolution
                ? new Time::Benchmark( "Resolving query to docids" ) : 0;
                
            q->ExecuteQueryToDocIDs( docset );
        }
            
        fh_context result = 0;
            
        if( action_print_count )
        {
            cout << "Found " << docset.size() << " matches. exiting" << endl;
            exit( 0 );
        }

        cerr << "Found " << docset.size() << " matches...." << endl;
        {
            Time::fh_benchmark bm = 0;
            if( BenchDocIDResolution )
            {
                bm = new Time::Benchmark( "Resolving docids" );
            }
            result = q->ResolveDocIDs( docset );
        }

        if( action_exec_many )
        {
            string given_cmd = a_collect_command_string_cache;
            Context::iterator ci = result->begin();

            while( ci != result->end() )
            {
                stringstream given_cmd_ss;
                given_cmd_ss << given_cmd;
                stringstream cmdss;
                char ch;
                while( given_cmd_ss >> noskipws >> ch )
                {
                    if( ch != '{' )
                        cmdss << ch;
                    else
                    {
                        given_cmd_ss >> ch;
                        break;
                    }
                }

                int len = given_cmd.length() + 2;
                for( ; ci != result->end();  )
                {
                    string earl = (*ci)->getURL();
                    int earllen = earl.length();

//                    cerr << "len:" << len << " earllen:" << earllen << endl;
                    if( len + earllen >= _POSIX_ARG_MAX )
                        break;
                    
                    len += earllen + 2;
                    cmdss << " " << earl << " ";
                    ++ci;
                }
                while( given_cmd_ss >> noskipws >> ch )
                    cmdss << ch;
                
                spawn( cmdss.str() );
            }
            return 0;
        }
        
        for( Context::iterator ci = result->begin(); ci != result->end(); ++ci )
        {
            if( action_print0 )
                cout << (*ci)->getURL() << '\0';
            else if( action_exec )
            {
                string earl = (*ci)->getURL();
                string given_cmd = a_collect_command_string_cache;
//                cerr << "given_cmd:" << given_cmd << " url:" << earl << endl;
                string cmd = Util::replace_all( given_cmd, "{}", earl );
                spawn( cmd );
            }
            else
                cout << (*ci)->getURL() << endl;
        }
            
        exit( 0 );
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


