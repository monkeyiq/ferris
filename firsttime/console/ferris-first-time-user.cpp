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

    $Id: ferris-first-time-user.cpp,v 1.6 2010/09/24 21:31:30 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <popt.h>
#include <glib.h>
#include <STLdb4/stldb4.hh>

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Runner.hh>
#include <Ferris/Enamel_priv.hh>

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace ::STLdb4;

const string PROGRAM_NAME = "ferris-first-time-user";

void setdb( fh_database db, string k, string v )
{
    db->set( k, v );
//     Dbt key( (void*)k.data(), k.length() );
//     Dbt data( (void*)v.data(), v.length() );

//     db->put( 0, &key, &data, 0 );
}
void setdb( fh_database db, string k, long v )
{
    db->set( k, tostr(v) );
//    setdb( db, k, tostr(v) );
}


string dbpath( const std::string& s )
{
    const gchar* home = g_get_home_dir();
    fh_stringstream ss;
    ss << home << "/.ferris/" << s;
    return tostr(ss);
}


struct fstabentry
{
    string dev;
    string mountPoint;
    string fstype;
    string mount_fstype;
    bool user_xattr;

    fstabentry( string dev, string mountPoint, string fstype, string mount_fstype, bool user_xattr )
        :
        dev(dev),
        mountPoint(mountPoint),
        fstype(fstype),
        mount_fstype(mount_fstype),
        user_xattr(user_xattr)
        {
        }

    bool supportsEA()
        {
            return fstype == "xfs" || user_xattr;
        }
};

bool getAllowsKernelEARegex_LargeStringsFirst( fstabentry& a, fstabentry& b )
{
    if( a.mountPoint.length() > b.mountPoint.length() )
        return true;
    if( a.mountPoint.length() < b.mountPoint.length() )
        return false;
    return a.mountPoint < b.mountPoint;
}


string getAllowsKernelEARegex()
{
    stringstream ss;
    string s;

    typedef list< fstabentry > l_t;
    l_t l;

    stringset_t ignoreFilesystemTypes;
    ignoreFilesystemTypes.insert("bind");
    ignoreFilesystemTypes.insert("swap");
//     ignoreFilesystemTypes.insert("tmpfs");
//     ignoreFilesystemTypes.insert("udf");
//     ignoreFilesystemTypes.insert("iso9660");
//     ignoreFilesystemTypes.insert("vfat");
//     ignoreFilesystemTypes.insert("smbfs");
//     ignoreFilesystemTypes.insert("nfs");
    
//    boost::regex fstab_regex("^([^#]\\S+)\\s(\\S+)\\s(\\S+)\\s(\\S+).*");
    boost::regex fstab_regex("^([^#]\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+).*");
    
    ifstream fstab("/etc/fstab");
    while( getline( fstab, s ) )
    {
        string dev = "";
        string mountPoint = "";
        string fstype = "auto";
        bool user_xattr = false;
        bool matched = false;

        boost::smatch matches;
        if(boost::regex_match(s, matches, fstab_regex ))
        {
//             cerr << "matches.size:" << matches.size() << endl;

//             for( int i=1; i<matches.size(); ++i )
//             {
//                 cerr << " match." << i << ":" << matches[i] << endl;
//             }
            
            
            if( matches.size() == 5 )
            {
                matched = true;

                dev         = matches[1];
                mountPoint  = matches[2];
                fstype      = matches[3];
                string att  = matches[4];

                if( ignoreFilesystemTypes.count( att ) )
                    continue;
                if( contains( att, "user_xattr" ) )
                    user_xattr = true;

            }
        }

        if( matched )
        {
            l.push_back( fstabentry( dev, mountPoint, fstype, "", user_xattr ) );
        }
    }

    l.sort( getAllowsKernelEARegex_LargeStringsFirst );

//     ss << "file:/+(";
//     bool v = true;
//     for( rs<l_t> r(l); r; ++r )
//     {
//         if( v ) v = false;
//         else    ss << "|";
        
//         if( r->supportsEA() )
//             ss << "(?=";
//         else
//             ss << "(?!";
//         ss << r->mountPoint << ")";
//     }
//     ss << ").*";


    
// pcregrep '^(?!((file:///tmp.*)|(file:///root.*)))' urls.txt
    ss << "^(?=file:/+(";
    bool v = true;
    for( rs<l_t> r(l); r; ++r )
    {
        if( r->supportsEA() )
        {
            if( v ) v = false;
            else    ss << "|";
            
            ss << "(";
            ss << r->mountPoint << ")";
        }
    }
    ss << ")).*";

    if( v )
        return "";
    
//     for( rs<l_t> r(l); r; ++r )
//     {
//         cerr << "mount:" << r->mountPoint << " supportsEA:" << r->supportsEA() << endl;
//     }
    
    return ss.str();
}

