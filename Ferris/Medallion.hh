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

    $Id: Medallion.hh,v 1.21 2011/05/06 21:30:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_MEDALLIONS_H_
#define _ALREADY_INCLUDED_FERRIS_MEDALLIONS_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Cache.hh>
#include <FerrisStreams/All.hh>
#include <FerrisStreams/All.hh>
#include <STLdb4/stldb4.hh>

#include <list>
#include <string>
#include "Ferris/FerrisStdHashMap.hh"

namespace boost {
    namespace serialization {
        class access;
    }
}

namespace Ferris
{
    struct FERRISEXP_API fh_emblem_less
    {
        bool operator()( fh_emblem k1, fh_emblem k2 ) const;
        
    };
    
    typedef std::list< fh_emblem > emblems_t;
    typedef std::set< fh_emblem, fh_emblem_less >  emblemset_t;
//    typedef std::set< fh_emblem >  emblemset_t;

//     /**
//      * Fixed hot emblems are saved into one 64bit bitmap for
//      * easy indexing.
//      */
//     typedef guint64 hotEmblemID_t;


    /**
     * Is the emblem 'em' in the list 'el'
     */
    FERRISEXP_API bool contains( emblems_t& el, fh_emblem em );

    bool setSkipLoadingEmblems( bool v );
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Single function to associate two emblems in a parent/child
     * relation.
     */
    FERRISEXP_API void link( fh_emblem parent, fh_emblem child );
    FERRISEXP_API void unlink( fh_emblem parent, fh_emblem child );

