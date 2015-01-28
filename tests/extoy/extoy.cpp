/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris ls
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

    $Id: extoy.cpp,v 1.3 2009/06/15 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <iostream>
#include <Ferris.hh>

using namespace std;
using namespace Ferris;

#if 1
int main( int argc, const char** argv )
{
    return 0;
}
#else


#define MAKESTATE   CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ )

class CodeState
{
    string fileName;
    int    line;
    string functionName;

public:
    CodeState( const string& fi, int li, const string& fu )
        :
        fileName(fi),
        line(li),
        functionName(fu)
        {
        }

    inline const string& getFileName() const
        {
            return fileName;
        }

    inline const string& getFunctionName() const
        {
            return functionName;
        }

    inline int getLine() const
        {
            return line;
        }
    
    
};



class ex
{
    string name;
    CodeState State;
    
    
public:
    inline ex( const CodeState& state, const string& s)
        :
        name(s),
        State(state)
        {
        }


    const string what()
        {
            cerr << "what()" << endl;
            ostringstream ss;
            cerr << "what() fn:" << State.getFileName() << endl;
            ss << "ex. name:" << name
               << " fileName:" << State.getFileName()
               << " line:" << State.getLine()
               << " functionName:" << State.getFunctionName()
               ;
            cerr << "XX:" << tostr(ss);
            return tostr(ss);
        }
    
    
};

#define Throw_ex(x) throw ex(MAKESTATE, x)



// template <class X, class SUPER>
// class X : public SUPER
// {
// };



#include "/ferris/plugins/context/postgresql/libferrispostgresql.cpp"
#include "/ferris/plugins/context/postgresql/libferrispostgresqlshared.cpp"
//#include "/ferris/plugins/context/recordfile/libferrisrecordfile.cpp"

#define DEBUG cerr
class FERRISEXP_CTXPLUGIN tupleContext
        :
        public StateLessEAHolder< tupleContext, leafContext >
    {
        typedef tupleContext                            _Self;
        typedef StateLessEAHolder< _Self, leafContext > _Base;

        stringmap_t EA;
        
    public:

        std::string
        priv_getRecommendedEA()
            {
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
            }

        static fh_iostream SL_getValueStream( tupleContext* c, const std::string& rdn, EA_Atom* atom )
            {
            }
        
        tupleContext( Context* parent, const std::string& rdn, const stringmap_t& EA )
            :
            _Base( parent, rdn )
            {
            }
    };

typedef FERRIS_STD_HASH_MAP< std::string, EA_Atom* > Attributes_t;
typedef FERRIS_STD_HASH_MAP< std::string, EA_Atom* > m_RDFCacheAttributes_t;
        typedef Loki::Functor< fh_istream,
                               LOKI_TYPELIST_3( Context*,
                                           const std::string&,
                                           EA_Atom* ) > GetIStream_Func_t;

int main( int argc, const char** argv )
{
    try {

        cerr << "sigc::trackable.sz:" << sizeof(sigc::trackable) << endl;
        cerr << "handlable.sz:" << sizeof(Handlable) << endl;
        cerr << "versioned.sz:" << sizeof(Versioned) << endl;
        cerr << "attr.sz:" << sizeof(Attribute) << endl;
        cerr << "attrcol.sz:" << sizeof(AttributeCollection) << endl;
        cerr << "attributes_t.sz:" << sizeof(Attributes_t) << endl;
        cerr << "context.sz:" << sizeof(Context) << endl;
        cerr << "PostgreSQLTupleContext.sz:" << sizeof(PostgreSQLTupleContext) << endl;
        cerr << "recordfile.tupleContext.sz:" << sizeof(tupleContext) << endl;
        cerr << "------" << endl;
        cerr << "EA_Atom.sz:" << sizeof(EA_Atom) << endl;
        cerr << "EA_Atom_ReadOnly.sz:" << sizeof(EA_Atom_ReadOnly) << endl;
        cerr << "GetIStream_Func_t.sz:" << sizeof(GetIStream_Func_t) << endl;
        cerr << "EA_Atom_ReadWrite_Base.sz:" << sizeof(EA_Atom_ReadWrite_Base<EA_Atom_ReadOnly>) << endl;
        cerr << "EA_Atom_ReadWrite.sz:" << sizeof(EA_Atom_ReadWrite) << endl;
        cerr << "------" << endl;
        cerr << "bool:" << sizeof(bool) << endl;
        cerr << "stringmap_t:" << sizeof(stringmap_t) << endl;
        cerr << "stringmap_t ptr:" << sizeof(stringmap_t*) << endl;
        
//        Throw_ex("test1");
    }
    catch( ex& e )
    {
        cerr << "Cought an ex: " << e.what() << endl;
    }
    catch(...) {cerr << "!!!!!!!!! O K  !!!!!!!!!!\n\n\n";}
    
    return 0;
}
#endif
