/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2011 Ben Martin

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

    $Id: libferrispostgresql.cpp,v 1.11 2009/04/18 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>

using namespace std;
//#define DEBUG cerr
#define DEBUG LG_PULSEAUDIO_D

#include <pulse/simple.h>
#include <pulse/introspect.h>
#include <pulse/mainloop.h>


namespace Ferris
{
    
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf );
    };

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

class PA;

    class PADataCache : public Handlable
    {
    public:
        PA*  m_pa;
        bool m_valid;
        
    public:
        PADataCache( PA* pa )
            : m_pa(pa)
            , m_valid(false)
        {}

        virtual std::string getName() 
        {
            return "";
        }
        
    };

    class PAVolumeAdjustable : public PADataCache
    {
    protected:
        virtual void   priv_setVolume( pa_volume_t v, int channel );
        virtual void   priv_setVolume( pa_volume_t v );
        virtual double priv_getVolume();
        virtual double priv_getVolume( int channel );
        
    public:
        PAVolumeAdjustable( PA* pa )
            : PADataCache(pa)
        {}

        void setVolume( pa_volume_t v, int channel );
        void setVolume( double v, int channel );
        void setVolume( pa_volume_t v );
        void setVolume( double v );
        double getVolume();
        double getVolume( int channel );
        virtual bool isMuted();
        virtual void setMuted( bool v );
        void toggleMuted();
    };
    FERRIS_SMARTPTR( PAVolumeAdjustable, fh_PAVolumeAdjustable );
    
        

    
    struct PAServerInfo : public PADataCache
    {
        std::string user_name;
        std::string host_name;
        std::string server_name;
        std::string server_version;
        std::string default_sink_name;
        std::string default_source_name;

        PAServerInfo( PA* pa ) : PADataCache(pa) 
        {}
    
    };


    struct PAClientInfo : public PADataCache
    {
        uint32_t 	index;
        std::string name;
        PAClientInfo( PA* pa ) : PADataCache(pa) 
        {}
        virtual std::string getName()
        { return name; }
    };
    FERRIS_SMARTPTR( PAClientInfo, fh_PAClientInfo );
    typedef std::list< fh_PAClientInfo > m_PAClientInfoCol_t;
    
    
    struct PASinkInputInfo : public PAVolumeAdjustable
    {
        uint32_t     index;
        std::string  name;
        uint32_t     client;
        uint32_t     sink;
        pa_cvolume 	 volume;
        int          mute;
        uint32_t     clientIndex;
        int          channels;

    protected:
        
        virtual void   priv_setVolume( pa_volume_t v, int channel );
        virtual void   priv_setVolume( pa_volume_t v );
        virtual double priv_getVolume();
        virtual double priv_getVolume( int channel );
        
    public:
        
        PASinkInputInfo( PA* pa ) : PAVolumeAdjustable(pa) 
        {}
        virtual std::string getName()
        { return name; }
        std::string getClientName();
        virtual bool isMuted();
        virtual void setMuted( bool v );
    };
    FERRIS_SMARTPTR( PASinkInputInfo, fh_PASinkInputInfo );
    typedef std::list< fh_PASinkInputInfo > m_PASinkInputInfoCol_t;

    struct PASinkInfo : public PAVolumeAdjustable
    {
        uint32_t     index;
        std::string  name;
        std::string  desc;
        pa_cvolume 	 volume;
        int          mute;
        int          channels;

    protected:

        virtual void   priv_setVolume( pa_volume_t v, int channel );
        virtual void   priv_setVolume( pa_volume_t v );
        virtual double priv_getVolume();
        virtual double priv_getVolume( int channel );

    public:

        PASinkInfo( PA* pa ) : PAVolumeAdjustable(pa) 
        {}

        virtual std::string getName()
        { return name; }

        virtual bool isMuted();
        virtual void setMuted( bool v );
    };
    FERRIS_SMARTPTR( PASinkInfo, fh_PASinkInfo );
    typedef std::list< fh_PASinkInfo > m_PASinkInfoCol_t;


class PA
{
    pa_context*      m_ctx;
    pa_mainloop*     m_paml;
    pa_mainloop_api* m_paapi;
    int              m_paready;
    bool             m_waitingDispatch;
    PAServerInfo     m_serverInfo;
    m_PAClientInfoCol_t    m_PAClientInfoCol;
    m_PASinkInputInfoCol_t m_PASinkInputInfoCol;
    m_PASinkInfoCol_t      m_PASinkInfoCol;
    
public:

