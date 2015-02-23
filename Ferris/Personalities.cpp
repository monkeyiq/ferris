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

    $Id: Personalities.cpp,v 1.2 2010/09/24 21:30:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/Personalities.hh>
#include <Ferris/Medallion_private.hh>
#include <Ferris/FerrisException.hh>

using namespace std;

namespace Ferris
{
    const string EMBLEM_PERSONALITY_TREE_NAME = "personalities";
    const string EMBLEM_USER_PERSONALITY_NAME = "user";
    const string EMBLEM_GENERAL_AGENT_NAME    = "generic-agent";

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    Personality::Personality( fh_etagere et, fh_emblem em )
        :
        m_et( et ),
        m_em( em )
    {
    }

    Personality::~Personality()
    {
    }

    string
    Personality::getName()
    {
        return m_em->getName();
    }

    emblemID_t
    Personality::getID()
    {
        return m_em->getID();
    }
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Nice little subclass that allows factory methods creation ability
     */
    class FERRISEXP_DLLLOCAL PersonalityImpl
        :
        public Personality
    {
    public:
        PersonalityImpl(fh_etagere et, fh_emblem em )
            :
            Personality( et, em )
            {}
            
        virtual ~PersonalityImpl()
            {}

        fh_emblem getEmblem()
            {
                return m_em;
            }
    };
    FERRIS_SMARTPTR( PersonalityImpl, fh_personalityImpl );
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    static fh_personalityImpl EmblemToPersonality( fh_emblem em, fh_etagere et = 0 )
    {
        static map< fh_emblem, fh_personalityImpl > cache;

        map< fh_emblem, fh_personalityImpl >::iterator ci = cache.find( em );
        if( ci != cache.end() )
        {
            return ci->second;
        }
        
        fh_personalityImpl pi = new PersonalityImpl( et, em );
        cache[ em ] = pi;
        return pi;
        
//         static Cache< fh_emblem, fh_personalityImpl > cache;

//         if( fh_personalityImpl pi = cache.get( em ) )
//         {
//             return pi;
//         }

//         fh_personalityImpl pi = new PersonalityImpl( et, em );
//         cache.put( em, pi );
//         return pi;
    }

    static fh_personalityImpl getCurrentUserPersonalityImpl( fh_etagere et = 0 )
    {
        if( !et )
            et = Factory::getEtagere();
        fh_emblem  em = getFerrisSystemEmblem( et );

        fh_emblem ptree = em->obtainChild( EMBLEM_PERSONALITY_TREE_NAME );
        fh_emblem user  = ptree->obtainChild( EMBLEM_USER_PERSONALITY_NAME );
        et->sync();
        return EmblemToPersonality( user, et );
    }
    
    struct EmblemByNameWithParents
    {
        string name;
        list<fh_personality> parents;
            
        EmblemByNameWithParents( string n,
                                 const std::list<fh_personality>& p )
            :
            name(n),
            parents(p)
            {
            }
        bool operator()( fh_emblem em )
            {
                if( em->getName() == name )
                {
//                    return true;
                    
                    list< fh_emblem > wanted_pem;
                    for( list<fh_personality>::iterator pi = parents.begin();
                         pi!=parents.end(); ++pi )
                    {
                        fh_personality tmp = *pi;
                        if( PersonalityImpl* pip =
                            dynamic_cast<PersonalityImpl*>( GetImpl( tmp )))
                        {
                            wanted_pem.push_back( pip->getEmblem() );
                        }
                    }
                    emblems_t actual_pem = em->getParents();

                    wanted_pem.sort();
                    actual_pem.sort();

                    list< fh_emblem > tmp;
                    set_difference( wanted_pem.begin(), wanted_pem.end(),
                                    actual_pem.begin(), actual_pem.end(),
                                    inserter( tmp, tmp.begin() ));

                    return tmp.empty();
                }
                return false;
            }
    };

    namespace Factory
    {
        fh_personality getCurrentUserPersonality( fh_etagere et )
        {
            return getCurrentUserPersonalityImpl( et );
        }

