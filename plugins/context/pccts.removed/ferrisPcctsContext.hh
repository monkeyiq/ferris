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

    $Id: ferrisPcctsContext.hh,v 1.4 2010/09/24 21:31:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FFILTER_CONTEXT_H_
#define _ALREADY_INCLUDED_FFILTER_CONTEXT_H_

#include <FerrisContextPlugin.hh>

//#include <pcctscfg.h>
#include <pccts/config.h>
#include <pccts/pccts_iostream.h>
#include <DLexerBase.h>
#include <ATokPtr.h>
#ifdef PCCTSLEXERFILENAME
#include PCCTSLEXERFILENAME
#else
#include <DLGLexer.h>
#endif
//typedef ANTLRCommonToken ANTLRToken;
#include <Support.hh>
#include <AST.h>

#include <Enamel.hh>

#include <string>
#include <list>

#include <PcctsChildContext.hh>

namespace Ferris
{
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


template <class ParentParser>
class ParserSyntaxHandler : public ParentParser
{
public:

    ParserSyntaxHandler( ANTLRTokenBuffer *input )
        :
        ParentParser( input )
        {
        }
    

            std::list< std::string > syntaxErrors;
            
            void syn(_ANTLRTokenPtr, ANTLRChar *egroup, SetWordType *eset,
                     ANTLRTokenType 	etok, int k)
                {
                    using namespace std;
                    string errorString = getSyntaxErrorString( egroup, eset, etok, k );
                    cerr << errorString << endl;
                    syntaxErrors.push_back( errorString );
                }
            
            std::string 
            edecodeToString(SetWordType *a)
                {
                    using namespace std;
                    stringstream retss;

                    register SetWordType *p = a;
                    register SetWordType *endp = &(p[ParentParser::bsetsize]);
                    register unsigned e = 0;

                    if ( ParentParser::set_deg(a)>1 ) retss << " {";
                    do {
                        register SetWordType t = *p;
                        register SetWordType *b = &(ParentParser::bitmask[0]);
                        do {
                            if ( t & *b ) retss << " " << ParentParser::token_tbl[e];
                            e++;
                        } while (++b < &(ParentParser::bitmask[sizeof(SetWordType)*8]));
                    } while (++p < endp);
                    if ( ParentParser::set_deg(a)>1 ) retss << " }";
                    return retss.str();
                }

