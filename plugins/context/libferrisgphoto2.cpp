/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: libferrisgphoto2.cpp,v 1.4 2010/09/24 21:31:38 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Ferris.hh>
#include <TypeDecl.hh>
#include <Trimming.hh>
#include <General.hh>
#include <Cache.hh>

#include <gphoto2.h>

#include <config.h>

#ifdef HAVE_IMLIB2
extern "C" {
#include <X11/Xlib.h>
#include <Imlib2.h>
};

#endif

using namespace std;


namespace Ferris
{
    static const std::string THUMBNAIL_EANAME_HAS_EXIF_THUMB = "exif:has-thumbnail";
    static const std::string THUMBNAIL_EANAME_RGBA           = "exif:thumbnail-rgba-32bpp";
    static const std::string THUMBNAIL_EANAME_W              = "exif:thumbnail-width";
    static const std::string THUMBNAIL_EANAME_H              = "exif:thumbnail-height";

    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf );
    };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    // Version 1.1
    
    class GPhoto;
    FERRIS_SMARTPTR( GPhoto, fh_gphoto );
    typedef std::list< fh_gphoto > cameras_t;
    
    class FERRISEXP_DLLLOCAL GPhoto
        :
        public CacheHandlable
    {
        Camera*    m_camera;
        GPContext* m_context;
        string     m_folder;
        string     m_filename;
        CameraAbilitiesList* m_abilities_list;

        void setModel( const std::string& model );
        void setPort( const std::string& port );
        stringlist_t tostrlist( CameraList* list );
        
    public:

        GPhoto( const std::string& model, const std::string& port = "usb:" );
        virtual ~GPhoto();

        static cameras_t Detect();
        static fh_gphoto getDefaultCamera();

        string getModel();
        
        stringlist_t getFolderNames( const std::string& path );
        stringlist_t getFileNames( const std::string& path );
        fh_istream getIStream( const std::string& path,
                               const std::string& filename,
                               CameraFileType type = GP_FILE_TYPE_NORMAL );
        fh_istream getThumbIStream( const std::string& path,
                                    const std::string& filename )
            {
                return getIStream( path, filename, GP_FILE_TYPE_PREVIEW );
            }
        
        void DeleteFile( const std::string& path,
                         const std::string& filename );
        
        bool getFileInfo( const std::string& path,
                          const std::string& filename,
                          CameraFileInfo* info )
            {
                int rc = gp_camera_file_get_info( m_camera,
                                                  path.c_str(),
                                                  filename.c_str(),
                                                  info,
                                                  m_context);
                return rc >= 0;
            }
        
        
    };

    struct PortInfoHolder
    {
        GPPortInfoList *il;
        PortInfoHolder( GPPortInfoList *il )
            : il( il ) {}
        ~PortInfoHolder() 
            {
                if( il ) gp_port_info_list_free (il);
            }
    };
    struct FileHolder
    {
        CameraFile* f;
        FileHolder( CameraFile* f )
            : f( f ) {}
        ~FileHolder() 
            {
                if( f ) gp_file_unref (f);
            }
    };

    
    
    GPhoto::GPhoto( const std::string& model, const std::string& port )
        :
        m_camera(0),
        m_context(0),
        m_folder("/"),
        m_filename(""),
        m_abilities_list(0)
    {
        gp_camera_new( &m_camera );
        m_context = gp_context_new();

        gp_abilities_list_new( &m_abilities_list );
        gp_abilities_list_load( m_abilities_list, m_context );

        setModel( model );
        setPort( port );
    }

    GPhoto::~GPhoto()
    {
        if (m_abilities_list) gp_abilities_list_free (m_abilities_list);
        if (m_camera)         gp_camera_unref (m_camera);
        if (m_context)        gp_context_unref (m_context);
    }
    
    cameras_t
    GPhoto::Detect()
    {
        cameras_t ret;
        
        GPContext* context = gp_context_new();
        int x, count;
        CameraList*          list      = NULL;
        CameraAbilitiesList* abilities = NULL;
        GPPortInfoList*      infolist  = NULL;
        const char *name = NULL, *value = NULL;

        gp_list_new( &list );
        gp_abilities_list_new( &abilities );
        gp_abilities_list_load( abilities, context );
        gp_port_info_list_new( &infolist );
        gp_port_info_list_load( infolist );
        gp_abilities_list_detect( abilities, infolist, list, context );
        gp_abilities_list_free( abilities );
        gp_port_info_list_free( infolist );

        count = gp_list_count ( list );

        try
        {
            for (x = 0; x < count; x++)
            {
                gp_list_get_name  ( list, x, &name);
                gp_list_get_value ( list, x, &value);
                
                ret.push_back( new GPhoto( name, value ));
            }
        }
        catch( exception& e )
        {
            if (context) gp_context_unref (context);
            if( list ) gp_list_free( list );
            throw;
        }
        
        if (context) gp_context_unref (context);
        if( list ) gp_list_free( list );
        return ret;
    }

    fh_gphoto
    GPhoto::getDefaultCamera()
    {
        cameras_t cl = Detect();
        if( cl.empty() )
        {
            fh_stringstream ss;
            ss << "No cameras found...";
            cerr << tostr(ss) << endl;
            Throw_GPhoto2( tostr(ss), 0 );
        }
        return cl.front();
    }
    
    int HandleErr( const std::string& msg, int rc )
    {
        if( rc < 0 )
        {
            LG_GPHOTO_D << msg;
//            cerr << msg;
            
            // FIXME: throw
            fh_stringstream ss;
            ss << "Gphoto2 error during:" << msg
               << " Error:" << gp_result_as_string( rc )
               << endl;
            Throw_GPhoto2( tostr(ss), 0 );
        }
        return rc;
    }
    
    int HandleErr( int rc )
    {
        return HandleErr("",rc);
    }

    void
    GPhoto::setModel( const std::string& model )
    {
        CameraAbilities a;
        int m;

        HandleErr(m = gp_abilities_list_lookup_model (m_abilities_list, model.c_str()));
        HandleErr(gp_abilities_list_get_abilities (m_abilities_list, m, &a));
        HandleErr(gp_camera_set_abilities (m_camera, a));
        gp_setting_set ("gphoto2", "model", a.model);
    }

    string
    GPhoto::getModel()
    {
        CameraAbilities a;
        HandleErr(gp_camera_get_abilities (m_camera, &a));
        return a.model;
    }
    
    void
    GPhoto::setPort( const std::string& port )
    {
        GPPortInfoList *il = NULL;
        PortInfoHolder h_il( il );
        int p;
        GPPortInfo info;

        /* Create the list of ports and load it. */
        HandleErr( "Getting port info list...", gp_port_info_list_new (&il));
        HandleErr( "Getting port info list...", gp_port_info_list_load (il));

        /* Search our port in the list. */
        p = gp_port_info_list_lookup_path (il, port.c_str() );
        if( p == GP_ERROR_UNKNOWN_PORT )
        {
            fh_stringstream ss;
            ss << "Can't find the port you specified:" << port;
            Throw_GPhoto2( tostr(ss), 0 );
        }
        HandleErr( "Getting port...", p );

        /* Get info about our port. */
        HandleErr( "Getting port information...",
                   gp_port_info_list_get_info (il, p, &info));

        /* Set the port of our camera. */
        HandleErr( "Setting port information...",
                   gp_camera_set_port_info ( m_camera, info));
        
//        gp_setting_set ("gphoto2", "port", info.path);
        char *path = 0;
        gp_port_info_get_path (info, &path);
        gp_setting_set ("gphoto2", "port", path);
        g_free(path);
    }
    
    stringlist_t
    GPhoto::tostrlist( CameraList* list )
    {
        stringlist_t ret;

        const char *name;
        unsigned int i;
        int count = gp_list_count( list );
        
        for (i = 0; i < count; i++)
        {
            HandleErr("getting folder name",
                      gp_list_get_name ( list, i, &name));
            ret.push_back( name );
        }
        return ret;
    }


    stringlist_t
    GPhoto::getFolderNames( const std::string& path )
    {
        CameraList* list = 0;
        LG_GPHOTO_D << "GPhoto::getFolderNames() path:" << path
                    << " m_camera:" << toVoid(m_camera)
                    << " m_context:" << toVoid(m_context)
                    << endl;
        gp_list_new( &list );
        gp_list_reset( list );
        HandleErr( "listing folders",
                   gp_camera_folder_list_folders ( m_camera,
                                                   path.c_str(),
                                                   list,
                                                   m_context));

        stringlist_t ret = tostrlist( list );
        gp_list_free( list );
        return ret;
    }
    
    stringlist_t
    GPhoto::getFileNames( const std::string& path )
    {
        CameraList* list;
        gp_list_new( &list );
        gp_list_reset( list );
        HandleErr( "error getting file list...",
                   gp_camera_folder_list_files (
                       m_camera, path.c_str(), list, m_context));

        stringlist_t ret = tostrlist( list );
        gp_list_free( list );
        return ret;
    }
    
    
    fh_istream
    GPhoto::getIStream( const std::string& path,
                        const std::string& filename,
                        CameraFileType type )
    {
        CameraFile *file = 0;
        FileHolder h_file( file );

        HandleErr( "Making gphoto2 file handle...",
                   gp_file_new (&file));
        
        HandleErr( "Populating gphoto2 file handle...",
                   gp_camera_file_get (m_camera,
                                       path.c_str(),
                                       filename.c_str(),
                                       type,
                                       file,
                                       m_context) );

        fh_stringstream ss;
        const char *data=0;
        long unsigned int size=0;

        HandleErr( "Getting file data...",
                   gp_file_get_data_and_size (file, &data, &size));

        ss.write( data, size );
        return ss;
    }

    
    void
    GPhoto::DeleteFile( const std::string& path,
                        const std::string& filename )
    {
        HandleErr( "Deleting file",
                   gp_camera_file_delete (m_camera,
                                          path.c_str(),
                                          filename.c_str(),
                                          m_context));
    }

    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    
    class gphotoContext;
    FERRIS_SMARTPTR( gphotoContext, fh_gphotoCtx );
    
    class FERRISEXP_CTXPLUGIN gphotoContext
        :
        public StateLessEAHolder< gphotoContext, FakeInternalContext >
    {
        typedef gphotoContext                    _Self;
        typedef StateLessEAHolder< gphotoContext, FakeInternalContext > _Base;

        
        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/

        friend fh_context Brew( RootContextFactory* rf );
        gphotoContext* priv_CreateContext( Context* parent, string rdn )
            {
                gphotoContext* ret = new gphotoContext();
                ret->setContext( parent, rdn );
                return ret;
            }

        _Self* getBaseContext()
            {
                if( this->getOverMountContext() != this )
                {
                    _Self* c = dynamic_cast<_Self*>(
                        this->getOverMountContext());
                    return c->getBaseContext();
                }

                
                _Self* c = dynamic_cast<_Self*>(this);
                while( c && c->isParentBound() )
                {
                    Context* p = c->getParent()->getOverMountContext();
                    if(_Self* nextc = dynamic_cast<_Self*>( p ))
                    {
                        c = nextc;
                    }
                    else
                    {
                        return c;
                    }
                }
                return c;
            }

        _Self* getCameraModelContext()
            {
                _Self* c = dynamic_cast<_Self*>(this);
                while( c && c->isParentBound() )
                {
                    _Self* p = dynamic_cast<_Self*>(c->getParent()->getOverMountContext());
//                     cerr << "gp_path(building) pp:" << p->isParentBound()
//                          << " c:" << c->getDirPath()
//                          << endl;
                    if( !p->isParentBound() )
                        return c;
                    c = p;
                }
                return 0;
            }
        
        fh_gphoto getCamera()
            {
//                _Self* s = getBaseContext();
                _Self* s = getCameraModelContext();
                if( !s )
                {
                    LG_GPHOTO_D << "Can not get camera model context for url:" << getURL() << endl;
                    return 0;
                }
                
                static Cache< _Self*, fh_gphoto > cache;
                cache.setTimerInterval( 3000 );

                if( fh_gphoto d = cache.get( s ) )
                {
                    LG_GPHOTO_D << "getCamera(cached object):" << s->getURL()
                                << " model:" << s->getDirName() << endl;
                    return d;
                }
                
                LG_GPHOTO_D << "getCamera(creating object):" << s->getURL()
                            << " model:" << s->getDirName() << endl;
                fh_gphoto ret = new GPhoto( s->getDirName() );
                cache.put( s, ret );
                return ret;
            }
        
        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/
    protected:

        long priv_read_addFiles( const stringlist_t& fl );
        virtual void priv_read();

        virtual bool
        supportsRename()
            {
                return false;
            }

        virtual bool supportsRemove()
            {
                return false;
            }

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            {
                return getCamera()->getIStream( gp_path(), gp_filename() );
            }
        bool m_info_isValid;
        CameraFileInfo m_info;
        
    public:
        
        gphotoContext()
            :
            m_info_isValid( false )
            {
                createStateLessAttributes();
                createAttributes();
            }

        virtual ~gphotoContext()
            {
            }

        string gp_path( bool DontTakeParent = false )
            {
                if( !isParentBound() )
                    return "/";

                stringlist_t sl;
                
                Context* c = dynamic_cast<_Self*>(this);
                if( !DontTakeParent && c->isParentBound() )
                    c = c->getParent();
                while( c && c->isParentBound() )
                {
                    Context* p = c->getParent()->getOverMountContext();
//                     cerr << "gp_path(building) pp:" << p->isParentBound()
//                          << " c:" << c->getDirPath()
//                          << endl;
                    if( p->isParentBound() )
                        sl.push_front( c->getDirName() );
                    c = p;
                }

                fh_stringstream ss;
                if( sl.empty() )
                {
                    ss << "/";
                }
                else
                {
                    for( stringlist_t::const_iterator si=sl.begin();si!=sl.end(); ++si )
                    {
                        ss << "/" << *si;
                    }
                }
                

//                 cerr << "gp_path() dirpath:" << getDirPath()
//                      << " ret:" << tostr(ss)
//                      << endl;
                
                return tostr(ss);
            }
        string gp_filename()
            {
                return getDirName();
            }

        CameraFileInfo& getFileInfo()
            {
                if( !m_info_isValid )
                {
                    if( isParentBound() )
                    {
                        LG_GPHOTO_D << "getFileInfo() this:" << getURL()
                                    << " gp_path:" << gp_path()
                                    << " gp_filename:" << gp_filename()
                                    << endl;
                        
                        m_info_isValid = true;
                        getCamera()->getFileInfo( gp_path(),
                                                  gp_filename(),
                                                  &m_info );
                    }
                }
                return m_info;
            }
        
        
        virtual std::string priv_getMimeType( bool fromContent = false )
            {
                CameraFileInfo& inf = getFileInfo();
                if (inf.file.fields == GP_FILE_INFO_NONE)
                    return "unknown";
                
                return inf.file.type;
            }

        fh_stringstream
        static SL_getSizeStream( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;
                if (inf.file.fields == GP_FILE_INFO_NONE)
                    ss << 0;
                else
                    ss << inf.file.size;
                return ss;
            }

        fh_stringstream
        static SL_getWidthStream( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;
                if (inf.file.fields == GP_FILE_INFO_NONE)
                    ss << 0;
                else
                    ss << inf.file.width;
                return ss;
            }

        fh_stringstream
        static SL_getHeightStream( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;
                if (inf.file.fields == GP_FILE_INFO_NONE)
                    ss << 0;
                else
                    ss << inf.file.height;
                return ss;
            }
        
        fh_stringstream
        static SL_getHasBeenDownLoadedStream( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;
                if (inf.file.fields == GP_FILE_INFO_NONE)
                    ss << 0;
                else
                    ss << (inf.file.status == GP_FILE_STATUS_DOWNLOADED);
                return ss;
            }
        
        fh_stringstream
        static SL_getReadable( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;
                if (inf.file.fields == GP_FILE_INFO_NONE)
                    ss << 0;
                else
                    ss << (inf.file.permissions & GP_FILE_PERM_READ);
                return ss;
            }
        
        fh_stringstream
        static SL_getDeletable( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;
                if (inf.file.fields == GP_FILE_INFO_NONE)
                    ss << 0;
                else
                    ss << (inf.file.permissions & GP_FILE_PERM_DELETE);
                return ss;
            }
        
        fh_stringstream
        static SL_getMTimeRawStream( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                LG_GPHOTO_D << "SL_getMTimeRawStream() c:" << c->getURL()
                            << " mtime-valid:" << (inf.file.fields != GP_FILE_INFO_NONE)
                            << " mtime:" << inf.file.mtime
                            << endl;
                
                fh_stringstream ss;
                if (inf.file.fields == GP_FILE_INFO_NONE)
                {
                    fh_stringstream ss;
                    ss << "mtime unavailable for file:" << c->getURL();
                    Throw_GPhoto2( tostr(ss), 0 );
                }
                else
                    ss << inf.file.mtime;
                return ss;
            }

        fh_stringstream
        static SL_getHasThumbStream( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;
                ss << (inf.preview.fields != GP_FILE_INFO_NONE);
                return ss;
            }

        fh_stringstream
        static SL_getThumbWidthStream( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;

                if( inf.preview.fields == GP_FILE_INFO_NONE )
                    ss << 0;
                else
                    ss << inf.preview.width;
                
                return ss;
            }

        fh_stringstream
        static SL_getThumbHeightStream( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;

                if( inf.preview.fields == GP_FILE_INFO_NONE )
                    ss << 0;
                else
                    ss << inf.preview.height;
                
                return ss;
            }

        fh_stringstream
        static SL_getThumbRGBAStream( gphotoContext* c, const std::string& rdn, EA_Atom* atom )
            {
                CameraFileInfo& inf = c->getFileInfo();
                fh_stringstream ss;
                
                if( inf.preview.fields == GP_FILE_INFO_NONE )
                {
                    fh_stringstream ss;
                    ss << "No thumbnail for image:" << c->getURL();
                    Throw_GPhoto2( tostr(ss), 0 );
                }
#ifdef HAVE_IMLIB2

                LG_GPHOTO_D << "Decoding thumbnail image to raw RGBA32 from camera" << endl;
                
                int width = inf.preview.width;
                int height = inf.preview.height;
                fh_istream thumbiss = c->getCamera()->getThumbIStream(
                    c->gp_path(), c->gp_filename() );
                string thumbstr = StreamToString( thumbiss );

                LG_GPHOTO_D << "width:" << width << " height:" << height
                            << " thumbnail.data.size:" << thumbstr.size()
                            << endl;

//                 Imlib_Load_Error imlib2err;
//                 im = imlib_load_image_with_error_return( dn.c_str(), &imlib2err );
                Imlib_Image im = imlib_create_image_using_data( width, height,
                                                                (uint*)thumbstr.data() );
                imlib_context_set_image(im);
                DATA32* d = imlib_image_get_data_for_reading_only();

                ss.write( (char*)d, width * height * 4 );
                imlib_context_set_image(im);
                imlib_free_image();
        
                return ss;
#endif
                
                {
                    fh_stringstream ss;
                    ss << "Can't decode images with this build of libferris! image:" << c->getURL();
                    Throw_GPhoto2( tostr(ss), 0 );
                }
            }
        
        virtual
        void
        createStateLessAttributes( bool force = false )
            {
                LG_GPHOTO_D << "createStateLessAttributes() " << endl;
                
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA  tryAddStateLessAttribute         
                    LG_GPHOTO_D << "createStateLessAttributes(2) " << endl;

                    SLEA( "size",   SL_getSizeStream,   FXD_FILESIZE );
                    SLEA( "width",  SL_getWidthStream,  FXD_WIDTH_PIXELS );
                    SLEA( "height", SL_getHeightStream, FXD_HEIGHT_PIXELS );
                    SLEA( "been-downloaded", SL_getHasBeenDownLoadedStream, XSD_BASIC_BOOL );
                    SLEA( "readable",        SL_getReadable,  XSD_BASIC_BOOL );
                    SLEA( "deletable",       SL_getDeletable, XSD_BASIC_BOOL );
                    SLEA( "mtime",           SL_getMTimeRawStream, FXD_UNIXEPOCH_T );

                    SLEA( THUMBNAIL_EANAME_HAS_EXIF_THUMB, SL_getHasThumbStream,    XSD_BASIC_BOOL );
                    SLEA( THUMBNAIL_EANAME_W,              SL_getThumbWidthStream,  FXD_WIDTH_PIXELS );
                    SLEA( THUMBNAIL_EANAME_H,              SL_getThumbHeightStream, FXD_HEIGHT_PIXELS );
                    SLEA( THUMBNAIL_EANAME_RGBA,           SL_getThumbRGBAStream,   FXD_BINARY_RGBA32 );

                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
        
        virtual void priv_createAttributes()
            {
//                 CameraFileInfo info;
                
//                 if( isParentBound()
//                     && getCamera()->getFileInfo( getParent()->getDirPath(),
//                                                  getDirName(),
//                                                  &info ))
//                 {

//                     if (info.preview.fields != GP_FILE_INFO_NONE)
//                     {
//                         if (info.preview.fields & GP_FILE_INFO_TYPE)
//                             printf (_("  Mime type:   '%s'\n"), info.preview.type);
//                         if (info.preview.fields & GP_FILE_INFO_SIZE)
//                             printf (_("  Size:        %li byte(s)\n"), info.preview.size);
//                         if (info.preview.fields & GP_FILE_INFO_WIDTH)
//                             printf (_("  Width:       %i pixel(s)\n"), info.preview.width);
//                         if (info.preview.fields & GP_FILE_INFO_HEIGHT)
//                             printf (_("  Height:      %i pixel(s)\n"), info.preview.height);
//                     }
                    
//                 }
                
                _Base::priv_createAttributes();
            }
        
        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
            }

    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    long
    gphotoContext::priv_read_addFiles( const stringlist_t& fl )
    {
        long addedCount = 0;
        
        for( stringlist_t::const_iterator fi = fl.begin(); fi!=fl.end(); ++fi )
        {
            string dn = *fi;
            LG_GPHOTO_D << "priv_read_addFiles() dn:" << dn << endl;
            priv_readSubContext( dn );
            ++addedCount;
        }
        
        return addedCount;
    }
    
    void
    gphotoContext::priv_read()
    {
        LG_GPHOTO_D << "gphotoContext::priv_read() url:" << getURL() << endl;

//        Factory::getPluginOutOfProcNotificationEngine().watchTree( this );

        EnsureStartStopReadingIsFiredRAII _raii1( this );

        string path = gp_path( true );

        LG_GPHOTO_D << "gphotoContext::priv_read() path:" << path << endl;

        if( !isParentBound() )
        {
            path = "/";
            cameras_t cameras = GPhoto::Detect();

            if( cameras.empty() )
            {
                LG_GPHOTO_W << "No cameras found!" << endl;
                cerr << "No cameras found!" << endl;
            }
            
            for( cameras_t::const_iterator ci = cameras.begin(); ci != cameras.end(); ++ci )
            {
                string dn = (*ci)->getModel();
                LG_GPHOTO_D << "gphotoContext::priv_read(root) camera:" << dn << endl;
                priv_readSubContext( dn );
            }
        }
        else
        {
            int addedCount = 0;
            
            clearContext();

            try
            {
                fh_gphoto camera = getCamera();
                addedCount += priv_read_addFiles( camera->getFolderNames( path ) );
                addedCount += priv_read_addFiles( camera->getFileNames( path ) );
                LG_GPHOTO_D << "path:" << path << " added file count:" << addedCount << endl;
            }
            catch( GPhoto2& e )
            {
                LG_GPHOTO_D << "path:" << path << " added file count:" << addedCount << endl;
                if( !addedCount )
                {
                    LG_GPHOTO_D << "testing lower path:" << tolowerstr()( path ) << endl;
                    if( ends_with( tolowerstr()( path ), ".jpg" ) )
                    {
                        stringstream ss;
                        ss << "Object is an image file. Not readable as a directory path:" << path;
                        LG_GPHOTO_D << tostr(ss) << endl;
                        Throw_FerrisNotReadableAsContext( tostr(ss), this );
                    }
                }
                LG_GPHOTO_D << "err:" << e.what() << endl;
                throw e;
            }
            
            
        }
        
        LG_GPHOTO_D << "gphotoContext::priv_read(done) path:" << getDirPath() << endl;
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
        {
            static gphotoContext c;
            fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
            return ret;
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
