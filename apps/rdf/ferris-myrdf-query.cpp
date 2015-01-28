/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-myrdf-query
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

    $Id: ferris-myrdf-query.cpp,v 1.6 2010/09/24 21:31:19 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * 
*/


#include <Ferris.hh>
#include <FerrisDOM.hh>
#include <FerrisRDFCore.hh>
#include <popt.h>

#include <Configuration_private.hh>
#include <EAIndexer_private.hh>
#include <STLdb4/stldb4.hh>
using namespace ::STLdb4;

using namespace std;
using namespace Ferris;
using namespace Ferris::RDFCore;

const string PROGRAM_NAME = "ferris-myrdf-query";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

unsigned long Verbose             = 0;
unsigned long XMLOutput           = 0;
const char* RedlandEAIndexURLCSTR = 0;

int main( int argc, char** argv )
{
    int exit_status = 0;

    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "rdf-eaindex-url", 'P', POPT_ARG_STRING, &RedlandEAIndexURLCSTR, 0,
                  "use rdf ea index model at given location", "" },

                { "xml", 'x', POPT_ARG_NONE, &XMLOutput, 0,
                  "show results in XML format", "" },
                
//                 { "trunc", 'T', POPT_ARG_NONE, &Truncate, 0,
//                   "truncate output file", "" },

//                 { "ea", 0, POPT_ARG_STRING, &EAName_CSTR, 0,
//                   "put redirection into this EA", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* queryfile");

        /* Now do options processing */
        {
            int c=-1;
            while ((c = poptGetNextOpt(optCon)) >= 0)
            {}
        }
        

        if (argc < 2)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        
        fh_istream queryiss = Factory::fcin();
        
        stringlist_t srcs;
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string srcURL = tmpCSTR;
            srcs.push_back( srcURL );
        }

        if( srcs.empty() )
        {
        }
        else
        {
            stringlist_t::iterator si = srcs.begin();
            if( *si == "-" )
            {
            }
            else
            {
                fh_context c = Resolve( *si );
                queryiss = c->getIStream();
            }
        }

        string query = StreamToString( queryiss );

        if( Verbose )
        {
            cerr << "query:" << query << endl;
            cerr << "----------------------------------------" << endl;
            cerr << "bindings..." << endl;
        }
        
        fh_model m = getDefaultFerrisModel();

        if( RedlandEAIndexURLCSTR )
        {
            // static const char* CFG_IDX_DBNAME_K = "cfg-idx-dbname";
            // static const char* CFG_IDX_DBNAME_DEF = "ferriseaindex";
            // static const char* CFG_IDX_DBOPTIONS_K = "cfg-idx-dboptions";
            // static const char* CFG_IDX_DBOPTIONS_DEF = "hash-type='bdb',contexts='yes',index-predicates='yes',";
            // static const char* CFG_IDX_STORAGENAME_K = "cfg-idx-storage-name";
            // static const char* CFG_IDX_STORAGENAME_DEF = "hashes";

            // string dbfilename = CleanupURL( (string)RedlandEAIndexURLCSTR
            //                                 + "/" + EAIndex::DB_EAINDEX );
            // fh_database db = new Database( dbfilename );
            
            // string storage_name = get_db4_string( db, CFG_IDX_STORAGENAME_K, "", true );
            // string db_name      = get_db4_string( db, CFG_IDX_DBNAME_K,      "", true );
            // string db_options   = get_db4_string( db, CFG_IDX_DBOPTIONS_K,   "", true );

            // m = Model::ObtainDB( storage_name, db_name, db_options );

            m = Model::FromMetadataContext( (string)RedlandEAIndexURLCSTR + "/metadata" );
        }

        if( XMLOutput )
        {
            vector<string> names;
            fh_domdoc dom = Factory::makeDOM( "results" );
            DOMElement* root = dom->getDocumentElement();
            
            BindingsIterator iter = m->findBindings( query );
            BindingsIterator e;
            if( iter != e )
            {
                stringlist_t sl = iter.bindingNames();
                copy( sl.begin(), sl.end(), back_inserter( names ) );
            }
        
            for( ; iter != e ; ++iter )
            {
                DOMElement* el = XML::createElement( dom, root, "binding" );

                for( int i=0; i < iter.bindingCount(); ++i )
                {
                    string n = names[i];
                    DOMElement* elchild = XML::createElement( dom, el, n );
                    fh_node rnode = iter[ i ];
                    if( rnode->isResource() )
                    {
                        XML::setChildText( dom, elchild, rnode->getURI()->toString() );
                    }
                    else
                    {
                        XML::setChildText( dom, elchild, rnode->toString() );
                    }
                }
            }
            
            
            fh_stringstream dss = tostream( dom, true );
            cout << StreamToString( dss );
        }
        else
        {
            BindingsIterator iter = m->findBindings( query );
            BindingsIterator e;
            if( iter != e )
            {
                stringlist_t sl = iter.bindingNames();
                copy( sl.begin(), sl.end(),
                      ostream_iterator<string>(cerr,"\n"));
            }
        
            for( ; iter != e ; ++iter )
            {
                cerr << "------------------------------------------------------------" << endl;
                cerr << "Result...sz:" << iter.bindingCount() << endl;
                for( int i=0; i < iter.bindingCount(); ++i )
                    cerr << " value:" << iter[ i ]->toString() << endl;
            }
        }
        
        


        
//         librdf_world* W = getWorld()->getRAW();
//         librdf_model* rawm = m->getRAW();
//         librdf_query* q = librdf_new_query( W,
//                                             "sparql", 0,
//                                             (const unsigned char*)query.c_str(), 0 );
//         librdf_query_results* qr = librdf_model_query_execute( rawm, q );
//         librdf_stream* stream = librdf_query_results_as_stream( qr );
//         cerr << " q:" << q
//              << " qr:" << qr
//              << " qr.count:" << librdf_query_results_get_count( qr )
//              << " qr.bc:" << librdf_query_results_get_bindings_count( qr )
//              << " stream:" << stream
//              << endl;
//         static fh_node format = Node::CreateURI( "http://www.w3.org/TR/2004/WD-rdf-sparql-XMLres-20041221/" );
// //        cerr << "R2Strieng:" <<  librdf_query_results_to_string( qr, format->getURI()->getRAW(), 0 ) << endl;
        

//         if( librdf_query_results_is_bindings ( qr ) )
//         {
//             int bc = librdf_query_results_get_bindings_count( qr );

//             for( int i=0; i<bc; ++i )
//             {
//                 const char* bname = librdf_query_results_get_binding_name( qr, i );
//                 librdf_node* n = librdf_query_results_get_binding_value( qr, i );

//                 cerr << "bname:" << bname << endl;
//                 if( n )
//                     cerr << "    v:" << librdf_node_to_string( n ) << endl;
//             }
            
//         }
        
}
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


