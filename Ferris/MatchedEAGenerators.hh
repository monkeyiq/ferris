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

    $Id: MatchedEAGenerators.hh,v 1.6 2010/09/24 21:30:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

#ifndef _ALREADY_INCLUDED_MATCHED_EA_GENERATORS_H_
#define _ALREADY_INCLUDED_MATCHED_EA_GENERATORS_H_ 1

namespace Ferris
{
    


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class FERRISEXP_API AttributeCreator
{
public:

    enum CreatePri_t {
        CREATE_PRI_MAX_INTERNAL_USE_ONLY=13,
        CREATE_PRI_NATURALP2=12,
        CREATE_PRI_NATURALP1=11,
        CREATE_PRI_NATURAL=10,
        CREATE_PRI_HIGH=7,
        CREATE_PRI_MED=5,
        CREATE_PRI_LOW=2,
        CREATE_PRI_NOT_SUPPORTED=0,
    };

    virtual CreatePri_t getCreatePriority() = 0;
    virtual fh_attribute CreateAttr(
        const fh_context& a,
        const std::string& rdn,
        fh_context md = 0 )
        throw(
            FerrisCreateAttributeFailed,
            FerrisCreateAttributeNotSupported
            ) = 0;

    virtual bool supportsCreateForContext( fh_context c );
};



class FERRISEXP_API MatchedEAGeneratorFactory
    :
    public AttributeCreator,
    public Handlable
{
public:

    typedef std::list<Attribute*> BrewColl_t;
    friend class GModuleMatchedEAGeneratorFactory;
    
protected:
    fh_matcher theMatcher;

    virtual void Brew( const fh_context& ctx ) = 0;

    MatchedEAGeneratorFactory( const fh_matcher& ma );
    MatchedEAGeneratorFactory();
    

public:

    /**
     * Used for statefull EA. if they don't match then the Context class need not
     * copy the EAGen object
     */
    bool hasInterest( const fh_context& ctx );
    

    virtual void tryBrew( const fh_context& ctx );

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Can attributes be created dynamically
     */
    virtual bool isDynamic();

    /**
     * If there should be one factory created per context
     * (ie. state is kept for the context by the factory)
     * then override this method and return true;
     * Defaults to stateless.
     */
    virtual bool hasState();

    
    /**
     * if isDynamic() == true then override this method
     * and test for single attributes by name when they are requested
     *
     * returns 1 if the attribute was added (same as addAttribute())
     */
    virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    /*
     * Override these two if the EA generator wants to support
     * creating new EA aswell. These should really be a different
     * abstraction, but...
     */
    virtual CreatePri_t getCreatePriority();

    virtual fh_attribute CreateAttr(
        const fh_context& a,
        const std::string& rdn,
        fh_context md = 0 )
        throw(
            FerrisCreateAttributeFailed,
            FerrisCreateAttributeNotSupported
            );


    /**
     * Possibility for a module to add attributes to recommended-ea
     * for any context. Called in Context::getRecommendedEA().
     */
    virtual void augmentRecommendedEA( const fh_context& a, fh_stringstream& ss );
    
};
FERRIS_SMARTPTR( MatchedEAGeneratorFactory, fh_matched_eafactory );
 
typedef MatchedEAGeneratorFactory*                          fp_matched_eafactory;
typedef MatchedEAGeneratorFactory                            f_matched_eafactory;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <gmodule.h>
class FERRISEXP_DLLLOCAL GModuleMatchedEAGeneratorFactory
    :
    public MatchedEAGeneratorFactory
{
    typedef MatchedEAGeneratorFactory _Base;

protected:
    
    std::string ImplName;
    GModule* ghandle;
    GModule* ghandle_factory;
    void     (*module_Brew)( AttributeCollection* ctx );

    MatchedEAGeneratorFactory* RealFactory;
    MatchedEAGeneratorFactory* (*CreateRealFactory)();

    bool (*module_isDynamic)();
    bool (*module_hasState)();
    bool (*module_tryBrew)( const fh_context& ctx, const std::string& eaname );
    
    AttributeCreator::CreatePri_t (*getCreatePri)();
    CreatePri_t CreatePriCache;

    /**
     * Load the _factory.so and attach function pointers
     */
    void ensureFactoryModuleVTable();
    

    virtual void Brew( const fh_context& ctx );

    void ensureGModuleIsLoaded();

public:
    
    GModuleMatchedEAGeneratorFactory( const fh_matcher& ma, const std::string& implname );

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Can attributes be created dynamically
     */
    virtual bool isDynamic();

    /**
     * If there should be one factory created per context
     * (ie. state is kept for the context by the factory)
     * then override this method and return true;
     * Defaults to stateless.
     */
    virtual bool hasState();
    
    /**
     * if isDynamic() == true then override this method
     * and test for single attributes by name when they are requested
     *
     * returns 1 if the attribute was added (same as addAttribute())
     */
    virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );

    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    virtual CreatePri_t getCreatePriority();
    virtual fh_attribute CreateAttr(
        const fh_context& a,
        const std::string& rdn,
        fh_context md = 0 )
        throw(
            FerrisCreateAttributeFailed,
            FerrisCreateAttributeNotSupported
            );
    virtual bool supportsCreateForContext( fh_context c );
    virtual void augmentRecommendedEA( const fh_context& a, fh_stringstream& ss );

};


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