    PA( const std::string& name );
    pa_context* getContext() const;

    const PAServerInfo&     getServerInfo();
    m_PAClientInfoCol_t&    getClientInfoCol();
    fh_PAClientInfo         getClientInfoCol( const std::string& n );
    fh_PAClientInfo         getClientInfoCol( uint32_t cid );
    const std::string       getDefaultSinkName();
    m_PASinkInputInfoCol_t& getInputInfoCol();
    m_PASinkInfoCol_t&      getInfoCol();
    fh_PASinkInfo           getDefaultSink();
    
    /**
     * To dispatch a pa_op call here, note that you have to callback
     * dispatchDone() to indicate when your operation was completed.
     */
    void dispatch( pa_operation* po );
    void dispatchDone();

    //
    // These are private for callbacks only
    //
    void setServerInfo(const pa_server_info *i);
    void appendInputInfo(const pa_sink_input_info *i);
    void appendInfo(const pa_sink_info *i);
    void appendClientInfo(const pa_client_info *i);
};

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

#define CBEND if(userdata) { PA* pa = (PA*)userdata; pa->dispatchDone(); }
#define CBEND_EOL if(eol && userdata) { if( eol ) { PA* pa = (PA*)userdata; pa->dispatchDone(); return; } }
    
void pa_context_success_cb( pa_context *c, int success, void *userdata )
{
    CBEND;
}

void pa_server_info_cb( pa_context *c, const pa_server_info *i, void *userdata )
{
    PA* pa = (PA*)userdata;
    pa->setServerInfo( i );
    CBEND;
}

void pa_sink_input_info_cb( pa_context *c, const pa_sink_input_info *i, int eol, void *userdata )
{
//    cerr << "================================= pa_sink_input_info_cb() eol:" << eol << endl;
    CBEND_EOL;
    PA* pa = (PA*)userdata;
    pa->appendInputInfo( i );
}

m_PASinkInputInfoCol_t&
PA::getInputInfoCol()
{
    m_PASinkInputInfoCol.clear();
    dispatch( pa_context_get_sink_input_info_list( m_ctx, pa_sink_input_info_cb, this ));
    return m_PASinkInputInfoCol;
}

void pa_sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
    CBEND_EOL;

    PA* pa = (PA*)userdata;
    pa->appendInfo( i );
}

void
PA::appendInfo(const pa_sink_info *i)
{
    fh_PASinkInfo o = new PASinkInfo(this);
    o->index  = i->index;
    o->name   = i->name;
    o->desc   = i->description;
    o->volume = i->volume;
    o->mute   = i->mute;
    o->channels = i->channel_map.channels;
    m_PASinkInfoCol.push_back(o);
}


m_PASinkInfoCol_t&
PA::getInfoCol()
{
    m_PASinkInfoCol.clear();
    dispatch( pa_context_get_sink_info_list( m_ctx, pa_sink_info_cb, this ));
    return m_PASinkInfoCol;
}

fh_PASinkInfo
PA::getDefaultSink()
{
    const std::string n = getDefaultSinkName();
    m_PASinkInfoCol_t& c = PA::getInfoCol();
    for( m_PASinkInfoCol_t::iterator iter = c.begin();
         iter != c.end(); ++iter )
    {
        fh_PASinkInfo o = *iter;
        if( o->name == n )
        {
            return o;
        }
    }
    return 0;
}




/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

static void pa_state_cb(pa_context *c, void *userdata)
{
	int *pa_ready = (int*)userdata;
	pa_context_state_t state = pa_context_get_state(c);
//    cerr << "state_cb:" << state << endl;
    
	switch  (state) {
		// There are just here for reference
		case PA_CONTEXT_UNCONNECTED:
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
		default:
			break;
		case PA_CONTEXT_FAILED:
		case PA_CONTEXT_TERMINATED:
			*pa_ready = 2;
			break;
		case PA_CONTEXT_READY:
			*pa_ready = 1;
			break;
	}
}