            std::string getSyntaxErrorString( ANTLRChar *egroup, SetWordType *eset, ANTLRTokenType etok, int k)
                {
                    using namespace std;
                    stringstream retss;
                    int line;
    
                    line = ParentParser::LT(1)->getLine();

                    ParentParser::syntaxErrCount++;                                   /* MR11 */

                    /* MR23  If the token is not an EOF token, then use the ->getText() value.

                    If the token is the EOF token the text returned by ->getText() 
                    may be garbage.  If the text from the token table is "@" use
                    "<eof>" instead, because end-users don't know what "@" means.
                    If the text is not "@" then use that text, which must have been
                    supplied by the grammar writer.
                    */
                    const char * errorAt = ParentParser::LT(1)->getText();
                    if (ParentParser::LA(1) == ParentParser::eofToken) {
                        errorAt = ParentParser::parserTokenName(ParentParser::LA(1));
                        if (errorAt[0] == '@') errorAt = "<eof>";
                    }
                    retss << "line:" << line << " syntax error at \"" << errorAt << "\"";
                    if ( !etok && !eset )
                    {
                        retss << endl;
                        return retss.str();
                    }
                    if ( k==1 )
                        retss << "\n    missing";
                    else
                    {
                        retss << "; \"" << ParentParser::LT(k)->getText() << "\" not";
                        if ( ParentParser::set_deg(eset)>1 ) retss << " in";
                    }
                    if ( ParentParser::set_deg(eset)>0 ) retss << edecodeToString(eset);
                    else retss << " " << ParentParser::token_tbl[etok];
                    if ( strlen(egroup) > 0 ) retss << " in " << egroup;
                    retss << endl;
                    return retss.str();
                }
    
};
    


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


template <class xLexer, class xParser, class xToken>
class ferrisPcctsContext
    :
    public childContext
{
    typedef childContext Super;
    typedef childContext _Base;
    typedef ferrisPcctsContext<xLexer,xParser,xToken> ThisClass;

public: typedef ferrisPcctsContext<xLexer,xParser,xToken> _Self;
private:    
    
    
    Context* priv_CreateContext( Context* parent, std::string rdn )
        {
            ferrisPcctsContext* ret = new ferrisPcctsContext( parent );
            ret->setContext( parent, rdn );
            return ret;
        }
    

private:
    
	typedef MyParserBlackBox< xLexer, xParser, xToken> box_t;
    box_t* box;
	ASTBase* root;
	fh_ifstream filestream; 

    fh_stringstream StaticStringDataSS;
    std::string StaticStringData;
    
    
    
protected:

    void makeTreeDebug( AST* node, const std::string& s )
        {
            LG_PCCTS_D << "makeTreeDebug s:" << s << "   " << node->getName() << endl;
        }
    
    fh_chcontext makeTreeNode( fh_chcontext parent,
                               AST* node,
                               const std::string& s,
                               const std::string& rdn )
        {
//             cerr << "makeTreeNode(1) rc:" << ref_count << endl;
            makeTreeDebug( node, s );

            LG_PCCTS_D << "adding rdn     :" << rdn
                       << " to parent:" << parent->getDirPath()
                       << endl;
//             cerr << "makeTreeNode() rdn:" << rdn
//                  << " s:" << s
//                  << " to parent:" << parent->getDirPath()
//                  << endl;

            fh_chcontext cc = childContext::Create( parent, rdn );

            LG_PCCTS_D << "created node   :" << cc->getDirPath() << endl;
            return cc;
        }
    
    void makeTree( fh_chcontext parent, AST* node, const std::string& s )
        {
            if( !node ) return;

//             cerr << "pccts::makeTree(top) rc:" << ref_count
//                  << " this:" << toVoid(this)
//                  << " s:" << s << endl;
            LG_PCCTS_D << "makeTree      s:" << s << endl;

//             cerr << "makeTree() node:" << node->getName() << endl;
            
            fh_chcontext child;
            child = makeTreeNode( parent, node, s, node->getName() );

            if( node->down() )
            {
                makeTree( child, (AST*)node->down(), s + node->getName() );
            }
    
            for( AST* n = node;  n = (AST*)n->right(); )
            {
                fh_chcontext child;
                child = makeTreeNode( parent, n, s, n->getName() );
                
                makeTree( child, (AST*)n->down(), s + n->getName());
            }
        }

    virtual void priv_read()
        {
            LG_PCCTS_D << "pccts::read(1) " << getDirPath() << endl;
            LG_FFILTER_D << "Path:" << getDirPath() << endl;

            staticDirContentsRAII _raii1( this );
            if( empty() )
            {
                LG_PCCTS_D << "pccts::read(2) " << getDirPath() << endl;
                if( StaticStringData.length() )
                {
                    LG_PCCTS_D << "pccts::read(2.a) StaticStringData:" << StaticStringData << endl;
                    fh_stringstream ss(StaticStringData);
                    StaticStringDataSS = ss;
                    box = new box_t( StaticStringDataSS );
                }
                else
                {
                    filestream.close();
                    filestream.open( getDirPath().c_str() );
                    box = new box_t( filestream );
                }
                root = NULL;
                box->parser()->interp(&root);
                LG_PCCTS_D << "pccts::read(3) " << getDirPath() << endl;
                if( !box->parser()->syntaxErrors.empty() )
                {
                    fh_stringstream ss;
                    int errorCount = box->parser()->syntaxErrors.size();
                    ss << "syntax error count:" << errorCount << endl;
                    cerr << tostr(ss) << endl;

                    for( stringlist_t::iterator si = box->parser()->syntaxErrors.begin();
                         si != box->parser()->syntaxErrors.end(); ++si )
                    {
                        ss << *si << endl;
                    }
                    LG_PCCTS_D << tostr(ss) << endl;
                    cerr << tostr(ss) << endl;
                    Throw_CanNotReadContextPcctsParseFailed( tostr(ss), this, box->parser()->syntaxErrors );
                }
                makeTree( this, (AST*)root, "" );
                dumpTree();
                LG_PCCTS_D << "pccts::read(4) " << getDirPath() << endl;
            }
            
            LG_PCCTS_D << "pccts::read(ret) " << endl;
        }

    ferrisPcctsContext( const fh_context& parent )
        :
        childContext( parent, "" ),
        root(0),
        StaticStringData("")
        {
            createStateLessAttributes();
            supplementStateLessAttributes();
        }
    
    fh_stringstream getStaticString( Context* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << StaticStringData;
            return ss;
        }
    
    virtual void priv_createAttributes()
        {
            addAttribute( "static-string", this, &_Self::getStaticString );
            Super::priv_createAttributes();
        }
    
public:

    
    ~ferrisPcctsContext()
        {
        }

    void setStaticString( const std::string& s )
        {
            StaticStringData = s;
        }
    

    static _Self* Create( Context* parent, const std::string& rdn )
        {
//             static ferrisPcctsContext x;
//             return x.priv_CreateContext( parent, rdn );
//             if( priv_isSubContextBound( rdn ) )
//             {
//                 return priv_getSubContext( rdn );
//             }

            _Self* ret = new ferrisPcctsContext( parent );
            ret->setContext( parent, rdn );
//            cerr << "ferrisPcctsContext::Create ret->rc:" << ret->ref_count << endl;
            return ret;
            
        }
    

    virtual fh_istream getIStream( ferris_ios::openmode m = ios::in )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
        {
            return f_ifstream( getDirPath() );
        }
    
};



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
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
};

#endif
