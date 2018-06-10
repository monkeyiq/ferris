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

    $Id: Medallion.cpp,v 1.31 2011/05/06 21:30:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <climits>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/version.hpp>

#include "Medallion.hh"
#include "Medallion_private.hh"
#include <Ferris/Personalities.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/Configuration_private.hh>
#include <Ferris/SyncDelayer.hh>

// medallion notification
#include <PluginOutOfProcNotificationEngine.hh>

#include "config.h"
#ifdef HAVE_DBUS
#include "DBus_private.hh"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace STLdb4;

namespace boost {
    namespace serialization {

        template<class Archive>
        void serialize(Archive & ar, ::Ferris::fh_cemblem& e, const unsigned int version)
        {
            ar & *(GetImpl(e));
        }

    } // namespace serialization

} // namespace boost

BOOST_CLASS_VERSION(::Ferris::Emblem, 3)



namespace Ferris
{
    /**
     * Having atexit() here makes doing things that use LG_XXX_D stuff
     * not work correctly.
     */
    class LazyOrAtExitFunction
    {
    public:
        typedef Loki::Functor< void, LOKI_TYPELIST_1( bool ) > f_functor;
        static void add( const f_functor& f );

        // Dont use these...
        static LazyOrAtExitFunction& getSingleton();
        void atExit();
        void atTimer();

    private:
        typedef list< f_functor > m_flist_t;
        m_flist_t m_flist;
        guint m_timer;
        int m_timer_interval;

        void reconnectTimer();
        void setupAtExit();
        LazyOrAtExitFunction();
        void priv_add( const f_functor& f );
        
    };

    LazyOrAtExitFunction::LazyOrAtExitFunction()
        :
        m_timer( 0 ),
        m_timer_interval( 2000 )
    {
        setupAtExit();
    }
    
    void
    LazyOrAtExitFunction::add( const f_functor& f )
    {
        LazyOrAtExitFunction::getSingleton().priv_add( f );
    }

    void
    LazyOrAtExitFunction::priv_add( const f_functor& f )
    {
        m_flist.push_back( f );
        reconnectTimer();
    }
    
    LazyOrAtExitFunction&
    LazyOrAtExitFunction::getSingleton()
    {
        static LazyOrAtExitFunction ret;
        return ret;
    }
    
    static void LazyOrAtExitFunction_AtExit()
    {
        LazyOrAtExitFunction::getSingleton().atExit();
    }

    void
    LazyOrAtExitFunction::atExit()
    {
        for( m_flist_t::iterator iter = m_flist.begin(); iter != m_flist.end(); ++iter )
        {
            (*iter)( true );
        }
        m_flist.clear();
    }

    static gint s_timer_f(gpointer data)
    {
        LazyOrAtExitFunction* sp = (LazyOrAtExitFunction*)data;
        sp->atTimer();
        return 0;
    }
    
    void
    LazyOrAtExitFunction::atTimer()
    {
        for( m_flist_t::iterator iter = m_flist.begin(); iter != m_flist.end(); ++iter )
        {
            (*iter)( false );
        }
        m_flist.clear();
    }

    void
    LazyOrAtExitFunction::reconnectTimer()
    {
        if( m_timer )
        {
            g_source_remove( m_timer );
            m_timer = 0;
        }
        if( m_timer_interval )
            m_timer = g_timeout_add( m_timer_interval,
                                     GSourceFunc(s_timer_f), this );
    }
    
    void
    LazyOrAtExitFunction::setupAtExit()
    {
        static bool alreadySetup = false;

        if( !alreadySetup )
        {
            alreadySetup = true;
            
            int rc = atexit( LazyOrAtExitFunction_AtExit );
            if( rc != 0 )
            {
                // error
                cerr << "ERROR: can not register atexit() function." << endl;
                cerr << "In some cases implicit sync() calls will not be effective" << endl;
            }
        }
    }
};

    
namespace Ferris
{
    const string EMBLEM_TOPLEVEL_SYSTEM_NAME = "libferris";
    
    const string DB_COLDEM  = "/cold-emblems.db";

    
    const string EM_NEXTID_KEY = "nextid";
    const string EM_NEXTID_DEFAULT = "1";

    const string EM_NEXTID_EAORDERING_KEY = "nextid-eaordering";
    const string EM_NEXTID_EAORDERING_DEFAULT = "1073741824";
    const int    EM_NEXTID_EAORDERING_DEFAULT_INT = 1073741824;
    
    const string COLDEM_PAYLOAD_PREKEY = "payload-";

    
    const string DB_ETAGERE  = "/etagere.db";
    const string DB_EMBLEMS_AND_ETAGERE_BOOST  = "/emblems-and-etagere.boost";
    const string DB_EMBLEMS_AND_ETAGERE_BOOST_TXT  = "/emblems-and-etagere.boost.txt";
    const string ETAGERE_ONTOLOGY_ID_EXISTS_KEY = "ontology-id-exists";
    const string ETAGERE_ONTOLOGY_ID_KEY = "ontology-id";
    const string ETAGERE_IDLIST_KEY = "et-idlist";

    const string ETAGERE_ROOT = getDotFerrisPath() + "etagere";

    bool contains( emblems_t& el, fh_emblem em )
    {
        return el.end() != find( el.begin(), el.end(), em );
    }
    