PA::PA( const std::string& name )
    : m_ctx(0)
    , m_paml(0)
    , m_paapi(0)
    , m_paready(false)
    , m_serverInfo(this)
{
    pa_proplist*     appProps = pa_proplist_new();
    m_paml  = pa_mainloop_new();
    m_paapi = pa_mainloop_get_api( m_paml );
    m_ctx   = pa_context_new_with_proplist( m_paapi,
                                            name.c_str(),
                                            appProps );
    pa_context_connect( m_ctx, NULL, pa_context_flags_t(0), NULL);
    pa_context_set_state_callback( m_ctx, pa_state_cb, &m_paready);
    pa_proplist_free( appProps );

    /**
     * Connect....
     */
    bool block = true;
    while( !m_paready )
    {
        int rc = pa_mainloop_iterate( m_paml, block, NULL );
    }
}

pa_context*
PA::getContext() const
{
    return m_ctx;
}

void
PA::setServerInfo(const pa_server_info *i)
{
    PAServerInfo& o = m_serverInfo;
    o.m_valid = true;
    o.user_name = i->user_name;
    o.host_name = i->host_name;
    o.server_name = i->server_name;
    o.server_version = i->server_version;
    o.default_sink_name = i->default_sink_name;
    o.default_source_name = i->default_source_name;
}


const PAServerInfo&
PA::getServerInfo()
{
    if( m_serverInfo.m_valid )
        return m_serverInfo;

    dispatch( pa_context_get_server_info( m_ctx, pa_server_info_cb, this ));
    return m_serverInfo;
}


    void
    PA::appendClientInfo(const pa_client_info *i)
    {
        DEBUG << "PA::appendClientInfo() idx:" << i->index << endl;
        fh_PAClientInfo o = new PAClientInfo(this);
        o->m_valid = true;
        o->name    = i->name;
        o->index   = i->index;
        m_PAClientInfoCol.push_back(o);
    }
    

static void pa_client_info_cb(pa_context *c, const pa_client_info *i, int eol, void *userdata)
{
    DEBUG << "client_info_cb() eol:" << eol << endl;
    CBEND_EOL;
    PA* pa = (PA*)userdata;
    pa->appendClientInfo( i );
}

    fh_PAClientInfo
    PA::getClientInfoCol( uint32_t cid )
    {
        m_PAClientInfoCol_t& c = getClientInfoCol();
        for( m_PAClientInfoCol_t::iterator iter = c.begin();
             iter != c.end(); ++iter )
        {
//            cerr << "cid:" << cid << " index:" << (*iter)->index << endl;
            if( (*iter)->index == cid )
                return *iter;
        }
        return 0;
    }
        
    
    fh_PAClientInfo
    PA::getClientInfoCol( const std::string& n )
    {
        m_PAClientInfoCol_t& c = getClientInfoCol();
        for( m_PAClientInfoCol_t::iterator iter = c.begin();
             iter != c.end(); ++iter )
        {
            if( (*iter)->getName() == n )
                return *iter;
        }
        return 0;
    }
    
    
    m_PAClientInfoCol_t&
    PA::getClientInfoCol()
    {
        m_PASinkInputInfoCol.clear();
        dispatch( pa_context_get_client_info_list( m_ctx, pa_client_info_cb, this ));
        return m_PAClientInfoCol;
    }
    

const std::string
PA::getDefaultSinkName()
{
    return getServerInfo().default_sink_name;
}


void
PA::appendInputInfo(const pa_sink_input_info *i)
{
    fh_PASinkInputInfo o = new PASinkInputInfo(this);
    o->m_valid = true;
    o->index   = i->index;
    o->name    = i->name;
    o->client  = i->client;
    o->sink    = i->sink;
    o->volume  = i->volume;
    o->mute    = i->mute;
    o->clientIndex = i->client;
    o->channels = i->channel_map.channels;
    m_PASinkInputInfoCol.push_back(o);
}



void
PA::dispatch( pa_operation* po )
{
    m_waitingDispatch = true;
    while( m_waitingDispatch )
    {
        bool block = true;
        int rc = pa_mainloop_iterate( m_paml, block, NULL);
    }
}

void
PA::dispatchDone()
{
    m_waitingDispatch = false;
}

std::string tostr( pa_volume_t v )
{
    char cv[PA_CVOLUME_SNPRINT_MAX];
    pa_volume_snprint(cv, sizeof(cv), v );
    return cv;
}

