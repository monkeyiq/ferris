/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2011 Ben Martin

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

    $Id: libffilter.cpp,v 1.3 2010/09/24 21:31:43 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "SpiritContext.hh"
#include "FerrisContextPlugin.hh"

using namespace std;

#define DEBUG LG_SPIRITCONTEXT_D

namespace Ferris
{
    extern "C"
    {
        void adjustMoveOpsToFront(iter_t i, std::map<parser_id, std::string>& rule_names )
        {
            if( i->children.size() >= 3 )
            {
                iter_t  f = i->children.begin();
                iter_t op = i->children.begin()+1;
                DEBUG << "id:" << rule_names[op->value.id()] << endl;
                if( rule_names[op->value.id()] == "cmp" )
                {
                    DEBUG << "swapping!" << endl;
                    swap( f, op );
                }
            }
            for( iter_t iter = i->children.begin(); iter != i->children.end(); ++iter )
            {
                adjustMoveOpsToFront( iter, rule_names );
            }
        }

        bool adjustRaiseAndOrNot(iter_t iter, std::map<parser_id, std::string>& rule_names )
        {
            return ends_with( rule_names[iter->value.id()], "cmp" );
        }
        
    
        void adjust(iter_t i, std::map<parser_id, std::string>& rule_names )
        {
            adjustMoveOpsToFront( i, rule_names );
            adjustRaiseGeneric  ( i, rule_names, adjustRaiseAndOrNot );

            for( iter_t iter = i->children.begin(); iter != i->children.end(); ++iter )
            {
                adjust( iter, rule_names );
            }
        }
    
        
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
        {
            try
            {
                DEBUG << "Brew()" << endl;
                
                R term  = regex_p("[^ ]+");
                R cmp   = token_node_d[str_p("AND") | str_p("&") | str_p("OR") | str_p("NOT") | str_p("MINUS")];
                R topLevel =  term >> *(no_node_d[str_p(" ")]>> cmp >> no_node_d[str_p(" ")] >> term);

                std::map<parser_id, std::string> rule_names;
                rule_names[topLevel.id()] = "topLevel";
                rule_names[term.id()]     = "term";
                rule_names[cmp.id()]      = "cmp";

                cerr << "brew()" << endl;
                Context* ret = BrewSpirit( topLevel, rf, rule_names, adjust );
                return ret;
            }
            catch( exception& e )
            {
                fh_stringstream ss;
                ss << "libffilter.cpp::Brew() cought:" << e.what() << endl;
                cerr << "error:" << e.what() << endl;
                Throw_RootContextCreationFailed( tostr(ss), 0 );
            }
            return 0;
        }
    }
};


    
