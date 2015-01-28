/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

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

    $Id: libferrissopranoeashared.cpp,v 1.6 2009/10/02 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <libferrissopranoeashared.hh>

namespace Ferris
{
    
    RDFCore::fh_model
    SopranoByteArrayAttribute::getModel( Context* c )
    {
        return getDefaultFerrisModel();
    }
    
    std::string
    SopranoByteArrayAttribute::getPredicateURI( const std::string& rdn )
    {
        return Semantic::getPredicateURI( rdn );
    }
    
    ferris_ios::openmode
    SopranoByteArrayAttribute::getSupportedOpenModes()
    {
        return ios::in | ios::out | ios::trunc | ios::binary | ios::ate;
    }
    
    void
    SopranoByteArrayAttribute::createOnDiskAttribute(
        const fh_context& parent,
        const std::string& uri )
    {
        string v = "";

        RDFCore::fh_model  m = getModel( GetImpl( parent ) );

        try
        {
            LG_RDF_D << "createOnDiskAttribute() parent:" << parent->getURL()
                     << " eaname:" << uri
                     << endl;
            
            fh_node mdnode = Semantic::ensureSopranoEASubjectNode( parent );
            m->insert( mdnode,
                       Node::CreateURI( uri ),
                       Node::CreateLiteral( v ) );
            Semantic::setSopranoEASubjectNode( mdnode );
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Failed to create RDF EA for c:" << parent->getURL()
               << " attribute name:" << uri
               << endl;
            ThrowFromErrno( 0, tostr(ss), 0 );
        }
        m->sync();
    }


    fh_iostream
    SopranoByteArrayAttribute::priv_getStream( Context* c, const std::string& rdn,
                                               fh_model m, fh_node subj, fh_node pred, EA_Atom* atom )
    {
        fh_stringstream ss;

        if( !subj )
        {
            fh_stringstream ss;
            ss << "reading attribute:" << rdn;
            Throw_NoSuchAttribute( tostr(ss), c );
        }
        LG_RDF_D << "priv_getStream() c:" << c->getURL()
                 << " subj:" << subj->toString()
                 << " pred:" << pred->toString()
                 << endl;
        
        
        fh_node obj = m->getObject( subj, pred );
        LG_RDF_D << "SopranoByteArrayAttribute::getStream() subj:" << subj->toString()
                 << " pred:" << pred->toString()
                 << " have-obj:" << isBound( obj )
                 << endl;
        
        if( obj )
        {
            ss << obj->toString();

            LG_RDF_D << "SopranoByteArrayAttribute::getStream() subj:" << subj->toString()
                     << " pred:" << pred->toString()
                     << " obj-value:" << obj->toString()
                     << endl;
            
//             cerr << "obj-value:" << obj->toString() << endl;
//             cerr << "lit-value:" << obj->getLiteralValue() << endl;
//             cerr << "is-res:" << obj->isResource() << endl;
//             cerr << "is-lit:" << obj->isLiteral() << endl;
//             cerr << "is-lit-xml:" << obj->isLiteralXML() << endl;
//             cerr << "is-blank:" <<   obj->isBlank() << endl;
        }
        else
        {
//             fh_ostream oss = Factory::fcerr();
//             m->write( oss );
            
            fh_stringstream ss;
            ss << "reading attribute:" << rdn;
            Throw_NoSuchAttribute( tostr(ss), c );
        }
//         cerr << "SopranoByteArrayAttribute::getStream() rdn:" << rdn
//              << " returning." << endl;
        
        return ss;
    }
    


    fh_iostream
    SopranoByteArrayAttribute::getStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_model  m = getModel( c );
        fh_node mdnode = Semantic::ensureSopranoEASubjectNode( c );
        return priv_getStream( c, rdn, m, mdnode, Node::CreateURI( getPredicateURI( rdn ) ), atom );
    }

    void
    SopranoByteArrayAttribute::setStream( Context* c, const std::string& rdn,
                                          EA_Atom* atom, fh_istream ss )
    {
//         ss.seekg(0);
//         LG_RDF_D << "XXXXXXXXX.tostr(3):" << StreamToString(ss) << endl;
//         LG_RDF_D << "XXXXXXXXX.tostr(3) gtell:" << ss.tellg() << endl;
//         ss.seekg(0);
        
        RDFCore::fh_model  m = getModel( c );

        LG_RDF_D << "SopranoAttr::setStream(top)" << endl;

        fh_node mdnode = Semantic::ensureSopranoEASubjectNode( c );
        fh_node pred = Node::CreateURI( getPredicateURI( rdn ) );

        string objectString = StreamToString(ss);
        LG_RDF_D << "SopranoAttr::setStream(1).a pred:" << pred->toString() << endl;
        LG_RDF_D << "SopranoAttr::setStream(1).a obj.sz:" << objectString.size() << endl;
//        LG_RDF_D << "SopranoAttr::setStream(1).a obj:" << objectString << endl;
        LG_RDF_D << "SopranoAttr::setStream(1).b" << endl;
        fh_node literalNode = Node::CreateLiteral( objectString );
//        LG_RDF_D << "SopranoAttr::setStream(1).a obj:" << literalNode->toString() << endl;
        m->set( mdnode, pred, literalNode );
        LG_RDF_D << "SopranoAttr::setStream(2)" << endl;
        Semantic::setSopranoEASubjectNode( mdnode );
        LG_RDF_D << "SopranoAttr::setStream(3)" << endl;
        m->sync();
        LG_RDF_D << "SopranoAttr::setStream() done" << endl;
    }

    
    
    SopranoByteArrayAttribute::SopranoByteArrayAttribute(
        const fh_context& parent,
        const string& rdn,
        const string& uri,
        bool forceCreate )
        :
        EA_Atom_ReadWrite( this, &SopranoByteArrayAttribute::getStream,
                           this, &SopranoByteArrayAttribute::getStream,
                           this, &SopranoByteArrayAttribute::setStream )
    {
        LG_RDF_D << "SopranoByteArrayAttribute() making EA for earl:" << parent->getURL()
                 << " predicate::" << uri
                 << " forceCreate:" << forceCreate
                 << endl;
        if( forceCreate )
        {
            createOnDiskAttribute( parent, uri );
        }
    }

    SopranoByteArrayAttribute::~SopranoByteArrayAttribute()
    {
    }

    SopranoByteArrayAttribute*
    SopranoByteArrayAttribute::Create( const fh_context& parent,
                                       const string& rdn,
                                       bool forceCreate )
    {
        return new SopranoByteArrayAttribute( parent,
                                              rdn,
                                              Semantic::getPredicateURI( rdn ),
                                              forceCreate );
    }

};

