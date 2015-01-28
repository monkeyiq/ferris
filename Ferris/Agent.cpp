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

    $Id: Agent.cpp,v 1.2 2010/09/24 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"

#include <Ferris/Agent.hh>
#include <Ferris/Agent_private.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Runner.hh>

using namespace std;

namespace Ferris
{
    namespace AI
    {
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * Synthetic agent that calls all the other agents
         */
        static const string COMPLETE_AGENT_UUID = "complete agent id";
        static const string COMPLETE_AGENT_NAME = "complete agent";

        static const string AI_AGENT_UUIDLIST_KEY = "ai_all_agent_uuid_list";
        static const string AI_AGENT_UUIDLIST_DEFAULT = "";

        static const string AI_AGENT_NAME_KEY = "ai_agent_name";
        static const string AI_AGENT_NAME_DEFAULT = "";
        
        static const string AI_AGENT_TYPE_KEY     = "ai_agent_type";
        static const string AI_AGENT_TYPE_BINARY  = "binary";
        static const string AI_AGENT_TYPE_DEFAULT = AI_AGENT_TYPE_BINARY;
        
        static const string AI_AGENT_IMPLEMENATION_KEY     = "ai_agent_implemenation";
        static const string AI_AGENT_IMPLEMENATION_DEFAULT = "";

        static const string AI_AGENT_STATEDIR_KEY     = "ai_agent_statedir";
        static const string AI_AGENT_STATEDIR_DEFAULT = "/tmp";
        
        static const string AI_AGENT_EMBLEMID_KEY     = "ai_agent_emblemid";
        static const string AI_AGENT_EMBLEMID_DEFAULT = "0";

        static const string AI_AGENT_PERSID_KEY     = "ai_agent_personality";
        static const string AI_AGENT_PERSID_DEFAULT = "0";

        static string getAgentConfig( const std::string& agentUUID,
                                      const std::string& uk,
                                      const std::string& def )
        {
            string k = agentUUID + "_" + uk;
            return getEDBString( FDB_GENERAL, k, def, true, true );
        }

        static void setAgentConfig( const std::string& agentUUID,
                                    const std::string& uk,
                                    const std::string& v )
        {
            string k = agentUUID + "_" + uk;
            return setEDBString( FDB_GENERAL, k, v );
        }

        typedef map< string, fh_agent > agents_t;
        static agents_t& getAllAgents( bool forceReload = false );
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        AgentImplemenation::AgentImplemenation( std::string stateDir )
            :
            m_stateDir( stateDir )
        {
        }

        void
        AgentImplemenation::setStateDir( const std::string& s )
        {
            fh_context c = Shell::touch( s, true, true );
            m_stateDir = s;
        }

        std::string
        AgentImplemenation::getStateDir()
        {
            return m_stateDir;
        }
            
