/******************************************************************************
*******************************************************************************
*******************************************************************************

    create a scale from the URLs in the EA index
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

    $Id: ferris-create-fca-scale-nominal.cpp,v 1.6 2010/09/24 21:31:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/


#include "libferrisfcascaling.hh"
#include <Ferris/FactoriesCreationCommon_private.hh>
#include <Ferris/EAIndexerSQLCommon_private.hh>
#include <Ferris/Medallion.hh>

const string PROGRAM_NAME = "ferris-create-fca-scale-nominal";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

string getBitFunctionName( const std::string& attributeName )
{
    static string prefix = "ferris_bitf_";
    string ea = EANameToSQLColumnName( attributeName );
    return prefix + ea;
}


typedef map< string, pair< string, string > > values_t;

void output( values_t& values )
{
    cerr << "output() sz:" << values.size() << endl;
    for( values_t::const_iterator vi = values.begin(); vi != values.end(); ++vi )
    {
        string rdn = vi->first;
        string ea  = vi->second.first;
        string v   = vi->second.second;

        if( v.empty() )
        {
            cout << cleanAttributeName(rdn) << ea << endl;
        }
        else
        {
//          cerr << " ea:" << ea << " v:" << v << endl;
            cout << cleanAttributeName(rdn) << " '(" << ea << "==" << v <<  ")'" << endl;
        }
    }
}

stringmap_t m_ExtraColumnsToInlineInDocmap;
stringlist_t bitflist;
static string hasValid = "has-valid";
unsigned long EmblemDownSet      = 0;
unsigned long DataDrivenEmblem   = 0;
unsigned long isTimeEA = 0;
unsigned long EmblemIncludeMissing = 0;
unsigned long EmblemUseEIDs = 0;
unsigned long EmblemIncludeOrder = 0;
string ValueTableName = "";
const char* ValueTableName_CSTR = 0;
const char* SplitChar_CSTR  = 0;
fh_etagere et = 0;

string getEmblemAttrName( fh_emblem attrem )
{
    stringstream rdnss;
    
    if( attrem->getUniqueName() != attrem->getName() )
    {
        emblems_t el = attrem->getParents();
        if( !el.empty() )
        {
            emblems_t::iterator e  = el.end();
            for( emblems_t::iterator ei = el.begin(); ei!=e; ++ei )
            {
                rdnss << (*ei)->getName() << "-";
            }
            rdnss << attrem->getName();
        }
        else
            rdnss << attrem->getUniqueName();
    }
    else
    {
        rdnss << attrem->getUniqueName();
    }

    return rdnss.str();
}


void makeEmblemAndDownSetFFilter( fh_emblem attrem, string& attrname, string& v )
{
    stringstream ss;
    
    ss << " '(|(" << attrname << "==" << v <<  ")";

    emblems_t el = attrem->getDownset();
    if( el.size() > 1 )
    {
        emblems_t::iterator e  = el.end();
        for( emblems_t::iterator ei = el.begin(); ei!=e; ++ei )
        {
            if( *ei == attrem )
                continue;
                            
            stringstream tmpss;
            tmpss << "emblem:id-" << (*ei)->getID();
            ss << "(" << tmpss.str() << "==" << v <<  ")";
        }
        ss << ")'";
        attrname = ss.str();
        v = "";
    }
}


void processAttribute( work& trans, values_t& values, string attrname, fh_emblem attrem = 0, fh_emblem baseEm = 0 )
{
    if( isTimeEA )
        ValueTableName = "timelookup";
    
    fh_stringstream selectss;
    string selectAttrName = attrname;
    string fromClause = " from docmap d ";
    bool isBoolEA = false;
    string attrCastClause = "::varchar ";
    cerr << "m_ExtraColumnsToInlineInDocmap.size:" << m_ExtraColumnsToInlineInDocmap.size() << endl;
            
    bool isNormalized
        = m_ExtraColumnsToInlineInDocmap.find( attrname ) == m_ExtraColumnsToInlineInDocmap.end();

    stringlist_t::const_iterator isBitfColumnIter = find( bitflist.begin(), bitflist.end(), attrname );

    string emblemColon = "emblem:";
    bool isEmblem = EmblemDownSet || starts_with( attrname, emblemColon );
    if( isEmblem )
    {
        if( EmblemUseEIDs )
        {
            stringstream ss;
            ss << "emblem:id-" << attrem->getID();
            attrname = ss.str();
        }
    }
    if( isEmblem || isBitfColumnIter != bitflist.end() )
    {
        stringstream ss;
        ss << getBitFunctionName( attrname ) << "(d.bitf)";
        selectAttrName = ss.str();
        isBoolEA = true;
        attrCastClause = " ";
    }
    else if( isNormalized )
    {
        selectAttrName = "attrvalue";
        stringstream ss;
//         ss << " from docattrs da, "
//            << ValueTableName << " l, attrmap an " << endl
//            << " where an.attrname = '" << attrname
//            << "' and an.attrid = da.attrid and da.vid = l.vid ";
        ss << " from docattrs da, "
           << ValueTableName << " l " << endl
           << " where da.attrid = "
           << "    (select max(attrid) from attrmap where attrname = '" << attrname << "') "
           << "  and da.vid = l.vid ";
        
        fromClause = ss.str();
    }
    if( isTimeEA )
    {
        stringstream ss;
        ss << "EXTRACT(EPOCH FROM " << selectAttrName << " )";
        selectAttrName = ss.str();
    }
    
    if( !DataDrivenEmblem && isEmblem )
    {
        {
            stringstream rdnss;
            rdnss << attrname.substr( emblemColon.length() );
            values.insert( make_pair( rdnss.str(), make_pair( attrname, "1" ) ) );
        }
        if( EmblemIncludeMissing )
        {
            stringstream rdnss;
            rdnss << "missing" << attrname.substr( emblemColon.length() + strlen("has") );
            values.insert( make_pair( rdnss.str(), make_pair( attrname, "0" ) ) );
        }
        return;
    }
    
    selectss << "SELECT distinct(" << selectAttrName << ")" << attrCastClause
             << fromClause << endl;
    cerr << "SQL:" << endl
         << tostr(selectss) << endl;
    result res = trans.exec( tostr( selectss ) );

    string SplitChar = SplitChar_CSTR ? SplitChar_CSTR : "";
    cerr << "result.size:" << res.size() << endl;
    for (result::const_iterator c = res.begin(); c != res.end(); ++c)
    {
        string v;
        stringstream rdnss;

        if( isBoolEA )
        {
            bool bv;
            c[0].to(bv);
            v = tostr(bv);
            if( bv )
            {
                if( starts_with( attrname, hasValid ) )
                {
                    rdnss << attrname;
                }
                else if( isEmblem )
                {
                    rdnss << getEmblemAttrName( attrem );
                    
//                     if( attrem->getUniqueName() != attrem->getName() )
//                     {
//                         emblems_t el = attrem->getParents();
//                         if( !el.empty() )
//                         {
//                             emblems_t::iterator e  = el.end();
//                             for( emblems_t::iterator ei = el.begin(); ei!=e; ++ei )
//                             {
//                                 rdnss << (*ei)->getName() << "-";
//                             }
//                             rdnss << attrem->getName();
//                         }
//                         else
//                             rdnss << attrem->getUniqueName();
//                     }
//                     else
//                     {
//                         rdnss << attrem->getUniqueName();
// //                      rdnss << attrname.substr( emblemColon.length() );
//                     }
                    
                    if( EmblemIncludeOrder )
                    {
                        makeEmblemAndDownSetFFilter( attrem, attrname, v );
                        
//                         stringstream ss;
//                         ss << " '(|(" << attrname << "==" << v <<  ")";

//                         emblems_t el = attrem->getDownset();
//                         if( el.size() > 1 )
//                         {
//                             emblems_t::iterator e  = el.end();
//                             for( emblems_t::iterator ei = el.begin(); ei!=e; ++ei )
//                             {
//                                 if( *ei == attrem )
//                                     continue;
                            
//                                 stringstream tmpss;
//                                 tmpss << "emblem:id-" << (*ei)->getID();
//                                 ss << "(" << tmpss.str() << "==" << v <<  ")";
//                             }
//                             ss << ")'";
//                             attrname = ss.str();
//                             v = "";
//                         }
                    }
                }
                else
                {
                    rdnss << attrname << "_" << v;
                }
            }
            else
            {
                if( starts_with( attrname, hasValid ) )
                {
                    rdnss << "no" << attrname.substr( hasValid.length() );
                }
                else if( isEmblem )
                {
                    if( EmblemIncludeMissing )
                    {
                        rdnss << "missing" << attrname.substr( emblemColon.length() + strlen("has") );
                    }
                    if( EmblemIncludeOrder )
                    {
                        string tattrname = attrname;
                        string tv = v;
                        makeEmblemAndDownSetFFilter( attrem, tattrname, tv );
                        values.insert( make_pair( getEmblemAttrName( attrem ), make_pair( tattrname, tv ) ) );
                    }
                }
                else
                {
                    rdnss << attrname << "_" << v;
                }
            }
        }
        else
        {
            c[0].to(v);
            if( v.empty() )
                continue;
            rdnss << attrname << "_";
            if( isTimeEA )
                rdnss << formalTimeValueForFormalAttribute( v );
            else
                rdnss << EANameToSQLColumnName(v);
        }

        if( !rdnss.str().empty() )
        {
            values.insert( make_pair( rdnss.str(), make_pair( attrname, v ) ) );
        }
        

        
        if( SplitChar_CSTR )
        {
            stringlist_t sl;
            Util::parseSeperatedList( v, sl, SplitChar[0] );

            if( !sl.empty() )
            {
                stringlist_t::const_iterator end = sl.end();
                --end;
                
                for( stringlist_t::const_iterator si = sl.begin(); si!=end; ++si )
                {
                    string v = *si;
                    cerr << "pfx v:" << v << endl;
                    stringstream rdnss;
                    rdnss << attrname << "_" << v;
                    values.insert( make_pair( rdnss.str(), make_pair( attrname, v ) ) );
                }
            }
        }
        
    }
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose            = 0;
        const char*   findexPath_CSTR    = 0;
        const char* UseETagereAtPath     = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "emblem-down-set", 'E', POPT_ARG_NONE, &EmblemDownSet, 0,
                  "create an attribute for each emblem in the downset of the given emblem", "" },

                { "emblem-include-missing", '0', POPT_ARG_NONE, &EmblemIncludeMissing, 0,
                  "create an attribute for each emblem for ==0 case as well", "0" },

                { "emblem-use-ids", 'i', POPT_ARG_NONE, &EmblemUseEIDs, 0,
                  "output ffilters using emblem IDs instead of names", "0" },

                { "emblem-include-order", 'O', POPT_ARG_NONE, &EmblemIncludeOrder, 0,
                  "include emblem partial order information into ffilters", "0" },
                
                { "data-driven-emblem-ffilters", 0, POPT_ARG_NONE, &DataDrivenEmblem, 0,
                  "do not create an attribute for a emblem scenario which is not in the index", "" },

                { "value-table", 'L', POPT_ARG_STRING, &ValueTableName_CSTR, 0,
                  "tablename containing values", "guessed from attribute name" },

                { "is-time-attribute", 'T', POPT_ARG_NONE, &isTimeEA, 0,
                  "handle attribute as a time_t", "" },
                
                { "split-char", '1', POPT_ARG_STRING, &SplitChar_CSTR, 0,
                  "split the values of the scale with this char and create a virtual hierarchy", "" },

                { "use-etagere", 0, POPT_ARG_STRING, &UseETagereAtPath, 0,
                  "use etagere at specified location instead of default", "" },
                
                { "index-path", 'P', POPT_ARG_STRING, &findexPath_CSTR, 0,
                  "path to existing postgresql EA index", "" },

                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* [-P ea-index-url] attrname");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        EAIndex::fh_idx idx = getEAIndex( findexPath_CSTR );
        string host   = idx->getConfig( CFG_IDX_HOST_K, CFG_IDX_HOST_DEF );
        string dbname = idx->getConfig( CFG_IDX_DBNAME_K, CFG_IDX_DBNAME_DEF );
        
        stringstream conSS;
        conSS << " host=" << host;
        conSS << " dbname=" << dbname;
        connection con( conSS.str() );
        work trans( con, "getting the schema..." );

        const char* CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K
        = CFG_EAIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K;

        m_ExtraColumnsToInlineInDocmap =
            Util::ParseKeyValueString(
                idx->getConfig( CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                                CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT )
                );
        
        string bitfColumnsRAW = idx->getConfig(
            CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_CREATE_K, 
            CFG_POSTGRESQLIDX_COLUMNS_TO_INLINE_IN_BITF_DEFAULT );
        Util::parseCommaSeperatedList( bitfColumnsRAW, bitflist );

        values_t values;
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string attrname = RootNameCSTR;
            if( ValueTableName_CSTR )
                ValueTableName = ValueTableName_CSTR;
            else
                ValueTableName = guessLookupTableName( trans, attrname );
            cerr << "ValueTableName:" << ValueTableName << endl;

            et = Ferris::Factory::getEtagere();
            if( UseETagereAtPath )
            {
                et = Ferris::Factory::makeEtagere( UseETagereAtPath );
                Ferris::Factory::setDefaultEtagere( et );
            }

            string emblemColon = "emblem:";
            bool isEmblem = EmblemDownSet || starts_with( attrname, emblemColon );
            fh_emblem attrem = 0;
            if( isEmblem )
            {
                if( !attrem )
                {
                    string emblemHasPFX = "emblem:has-";
                    string emblemName = attrname;
                    if( starts_with( attrname, emblemHasPFX ) )
                        emblemName = attrname.substr( emblemHasPFX.length() );
                    
                    attrem = et->getEmblemByName( emblemName );
                }
//                 cerr << "attrem:" << attrem << endl;
//                 cerr << "attrem.name:" << attrem->getName() << endl;
//                 cerr << "attrem.id:" << (long long)attrem->getID() << endl;
            }
            
            processAttribute( trans, values, attrname, attrem, attrem );

            if( EmblemDownSet )
            {
                string emblemHasPFX = "emblem:has-";
                string emblemName = attrname;
                if( starts_with( attrname, emblemHasPFX ) )
                    emblemName = attrname.substr( emblemHasPFX.length() );
                if( starts_with( attrname, "emblem:id-" ) )
                    emblemName = attrname.substr( strlen("emblem:id-") );
                cerr << "emblemName:" << emblemName << endl;

                fh_emblem em = et->getEmblemByName( emblemName );
                emblems_t el = em->getDownset();
                for( emblems_t::const_iterator ei = el.begin(); ei!=el.end(); ++ei )
                {
                    stringstream ss;
                    ss << emblemHasPFX << (*ei)->getName();
                    processAttribute( trans, values, ss.str(), *ei, attrem );
                }
            }
        }
        output( values );
    }
    catch( exception& e )
    {
        LG_FCA_ER << PROGRAM_NAME << " cought exception:" << e.what() << endl;
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


