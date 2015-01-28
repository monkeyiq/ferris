/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001-2003 Ben Martin

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

    $Id: AgentSVMLight.cpp,v 1.3 2010/09/24 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"

#include <Ferris/Agent.hh>
#include <Ferris/Agent_private.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Runner.hh>
#include <Ferris/FullTextIndexer.hh>
#include <Ferris/Numeric.hh>
#include <Indexing/IndexPrivate.hh>

using namespace std;
using namespace Ferris::FullTextIndex;
using namespace STLdb4;


namespace Ferris
{
    
    namespace AI
    {

        typedef map< termid_t, double > FeatureSet_t;
        
        class SvmLight_BinaryClassifierAgentImplemenation;
        FERRIS_SMARTPTR( SvmLight_BinaryClassifierAgentImplemenation, fh_svmLightAgentImpl );
        
        class FERRISEXP_DLLLOCAL SvmLight_BinaryClassifierAgentTrainerImplemenation
            :
            public BinaryClassifierAgentTrainerImplemenation
        {
            fh_svmLightAgentImpl m_agent;

            
            bool m_dirty;
            FeatureSet_t m_TotalTF; // Total number of times each term appears.
            
            struct TrainCase
            {
                double desiredOutCome;
                FeatureSet_t m_features;

                TrainCase( double desiredOutCome )
                    :
                    desiredOutCome( desiredOutCome )
                    {
                    };
            };
            typedef list< pair< string, TrainCase > > m_cases_t;
            m_cases_t m_cases;

            
        public:

            SvmLight_BinaryClassifierAgentTrainerImplemenation( fh_svmLightAgentImpl agent );
            virtual void addCase( fh_context c, double desiredOutCome );
            virtual void removeCase( fh_context c );
            virtual void train();
            virtual void sync();
            void load();
        };
        

        /**
         * This is a binary classifier using support vector models and in particular
         * the svm_light library by Thorsten Joachims to provide the core of the SVM logic.
         *
         * A big thanks to Thorsten Joachims for putting up with my pesky
         * questions about how to feed a SVM machine :) Hopefully most of
         * the little assumptions I made to avoid pestering Thorsten are
         * correct enough until I get the SVM text classification book.
         */
        class FERRISEXP_DLLLOCAL SvmLight_BinaryClassifierAgentImplemenation
            :
            public BinaryClassifierAgentImplemenation,
            public PathManager
        {
            typedef BinaryClassifierAgentImplemenation _Base;

            friend class SvmLight_BinaryClassifierAgentTrainerImplemenation;
            
            fh_lexicon   m_lex;
            fh_database  m_db;
            FeatureSet_t m_overallCounts; // contains the IDF for each term known.
            termid_t     m_highestTermID;

            void save();
            void load();
            
        public:
            SvmLight_BinaryClassifierAgentImplemenation();
            ~SvmLight_BinaryClassifierAgentImplemenation();
            virtual void setStateDir( const std::string& s );
            
            fh_database getDB();
            void sync();

            void convertTermFrequencyToIDFValues( int N, FeatureSet_t& features );
            void convertIDFToTermFrequencyValues( int N, FeatureSet_t& features );
            FeatureSet_t& scaleTermFreqFeatureSetByIDFSet( FeatureSet_t& input, FeatureSet_t& scale );
            FeatureSet_t& normalize( FeatureSet_t& input );
            

            typedef map< string, termid_t > m_getTermID_Cache_t;
            m_getTermID_Cache_t m_getTermID_Cache;
            termid_t getTermID( const std::string& term, bool CreateIfDoesntExist = false );
            string getToken( fh_istream& iss );
            FeatureSet_t& calculateTermFreq( fh_context c,
                                             FeatureSet_t& s,
                                             FeatureSet_t& overallCounts,
                                             bool CreateIfDoesntExist );


            double readSerializedFeatureSet(  fh_istream& oss, FeatureSet_t& features );
            void   writeSerializedFeatureSet( fh_ostream& oss, double choice, FeatureSet_t& features );

            fh_bTrainerImpl getTrainer();
            virtual double classify( fh_context c );

            
            string    getModelFileName();
            string    getCommandName();
            fh_runner getRunner();
            std::string getImplementationName()
                {
                    return "svm_light";
                }

            // PathManager
            virtual std::string getBasePath()
                {
                    return getStateDir();
                }
            
            virtual std::string getConfig( const std::string& k, const std::string& def,
                                   bool throw_for_errors = false )
                {
                    return def;
                }
            virtual void setConfig( const std::string& k, const std::string& v )
                {}
            
        };
        static bool
        SvmLight_BinaryClassifierAgentImplemenation_Registered =
        RegisterBinaryClassifierAgentImplemenationFactory( 
//        BinaryClassifierAgentImplemenationFactory::Instance().Register(
            "svm_light",
            &MakeObject< BinaryClassifierAgentImplemenation, SvmLight_BinaryClassifierAgentImplemenation >::Create );