std::string tostr( pa_cvolume v )
{
    char cv[PA_CVOLUME_SNPRINT_MAX];
    pa_cvolume_snprint(cv, sizeof(cv), &v );
    return cv;
}

double tofloat( pa_volume_t v )
{
    double ret = v;
    if( ret )
        ret /= PA_VOLUME_NORM;
    return ret;
}

double tofloat( std::string s )
{
    stringstream ss;
    ss << s;
    double ret;
    ss >> ret;
    return ret;
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

    void PAVolumeAdjustable::priv_setVolume( pa_volume_t v, int channel )
    {
    }
    
    void PAVolumeAdjustable::priv_setVolume( pa_volume_t v )
    {
    }
    
    double PAVolumeAdjustable::priv_getVolume()
    {
        return 0;
    }
    
    double PAVolumeAdjustable::priv_getVolume( int channel )
    {
        return 0;
    }

    bool PAVolumeAdjustable::isMuted()
    {
        return false;
    }
    
    void PAVolumeAdjustable::setMuted( bool v )
    {
    }

    void PAVolumeAdjustable::toggleMuted()
    {
        bool v = isMuted();
        setMuted( !v );
    }
    
    
    
    void
    PAVolumeAdjustable::setVolume( pa_volume_t v, int channel )
    {
        return priv_setVolume( v, channel );
    }
    
    void
    PAVolumeAdjustable::setVolume( double v, int channel )
    {
        return setVolume( (pa_volume_t)( v  * PA_VOLUME_NORM ), channel );
    }
    
    void
    PAVolumeAdjustable::setVolume( pa_volume_t v )
    {
        return priv_setVolume( v );
    }
    
    void
    PAVolumeAdjustable::setVolume( double v )
    {
        return setVolume( (pa_volume_t)( v  * PA_VOLUME_NORM ) );
    }
    
    double
    PAVolumeAdjustable::getVolume()
    {
        return priv_getVolume();
    }
    
    double
    PAVolumeAdjustable::getVolume( int channel )
    {
        return priv_getVolume( channel );
    }
    
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

    std::string
    PASinkInputInfo::getClientName()
    {
        fh_PAClientInfo ci = m_pa->getClientInfoCol( clientIndex );
        return ci->getName();
    }

double
PASinkInputInfo::priv_getVolume()
{
    return tofloat(pa_cvolume_avg(&volume));
}

double
PASinkInputInfo::priv_getVolume( int channel )
{
    if( channel < 0 )
        return getVolume();
    
    if( channel > channels )
        return 0;
    
    return tofloat(volume.values[channel]);
}


    void
    PASinkInputInfo::priv_setVolume( pa_volume_t v, int channel )
    {
        if( channel == -1 )
            return setVolume( v );
    
        pa_cvolume cvol;
        pa_cvolume_init ( &cvol );
        pa_cvolume_set 	( &cvol, channels, volume.values[0] );
        for( int i = 0; i < channels; ++i )
            cvol.values[i] = volume.values[i];
        cvol.values[channel] = v;

        m_pa->dispatch( pa_context_set_sink_input_volume( m_pa->getContext(),
                                                          index,
                                                          &cvol,
                                                          pa_context_success_cb,
                                                          m_pa ));
    
    }

    void
    PASinkInputInfo::priv_setVolume( pa_volume_t v )
    {
        pa_cvolume cvol;
        pa_cvolume_init ( &cvol );
        pa_cvolume_set 	( &cvol, channels, v );
        m_pa->dispatch( pa_context_set_sink_input_volume( m_pa->getContext(),
                                                          index,
                                                          &cvol,
                                                          pa_context_success_cb,
                                                          m_pa ));
    }


    bool
    PASinkInputInfo::isMuted()
    {
        return mute;
    }
    
    void
    PASinkInputInfo::setMuted( bool v )
    {
        mute = v;
        m_pa->dispatch(
            pa_context_set_sink_input_mute(
                m_pa->getContext(),
                index,
                mute,
                pa_context_success_cb, m_pa ));
    }

    /******************************/
    /******************************/
    /******************************/
    /******************************/
    
// void
// PASinkInfo::setVolume( double v, int channel )
// {
//     return setVolume( (pa_volume_t)( v  * PA_VOLUME_NORM ), channel );
// }
// void
// PASinkInfo::setVolume( double v )
// {
//     return setVolume( (pa_volume_t)( v  * PA_VOLUME_NORM ) );
// }

void
PASinkInfo::priv_setVolume( pa_volume_t v, int channel )
{
    if( channel == -1 )
        return setVolume( v );
    
    pa_cvolume cvol;
    pa_cvolume_init ( &cvol );
    pa_cvolume_set 	( &cvol, channels, volume.values[0] );
    for( int i = 0; i < channels; ++i )
        cvol.values[i] = volume.values[i];
    cvol.values[channel] = v;

    m_pa->dispatch( pa_context_set_sink_volume_by_name( m_pa->getContext(),
                                                        name.c_str(),
                                                        &cvol,
                                                        pa_context_success_cb,
                                                        m_pa ));
    
}


void
PASinkInfo::priv_setVolume( pa_volume_t v )
{
    pa_cvolume cvol;
    pa_cvolume_init ( &cvol );
    pa_cvolume_set 	( &cvol, channels, v );
    m_pa->dispatch( pa_context_set_sink_volume_by_name( m_pa->getContext(),
                                                        name.c_str(),
                                                        &cvol,
                                                        pa_context_success_cb,
                                                        m_pa ));
}

    double
    PASinkInfo::priv_getVolume()
    {
        return tofloat(pa_cvolume_avg(&volume));
    }

    double
    PASinkInfo::priv_getVolume( int channel )
    {
        if( channel < 0 )
            return getVolume();
    
        if( channel > channels )
            return 0;
    
        return tofloat(volume.values[channel]);
    }

    bool
    PASinkInfo::isMuted()
    {
        return mute;
    }
    
    void
    PASinkInfo::setMuted( bool v )
    {
        mute = v;
        m_pa->dispatch(
            pa_context_set_sink_mute_by_name(
                m_pa->getContext(),
                name.c_str(),
                mute,
                pa_context_success_cb, m_pa ));
    }
    

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    class FERRISEXP_CTXPLUGIN SinkInfoVolumeContext
        :
        public StateLessEAHolder< SinkInfoVolumeContext, leafContext >
    {
        typedef StateLessEAHolder< SinkInfoVolumeContext, leafContext > _Base;
        typedef SinkInfoVolumeContext _Self;

        PA* m_pa;
        fh_PAVolumeAdjustable m_info;
        int m_channel;
        
      public:

        SinkInfoVolumeContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_info(0)
            , m_channel(-1)
            {
                createStateLessAttributes();

//                addNullAttr( "user-owner-number" );
            }
        void constructObject( PA* pa, fh_PAVolumeAdjustable o, int channel )
        {
            m_pa   = pa;
            m_info = o;
            m_channel = channel;
        }
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        virtual std::string getRecommendedEA()
        {
            return adjustRecommendedEAForDotFiles(this, "name,content");
        }

        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::trunc | ios::binary;
            }

        fh_stringstream
        real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
                ss << m_info->getVolume( m_channel );
                return ss;
            }

        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            {
                fh_stringstream ret = real_getIOStream( m );
                return ret;
            }
        
        
        fh_iostream
        priv_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ret = real_getIOStream( m );
                ret->getCloseSig().connect(
                    sigc::bind(
                        sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
                return ret;
            }
        
        
        void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
        {
            DEBUG << "SinkInfoVolumeContext::OnStreamClosed()" << endl;
            if( !(m & std::ios::out) )
                return;
            AdjustForOpenMode_Closing( ss, m, tellp );
            const string data = StreamToString(ss);
            DEBUG << "OnStreamClosed() data:" << data << endl;
            DEBUG << "OnStreamClosed() name:" << m_info->getName() << endl;
            DEBUG << "OnStreamClosed()  volA:" << m_info->getVolume( m_channel ) << endl;

            double v = tofloat( data );
            DEBUG << "OnStreamClosed() v:" << v << endl;
            m_info->setVolume( v, m_channel );
        
        }
        
    };


    class FERRISEXP_CTXPLUGIN SinkInfoMuteContext
        :
        public StateLessEAHolder< SinkInfoMuteContext, leafContext >
    {
        typedef StateLessEAHolder< SinkInfoMuteContext, leafContext > _Base;
        typedef SinkInfoMuteContext _Self;

        PA* m_pa;
        fh_PAVolumeAdjustable m_info;
        
      public:

        SinkInfoMuteContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_info(0)
            {
                createStateLessAttributes();

//                addNullAttr( "user-owner-number" );
            }
        void constructObject( PA* pa, fh_PAVolumeAdjustable o )
        {
            m_pa   = pa;
            m_info = o;
        }
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        virtual std::string getRecommendedEA()
        {
            return adjustRecommendedEAForDotFiles(this, "name,content");
        }

        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::trunc | ios::binary;
            }

        fh_stringstream
        real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
                ss << m_info->isMuted();
                return ss;
            }

        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            {
                fh_stringstream ret = real_getIOStream( m );
                return ret;
            }
        
        
        fh_iostream
        priv_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ret = real_getIOStream( m );
                ret->getCloseSig().connect(
                    sigc::bind(
                        sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
                return ret;
            }
        
        
        void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
        {
            DEBUG << "SinkInfoMuteContext::OnStreamClosed()" << endl;
            if( !(m & std::ios::out) )
                return;
            AdjustForOpenMode_Closing( ss, m, tellp );
            const string data = StreamToString(ss);
            DEBUG << "OnStreamClosed() data:" << data << endl;
            bool v = isTrue( data );
            DEBUG << "OnStreamClosed() v:" << v << endl;
            m_info->setMuted(v);
        
        }
        
    };



    class FERRISEXP_CTXPLUGIN SinkInfoContext
        :
        public StateLessEAHolder< SinkInfoContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< SinkInfoContext, FakeInternalContext > _Base;
        typedef SinkInfoContext _Self;

        PA* m_pa;
        fh_PASinkInfo m_info;
        
      public:
        SinkInfoContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_info(0)
            {
                 createStateLessAttributes();
            }
        void constructObject( PA* pa, fh_PASinkInfo o )
        {
            m_pa   = pa;
            m_info = o;
        }
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        void priv_read()
        {
            DEBUG << "infoc priv_read() url:" << getURL()
                  << " have read:" << getHaveReadDir()
                  << endl;
            
            EnsureStartStopReadingIsFiredRAII _raii1( this );
            SinkInfoVolumeContext* c = 0;
            c = priv_ensureSubContext( "volume-all", c );
            c->constructObject( m_pa, m_info, -1 );

            SinkInfoMuteContext* mc = 0;
            mc = priv_ensureSubContext( "mute", mc );
            mc->constructObject( m_pa, m_info );
            
            for( int i=0; i < m_info->channels; ++i )
            {
                std::stringstream rdnss;
                rdnss << "volume-" << i;
                c = priv_ensureSubContext( rdnss.str(), c );
                c->constructObject( m_pa, m_info, i );
            }
            
            
            DEBUG << "infoc read complete...sz:" << getSubContextCount() << endl;
        }
        
    };

    class FERRISEXP_CTXPLUGIN SinkInfoDirContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef SinkInfoDirContext  _Self;

        PA* m_pa;
        
      public:
        SinkInfoDirContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                 createStateLessAttributes();
            }
        void constructObject( PA* pa )
        {
            m_pa = pa;
        }
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        virtual bool isDir()
        {
            return true;
        }
        void priv_read()
        {
            DEBUG << "infodir priv_read() url:" << getURL()
                  << " have read:" << getHaveReadDir()
                  << endl;
            
            EnsureStartStopReadingIsFiredRAII _raii1( this );
            m_PASinkInfoCol_t info = m_pa->getInfoCol();
            for( m_PASinkInfoCol_t::iterator iter = info.begin();
                 iter != info.end(); ++iter )
            {
                fh_PASinkInfo o = *iter;
                std::string rdn = o->name;
                DEBUG << "sinkinfo dir has child:" << rdn << endl;
                SinkInfoContext* c = 0;
                c = priv_ensureSubContext( rdn, c );
                c->constructObject( m_pa, o );
            }

            fh_PASinkInfo o = m_pa->getDefaultSink();
            std::string rdn = "default";
            DEBUG << "sinkinfo dir has child:" << rdn << endl;
            SinkInfoContext* c = 0;
            c = priv_ensureSubContext( rdn, c );
            c->constructObject( m_pa, o );
            
            DEBUG << "sinkinfo dir read complete...sz:" << getSubContextCount() << endl;
        }
        
    };

