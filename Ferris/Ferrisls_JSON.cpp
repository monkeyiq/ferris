/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferrisls client helper code.

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

    $Id: Ferrisls_XML.cpp,v 1.4 2010/09/24 21:30:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <FerrisDOM.hh>
#include <Ferrisls.hh>
#include <FilteredContext.hh>
#include <Context.hh>
#include <iomanip>
#include <ContextSetCompare_private.hh>
#include "Trimming.hh"
#include "ValueRestorer.hh"
#include <FerrisBoost.hh>
#include <FerrisStdHashMap.hh>
#include <FerrisQt_private.hh>
#include <qjson/serializer.h>

using namespace std;


namespace Ferris
{
    struct Ferrisls_json_display_private
    {
        QVariantMap top;
        std::list< QVariantMap > topstack;
        QVariantMap dm;
        
        Ferrisls_json_display_private()
            {
            }
    };
    
    
    Ferrisls_json_display::Ferrisls_json_display()
        :
        P( new Ferrisls_json_display_private() )
    {
    }
    
    Ferrisls_json_display::~Ferrisls_json_display()
    {
        delete P;
    }

    
    void
    Ferrisls_json_display::PrintEA( fh_context ctx,
                                   int i,
                                   const std::string& attr,
                                   const std::string& EA )
    {
        fh_stringstream bss;
        bss << EA;
        std::string ealine;
        getline( bss, ealine );
        PostfixTrimmer trimmer;
        trimmer.push_back( "\r" );
        string v = trimmer( ealine );
        P->dm[ attr.c_str() ] = v.c_str();
    }
        
    void
    Ferrisls_json_display::ShowAttributes( fh_context ctx )
    {
        cerr << "ShowAttributes() ctx:" << ctx->getURL() << endl;
        P->dm = QVariantMap();
        _Base::ShowAttributes( ctx );
        P->top[ ctx->getDirName().c_str() ] = P->dm;
        P->dm = QVariantMap();

        // stringlist_t& an = getDefaultEAToIncludeJSON();
        // QVariantMap dm;
        // contextToJSONProcessContext( ctx, dm, an, false );
    }

    void
    Ferrisls_json_display::workStarting()
    {
    }

    void
    Ferrisls_json_display::workComplete()
    {
        QJson::Serializer zz;
        QByteArray ba = zz.serialize( P->top );
        string ret = tostr(ba);
        cout << ret << flush;
    }

    void
    Ferrisls_json_display::EnteringContext(fh_context ctx)
    {
        cerr << "EnteringContext() ctx:" << ctx->getURL() << endl;
        P->dm = QVariantMap();
        _Base::ShowAttributes( ctx );
        P->top[ "self" ] = P->dm;
        P->topstack.push_back( P->top );
        P->top = QVariantMap();
        P->dm = QVariantMap();
    }

    void
    Ferrisls_json_display::LeavingContext(fh_context ctx)
    {
        cerr << "LeavingContext() ctx:" << ctx->getURL() << endl;
        QVariantMap t = P->top;
        P->top = P->topstack.back();
        P->topstack.pop_back();
        P->top[ "children" ] = t;
        P->dm = QVariantMap();
    }


    
};

