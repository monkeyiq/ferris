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

    $Id: ferris-create-fca-scale-numeric.cpp,v 1.8 2010/09/24 21:31:09 ben Exp $

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

const string PROGRAM_NAME = "ferris-create-fca-scale-numeric";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

unsigned long isTimeEA = 0;
unsigned long ExcludeZero = 0;
unsigned long ReverseOrder = 0;
string ffilterReverseComparitor = ">";
string ffilterComparitor = "<=";

typedef map< string, double, versionltstr > values_t;

unsigned long IncludeMinValue = 0;

template < class v_iterator_t >
values_t&
performDataDrivenOrdinalScale(
    values_t& v,
    v_iterator_t vbegin,
    v_iterator_t vend,
    int v_size,
    int TargetNumberOfAttrs )
{
    int advancement = (v_size-1) / TargetNumberOfAttrs;
    v_iterator_t vi = vbegin;
    if( !IncludeMinValue )
        advance( vi, advancement );
        
    for( int i=!IncludeMinValue; i<TargetNumberOfAttrs; ++i )
    {
        if( vi == vend )
            break;
        v[ vi->first ] = vi->second;
        advance( vi, advancement );
    }
    vi = vend;
    --vi;
    v[ vi->first ] = vi->second;
    return v;
}

template < class v_iterator_t >
values_t&
performOrdinalScale(
    values_t& v,
    v_iterator_t vbegin,
    v_iterator_t vend,
    int v_size,
    int TargetNumberOfAttrs,
    bool isDataDriven,
    const std::string attrname
    )
{
    if( !TargetNumberOfAttrs )
    {
        copy( vbegin, vend, inserter( v, v.end() ));
        return v;
    }
    
    if( isDataDriven )
        return performDataDrivenOrdinalScale(  v, vbegin, vend, v_size, TargetNumberOfAttrs );

    float min = vbegin->second;
    v_iterator_t last = vend;
    --last;
    float max = last->second;

    int value_incr = (int)((MAX(max,min)-MIN(max,min))/TargetNumberOfAttrs);
    if( min > max )
        value_incr *= -1;
    int value = (int)(min);
    if( !IncludeMinValue )
        value += value_incr;

    cerr << "start total size:" << distance( vbegin, vend ) << "  min:" << fixed << min << " max:" << fixed << max << endl;
    cerr << " mintt:" << Time::toTimeString( (time_t)min, "%Y %b %e %H:%M" ) << endl;
    cerr << " maxtt:" << Time::toTimeString( (time_t)max, "%Y %b %e %H:%M" ) << endl;
    cerr << " value_incr:" << value_incr << endl;
    
    for( int i=!IncludeMinValue; i<TargetNumberOfAttrs; ++i, value+=value_incr )
    {
        cerr << "generate attr value:" << fixed << value << endl;
        cerr << " valuett:" << Time::toTimeString( (time_t)value, "%Y %b %e %H:%M" ) << endl;
        
        stringstream ss;
        if( isTimeEA )
        {
            ss << attrname << "_" << formalTimeValueForFormalAttribute(tostr(value));
        }
        else
        {
            ss << attrname << "_" << value;
        }
        v[ ss.str() ] = value;
    }
    
    stringstream ss;
    if( isTimeEA )
    {
        ss << attrname << "_" << formalTimeValueForFormalAttribute(tostr(max));
    }
    else
    {
        ss << attrname << "_" << max;
    }
    
    v[ ss.str() ] = max;

    cerr << "total size:" << distance( vbegin, vend ) << "  min:" << fixed << min << " max:" << fixed << max << endl;
    cerr << " mintt:" << Time::toTimeString( (time_t)min, "%Y %b %e %H:%M" ) << endl;
    cerr << " maxtt:" << Time::toTimeString( (time_t)max, "%Y %b %e %H:%M" ) << endl;

    return v;
}