        /********************/
        /********************/
        /********************/


        SvmLight_BinaryClassifierAgentTrainerImplemenation::SvmLight_BinaryClassifierAgentTrainerImplemenation(
            fh_svmLightAgentImpl agent )
            :
            m_agent( agent ),
            m_dirty( false )
        {
            load();
        }

        void
        SvmLight_BinaryClassifierAgentTrainerImplemenation::addCase( fh_context c, double desiredOutCome )
        {
            m_dirty = true;

            string earl = c->getURL();
            m_cases.push_back( make_pair( earl, TrainCase( desiredOutCome ) ));
            
            TrainCase&            d = m_cases.back().second;
            double          outcome = d.desiredOutCome;
            FeatureSet_t&  features = d.m_features;
            int   numberOfDocuments = m_cases.size();
            
            m_agent->calculateTermFreq( c, features, m_TotalTF, true );

//             m_agent->convertTermFrequencyToIDFValues( numberOfDocuments, m_agent->m_overallCounts );
//             m_agent->scaleTermFreqFeatureSetByIDFSet( d.m_features, overallCounts );
//             m_agent->normalize( d.m_features );
            
        }

        template< class T >
        struct firstEquals
        {
            typedef typename T::first_type V;
            
            V v;
            firstEquals( V& v ) : v(v) 
                {}
            
            bool operator()( T& x )
                {
                    return v == x.first;
                }
        };
        
        
        void
        SvmLight_BinaryClassifierAgentTrainerImplemenation::removeCase( fh_context c )
        {
            m_dirty = true;
            
            string earl = c->getURL();
            m_cases_t::iterator ci = find_if( m_cases.begin(),
                                              m_cases.end(),
                                              firstEquals< pair< string, TrainCase > >(earl) );
            if( ci != m_cases.end() )
            {
                //
                // We have to remove its effect from the TotalTF and IDF figures.
                //
                TrainCase&            d = ci->second;
                FeatureSet_t&  features = d.m_features;

                for( FeatureSet_t::iterator fi = features.begin(); fi!=features.end(); ++fi )
                {
                    m_TotalTF[ fi->first ] -= fi->second;
                }

                //
                // And now we can forget about this test case. sync() will remove it for us.
                //
                m_cases.erase( ci );
            }
        }


        void
        SvmLight_BinaryClassifierAgentTrainerImplemenation::load()
        {
//             cerr << "Reading the total TF FeatureSet from:"
//                  << m_agent->getStateDir() + "/ferris_total_tf"
//                  << endl;
            
            {
                string FileName = m_agent->getStateDir() + "/ferris_total_tf";
                fh_ifstream iss( FileName );
                m_agent->readSerializedFeatureSet( iss, m_TotalTF );
//                cerr << "m_TotalTF.size:" << m_TotalTF.size() << endl;
            }

            {
                string FileListFileName = m_agent->getStateDir() + "/ferris_training_filelist";
                string FeaturesFileName = m_agent->getStateDir() + "/ferris_training_features";
                fh_ifstream iss_fname( FileListFileName );
                fh_ifstream iss_features( FeaturesFileName );
                string s;
                
                while( getline( iss_fname, s ))
                {
                    double desiredOutCome = 0;
                    m_cases.push_back( make_pair( s, TrainCase( desiredOutCome ) ));
                    TrainCase& d = m_cases.back().second;
                    d.desiredOutCome = m_agent->readSerializedFeatureSet( iss_features, d.m_features );

//                    cerr << "Loaded " << d.m_features.size() << " features for:" << s << endl;
                }
            }
        }
        
