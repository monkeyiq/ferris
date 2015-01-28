/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libferrissoprano.cpp,v 1.12 2009/10/02 21:30:16 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <libferrissopranoeashared.hh>
#include <FerrisBoost.hh>

using namespace std;

namespace Ferris
{
    using namespace RDFCore;
    using namespace Semantic;

    
    class FERRISEXP_DLLLOCAL EAGenerator_Soprano
        :
        public MatchedEAGeneratorFactory
    {
    protected:

        virtual void Brew( const fh_context& a );

        void generateAttribute( const fh_context& a,
                                const std::string& rdn,
                                bool forceCreate = false );
        
    public:

        EAGenerator_Soprano();

        virtual bool isDynamic()
            {
                return true;
            }
        virtual bool supportsCreateForContext( fh_context c )
            {
                return true;
            }
        
        fh_attribute CreateAttr(
            const fh_context& a,
            const string& rdn,
            fh_context md = 0 )
            throw(
                FerrisCreateAttributeFailed,
                FerrisCreateAttributeNotSupported
                );

        virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    
    
    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


/**
 * We allow the schema:eaname to be saved on disk as an RDF EA aswell.
 * This is a little tricky, if its not saved on disk then we return
 * the URL for FXD_BINARY_NATIVE_EA as the data in Stream requests.
 * if the user updates the stream with some data then we save it to
 * disk and return the disk data in future Stream requests.
 */
class FERRISEXP_DLLLOCAL SopranoByteArrayAttributeSchema
    :
        public SopranoByteArrayAttribute
{
    typedef SopranoByteArrayAttributeSchema _Self;
    typedef SopranoByteArrayAttribute       _Base;

    
public:

    virtual fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
        {
            RDFCore::fh_model  m = getDefaultFerrisModel();

            try
            {
                return _Base::getStream( c, rdn, atom );
            }
            catch( NoSuchAttribute& e )
            {
                // try for subtree schema
                if( c->isParentBound() )
                {
                    string subtreeurl = getStrAttr( c->getParent(), "subtree" + rdn, "" );
                    if( !subtreeurl.empty() )
                    {
                        fh_stringstream ss;
                        ss << subtreeurl;
                        return ss;
                    }
                }
                Factory::xsdtypemap_t tmap;
                Factory::makeBasicTypeMap( tmap );
                fh_context schema = tmap[ FXD_BINARY_NATIVE_EA ];
                
                fh_stringstream ss;
                ss << schema->getURL();
                return ss;
            }
        }
    
    SopranoByteArrayAttributeSchema( const fh_context& parent,
                                     const string& rdn,
                                     const string& uri,
                                     bool forceCreate = false
        )
        :
        _Base( parent, rdn, uri, forceCreate )
        {
            LG_RDF_D << "SopranoByteArrayAttributeSchema() parent:" << parent->getURL()
                     << " rdn:" << rdn
                     << " uri:" << uri
                     << " forceCreate:" << forceCreate
                     << endl;

//             if( forceCreate )
//             {
//                 RDFCore::fh_model m = getDefaultFerrisModel();
//                 fh_node subj = Node::CreateURI( parent->getURL() );
//                 fh_node pred = Node::CreateURI( getPredicateURI( rdn ) );
//                 m->insert( subj, pred, Node::CreateLiteral( "" ) );
//             }
            
        }

    
    static SopranoByteArrayAttributeSchema* Create( const fh_context& parent,
                                                    const string& rdn,
                                                    bool forceCreate = false )
        {
            return new SopranoByteArrayAttributeSchema( parent,
                                                        rdn,
                                                        Semantic::getPredicateURI( rdn ),
                                                        false ); // forceCreate );
        }
    
};




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


EAGenerator_Soprano::EAGenerator_Soprano()
    :
    MatchedEAGeneratorFactory()
{
}

void
EAGenerator_Soprano::generateAttribute( const fh_context& a,
                                        const std::string& rdn,
                                        bool forceCreate )
{
    LG_RDF_D << "EAGenerator_Soprano::generateAttribute() a:" << a->getURL()
             << " rdn:" << rdn
             << endl;
    
    a->addAttribute( rdn,
                     (EA_Atom*)SopranoByteArrayAttribute::Create( a, rdn, forceCreate ),
                     FXD_BINARY_NATIVE_EA
        );

    //
    // Attach schema
    //
    if( !starts_with( rdn, "schema:" ))
    {
        string schema_name      = "schema:" + rdn;

        if( ! a->isAttributeBound( schema_name, false ))
        {
            if( ! a->addAttribute(
                    schema_name,
                    (EA_Atom*)SopranoByteArrayAttributeSchema::Create( a, schema_name, forceCreate ),
                    XSD_SCHEMA ))
            {
                ostringstream ss;
                ss << "Can't create schema RDF statement for rdn:" << rdn
                   << " schema_name:" << schema_name
                   << " forceCreate:" << forceCreate;
                Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
            }
        }
    }
}



void
EAGenerator_Soprano::Brew( const fh_context& a )
{
//    Time::Benchmark("Soprano::Brew() " + a->getURL() );
    LG_RDF_D << "EAGenerator_Soprano::Brew() url:" << a->getURL() << endl;
//    cerr << "EAGenerator_Soprano::Brew() url:" << a->getURL() << endl;

//     if( starts_with( a->getURL(), "postgres" ))
//         return;
    
    static bool brewing = false;
    if( brewing )
        return;
    Util::ValueRestorer< bool > dummy1( brewing, true );

    LG_RDF_D << "EAGenerator_Soprano::Brew(2) url:" << a->getURL() << endl;

    // Sometimes we don't want to even try RDF EA for a context.
    {
        const char* FERRIS_RDF_STORED_EA_SKIP_URL_REGEX_CSTR = getenv("FERRIS_RDF_STORED_EA_SKIP_URL_REGEX");
        
        if( FERRIS_RDF_STORED_EA_SKIP_URL_REGEX_CSTR )
        {
            static boost::regex rex = toregexi( FERRIS_RDF_STORED_EA_SKIP_URL_REGEX_CSTR );
            if( regex_match( a->getURL(), rex ))
            {
                LG_RDF_D << "EAGenerator_Soprano::Brew() SKIP_URL_REGEX matches for url:" << a->getURL() << endl;
                return;
            }
        }
    }
    
    LG_RDF_D << "EAGenerator_Soprano::Brew(3) url:" << a->getURL() << endl;

    
    try
    {
        stringlist_t rdf_ea_names;

        string            earl = a->getURL();
        RDFCore::fh_model        m = getDefaultFerrisModel();

        fh_node mdnode = Semantic::tryToGetSopranoEASubjectNode( a );

        LG_RDF_D << "Brew() "
                 << " url:" << a->getURL()
                 << " mdnode:" << toVoid(mdnode)
                 << endl;
        
//        cerr << "EA_Soprano::Brew() earl:" << earl << " hasEA:" << isBound(hasEA) << endl;

        if( !mdnode )
            return;

        LG_RDF_D << "Brew(1) "
                 << " url:" << a->getURL()
                 << " uuid:" << tryToGetUUIDNode(a)->toString()
                 << " mdnode:" << mdnode->toString()
                 << endl;

        RDFCore::NodeIterator iter = m->getArcsOut( mdnode );
        bool virgin = true;
        for( ; iter != NodeIterator(); ++iter )
        {
            if( virgin )
            {
                virgin = false;
                a->setHasDynamicAttributes( true );
            }
            
            LG_RDF_D << "Brew(2) "
                     << " url:" << a->getURL()
                     << " mdnode:" << toVoid(mdnode)
                     << " iter:" << (*iter)->toString()
                     << endl;
            
            if( !(*iter)->isResource() )
                continue;
            LG_RDF_D << "Brew(2.1)" << endl;
            
            string rdn = (*iter)->getURI()->toString();
//             cerr << "Found another arc out for earl:" << earl
//                  << " rdn:" << rdn
//                  << endl;

            LG_RDF_D << "Brew(2.2)" << endl;

            rdn = Semantic::stripPredicateURIPrefix( rdn );
            LG_RDF_D << "Brew(3) rdn:" << rdn << endl;
            
            generateAttribute( a, rdn );
            rdf_ea_names.push_back( rdn );
        }

        if( !virgin )
        {
            /*
             * Add a new EA showing the names of all the RDF EA
             */
            a->addAttribute( "rdf-ea-names",
                             Util::createCommaSeperatedList( rdf_ea_names ),
                             FXD_EANAMES,
                             true );
        }
    }
    catch( exception& e )
    {
        LG_RDF_ER << "Failed to load RDF EA, error:" << e.what() << endl;
    }

    LG_RDF_D << "Brew() "
             << " url:" << a->getURL()
             << " complete"
             << endl;
}

bool
EAGenerator_Soprano::tryBrew( const fh_context& ctx, const std::string& eaname )
{
//    Time::Benchmark("Soprano::tryBrew() " + ctx->getURL() );
    
//     cerr << "EAGenerator_Soprano::tryBrew() url:" << ctx->getURL()
//          << " eaname:" << eaname
//          << endl;

    static bool brewing = false;
    if( brewing )
        return false;
    Util::ValueRestorer< bool > dummy1( brewing, true );

    LG_RDF_D << "tryBrew() "
             << " url:" << ctx->getURL()
             << " eaname:" << eaname
             << endl;
    bool ret = false;

    RDFCore::fh_model m = getDefaultFerrisModel();
    string     earl = ctx->getURL();
    fh_node  mdnode = Semantic::tryToGetSopranoEASubjectNode( ctx );
    
    if( mdnode )
    {
        fh_node        pred = Node::CreateURI( Semantic::getPredicateURI( eaname ));
        fh_node     objnode = m->getObject( mdnode, pred );

        LG_RDF_D << "tryBrew() "
                 << " url:" << ctx->getURL()
                 << " eaname:" << eaname
                 << " mdnode:" << mdnode
                 << " objnode:" << objnode
                 << endl;
        
        if( objnode )
        {
            ret = true;
            generateAttribute( ctx, eaname );
            return ret;
        }
    }
    
    return ret;
}



fh_attribute
EAGenerator_Soprano::CreateAttr( const fh_context& a,
                                 const string& urdn,
                                 fh_context md )
    throw(
        FerrisCreateAttributeFailed,
        FerrisCreateAttributeNotSupported
        )
{
    try
    {
        string rdn = urdn;

//        cerr << "EAGenerator_Soprano::CreateAttr() rdn:" << rdn << endl;
        LG_RDF_D << "EAGenerator_Soprano::CreateAttr() rdn:" << rdn << endl;
//        cerr << "EAGenerator_Soprano::CreateAttr() rdn:" << rdn << endl;

        string      earl = a->getURL();
        RDFCore::fh_model  m = getDefaultFerrisModel();
        
        try
        {
            ensureSopranoEASubjectNode( a );
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Failed to create RDF EA for c:" << a->getURL()
               << " attribute name:" << rdn
               << " error:" << e.what()
                   << endl;
            ThrowFromErrno( 0, tostr(ss), 0 );
        }

        generateAttribute( a, rdn, true );
        fh_attribute ret = a->getAttribute( rdn );
        return ret;
    }
    catch( FerrisCreateAttributeFailed& e )
    {
        cerr << "RDF out of band EA module::create() e:" << e.what() << endl;
        throw e;
    }
    catch( exception& e )
    {
        fh_stringstream ss;
        ss << e.what();
        cerr << "RDF out of band EA module::create() e:" << e.what() << endl;
//        cerr << tostr(ss) << endl;
        Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
    }
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C"
{
    FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
    {
        return new EAGenerator_Soprano();
    }
};



 
};