    bool fh_emblem_less::operator()( fh_emblem k1, fh_emblem k2 )
    {
        return k1->getID() < k2->getID();
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    stringlist_t tostrlist( const std::set< emblemID_t >& el )
    {
        stringlist_t ret;
        for( std::set< emblemID_t >::const_iterator ei = el.begin();
             ei != el.end(); ++ei )
        {
            ret.push_back( tostr(*ei) );
        }
        return ret;
    }

    std::set< emblemID_t > toidset( const stringlist_t& sl )
    {
        std::set< emblemID_t > ret;
        for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
        {
            ret.insert( toType<emblemID_t>(*si) );
        }
        return ret;
    }

    static void addAllParentsTransitive( emblems_t& ret, fh_emblem em,
                                         Emblem::limitedViewPri_t cutoff = Emblem::LIMITEDVIEW_PRI_LOW )
    {
        if( em->getLimitedViewPriority() >= cutoff )
            ret.push_back( em );
        emblems_t p = em->getParents();
        for( emblems_t::iterator ci = p.begin(); ci != p.end(); ++ci )
        {
            addAllParentsTransitive( ret, *ci, cutoff );
        }
    }

    static void addAllChildrenTransitive( emblems_t& ret, fh_emblem em,
                                          Emblem::limitedViewPri_t cutoff = Emblem::LIMITEDVIEW_PRI_LOW )
    {
        if( em->getLimitedViewPriority() >= cutoff )
            ret.push_back( em );
        emblems_t p = em->getChildren();
        for( emblems_t::iterator ci = p.begin(); ci != p.end(); ++ci )
        {
            if( (*ci)->getLimitedViewPriority() >= cutoff )
                ret.push_back( *ci );
            addAllChildrenTransitive( ret, *ci, cutoff );
        }
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    link( fh_emblem parent, fh_emblem child )
    {
        parent->addChild( child );
        child->addParent( parent );
    }
    
    void
    unlink( fh_emblem parent, fh_emblem child )
    {
        parent->removeChild( child );
        child->removeParent( parent );
    }
    

    Emblem::Emblem( fh_etagere et, emblemID_t eid )
        :
        m_et( et ),
        m_id( eid ),
        m_name( "new emblem" ),
        m_iconname( "icons://unknown.png" ),
        m_menuSizedIconName( "icons://unknown-mu.png" ),
        m_description( "" ),
        m_digitalLatitude( 0 ),
        m_digitalLongitude( 0 ),
        m_zoomRange( 0 ),
        m_limitedViewPriority( LIMITEDVIEW_PRI_DEFAULT ),
        m_upset_cache_only_isDirty( true ),
        m_isTransitiveChildOfEAOrderingRootEmblem_isValid( false ),
        m_isTransitiveChildOfEAOrderingRootEmblem( false )
    {
    }

    emblemID_t UseGetNextID( Emblem* e )
    {
        return e->getNextID();
    }
    emblemID_t UseGetNextID_EAOrdering( Emblem* e )
    {
        return e->getNextID_EAOrdering();
    }
        

    
    void
    Emblem::setup( getNextIDFunctor_t f )
    {
        if( m_id )  load();
        else        m_id = f( this );
    }
    
    void
    Emblem::setDirty()
    {
        m_upset_cache_only_isDirty = true;
        m_et->setDirty();
    }
    
    bool
    Emblem::isDirty()
    {
        return m_et->isDirty();
    }
    
    emblemID_t
    Emblem::getNextID()
    {
        setDirty();

//         string s = get_db4_string( m_et->getDBPath(),
//                                    EM_NEXTID_KEY,
//                                    EM_NEXTID_DEFAULT, false );

        string s;
        fh_database db = m_et->getDB();
        s = db->getWithDefault( EM_NEXTID_KEY, s, EM_NEXTID_DEFAULT );

        LG_EMBLEM_D << "Emblem::getNextID() s:" << s
                    << " def:" << EM_NEXTID_DEFAULT
                    << endl;
        
        emblemID_t ret = toType<emblemID_t>( s );
//        set_db4_string( m_et->getDBPath(), EM_NEXTID_KEY, tostr(ret+1) );
        db->set( EM_NEXTID_KEY, tostr(ret+1) );
        db->sync();
        return ret;
    }

    emblemID_t
    Emblem::getNextID_EAOrdering()
    {
        setDirty();

        string s;
        fh_database db = m_et->getDB();
        s = db->getWithDefault( EM_NEXTID_EAORDERING_KEY, s, EM_NEXTID_EAORDERING_DEFAULT );

        LG_EMBLEM_D << "Emblem::getNextID() s:" << s
                    << " def:" << EM_NEXTID_EAORDERING_DEFAULT
                    << endl;
        
        emblemID_t ret = toType<emblemID_t>( s );
        db->set( EM_NEXTID_EAORDERING_KEY, tostr(ret+1) );
        db->sync();
        return ret;
    }
    
    
    void
    Emblem::load_common( fh_stringstream& iss )
    {
        stringmap_t m;
        XML::readMessage( iss, m );

//         cerr << "Emblem::load_common() dump" << endl;
//         for(stringmap_t::const_iterator mi = m.begin(); mi!=m.end(); ++mi )
//             cerr << " k:" << mi->first << " v:" << mi->second;
        
        
        m_name         = m["name"];
        m_iconname     = m["iconname"];
        m_menuSizedIconName = m[ "menusizediconname" ];
        m_description  = m["description"];
        if( m.end() != m.find("viewpri"))
            m_limitedViewPriority = (limitedViewPri_t)(toint(m["viewpri"]));
        
        m_parents  = toidset( Util::parseCommaSeperatedList( m["parents"] ) );
        m_children = toidset( Util::parseCommaSeperatedList( m["children"] ) );

        LG_EMBLEM_D << "Emblem::load_common() id:" << m_id
                    << " pri:" << m_limitedViewPriority
                    << " children:" << m["children"]
                    << endl;
    }

    void
    Emblem::save_common( fh_ostream oss )
    {
        stringlist_t sl;
        stringmap_t m;
        m["name"]         = m_name;
        m["iconname"]     = m_iconname;
        m[ "menusizediconname" ] = m_menuSizedIconName;
        m["description"]  = m_description;
        m["viewpri"]      = tostr(m_limitedViewPriority);
        sl                = tostrlist( m_parents );
        m["parents"]      = Util::createCommaSeperatedList( sl );
        sl                = tostrlist( m_children );
        m["children"]     = Util::createCommaSeperatedList( sl );
        XML::writeMessage( oss, m );

        LG_EMBLEM_D << "Emblem::save_common() id:" << m_id
                    << " pri:" << m_limitedViewPriority
                    << " parents.sz:" <<  m_parents.size()
                    << " children.sz:" <<  m_children.size()
                    << " children:" <<  Util::createCommaSeperatedList( sl )
                    << endl;
    }

    void
    Emblem::save( bool force )
    {
        if( force || isDirty() )
            priv_save();
    }
    
    void
    Emblem::load()
    {
        priv_load();
    }
    
    
    std::string
    Emblem::getName()
    {
        return m_name;
    }

    std::string
    Emblem::getUniqueName()
    {
        if( !m_uniqName.empty() )
            return m_uniqName;

        
        string name = getName();
        LG_EMBLEM_D << "getUniqueName() determining name... n:" << name << endl;
        
//         int count = 0;
//         emblems_t el = m_et->getAllEmblems();
//         for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
//         {
//             if( (*ei)->getName() == name )
//                 ++count;
//         }

        emblemset_t matches;
        m_et->getAllEmblemsWithName( matches, name );
        int count = matches.size();
        
        
        if( count > 1 || name.find("-") != string::npos )
        {
            name += "-";
            name += tostr(getID());
        }

        m_uniqName = name;
        setDirty();
        return name;
    }
    
    std::string
    Emblem::getIconName()
    {
        return m_iconname;
    }

    std::string
    Emblem::getMenuSizedIconName()
    {
        return m_menuSizedIconName;
    }
    
    
    std::string
    Emblem::getDescription()
    {
        return m_description;
    }

    void
    Emblem::setName( const std::string& v )
    {
        setDirty();
        m_et->priv_emblemBeingRenamed( (ColdEmblem*)this, m_name, v );
        m_name = v;

        // force an update of the uniqueName because we have changed our primary name.
        m_uniqName = "";
        getUniqueName();
    }
    
    void
    Emblem::setIconName( const std::string& v )
    {
        setDirty();
        getIconName_Changed_Sig().emit( this, m_iconname, v );
        m_iconname = v;
    }

    void
    Emblem::setMenuSizedIconName( const std::string& v )
    {
        setDirty();
        getMenuSizedIconName_Changed_Sig().emit( this, m_menuSizedIconName, v );
        m_menuSizedIconName = v;
    }
    
    Emblem::IconName_Changed_Sig_t&
    Emblem::getIconName_Changed_Sig()
    {
        return IconName_Changed_Sig;
    }

    Emblem::IconName_Changed_Sig_t&
    Emblem::getMenuSizedIconName_Changed_Sig()
    {
        return MenuSizedIconName_Changed_Sig;
    }
    
    Emblem::AddedParent_Sig_t&
    Emblem::getAddedParent_Sig()
    {
        return AddedParent_Sig;
    }

    Emblem::AddedChild_Sig_t&
    Emblem::getAddedChild_Sig()
    {
        return AddedChild_Sig;
    }

    Emblem::RemovedParent_Sig_t&
    Emblem::getRemovedParent_Sig()
    {
        return RemovedParent_Sig;
    }
    
    Emblem::RemovedChild_Sig_t&
    Emblem::getRemovedChild_Sig()
    {
        return RemovedChild_Sig;
    }
    
    void
    Emblem::setDescription( const std::string& v )
    {
        setDirty();
        m_description = v;
    }

    Emblem::limitedViewPri_t
    Emblem::getLimitedViewPriority()
    {
        return m_limitedViewPriority;
    }
    
    void
    Emblem::setLimitedViewPriority( limitedViewPri_t v )
    {
        setDirty();
        m_limitedViewPriority = v;
    }

    double
    Emblem::getDigitalLatitude()
    {
//        cerr << "Emblem::getDigitalLatitude():" << m_digitalLatitude << endl;
        return m_digitalLatitude;
    }
    
    double
    Emblem::getDigitalLongitude()
    {
        return m_digitalLongitude;
    }
        
    void
    Emblem::setDigitalLatitude( double v )
    {
//        cerr << "Emblem::setDigitalLatitude():" << v << endl;
        
        setDirty();
        m_digitalLatitude = v;
    }
    void
    Emblem::setDigitalLongitude( double v )
    {
        setDirty();
        m_digitalLongitude = v;
    }

    double
    Emblem::getZoomRange()
    {
        return m_zoomRange;
    }
        
    void
    Emblem::setZoomRange( double v )
    {
        setDirty();
        m_zoomRange = v;
    }
    
    
    
    
    emblems_t
    Emblem::getParents()
    {
        emblems_t ret;
        for( m_parents_t::const_iterator ci = m_parents.begin(); ci != m_parents.end(); ++ci )
        {
            ret.push_back( m_et->getEmblemByID( *ci ) );
        }
        return ret;
    }
    
    emblems_t
    Emblem::getChildren()
    {
        emblems_t ret;
        for( m_parents_t::const_iterator ci = m_children.begin(); ci != m_children.end(); ++ci )
        {
            if( !m_et->getEmblemByID( *ci ) )
                cerr << "getChildren() this:" << getID() << " CAN NOT FIND CHILD:" << *ci << endl;
            
            ret.push_back( m_et->getEmblemByID( *ci ) );
        }
        return ret;
    }
    bool
    Emblem::hasChildren()
    {
        return !m_children.empty();
    }
    


    void
    Emblem::forceUpdateParentsAndChildrenIDs()
    {
        for( m_parents_t::const_iterator ci = m_parents.begin(); ci != m_parents.end(); )
        {
            m_parents_t::const_iterator t = ci;
            ++ci;
            if( !m_et->getEmblemByID( *t ) )
            {
                setDirty();
                m_parents.erase( *t );
            }
        }
        for( m_parents_t::const_iterator ci = m_children.begin(); ci != m_children.end(); )
        {
            m_parents_t::const_iterator t = ci;
            ++ci;
            if( !m_et->getEmblemByID( *t ) )
            {
                setDirty();
                m_children.erase( *t );
            }
        }
    }
    
    
    typedef map< emblemID_t, fh_emblem > col_t;
    static void x_addAllParentsTransitive( col_t& ret, fh_emblem em,
                                           Emblem::limitedViewPri_t cutoff = Emblem::LIMITEDVIEW_PRI_LOW )
    {
        if( !em )
            return;
        
        if( em->getLimitedViewPriority() >= cutoff )
            ret[ em->getID() ] = em;
        
        emblems_t p = em->getParents();
        for( emblems_t::iterator ci = p.begin(); ci != p.end(); ++ci )
        {
            x_addAllParentsTransitive( ret, *ci, cutoff );
        }
    }
    
    const emblems_t&
    Emblem::getUpset()
    {
        if( !m_upset_cache_only_isDirty )
        {
            return m_upset_cache_only;
        }
        
        m_upset_cache_only.clear();

        col_t col;
        x_addAllParentsTransitive( col, this );
        col_t::const_iterator end = col.end();
        for( col_t::const_iterator iter = col.begin(); iter != end; ++iter )
        {
            m_upset_cache_only.push_back( iter->second );
        }
        
        
//         addAllParentsTransitive( ret, this );
//         ret.sort();
//         emblems_t::iterator e = unique( ret.begin(), ret.end() );
//         ret.erase( e, ret.end() );

        m_upset_cache_only_isDirty = false;
        return m_upset_cache_only;
    }
    
    emblems_t
    Emblem::getDownset()
    {
        emblems_t ret;
        addAllChildrenTransitive( ret, this );
        ret.sort();
        emblems_t::iterator e = unique( ret.begin(), ret.end() );
        ret.erase( e, ret.end() );
        return ret;
    }

    static void dump( string msg, emblems_t el )
    {
        LG_EMBLEM_D << "dump(start) --- " << msg << endl;
        for( emblems_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
        {
            fh_emblem em = *ei;
            LG_EMBLEM_D << " em:" << em->getName() << " id:" << em->getID() << endl;
        }
        LG_EMBLEM_D << "dump(end) --- " << msg << endl;
    }
    
    
    emblems_t
    Emblem::getPossibleParents()
    {
        emblems_t tmp;
        emblems_t ret     = m_et->getAllEmblems();
        emblems_t upset   = getUpset();
        emblems_t downset = getDownset();
        emblems_t parents = getParents();

        ret.sort();
        upset.sort();
        downset.sort();
        parents.sort();
        
//         dump( "ret", ret );
//         dump( "upset", upset );
//         dump( "downset", downset );
//         dump( "parents", parents );
        
        set_difference( ret.begin(),   ret.end(),
                        upset.begin(), upset.end(),
                        back_inserter( tmp ));
        ret = tmp; tmp.clear();

        set_difference( ret.begin(),     ret.end(),
                        downset.begin(), downset.end(),
                        back_inserter( tmp ));
        ret = tmp; tmp.clear();

        set_union( ret.begin(),     ret.end(),
                   parents.begin(), parents.end(),
                   back_inserter( tmp ));
        ret = tmp; tmp.clear();

        emblems_t::iterator self = find( ret.begin(), ret.end(), this );
        if( self != ret.end() )
            ret.erase( self );
        
        return ret;
    }

    emblems_t
    Emblem::getPossibleChildren()
    {
        emblems_t tmp;
        emblems_t ret      = m_et->getAllEmblems();
        emblems_t upset    = getUpset();
        emblems_t children = getChildren();

        ret.sort();
        upset.sort();
        children.sort();

        set_difference( ret.begin(),   ret.end(),
                        upset.begin(), upset.end(),
                        back_inserter( tmp ));
        ret = tmp; tmp.clear();

        set_union( ret.begin(),      ret.end(),
                   children.begin(), children.end(),
                   back_inserter( tmp ));
        ret = tmp; tmp.clear();
        
        emblems_t::iterator self = find( ret.begin(), ret.end(), this );
        if( self != ret.end() )
            ret.erase( self );
        
        return ret;
    }
    
    void
    Emblem::addParent( fh_emblem em )
    {
        // transitive sanity check
        emblems_t upset = getUpset();
        if( upset.end() != find( upset.begin(), upset.end(), em ) )
        {
            fh_stringstream ss;
            ss << "Can not add parent emblem:" << em->getName()
               << " because it is already included in the upset of:" << getName();
            Throw_CanNotAddEmblemException( tostr(ss), 0 );
        }
        
        setDirty();
        m_parents.insert( em->getID() );
        getAddedParent_Sig().emit( this, em );
        getAddedChild_Sig().emit( em, this );
        if( em == private_getAttributeRootEmblem( m_et ) ||
            em->isTransitiveChildOfEAOrderingRootEmblem() )
        {
            updateTransitiveChildrenProperty_ChildOfEAOrderingRootEmblem( true );
        }
    }
    
    void
    Emblem::addChild( fh_emblem em )
    {
        // transitive sanity check
        emblems_t downset = getDownset();
        if( downset.end() != find( downset.begin(), downset.end(), em ) )
        {
            fh_stringstream ss;
            ss << "Can not add child emblem:" << em->getName()
               << " because it is already included in the downset of:" << getName();
            Throw_CanNotAddEmblemException( tostr(ss), 0 );
        }

        setDirty();
        m_children.insert( em->getID() );
        getAddedChild_Sig().emit( this, em );
        getAddedParent_Sig().emit( em, this );
    }

    void
    Emblem::updateTransitiveChildrenProperty_ChildOfEAOrderingRootEmblem( bool v )
    {
        m_isTransitiveChildOfEAOrderingRootEmblem_isValid = true;
        m_isTransitiveChildOfEAOrderingRootEmblem = v;

        fh_emblem eaOrderingRootEM = private_getAttributeRootEmblem( m_et );
        
        emblems_t el = getDownset();
        emblems_t::iterator e = el.end();
        for( emblems_t::iterator ei = el.begin(); ei != e; ++ei )
        {
            if( !v )
            {
                // Make sure a transitive child can not still reach eaOrderingRootEM
                // by another path before removing this property
                const emblems_t& upset = (*ei)->getUpset();
                if( find( upset.begin(), upset.end(), eaOrderingRootEM ) != upset.end() )
                {
                    (*ei)->m_isTransitiveChildOfEAOrderingRootEmblem_isValid = true;
                    (*ei)->m_isTransitiveChildOfEAOrderingRootEmblem = true;
                    continue;
                }
            }
            
            (*ei)->m_isTransitiveChildOfEAOrderingRootEmblem_isValid = true;
            (*ei)->m_isTransitiveChildOfEAOrderingRootEmblem = v;
        }
    }
    
    
    void
    Emblem::removeParent( fh_emblem em )
    {
        setDirty();
        m_parents.erase( em->getID() );
        
        if( em == private_getAttributeRootEmblem( m_et ) )
        {
            updateTransitiveChildrenProperty_ChildOfEAOrderingRootEmblem( false );
        }
    }
    void
    Emblem::removeChild( fh_emblem em )
    {
        setDirty();
        m_children.erase( em->getID() );
    }
    
    
    bool
    Emblem::hasParent( fh_emblem em )
    {
        return m_parents.find( em->getID() ) != m_parents.end();
    }
    
    bool
    Emblem::hasChild( fh_emblem em )
    {
        return m_children.find( em->getID() ) != m_children.end();
    }

    struct FERRISEXP_DLLLOCAL EmblemByName
    {
        string name;
        EmblemByName( string n )
            :
            name(n)
            {
            }
        bool operator()( fh_emblem em )
            {
                return em->getName() == name;
            }
    };

    fh_emblem
    Emblem::findChild( const std::string& name )
    {
        emblems_t ch = getChildren();
        emblems_t::iterator ei = find_if( ch.begin(), ch.end(),
                                          EmblemByName( name ) );
        if( ei != ch.end() )
        {
            return *ei;
        }

        fh_stringstream ss;
        ss << "no emblem found with given name:" << name;
        Throw_EmblemNotFoundException( tostr(ss), 0 );
    }
    
    fh_emblem
    Emblem::obtainChild( const std::string& name )
    {
        emblems_t ch = getChildren();
        emblems_t::iterator ei = find_if( ch.begin(), ch.end(),
                                          EmblemByName( name ));
        if( ei != ch.end() )
        {
            return *ei;
        }

        fh_emblem ret = m_et->createColdEmblem( name );
        link( this, ret );
        return ret;
    }

    fh_emblem
    Emblem::obtainChild_EAOrdering( const std::string& name )
    {
        emblems_t ch = getChildren();
        emblems_t::iterator ei = find_if( ch.begin(), ch.end(),
                                          EmblemByName( name ));
        if( ei != ch.end() )
        {
            return *ei;
        }

        fh_emblem ret = m_et->createColdEmblem_EAOrdering( name );
        link( this, ret );
        return ret;
    }
    
    emblemID_t
    Emblem::getID() const
    {
        return m_id;
    }

    void
    Emblem::dumpTo( fh_ostream oss )
    {
        oss << "  <emblem id=\"" << m_id << "\" name=\"" << m_name << "\" " << endl
            << "       iconname=\"" << m_iconname << "\" "
            << "       menuiconname=\"" << m_menuSizedIconName << "\" "
            << "description=\"" << m_description << "\" >" << endl;
        oss << "      <parents>" << endl;
        for( m_parents_t::const_iterator ci = m_parents.begin(); ci != m_parents.end(); ++ci )
            oss << "          <emblemref id=\"" << *ci << "\" />" << endl;
        oss << "      </parents>" << endl;
        oss << "      <children>" << endl;
        for( m_parents_t::const_iterator ci = m_children.begin(); ci != m_children.end(); ++ci )
            oss << "          <emblemref id=\"" << *ci << "\" />" << endl;
        oss << "      </children>" << endl;
        oss << "  </emblem>" << endl;
    }

    void
    Emblem::forceUpdateTransitiveChildOfEAOrderingRootEmblem()
    {
        setDirty();
        m_isTransitiveChildOfEAOrderingRootEmblem_isValid = true;
        m_isTransitiveChildOfEAOrderingRootEmblem = false;

        if( !m_parents.empty() )
        {
            fh_emblem   eaOrderingRootEM = private_getAttributeRootEmblem( m_et );
            const emblems_t& upset = getUpset();
            if( find( upset.begin(), upset.end(), eaOrderingRootEM ) != upset.end() )
            {
                m_isTransitiveChildOfEAOrderingRootEmblem = true;
            }
        }
    }
    
    bool
    Emblem::isTransitiveChildOfEAOrderingRootEmblem()
    {
//         cerr << "Emblem::isTransitiveChildOfEAOrderingRootEmblem() n:" << getName()
//              << " v:" << m_isTransitiveChildOfEAOrderingRootEmblem_isValid
//              << " ret:" << m_isTransitiveChildOfEAOrderingRootEmblem
//              << endl;
        if( m_isTransitiveChildOfEAOrderingRootEmblem_isValid )
        {
            return m_isTransitiveChildOfEAOrderingRootEmblem;
        }

        forceUpdateTransitiveChildOfEAOrderingRootEmblem();
        return m_isTransitiveChildOfEAOrderingRootEmblem;
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    std::string
    ColdEmblem::getDBPath()
    {
        string dbpath = m_et->getBasePath() + DB_COLDEM;
        return dbpath;
    }
    
    ColdEmblem::ColdEmblem( fh_etagere et, emblemID_t eid )
        :
        Emblem( et, eid )
    {
    }

    void ColdEmblem::setEtagere( fh_etagere et )
    {
        m_et = et;
    }
    
    // STLdb4::fh_database
    // ColdEmblem::getDBCache( const std::string& dbpath )
    // {
    //     static ColdEmblem::m_dbcache_t* m_dbcache = 0;
    //     if( !m_dbcache )
    //         m_dbcache = new ColdEmblem::m_dbcache_t;

    //     m_dbcache_t::iterator di = m_dbcache->find( dbpath );
    //     if( di == m_dbcache->end() )
    //     {
    //         fh_database db = new Database( dbpath );
    //         di = m_dbcache->insert( make_pair( dbpath, db ) ).first;
    //     }
    //     return di->second;
    // }
    
    STLdb4::fh_database
    ColdEmblem::getDB()
    {
        string dbpath = getDBPath();
        if( !m_db )
            m_db = new Database( dbpath );
        return m_db;
        
//        LG_EMBLEM_D << "ColdEmblem::getDB() path:" << dbpath << endl;
//        return getDBCache( dbpath );
    }
    
    fh_cemblem
    ColdEmblem::Load( fh_etagere et, emblemID_t eid, getNextIDFunctor_t f )
    {
        fh_cemblem ret = new ColdEmblem( et, eid );
        ret->setup( f );
        return ret;
    }

//     fh_cemblem
//     ColdEmblem::Load_EAOrdering( fh_etagere et, emblemID_t eid )
//     {
//         fh_cemblem ret = new ColdEmblem( et, eid );
//         ret->setup_EAOrdering();
//         return ret;
//     }
    

    void
    ColdEmblem::priv_load()
    {
        g_return_if_fail( m_id != 0 );

        string payloadkey = COLDEM_PAYLOAD_PREKEY + tostr(m_id);
        string s;
//         LG_EMBLEM_D << "ColdEmblem::priv_load() payloadkey:" << payloadkey
//              << " m_id:" << m_id
//              << " tostr(m_id):" << tostr(m_id)
//              << " tostrX(m_id):" << tostrX(m_id)
//              << endl;
        
        getDB()->get( payloadkey, s );
//         string s = get_db4_string( getDBPath(),
//                                    payloadkey, "", true );

        LG_EMBLEM_D << "ColdEmblem::priv_load() db:" << getDBPath()
                    << " key:" << payloadkey
                    << " data:" << s
                    << endl;
        
        fh_stringstream ss;
        ss << s;
        load_common( ss );
    }
    
    void
    ColdEmblem::priv_save()
    {
        g_return_if_fail( m_id != 0 );

        string payloadkey = COLDEM_PAYLOAD_PREKEY + tostr(m_id);
        fh_stringstream ss;
        save_common( ss );
//        set_db4_string( getDBPath(), payloadkey, tostr(ss) );
        getDB()->set( payloadkey, tostr(ss) );

        // FIXME: we should only sync after saving all instead of after saving each
        getDB()->sync();
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    Etagere::Etagere( const std::string& path )
        :
        m_basepath( path ),
        m_isDirty( false ),
        m_haveLoadedAll( false ),
        m_db( 0 )
    {
        if( path.empty() )
        {
            m_basepath = getDotFerrisPath() + "etagere";
        }
        LG_EMBLEM_D << "Etagere::Etagere() path:" << m_basepath << endl;
        loadAllEmblems();
    }
    

    Etagere::~Etagere()
    {
        sync();
    }

    void
    Etagere::dump()
    {
        LG_EMBLEM_D << "Etagere::dump(start)" << endl;
        for( m_cemblems_t::iterator ci = m_cemblems.begin();
             ci != m_cemblems.end(); ++ci )
        {
            LG_EMBLEM_D << "  id:" << ci->first << " name:" << ci->second->getName() << endl;
            m_cemblemsByName_t::const_iterator byName = m_cemblemsByName.find( ci->second->getName() );
            if( byName == m_cemblemsByName.end() )
            {
                LG_EMBLEM_ER << "  id:" << ci->first << " NOT FOUND in byName mapping too." << endl;
            }
        }
        LG_EMBLEM_D << "Etagere::dump(end)" << endl;
    }

    void EtagereSync( Etagere* et, SyncDelayer* )
    {
        LG_EMBLEM_D << "EtagereSync()" << endl;
        et->sync();
    }
    
    void
    Etagere::sync()
    {
        LG_EMBLEM_D << "Etagere::sync() sync delayer exists:" << SyncDelayer::exists() << endl;
        if( !SyncDelayer::exists() )
        {
            saveAllEmblems();
        }
        else
        {
            LG_EMBLEM_D << "Etagere::sync() SyncDelayer exists, delaying sync()" << endl;
            cerr << "Etagere::sync() SyncDelayer exists, delaying sync()" << endl;
            typedef Loki::Functor< void, LOKI_TYPELIST_2( Etagere*, SyncDelayer* ) > F;
            F f( EtagereSync );
            SyncDelayer::add( this, Loki::BindFirst( f, this ) );
        }
    }

    void
    Etagere::OnOutOfProcNewEmblemNotification( std::set< emblemID_t >& eset )
    {
        LG_EMBLEM_D << "Etagere::OnOutOfProcNewEmblemNotification() eset.sz:" << eset.size() << endl;
        std::ifstream ifs( getBoostSerializePath().c_str() );
        boost::archive::binary_iarchive archive( ifs );

        if( m_isDirty )
        {
            LG_EMBLEM_D << "Etagere::OnOutOfProcNewEmblemNotification(we are dirty, ignoring signal)"
                        << " eset.sz:" << eset.size() << endl;
            return;
        }

        guint32 m_cemblems_sz = 0;
        archive >> m_cemblems_sz;
        LG_EMBLEM_D << "boost cache loading m_cemblems_sz:" << m_cemblems_sz << endl;
        emblemID_t lastEID = 0;
        for( int i = 0; i < m_cemblems_sz; ++i )
        {
            fh_cemblem em = new ColdEmblem( 0, i );
            Emblem& e = *(GetImpl(em));
            archive >> e;
            em->setEtagere( this );

            if( eset.count( em->getID() ) )
            {
                LG_EMBLEM_D << "Found new emblem from out-of-proc signal eid:" << em->getID() << endl;
                m_cemblems.insert( make_pair( em->getID(), em ) );
                m_cemblemsByName.insert( make_pair( em->getName(), em ) );
                LG_EMBLEM_D << "loaded eid:" << em->getID() << " name:" << em->getName() << endl;
                if( em->getID() > lastEID )
                    lastEID = em->getID();

                // the parents are still unaware of the new child.
                emblems_t pl = em->getParents();
                for( emblems_t::iterator pi=pl.begin(); pi!=pl.end(); ++pi )
                {
                    (*pi)->m_children.insert( em->getID() );
                }
                

                
                eset.erase( em->getID() );
                getEmblemCreated_Sig().emit( this, em );
                getAddedChild_Sig().emit( this, em );
                
                return;
            }
        }

        if( !eset.empty() )
        {
            for( std::set< emblemID_t >::iterator ei = eset.begin(); ei != eset.end(); ++ei )
            {
                LG_EMBLEM_W << "Can not find new emblem from out-of-proc signal eid:" << *ei << endl;
            }
        }
    }

    template< class ArchiveClassT >
    void
    Etagere::loadAllEmblems( ArchiveClassT& archive )
    {
        LG_EMBLEM_D << "Etagere::loadAllEmblems() loading boost...1" << endl;
        guint32 m_cemblems_sz = 0;
        archive >> m_cemblems_sz;
        LG_EMBLEM_D << "Etagere::loadAllEmblems() loading boost...2" << endl;
        LG_EMBLEM_D << "boost cache loading m_cemblems_sz:" << m_cemblems_sz << endl;
        emblemID_t lastEID = 0;
        for( int i = 0; i < m_cemblems_sz; ++i )
        {
            fh_cemblem em = new ColdEmblem( 0, i );
            Emblem& e = *(GetImpl(em));
            archive >> e;
            em->setEtagere( this );
            m_cemblems.insert( make_pair( em->getID(), em ) );
            m_cemblemsByName.insert( make_pair( em->getName(), em ) );
            LG_EMBLEM_D << "loaded eid:" << em->getID() << " name:" << em->getName() << endl;
            if( em->getID() > lastEID && em->getID() < EM_NEXTID_EAORDERING_DEFAULT_INT )
                lastEID = em->getID();
        }
            
        {
            fh_database db = getDB();
            string s;
            s = db->getWithDefault( EM_NEXTID_KEY, s, EM_NEXTID_DEFAULT );
            emblemID_t WhatNextIDShouldBe = toType<emblemID_t>( s );
                
            if( lastEID > WhatNextIDShouldBe )
            {
                LG_EMBLEM_W << "Somehow the last emblem ID loaded was larger than what the nextID is.."
                            << " adjusting the nextID so no trouble will be caused."
                            << endl;
                db->set( EM_NEXTID_KEY, tostr(lastEID+1) );
                db->sync();
            }
        }
    }

    static bool& getSkipLoadingEmblems()
    {
        static bool v = false;
        return v;
    }

    bool setSkipLoadingEmblems( bool v )
    {
        bool ret = getSkipLoadingEmblems();
        getSkipLoadingEmblems() = v;
        return ret;
    }
    
    
    
    void
    Etagere::loadAllEmblems()
    {
//        cerr << "Etagere::loadAllEmblems() skip:" << getSkipLoadingEmblems() << endl;
        
        if( getSkipLoadingEmblems() )
        {
            m_haveLoadedAll = true;
            return;
        }

        if( m_haveLoadedAll )
            return;

        LG_EMBLEM_D << "Etagere::loadAllEmblems() top" << endl;

        bool useBoost = 0;
        
        {
            if( 0==access( getBoostSerializePathTxt().c_str(), R_OK)
                || 0==access( getBoostSerializePath().c_str(), R_OK) )
            {
                useBoost = 1;
            }
        }
        
        
        if( useBoost )
        {
            string txtConfig = getBoostSerializePathTxt();
            // if( 0==access( txtConfig.c_str(), R_OK))
            // {
            //     LG_EMBLEM_D << "Etagere::loadAllEmblems(txt) loading boost file at:"
            //                 << txtConfig << endl;
            //     std::ifstream ifs( txtConfig.c_str() );
            //     boost::archive::text_iarchive archive( ifs );
            //     loadAllEmblems( archive );
            // }
            // else
            {
                LG_EMBLEM_D << "Etagere::loadAllEmblems(B) loading boost file at:"
                            << getBoostSerializePath() << endl;
                try 
                {
                    std::ifstream ifs( getBoostSerializePath().c_str() );
                    boost::archive::binary_iarchive archive( ifs );
                    loadAllEmblems( archive );
                }
                catch( exception& e )
                {
                    if( 0==access( txtConfig.c_str(), R_OK))
                    {
                        LG_EMBLEM_W << "Etagere::loadAllEmblems(binary failed) e:" << e.what() << endl;
                        LG_EMBLEM_W << "Etagere::loadAllEmblems(binary failed, trying text) loading boost file at:"
                                    << txtConfig << endl;
                        std::ifstream ifs( txtConfig.c_str() );
                        boost::archive::text_iarchive archive( ifs );
                        loadAllEmblems( archive );
                    }
                    else
                    {
                        throw;
                    }
                }
                
            }

#if 0

            LG_EMBLEM_D << "Etagere::loadAllEmblems(B) loading boost file at:"
                        << getBoostSerializePath() << endl;
            std::ifstream ifs( getBoostSerializePath().c_str() );
            LG_EMBLEM_D << "Etagere::loadAllEmblems() loading boost...0" << endl;
            boost::archive::binary_iarchive archive( ifs );
            LG_EMBLEM_D << "Etagere::loadAllEmblems() loading boost...01" << endl;
            
            LG_EMBLEM_D << "Etagere::loadAllEmblems() loading boost...1" << endl;
            guint32 m_cemblems_sz = 0;
            archive >> m_cemblems_sz;
            LG_EMBLEM_D << "Etagere::loadAllEmblems() loading boost...2" << endl;
            LG_EMBLEM_D << "boost cache loading m_cemblems_sz:" << m_cemblems_sz << endl;
            emblemID_t lastEID = 0;
            for( int i = 0; i < m_cemblems_sz; ++i )
            {
                fh_cemblem em = new ColdEmblem( 0, i );
                Emblem& e = *(GetImpl(em));
                archive >> e;
                em->setEtagere( this );
                m_cemblems.insert( make_pair( em->getID(), em ) );
                m_cemblemsByName.insert( make_pair( em->getName(), em ) );
                LG_EMBLEM_D << "loaded eid:" << em->getID() << " name:" << em->getName() << endl;
                if( em->getID() > lastEID && em->getID() < EM_NEXTID_EAORDERING_DEFAULT_INT )
                    lastEID = em->getID();
            }
            
            {
                fh_database db = getDB();
                string s;
                s = db->getWithDefault( EM_NEXTID_KEY, s, EM_NEXTID_DEFAULT );
                emblemID_t WhatNextIDShouldBe = toType<emblemID_t>( s );
                
                if( lastEID > WhatNextIDShouldBe )
                {
                    LG_EMBLEM_W << "Somehow the last emblem ID loaded was larger than what the nextID is.."
                                << " adjusting the nextID so no trouble will be caused."
                                << endl;
                    db->set( EM_NEXTID_KEY, tostr(lastEID+1) );
                    db->sync();
                }
            }
            
            
            LG_EMBLEM_D << "boost streaming is OK!" << endl;
//            cerr << "completed loading etagere from boost file..." << endl;

#endif
        }
        else
        {
            fh_database db = getDB();
            try
            {
                string idstr;
                db->get( ETAGERE_IDLIST_KEY, idstr );
                LG_EMBLEM_D << "Etagere::loadAllEmblems() idlist:" << idstr << endl;
//            LG_EMBLEM_D << "Etagere::loadAllEmblems() idlist:" << idstr << endl;

                stringlist_t idlist = Util::parseCommaSeperatedList( idstr );

                for( stringlist_t::iterator si = idlist.begin(); si != idlist.end(); ++si )
                {
//                cerr << "Etagere::loadAllEmblems(si):" << *si << endl;
                    LG_EMBLEM_D << "Etagere::loadAllEmblems(si):" << *si << endl;
                    emblemID_t id = toType<emblemID_t>( *si );
                    try
                    {
                        fh_cemblem em = ColdEmblem::Load( this, id );
                        m_cemblems.insert( make_pair( em->getID(), em ) );
                        m_cemblemsByName.insert( make_pair( em->getName(), em ) );
                    }
                    catch( exception& e )
                    {
                        LG_EMBLEM_ER << "Etagere::loadAllEmblems E:" << e.what() << endl;
//                    cerr << "Etagere::loadAllEmblems E:" << e.what() << endl;
                        BackTrace();
                        throw;
                    }
                }
            }
            catch( exception& e )
            {
                LG_EMBLEM_ER << "Etagere::loadAllEmblems() e:" << e.what() << endl;
            
            }
            
        }
        
        // ensure there is a ontology ID
        // the first time setup there will be none and this
        // will automatically create one for the user.
        getOntologyID();
        
        m_haveLoadedAll = true;
    }

    template< class ArchiveClassT >
    void
    Etagere::saveAllEmblems( ArchiveClassT& archive )
    {
        guint32 m_cemblems_sz = m_cemblems.size();
        archive << m_cemblems_sz;
        for( m_cemblems_t::iterator ci = m_cemblems.begin();
             ci != m_cemblems.end(); ++ci )
        {
            fh_cemblem em = ci->second;
            LG_EMBLEM_D << "saving em:" << em->getName() << " id:" << em->getID() << endl;
            const Emblem& e = *(GetImpl(em));
            archive << e;
        }
    }
    
    void
    Etagere::saveAllEmblems( bool force )
    {
        if( getSkipLoadingEmblems() )
        {
            return;
        }
        
        if( !force && !m_isDirty )
        {
//             LG_EMBLEM_D << "Etagere::saveAllEmblems(exit early) force:" << force
//                         << " dirty:" << m_isDirty
//                         << endl;
            return;
        }
        
        
        LG_EMBLEM_D << "Etagere::saveAllEmblems(top)" << endl;
//        BackTrace();
        
        stringlist_t idlist;
        
        {
            {
                string txtConfig = getBoostSerializePathTxt();
                LG_EMBLEM_D << "saving boost file:" << txtConfig << endl;
                std::ofstream ofs( txtConfig.c_str() );
                boost::archive::text_oarchive archive( ofs );
                saveAllEmblems( archive );
            }
            
            {
                LG_EMBLEM_D << "saving boost file:" << getBoostSerializePath() << endl;
                std::ofstream ofs( getBoostSerializePath().c_str() );
                boost::archive::binary_oarchive archive( ofs );
                saveAllEmblems( archive );
            }
            LG_EMBLEM_D << "saved boost file:" << getBoostSerializePath() << endl;
        }

        /////////////////////////////////////////
        /////////////////////////////////////////
        /////////////////////////////////////////




            if( const gchar* p = g_getenv ("LIBFERRIS_SAVE_DB4_ETAGERE") )
            {
        
                for( m_cemblems_t::iterator ci = m_cemblems.begin();
                     ci != m_cemblems.end(); ++ci )
                {
                    idlist.push_back( tostr(ci->first) );
                    ci->second->save();
                }
                LG_EMBLEM_D << "Etagere::saveAllEmblems()"
                            << " Setting idlistkey:" << Util::createCommaSeperatedList( idlist )
                            << " in db file:" << getDBPath()
                            << endl;

                fh_database db = getDB();
                db->set( ETAGERE_IDLIST_KEY, Util::createCommaSeperatedList( idlist ) );
                db->sync();
            }
            
//         set_db4_string( getDBPath(), ETAGERE_IDLIST_KEY,
//                         Util::createCommaSeperatedList( idlist ) );

        m_isDirty = false;

        // Tell other apps that there are some new emblems for them to load.
        Factory::getPluginOutOfProcNotificationEngine().signalEtagereNewEmblems( this, m_newEmblemIDs );
    }

//     void EtagereSync( Etagere* et, bool v )
//     {
//         et->sync();
//     }
    
    void
    Etagere::setDirty()
    {
        m_isDirty = true;
//         typedef Loki::Functor< void, LOKI_TYPELIST_2( Etagere*, bool ) > F;
//         F f( EtagereSync );
//         LazyOrAtExitFunction::add( Loki::BindFirst( f, this ) );
    }
    
    bool
    Etagere::isDirty()
    {
        return m_isDirty;
    }
    
    std::string
    Etagere::getDBPath()
    {
        return getBasePath() + DB_ETAGERE;
    }

    std::string
    Etagere::getBoostSerializePath()
    {
        return CleanupURL( getBasePath() + DB_EMBLEMS_AND_ETAGERE_BOOST );
    }

    std::string
    Etagere::getBoostSerializePathTxt()
    {
        return CleanupURL( getBasePath() + DB_EMBLEMS_AND_ETAGERE_BOOST_TXT );
    }
    
    
    
    fh_database
    Etagere::getDB()
    {
        if( m_db )
            return m_db;

        LG_EMBLEM_D << "Etagere::getDB(1) path:" << getDBPath() << endl;
        m_db = new Database( getDBPath() );
        LG_EMBLEM_D << "Etagere::getDB(2) path:" << getDBPath() << endl;
        return m_db;
    }
    
    
    
//     fh_emblem
//     Etagere::getHotEmblem( hotEmblemID_t eid )
//     {
//         fh_stringstream ss;
//         ss << "hot emblems are currently not supported";
//         Throw_EmblemNotFoundException( tostr(ss), 0 );
//     }

    
    fh_cemblem
    Etagere::getColdEmblem( emblemID_t eid )
    {
//        LG_EMBLEM_D << "Etagere::getColdEmblem(top)" << endl;
        if( eid )
        {
            loadAllEmblems();
            
            m_cemblems_t::iterator ci = m_cemblems.find( eid );
            if( ci != m_cemblems.end() )
                return ci->second;

            if( m_haveLoadedAll )
            {
                // we have loaded all emblems and not found it
                // also they wanted it from disk because eid!=0
                // so its either never existed or they have deleted it
                return 0;
            }
            
        }

        cerr << "WARNING Etagere::getColdEmblem() id==0" << endl;
        BackTrace();
//         LG_EMBLEM_D << "Etagere::getColdEmblem() loading eid:" << eid
//              << " m_cemblems.sz:" << m_cemblems.size()
//              << endl;
        fh_cemblem em = ColdEmblem::Load( this, eid );
        m_cemblems.insert( make_pair( em->getID(), em ) );
        m_cemblemsByName.insert( make_pair( em->getName(), em ) );
        return em;
    }

    emblems_t&
    Etagere::getAllEmblemsWithName( emblems_t& ret, const std::string& name )
    {
        loadAllEmblems();

        pair<m_cemblemsByName_t::const_iterator,
            m_cemblemsByName_t::const_iterator> eq = m_cemblemsByName.equal_range( name );

        m_cemblemsByName_t::const_iterator iter = eq.first;
        for( ; iter != eq.second; ++iter )
        {
            ret.push_back( iter->second );
        }

        return ret;
    }

    emblemset_t&
    Etagere::getAllEmblemsWithName( emblemset_t& ret, const std::string& name )
    {
        loadAllEmblems();

        pair<m_cemblemsByName_t::const_iterator,
            m_cemblemsByName_t::const_iterator> eq = m_cemblemsByName.equal_range( name );

        m_cemblemsByName_t::const_iterator iter = eq.first;
        for( ; iter != eq.second; ++iter )
        {
            ret.insert( iter->second );
        }

        return ret;
    }
    
    /**
     * Get or Create the emblem
     */
    fh_emblem
    Etagere::obtainEmblemByName( const std::string& name )
    {
        try
        {
            return getEmblemByName( name );
        }
        catch( EmblemNotFoundException& e )
        {
            fh_emblem ret = createColdEmblem( name );
            sync();
            return ret;
        }
        catch( ... )
        {
            throw;
        }
    }
    
    

    fh_emblem
    Etagere::getEmblemByName( const std::string& name )
    {
        loadAllEmblems();

        m_cemblemsByName_t::const_iterator byNameIter = m_cemblemsByName.find( name );
        if( byNameIter != m_cemblemsByName.end() )
        {
            return byNameIter->second;
        }
        
//         for( m_cemblems_t::iterator ci = m_cemblems.begin();
//              ci != m_cemblems.end(); ++ci )
//         {
//             if( ci->second->getName() == name )
//                 return ci->second;
//         }

        fh_stringstream ss;
        ss << "no emblem found with given name:" << name;
        Throw_EmblemNotFoundException( tostr(ss), 0 );
    }

//     bool
//     Etagere::isEmblemNameUnique( const std::string& name )
//     {
//         int count = 0;
//         loadAllEmblems();

//         m_cemblemsByName_t::const_iterator byNameIter = m_cemblemsByName.find( name );
//         if( byNameIter != m_cemblemsByName.end() )
//         {
//             fh_emblem em = byNameIter->second;
//             return em->getName() == em->getUniqueName();
//         }
//         return true;
//     }
    
    fh_emblem
    Etagere::getEmblemByUniqueName( const std::string& name )
    {
        if( !contains( name, "-" ) )
            return getEmblemByName( name );

        string seid = name.substr( name.rfind('-')+1 );
        if( seid.empty() )
        {
            fh_stringstream ss;
            ss << "no emblem found with given name:" << name;
            Throw_EmblemNotFoundException( tostr(ss), 0 );
        }
        return getEmblemByID( toType<emblemID_t>( seid ));
    }

    
    fh_emblem
    Etagere::getEmblemByID( emblemID_t id )
    {
        return getColdEmblem( id );
    }
    
    emblems_t
    Etagere::getAllEmblems()
    {
        emblems_t ret;
        
        loadAllEmblems();
        for( m_cemblems_t::iterator ci = m_cemblems.begin(); ci != m_cemblems.end(); ++ci )
        {
            ret.push_back( ci->second );
        }

        return ret;
    }

    emblems_t
    Etagere::getAllEmblemsUniqueName()
    {
        stringset_t sset;
        emblems_t ret;
        
        loadAllEmblems();

        for( m_cemblems_t::iterator ci = m_cemblems.begin(); ci != m_cemblems.end(); ++ci )
        {
            string n = ci->second->getName();
            
            if( !sset.count( n ) )
            {
                ret.push_back( ci->second );
                sset.insert( n );
            }
        }


        // PURE DEBUG
//         {
//             emblems_t& emlist = ret;
//             LG_EMBLEM_D << "Uret Emblem list begin------------------" << endl;
//             for( emblems_t::iterator ei = emlist.begin(); ei != emlist.end(); ++ei )
//             {
//                 fh_emblem em = *ei;
//                 LG_EMBLEM_D << "em.list:" << em->getName() << endl;
//             }
//             LG_EMBLEM_D << "Uret Emblem list end--------------------" << endl;
//         }
        
        return ret;
    }

    void
    Etagere::visitAllEmblems( const f_emblemVisitor& constf )
    {
        f_emblemVisitor f = constf;
        for( m_cemblems_t::iterator ci = m_cemblems.begin(); ci != m_cemblems.end(); ++ci )
        {
            f( ci->second );
        }
    }
    
    string
    Etagere::getOntologyID()
    {
        fh_database db = getDB();
        try
        {
            string payload;
            db->get( ETAGERE_ONTOLOGY_ID_EXISTS_KEY, payload );
        }
        catch( exception& e )
        {
            // generate and save.
            string s = Util::makeUUID();
            db->set( ETAGERE_ONTOLOGY_ID_EXISTS_KEY, "1" );
            db->set( ETAGERE_ONTOLOGY_ID_KEY, s );
            db->sync();
        }

        try
        {
            string payload;
            db->get( ETAGERE_ONTOLOGY_ID_KEY, payload );
            return payload;
        }
        catch( exception& e )
        {
            // problem reading the onto-id
            fh_stringstream ss;
            ss << "Failed to load the ontology uuid from file:" << getDBPath()
               << " e:" << e.what();
            Throw_EtagereException( tostr(ss), 0 );
        }
        

        
//         bool shouldHaveUUID = toint(get_db4_string( getDBPath(),
//                                                     ETAGERE_ONTOLOGY_ID_EXISTS_KEY,
//                                                     "0", false ));
//         string s = get_db4_string( getDBPath(), ETAGERE_ONTOLOGY_ID_KEY, "", false );

//         if( shouldHaveUUID && s.empty() )
//         {
//             // problem reading the onto-id
//             fh_stringstream ss;
//             ss << "Failed to load the ontology uuid from file:" << getDBPath();
//             Throw_EtagereException( tostr(ss), 0 );
//         }
        
//         if( s.empty() )
//         {
//             // generate and save.
//             s = Util::makeUUID();
//             set_db4_string( getDBPath(), ETAGERE_ONTOLOGY_ID_EXISTS_KEY, "1" );
//             set_db4_string( getDBPath(), ETAGERE_ONTOLOGY_ID_KEY, s );
//         }
//         return s;
    }

    Emblem::limitedViewPri_t
    Etagere::getLowestEmblemPriorityToShow()
    {
        fh_database db = getDB();
        try
        {
            string payload;
            db->get( CFG_LOWEST_EMBLEM_PRI_TO_SHOW_K, payload );
            Emblem::limitedViewPri_t ret = Emblem::limitedViewPri_t(toint(( payload )));
            return ret;
        }
        catch( exception& e )
        {
            return Emblem::limitedViewPri_t(
                toint(( CFG_LOWEST_EMBLEM_PRI_TO_SHOW_DEFAULT )));
        }
        
//         string raw = get_db4_string( getDBPath(),
//                                      CFG_LOWEST_EMBLEM_PRI_TO_SHOW_K,
//                                      CFG_LOWEST_EMBLEM_PRI_TO_SHOW_DEFAULT,
//                                      false );
//         Emblem::limitedViewPri_t ret = Emblem::limitedViewPri_t(toint(( raw )));
//         return ret;
    }
    
    void
    Etagere::setLowestEmblemPriorityToShow( Emblem::limitedViewPri_t v )
    {
        fh_database db = getDB();
        db->set( CFG_LOWEST_EMBLEM_PRI_TO_SHOW_K, tostr( v ) );
//         set_db4_string( getDBPath(), CFG_LOWEST_EMBLEM_PRI_TO_SHOW_K, tostr( v ));
    }
    
    void
    Etagere::erase( fh_emblem em )
    {
        loadAllEmblems();
        setDirty();

        m_newEmblemIDs.erase( em->getID() );
        
        m_cemblems.erase( m_cemblems.find( em->getID() ));
        {
            m_cemblemsByName_t::iterator iter = m_cemblemsByName.find( em->getName() );
            if( iter != m_cemblemsByName.end() )
                m_cemblemsByName.erase( iter );
        }

        
        emblems_t pl = em->getParents();
        for( emblems_t::iterator pi = pl.begin(); pi!=pl.end(); ++pi )
        {
            unlink( *pi, em );
        }

        emblems_t cl = em->getChildren();
        for( emblems_t::iterator ci = cl.begin(); ci!=cl.end(); ++ci )
        {
            unlink( em, *ci );
        }

        if( em->getParents().size() == 0 )
        {
            getRemovedChild_Sig().emit( this, em );
        }
    }
    
    fh_cemblem
    Etagere::createColdEmblem_common( fh_cemblem em )
    {
        string name = em->getName();
        
        m_cemblems.insert( make_pair( em->getID(), em ) );
        m_cemblemsByName.insert( make_pair( em->getName(), em ) );
//        sync();
        LG_EMBLEM_D << "Etagere::createColdEmblem(about to fire) name:" << name << endl;
        getEmblemCreated_Sig().emit( this, em );
        LG_EMBLEM_D << "Etagere::createColdEmblem(fired) name:" << name << endl;
        getAddedChild_Sig().emit( this, em );

        // make sure the emblem caches this info
        em->getUniqueName();

        // there are no parents, this will just quickly setup the cache.
        em->isTransitiveChildOfEAOrderingRootEmblem();

        // cache the new emblemID for notification after a sync()
        m_newEmblemIDs.insert( em->getID() );
        
        return em;
    }

    fh_cemblem
    Etagere::createColdEmblem( const std::string& name )
    {
        setDirty();
        fh_cemblem em = ColdEmblem::Load( this );
        em->m_name = name;
        return createColdEmblem_common( em );
    }
    
    fh_cemblem
    Etagere::createColdEmblem_EAOrdering( const std::string& name )
    {
        setDirty();
        fh_cemblem em = ColdEmblem::Load( this, 0, &UseGetNextID_EAOrdering );
        em->m_name = name;
        return createColdEmblem_common( em );
    }
    
    
    std::string
    Etagere::getBasePath()
    {
        return m_basepath;
    }
    
    void
    Etagere::dumpTo( fh_ostream oss )
    {
        emblems_t el = getAllEmblems();
        oss << "<etagere>" << endl;
        for( emblems_t::const_iterator ei = el.begin(); ei != el.end(); ++ei )
        {
            (*ei)->dumpTo( oss );
        }
        oss << "</etagere>" << endl;
    }

    Etagere::EmblemCreated_Sig_t&
    Etagere::getEmblemCreated_Sig()
    {
        return EmblemCreated_Sig;
    }
    
    Etagere::AddedChild_Sig_t&
    Etagere::getAddedChild_Sig()
    {
        return AddedChild_Sig;
    }
    
    Etagere::RemovedChild_Sig_t&
    Etagere::getRemovedChild_Sig()
    {
        return RemovedChild_Sig;
    }


    
    
    void
    Etagere::priv_emblemBeingRenamed( fh_cemblem e,
                                      const std::string& oldn,
                                      const std::string& newn )
    {
        m_cemblemsByName.erase( oldn );
        m_cemblemsByName.insert( make_pair( newn, e ) );
    }
    



    
    namespace Factory 
    {
        static fh_etagere& get_etagereOverride()
        {
            static fh_etagere etagereOverride = 0;
            return etagereOverride;
        }
        static void sync_etagereOverride()
        {
            if( get_etagereOverride() )
                get_etagereOverride()->sync();
        }
        
        void setDefaultEtagere( fh_etagere et )
        {
            static bool registered = false;
            if( !registered )
            {
                registered = 1;
                atexit( sync_etagereOverride );
            }
            
            get_etagereOverride() = et;
        }

        static fh_etagere ensureRequiredEmblems( fh_etagere et )
        {
            fh_emblem eaordering = private_getAttributeRootEmblem( et );
            fh_emblem libferris  = getFerrisSystemEmblem( et );
            fh_emblem geospatial = libferris->obtainChild( "libferris-geospatial" );
            et->sync();
            
            return et;
        }
        
        fh_etagere getEtagere()
        {
            try
            {
                if( get_etagereOverride() )
                    return get_etagereOverride();
            
                static fh_etagere e = 0;
                if( !e )
                {
                    e = new Etagere();
                    ensureRequiredEmblems( e );
                }
                return e;
            }
            catch( exception& e )
            {
                LG_EMBLEM_ER << "ERROR: getEtagere() e:" << e.what() << endl;
                LG_EMBLEM_ER << endl;
                LG_EMBLEM_ER << "===> Make sure you have run: ferris-first-time-user --setup-defaults"
                     << endl;
                LG_EMBLEM_ER << endl;
                throw;
            }
        }

        fh_etagere makeEtagere( const std::string& path )
        {
            Shell::acquireContext( path );
            fh_etagere e = new Etagere( path );
            ensureRequiredEmblems( e );
            return e;
        }
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    Times::Times()
        :
        m_mtime( Time::getTime() ),
        m_atime( Time::getTime() )
    {
    }
        

    void
    Times::setATime( time_t t )
    {
        m_atime = t;
    }
    
    void
    Times::setMTime( time_t t )
    {
        m_mtime = t;
    }
    
    time_t
    Times::getATime()
    {
        return m_atime;
    }

    void
    Times::touchATime()
    {
        m_atime = Time::getTime();
    }
    
    void
    Times::touchMTime()
    {
        m_mtime = Time::getTime();
    }
    
    time_t
    Times::getMTime()
    {
        return m_mtime;
    }

    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    MedallionBelief::MedallionBelief( Medallion* m, fh_emblem em, fh_personality p, 
                                      double sureness, fh_times t )
        :
        m_medallion( m ),
        m_emblem( em ),
        m_personality( p ),
        m_sureness( sureness ),
        m_times( t ? t : new Times() )
    {
    }
    
    MedallionBelief::MedallionBelief()
        :
        m_medallion( 0 ),
        m_emblem( 0 ),
        m_personality( Factory::getCurrentUserPersonality() ),
        m_sureness( SURENESS_NULL ),
        m_times( new Times() )
    {
    }
    
    MedallionBelief::~MedallionBelief()
    {
    }

    fh_medallion
    MedallionBelief::getMedallion() const
    {
        return m_medallion;
    }
    
    fh_emblem
    MedallionBelief::getEmblem() const
    {
        return m_emblem;
    }
    
    fh_personality
    MedallionBelief::getPersonality() const
    {
        return m_personality;
    }
    
    fh_times
    MedallionBelief::getTimes() const
    {
        if( !m_times )
            m_times = new Times();
        
        return m_times;
    }

    double
    MedallionBelief::getSureness() const
    {
        return m_sureness;
    }

    void
    MedallionBelief::setSureness( double v )
    {
        m_sureness = v;
    }
    
    
    double
    MedallionBelief::clampJudgementSureness( double ret )
    {
        if( ret > MedallionBelief::SURENESS_MAX ) ret = MedallionBelief::SURENESS_MAX;
        if( ret < MedallionBelief::SURENESS_MIN ) ret = MedallionBelief::SURENESS_MIN;
        return ret;
    }

//     MedallionBelief::ref_count_t
//     MedallionBelief::AddRef()
//     {
//         if( getReferenceCount() >= 1 )
//             m_medallion->AddRef();
//         return _Base::AddRef();
//     }
        
//     MedallionBelief:: ref_count_t
//     MedallionBelief::Release()
//     {
//         if( getReferenceCount() > 1 )
//             m_medallion->Release();
//         return _Base::Release();
//     }
    
    /********************************************************************************/
    /********************************************************************************/
    
//     void
//     Medallion::JudgementList::add( fh_personality p,
//                                    double judgementSureness,
//                                    fh_times t )
//     {
//         MedallionBelief a( p, judgementSureness, t );
//         m_assertions[ p ] =  a;
//     }

//     fh_times
//     Medallion::JudgementList::getTimes( fh_personality p )
//     {
//         m_assertions_t::iterator ai = m_assertions.find( p );
//         if( ai != m_assertions.end() )
//         {
//             return ai->second.times;
//         }
//         return 0;
//     }
    
//     double
//     Medallion::JudgementList::getSureness( fh_personality p )
//     {
//         m_assertions_t::iterator ai = m_assertions.find( p );
//         if( ai != m_assertions.end() )
//         {
//             return ai->second.judgementSureness;
//         }
//         return MedallionBelief::SURENESS_NULL;
//     }

    fh_medallionBelief
    Medallion::createBelief( fh_emblem em, fh_personality p, double sureness, fh_times t )
    {
        if( !t )
            t = new Times();

        fh_medallionBelief ret = new MedallionBelief( this, em, p, sureness, t );
        m_assertions_t& m_assertions = m_cemblems_attach[ em ];
        m_assertions.insert( make_pair( p, ret ) );
        return ret;
    }

    fh_medallionBelief
    Medallion::obtainBelief( fh_emblem em, fh_personality p,
                             double sureness, fh_times t )
    {
        if( hasBelief( em, p ) )
            return getBelief( em, p );
        return createBelief( em, p, sureness, t );
    }
    
    void
    Medallion::eraseBelief( fh_emblem em, fh_personality p )
    {
        m_assertions_t& m_assertions = m_cemblems_attach[ em ];
        m_assertions.erase( p );
    }
    
    
//     fh_medallionBelief
//     Medallion::JudgementList::getBelief( Medallion* m, fh_emblem em, fh_personality p,
//                                          double sureness, fh_times t )
//     {
//         m_assertions_t::iterator ai = m_assertions.find( p );
//         if( ai == m_assertions.end() )
//         {
//             ai = m_assertions.insert(
//                 make_pair( p, m->createBelief( em, p, sureness, t ) ) ).first;
//         }
//         return ai->second;
//     }
    
//     void
//     Medallion::JudgementList::eraseBelief( fh_personality p )
//     {
//         m_assertions.erase( p );
//     }

//     bool
//     Medallion::JudgementList::hasBelief( fh_personality p )
//     {
//         return m_assertions.find( p ) != m_assertions.end();
//     }
    
//     std::list< fh_personality >
//     Medallion::JudgementList::getListOfPersonalitiesWhoHaveOpinion()
//     {
//         std::list< fh_personality > ret;

//         for( m_assertions_t::iterator ai = m_assertions.begin(); ai != m_assertions.end(); ++ai )
//             ret.push_back( ai->first );
        
//         return ret;
//     }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    Medallion::Medallion( fh_context c )
        :
        m_context( c ),
        m_isDirty( false ),
        m_dontSave( false )
    {
        load( m_context );
    }
    
    Medallion::~Medallion()
    {
//         cerr << "~Medallion() m_context:" << m_context->getURL() << endl;
//         BackTrace();
        sync();
    }

    void
    Medallion::setDirty()
    {
        m_isDirty = true;
    }

    void
    Medallion::sync()
    {
        if( m_isDirty )
        {
            save( m_context );
            LG_EMBLEM_D << "Medallion::sync() sending message to other procs." << endl;
            m_context->Emit_MedallionUpdated();
            Factory::getPluginOutOfProcNotificationEngine().signalMedallionUpdated( m_context );
            
#ifdef HAVE_DBUS
            if( m_context )
            {
                string earl = m_context->getURL();
                LG_DBUS_D << "Emitting MedallionSaved for earl:" << earl << endl;
                DBusConnection* conn = DBus::getSessionBus();
                DBus::Signal sig( "/", "org.libferris.desktop", "MedallionSaved");
                sig.push_back( earl );
                sig.push_back( m_context->getDirPath() );
                sig.send( conn );
                LG_DBUS_D << "Done Emitting MedallionSaved for earl:" << earl << endl;
            }
#endif                        

        }
    }

    void
    Medallion::reload()
    {
        if( m_isDirty )
        {
            LG_EMBLEM_ER << "ERROR: Medallion::reload() called on dirty medallion"
                 << " c:" << m_context->getURL()  << endl;
            return;
        }

        m_cemblems.clear();
        m_cemblems_attach.clear();
        LG_EMBLEM_D << "Medallion::reload() c:" << m_context->getURL() << endl;
        load( m_context );
    }
    
    
    
    string
    Medallion::getEAName()
    {
        string ret = "";
        uid_t uid = Shell::getUserID();
        ret = string("medallion.") + tostr(uid);
        return ret;
    }

    static const string VERSION_KEY      = "medallion-version";
    static const string VERSION_VALUE_V1 = "1";
    static const string VERSION_VALUE_V2 = "2";
    static const string V1_ONTOLOGY_ID   = "ontology-id";
    static const string V1_COLD_EMBLEM_IDLIST_KEY = "cold-emblem-id-list";
    static const string V1_MTIME_KEY     = "mtime";
    static const string V1_EMBLEM_LIST_EID_KEY   = "eid";
    static const string V1_EMBLEM_LIST_MTIME_KEY = "mtime";

    
    static const string V2_EMBLEM_LIST_ATIME_KEY    = "atime";
    static const string V2_EMBLEM_LIST_SURENESS_KEY = "sureness";
    static const string V2_EMBLEM_LIST_PERSONALITY_ID_KEY = "personality-id";

    
    void
    Medallion::load( fh_context c )
    {
        Factory::ensureXMLPlatformInitialized();
        fh_etagere et = Factory::getEtagere();

        string xml = "";

        try
        {
//             cerr << "medallion::load() c:" << c->getURL()
//                  << " getEAName():" << getEAName()
//                  << endl;
            xml = getStrAttr( c, getEAName(), "", true, true );
            LG_EMBLEM_D << "Medallion::load() medallion:" << xml << endl;
        }
        catch( NoSuchAttribute& e )
        {
            m_isDirty = false;
            return;
        }

        fh_stringstream xmlss;
        xmlss << xml;

//         stringmap_t m;
//         XML::readMessage( xmlss, m );
            

//         string version = m[ VERSION_KEY ];
//         if( version != VERSION_VALUE_V1 )
//         {
//             fh_stringstream ss;
//             ss << "Medallion exists but has incorrect version:" << version;
//             Throw_MedallionException( tostr(ss), GetImpl(c) );
//         }

//         if( m[ V1_ONTOLOGY_ID ] != et->getOntologyID() )
//         {
//             LG_EMBLEM_D << "Warning: found a medallion at:" << c->getURL() << endl
//                  << " but it is not in the native ontology, ignoring and preserving it." << endl;
//             m_isDirty = false;
//             m_dontSave = true;
//             return;
//         }
        
//         string idstr = m[ V1_COLD_EMBLEM_IDLIST_KEY ];
//         if( idstr.empty() )
//         {
//             LG_EMBLEM_D << "no medallion found for c:" << c->getURL() << endl;
//         }
//         else
//         {
//             LG_EMBLEM_D << "Medallion::load() found:" << idstr << endl;
//             stringlist_t idlist = Util::parseCommaSeperatedList( idstr );
//             m_cemblems.clear();
//             for( stringlist_t::iterator si = idlist.begin(); si != idlist.end(); ++si )
//             {
//                 LG_EMBLEM_D << "Medallion::load() found em:" << *si << endl;
//                 fh_cemblem em = et->getColdEmblem( toType<emblemID_t>( *si ) );
//                 m_cemblems.insert( em );
//             }
//         }

        
        fh_domdoc    doc  = Factory::StreamToDOM( xmlss );
        DOMElement*  root = doc->getDocumentElement();

        if( getAttribute( root, V1_ONTOLOGY_ID ) != et->getOntologyID() )
        {
            LG_EMBLEM_W << "Warning: found a medallion at:" << c->getURL() << endl
                        << " but it is not in the native ontology, ignoring and preserving it." << endl;
            m_isDirty = false;
            m_dontSave = true;
            return;
        }

        //
        // All previous on disk formats should be supported for reading.
        // medallions should be saved in the most recent on disk format.
        //
        string version = getAttribute( root, VERSION_KEY );
        fh_personality userper = Factory::getCurrentUserPersonality( et );
        
        if( version == VERSION_VALUE_V1 )
        {
            if( DOMElement* emblemlist = XML::getChildElement( root, "emblemlist" ))
            {
                typedef std::list< DOMElement* > el_t;
                el_t el = XML::getAllChildrenElements( emblemlist, "emblem", false );
                for( el_t::iterator ei = el.begin(); ei != el.end(); ++ei )
                {
                    string seid  = getAttribute( *ei, V1_EMBLEM_LIST_EID_KEY   );
                    string mtime = getAttribute( *ei, V1_EMBLEM_LIST_MTIME_KEY );

                    fh_cemblem em = et->getColdEmblem( toType<emblemID_t>( seid ) );
                    m_cemblems.insert( em );
                    fh_times ti = new Times();
                    ti->setMTime( toType<time_t>(mtime) );
//                    m_cemblems_times[ em ] = ti;
//                     m_cemblems_attach[ em ].add( userper,
//                                                  MedallionBelief::SURENESS_MAX,
//                                                  ti );
                    createBelief( em, userper, MedallionBelief::SURENESS_MAX, ti );
                }
            }
        }
        else if( version == VERSION_VALUE_V2 )
        {
            if( DOMElement* emblemlist = XML::getChildElement( root, "emblemlist" ))
            {
                typedef std::list< DOMElement* > el_t;
                el_t el = XML::getAllChildrenElements( emblemlist, "emblem", false );
                for( el_t::iterator emblemiter = el.begin(); emblemiter != el.end(); ++emblemiter )
                {
                    string seid  = getAttribute( *emblemiter, V1_EMBLEM_LIST_EID_KEY   );
                    fh_cemblem em = et->getColdEmblem( toType<emblemID_t>( seid ) );
                    m_cemblems.insert( em );
                    
                    el_t xbelieflist = XML::getAllChildrenElements( *emblemiter, "belief", false );
                    for( el_t::iterator pi = xbelieflist.begin(); pi != xbelieflist.end(); ++pi )
                    {
                        string mtime = getAttribute( *pi, V1_EMBLEM_LIST_MTIME_KEY );
                        string atime = getAttribute( *pi, V2_EMBLEM_LIST_ATIME_KEY );
                        string sureness = getAttribute( *pi, V2_EMBLEM_LIST_SURENESS_KEY );
                        string perid = getAttribute( *pi, V2_EMBLEM_LIST_PERSONALITY_ID_KEY );
                    
                        fh_times ti = new Times();
                        ti->setMTime( toType<time_t>(mtime) );
                        ti->setATime( toType<time_t>(atime) );
                        fh_personality per = obtainPersonality( toType<emblemID_t>( perid ) );
//                        m_cemblems_attach[ em ].add( per, toType<double>(sureness), ti );
//                         m_cemblems_attach[ em ].getBelief( this, em, per,
//                                                            toType<double>(sureness), ti );
                        createBelief( em, per, toType<double>(sureness), ti );
                        
                    }
                }
            }
        }
        else
        {
            fh_stringstream ss;
            ss << "Medallion exists but has incorrect version:" << version;
            Throw_MedallionException( tostr(ss), GetImpl(c) );
        }
        
        m_isDirty = false;
    }
    
    void
    Medallion::save( fh_context c )
    {
        if( m_dontSave )
            return;

        Factory::ensureXMLPlatformInitialized();
        fh_etagere et = Factory::getEtagere();

        /********************************************************************************/
        // VERSION_VALUE_V1
        /********************************************************************************/
//         DOMDocument* doc  = Factory::makeDOM( "medallion" );
//         DOMElement*  root = doc->getDocumentElement();
//         setAttribute( root, VERSION_KEY,    VERSION_VALUE_V1 );
//         setAttribute( root, V1_ONTOLOGY_ID, et->getOntologyID() );
//         setAttribute( root, V1_MTIME_KEY,   tostr(Time::getTime()) );
//         DOMElement* emblemlist = XML::createElement( doc, root, "emblemlist" );
//         for( m_cemblems_t::iterator ci = m_cemblems.begin();
//              ci != m_cemblems.end(); ++ci )
//         {
//             fh_emblem em = *ci;
//             DOMElement* e = XML::createElement( doc, emblemlist, "emblem" );
//             string  mtime = tostr(getEmblemTimes( em )->getMTime());
            
//             setAttribute( e, V1_EMBLEM_LIST_EID_KEY,   tostr(em->getID()) );
//             setAttribute( e, V1_EMBLEM_LIST_MTIME_KEY, mtime );
//         }
//         fh_stringstream ss = tostream( doc );
//         setStrAttr( c, getEAName(), tostr(ss), true );
//         m_isDirty = false;


        fh_domdoc    doc  = Factory::makeDOM( "medallion" );
        DOMElement*  root = doc->getDocumentElement();
        setAttribute( root, VERSION_KEY,    VERSION_VALUE_V2 );
        setAttribute( root, V1_ONTOLOGY_ID, et->getOntologyID() );
        setAttribute( root, V1_MTIME_KEY,   tostr(Time::getTime()) );
        DOMElement* emblemlist = XML::createElement( doc, root, "emblemlist" );
        for( m_cemblems_t::iterator ci = m_cemblems.begin();
             ci != m_cemblems.end(); ++ci )
        {
            fh_emblem em = *ci;
            DOMElement* e = XML::createElement( doc, emblemlist, "emblem" );
            setAttribute( e, V1_EMBLEM_LIST_EID_KEY, tostr(em->getID()) );

            typedef std::list< fh_personality > plist_t;
            plist_t plist = getListOfPersonalitiesWhoHaveOpinion( em );
            for( plist_t::iterator pi = plist.begin(); pi!=plist.end(); ++pi )
            {
                fh_personality     per = *pi;
                fh_medallionBelief bel = getBelief( em, per );
                
                DOMElement* pe = XML::createElement( doc, e, "belief" );
                string  mtime = tostr( bel->getTimes()->getMTime());
                string  atime = tostr( bel->getTimes()->getATime());
                fh_stringstream ss;
                ss << bel->getSureness();
                string  sureness = tostr(ss);
                string  perid = tostr(per->getID());
                
                setAttribute( pe, V2_EMBLEM_LIST_PERSONALITY_ID_KEY, perid );
                setAttribute( pe, V2_EMBLEM_LIST_SURENESS_KEY,       sureness );
                setAttribute( pe, V1_EMBLEM_LIST_MTIME_KEY,          mtime );
                setAttribute( pe, V2_EMBLEM_LIST_ATIME_KEY,          atime );
            }
        }
        fh_stringstream ss = tostream( doc );
        setStrAttr( c, getEAName(), tostr(ss), true, true, true );
        m_isDirty = false;
    }
    
    /********************************************************************************/
    /*** USER INTERFACE *************************************************************/
    /********************************************************************************/

    /**
     * Force the assertion that this medallion should contain the given emblem
     */ 
    void
    Medallion::addEmblem( fh_emblem em )
    {
        addEmblem( em,
                   Factory::getCurrentUserPersonality(),
                   MedallionBelief::SURENESS_MAX );
    }

    /**
     * Force the assertion that this medallion should not contain the given emblem
     */ 
    void
    Medallion::retractEmblem( fh_emblem em )
    {
        retractEmblem( em,
                       Factory::getCurrentUserPersonality(),
                       MedallionBelief::SURENESS_MIN );
    }
    
    /**
     * Remove previous assertions and retractions, allow lower level items to offer
     * hints on if this emblem shoudl be attached to the medallion but do not
     * override the choice one way or the other.
     */ 
    void
    Medallion::removeEmblem( fh_emblem em )
    {
        removeEmblem( em, Factory::getCurrentUserPersonality() );
    }
    
    bool
    Medallion::hasEmblem( fh_emblem em )
    {
        return hasEmblem( em, Factory::getCurrentUserPersonality() );
    }

    bool
    Medallion::hasRetractedEmblem( fh_emblem em )
    {
        LG_EMBLEM_D << "Medallion::hasRetractedEmblem() em:" << em->getName() << endl;

        if( hasBelief( em ) )
        {
            fh_medallionBelief b = getBelief( em );
            LG_EMBLEM_D << "Medallion::hasRetractedEmblem() em:" << em->getName()
                 << " sureness:" << b->getSureness()
                 << endl;
            return b->getSureness() < 0;
        }
        return false;
    }
    
    

    /**
     * if 'has' is true then make sure this medallion has em
     * is 'has' is false then make sure the med hasn't got the emblem
     */
    void
    Medallion::ensureEmblem( fh_emblem em, bool has )
    {
        if( has )
        {
            if( !hasEmblem( em ) )
                addEmblem( em );
        }
        else
        {
            if( hasEmblem( em ) )
                retractEmblem( em );
        }
    }
    
    /********************************************************************************/
    /*** AGENT INTERFACE ************************************************************/
    /********************************************************************************/

    /**
     * Agents can use a personality to assign emblems to files and give
     * a level of "sureness" that they feel this attachment should be done.
     */
    void
    Medallion::addEmblem( fh_emblem em, fh_personality pers, double judgementSureness )
    {
        setDirty();

        if( ColdEmblem* ce = dynamic_cast<ColdEmblem*>(GetImpl(em)))
            m_cemblems.insert( ce );

        if( judgementSureness < 0 )
            judgementSureness = -1 * judgementSureness;

        fh_medallionBelief a = obtainBelief( em, pers );
        a->setSureness( judgementSureness );
        a->getTimes()->touchATime();
        a->getTimes()->touchMTime();
    }
    
    void
    Medallion::retractEmblem( fh_emblem em, fh_personality pers, double judgementSureness )
    {
        setDirty();

        if( ColdEmblem* ce = dynamic_cast<ColdEmblem*>(GetImpl(em)))
            m_cemblems.insert( ce );
//         if( ColdEmblem* ce = dynamic_cast<ColdEmblem*>(GetImpl(em)))
//             m_cemblems.erase( m_cemblems.find( ce ) );

        if( judgementSureness > 0 )
            judgementSureness = -1 * judgementSureness;

        fh_medallionBelief a = obtainBelief( em, pers );
        a->setSureness( judgementSureness );
        a->getTimes()->touchATime();
        a->getTimes()->touchMTime();
    }

    void
    Medallion::removeEmblem( fh_emblem em, fh_personality pers )
    {
        setDirty();

        bool removeEmblemAttachment = false;
        m_assertions_t& m_assertions = m_cemblems_attach[ em ];
        if( m_assertions.size() <= 1 )
            removeEmblemAttachment = true;
        
        eraseBelief( em, pers );

        if( removeEmblemAttachment )
        {
            fh_cemblem cem = dynamic_cast<ColdEmblem*>(GetImpl(em));
            m_cemblems.erase(cem);
        }
    }
    
    
    bool
    Medallion::hasEmblem( fh_emblem em, fh_personality pers )
    {
        fh_cemblem cem = dynamic_cast<ColdEmblem*>(GetImpl(em));

        bool ret = m_cemblems.find( cem ) != m_cemblems.end();

        if( ret )
        {
            fh_medallionBelief b = obtainBelief( em, pers );
            b->getTimes()->touchATime();

            return b->getSureness() > 0;
        }

        return ret;
    }
    void
    Medallion::ensureEmblem( fh_emblem em, fh_personality pers,
                             double judgementSureness, bool has )
    {
        if( has )
        {
            if( !hasEmblem( em, pers ) )
                addEmblem( em, pers, judgementSureness );
        }
        else
        {
            if( hasEmblem( em, pers ) )
                retractEmblem( em, pers, judgementSureness );
        }
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    double
    Medallion::getFuzzyBelief( fh_emblem em )
    {
        try
        {
            fh_medallionBelief ub = getBelief( em );
            double s = ub->getSureness();
            if( s != MedallionBelief::SURENESS_NULL )
                return s;
        }
        catch( exception& e )
        {}
        
        double ret = MedallionBelief::SURENESS_NULL;
        int count = 0;
        
        typedef std::list< fh_personality > plist_t;
        plist_t plist = getListOfPersonalitiesWhoHaveOpinion( em );

        for( plist_t::iterator pi = plist.begin(); pi!=plist.end(); ++pi )
        {
            fh_medallionBelief bel = getBelief( em, *pi );
            ret += bel->getSureness();
            ++count;
        }

        if( count )
            ret /= count;
        
        return MedallionBelief::clampJudgementSureness( ret );
    }
    
    
//     fh_times
//     Medallion::getEmblemTimes( fh_emblem em, fh_personality pers )
//     {
//         if( !pers )
//             pers = Factory::getCurrentUserPersonality();
        
//         m_cemblems_attach_t::iterator iter = m_cemblems_attach.find( em );
        
//         if( iter != m_cemblems_attach.end()
//             && iter->second.hasBelief( pers ) )
//         {
//             return iter->second.getBelief( this, em, pers ).getTimes();
//         }

//         fh_times ti = new Times();
// //         m_cemblems_attach[ em ].add( pers,
// //                                      MedallionBelief::SURENESS_MAX,
// //                                      ti );
//         return ti;
//     }

//     double
//     Medallion::getEmblemJudgementSureness( fh_emblem em, fh_personality pers )
//     {
//         if( !pers )
//             pers = Factory::getCurrentUserPersonality();
        
//         m_cemblems_attach_t::iterator iter = m_cemblems_attach.find( em );
        
//         if( iter != m_cemblems_attach.end()
//             && iter->second.hasBelief( pers ) )
//         {
//             return iter->second.getBelief( this, em, pers ).getSureness();
//         }

//         return MedallionBelief::SURENESS_NULL;
//     }
    
    
    std::list< fh_personality >
    Medallion::getListOfPersonalitiesWhoHaveOpinion( fh_emblem em )
    {
        m_assertions_t& m_assertions = m_cemblems_attach[ em ];
        std::list< fh_personality > ret;

        for( m_assertions_t::iterator ai = m_assertions.begin(); ai != m_assertions.end(); ++ai )
            ret.push_back( ai->first );
        
        return ret;
    }

    fh_medallionBelief
    Medallion::getBelief( fh_emblem em, fh_personality pers )
    {
        if( !pers )
            pers = Factory::getCurrentUserPersonality();
        
        m_assertions_t& m_assertions = m_cemblems_attach[ em ];

        m_assertions_t::iterator ai = m_assertions.find( pers );
        if( ai == m_assertions.end() )
        {
            fh_stringstream ss;
            ss << "No belief has been expressed by personality:" << pers->getName()
               << " for the emblem:" << em->getName() << endl;
            Throw_NoSuchBeliefException( tostr(ss), 0 );
        }
        return ai->second;
                        
    }
    
    bool
    Medallion::hasBelief( fh_emblem em, fh_personality p )
    {
        if( !p )
            p = Factory::getCurrentUserPersonality();
        
        m_assertions_t& m_assertions = m_cemblems_attach[ em ];
        m_assertions_t::iterator ai = m_assertions.find( p );
        return ai != m_assertions.end();
    }
    
    
    emblems_t
    Medallion::getMostSpecificEmblems( Emblem::limitedViewPri_t cutoff )
    {
        if( cutoff == Emblem::LIMITEDVIEW_PRI_USER_CONFIG )
            cutoff = Factory::getEtagere()->getLowestEmblemPriorityToShow();
                
        emblems_t ret;
        for( m_cemblems_t::iterator ci = m_cemblems.begin();
             ci != m_cemblems.end(); ++ci )
        {
            if( (*ci)->getLimitedViewPriority() >= cutoff )
                ret.push_back( *ci );
        }
        return ret;
    }

    emblems_t
    Medallion::getAllEmblems( Emblem::limitedViewPri_t cutoff )
    {
        if( cutoff == Emblem::LIMITEDVIEW_PRI_USER_CONFIG )
            cutoff = Factory::getEtagere()->getLowestEmblemPriorityToShow();
        
        emblems_t input = getMostSpecificEmblems();
        emblems_t tmp;

        for( emblems_t::iterator ci = input.begin(); ci != input.end(); ++ci )
        {
            addAllParentsTransitive( tmp, *ci, cutoff );
        }

        tmp.sort();
        emblems_t::iterator end = unique( tmp.begin(), tmp.end() );
        emblems_t ret;
        copy( tmp.begin(), end, back_inserter( ret ) );
        return ret;
    }

string emblemListToEAQuery( emblemset_t& el, char combineOpcode )
    {
        stringstream qss;
        
        if( el.size() > 1 )
        {
            qss << "(" << combineOpcode;
        }
        
        bool v = true;
        for( emblemset_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
        {
            fh_emblem em = *ei;
            if( v ) v = false;
            else    qss << "";
        
            qss << "(emblem:id-" << em->getID() << "==1)";
        }

        if( el.size() > 1 )
            qss << ")";

        return tostr(qss);
    }


    double DRangeToKiloMeters( double d )
    {
        double kmPerDigitalDigit = 111.10527282045992;
        d *= kmPerDigitalDigit;
        return d;
    }

    double KiloMetersToDRange( double d )
    {
        double kmPerDigitalDigit = 111.10527282045992;
        d /= kmPerDigitalDigit;
        return d;
    }
    
    emblemset_t& getEmblemsNear( emblemset_t& result,
                                 fh_emblem em,
                                 double kmrange,
                                 fh_etagere et,
                                 bool ShowDownSet )
    {
        if( !et )
            et = ::Ferris::Factory::getEtagere();
        
        double elat = em->getDigitalLatitude();
        double elong = em->getDigitalLongitude();

        double range = KiloMetersToDRange( kmrange );
        fh_emblem geospatialem = et->getEmblemByName( "libferris-geospatial" );
        emblems_t all = geospatialem->getDownset();
        for( emblems_t::iterator ai = all.begin(); ai != all.end(); ++ai )
        {
            double alat = (*ai)->getDigitalLatitude();
            double along = (*ai)->getDigitalLongitude();
            if( !alat || !along )
                continue;
            if( fabs( alat - elat ) < range )
            {
                if( fabs( along - elong ) < range )
                {
                    if( ShowDownSet )
                    {
                        emblems_t ds = (*ai)->getDownset();
                        copy( ds.begin(), ds.end(), inserter( result, result.end() ) );
                    }
                    else
                    {
                        result.insert( *ai );
                    }
                }
            }
        }

        return result;
    }
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    fh_emblem getFerrisSystemEmblem( fh_etagere et )
    {
        if( !et )
            et = Factory::getEtagere();

        try
        {
            fh_emblem ret = et->getEmblemByName( EMBLEM_TOPLEVEL_SYSTEM_NAME );
            return ret;
        }
        catch( EmblemNotFoundException& e )
        {
            fh_cemblem ret = et->createColdEmblem( EMBLEM_TOPLEVEL_SYSTEM_NAME );
            return ret;
        }
    }


    fh_emblem getShouldSkipIndexingEmblem()
    {
        string EMBLEM_SHOULD_SKIP_INDEXING = "should-skip-indexing";

        static fh_emblem ret = 0;

        if( !ret )
        {
            fh_etagere et  = ::Ferris::Factory::getEtagere();
            fh_emblem  em  = getFerrisSystemEmblem( et );
            fh_emblem e    = em->obtainChild( EMBLEM_SHOULD_SKIP_INDEXING );
            ret = e;
        }
        return ret;
    }
    

    fh_emblem getDummyTreeModelEmblem()
    {
        string EMBLEM_CHILD_SYSTEM_NAME = "system";
        string EMBLEM_CHILD_DUMMYTREEMODELEMBLEM = "dummy-gtk-tree-model-emblem";

        static fh_emblem ret = 0;

        if( !ret )
        {
            
            fh_etagere et  = Factory::getEtagere();
            fh_emblem  em  = getFerrisSystemEmblem( et );
            fh_emblem syse = em->obtainChild( EMBLEM_CHILD_SYSTEM_NAME );
            fh_emblem e    = syse->obtainChild( EMBLEM_CHILD_DUMMYTREEMODELEMBLEM );
            ret = e;
        }
        return ret;
    }
    

    fh_emblem private_getAttributeRootEmblem( fh_etagere et )
    {
        static fh_emblem default_etagere_ret_cache = 0;
        
        bool use_cache = false;
        if( !et )
            use_cache = true;
        else if( et == Factory::getEtagere() )
            use_cache = true;
        if( use_cache && default_etagere_ret_cache )
            return default_etagere_ret_cache;
        
        
        static const string EMBLEM_EANAMES_ORDERING_NAME = "ea-ordering";

        if( !et )
            et = Factory::getEtagere();
        fh_emblem  em = getFerrisSystemEmblem( et );
        fh_emblem ret = em->obtainChild( EMBLEM_EANAMES_ORDERING_NAME );

        if( et == Factory::getEtagere() )
            default_etagere_ret_cache = ret;
        
        return ret;
    }
    
};
