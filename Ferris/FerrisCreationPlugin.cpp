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

    $Id: FerrisCreationPlugin.cpp,v 1.6 2010/09/24 21:30:35 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisCreationPlugin.hh>
#include <config.h>
#include <Functor.h>

using namespace std;

namespace Ferris
{
    mode_t getModeFromMetaData( fh_context md )
    {
        LG_NATIVE_D << "getModeFromMetaData(T)" << endl;
//         cerr << "getModeFromMetaData TOP" << endl;
//         BackTrace();
        
        string modestr = getStrSubCtx( md, "mode", "-1" );
        mode_t mode    = 0;
        LG_NATIVE_D << "getModeFromMetaData(1)" << endl;

//        cerr << "getModeFromMetaData() modestr:" << modestr << endl;
        
        if( modestr == "-1" )
        {
            LG_NATIVE_D << "getModeFromMetaData(A)" << endl;
            if( !md->isSubContextBound( "user-readable" )
                && !md->isSubContextBound( "user-writable" )
                && !md->isSubContextBound( "user-executable" )
                && !md->isSubContextBound( "group-readable" )
                && !md->isSubContextBound( "group-writable" )
                && !md->isSubContextBound( "group-executable" )
                && !md->isSubContextBound( "other-readable" )
                && !md->isSubContextBound( "other-writable" )
                && !md->isSubContextBound( "other-executable" ) )
            {
                string rdn = md->getDirName();
                bool isDirectory = (rdn=="dir");
//                 cerr << "getModeFromMetaData() USING DEFAULT u=rwx"
//                      << " isDir:" << isDirectory
//                      << endl;
                if( isDirectory )
                    mode = S_IRUSR | S_IWUSR | S_IXUSR;
                else
                    mode = S_IRUSR | S_IWUSR;
            }
            else
            {
                mode = 0
                    | (toint(getStrSubCtx( md, "user-readable", "1" )) ? S_IRUSR : 0)
                    | (toint(getStrSubCtx( md, "user-writable", "1" )) ? S_IWUSR : 0)
                    | (toint(getStrSubCtx( md, "user-executable", "0" )) ? S_IXUSR : 0)
                    | (toint(getStrSubCtx( md, "group-readable", "0" )) ? S_IRGRP : 0)
                    | (toint(getStrSubCtx( md, "group-writable", "0" )) ? S_IWGRP : 0)
                    | (toint(getStrSubCtx( md, "group-executable", "0" )) ? S_IXGRP : 0)
                    | (toint(getStrSubCtx( md, "other-readable", "0" )) ? S_IROTH : 0)
                    | (toint(getStrSubCtx( md, "other-writable", "0" )) ? S_IWOTH : 0)
                    | (toint(getStrSubCtx( md, "other-executable", "0" )) ? S_IXOTH : 0)
                    | 0;
            }
        }
        else
        {
//            mode = toType<mode_t>( modestr );

            LG_NATIVE_D << "getModeFromMetaData(B) modestr:" << modestr << endl;
            
            mode = Factory::MakeInitializationMode( modestr );
            LG_NATIVE_D << "getModeFromMetaData(C)" << endl;
        }

        LG_NATIVE_D << "getModeFromMetaData() modestr:" << modestr
                    << " mode:" << mode
                    << endl;
        
        return mode;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    struct FERRISEXP_DLLLOCAL CreationModuleData
    {
        string libname;
        string xsd;
        string simpleTypes;

        CreationModuleData(
            string libname,
            string xsd,
            string simpleTypes )
            :
            libname( libname ),
            xsd( xsd ),
            simpleTypes( simpleTypes )
            {
            }
    };
    
    
    typedef map< string, CreationModuleData > pmap_t;
    static pmap_t& getNativeMap()
    {
        static pmap_t o;
        return o;
    }
    static pmap_t& getNonNativeMap()
    {
        static pmap_t o;
        return o;
    }

    static pmap_t& getMap( bool requiresNativeKernelDrive )
    {
        if( requiresNativeKernelDrive )
            return getNativeMap();
        return getNonNativeMap();
    }
    
    bool RegisterCreationModule( const std::string& libname,
                                 const std::string& ferristype,
                                 const std::string& xsd,
                                 bool requiresNativeKernelDrive,
                                 const std::string& simpleTypes )
    {
        getMap( requiresNativeKernelDrive ).insert(
            make_pair( ferristype, CreationModuleData( libname, xsd, simpleTypes )));
        return true;
    }

    struct FERRISEXP_DLLLOCAL SubCreate_CustomType
    {
        string libname;

        SubCreate_CustomType( string libname )
            :
            libname( libname )
            {
            }
        
        fh_context create( fh_context c, fh_context md )
            {
                typedef map< string, fh_CreationStatelessFunctor > cache_t;
                static cache_t cache;
                cache_t::iterator ci = cache.find( libname );

                LG_CREATE_D << "SL_SubCreate_CustomType() libname:" << libname << endl;

                /**
                 * cache it if its not there
                 */
                if( ci == cache.end() )
                {
//                    const string library_path = FERRIS_CREATION_PLUGIN_DIR + "/" + libname;
                    const string library_path = makeFerrisPluginPath( "creation", libname );
                    
                    GModule* gmod = g_module_open ( library_path.c_str(), G_MODULE_BIND_LAZY );
                    fh_CreationStatelessFunctor (*creator_function)();
    
                    if( !gmod )
                    {
                        fh_stringstream ss;
                        ss  << "Error, unable to open module file:" << library_path << " "
                            << g_module_error ()
                            << endl;
                        Throw_GModuleOpenFailed( tostr(ss), 0 );
                    }

                    if (!g_module_symbol( gmod, "Create", (gpointer*)&creator_function ))
                    {
                        ostringstream ss;
                        ss  << "Error, unable to resolve Create in module file:" << library_path
                            << " this should never happen. Please report it to the mailing list"
                            << " "
                            << g_module_error()
                            << endl;
                        Throw_GModuleOpenFailed( tostr(ss), 0 );
                    }

                    fh_CreationStatelessFunctor f = creator_function();
                    LG_PLUGIN_D << "creator_function:" << toVoid( creator_function )
                                << " f:" << toVoid(f)
                                << endl;
                    ci = cache.insert( make_pair( libname, f ) ).first;
                }

                /**
                 * Should never happen
                 */
                if( ci == cache.end() || !ci->second )
                {
                    LG_PLUGIN_W << "SubCreate_CustomType::create() can't find library:" << libname
                                << endl;
                    fh_stringstream ss;
                    ss << "can't find library:" << libname << endl;
                    Throw_FerrisCreateSubContextFailed( tostr(ss), 0 );
                }
                
                
                return ci->second->create( c, md );
            }
    };

    typedef map< string, SubCreate_CustomType* > CreateObjects_t;

    static void insertCreatorModules_init( CreateObjects_t& output, 
                                           pmap_t::iterator begin,
                                           pmap_t::iterator end )
    {
        for( pmap_t::iterator mi = begin; mi != end; ++mi )
        {
            CreationModuleData&    d = mi->second;
            string ferristype        = mi->first;

            SubCreate_CustomType* obj = new SubCreate_CustomType( d.libname );
            output.insert( make_pair( d.libname, obj ));
        }
    }
    
                                      
    void insertCreatorModules( Context::CreateSubContextSchemaPart_t& m,
                               bool requiresNativeKernelDrive )
    {
        typedef pmap_t::iterator itert;
        typedef Loki::Functor< fh_context,
            LOKI_TYPELIST_2( fh_context, fh_context ) > Perform_t;
        static CreateObjects_t CreateObjects;

        if( CreateObjects.empty() )
        {
            insertCreatorModules_init( CreateObjects,
                                       getMap( false ).begin(),
                                       getMap( false ).end() );
            insertCreatorModules_init( CreateObjects,
                                       getMap( true ).begin(),
                                       getMap( true ).end() );
        }
        
        for( itert mi = getMap( requiresNativeKernelDrive ).begin();
             mi != getMap( requiresNativeKernelDrive ).end(); ++mi )
        {
            CreationModuleData&    d = mi->second;
            string ferristype        = mi->first;

            LG_CREATE_D << "insertAllCreatorModules() ftype:" << ferristype
                        << " libname:" << d.libname
                        << " xsd:" << d.xsd
                        << endl;
            
//             typedef Loki::Functor< fh_context,
//                 LOKI_TYPELIST_3( string, fh_context, fh_context ) > WrappedFunc_t;
//             WrappedFunc_t wf( SL_SubCreate_CustomType );
            
//             m[ ferristype ] = Context::SubContextCreator(
//                 Loki::BindFirst( wf, d.libname ),
//                 d.xsd );

//             if( ferristype == "dbxml" )
//             {
//                 cerr << "ferristype is dbxml. Making one on the side" << endl;
//                 SubCreate_CustomType* scc = CreateObjects[ d.libname ];

//                 try
//                 {
//                     fh_context c = Resolve( "/tmp/junk" );
//                     fh_mdcontext md = new f_mdcontext();
//                     fh_mdcontext child = md->setChild( "dbxml", "" );
//                     child->setChild( "name",  "frodo.dbxml" );
//                     child->setChild( "value", "<x/>" );
//                     fh_context newc = scc->create( c, md );
//                     cerr << "newc:" << newc->getURL() << endl;
//                 }
//                 catch( exception& e )
//                 {
//                     cerr << "ERROR:" << e.what() << endl;
                    
//                 }
                
// //                Perform_t f( CreateObjects[ d.libname ], &SubCreate_CustomType::create );
                
//             }
            
            m[ ferristype ] = Context::SubContextCreator(
                Perform_t( CreateObjects[ d.libname ], &SubCreate_CustomType::create ),
                d.xsd,
                d.simpleTypes );
        }
    }

    void insertAllCreatorModules( Context::CreateSubContextSchemaPart_t& m )
    {
        insertCreatorModules( m, false );
        insertCreatorModules( m, true );
    }

    void insertAbstractCreatorModules( Context::CreateSubContextSchemaPart_t& m )
    {
        insertCreatorModules( m, false );
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    fh_context
    CreationStatelessFunctor::SubCreate_file( fh_context c, fh_context md )
    {
        return c->SubCreate_file( c, md );
    }
    
    fh_context
    CreationStatelessFunctor::SubCreate_ea( fh_context c, fh_context md )
    {
        return c->SubCreate_ea( c, md );
    }
    
    
};