        void
        AgentImplemenation::initialize()
        {
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        BinaryClassifierAgentTrainerImplemenationStateMinder::
        BinaryClassifierAgentTrainerImplemenationStateMinder()
            :
            m_dirty( false )
        {
        }

        void
        BinaryClassifierAgentTrainerImplemenationStateMinder::load(
            const std::string& stateDir )
        {
            m_cases.clear();
            m_dirty = false;
            
            fh_ifstream iss( stateDir + "/ferris_training_model" );
            while( iss )
            {
                string earl;
                double sureness;

                if( iss >> sureness && iss >> earl )
                    m_cases[ earl ] = sureness;
            }
        }
        
        void
        BinaryClassifierAgentTrainerImplemenationStateMinder::sync(
            const std::string& stateDir )
        {
            if( m_dirty )
            {
                fh_ofstream oss( stateDir + "/ferris_training_model" );
                for( m_cases_t::iterator ci = m_cases.begin(); ci!=m_cases.end(); ++ci )
                {
                    oss << ci->second << " " << ci->first << nl;
                }
                oss << flush;
            }
        }
        
            
        void
        BinaryClassifierAgentTrainerImplemenationStateMinder::addCase(
            fh_context c, double desiredOutCome )
        {
            m_cases.insert( make_pair( c->getURL(), desiredOutCome ));
        }
        
        void
        BinaryClassifierAgentTrainerImplemenationStateMinder::removeCase(
            fh_context c )
        {
            m_cases.erase( m_cases.find( c->getURL() ) );
        }

        double
        BinaryClassifierAgentTrainerImplemenationStateMinder::lookup(
            const std::string& earl )
        {
            m_cases_t::iterator ci = m_cases.find( earl );
            if( ci != m_cases.end() )
            {
                return ci->second;
            }
            return 0;
        }
            
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        Agent::Agent( const std::string& uuid,
                      const std::string& name,
                      fh_emblem em,
                      fh_personality pers )
            :
            m_uuid( uuid ),
            m_name( name ),
            m_em( em ),
            m_pers( pers )
        {
        }

        void
        Agent::save()
        {
            setAgentConfig( getUUID(), AI_AGENT_NAME_KEY,          getName() );
            setAgentConfig( getUUID(), AI_AGENT_EMBLEMID_KEY,      tostr( m_em->getID() ) );
            setAgentConfig( getUUID(), AI_AGENT_PERSID_KEY,        tostr( m_pers->getID() ) );
            setAgentConfig( getUUID(), AI_AGENT_IMPLEMENATION_KEY, getAgentImplemenationName() );
            setAgentConfig( getUUID(), AI_AGENT_STATEDIR_KEY,      getStateDir() );
            setAgentConfig( getUUID(), AI_AGENT_TYPE_KEY,          AI_AGENT_TYPE_BINARY );
        }


        
        std::string
        Agent::getUUID()
        {
            return m_uuid;
        }
        
        std::string
        Agent::getName() 
        {
            return m_name;
        }

        void
        Agent::setName( const std::string& v )
        {
            m_name = v;
            setAgentConfig( getUUID(), AI_AGENT_NAME_KEY, getName() );
        }
        
        

        fh_emblem
        Agent::getEmblem()
        {
            return m_em;
        }

        void
        Agent::setEmblem( fh_emblem em )
        {
            m_em = em;
            setAgentConfig( getUUID(), AI_AGENT_EMBLEMID_KEY,      tostr( em->getID() ) );
        }

        bool
        Agent::allowsManyEmblems()
        {
            return false;
        }
        
        emblems_t
        Agent::getEmblems()
        {
            emblems_t ret;
            ret.push_back( getEmblem() );
            return ret;
        }
        
        void
        Agent::setEmblems( const emblems_t& el )
        {
            if( el.size() > 1 )
            {
                fh_stringstream ss;
                ss << "Agent can not handle classification with a set of outcomes";
                Throw_AgentOnlyHandlesOneEmblemException( tostr(ss), 0 );
            }
            if( !el.empty() )
            {
                setEmblem( el.front() );
            }
        }
        
        fh_personality
        Agent::getPersonality()
        {
            return m_pers;
        }
        
        void
        Agent::setPersonality( fh_personality pers )
        {
            m_pers = pers;
            setAgentConfig( getUUID(), AI_AGENT_PERSID_KEY,        tostr( pers->getID() ) );
        }
        
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        /**
         * This is a liaison class giving a ferris world view of binary classifier
         * agents. 
         *
         * Ferris apps should use this class, folks writing agents should use the
         * BinaryClassifierAgentImplementation class.
         *
         * This also abstracts away the personality that the agent will be using.
         * It is thus fully in libferris' control as to what agents get to
         * assign what emblems and what personalities are used by those agents.
         */
        class FERRISEXP_DLLLOCAL BinaryClassifierAgent
            :
            public Agent
        {
            friend fh_agent loadAgent( const std::string& agentUUID );
            friend fh_agent createBinaryAgent( const std::string& agentName,
                                                     const std::string& agentImplemenationName,
                                                     const std::string& stateDir,
                                                     fh_emblem em,
                                                     fh_personality pers );
            
            fh_bAgentImpl   m_agent;
            fh_bTrainerImpl m_trainer;

            BinaryClassifierAgent( const std::string& uuid,
                                   const std::string& name,
                                   fh_bAgentImpl m_agent,
                                   fh_emblem em,
                                   fh_personality pers );


            /**
             * Throw an exception if the object is not in a valid state.
             * This method can be called to ensure we have bindings for
             * all object variables.
             */
            void assertWeAreValid();
            
        public:
            virtual ~BinaryClassifierAgent();


            virtual void addTrainingExample( fh_context c );
            virtual void train();
            virtual void classify( fh_context c );

            std::string    getAgentImplemenationName();
            void           setAgentImplemenationName( const std::string& v );

            virtual std::string    getStateDir();
            virtual void           setStateDir( const std::string& v );
            virtual stringlist_t   getAlternateImplementationNames();
            
        };

        /**
         * This class contains all the active agents and will
         * classify a file using each agent that the user currently wants
         * involved in classification.
         */
        class FERRISEXP_DLLLOCAL CompleteAgent
            :
            public Agent
        {
            friend fh_agent getCompleteAgent();

            typedef std::list< fh_agent > m_agents_t;
            m_agents_t m_agents;
            
            CompleteAgent();
        public:
            virtual ~CompleteAgent();
            
            virtual void addTrainingExample( fh_context c );
            virtual void train();
            virtual void classify( fh_context c );

            virtual std::string    getAgentImplemenationName()
                {
                    return "none";
                }
            
            virtual void           setAgentImplemenationName( const std::string& v )
                {
                }
            virtual std::string    getStateDir()
                {
                    return "";
                }
            virtual void           setStateDir( const std::string& v )
                {
                }
            virtual stringlist_t   getAlternateImplementationNames()
                {
                    stringlist_t d;
                    return d;
                }
            
            
        };


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        

        stringlist_t&
        getBinaryAgentImplemenationNames()
        {
            static stringlist_t sl;
            return sl;
        }


        stringlist_t&
        getAgentNames()
        {
            static stringlist_t ret;

            ret.clear();
            agents_t::iterator end = getAllAgents().end();
            for( agents_t::iterator di = getAllAgents().begin(); di!=end; ++di )
            {
                ret.push_back( di->first );
            }
            
            return ret;
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        BinaryClassifierAgent::BinaryClassifierAgent(
            const std::string& uuid,
            const std::string& name,
            fh_bAgentImpl m_agent,
            fh_emblem em,
            fh_personality pers )
            :
            Agent( uuid,
                   name,
                   em,
                   pers ),
            m_agent( m_agent ),
            m_trainer( 0 )
        {
        }
        
        BinaryClassifierAgent::~BinaryClassifierAgent()
        {
        }

        std::string
        BinaryClassifierAgent::getAgentImplemenationName()
        {
            return m_agent->getImplementationName();
        }
        
        void
        BinaryClassifierAgent::setAgentImplemenationName( const std::string& v )
        {
            if( v != getAgentImplemenationName() )
            {
                fh_bAgentImpl  agent =
                    BinaryClassifierAgentImplemenationFactory::Instance().CreateObject( v );
                agent->setStateDir( m_agent->getStateDir() );
                m_agent = agent;
            }
            
            setAgentConfig( getUUID(), AI_AGENT_IMPLEMENATION_KEY, v );
        }

        std::string
        BinaryClassifierAgent::getStateDir()
        {
            return m_agent->getStateDir();
        }
        
        void
        BinaryClassifierAgent::setStateDir( const std::string& v )
        {
            m_agent->setStateDir( v );
            setAgentConfig( getUUID(), AI_AGENT_STATEDIR_KEY, v );
        }
        
        void
        BinaryClassifierAgent::assertWeAreValid()
        {
        }

        stringlist_t
        BinaryClassifierAgent::getAlternateImplementationNames()
        {
            stringlist_t ret;

            stringlist_t& sl = getBinaryAgentImplemenationNames();
            copy( sl.begin(), sl.end(), back_inserter( ret ));
            
            return ret;
        }
        
        void
        BinaryClassifierAgent::addTrainingExample( fh_context c )
        {
            assertWeAreValid();

            if( !m_trainer )
            {
                m_trainer = m_agent->getTrainer();
            }
            
            fh_medallion med = c->getMedallion();
            bool desired = med->hasEmblem( getEmblem() );

//             cerr << "BinaryClassifierAgent::addTrainingExample() c:" << c->getURL()
//                  << " desired:" << desired
//                  << endl;
            m_trainer->addCase( c,
                                desired
                                ? MedallionBelief::SURENESS_MAX
                                : MedallionBelief::SURENESS_MIN );
        }
        
        void
        BinaryClassifierAgent::train()
        {
            assertWeAreValid();

            m_trainer->train();
            m_trainer = 0;
        }
        
        void
        BinaryClassifierAgent::classify( fh_context c )
        {
            assertWeAreValid();

            double sureness = m_agent->classify( c );
            fh_medallion med = c->getMedallion();
            med->addEmblem( getEmblem(), getPersonality(), sureness );
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        CompleteAgent::CompleteAgent()
            :
            Agent( COMPLETE_AGENT_UUID, COMPLETE_AGENT_NAME, 0, 0 )
        {
            stringlist_t& sl = getAgentNames();
            for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
            {
                m_agents.push_back( getAgent( *si ) );
            }
        }
        
        CompleteAgent::~CompleteAgent()
        {
        }
        
        void
        CompleteAgent::addTrainingExample( fh_context c )
        {
            for( m_agents_t::iterator di = m_agents.begin(); di != m_agents.end(); ++di )
            {
                (*di)->addTrainingExample( c );
            }
        }
        
        void
        CompleteAgent::train()
        {
            for( m_agents_t::iterator di = m_agents.begin(); di != m_agents.end(); ++di )
            {
                (*di)->train();
            }
        }
        
        void
        CompleteAgent::classify( fh_context c )
        {
            for( m_agents_t::iterator di = m_agents.begin(); di != m_agents.end(); ++di )
            {
                (*di)->classify( c );
            }
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        fh_agent createBinaryAgent( const std::string& agentName,
                                          const std::string& agentImplemenationName,
                                          const std::string& stateDir,
                                          fh_emblem em,
                                          fh_personality pers )
        {
            fh_agent ret = 0;

            // make sure the name is not in use
            stringlist_t& sl = getBinaryAgentImplemenationNames();
            if( sl.end() != find( sl.begin(), sl.end(), agentName ))
            {
                fh_stringstream ss;
                ss << "Can not create a new agent with name:" << agentName
                   << " because an agent already exists with that name" << endl;
                Throw_AgentAlreadyExistsException( tostr(ss), 0 );
            }

            std::string uuid = Util::makeUUID();
            
            fh_bAgentImpl  agent =
                BinaryClassifierAgentImplemenationFactory::Instance().CreateObject(
                    agentImplemenationName );
            agent->setStateDir( stateDir );
            agent->initialize();
            ret = new BinaryClassifierAgent( uuid,
                                                     agentName,
                                                     agent,
                                                     em,
                                                     pers );

            setAgentConfig( uuid, AI_AGENT_NAME_KEY,          agentName );
            setAgentConfig( uuid, AI_AGENT_IMPLEMENATION_KEY, agentImplemenationName );
            setAgentConfig( uuid, AI_AGENT_STATEDIR_KEY,      stateDir );
            setAgentConfig( uuid, AI_AGENT_EMBLEMID_KEY,      tostr( em->getID() ) );
            setAgentConfig( uuid, AI_AGENT_PERSID_KEY,        tostr( pers->getID() ) );
            setAgentConfig( uuid, AI_AGENT_TYPE_KEY,          AI_AGENT_TYPE_BINARY );
            
            {
                string n = getEDBString( FDB_GENERAL,
                                         AI_AGENT_UUIDLIST_KEY,
                                         AI_AGENT_UUIDLIST_DEFAULT );
                stringlist_t sl = Util::parseCommaSeperatedList( n );
                sl.push_back( uuid );
                setEDBString( FDB_GENERAL, AI_AGENT_UUIDLIST_KEY, Util::createCommaSeperatedList( sl ));
            }
            
            return ret;
        }
        

        fh_agent getAgent( const std::string& agentName )
        {
            agents_t::iterator di = getAllAgents().find( agentName );
            if( di == getAllAgents().end() )
            {
                fh_stringstream ss;
                ss << "No agent exists with name:" << agentName;
                Throw_NoSuchAgentException( tostr(ss), 0 );
            }
            return di->second;
        }
        
        void
        eraseAgent( fh_agent d )
        {
            string name = d->getName();

            string n = getEDBString( FDB_GENERAL,
                                     AI_AGENT_UUIDLIST_KEY,
                                     AI_AGENT_UUIDLIST_DEFAULT );
            stringlist_t sl = Util::parseCommaSeperatedList( n );
            sl.erase( find( sl.begin(), sl.end(), d->getUUID() ) );
            setEDBString( FDB_GENERAL, AI_AGENT_UUIDLIST_KEY, Util::createCommaSeperatedList( sl ));
        }
        
        fh_agent getCompleteAgent()
        {
            fh_agent ret = new CompleteAgent();
            return ret;
        }

        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

#ifdef HAVE_BOGOFILTER

        class BogoFilter_BinaryClassifierAgentImplemenation;
        FERRIS_SMARTPTR( BogoFilter_BinaryClassifierAgentImplemenation, fh_bogoAgentImpl );
        
        class FERRISEXP_DLLLOCAL BogoFilter_BinaryClassifierAgentTrainerImplemenation
            :
            public BinaryClassifierAgentTrainerImplemenation
        {
            fh_bTrainerImplStateMinder m_stateMinder;
            
            typedef list< pair< fh_context, double > > m_cases_t;
            m_cases_t m_added_cases;
            m_cases_t m_removed_cases;
            
            fh_bogoAgentImpl m_agent;
            
        public:

            BogoFilter_BinaryClassifierAgentTrainerImplemenation( fh_bogoAgentImpl agent );
            virtual void addCase( fh_context c, double desiredOutCome );
            virtual void removeCase( fh_context c );
            virtual void trainOnFile( fh_context c,
                                      double sureness,
                                      bool wasAdded );
            virtual void train();
            virtual void sync();
        };
        
        
        class FERRISEXP_DLLLOCAL BogoFilter_BinaryClassifierAgentImplemenation
            :
            public BinaryClassifierAgentImplemenation
        {
            
        public:

            string getCommandName();
            void addParsingCommandLineOptions( fh_stringstream& ss, bool fileIsHTML = false );
            void addConfigfileCommandLineOptions( fh_stringstream& ss );
            fh_bTrainerImpl getTrainer();
            virtual double classify( fh_context c );

            fh_runner getRunner();
            std::string getImplementationName()
                {
                    return "bogofilter";
                }
        };
        static bool
        BogoFilter_BinaryClassifierAgentImplemenation_Registered =
        RegisterBinaryClassifierAgentImplemenationFactory( 
//        BinaryClassifierAgentImplemenationFactory::Instance().Register(
            "bogofilter",
            &MakeObject< BinaryClassifierAgentImplemenation, BogoFilter_BinaryClassifierAgentImplemenation >::Create );

        /********************/
        /********************/
        /********************/

        BogoFilter_BinaryClassifierAgentTrainerImplemenation::BogoFilter_BinaryClassifierAgentTrainerImplemenation(
            fh_bogoAgentImpl agent )
            :
            m_agent( agent )
        {
            m_stateMinder->load( agent->getStateDir() );
        }
            
        void
        BogoFilter_BinaryClassifierAgentTrainerImplemenation::addCase(
            fh_context c, double desiredOutCome )
        {
            m_stateMinder->addCase( c, desiredOutCome );
            m_added_cases.push_back( make_pair( c, desiredOutCome ));
        }
        void
        BogoFilter_BinaryClassifierAgentTrainerImplemenation::removeCase( fh_context c )
        {
            string earl = c->getURL();
            m_removed_cases.push_back( make_pair( c, m_stateMinder->lookup( earl ) ));
            m_stateMinder->removeCase( c );
        }

        void
        BogoFilter_BinaryClassifierAgentTrainerImplemenation::sync()
        {
            m_stateMinder->sync( m_agent->getStateDir() );
        }
        
        void
        BogoFilter_BinaryClassifierAgentTrainerImplemenation::trainOnFile(
            fh_context c, double sureness, bool wasAdded )
        {
//            cerr << "training on, outcome:" << sureness << " c:" << c->getURL() << endl;

            fh_stringstream ss;
            ss << m_agent->getCommandName() << " ";

            if( wasAdded )
            {
                if( sureness > MedallionBelief::SURENESS_NULL )
                    ss << " -n ";
                else
                    ss << " -s ";
            }
            else
            {
                if( sureness > MedallionBelief::SURENESS_NULL )
                    ss << " -N ";
                else
                    ss << " -S ";
            }
            
                
            m_agent->addConfigfileCommandLineOptions( ss );
            m_agent->addParsingCommandLineOptions( ss );

//            cerr << "TrainCMD:" << tostr(ss) << endl;

            fh_runner r = m_agent->getRunner();
            r->setCommandLine( tostr(ss) );
            r->Run();
            {
                fh_istream iss = c->getIStream();
                fh_ostream oss = r->getStdIn();

                copy( istreambuf_iterator<char>(iss),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(oss));
                oss << flush;
            }
            close( r->getStdInFd() );
            gint e = r->getExitStatus();
        }
        
        void
        BogoFilter_BinaryClassifierAgentTrainerImplemenation::train()
        {
            for( m_cases_t::iterator ci = m_added_cases.begin();
                 ci!=m_added_cases.end(); ++ci )
            {
                fh_context  c  = ci->first;
                double outcome = ci->second;

                trainOnFile( c, outcome, true );
            }

            for( m_cases_t::iterator ci = m_removed_cases.begin();
                 ci!=m_removed_cases.end(); ++ci )
            {
                fh_context  c  = ci->first;
                double outcome = ci->second;

                trainOnFile( c, outcome, false );
            }
        }
        

        /********************/
        /********************/
        /********************/

        string
        BogoFilter_BinaryClassifierAgentImplemenation::getCommandName()
        {
            return "bogofilter";
        }
        
        void
        BogoFilter_BinaryClassifierAgentImplemenation::addParsingCommandLineOptions( fh_stringstream& ss,
                                                                        bool fileIsHTML )
        {
            ss << " -PH -Pi ";
            if( fileIsHTML ) ss << " -Pt ";
            else             ss << " -PT ";
        }
        
        void
        BogoFilter_BinaryClassifierAgentImplemenation::addConfigfileCommandLineOptions( fh_stringstream& ss )
        {
            ss << " -d " << getStateDir();
            ss << " -W ";
        }
        
        fh_bTrainerImpl
        BogoFilter_BinaryClassifierAgentImplemenation::getTrainer()
        {
            return new BogoFilter_BinaryClassifierAgentTrainerImplemenation( this );
        }
            
        double
        BogoFilter_BinaryClassifierAgentImplemenation::classify( fh_context c )
        {
            fh_stringstream ss;
            ss << getCommandName() << " ";
            addConfigfileCommandLineOptions( ss );
            addParsingCommandLineOptions( ss );
            ss << " -T -3 ";

//            cerr << "Classify CMD:" << tostr(ss) << endl;

            double ret = 0;

            fh_runner r = getRunner();
            r->setCommandLine( tostr(ss) );
            r->Run();
            
            {
                fh_ostream oss = r->getStdIn();
                fh_istream iss = c->getIStream();

//                 copy( istreambuf_iterator<char>(iss),
//                       istreambuf_iterator<char>(),
//                       ostreambuf_iterator<char>(oss));
//                 oss << flush;
                
                char c;
                int byteW = 0;
                for( ; iss >> noskipws >> c; ++byteW )
                    oss << c;
                oss << flush;
                
            }
            close( r->getStdInFd() );
            
            fh_istream cmdoutput = r->getStdOut();
            char yesno = ' ';
            cmdoutput >> yesno;
            cmdoutput >> ret;

//            cerr << "Classify YesNo1:" << yesno << " ret:" << ret << endl;
            
            if( yesno == 'H' )
            {
                ret *= 100;
            }
            else if( yesno == 'S' )
            {
                ret *= -100;
            }
            else
                ret = 0;
            
            gint e = r->getExitStatus();
//             cerr << "Classify(9) e:" << e << endl;
//             cerr << "Classify YesNo:" << yesno << " ret:" << ret << endl;
            return ret;
        }

        fh_runner
        BogoFilter_BinaryClassifierAgentImplemenation::getRunner()
        {
            fh_runner r = new Runner();
            r->setSpawnFlags(
                GSpawnFlags( G_SPAWN_SEARCH_PATH |
                             G_SPAWN_DO_NOT_REAP_CHILD |
                             G_SPAWN_STDERR_TO_DEV_NULL |
                             r->getSpawnFlags()));
            r->setConnectStdIn( true );
            
            return r;
        }
        
        
#endif

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        fh_agent loadAgent( const std::string& agentUUID )
        {
            fh_agent ret = 0;

            string agentName = getAgentConfig( agentUUID,
                                               AI_AGENT_NAME_KEY,
                                               AI_AGENT_NAME_DEFAULT );
            
            string agentImplemenationName = getAgentConfig( agentUUID,
                                                            AI_AGENT_IMPLEMENATION_KEY,
                                                            AI_AGENT_IMPLEMENATION_DEFAULT );
            string stateDir = getAgentConfig( agentUUID,
                                              AI_AGENT_STATEDIR_KEY,
                                              AI_AGENT_STATEDIR_DEFAULT );
            string emblemID = getAgentConfig( agentUUID,
                                              AI_AGENT_EMBLEMID_KEY,
                                              AI_AGENT_EMBLEMID_DEFAULT );
            string persID = getAgentConfig( agentUUID,
                                            AI_AGENT_PERSID_KEY,
                                            AI_AGENT_PERSID_DEFAULT );

            string agentType = getAgentConfig( agentUUID,
                                            AI_AGENT_TYPE_KEY,
                                            AI_AGENT_TYPE_DEFAULT );
            
            fh_etagere et = Factory::getEtagere();
            fh_emblem  em = et->getEmblemByID( toType<emblemID_t>( emblemID ));
            fh_personality pers = obtainPersonality( toType<emblemID_t>( persID ));
            
            fh_bAgentImpl agent =
                BinaryClassifierAgentImplemenationFactory::Instance().CreateObject(
                    agentImplemenationName );
            agent->setStateDir( stateDir );
            agent->initialize();

            ret = new BinaryClassifierAgent( agentUUID, agentName, agent, em, pers );
            return ret;
        }

        /**
         * A nice little static cache of loaded persistent agents
         */
        agents_t& getAllAgents( bool forceReload )
        {
            static agents_t cache;
            if( forceReload || cache.empty() )
            {
                cache.clear();
                string n = getEDBString( FDB_GENERAL,
                                         AI_AGENT_UUIDLIST_KEY,
                                         AI_AGENT_UUIDLIST_DEFAULT );
                stringlist_t sl = Util::parseCommaSeperatedList( n );
                
                for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
                {
                    fh_agent d = loadAgent( *si );
                    cache.insert( make_pair( d->getName(), d ));
                }
            }
            return cache;
        }
        
    };
};
