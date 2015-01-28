/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: Agent_private.hh,v 1.4 2010/09/24 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_AGENT_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_AGENT_PRIV_H_

#include <Ferris/Agent.hh>
#include <Factory.h>
#include <Ferris/FerrisStdHashMap.hh>

namespace Ferris
{
    namespace AI
    {
        template < class Base,class Sub >
        struct MakeObject
        {
            static Base* Create()
                { return new Sub(); }
        };

        /**
         * Top level agent implemenation class... Mr Anderson.
         *
         * This class is intended for pluging in AI algorithms not direct
         * use by libferris clients.
         */
        class FERRISEXP_DLLLOCAL AgentImplemenation
            :
            public Handlable
        {
            friend fh_agent loadAgent( const std::string& agentUUID );
            friend fh_agent createBinaryAgent( const std::string& agentName,
                                               const std::string& agentImplemenationName,
                                               const std::string& stateDir,
                                               fh_emblem em,
                                               fh_personality pers );

            
            /**
             * This is a directory that the agent can save its past learnt
             * data into. The agent should leave alone a file 'ferris-agent-metadata.db'
             * if such a file exists.
             */
            std::string m_stateDir;

        public:

            AgentImplemenation( std::string stateDir = "" );

            virtual std::string getImplementationName() = 0;
            virtual void setStateDir( const std::string& s );
            virtual std::string getStateDir();
            
            /**
             * Called after all object state is set so that the agent can
             * read info from m_stateDir etc and prepare for work
             */
            virtual void initialize();
        };



        /**
         * AI plugin class for training binary classifiers
         *
         * Note that this is an interface that is returned from the AgentImplemenation
         * itself and shouldn't be created directly by libferris code that
         * is not the AgentImplemenation for this trainer.
         */
        class FERRISEXP_DLLLOCAL BinaryClassifierAgentTrainerImplemenation
            :
            public Handlable
        {
        public:

            /**
             * When a new test case is added for the agent to train on
             * this method is called. It is here to allow the agent to
             * calculate and store state relating to this new file in
             * its current form. Although all the training example names
             * will be available to the train() method this method allows
             * the following:
             *
             * 1) capture of file's byte content at the time closest to
             *    when the uesr decides to add it as a training example.
             *    For example, when the user adds a website page they are
             *    probably only asserting that it is in a category at the
             *    current point in time and if the site's content changes
             *    it may adversely effect the agent's performance.
             *
             * 2) Allow caching of state for the new context to speed up
             *    training. For example the FeatureSet for the context will
             *    be calculated and persisted by this call
             *
             * Cases that should no longer be trained on will be submitted
             * to removeCase() so that cached state for this file can be
             * cleaned up.
             *
             * The usual pattern for use of the trainer is the addCase() one
             * or more new examples for the model and then train(); Note that
             * only the new cases are submitted with addCase(), it is assumed
             * that the trainer remembers what cases have been submitted in the
             * past.
             *
             * 
             * @param c the file to add to the training set
             * @param desiredOutCome is a value in the range 0 to 100 that
             *                       shows a how sure the file is in the class.
             *                       Binary classifiers can just perform ">0"
             *                       to ignore the extra data provided.
             */
            virtual void addCase( fh_context c, double desiredOutCome ) = 0;

            /**
             * This case is no longer part of the training data. Remove
             * any cached state for that file.
             */
            virtual void removeCase( fh_context c ) = 0;
            
            /**
             * Retrain the agent with the examples provided in
             * previous calls to addCase()
             *
             */
            virtual void train() = 0;

            /**
             * Write out any changes to state
             */
            virtual void sync()
                {}
            
        };
        FERRIS_SMARTPTR( BinaryClassifierAgentTrainerImplemenation, fh_bTrainerImpl );
        
        /**
         * AI plugin class for doing binary classification
         */
        class FERRISEXP_DLLLOCAL BinaryClassifierAgentImplemenation
            :
            public AgentImplemenation
        {
        public:

            /**
             * Get a trainer for this agent
             */
            virtual fh_bTrainerImpl getTrainer() = 0;

            /**
             * Give the classification for this file based on previous training.
             */
            virtual double classify( fh_context c ) = 0;
        };
        FERRIS_SMARTPTR( BinaryClassifierAgentImplemenation, fh_bAgentImpl );

        /**
         * This keeps track of the < sureness, url > data and persists such
         * information for next run. It is handy in some agents which need
         * to know how a document was asserted in order to removed it from
         * the system.
         *
         * Its a seperate class because many agents will want to have such
         * functionality.
         */
        class FERRISEXP_DLLLOCAL BinaryClassifierAgentTrainerImplemenationStateMinder
            :
            public Handlable
        {
            bool m_dirty;
            typedef FERRIS_STD_HASH_MAP< std::string, double > m_cases_t;
            m_cases_t m_cases;
            
        public:
            BinaryClassifierAgentTrainerImplemenationStateMinder();
            void addCase( fh_context c, double desiredOutCome );
            void removeCase( fh_context c );
            double lookup( const std::string& earl );
            void load( const std::string& stateDir );
            void sync( const std::string& stateDir );
        };
        FERRIS_SMARTPTR( BinaryClassifierAgentTrainerImplemenationStateMinder,
                         fh_bTrainerImplStateMinder );
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        typedef Loki::SingletonHolder<
            Loki::Factory< BinaryClassifierAgentImplemenation, std::string >,
            Loki::CreateUsingNew, Loki::NoDestroy >
        BinaryClassifierAgentImplemenationFactory;

        template< typename ProductCreator >
        bool RegisterBinaryClassifierAgentImplemenationFactory( const std::string& agentName,
                                                   ProductCreator creator )
        {
            getBinaryAgentImplemenationNames().push_back( agentName );
            return BinaryClassifierAgentImplemenationFactory::Instance().Register( agentName, creator );
        }
        
    };
};
#endif
