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

    $Id: Personalities.hh,v 1.2 2010/09/24 21:30:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_PERSONALITIES_H_
#define _ALREADY_INCLUDED_FERRIS_PERSONALITIES_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Medallion.hh>

namespace Ferris
{
    /**
     * Whenever a program or agent needs to identify itself to libferris
     * it should obtain and use a 'personality'. This is split out of the
     * Medallion.hh header because it is a reasonably generic thing that
     * some program might wish to make assertions or perform actions
     * and give some 'token' as to what program or group of programs it
     * was acting as.
     *
     * This is somewhat different from the process group and username becase
     * it is expected that a user will have many programs running and will
     * wish to discern the actions of various single programs or groups.
     * For example when one has many agents adding hints as to what emblems
     * a file should have attached then you will want to be able to identify
     * the different Machine Learning code that has made assertions because
     * you can trust a given ML agent more or less than another one for a
     * specific filetype or just in the general.
     *
     * To keep the code simple it is best to have unique names for all
     * personailities in the system and not rely on the covering
     * (ie. parent-child) relation to provide unique names.
     */
    class FERRISEXP_API Personality
        :
        public CacheHandlable
    {
        NOT_COPYABLE( Personality );
    protected:
        
        fh_etagere m_et;
        fh_emblem  m_em;

        Personality( fh_etagere et, fh_emblem em );
        
    public:
        virtual ~Personality();

        std::string getName();
        emblemID_t  getID();
    };

    namespace Factory
    {
        /**
         * Get the personality of the current user. This should be used in
         * direct HCI manipulation when the user is asserting or retracting
         * facts. Agents should register and use a personaility of their
         * own to discern agents from direct user beliefs. 
         */
        FERRISEXP_API fh_personality getCurrentUserPersonality( fh_etagere et = 0 );
        
        /**
         * Get the personality that agents that are classifying files
         * should use.
         */
        FERRISEXP_API fh_personality getGenericClassifierAgentPersonality( fh_etagere et = 0 );
        
        
        /**
         * Note that there will be overloaded versions that take less params
         * but the most specific is described in full detail.
         *
         * Create a new personality in the given etagere
         * (usually done with the default Factory::getEtagere())
         * the new personality will have the given name for identification
         * and will have the parent personalities given. Note that
         * the belief system uses the ordering of personalities to
         * resolve who to trust more.
         *
         * @param et The Etagere that we should save our data into. This
         *           is usually the default for the user Factory::getEtagere()
         * @param name What the new personality should be called. Should be
         *             reasonably specific and not a reversed name. Reserved
         *             names are "user" and all caps names.
         * @param parents if given and the collection is non-empty then this
         *                is a list of what parents the sought personality should
         *                have.
         */
        FERRISEXP_API fh_personality createPersonality( fh_etagere et,
                                                        const std::string& name,
                                                        const std::list<fh_personality>& parents );


        /**
         * Note that there will be overloaded versions that take less params
         * but the most specific is described in full detail.
         *
         */
        FERRISEXP_API fh_personality createPersonality( const std::string& name,
                                                        const std::list<fh_personality>& parents );

        /**
         * See also the header comment for the most specific version of
         * createPersonality().
         *
         * This will create a personality that is only under the user personality.
         * The new personality will share the second top trust level with all other
         * personalities that are direct decendants of the user personaility.
         */
        FERRISEXP_API fh_personality createPersonality( const std::string& name );
    };

    /**
     * If the given personality exists then this is like a call to
     * findPersonality()
     *
     * If the given personality doesn't exist then it is loke a call to
     * createPersonality()
     */
    FERRISEXP_API fh_personality obtainPersonality( fh_etagere et,
                                                    const std::string& name,
                                                    const std::list<fh_personality>& parents );

    FERRISEXP_API fh_personality obtainPersonality( const std::string& name );
    FERRISEXP_API fh_personality obtainPersonality( emblemID_t id );
    
    /**
     * Try to find the personality looking at the top node (the user) and its
     * children. optionally decend into all the children attempting to find all
     * personalities with the given name.
     *
     * In the case where the user has two personailities with the same name the
     * first one is returned.
     *
     * @param name Personality should have the given name
     * @param decend Should all personailities be considered or just the user
     *               and his direct children.
     */
    FERRISEXP_API fh_personality findPersonality( const std::string& name, bool decend = true);

    /**
     * Find the personaility with the given name and who's parents have
     * the names given.
     *
     * If there are no personalities that match the constraints given then
     * no exception is thrown, the return value is just empty.
     *
     */
    FERRISEXP_API
    std::list< fh_personality >
    findAllPersonalities( const std::string& name,
                          const std::list<fh_personality>& parents );

    /**
     * Get a list of all available personalities
     */
    FERRISEXP_API
    std::list< fh_personality >
    findAllPersonalities();
    
};
#endif