        void
        SvmLight_BinaryClassifierAgentTrainerImplemenation::sync()
        {
            if( m_dirty )
            {
                m_dirty = false;
                
                {
                    string FileName = m_agent->getStateDir() + "/ferris_total_tf";
                    fh_ofstream oss( FileName );
                    m_agent->writeSerializedFeatureSet( oss, 0, m_TotalTF );
                    oss << flush;
                }

                
                {
                    string FileListFileName = m_agent->getStateDir() + "/ferris_training_filelist";
                    string FeaturesFileName = m_agent->getStateDir() + "/ferris_training_features";
                    fh_ofstream oss_fname( FileListFileName );
                    fh_ofstream oss_features( FeaturesFileName );
                
                    for( m_cases_t::iterator ci = m_cases.begin(); ci!=m_cases.end(); ++ci )
                    {
                        TrainCase& d = ci->second;
                    
                        oss_fname << ci->first << endl;
                        m_agent->writeSerializedFeatureSet( oss_features,
                                                            d.desiredOutCome,
                                                            d.m_features );
                    }
                    oss_fname << flush;
                    oss_features << flush;
                }
                
                
            }
        }
        
        void
        SvmLight_BinaryClassifierAgentTrainerImplemenation::train()
        {
            int numberOfDocuments = m_cases.size();
            FeatureSet_t& overallCounts = m_agent->m_overallCounts;
            overallCounts.clear();
            overallCounts = m_TotalTF;
            m_agent->convertTermFrequencyToIDFValues( numberOfDocuments, overallCounts );
            m_agent->sync();
            

            //
            // We want to scale the FeatureSet TF counts by the IDF and then
            // normalize them for svm_learn but we then want to throw away
            // that scaled data and go back to having just the TF FeatureSet values
            //
            sync();
            
            
            
//             int numberOfDocuments = m_cases.size();
//             FeatureSet_t& overallCounts = m_agent->m_overallCounts;
//             overallCounts.clear();
            
//             for( m_cases_t::iterator ci = m_cases.begin(); ci!=m_cases.end(); ++ci )
//             {
//                 fh_context   c = ci->first;
//                 TrainCase&   d = ci->second;
//                 double outcome = d.desiredOutCome;

//                 cerr << "training on, outcome:" << outcome << " c:" << c->getURL() << endl;

//                 m_agent->calculateTermFreq( c, d.m_features, overallCounts, true );
//             }
//             m_agent->convertTermFrequencyToIDFValues( numberOfDocuments, overallCounts );

//             // sync all lexicon/IDF changes here while debugging.
//             m_agent->sync();

            for( m_cases_t::iterator ci = m_cases.begin(); ci!=m_cases.end(); ++ci )
            {
                TrainCase&   d = ci->second;
                m_agent->scaleTermFreqFeatureSetByIDFSet( d.m_features, overallCounts );
                m_agent->normalize( d.m_features );
            }

            
            //
            // We now have the feature set which is the TF * IDF for each term in each document.
            //
            // For the first attempt we will create an input file that is in the format that
            // the svm_learn command line tool is expecting.
            // This gives the ability to debug the values while developing and the time critical
            // case is by far the svm_classify() case rather than training.
            //
            string trainingFileName = m_agent->getStateDir() + "/tmp_svm_training_input";
            fh_ofstream oss( trainingFileName );
//            cerr << "Number of training cases:" << m_cases.size() << endl;
            
            for( m_cases_t::iterator ci = m_cases.begin(); ci!=m_cases.end(); ++ci )
            {
                TrainCase&   d = ci->second;
                m_agent->writeSerializedFeatureSet( oss, d.desiredOutCome, d.m_features );
            }
            oss << flush;

            //
            // Throw away the scaled and normailized FeatureSets for the files
            //
            load();
            
            
            fh_stringstream ss;
            ss << "svm_learn ";
            ss << " " << trainingFileName << " ";
            ss << m_agent->getModelFileName();

//            cerr << "SVM CMD:" << tostr(ss) << endl;
            
            fh_runner r = m_agent->getRunner();
            r->setCommandLine( tostr(ss) );
            r->Run();
            gint e = r->getExitStatus();

//            cerr << "SVM e:" << e << " for CMD:" << tostr(ss) << endl;

            sync();
            m_agent->sync();
            m_cases.clear();
        }

