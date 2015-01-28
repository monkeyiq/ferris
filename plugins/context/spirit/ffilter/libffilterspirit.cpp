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

#include "../SpiritContext.hh"
#include "FerrisContextPlugin.hh"

using namespace std;

#define DEBUG LG_SPIRITCONTEXT_D

namespace Ferris
{
    extern "C"
    {
        void adjustMoveOpsToFront(iter_t i, std::map<parser_id, std::string>& rule_names )
        {
            if( i->children.size() == 3 )
            {
                iter_t  f = i->children.begin();
                iter_t op = i->children.begin()+1;
                DEBUG << "id:" << rule_names[op->value.id()] << endl;
                if( rule_names[op->value.id()] == "comparison" )
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
            return ends_with( rule_names[iter->value.id()], "-eamatch" );
        }
        
        bool adjustRaiseCmp(iter_t iter, std::map<parser_id, std::string>& rule_names )
        {
            return( ends_with( rule_names[iter->value.id()], "comparison" ));
        }
    
        void adjust(iter_t i, std::map<parser_id, std::string>& rule_names )
        {
            adjustMoveOpsToFront( i, rule_names );
            adjustRaiseGeneric  ( i, rule_names, adjustRaiseAndOrNot );
            adjustRaiseGeneric  ( i, rule_names, adjustRaiseCmp );

            for( iter_t iter = i->children.begin(); iter != i->children.end(); ++iter )
            {
                adjust( iter, rule_names );
            }
        }
    
        
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                R ab_getEAmt = str_p("mtime");
                R eakey = regex_p("[]\\[\\ a-z\\.A-Z0-9\\-\\_\\*\\$\\^\\\\\\/\\~\\:]+");

                R EQREGEX = str_p("=~");
                R EQLT    = str_p("<=");
                R EQGT    = str_p(">=");
                R EQ      = str_p("==");
                R EQLTSC  = str_p("<?=");
                R EQGTSC  = str_p(">?=");
                R EQSC    = str_p("=?=");
                R comparison = token_node_d[( EQREGEX | EQLT | EQGT | EQ | EQLTSC | EQGTSC )];
                R eavalue = regex_p("[^)]*");
                
                R eamatch = ( eakey >> root_node_d[comparison] >> eavalue);
                R eamatchbracketed =  inner_node_d[ch_p('(') >> eamatch >> ch_p(')')];
                R noteamatch = str_p("!");
                R oreamatch  = str_p("|");
                R andeamatch = str_p("&");
                R topLevel =  no_node_d[ch_p('(')]
                    >> ( eamatch
                         | ( noteamatch >>   topLevel  )
                         | ( oreamatch  >> +(topLevel) )
                         | ( root_node_d[andeamatch] >> +(topLevel) )
                        )
                    >> no_node_d[ch_p(')')];

                std::map<parser_id, std::string> rule_names;
                rule_names[topLevel.id()]   = "topLevel";
                rule_names[eamatch.id()]    = "eamatch";
                rule_names[andeamatch.id()] = "and-eamatch";
                rule_names[oreamatch.id()]  = "or-eamatch";
                rule_names[noteamatch.id()] = "not-eamatch";
                rule_names[eakey.id()]      = "key";
                rule_names[eavalue.id()]    = "value";
                rule_names[comparison.id()] = "comparison";

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


    
