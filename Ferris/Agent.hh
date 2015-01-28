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

    $Id: Agent.hh,v 1.2 2010/09/24 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_AGENT_H_
#define _ALREADY_INCLUDED_FERRIS_AGENT_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <Ferris/TypeDecl.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/Personalities.hh>

namespace Ferris
{
    namespace AI
    {
        
        class Agent;
        FERRIS_SMARTPTR( Agent, fh_agent );

        /**
         * Get the names of all agents that have been created.
         *
         * Agent creation can be done via the agent capplet or
         * using one of the createBinaryAgent() type calls.
         */
        FERRISEXP_API stringlist_t& getAgentNames();

        /**
         * Get a reference to an agent given its name. There will
         * only be one instance of that agent in any given process
         * subsequent calls to getAgent() with the same name return
         * the same instance.
         */
        FERRISEXP_API fh_agent getAgent( const std::string& agentName );

        /** 
         * Create an agent that can apply all agents that the
         * user has setup to classify a file.
         */
        FERRISEXP_API fh_agent getCompleteAgent();
        
        /**
         * Remove an agent from the system. Future calls to
         * getAgent() for this agent name will fail, future calls to
         * createBinaryAgent() with this agent name will create a
         * new persistant agent instance.
         */
        FERRISEXP_API void eraseAgent( fh_agent d );
        

        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * Get the names of all the binary agents the system knows about.
         * These are the only valid values for agentImplemenationName in
         * createBinaryAgent()
         */
        FERRISEXP_API stringlist_t& getBinaryAgentImplemenationNames();
        
        /**
         * libferris clients wanting to run agents over a file should
         * use getCompleteAgent(). This is only for creating new agent
         * instances.
         * 
         * Create a binary classification agent given the agent's
         * name. This agent will be using the given emblem to assign its
         * fuzzy results to and to train on. The stateDir is where this agent
         * will live and it can store its learnt state in that directory.
         *
         * @see getAgent()
         */
        FERRISEXP_API fh_agent createBinaryAgent( const std::string& agentName,
                                                  const std::string& agentImplemenationName,
                                                  const std::string& stateDir,
                                                  fh_emblem em,
                                                  fh_personality pers );

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        
        
        /**
         * This is a liaison class giving a ferris world view of agents.
         * An agent is a persistent binding between an AgentImplementation
         * where its state shall be stored, what emblem(s) to use for storing
         * beliefs and what personality the agent should use when making assertion
         * and retraction statements.
         *
         * Ferris apps should use this class, folks writing agent implemenations
         * or looking to hookup their favorate AI algorithm should see Ferris/Agent.cpp
         * and a subclass of AgentImplemenation for example, BinaryClassifierAgentImplemenation.
         *
         * As this is the primary abstraction for interacting with the agents in
         * the ferris world this class gets to be called Agent. The implementation
         * that an agent uses to actually perform its task is not exported to
         * the ferris world and exists only as AgentImplemenation subclasses that are
         * conditionally compiled within Agent.cpp
         */
        class FERRISEXP_API Agent
            :
            public Handlable
        {
            
            std::string     m_uuid;
            std::string     m_name;
            fh_emblem       m_em;
            fh_personality  m_pers;

            void save();
            
        public:

            Agent( const std::string& UUID,
                   const std::string& name,
                   fh_emblem em,
                   fh_personality pers );

            std::string    getUUID();
            std::string    getName();
            void           setName( const std::string& v );
            fh_emblem      getEmblem();
            void           setEmblem( fh_emblem em );

            bool               allowsManyEmblems();
            Ferris::emblems_t  getEmblems();
            void               setEmblems( const Ferris::emblems_t& el );
            
            fh_personality getPersonality();
            void           setPersonality( fh_personality pers );
            virtual std::string    getAgentImplemenationName() = 0;
            virtual void           setAgentImplemenationName( const std::string& v ) = 0;
            virtual std::string    getStateDir() = 0;
            virtual void           setStateDir( const std::string& v ) = 0;
            virtual stringlist_t   getAlternateImplementationNames() = 0;
            
            
            /**
             * Add a training example
             */
            virtual void addTrainingExample( fh_context c ) = 0;

            /**
             * retain agent
             */
            virtual void train() = 0;

            /**
             * Attach belief to context based on what the agent thinks.
             */
            virtual void classify( fh_context c ) = 0;

            
        };

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    };
};
#endif
