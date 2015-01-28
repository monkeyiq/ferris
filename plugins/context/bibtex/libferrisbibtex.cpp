/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
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

    $Id: libferrisbibtex.cpp,v 1.4 2010/09/24 21:31:32 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Ferris.hh>
#include <TypeDecl.hh>
#include <Trimming.hh>
#include <General.hh>
#include <Cache.hh>

#include <config.h>

extern "C" {
#include <btparse.h>
};


using namespace std;


namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    /**
     * Context for a single @entry{} in the bibtex
     */
    class FERRISEXP_CTXPLUGIN bibtexEntryContext
        :
        public leafContext
    {
        typedef bibtexEntryContext  _Self;
        typedef leafContext         _Base;

    public:

        bibtexEntryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        
        virtual ~bibtexEntryContext()
            {
            }
    };

    /**
     *
     */
    class FERRISEXP_CTXPLUGIN bibtexRegularEntryContext
        :
        public StateLessEAHolder< bibtexRegularEntryContext, bibtexEntryContext >
    {
        typedef bibtexRegularEntryContext                                          _Self;
        typedef StateLessEAHolder< bibtexRegularEntryContext, bibtexEntryContext > _Base;

        stringmap_t m_sm;
        static stringset_t& getStatelessNames()
            {
                static stringset_t ss;
                if( ss.empty() )
                {
                    ss.insert("author");
                    ss.insert("title");
                    ss.insert("booktitle");
                    ss.insert("year");
                    ss.insert("subject");
                    ss.insert("note");
                    ss.insert("pages");
                    ss.insert("file");
                    ss.insert("editor");
                    ss.insert("publisher");
                    ss.insert("series");
                    ss.insert("url");
                    ss.insert("isbn");
                    ss.insert("abstract");
                    ss.insert("seealso");
                }
                return ss;
            }

        
        
    protected:
        virtual std::string priv_getRecommendedEA()
            {
                return adjustRecommendedEAForDotFiles(this, "name,author,title,booktitle,year");
            }
        static fh_istream
        SL_getStatelessAttributeIStream( const string& key, bibtexRegularEntryContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ret;
                ret << c->m_sm[ key ];
                return ret;
            }
        
        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    for( stringset_t::const_iterator si = getStatelessNames().begin();
                         si != getStatelessNames().end(); ++si )
                    {
                        tryAddStateLessAttribute( *si,
                                                  Loki::BindFirst(
                                                      Loki::Functor<fh_istream, LOKI_TYPELIST_4( const string&,
                                                                                            bibtexRegularEntryContext*,
                                                                                            const std::string&,
                                                                                            EA_Atom* ) >
                                                  (SL_getStatelessAttributeIStream), *si ),
                            FXD_BINARY );
                    }
                    _Base::createStateLessAttributes( 1 );
                }
            }
        
    public:
        
        bibtexRegularEntryContext( Context* parent, const std::string& rdn, AST* entry_ast )
            :
            _Base( parent, rdn )
            {
                char* entry_type = bt_entry_type ( entry_ast );

                LG_BIBTEX_D << "found rdn:" << rdn << endl;
                            
                char* field_name = 0;
                AST* field = NULL;
                while (field = bt_next_field (entry_ast, field, &field_name))
                {
                    string key   = field_name;
                    string value = bt_get_text ( field );
                    LG_BIBTEX_D << "found key:" << key << " value:" << value << endl;

                    if( getStatelessNames().find( key ) != getStatelessNames().end() )
                    {
                        m_sm[ key ] = value;
                    }
                    else
                    {
                        addAttribute( key, value );
                    }
                }
                
                createStateLessAttributes( true );
            }
        
        virtual ~bibtexRegularEntryContext()
            {
            }
    };
    FERRIS_SMARTPTR( bibtexRegularEntryContext, fh_bibtexRegularEntryContext );
    

    /*
     * base bibtex file context
     */
    class FERRISEXP_CTXPLUGIN bibtexRootContext
        :
        public FakeInternalContext
    {
        typedef bibtexRootContext    _Self;
        typedef FakeInternalContext  _Base;

    protected:

        virtual void priv_read()
            {
                staticDirContentsRAII _raii1( this );

                LG_BIBTEX_D << "priv_read() url:" << getURL() << endl;

                if( empty() )
                {
                    string earl = getURL();
                    string path = getCoveredContext()->getDirPath();
                    
                    LG_BIBTEX_D << "reading bibtex file at url:" << earl
                                << " path:" << path
                                << endl;
                    boolean status = 0;
                    ushort  options = 0;
                    AST* entries = bt_parse_file( (char*)path.c_str(), options, &status );
                    AST* entry = NULL;
                    while (entry = bt_next_entry (entries, entry))
                    {
                        bt_metatype mt = bt_entry_metatype ( entry );
                        
                        if( mt != BTE_REGULAR )
                            continue;
                        
                        string rdn = bt_entry_key ( entry );
                        LG_BIBTEX_D << "got entry! rdn:" << rdn << endl;
                        if( priv_isSubContextBound( rdn ) )
                        {
                            LG_BIBTEX_W << "Found duplicate entry:" << rdn << endl;
                            continue;
                        }
                        
                        fh_bibtexRegularEntryContext child = new bibtexRegularEntryContext( this, rdn, entry );
                        Insert( GetImpl(child) );
                    }
                }
            }
    public:
        
        bibtexRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        bibtexRootContext()
            {
                createStateLessAttributes();
                createAttributes();
            }
        
        virtual ~bibtexRootContext()
            {
            }
        virtual std::string priv_getRecommendedEA()
            {
                cerr << "bibtexRootContext::priv_getREA()" << endl;
                return "name,author,title,booktitle,year";
            }
        virtual std::string getRecommendedEA()
            {
                cerr << "bibtexRootContext::getREA()" << endl;
                return "name,author,title,booktitle,year";
            }

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        bibtexRootContext* priv_CreateContext( Context* parent, string rdn )
            {
                bibtexRootContext* ret = new bibtexRootContext();
                ret->setContext( parent, rdn );
                return ret;
            }
        
    };
    

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    extern "C"
    {
        struct btSingleton 
        {
            btSingleton()
                {
                    bt_initialize ();
                }
            ~btSingleton()
                {
//                    bt_cleanup ();
                }
            
        };
        
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                static btSingleton _btSingleton;

                static bibtexRootContext c;
                fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
                return ret;
                
//                 const string& root = rf->getInfo( RootContextFactory::ROOT );
//                 const string& path = rf->getInfo( RootContextFactory::PATH );
// //                cerr << " root:" << root << " path:" << path << endl;

//                 static bibtexRootContext* c = 0;

//                 if( !c )
//                 {
//                     LG_EVO_D << "Making bibtexRootContext(1) " << endl;
//                     c = new bibtexRootContext(0, "/");
            
//                     // Bump ref count.
//                     static fh_context keeper = c;
//                     static fh_context keeper2 = keeper;
//                     LG_EVO_D << "Making bibtexRootContext(2) " << endl;
//                 }

//                 fh_context ret = c;

//                 if( root != "/" )
//                 {
//                     fh_stringstream ss;
//                     ss << root << "/" << path;
//                     rf->AddInfo( RootContextFactory::PATH, tostr(ss) );
//                 }

//                 return ret;
            }
            catch( exception& e )
            {
                LG_EVO_D << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