        fh_personality getGenericClassifierAgentPersonality( fh_etagere et )
        {
            if( !et )
                et = Factory::getEtagere();
            
            fh_personalityImpl p = getCurrentUserPersonalityImpl( et );
            fh_emblem em = p->getEmblem();
            fh_emblem generalagent = em->obtainChild( EMBLEM_GENERAL_AGENT_NAME );
            et->sync();
            return EmblemToPersonality( generalagent, et );
            
        }
        

        
        fh_personality createPersonality( fh_etagere et,
                                          const std::string& name,
                                          const std::list<fh_personality>& parents )
        {
            fh_personalityImpl p  = getCurrentUserPersonalityImpl();
            fh_emblem          em = p->getEmblem();
            emblems_t          ds = em->getDownset();

            emblems_t::iterator ei = find_if( ds.begin(), ds.end(),
                                              EmblemByNameWithParents( name, parents ));
            if( ei != ds.end() )
            {
                return EmblemToPersonality( em, et );
            }

            fh_stringstream ss;
            ss << "No personaility with name:" << name << " found";
            Throw_NoSuchPersonalityException( tostr(ss), 0 );
        }

        fh_personality createPersonality( const std::string& name,
                                          const std::list<fh_personality>& parents )
        {
            return createPersonality( Factory::getEtagere(), name, parents );
        }

        fh_personality createPersonality( const std::string& name )
        {
            std::list<fh_personality> parents;
            return createPersonality( Factory::getEtagere(), name, parents );
        }
    };

    
    fh_personality
    obtainPersonality( fh_etagere et,
                       const std::string& name,
                       const std::list<fh_personality>& parents )
    {
        try
        {
            std::list< fh_personality > pl = findAllPersonalities( name, parents );
            if( !pl.empty() )
                return pl.front();
        }
        catch( NoSuchPersonalityException& e )
        {
        }
        return Factory::createPersonality( et, name, parents );
    }

    fh_personality
    obtainPersonality( const std::string& name )
    {
        std::list<fh_personality> parents;
        return obtainPersonality( Factory::getEtagere(), name, parents );
    }
    
    fh_personality
    obtainPersonality( emblemID_t id )
    {
        fh_etagere et = Factory::getEtagere();
        fh_emblem  em = et->getEmblemByID( id );
        return EmblemToPersonality( em, et );
    }
    
    fh_personality
    findPersonality( const std::string& name, bool decend )
    {
        fh_etagere et = Factory::getEtagere();
        std::list<fh_personality> parents;
        
        if( decend )
        {
            std::list< fh_personality > pl = findAllPersonalities( name, parents );
            if( pl.empty() )
            {
                fh_stringstream ss;
                ss << "No personaility with name:" << name << " found";
                Throw_NoSuchPersonalityException( tostr(ss), 0 );
            }
            return pl.front();
        }

        try
        {
            fh_personalityImpl p = getCurrentUserPersonalityImpl( et );
            fh_emblem em = p->getEmblem();
            fh_emblem child = em->findChild( name );
            return EmblemToPersonality( child, et );
        }
        catch( EmblemNotFoundException& e )
        {
            fh_stringstream ss;
            ss << "No personaility with name:" << name << " found";
            Throw_NoSuchPersonalityException( tostr(ss), 0 );
        }
        fh_stringstream ss;
        ss << "No personaility with name:" << name << " found";
        Throw_NoSuchPersonalityException( tostr(ss), 0 );
    }
    
    std::list< fh_personality >
    findAllPersonalities( const std::string& name,
                          const std::list<fh_personality>& parents )
    {
        fh_etagere et = Factory::getEtagere();
        std::list< fh_personality > ret;

        fh_personalityImpl p = getCurrentUserPersonalityImpl( et );
        fh_emblem em = p->getEmblem();
        emblems_t ds = em->getDownset();

        emblems_t::iterator begin = ds.begin();
        emblems_t::iterator end   = ds.end();

        while( begin != end )
        {
            emblems_t::iterator ei = find_if( begin, end,
                                              EmblemByNameWithParents( name, parents ));
            if( ei == end )
                break;

            ret.push_back( EmblemToPersonality( *ei, et ) );
            begin = ei;
            ++begin;
        }

        return ret;
    }
    
    std::list< fh_personality >
    findAllPersonalities()
    {
        std::list< fh_personality > ret;

        fh_etagere et = Factory::getEtagere();
        fh_personalityImpl p = getCurrentUserPersonalityImpl( et );
        fh_emblem em = p->getEmblem();
        emblems_t ds = em->getDownset();
        emblems_t::iterator ei  = ds.begin();
        emblems_t::iterator end = ds.end();
        for( ; ei != end; ++ei )
        {
            ret.push_back( EmblemToPersonality( *ei, et ) );
        }
        return ret;
    }
    
    
    
};