        /********************/
        /********************/
        /********************/

//        const char* SVMLIGHT_LEXICON_CLASS_DEFAULT = "FrontCodedBlocks (3-in-4)";
        const char* SVMLIGHT_LEXICON_CLASS_DEFAULT = "XML";
        const char* SVMLIGHT_CFG_NEXT_TID_KEY      = "config-next-termid";
        
        SvmLight_BinaryClassifierAgentImplemenation::SvmLight_BinaryClassifierAgentImplemenation()
            :
            m_lex( 0 ),
            m_highestTermID( 0 )
        {
        }
        
        SvmLight_BinaryClassifierAgentImplemenation::~SvmLight_BinaryClassifierAgentImplemenation()
        {
        }

        void
        SvmLight_BinaryClassifierAgentImplemenation::setStateDir( const std::string& s )
        {
            _Base::setStateDir( s );
            m_lex = LexiconFactory::Instance().CreateObject( SVMLIGHT_LEXICON_CLASS_DEFAULT );
            m_lex->setPathManager( this );
            m_lex->setIndex( 0 );
        }
        

        /**
         * Have to save and load the IDF FeatureSet ourself :(
         */
        void
        SvmLight_BinaryClassifierAgentImplemenation::save()
        {
            fh_ofstream oss( getStateDir() + "/idf-vector" );
            writeSerializedFeatureSet( oss, 1, m_overallCounts );
        }

        /**
         * Have to save and load the IDF FeatureSet ourself :(
         */
        void
        SvmLight_BinaryClassifierAgentImplemenation::load()
        {
            fh_ifstream iss( getStateDir() + "/idf-vector" );
            readSerializedFeatureSet( iss, m_overallCounts );
        }
        
        
        void
        SvmLight_BinaryClassifierAgentImplemenation::sync()
        {
            save();
            
            if( m_lex ) 
                m_lex->sync();

            if( m_highestTermID )
            {
                getDB()->set( SVMLIGHT_CFG_NEXT_TID_KEY, tostr(m_highestTermID) );
            }
            
            if( m_db )
                m_db->sync();
        }
        

        fh_database
        SvmLight_BinaryClassifierAgentImplemenation::getDB()
        {
            if( !m_db )
            {
                try
                {
                    m_db = new Database( getStateDir() + "/ferris-svm-config.db" );
                }
                catch( dbNonExist& e )
                {
//                    cerr << "Creating database..." << endl;
                    m_db = new Database( DB_HASH, getStateDir() + "/ferris-svm-config.db" );
                    m_db->set( SVMLIGHT_CFG_NEXT_TID_KEY, "0" );
                    m_db->sync();
//                    cerr << "Created db" << endl;
                }
            }
            return m_db;
        }
        

        string
        SvmLight_BinaryClassifierAgentImplemenation::getModelFileName()
        {
            return getStateDir() + "/" + "svm_light_model";
        }
        
        string
        SvmLight_BinaryClassifierAgentImplemenation::getCommandName()
        {
            return "svm_classify";
        }
        