/******************************/
/******************************/
/******************************/

    class FERRISEXP_CTXPLUGIN ClientStreamDirContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef SinkInfoDirContext  _Self;

        PA* m_pa;
        fh_PASinkInputInfo m_info;

      public:
        ClientStreamDirContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_info(0)
            {
                 createStateLessAttributes();
            }
        void constructObject( PA* pa, fh_PASinkInputInfo info )
        {
            m_pa   = pa;
            m_info = info;
        }
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        void priv_read()
        {
            DEBUG << "streams dir priv_read() url:" << getURL()
                  << " have read:" << getHaveReadDir()
                  << endl;
            
            EnsureStartStopReadingIsFiredRAII _raii1( this );

            fh_PASinkInputInfo o = m_info;
            DEBUG << "name       :" << o->getName() << endl;
            DEBUG << "client-name:" << o->getClientName() << endl;
            
            std::stringstream rdnss;
            rdnss << o->getName();

            FakeInternalContext* fc = 0;
            fc = priv_ensureSubContext( rdnss.str(), fc );
            
            SinkInfoVolumeContext* c = 0;
            c = fc->priv_ensureSubContext( "volume-all", c );
            c->constructObject( m_pa, o, -1 );

            SinkInfoMuteContext* mc = 0;
            mc = fc->priv_ensureSubContext( "mute", mc );
            mc->constructObject( m_pa, o );
            
            for( int i=0; i < m_info->channels; ++i )
            {
                std::stringstream rdnss;
                rdnss << "volume-" << i;
                c = fc->priv_ensureSubContext( rdnss.str(), c );
                c->constructObject( m_pa, m_info, i );
            }
            
        
            DEBUG << "streams dir read complete...sz:" << getSubContextCount() << endl;
        }
        
    };

    class FERRISEXP_CTXPLUGIN StreamsTopLevelDirContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef SinkInfoDirContext  _Self;

        PA* m_pa;
        
      public:
        StreamsTopLevelDirContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                 createStateLessAttributes();
            }
        void constructObject( PA* pa )
        {
            m_pa = pa;
        }
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        virtual bool isDir()
        {
            return true;
        }
        void priv_read()
        {
            DEBUG << "streams dir priv_read() url:" << getURL()
                  << " have read:" << getHaveReadDir()
                  << endl;
            
            EnsureStartStopReadingIsFiredRAII _raii1( this );

            m_PASinkInputInfoCol_t col = m_pa->getInputInfoCol();
            DEBUG << "col.sz:" << col.size() << endl;
            for( m_PASinkInputInfoCol_t::iterator iter = col.begin();
                 iter != col.end(); ++iter )
            {
                fh_PASinkInputInfo o = *iter;
                DEBUG << "name       :" << o->getName() << endl;
                DEBUG << "client-name:" << o->getClientName() << endl;

                std::stringstream rdnss;
                rdnss << o->getClientName();
                ClientStreamDirContext* c = 0;
                c = priv_ensureSubContext( rdnss.str(), c );
                c->constructObject( m_pa, o );
            }
                
            DEBUG << "streams dir read complete...sz:" << getSubContextCount() << endl;
        }
        
    };

/******************************/
/******************************/
/******************************/

    class FERRISEXP_CTXPLUGIN PulseRootContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef PulseRootContext    _Self;

        bool m_haveRead;
        PA   m_pa;
        
      public:


        PulseRootContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_haveRead( false )
            , m_pa("libferris")
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;

            }

        PA* getPA()
        {
            return &m_pa;
        }
        
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                           << " have read:" << getHaveReadDir()
                           << endl;

                if( m_haveRead )
                {
                    emitExistsEventForEachItemRAII _raii2(this);
                    return;
                }
                m_haveRead = true;
                
       
                EnsureStartStopReadingIsFiredRAII _raii1( this );

                SinkInfoDirContext* c = 0;
                c = priv_ensureSubContext( "output", c );
                c->constructObject( getPA() );

                {
                    StreamsTopLevelDirContext* c = 0;
                    c = priv_ensureSubContext( "streams", c );
                    c->constructObject( getPA() );
                }
            }
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
        {
            try
            {
                DEBUG << "Brew()" << endl;

                static PulseRootContext* c = 0;
                if( !c )
                {
                    c = new PulseRootContext(0, "/" );
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                }
                fh_context ret = c;
                return ret;
            }
            catch( exception& e )
            {
                LG_PG_W << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};

