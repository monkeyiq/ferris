/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisannodex.cpp,v 1.6 2010/09/24 21:31:32 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>
#include <Ferris/Runner.hh>

#include <algorithm>
#include <numeric>

#include <config.h>

using namespace std;

extern "C" {
    // This is a real bad hack (TM)
#define class ansiclass
#include <annodex/annodex.h>
#include <cmml.h>
#undef class
};


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ANNODEX_CHUNK_SIZE_TO_READ (4*4096)

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    double totime( CMML_Time* st )
    {
        if( !st )
            return 0;
        
        double ret = 0;
        if( st->type == CMML_UTC_TIME )
        {
            CMML_Time* st2 = cmml_time_utc_to_sec( st, 0, 0 );
            ret = st2->t.sec;
            cmml_time_free( st2 );
        }
        else
        {
            ret = st->t.sec;
        }
        return ret;
    }
    
    
    struct FerrisAnxClip
    {
        FerrisAnxClip( ANNODEX* anx, const AnxClip * c )
            {
                start_time = anx_tell_time (anx);
                
                clip_id = c->clip_id;
                title = c->title;
                lang = c->lang;        
                dir = c->dir;         
                track = c->track;       
                anchor_id = c->anchor_id;   
                anchor_class = c->anchor_class;
                anchor_title = c->anchor_title;
                anchor_lang = c->anchor_lang; 
                anchor_dir = c->anchor_dir;  
                anchor_href = c->anchor_href; 
                anchor_text = c->anchor_text; 
                img_id = c->img_id;      
                img_class = c->img_class;   
                img_title = c->img_title;   
                img_lang = c->img_lang;    
                img_dir = c->img_dir;     
                img_src = c->img_src;     
                img_alt = c->img_alt;     
                desc_id = c->desc_id;     
                desc_class = c->desc_class;  
                desc_title = c->desc_title;  
                desc_lang = c->desc_lang;   
                desc_dir = c->desc_dir;    
                desc_text = c->desc_text;   
            }
        FerrisAnxClip( const CMML_Clip* c )
            :
            start_time( 0 )
            {
                start_time = totime( c->start_time );
        
                clip_id = c->clip_id;
                title = c->title;
                lang = c->lang;        
                dir = c->dir;         
                track = c->track;       
                anchor_id = c->anchor_id;   
                anchor_class = c->anchor_class;
                anchor_title = c->anchor_title;
                anchor_lang = c->anchor_lang; 
                anchor_dir = c->anchor_dir;  
                anchor_href = c->anchor_href; 
                anchor_text = c->anchor_text; 
                img_id = c->img_id;      
                img_class = c->img_class;   
                img_title = c->img_title;   
                img_lang = c->img_lang;    
                img_dir = c->img_dir;     
                img_src = c->img_src;     
                img_alt = c->img_alt;     
                desc_id = c->desc_id;     
                desc_class = c->desc_class;  
                desc_title = c->desc_title;  
                desc_lang = c->desc_lang;   
                desc_dir = c->desc_dir;    
                desc_text = c->desc_text;   
            }
        
        const char *clip_id;         /**< id attribute of clip */
        const char *_class;           /**< class attribute of clip */
        const char *title;           /**< title attribute of clip */
        const char *lang;            /**< language attribute of desc element */
        const char *dir;             /**< directionality of lang */
        const char *track;           /**< track attribute of clip */
        const char *anchor_id;       /**< id attribute of anchor */
        const char *anchor_class;    /**< class attribute of clip */
        const char *anchor_title;    /**< title attribute of clip */
        const char *anchor_lang;     /**< language of anchor */
        const char *anchor_dir;      /**< directionality of lang */
        const char *anchor_href;     /**< href out of clip */
        const char *anchor_text;     /**< anchor text */
        const char *img_id;          /**< id attribute of image */
        const char *img_class;       /**< class attribute of image */
        const char *img_title;       /**< title attribute of image */
        const char *img_lang;        /**< language of img */
        const char *img_dir;         /**< directionality of lang */
        const char *img_src;         /**< keyframe image of clip */
        const char *img_alt;         /**< alternate text for image */
        const char *desc_id;         /**< id attribute of desc element */
        const char *desc_class;      /**< class attribute of desc */
        const char *desc_title;      /**< title attribute of desc */
        const char *desc_lang;       /**< language attribute of desc element */
        const char *desc_dir;        /**< directionality of lang */
        const char *desc_text;       /**< the description itself */

        double start_time; // botched between AnxClip and CMML_Clip by me.
    };
    
    
    /*
     * context for a clip file itself
     */
    class FERRISEXP_CTXPLUGIN annodexClipContext
        :
        public leafContext
    {
        typedef annodexClipContext  _Self;
        typedef leafContext         _Base;

        string dataOrEmpty( const char* d )
            {
                return d ? d : "";
            }
        
    protected:

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;

                static Util::SingleShot v;
                if( v() )
                {
                    anx_init_importers ("*/*");
                }

                
                string infilename = getParent()->getDirPath();
                char templatestr[100] = "/tmp/libferris-annodex-tmp-XXXXXX";
                int fd = mkstemp( templatestr );
                if( fd == -1 )
                {
                    fh_stringstream ss;
                    ss << errnum_to_string( "Failed to create a temporary file:", errno );
                    Throw_CanNotGetStream( tostr(ss), this );
                }
                FILE* outfile = fopen( templatestr, "w" );
                if( !outfile )
                {
                    unlink( templatestr );
                    fh_stringstream ss;
                    ss << errnum_to_string( "Failed to open a temporary file:", errno );
                    Throw_CanNotGetStream( tostr(ss), this );
                }
                unlink( templatestr );

                errno = 0;
                ANNODEX* anx = anx_open_stdio( outfile, ANX_WRITE);
                if ( !anx )
                {
                    fh_stringstream ss;
                    ss << errnum_to_string( "Failed to open annodex temporary file:", errno );
                    Throw_CanNotGetStream( tostr(ss), this );
                }

                double seek_offset = toType<double>(getStrAttr( this, "start", "0" ));
                double seek_end    = toType<double>(getStrAttr( this, "end",   "0" ));
                
                LG_ANNODEX_D << "priv_getIStream() seek_offset:" << seek_offset
                             << " seek_end:" << seek_end << endl;
                anx_set_presentation_time (anx, seek_offset);
                anx_set_basetime (anx, 0.0);

                int import_rc = anx_write_import (anx, (char*)infilename.c_str(), NULL, NULL,
                                                  seek_offset, seek_end, 0);
                
                if ( import_rc == -1 )
                {
                    anx_close (anx);
                    close( fd );
                    fh_stringstream ss;
                    ss << errnum_to_string( "Failed import of original file:", errno );
                    Throw_CanNotGetStream( tostr(ss), this );
                }

                int n = -1;
                while ((n = anx_write (anx, ANNODEX_CHUNK_SIZE_TO_READ )) != 0);

                if (anx_close (anx) != NULL)
                {
                    close( fd );
                    fh_stringstream ss;
                    ss << errnum_to_string( "Failed to close annodex stream:", errno );
                    Throw_CanNotGetStream( tostr(ss), this );
                }

                
                lseek( fd, 0, SEEK_SET );
                fh_istream ret = Factory::MakeFdIStream( fd, true );
                return ret;
            }

    public:
        
        annodexClipContext( Context* parent, std::string rdn,
                            const FerrisAnxClip& clip, double start_time )
            :
            _Base( parent, rdn )
            {
                XSDBasic_t sc = XSD_BASIC_STRING;

                addAttribute( "start", tostr( start_time ), XSD_BASIC_DOUBLE );
                
                addAttribute( "clip-id", dataOrEmpty( clip.clip_id ), sc );
//                addAttribute( "class",   dataOrEmpty( clip._class ), sc );
                addAttribute( "title", dataOrEmpty( clip.title ), sc );
                addAttribute( "language-human", dataOrEmpty( clip.lang ), sc );
                addAttribute( "track", dataOrEmpty( clip.track ), sc );

                addAttribute( "anchor-id", dataOrEmpty( clip.anchor_id ), sc );
                addAttribute( "anchor-class", dataOrEmpty( clip.anchor_class ), sc );
                addAttribute( "anchor-title", dataOrEmpty( clip.anchor_title ), sc );
                addAttribute( "anchor-lang", dataOrEmpty( clip.anchor_lang ), sc );
                addAttribute( "anchor-href", dataOrEmpty( clip.anchor_href ), sc );
                addAttribute( "anchor-text", dataOrEmpty( clip.anchor_text ), sc );

                addAttribute( "image-id", dataOrEmpty( clip.img_id ), sc );
                addAttribute( "image-class", dataOrEmpty( clip.img_class ), sc );
                addAttribute( "image-title", dataOrEmpty( clip.img_title ), sc );
                addAttribute( "image-language-human", dataOrEmpty( clip.img_lang ), sc );
                addAttribute( "image-text", dataOrEmpty( clip.img_alt ), sc );
//                addAttribute( "rgba-32bpp", dataOrEmpty( clip.img_src ), sc );
//                addAttribute( "width", dataOrEmpty( clip.img_src ), sc );
//                addAttribute( "height", dataOrEmpty( clip.img_src ), sc );

                addAttribute( "desc-id", dataOrEmpty( clip.desc_id ), sc );
                addAttribute( "desc-class", dataOrEmpty( clip.desc_class ), sc );
                addAttribute( "desc-title", dataOrEmpty( clip.desc_title ), sc );
                addAttribute( "desc-language-human", dataOrEmpty( clip.desc_lang ), sc );
                addAttribute( "desc-text", dataOrEmpty( clip.desc_text ), sc );
                addAttribute( "description", dataOrEmpty( clip.desc_text ), sc );

                LG_ANNODEX_D << "clip name:" << rdn << " desc:" << dataOrEmpty( clip.desc_text ) << endl;
            }
        
        virtual ~annodexClipContext()
            {}

        virtual std::string getRecommendedEA()
            {
                return "name,title,start,end,anchor-href,description";
            }

        void setEndTime( double t )
            {
                addAttribute( "end", tostr( t ), XSD_BASIC_DOUBLE );
            }
    };



    
    static int tramp_read_head (ANNODEX * anx, const AnxHead * head, void * user_data);
    static int tramp_read_clip (ANNODEX * anx, const AnxClip * clip, void * user_data);
    static int tramp_read_head_cmml (CMML *cmml, const CMML_Head *head, void *user_data);
    static int tramp_read_clip_cmml (CMML *cmml, const CMML_Clip *clip, void *user_data);
    FERRIS_CTX_SMARTPTR( annodexClipContext, fh_annodexClipContext );

    /*
     * context for media file itself
     */
    class FERRISEXP_CTXPLUGIN annodexContext
        :
        public FakeInternalContext
    {
        typedef annodexContext     _Self;
        typedef FakeInternalContext _Base;

        fh_annodexClipContext m_lastCreatedClipContext;
        int m_nextn;
        
    protected:

        string getAnnodexFileName()
            {
                annodexContext* c = dynamic_cast<annodexContext*>(this);
                while( c && c->isParentBound() )
                {
                    Context* p = c->getParent()->getOverMountContext();
                    if(annodexContext* nextc = dynamic_cast<annodexContext*>( p ))
                    {
                        c = nextc;
                    }
                    else
                    {
                        break;
                    }
                }
                string ret = c->getURL();
                if( starts_with( ret, "http:///" ) )
                    ret.replace( 0, 8, "http://" );
                return ret;
            }
        
        virtual void priv_read()
            {
                staticDirContentsRAII _raii1( this );
                
                LG_ANNODEX_D << "annodexContext::priv_read()" << endl;
                if( empty() )
                {
                    string infilename = getURL();
                    LG_ANNODEX_D << "priv_read() infilename:" << infilename << endl;

                    if( starts_with( infilename, "http:" ) )
                    {
                        infilename = getAnnodexFileName();
                        LG_ANNODEX_D << "priv_read(http) infilename:" << infilename << endl;
                        
                        stringstream cmdss;
                        cmdss << "curl -H \"Accept: text/x-cmml\" "
                              << ImplementationDetail::getCURLProxyCommandLineOption()
                              << " -L " << infilename;
                        fh_runner r = new Runner();
                        r->setSpawnFlags(
                            GSpawnFlags(
                                G_SPAWN_SEARCH_PATH |
                                G_SPAWN_STDERR_TO_DEV_NULL |
                                r->getSpawnFlags()));
                        r->setCommandLine( tostr(cmdss) );
                        LG_ANNODEX_D << "About to run cmd:" << tostr(cmdss) << endl;
                        r->Run();
                        fh_istream stdoutss = r->getStdOut();
                        stringstream datass;
                        LG_ANNODEX_D << "getting data..." << endl;
                        copy( istreambuf_iterator<char>(stdoutss),
                              istreambuf_iterator<char>(),
                              ostreambuf_iterator<char>(datass));
                        LG_ANNODEX_D << "reaping child..." << endl;
                        gint e = r->getExitStatus();
                        if( e > 1 )
                        {
                            stringstream ss;
                            ss << "Can not get CMML content for HTTP requested annodex file:" << infilename << endl;
                            Throw_CanNotReadContext( tostr(ss), this );
                        }
                        LG_ANNODEX_D << "data:" << tostr(datass) << endl;
                        
                        
                        char templatestr[100] = "/tmp/libferris-annodex-cmml-tmp-XXXXXX";
                        {
                            int fd = mkstemp( templatestr );
                            string d = tostr(datass);
                            write( fd, d.c_str(), d.length() );
                            close( fd );
                        }
//                        char templatestr[100] = "/tmp/libferris-annodex-cmml-tmp-eD6rEU";
                        FILE* ifile = fopen( templatestr, "r" );
                        unlink( templatestr );
                        CMML* doc = cmml_new( ifile );
                        cmml_set_read_callbacks (doc, NULL,
                                                 tramp_read_head_cmml,
                                                 tramp_read_clip_cmml,
                                                 this );

                        int bytes_read = 0;
                        int n = 0;
                        LG_ANNODEX_D << "starting bytes_read:" << bytes_read << endl;
                        while (((n = cmml_read (doc, ANNODEX_CHUNK_SIZE_TO_READ )) > 0))
                        {
                            bytes_read += n;
                            LG_ANNODEX_D << "bytes_read:" << bytes_read << endl;
                        }
                        
                        if( m_lastCreatedClipContext )
                        {
                            m_lastCreatedClipContext->setEndTime( end_time );
                            m_lastCreatedClipContext = 0;
                        }
                        
                        ifile = cmml_destroy(doc);
                        fclose(ifile);
                    }
                    else
                    {
                        if( starts_with( infilename, "file:" ) )
                            infilename = getDirPath();

                        LG_ANNODEX_D << "priv_read(not http) infilename:" << infilename << endl;
                        
                        ANNODEX* anx = NULL;
                        anx = anx_open ((char*)infilename.c_str(), ANX_READ);
                        if (anx)
                        {
                            anx_set_read_head_callback (anx, tramp_read_head, this );
                            anx_set_read_clip_callback (anx, tramp_read_clip, this );
                            int n = -1;
                            int bytes_read = 0;
                            while ((n = anx_read (anx, ANNODEX_CHUNK_SIZE_TO_READ )) > 0)
                            {
                                bytes_read+=n;
                                LG_ANNODEX_D << "annodexContext::priv_read() bytes_read:" << bytes_read << endl;
                            }
                            if (n == -1)
                            {
                                anx_close (anx);
                                fh_stringstream ss;
                                ss << errnum_to_string( "Failed to read annodex stream:", errno );
                                Throw_CanNotReadContext( tostr(ss), this );
                            }

                            double t = anx_tell_time (anx);
                            if( m_lastCreatedClipContext )
                            {
                                m_lastCreatedClipContext->setEndTime( t );
                                m_lastCreatedClipContext = 0;
                            }
                        
                            if (anx_close (anx) != NULL)
                            {
                                fh_stringstream ss;
                                ss << errnum_to_string( "Failed to read close annodex stream:", errno );
                                Throw_CanNotReadContext( tostr(ss), this );
                            }
                        }
                    }
                }
            }
    public:

        double end_time;
        annodexContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_nextn( 1 ),
            m_lastCreatedClipContext( 0 ),
            end_time( 0 )
            {
                LG_ANNODEX_D << "annodexContext()" << endl;
                createStateLessAttributes();
            }
        annodexContext()
            :
            end_time( 0 )
            {
                createStateLessAttributes();
                createAttributes();
            }
        
        virtual ~annodexContext()
            {
            }

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        annodexContext* priv_CreateContext( Context* parent, string rdn )
            {
                annodexContext* ret = new annodexContext();
                ret->setContext( parent, rdn );
                return ret;
            }


        int read_head( void*, const AnxHead * head )
            {
                if( head->head_id )
                    addAttribute( "header-id", head->head_id, XSD_BASIC_STRING );
                if( head->lang )
                    addAttribute( "header-language-human", head->lang, XSD_BASIC_STRING );
                if( head->profile )
                    addAttribute( "header-profile", head->profile, XSD_BASIC_STRING );
                if( head->title )
                    addAttribute( "title", head->title, XSD_BASIC_STRING );
                if( head->title_id )
                    addAttribute( "title-id", head->title_id, XSD_BASIC_STRING );
                if( head->title_lang )
                    addAttribute( "title-language-human", head->title_lang, XSD_BASIC_STRING );
                if( head->base_id )
                    addAttribute( "base-id", head->base_id, XSD_BASIC_STRING );
                if( head->base_href )
                    addAttribute( "base-href", head->base_href, XSD_BASIC_STRING );
                
                return 0;
            }

        /**
         * Work out a suitable unique file name for the clip
         */
        string deduce_rdn( const FerrisAnxClip& c )
            {
                string rdn = "";
                
                if( c.clip_id )         rdn = c.clip_id;
                else if( c.title )      rdn = c.title;
                else if( c.anchor_id )  rdn = c.anchor_id;
                else                    rdn = tostr( m_nextn++ );
                
                rdn = monsterName( rdn );
                return rdn;
            }

        int read_clip( const FerrisAnxClip& clip )
            {
                double t = clip.start_time;
                string rdn = deduce_rdn( clip );

                if( m_lastCreatedClipContext )
                {
                    m_lastCreatedClipContext->setEndTime( t );
                }
                LG_ANNODEX_D << "read_clip() rdn:" << rdn << endl;
                fh_annodexClipContext c = new annodexClipContext( this, rdn, clip, t );
                m_lastCreatedClipContext = c;
                addNewChild( c );
                return 0;
            }
    };
    
    static int tramp_read_head (ANNODEX * anx, const AnxHead * head, void * user_data)
    {
        return ((annodexContext*)(user_data))->read_head( anx, head );
    }
    
    static int tramp_read_clip (ANNODEX * anx, const AnxClip * clip, void * user_data)
    {
        FerrisAnxClip fc( anx, clip );
        return ((annodexContext*)(user_data))->read_clip( fc );
    }
    static int tramp_read_head_cmml (CMML *cmml, const CMML_Head *head, void *user_data)
    {
        LG_ANNODEX_D << "tramp_read_head_cmml(called)" << endl;
        int ret = ((annodexContext*)(user_data))->read_head( cmml, (const AnxHead *)head );
        LG_ANNODEX_D << "tramp_read_head_cmml(ret:" << ret << ")" << endl;
        return ret;
    }
    static int tramp_read_clip_cmml (CMML *cmml, const CMML_Clip *clip, void *user_data)
    {
        annodexContext* ac = (annodexContext*)user_data;
        ac->end_time = totime( clip->end_time );
        
        FerrisAnxClip fc( clip );
        return ((annodexContext*)(user_data))->read_clip( fc );
    }
    

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                static annodexContext c;
                fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
                return ret;
            }
            catch( exception& e )
            {
                LG_ANNODEX_ER << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