void output( values_t& v, const std::string& attrname, const std::string& ffilterComparitor )
{
    int idx = 0;
    values_t::const_iterator vend = v.end();
    for( values_t::const_iterator vi = v.begin(); vi!=vend; ++vi, ++idx )
    {
        string     s = vi->first;
        double value = vi->second;

        if( isTimeEA )
        {
//            cout << " " << attrname << "_" << formalTimeValueForFormalAttribute(s) << " " << "'";
            cout << " " << formalTimeValueForFormalAttribute(s) << " " << "'";
        }
        else
        {
            cout << " " << s << " " << "'";
        }
        

        if( !idx && ExcludeZero && ReverseOrder )
        {
            value++;
        }
        
        if( !ReverseOrder && ExcludeZero )
        {
            cout <<   "(&(" << attrname << ffilterReverseComparitor << 0 << ")";
        }
        
        cout <<   "(" << attrname << ffilterComparitor << (int)value << ")";
        
        if( !ReverseOrder && ExcludeZero )
        {
            cout << ")";
        }
        

        
        cout <<  "' " << endl;
    }
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose             = 0;
        unsigned long MakeDataDrivenScale = 0;
        unsigned long TargetNumberOfAttrs = 10;
        unsigned long StdDeviations = 0;
        double StdDeviationsIncr = 0.5;
        const char*   findexPath_CSTR    = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "exclude-zero", 'z', POPT_ARG_NONE, &ExcludeZero, 0,
                  "explicitly exclude zero from output ffilters", "" },
                
                { "data-driven-scale", 'd', POPT_ARG_NONE, &MakeDataDrivenScale, 0,
                  "derive the scale biased on the data given in the index", "" },

                { "target-number-of-attributes", 'g', POPT_ARG_INT, &TargetNumberOfAttrs, 10,
                  "desired number of formal attributes to create", "" },
                
                { "reverse", 'r', POPT_ARG_NONE, &ReverseOrder, 0,
                  "reverse the sorting order", "" },

                { "std-deviations", 'S', POPT_ARG_INT, &StdDeviations, 0,
                  "create a scale for the given number of std deviations from mean", "" },

                { "std-deviations-increment", 0, POPT_ARG_DOUBLE, &StdDeviationsIncr, 0.5,
                  "generate a new attribute every x fraction of a standard deviation", "" },
                
                { "include-min-value", 'M', POPT_ARG_NONE, &IncludeMinValue, 0,
                  "include min value aswell, eg. <=0. Use with ferris-create-fca-scale-numeric-ordinal", "" },

                { "is-time-attribute", 'T', POPT_ARG_NONE, &isTimeEA, 0,
                  "handle attribute as a time_t", "" },
                
                
                { "index-path", 'P', POPT_ARG_STRING, &findexPath_CSTR, 0,
                  "path to existing postgresql EA index", "" },

                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* [-P ea-index-url] ");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        EAIndex::fh_idx idx = getEAIndex( findexPath_CSTR );
        string host   = idx->getConfig( CFG_IDX_HOST_K, CFG_IDX_HOST_DEF );
        string dbname = idx->getConfig( CFG_IDX_DBNAME_K, CFG_IDX_DBNAME_DEF );
        if( ReverseOrder )
        {
            ffilterComparitor = ">=";
            ffilterReverseComparitor = "<";
        }
        
        
        stringstream conSS;
        conSS << " host=" << host;
        conSS << " dbname=" << dbname;
        connection con( conSS.str() );
        work trans( con, "getting the schema..." );

        const char* CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K
        = CFG_EAIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K;
        
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string attrname = RootNameCSTR;
            string ValueTableName = guessLookupTableName( trans, attrname );
            fh_stringstream selectss;
            string selectAttrName = attrname;
            string fromClause = " from docmap ";

            stringmap_t m_ExtraColumnsToInlineInDocmap;
            m_ExtraColumnsToInlineInDocmap =
                Util::ParseKeyValueString(
                    idx->getConfig( CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                                    CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT )
                    );
            cerr << "m_ExtraColumnsToInlineInDocmap.size:" << m_ExtraColumnsToInlineInDocmap.size() << endl;
            
            bool isNormalized
                = m_ExtraColumnsToInlineInDocmap.find( attrname ) == m_ExtraColumnsToInlineInDocmap.end();
            
            if( isNormalized )
            {
                selectAttrName = "attrvalue";
                stringstream ss;
//                 ss << " from docattrs da, "
//                    << ValueTableName << " l, attrmap an " << endl
//                    << " where an.attrname = '" << attrname
//                    << "' and an.attrid = da.attrid and da.vid = l.vid ";
                ss << " from docattrs da, "
                   << ValueTableName << " l " << endl
                   << " where da.attrid = "
                   << "    (select max(attrid) from attrmap where attrname = '" << attrname << "') "
                   << "  and da.vid = l.vid ";
                
                fromClause = ss.str();
            }
            
            
            
            if( StdDeviations )
            {
                selectss << "SELECT "
                         << " avg(" << selectAttrName << ") as avg,"
                         << " stddev(" << selectAttrName << ") as stddev,"
                         << " min(" << selectAttrName << ") as mi,"
                         << " max(" << selectAttrName << ") as mx "
                         << fromClause << endl;
                cerr << "SQL:" << tostr(selectss) << endl;
                result res = trans.exec( tostr( selectss ) );
                double mean = 0;
                double stddev = 0;
                double min = 0;
                double max = 0;
                
                for (result::const_iterator c = res.begin(); c != res.end(); ++c)
                {
                    c[0].to(mean);
                    c[1].to(stddev);
                    c[2].to(min);
                    c[3].to(max);
                }
                cerr << "mean:" << mean << " stddev:" << stddev << " min:" << min << " max:" << max << endl;
                
                bool haveOverflowedMax = 0;
                values_t values;
                values[ attrname + "_mean" ] = mean;
                for( double i = StdDeviationsIncr; i <= StdDeviations; i+=StdDeviationsIncr )
                {
                    stringstream pattrss;
                    stringstream nattrss;
                    pattrss << attrname << "_p" << i;
                    nattrss << attrname << "_n" << i;
                    int p = ((int)(mean + i*stddev));
                    int n = ((int)(mean - i*stddev));
                    
                    if( !haveOverflowedMax )
                    {
                        if( max && p > max )
                        {
                            haveOverflowedMax = 1;
                            p = (int)max;
                        }

                        values[ pattrss.str() ] = p;
                    }
                    
                    if( min && p > min )
                        values[ nattrss.str() ] = n;
                }
                output( values, attrname, ffilterComparitor );
                
                continue;
            }

            if( isTimeEA )
            {
                stringstream ss;
                ss << "EXTRACT(EPOCH FROM " << selectAttrName << " )";
                selectAttrName = ss.str();
            }
            
            selectss << "SELECT distinct(" << selectAttrName << ") "
                     << fromClause << endl;
            cerr << "SQL: " << tostr(selectss) << endl;
            result res = trans.exec( tostr( selectss ) );
            values_t values;
            cerr << "res.sz: " << res.size() << endl;

            for (result::const_iterator c = res.begin(); c != res.end(); ++c)
            {
                stringstream rdnss;
                if( isTimeEA )
                {
                    time_t tt = 0;
                    c[0].to(tt);
                    rdnss << attrname << "_" << formalTimeValueForFormalAttribute( tostr( tt ) );
                    cerr << "rdn:" << tostr(rdnss) << endl;
                    values.insert( make_pair( rdnss.str(), tt ) );
                }
                else
                {
                    double v = 0;
                    c[0].to(v);
                    rdnss << attrname << "_" << v;
                    values.insert( make_pair( rdnss.str(), v ) );
                }
                
            }

            cerr << "values.sz:" << values.size() << endl;
            
            
            if( !values.empty() )
            {
                values_t newValues;
                // ordinal scale.
                if( ReverseOrder )
                    performOrdinalScale( newValues,
                                         values.rbegin(), values.rend(), values.size(),
                                         TargetNumberOfAttrs, MakeDataDrivenScale, attrname );
                else
                    performOrdinalScale( newValues,
                                         values.begin(), values.end(), values.size(),
                                         TargetNumberOfAttrs, MakeDataDrivenScale, attrname );
                output( newValues, attrname, ffilterComparitor );
            }
        }
    }
    catch( exception& e )
    {
        LG_FCA_ER << PROGRAM_NAME << " cought exception:" << e.what() << endl;
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