        fh_bTrainerImpl
        SvmLight_BinaryClassifierAgentImplemenation::getTrainer()
        {
            return new SvmLight_BinaryClassifierAgentTrainerImplemenation( this );
        }

        /**
         * Get or create a new termid for the given term string. the Mapping is
         * persistent of string -> ID
         *
         * @param CreateIfDoesntExist if true then create a term if its not already there.
         *        Its redundant to create new termIDs when classifying because the feature
         *        can't match what we have learned on.
         */
        termid_t
        SvmLight_BinaryClassifierAgentImplemenation::getTermID(
            const std::string& term,
            bool CreateIfDoesntExist )
        {
            m_getTermID_Cache_t::iterator ci = m_getTermID_Cache.find( term );
            if( ci != m_getTermID_Cache.end() )
            {
                return ci->second;
            }
            
            termid_t tid = m_lex->lookup( term );
            
            if( CreateIfDoesntExist && !tid )
            {
                fh_database db = getDB();
                string tids = "1";

                if( m_highestTermID )
                {
                    ++m_highestTermID;
                    tid = m_highestTermID;
                }
                else
                {
//                    cerr << "getting next tid..." << endl;
                    db->get( SVMLIGHT_CFG_NEXT_TID_KEY, tids );
//                    cerr << "getting next tid...2" << endl;
                    tid = toType<termid_t>( tids ) + 1;
                    m_highestTermID = tid;
//                    db->set( SVMLIGHT_CFG_NEXT_TID_KEY, tostr(tid) );
                }
                
                m_lex->insert( term, tid );
            }
            m_getTermID_Cache.insert( make_pair( term, tid ));
            return tid;
        }
        
        /**
         * Get a token from the stream that us suitable as a feature
         *
         * @see getTermID() for how to turn the term returned into a unique ID
         *      suitable for use as a feature label for svm_light
         */
        string
        SvmLight_BinaryClassifierAgentImplemenation::getToken( fh_istream& iss )
        {
            string s;
            if( iss >> s && !iss.eof() )
            {
                return s;
            }
            return "";
        }

        double
        SvmLight_BinaryClassifierAgentImplemenation::readSerializedFeatureSet(
            fh_istream& user_iss, FeatureSet_t& features )
        {
            double choice = 5.0;
            
            string line;
            if( getline( user_iss, line ) )
            {
                fh_stringstream iss;
                iss << line;
                
                iss >> choice;

                while( iss )
                {
                    termid_t tid = 0;
                    char colon   = ':';
                    double   val = 0;

                    if( iss >> tid && iss >> colon && iss >> val )
                    {
                        features[ tid ] = val;
                    }
                }
            }
            
//            cerr << "readSerializedFeatureSet() size:" << features.size() << endl;
            
            return choice;
        }

        
        /**
         * This takes a choice and a feature set and writes a serialized version
         * to the given stream in a format that the svm_light 5.00 commands understands
         */
        void
        SvmLight_BinaryClassifierAgentImplemenation::writeSerializedFeatureSet(
            fh_ostream& oss,
            double choice,
            FeatureSet_t& fs )
        {
            int    svmchoice = 0;
                
            if( choice < MedallionBelief::SURENESS_NULL )
                svmchoice = -1;
            if( choice > MedallionBelief::SURENESS_NULL )
                svmchoice =  1;
                
            // -1 1:0.43 3:0.12 9284:0.2
            oss << svmchoice << " ";
            for( FeatureSet_t::iterator iter = fs.begin(); iter != fs.end(); ++iter )
            {
                oss << iter->first << ":" << iter->second << " ";
            }
            oss << endl;
        }
        