/**
 * EA Generators can now have their _factory.so linked directly to libferris
 * this is a bridge to allow the factory to be static but the mainlib.so for
 * the plugin to be dynamically loaded.
 */
class FERRISEXP_API StaticGModuleMatchedEAGeneratorFactory
    :
    public GModuleMatchedEAGeneratorFactory
{
    typedef GModuleMatchedEAGeneratorFactory _Base;

    std::string m_shortName;
    bool        m_isDynamic;
    bool        m_hasState;

protected:


public:
    
    StaticGModuleMatchedEAGeneratorFactory(
        const fh_matcher& ma,
        const std::string& implname,
        const std::string& shortname,
        bool isDynamic,
        bool hasState,
        AttributeCreator::CreatePri_t CreatePri );

    StaticGModuleMatchedEAGeneratorFactory* clone();
    
    /********************/
    /********************/

    virtual bool isDynamic();
    virtual bool hasState();
    virtual CreatePri_t getCreatePriority();
    virtual const std::string& getShortName();

    /********************/
    /********************/
};


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


class FERRISEXP_API Image : public CacheHandlable
{
public:
    enum LoadType {
        LOAD_ONLY_METADATA  = 1<<1,
        LOAD_COMPLETE_IMAGE = 1<<2
    };

    virtual ~Image();
    
    virtual guint32  getWidth();
    virtual guint32  getHeight();
    virtual int      getDepth();
    virtual int      getDepthPerColor();
    virtual double   getGamma();
    virtual bool     hasAlpha();
    /**
     * Get the 32 bit per pixel raw image data.
     * note that the returned data is owned by this object,
     * to explicitly reclaim it when you are not using it call freeRGBA();
     */
    virtual guint32* getRGBA();
    virtual guint32  getRGBASize();
    virtual double   getAspectRatio();
    virtual bool     isValid();

    

    /**
     * Called to free the rgba member.
     */
    virtual void freeRGBA();

    void setNewWidth( guint32 v );
    void setNewHeight( guint32 v );

    virtual void updateFromStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
    
protected:

    Image( fh_istream _ss );
    
    guint32 getNewWidth();
    guint32 getNewHeight();
    
    fh_istream ss;

    guint32 w;
    guint32 h;
    int d;
    int a;
    double gamma;
    float aspect_ratio;
    bool is_valid;
    bool failed_to_load;
    
    guint32* rgba;

    bool GotMetaData;
    virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA ) = 0;
    /**
     * Override this to provide write support for the module. The default simply throws
     * an exception.
     */
    virtual void priv_saveImageData( fh_context c, fh_istream imageStream );

    /**
     * This does a little handywork for you and then calls priv_ensureDataLoaded()
     * which is the method you should implement
     */
    void ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );

    inline void setWidth ( guint32 v ) { w = v; }
    inline void setHeight( guint32 v ) { h = v; }
    inline void setAlpha ( int     v ) { a = v; }
    inline void setGamma ( double  v ) { gamma = v; }
    inline void setAspectRatio( float v ) { aspect_ratio = v; }
    inline void setDepthPerColor( int     v ) { d = v; }
    inline void setValid( bool v ) { is_valid = v; }
    
    void ensureRGBA_IsAllocated();
    

            
    void convert_scanline_RGB_to_RGBA( guint32* scanline, guint8 alpha = 255 );
    void convertRGB_to_RGBA();
    
private:
    guint32 newWidth;
    guint32 newHeight;

};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
};

#endif // #ifndef _ALREADY_INCLUDED_MATCHED_EA_GENERATORS_H_