    FERRISEXP_API emblemID_t UseGetNextID( Emblem* e );
    FERRISEXP_API emblemID_t UseGetNextID_EAOrdering( Emblem* e );

    
    /**
       //
       // FIXME: update this crappy old comment
       //
//      * An emblem is a single atomic attribute that can be contained in a
//      * medallion. Emblems can be either part of the fixed "hot" set which
//      * are saved as a bitmap for faster searching operations or can be
//      * saved in a format that favours sparse and slower usage (a cold medallion).
//      * For example mastering a DVD-R one might create a new cold medallion
//      * and tag all the files and directories that are part of the image of that
//      * ROM. One expects far less files to be tagged than 50% of fixed storage
//      * and also one would be willing to accept slower searches on this emblem.
//      *
//      * hot emblems are things like; favourates, anime, mydevelopment or webreadable.
//      * ie. things that one might consider to be emblems in nautilus are hot
//      * emblems in libferris. You want faster access to searching on hot emblems.
//      * There is currently a restriction of 64bit less some control bits on hot
//      * emblems. The restriction could be removed in a future release.
//      *
//      * First emblems being added are cold only.
     */
    class FERRISEXP_API Emblem
        :
        public Handlable
    {
        typedef Emblem _Self;
        friend class Etagere;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
            {
//                 // serialize base class information
//                 ar & boost::serialization::base_object<Handlable>(*this);
                
                ar & m_id;
                ar & m_name;
                ar & m_iconname;
                ar & m_description;
                ar & m_limitedViewPriority;
                ar & m_parents;
                ar & m_children;
                if( version > 0 )
                {
                    ar & m_uniqName;
                }
                if( version > 1 )
                {
                    ar & m_isTransitiveChildOfEAOrderingRootEmblem_isValid;
                    ar & m_isTransitiveChildOfEAOrderingRootEmblem;
                }
                if( version > 2 )
                {
                    ar & m_digitalLatitude;
                    ar & m_digitalLongitude;
                    ar & m_zoomRange;
                }
                
                

//                std::cerr << "version:" << version << " name:" << m_name << " UN:" << m_uniqName << std::endl;
            }
        
    protected:

        fh_etagere   m_et;
        emblemID_t   m_id;
        std::string  m_name;
        std::string  m_uniqName;
        std::string  m_iconname;
        std::string  m_menuSizedIconName;
        std::string  m_description;
        double       m_digitalLatitude;
        double       m_digitalLongitude;
        double       m_zoomRange;

        typedef std::set< emblemID_t > m_parents_t;
        typedef std::set< emblemID_t > m_children_t;
        m_parents_t   m_parents;
        m_children_t  m_children;

        bool      m_upset_cache_only_isDirty;
        emblems_t m_upset_cache_only;

        bool m_isTransitiveChildOfEAOrderingRootEmblem_isValid;
        bool m_isTransitiveChildOfEAOrderingRootEmblem;
        
        /**
         * NOTE: you *must* call setup() after object creation
         * If eid is zero then create a new cold emblem otherwise
         * load the information for the cold emblem identified by
         * eid.
         */
        Emblem( fh_etagere et, emblemID_t eid = 0 );

        typedef Loki::Functor< emblemID_t, LOKI_TYPELIST_1( Emblem* ) > getNextIDFunctor_t;
        friend emblemID_t UseGetNextID( Emblem* e );
        friend emblemID_t UseGetNextID_EAOrdering( Emblem* e );
        
        /**
         * load or create a new ID for this Emblem
         */
        void setup( getNextIDFunctor_t f = &UseGetNextID );

        /**
         * call here to mark this emblem as changed and needing to
         * be save()ed to disk. Non changed emblems do nothing when
         * save( false ) is called.
         */
        void setDirty();

        /**
         * Have we changed.
         */
        bool isDirty();
        
        /**
         * Get the next emblem ID that is usable and increment
         * the counter so that the next call to getNextID() will
         * return a different number.
         */
        emblemID_t getNextID();
        emblemID_t getNextID_EAOrdering();
        
        /**
         * Loads all the common settings for an emblem such
         * as its name, icon, description
         */
        void load_common( fh_stringstream& iss );

        /**
         * Save all data loaded by load_common()
         */
        void save_common( fh_ostream oss );

        /**
         * Subclasses of Emblem will usually have different key matter
         * and thus will obtain their data from many places. The load
         * is performed via priv_load() which should pass an istream to
         * load_common() to load the common emblem data. Common emblem
         * data may be versioned to allow forward migration.
         */
        virtual void priv_load() = 0;

        /**
         * @see priv_load()
         */
        virtual void priv_save() = 0;

        void save( bool force = false );
        void load();

        /**
         * associate two emblems in a parent - child relation
         */
        friend void link( fh_emblem parent, fh_emblem child );
        friend void unlink( fh_emblem parent, fh_emblem child );
        /**
         * use the link() function to obtain access to addParent() and addChild()
         * in a single function call.
         */
        void addParent( fh_emblem em );
        void addChild( fh_emblem em );
        void removeParent( fh_emblem em );
        void removeChild( fh_emblem em );

        
    public:

        emblemID_t getID() const;
        
        emblems_t getParents();
        emblems_t getChildren();
        bool hasChildren();
        
        /**
         * Mainly for migration method, update m_children and m_parents taking into
         * account for emblem IDs which do not exist anymore. This method shouldn't
         * be needed normally.
         */
        void forceUpdateParentsAndChildrenIDs();
        

        const emblems_t& getUpset();
        emblems_t getDownset();

        /**
         * Possible parents are non transtive parents and other emblems
         * which are not transitive children
         */
        emblems_t getPossibleParents();

        /**
         * Get a list of emblems which could be added as children of this
         * emblem
         */
        emblems_t getPossibleChildren();
        
        /**
         * Is the passed emblem a parent of this emblem
         */
        bool hasParent( fh_emblem em );
        bool hasChild( fh_emblem em );

        /**
         * If there is a child which has the name given then return it
         * otherwise create a new emblem which is a child of this one
         * and return it.
         *
         * Useful in apps which are wanting to quickly find or setup
         * an emblem ordering
         */
        fh_emblem obtainChild( const std::string& name );
        fh_emblem obtainChild_EAOrdering( const std::string& name );

        /**
         * Get the child emblem by name. An exception is thrown if
         * there is no such child
         *
         * @throws EmblemNotFoundException
         * @see hasChild()
         * @see obtainChild()
         */
        fh_emblem findChild( const std::string& name );
        
        std::string getName();
        std::string getUniqueName();
        std::string getIconName();
        std::string getMenuSizedIconName();
        std::string getDescription();

        void setName( const std::string& v );
        void setIconName( const std::string& v );
        void setMenuSizedIconName( const std::string& v );
        void setDescription( const std::string& v );

        double getDigitalLatitude();
        double getDigitalLongitude();
        double getZoomRange();
        void setDigitalLatitude( double v );
        void setDigitalLongitude( double v );
        void setZoomRange( double v );
        
        
        /**
         * Some views like inline icons in a treelist have very limited
         * space. One can set ShowInLimitedViews to false to avoid cluttering
         * such limited views with too many emblems.
         */
        enum limitedViewPri_t {
            LIMITEDVIEW_PRI_USER_CONFIG = -101, //< Use the cutoff that the user has set
            LIMITEDVIEW_PRI_LOW = -100,
            LIMITEDVIEW_PRI_DEFAULT = 0,
            LIMITEDVIEW_PRI_HI = 100
        };
        limitedViewPri_t getLimitedViewPriority();
        void setLimitedViewPriority( limitedViewPri_t v );
        
        /**
         * emblem is the emblem that is changing, the first string is the old icon
         * name and the second string is the new iconname
         */
        typedef sigc::signal< void ( fh_emblem, std::string, std::string ) > IconName_Changed_Sig_t;
        IconName_Changed_Sig_t& getIconName_Changed_Sig();
        IconName_Changed_Sig_t& getMenuSizedIconName_Changed_Sig();

        /**
         * When a new parent has been added this is fired.
         * the first parameter is the emblem itself, the second is the new parent
         */
        typedef sigc::signal< void ( fh_emblem, fh_emblem ) > AddedParent_Sig_t;
        AddedParent_Sig_t& getAddedParent_Sig();

        /**
         * When a new child has been added this is fired.
         * the first parameter is the emblem itself, the second is the new child
         */
        typedef sigc::signal< void ( fh_emblem, fh_emblem ) > AddedChild_Sig_t;
        AddedChild_Sig_t& getAddedChild_Sig();

        /**
         * When a parent has been removed this is fired.
         * the first parameter is the emblem itself, the second is the old parent
         */
        typedef sigc::signal< void ( fh_emblem, fh_emblem ) > RemovedParent_Sig_t;
        RemovedParent_Sig_t& getRemovedParent_Sig();

        /**
         * When a child has been removed this is fired.
         * the first parameter is the emblem itself, the second is the old child
         */
        typedef sigc::signal< void ( fh_emblem, fh_emblem ) > RemovedChild_Sig_t;
        RemovedChild_Sig_t& getRemovedChild_Sig();
        
        /**
         * Dump the emblem info as XML to the given stream
         */
        virtual void dumpTo( fh_ostream oss );

        /**
         * Returns true if this emblem is a (transitive) child of the EA Ordering emblem.
         */
        bool isTransitiveChildOfEAOrderingRootEmblem();

        /**
         * Update the cache used by isTransitiveChildOfEAOrderingRootEmblem()
         * this should only be needed in rare cases for migration etc.
         */
        void forceUpdateTransitiveChildOfEAOrderingRootEmblem();
    private:

        void updateTransitiveChildrenProperty_ChildOfEAOrderingRootEmblem( bool v );
        
        IconName_Changed_Sig_t IconName_Changed_Sig;
        IconName_Changed_Sig_t MenuSizedIconName_Changed_Sig;
        AddedParent_Sig_t      AddedParent_Sig;
        AddedChild_Sig_t       AddedChild_Sig;
        RemovedParent_Sig_t    RemovedParent_Sig;
        RemovedChild_Sig_t     RemovedChild_Sig;

        limitedViewPri_t       m_limitedViewPriority;
    };