        /**
         * This generates a featureSet for the given context which has as its values
         * the r_d_t which is 1 + ln( f_d_t );
         * the r_d_t is also known as the TF or TermFrequency.
         *
         * The total number of times each term occurs is accumulated in overallCounts
         * for use in generating the IDF using convertTermFrequencyToIDFValues()
         *
         * @param CreateIfDoesntExist If a term is found in the contents of 'c' that is not
         *        in the lexicon should it be added to the lexicon and feature set.
         */
        FeatureSet_t&
        SvmLight_BinaryClassifierAgentImplemenation::calculateTermFreq( fh_context c,
                                                                        FeatureSet_t& features,
                                                                        FeatureSet_t& overallCounts,
                                                                        bool CreateIfDoesntExist )
        {
            features.clear();
            bool isCaseSensitive = true;

            
            fh_istream iss;
            try
            {
                fh_attribute a = c->getAttribute( "as-text" );
                iss = a->getIStream();
            }
            catch( exception& e )
            {
                iss = c->getIStream();
            }
            
//             {
//                 Time::Benchmark frbm( "Time to read in c:" + c->getURL() );
//                 frbm.start();
//                 for( string s; !(s = getToken( iss )).empty(); )
//                 {
//                     if( !isCaseSensitive )
//                     {
//                         s = foldcase( s );
// //                    cerr << "case folded:" << s << endl;
//                     }
//                 }
//                 frbm.print();
//             }
//             iss = c->getIStream();
            
            
            for( string s; !(s = getToken( iss )).empty(); )
            {
                if( !isCaseSensitive )
                {
                    s = foldcase( s );
//                    cerr << "case folded:" << s << endl;
                }

                termid_t tid = getTermID( s, true ); //, CreateIfDoesntExist );
//                cerr << "calculateTermFreq() s:" << s << " tid:" << tid << endl;
                if( tid )
                {
                    features[ tid ] += 1;
                    overallCounts[ tid ] += 1;
                }
            }

            for( FeatureSet_t::iterator iter = features.begin(); iter != features.end(); ++iter )
            {
                double f_d_t = iter->second;
                double r_d_t  = 1.0 + log( f_d_t );

                iter->second = r_d_t;
            }

            return features;
        }

        /**
         * This assumes that "features" has been updated to contain the total number of times
         * a term occurs in all the training data.
         *
         * The result is a map from the term to the IDF (w_t).
         *
         * w_t = ln( 1 + N / f_t );
         *
         * @param N number of documents
         */
        void
        SvmLight_BinaryClassifierAgentImplemenation::convertTermFrequencyToIDFValues(
            int N,
            FeatureSet_t& features )
        {
            for( FeatureSet_t::iterator iter = features.begin(); iter != features.end(); ++iter )
            {
                double f_t = iter->second;
//                double w_t = log( 1.0 + N / f_t );
                double w_t = 1.0 / f_t;
                iter->second = w_t;
            }
        }

        /**
         * Inverse operation of convertTermFrequencyToIDFValues().
         *
         * For a document count N and TF FeatureSet Z
         * convertTermFrequencyToIDFValues( N, Z );
         * convertIDFToTermFrequencyValues( N, Z );
         * should give back the original Z FeatureSet.
         */
        void
        SvmLight_BinaryClassifierAgentImplemenation::convertIDFToTermFrequencyValues(
            int N,
            FeatureSet_t& features )
        {
            for( FeatureSet_t::iterator iter = features.begin(); iter != features.end(); ++iter )
            {
                double w_t = iter->second;
//                double f_t = ( exp( w_t ) - 1.0 ) / N;
                double f_t = 1.0 / w_t;
                iter->second = f_t;
            }
        }
        
        

        /**
         * This is ment to take a FeatureSet that continas the TF value and a FeatureSet
         * that contains the IDF (as scale) and modify the input featureSet to become TF * IDF
         * for each term.
         */
        FeatureSet_t&
        SvmLight_BinaryClassifierAgentImplemenation::scaleTermFreqFeatureSetByIDFSet(
            FeatureSet_t& input, FeatureSet_t& scale )
        {
            for( FeatureSet_t::iterator iter = input.begin(); iter != input.end(); ++iter )
            {
                double tf = iter->second;
                double w_d_t = tf * scale[ iter->first ];
                iter->second = tf;
            }
        }