int main( int argc, char** argv )
{
    poptContext optCon;

    const char* ftp_proxy_address  =  0;
    long        ftp_proxy_port     = -1;
    const char* ftp_proxy_userpass =  0;
    long        ftp_use_netrc      = -1;
    long        ftp_use_proxy      = -1;
    long        ftp_use_ssl        = -1;
    long        ftp_use_ssl_version= -1;
    long        logging_all        = -1;
    long        logging_none       = -1;
    long        setup_defaults     =  0;
    const char* sqlplus_add_server =  0;
    long        vm_maxfreeatonce       = -1;
    long        vm_maxnumberinfreelist = -1;
    long        vm_autocleanup         = -1;
    long        update_allowskernelearegex = -1;
    long        Verbose = 0;

    try
    {
        struct poptOption optionsTable[] = {

            { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
              "give more information about current activity", "" },

            { "ftp-proxy-address", 0, POPT_ARG_STRING, &ftp_proxy_address, 0,
              "ftp proxy server address", "" },

            { "ftp-proxy-port", 0, POPT_ARG_INT, &ftp_proxy_port, 0,
              "ftp proxy server port", "" },

            { "ftp-proxy-userpass", 0, POPT_ARG_STRING, &ftp_proxy_userpass, 0,
              "Add user and password info for ftp proxy. Format user:pass", "" },

            { "ftp-use-netrc", 0, POPT_ARG_NONE, &ftp_use_netrc, 0,
              "Use ~/.netrc for ftp proxy server settings", "" },

            { "ftp-use-proxy", 0, POPT_ARG_NONE, &ftp_use_proxy, 0,
              "Use proxy server for ftp", "" },

            { "ftp-use-ssl", 0, POPT_ARG_NONE, &ftp_use_ssl, 0,
              "Use SSL for ftp connections", "" },

            { "ftp-use-ssl-version", 0, POPT_ARG_INT, &ftp_use_ssl_version, 0,
              "Use SSL version (?) for ftp connections", "3" },

            { "logging-all", 0, POPT_ARG_NONE, &logging_all, 0,
              "Turn on most verbose debug levels", "" },

            { "logging-none", 0, POPT_ARG_NONE, &logging_none, 0,
              "Turn on least verbose debug levels", "" },
            
            { "setup-defaults", 0, POPT_ARG_NONE, &setup_defaults, 0,
              "Setup default settings for all (new install)", "" },

#ifdef FERRIS_HAVE_SQLPLUS        
            { "sqlplus-add-server", 0, POPT_ARG_STRING, &sqlplus_add_server, 0,
              "Add new MySQL server. Format ServerAddress:UserName:Password", "" },
#endif

            { "vm-max-free-at-once", 0, POPT_ARG_INT, &vm_maxfreeatonce, 0,
              "Maximum number of context objects to reclaim at one time", "1000" },

            { "vm-max-freelist-size", 0, POPT_ARG_INT, &vm_maxnumberinfreelist, 0,
              "Maximum number of contexts in free list", "10000" },

            { "vm-autoclean", 0, POPT_ARG_INT, &vm_autocleanup, 0,
              "Invoke memory claimer automatically (1|0)", "1" },

            { "update-allows-kernel-ea-regex", 0, POPT_ARG_NONE, &update_allowskernelearegex, 0,
              "Update the regex that tells libferris which filesystems support creation of EA", "" },
            
            
            POPT_AUTOHELP
            POPT_TABLEEND
        };

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

        if (argc < 1) {
            poptPrintUsage(optCon, stderr, 0);
            exit(1);
        }

        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
        }

        {
            string dotferrispath = (string)g_get_home_dir() + "/.ferris";
            struct stat buf;
            int rc = lstat( dotferrispath.c_str(), &buf );
            if( rc == -1 )
            {
                if( errno == ENOENT )
                {
                    cerr << "~/.ferris not found, setting it up" << endl;
                    setup_defaults = true;
                }
            }
        }

        if( Verbose )
        {
            cout << "Starting up..." << endl;
            if( setup_defaults )
                cout << "Setting up current user with defaults" << endl;
        }
        
        
        /*
         * Do actions.
         */
        if( setup_defaults )
        {
            if( Verbose )
                cout << "Setting up ~/.ferris from skel files..." << endl;

            // Do it this way to avoid Runner trying to use logging and causing
            // a catch-22
            g_spawn_command_line_sync( "ferris-setup-from-skels.sh", 0, 0, 0, 0 );
            
            {
                fh_runner r = new Runner();
                r->setCommandLine( "ferris-setup-from-skels.sh" );
                r->setSpawnFlags(
                    GSpawnFlags(
                        G_SPAWN_SEARCH_PATH |
                        G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
//                         G_SPAWN_STDERR_TO_DEV_NULL |
//                         G_SPAWN_STDOUT_TO_DEV_NULL |
                        r->getSpawnFlags()));
                r->Run();
                r->getExitStatus();
            }

            if( Verbose )
                cout << "Generating menus from gnome desktop files..." << endl;
            
//             {
//                 fh_runner r = new Runner();
//                 r->setCommandLine( "ferris-generate-gnome-menu.sh" );
//                 r->setSpawnFlags(
//                     GSpawnFlags(
//                         G_SPAWN_SEARCH_PATH |
//                         G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
//                         G_SPAWN_FILE_AND_ARGV_ZERO |
// //                         G_SPAWN_STDERR_TO_DEV_NULL |
// //                         G_SPAWN_STDOUT_TO_DEV_NULL |
//                         r->getSpawnFlags()));
//                 r->Run();
//                 r->getExitStatus();
//             }
            
            logging_none = 1;
            update_allowskernelearegex = 1;
//            system("/usr/local/bin/ferris-setup-from-skels.sh");
//            exit(0);
        }

        if( Verbose )
            cout << "Tweaking up ~/.ferris db4 files..." << endl;
        
        fh_database loggingdb = new Database( dbpath( "logging.db" ) );
        fh_database securedb  = new Database( dbpath( "secure.db" )  );
        fh_database generaldb = new Database( dbpath( "general.db" ) );
        
        if( ftp_proxy_address    ) setdb( generaldb, "curl-use-proxy-name",    ftp_proxy_address );
        if( -1 != ftp_proxy_port ) setdb( generaldb, "curl-use-proxy-port",    ftp_proxy_port );
        if( ftp_proxy_userpass   ) setdb( generaldb, "curl-use-proxy-userpass",ftp_proxy_userpass );
        if( -1 != ftp_use_netrc  ) setdb( generaldb, "curl-use-netrc",         ftp_use_netrc );
        if( -1 != ftp_use_proxy  ) setdb( generaldb, "curl-use-proxy",         ftp_use_proxy );
        if( -1 != ftp_use_ssl    ) setdb( generaldb, "curl-use-ssl",           ftp_use_ssl );
        if( -1 != ftp_use_ssl_version    )
            setdb( generaldb, "curl-use-ssl-version-number", ftp_use_ssl_version );
        if( -1 != logging_none    )
        {
            typedef Ferris::Logging::Loggers_t FL;
            typedef FL::iterator FLI;
            FL& Loggers = Ferris::Logging::getLoggers();
            for( FLI iter = Loggers.begin(); iter != Loggers.end(); ++iter )
            {
                const string& k = iter->first;
                setdb( loggingdb, k, 0 );
            }
        }
        if( -1 != logging_all    )
        {
            typedef Ferris::Logging::Loggers_t FL;
            typedef FL::iterator FLI;
            FL& Loggers = Ferris::Logging::getLoggers();
            for( FLI iter = Loggers.begin(); iter != Loggers.end(); ++iter )
            {
                const string& k = iter->first;
                setdb( loggingdb, k, 0xFFFF );
            }
        }
#ifdef FERRIS_HAVE_SQLPLUS        
        if( sqlplus_add_server )
        {
            const string DBNAME = FDB_SECURE;
            string serv = "";
            string user = "";
            string pass = "";

            fh_stringstream inss;
            inss << sqlplus_add_server;
            getline( inss, serv, ":" );
            getline( inss, user, ":" );
            getline( inss, pass );
            
            if( !serv.length()  || !user.length() )
            {
                cerr << "Error must have length for both server and username." << endl;
                cerr << "offending string:" << sqlplus_add_server << endl;
            }
            
            {
                fh_stringstream ss;
                ss << "mysql-server-" << serv;
                setConfigString( DBNAME, tostr(ss), "1" );
            }
            
            setUserPass( serv, user, pass );            
        }
#endif

        if( -1 != vm_maxfreeatonce )
            setdb( generaldb, "vm-auto-cleanup-maxfreeatonce",vm_maxfreeatonce );

        if( -1 != vm_maxnumberinfreelist )
            setdb( generaldb, "vm-auto-cleanup-maxnumberinfreelist",vm_maxnumberinfreelist );

        if( -1 != vm_autocleanup )
            setdb( generaldb, "vm-auto-cleanup",vm_autocleanup );

        if( -1 != update_allowskernelearegex )
        {
            string s = getAllowsKernelEARegex();
            cerr << "kernel ea regex:" << s << endl;
            setdb( generaldb, CFG_ALLOWS_KERNEL_EA_REGEX, s );
        }
        
        if( Verbose )
            cout << "Syncing ~/.ferris db4 files..." << endl;

        loggingdb->sync();
        securedb->sync();
        generaldb->sync();

    }
    catch( dbException& e )
    {
        cerr << PROGRAM_NAME << ": cought e:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        cerr << PROGRAM_NAME << ": cought e:" << e.what() << endl;
        exit(1);
    }
    
    poptFreeContext(optCon);
    return 0;
}