    /**
     * Cold emblems store a unique integer as their primary
     * identifier in a medallion. A zero value for the integer
     * is undefined.
     */
    class FERRISEXP_API ColdEmblem
        :
        public Emblem
    {
        typedef ColdEmblem _Self;
        typedef Emblem     _Base;
        friend class Etagere;

        void setEtagere( fh_etagere et );
        
//         friend class boost::serialization::access;
//         template<class Archive>
//         void serialize(Archive & ar, const unsigned int version)
//             {
//                 // serialize base class information
//                 ar & boost::serialization::base_object<Emblem>(*this);
//             }
        

        /**
         * db file where cold emblems being saved
         */
        std::string getDBPath();

        /**
         * cache the open database
         */
//        typedef FERRIS_STD_HASH_MAP< std::string, STLdb4::fh_database > m_dbcache_t;
        STLdb4::fh_database getDB();
        
        /**
         * Only create cold emblems using Load()
         */
        ColdEmblem( fh_etagere et, emblemID_t eid );

        /**
         * If eid is zero then create a new cold emblem otherwise
         * load the information for the cold emblem identified by
         * eid.
         */
        static fh_cemblem Load( fh_etagere et, emblemID_t eid = 0, getNextIDFunctor_t f = &UseGetNextID );
//        static fh_cemblem Load_EAOrdering( fh_etagere et, emblemID_t eid = 0 );
        
    protected:
        virtual void priv_load();
        virtual void priv_save();
//        STLdb4::fh_database getDBCache( const std::string& k );
        STLdb4::fh_database m_db;
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    namespace Factory 
    {
        /**
         * For testing its nice to be able to set the default etagere
         * to someplace in /tmp that has been setup for it. Set it to
         * zero to revert to the default etagere that getEtagere() gives
         * initially.
         *
         * Mainly useful for testing but also handy if any app wants to
         * have its own private medallion tree.
         */
        FERRISEXP_API void setDefaultEtagere( fh_etagere et = 0 );

        /**
         * get the default etagere
         */
        FERRISEXP_API fh_etagere getEtagere();

        /**
         * Create a new etagere at the path given
         */
        FERRISEXP_API fh_etagere makeEtagere( const std::string& path );
    }

    /**
     * Emblems are ordered in a partial order instead of a tree. The main
     * impact of this on a tree based design being that a single emblem
     * can have more than one parent. This anime can be both a subemblem of
     * video and faviourates.
     *
     * For medallions owned by the same person, ie. user on a machine,
     * there should only be one Etagere which arranges the emblems.
     * Note that changing the ordering of the Etagere by adding a new
     * parent to an emblem should automatically effect all the emblems
     * for that user.
     *
     * From wordnet:
     * 1. etagere -- (a piece of furniture with open shelves for displaying small ornaments)
     */
    class FERRISEXP_API Etagere
        :
        public Handlable
    {
        friend class Medallion;
        friend class Emblem;
        friend fh_etagere Factory::getEtagere();
        friend fh_etagere Factory::makeEtagere( const std::string& path );

        typedef std::map< emblemID_t, fh_cemblem > m_cemblems_t;
        m_cemblems_t m_cemblems;
        typedef std::multimap< std::string, fh_cemblem > m_cemblemsByName_t;
        m_cemblemsByName_t m_cemblemsByName;

        typedef std::set< emblemID_t > eset_t;
        eset_t m_newEmblemIDs;
        
        std::string m_basepath;

        bool m_isDirty;
        bool m_haveLoadedAll;

        void loadAllEmblems();
        void saveAllEmblems( bool force = false );

        template< class ArchiveClassT >
        void loadAllEmblems( ArchiveClassT& archive );
        template< class ArchiveClassT >
        void saveAllEmblems( ArchiveClassT& archive );
        
        
//         fh_emblem  getHotEmblem( hotEmblemID_t   eid );
        fh_cemblem getColdEmblem( emblemID_t eid );

        Etagere( const std::string& basepath = "" );

        void dump();

        /**
         * call here to mark this etagere as changed and needing to
         * be save()ed to disk. Non changed etageres do nothing when
         * saveAllEmblems( false ) is called.
         */
        void setDirty();

        /**
         * Have we changed.
         */
        bool isDirty();

        /**
         * get the db4 file that the etagere is saved in
         */
        STLdb4::fh_database m_db;
        STLdb4::fh_database getDB();

        fh_cemblem createColdEmblem_common( fh_cemblem em );
        
    public:
        virtual ~Etagere();
        void sync();
        
        emblems_t& getAllEmblemsWithName( emblems_t& ret, const std::string& name );
        emblemset_t& getAllEmblemsWithName( emblemset_t& ret, const std::string& name );
        fh_emblem obtainEmblemByName( const std::string& name );
        fh_emblem getEmblemByName( const std::string& name );
        fh_emblem getEmblemByUniqueName( const std::string& name );
//        bool      isEmblemNameUnique( const std::string& name );
        fh_emblem getEmblemByID( emblemID_t id );
        emblems_t getAllEmblems();
        emblems_t getAllEmblemsUniqueName();

        typedef Loki::Functor< void, LOKI_TYPELIST_1( const fh_emblem& ) > f_emblemVisitor;
        void visitAllEmblems( const f_emblemVisitor& f );

        /**
         * Each medallion should know what etagere it belongs to by
         * checking this value.
         */
        std::string getOntologyID();

        Emblem::limitedViewPri_t getLowestEmblemPriorityToShow();
        void setLowestEmblemPriorityToShow( Emblem::limitedViewPri_t v );

        void erase( fh_emblem em );

        /**
         * Dump the structure of the etagere as XML to the given stream
         */
        void dumpTo( fh_ostream oss );
        
        
        /********************/
        /*** factory ********/
        /********************/

        fh_cemblem createColdEmblem( const std::string& name = "new emblem" );
        fh_cemblem createColdEmblem_EAOrdering( const std::string& name = "new emblem" );

        
        /********************/
        /*** paths **********/
        /********************/
        
        /**
         * base path where this Etagere is being saved
         */
        std::string getBasePath();

        /**
         * db file where etagere metadata is being saved
         */
        std::string getDBPath();


        /**
         * path for new boost serialization file
         */
        std::string getBoostSerializePath();
        std::string getBoostSerializePathTxt();
        
        /********************/
        /*** signals ********/
        /********************/

        /**
         * When a new emblem has been created this is fired.
         * the first parameter is the etagere, the second is the new child
         */
        typedef sigc::signal< void ( fh_etagere, fh_emblem ) > EmblemCreated_Sig_t;
        EmblemCreated_Sig_t& getEmblemCreated_Sig();

        /**
         * When a new child has been added this is fired.
         * the first parameter is the etagere, the second is the new child
         */
        typedef sigc::signal< void ( fh_etagere, fh_emblem ) > AddedChild_Sig_t;
        AddedChild_Sig_t& getAddedChild_Sig();
        
        /**
         * When a child has been removed this is fired.
         * the first parameter is the etagere, the second is the old child
         */
        typedef sigc::signal< void ( fh_etagere, fh_emblem ) > RemovedChild_Sig_t;
        RemovedChild_Sig_t& getRemovedChild_Sig();

        /**
         * Out of process signal updates.
         */
        void OnOutOfProcNewEmblemNotification( std::set< emblemID_t >& eset );
        
    protected:

        void priv_emblemBeingRenamed( fh_cemblem e,
                                      const std::string& oldn,
                                      const std::string& newn );



    private:

        EmblemCreated_Sig_t  EmblemCreated_Sig;
        AddedChild_Sig_t     AddedChild_Sig;
        RemovedChild_Sig_t   RemovedChild_Sig;
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_API Times
        :
        public Handlable
    {
        time_t m_mtime;
        time_t m_atime;
    public:
        Times();

        void setATime( time_t t = 0 );
        void setMTime( time_t t = 0 );

        void touchATime();
        void touchMTime();
        
        time_t getATime();
        time_t getMTime();
    };
    FERRIS_SMARTPTR( Times, fh_times );
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * a belief that is held for a medallion, about an emblem by a given
     * personality at a point in time.
     *
     * retraction is supported by a negative sureness and positive sureness
     * represents assertion.
     *
     * Various interesting metadata about the belief is available from this object.
     */
    class FERRISEXP_API MedallionBelief
        :
        public Handlable
    {
        typedef Handlable _Base;
        NOT_COPYABLE( MedallionBelief );
        friend class Medallion;
        
        Medallion*       m_medallion;
        fh_emblem        m_emblem;
        fh_personality   m_personality;
        double           m_sureness;
        mutable fh_times m_times;
            
        MedallionBelief( Medallion* m, fh_emblem em, fh_personality p, 
                         double sureness = 0, fh_times t = 0 );
        MedallionBelief();

    protected:

        void setSureness( double v );
        
    public:
        virtual ~MedallionBelief();

        enum {
            SURENESS_MAX = 100,
            SURENESS_NULL = 0,
            SURENESS_MIN = -100
        };
        
        fh_medallion   getMedallion() const;
        fh_emblem      getEmblem() const;
        fh_personality getPersonality() const;
        fh_times       getTimes() const;
        double         getSureness() const;

        static double clampJudgementSureness( double );

        /********************************************************************************/
        /*** This is a little tricky. We expect to be in a collection that medallion ****/
        /*** holds and thus to exist with an rc of 1 while the medallion lives.      ****/
        /*** However, since handles to the belief can be obtained by the developer   ****/
        /*** we have to bump/drop the medallion reference count with ours so that    ****/
        /*** the medallion is still existing if there are external handles to a      ****/
        /*** belief that references a medallion *****************************************/
        /********************************************************************************/
//         virtual ref_count_t AddRef();
//         virtual ref_count_t Release();
    };

    
    
    /**
     * A medallion is the collection of emblems that a particular fh_context
     * object holds. There are convenience methods to allow one to interact
     * with the emblem partial order and get at the emblems that are contained
     * in this medallion easily.
     *
     * Note that a flyweight pattern is used on the emblems, ie. The emblem
     * for anime is shared between both the main partial ordering and each
     * medallion containing that emblem.
     */
    class FERRISEXP_API Medallion
        :
        public CacheHandlable
    {
        fh_context m_context;
        bool m_isDirty;  //< Has the medallion changed
        bool m_dontSave; //< If there is a medallion on disk that we want to not change (eg from other onto)

        /********************************************************************************/
        // internal data structures as at VERSION_VALUE_V1
        /********************************************************************************/
//         typedef std::set< fh_cemblem > m_cemblems_t;
//         m_cemblems_t m_cemblems;
//         typedef std::map< fh_emblem, fh_times > m_cemblems_times_t;
//         m_cemblems_times_t m_cemblems_times;
        
        /********************************************************************************/
        // internal data structures as at VERSION_VALUE_V2
        /********************************************************************************/

        fh_medallionBelief createBelief( fh_emblem em, fh_personality p,
                                         double sureness = MedallionBelief::SURENESS_NULL,
                                         fh_times t = 0 );
        fh_medallionBelief obtainBelief( fh_emblem em, fh_personality p,
                                         double sureness = MedallionBelief::SURENESS_NULL,
                                         fh_times t = 0 );

        void eraseBelief( fh_emblem em, fh_personality p );
        
//         struct JudgementList
//         {
//             typedef std::map< fh_personality, fh_medallionBelief > m_assertions_t;
//             m_assertions_t m_assertions;

//             /**
//              * Get the belief for the given personality. Create it if its not
//              * already there.
//              */
//             fh_medallionBelief getBelief( Medallion* m, fh_emblem em, fh_personality p,
//                                           double sureness = MedallionBelief::SURENESS_NULL,
//                                           fh_times t = new Times() );

//             /**
//              * make the attachment as though it never happened, different to a
//              * retraction in that with this method the original assertion is forgoten
//              * and it is like nothing was done to start with
//              */
//             void eraseBelief( fh_personality p );

//             /**
//              * Check if the given personality has a belief about this emblem/medallion combo
//              */
//             bool hasBelief( fh_personality p );
            
//             std::list< fh_personality > getListOfPersonalitiesWhoHaveOpinion();
//         };
        typedef std::set< fh_cemblem > m_cemblems_t;
        m_cemblems_t m_cemblems;
//        typedef std::map< fh_emblem, JudgementList > m_cemblems_attach_t;
        typedef std::map< fh_personality, fh_medallionBelief > m_assertions_t;
        typedef std::map< fh_emblem, m_assertions_t >          m_cemblems_attach_t;
        m_cemblems_attach_t m_cemblems_attach;

        
        friend class Context;
        Medallion( fh_context c );

        std::string getEAName();
        void load( fh_context c );
        void save( fh_context c );

        void setDirty();
        
    public:
        virtual ~Medallion();
        
        /********************************************************************************/
        /*** USER INTERFACE *************************************************************/
        /********************************************************************************/

        void addEmblem( fh_emblem em );
        void retractEmblem( fh_emblem em );
        void removeEmblem( fh_emblem em );
        bool hasEmblem( fh_emblem em );
        bool hasRetractedEmblem( fh_emblem em );
        void ensureEmblem( fh_emblem em, bool has );

        /********************************************************************************/
        /*** AGENT INTERFACE ************************************************************/
        /********************************************************************************/

        void addEmblem( fh_emblem em, fh_personality pers, double judgementSureness );
        void retractEmblem( fh_emblem em, fh_personality pers, double judgementSureness );
        void removeEmblem( fh_emblem em, fh_personality pers );
        bool hasEmblem( fh_emblem em, fh_personality pers );
        void ensureEmblem( fh_emblem em, fh_personality pers, double judgementSureness, bool has );
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * Get a resolution of all the assertion/retraction statements offered by all
         * personalities for the emblem as a rating from -100 to 100 as to what the
         * current guess is on if the emblem should be attached.
         */
        double getFuzzyBelief( fh_emblem em );

        /**
         * A list of personalities who have asserted or retracted for this medallion
         * about a given emblem
         */
        std::list< fh_personality > getListOfPersonalitiesWhoHaveOpinion( fh_emblem em );

        /**
         * If a personality has expressed a belief on this medallion relating to
         * the given emblem then return that belief. Otherwise throw an exception.
         *
         * @param em Emblem to find change times for
         * @param pers Personality that made the assertion or retraction. Default is
         *             the primary user personality
         * @throws NoSuchBeliefException
         */
        fh_medallionBelief getBelief( fh_emblem em, fh_personality pers = 0 );

        /**
         * Check if a personality has expressed a belief on this medallion relating to
         * the given emblem
         *
         * @param em Emblem to find change times for
         * @param pers Personality that made the assertion or retraction. Default is
         *             the primary user personality
         */
        bool hasBelief( fh_emblem em, fh_personality p = 0 );
        
//         /**
//          * Get the times that an operation was performed on this medallion
//          * with the given emblem and personality
//          *
//          * @param em Emblem to find change times for
//          * @param pers Personality that made the assertion or retraction. Default is
//          *             the primary user personality
//          * @return the times information for the last assertion/retraction made by pers.
//          */
//         fh_times getEmblemTimes( fh_emblem em, fh_personality pers = 0 );

//         /**
//          * Get the sureness about an assertoin/retraction on this medallion
//          * with the given emblem and personality
//          *
//          * @param em Emblem to find change times for
//          * @param pers Personality that made the assertion or retraction. Default is
//          *             the primary user personality
//          * @return the sureness of the judgement made by the given personality for this emblem.
//          */
//         double getEmblemJudgementSureness( fh_emblem em, fh_personality pers = 0 );
        
        virtual void sync();
        
        /**
         * This gets only the most derived emblems that this medallion
         * contains using the current Etagere as a mapping.
         *
         * @param cutoff for GUI display one can set the lowest priority emblem
         *               that they are interested in. Matching emblems with a lower
         *               priority are ignored. default is set by user in a capplet
         */
        emblems_t getMostSpecificEmblems( Emblem::limitedViewPri_t cutoff = Emblem::LIMITEDVIEW_PRI_USER_CONFIG );

        /**
         * This gets all the emblems that this medallion contains using
         * the current Etagere as a mapping. ie. the return value is
         * x = getMostSpecificEmblems() plus all the transitive parents
         * of x included only once.
         *
         * @param cutoff for GUI display one can set the lowest priority emblem
         *               that they are interested in. Matching emblems with a lower
         *               priority are ignored. default is set by user in a capplet
         */
        emblems_t getAllEmblems( Emblem::limitedViewPri_t cutoff = Emblem::LIMITEDVIEW_PRI_USER_CONFIG );

        /**
         * load the medallion again. used by the out-of-proc engine to
         * update all medallions in each process when sync() is called.
         *
         * To prevent strange data loss it is an error to call this
         * method on a dirty medallion
         */
        void reload();
    };

    /**
     * make an EA Query string like
     * (emblem:has-foo)(emblem:has-bar)
     * but using them emblem IDs and if logicalOr==false then combine with
     * logical and
     */
    std::string emblemListToEAQuery( emblemset_t& el, char combineOpcode = '|' );
    double DRangeToKiloMeters( double d );
    double KiloMetersToDRange( double d );
    emblemset_t& getEmblemsNear( emblemset_t& result,
                                 fh_emblem em,
                                 double kmrange,
                                 fh_etagere et = 0,
                                 bool ShowDownSet = false );
    
};

namespace Loki
{
    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP
    >
    inline bool operator<(const SmartPtr<T, OP, CP, KP, SP>& lhs, const ::Ferris::Emblem* rhs)
    {
        return lhs->getID() < rhs->getID();
    }
};

#endif