        template <class _Tp>
        struct SumSquared : public binary_function<double,double,_Tp>
        {
            double operator()( double result, const _Tp& y) const
                {
                    return result + pow( y.second , 2.0 );
                }
        };

        template<class T>
        struct ScaleByValue : public unary_function<T, void>
        {
            double scale;
            ScaleByValue( double scale ) : scale( scale ) {}
            
            void operator() ( T& x )
                {
                    x.second = x.second * scale;
                }
        };

        /**
         * Given the vector x=(x_1,...,x_N) from TF * IDF scores, then you compute 
         * the normalized vectors xx=(xx_1,...,xx_N) as
         *
         * xx_i = x_i/\sqrt(\sum(x_i^2))
         * (in Latex notation).
         *
         */
        FeatureSet_t&
        SvmLight_BinaryClassifierAgentImplemenation::normalize( FeatureSet_t& input )
        {
            double SumOfSquares = accumulate( input.begin(), input.end(),
                                              0.0, SumSquared<FeatureSet_t::value_type>() );
            for_each( input.begin(), input.end(),
                      ScaleByValue<FeatureSet_t::value_type>( 1.0 / sqrt( SumOfSquares ) ));
        }
        
        double
        SvmLight_BinaryClassifierAgentImplemenation::classify( fh_context c )
        {
            string exampleFeatureSetFileName = getStateDir() + "/svm_files_feature_set";
            string outputFileName = getStateDir() + "/svm_classification_output";

            
            
            fh_stringstream ss;
            ss << getCommandName() << " ";
            ss << " " << exampleFeatureSetFileName << " ";
            ss << " " << getModelFileName() << " ";
            ss << " " << outputFileName << " ";

            //
            // FIXME: What do we do about the IDF? At current I am taking the TotalTF
            // vector and adjusting it for the new file before creating a new IDF vector.
            // This relies on IDF = 1 / TF. ie. the IDF calculation not needing the
            // number of documents in total (N). Should be easy to change this as the
            // trainer knows what N is aswell as storing TotalTF.
            //
            
            FeatureSet_t localIDF = m_overallCounts;
            convertIDFToTermFrequencyValues( 0, localIDF );
            
            FeatureSet_t features;
//            cerr << "generating feature set for file c:" << c->getURL() << endl;
            calculateTermFreq( c, features, localIDF, false );


//            cerr << "file c:" << c->getURL() << " has feature count:" << features.size() << endl;
            
            convertTermFrequencyToIDFValues( 0, localIDF );
            scaleTermFreqFeatureSetByIDFSet( features, localIDF );
            normalize( features );

            {
                fh_ofstream oss( exampleFeatureSetFileName );
                writeSerializedFeatureSet( oss, 1, features );
                oss << flush;
            }
            
//            cerr << "SVM CMD:" << tostr(ss) << endl;
            
            fh_runner r = getRunner();
            r->setCommandLine( tostr(ss) );
            r->Run();
            gint e = r->getExitStatus();

            double ret = 0;

            if( !e )
            {
                fh_ifstream iss( outputFileName );
                double reply = 0;
                if( iss >> reply )
                {
                    ret = reply * 100.0;
                    ret = Numeric::clamp( ret, -100.0, 100.0 );
                }
            }

//            cerr << "SVM e:" << e << " ret:" << ret << " for CMD:" << tostr(ss) << endl;
            
            return ret;
        }

        fh_runner
        SvmLight_BinaryClassifierAgentImplemenation::getRunner()
        {
            fh_runner r = new Runner();

            r->setSpawnFlags(
                GSpawnFlags( G_SPAWN_SEARCH_PATH |
                             G_SPAWN_DO_NOT_REAP_CHILD |
                             G_SPAWN_STDOUT_TO_DEV_NULL |
                             G_SPAWN_STDERR_TO_DEV_NULL |
                             r->getSpawnFlags()));
            return r;
        }

        
    };
};
