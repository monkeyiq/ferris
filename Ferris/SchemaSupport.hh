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

    $Id: SchemaSupport.hh.in,v 1.6 2010/09/24 21:30:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>

#ifndef _ALREADY_INCLUDED_FERRIS_SCHEMA_SUPPORT_H_
#define _ALREADY_INCLUDED_FERRIS_SCHEMA_SUPPORT_H_

#include <string>


namespace Ferris
{
    /**
     * These are the basic always present types in the schema. They include many of
     * the standard XSD basic types and are augmented with types that are file
     * system specific.
     *
     * Note that each context in the schema also stores its XSDBasic_t value so
     * that one can test an EA to see what type it is easily using getSchemaType()
     */
    enum XSDBasic_t {
        XSD_UNKNOWN      = 290,
        XSD_SCHEMA       = 291,
        XSD_BASIC_BOOL   = 300,
        XSD_BASIC_FLOAT  = 301,
        XSD_BASIC_DOUBLE = 302,
        XSD_BASIC_STRING = 303,
        XSD_BASIC_INT    = 304,
        XSD_BASIC_INTEGER= 346,
        FXD_MODE_T       = 305,      //< mode_t feild out of lstat(2)
        FXD_UNIXEPOCH_T  = 306, // seconds since the unix epoch. a time_t
        FXD_INODE_T      = 307,     // ino_t from lstat(2)
        FXD_MIMETYPE     = 308,    // string which should represent the mimetype of data
        FXD_GID_T        = 309,       // gid_t from lstat(2)
        FXD_UID_T        = 310,       // uid_t from lstat(2)
        FXD_USERNAME     = 311,    // user login name on the box
        FXD_GROUPNAME    = 312,   // group name on the box
        FXD_PID          = 313,         // process id
        FXD_FILESIZE     = 314,    // usually 64 bit.
        FXD_URL          = 315,         // a url that may be of interest to the user
        FXD_URL_IMPLICIT_RESOLVE = 316, // like FXD_URL but
                                              // it is recommended to Resolve()
                                              // the url and show the data at that location instead
                                              // of the url itself.
        FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM = 350, // like 
                                              // FXD_URL but the filesystem that is obtained
                                              // by Resolve()ing the URL string is the interesting
                                              // thing. This differs slightly from
                                              // FXD_URL_IMPLICIT_RESOLVE in that this schema
                                              // type recommends displaying the target as a dir.
        FXD_MACHINE_NAME = 317,// computer name in dns relative to local machine
        FXD_FSID_T       = 318,      // see STATFS(2) fsid_t
        FXD_INT32        = 319,       // 32 bit signed integer
        FXD_UINT32       = 320,      // 32 bit unsigned integer
        FXD_LONG         = 321,        // whatever a c 'long' type size is on this machine.
        FXD_BINARY       = 322,      // raw binary data
        FXD_BINARY_RGBA32    = 323,   // 32 bit per pixel RGBA image data
        FXD_BINARY_NATIVE_EA = 324,// binary data stored as on disk EA through the kernel
        FXD_BINARY_PGMPIPE   = 325,  // a series of pgm images
        FXD_BINARY_A52PIPE   = 326,  // an audio track in a52 format
        FXD_EANAMES          = 327,         // A list of ea names seperated by a single comma between each ea
        FXD_DIGEST           = 328,          // A string which is known to be a digest of a
                                                      // binary object. it is also written that each digest
                                                      // will have a similar length
        FXD_XMLDOCSTRING     = 329,    // A string containing a valid xml document
        FXD_PIXELCOUNT       = 330,      // A width or height or length in screen pixels.
        FXD_WIDTH_PIXELS     = 331,    // A width  in screen pixels.
        FXD_HEIGHT_PIXELS    = 332,   // A height in screen pixels.
        FXD_PRIMARY_KEY      = 333,     // primary key in a database or some grouping that
                                                      // is being used as the primary key would be
        FXD_PRIMARY_KEY_REAL    = 334,    // real primary key from a relational database
        FXD_PRIMARY_KEY_VIRTUAL = 335, // a grouping of columns that is being treated 
                                                             // as a primary key for this context
                                                             // (eg. synthetic key made for a query)
        FXD_FTX_RANK    = 336,           // full text query ranking for a document
        FXD_FFILTER     = 337,            // ferris-filter
        FXD_STR_IP4ADDR = 338,        // ipv4 string address
        FXD_IP4PORT     = 339,            // ipv4 port number

        FXD_DISTINGUISHED_PERSON = 340, // an object that owns or issues a certificate
        FXD_CIPHER_NAME      = 341,      // name of a cipher
        FXD_CIPHER_VERSION   = 342,   // version of FXD_CIPHER_NAME
        FXD_CIPHER_NAME_LIST = 343, // comma seperated list of FXD_CIPHER_NAME objects
        FXD_CIPHER_BITS      = 344,      // number of bits in use
        FXD_STRINGLIST       = 345,       // a list of strings seperated by commas
        FXD_URLLIST          = 351,          // a list of urls seperated by commas
        FXD_UNIXEPOCH_STRING = 347, //< a time_t value flattened to a string

        FXD_INTLIST = 352, // list of XSD_BASIC_INT comma seperated

        FXD_EXIF_VERSION      = 360,     // exif version type
        FXD_EXIF_USER_COMMENT = 361,// 8 byte encoding prefixed string
        FXD_EXIF_COPYRIGHT    = 362,   // specially formatted string
        FXD_EXIF_FLASH        = 363,       // if flash fired and was ment to string
        FXD_EXIF_EXPOSURE_PROGRAM = 364, // custom string from enum
        FXD_UNIT_NAME_LENGTH  = 365, // strings for length: Centimeter etc 
        FXD_EXIF_DATETIME_STRING = 366, // special formated date/time str

        
        // A string which can only take on a handful of values //
        FXD_ENUMERATED_STRING = 367,
        
        // exif data which have an enumerated range of allowed values //
        FXD_EXIF_SENSING_METHOD    = 368, 
        FXD_EXIF_ORIENTATION       = 369,
        FXD_EXIF_METERING_MODE     = 370,
        FXD_EXIF_YCBCR_POSITIONING = 371,
        FXD_EXIF_COMPRESSION       = 372,
        FXD_EXIF_LIGHT_SOURCE      = 373,

        FXD_MODE_STRING_T       = 374,      //< mode_t feild out of lstat(2) as string

        FXD_LATLONG   = 375,
        FXD_LATITUDE  = 376,
        FXD_LONGITUDE = 377,

        FXD_XLIST = 400,
        FXD_UINT64 = 401,
        FXD_INT64 = 402,

        FXDC_READONLY     = 1L<<25, //< read only ea
        FXDC_IS_GENERATED = 1L<<26, //< is the value generated
        FXDC_IS_STATELESS = 1L<<27  //< is the ea attached in a stateless manner (maybe not have this?)

        
    };

    /***
     * Default max width for an attribute
     */
    const std::string SCHEMA_DEFAULT_DISPLAY_WIDTH = "default-display-width";

    const std::string SCHEMA_DEFAULT_DISPLAY_PRECISION = "default-display-precision";

    /**
     * Default justification for an attribute
     * @see struct justification in SignalStreams.hh for an iostream manipulator that
     *      handles this value
     */
    const std::string SCHEMA_JUSTIFICATION = "justification";

    /**
     * Default sorting comparison style. For example, int, string, cis
     */
    const std::string SCHEMA_DEFAULTSORT = "defaultsort";

    /**
     * Get the SCHEMA_DEFAULTSORT value for the schema context sc.
     *
     * @param sc the schema context as returned by getSchema()
     *
     * @return the value of SCHEMA_DEFAULTSORT for sc parsed into individual strings.
     */
    FERRISEXP_API stringlist_t getSchemaDefaultSortList( fh_context sc );
    /**
     * Only gets first
     * Get the SCHEMA_DEFAULTSORT value for the schema context sc.
     *
     * @param sc the schema context as returned by getSchema()
     *
     * @return the value of SCHEMA_DEFAULTSORT for sc parsed into individual strings.
     */
    FERRISEXP_API std::string getSchemaDefaultSort( fh_context sc );
    
    /**
     * Some of the parts of a XSDBasic_t detail if the type is read only etc
     * they are really just augmentation on the type and can be masked off
     * with this function to see if a type is equal to another type which may
     * have different read/write access
     */
    FERRISEXP_API XSDBasic_t maskOffXSDMeta( XSDBasic_t t );

    /**
     * Add a basic schema for the ea with name rawname giving the builtin type t
     * rawname is for example "size" and then the schema is bound to "schema:size"
     *
     * This method attaches stateless schema data. For attributes which are bound
     * using addAttribute() see attachGenericSchema().
     *
     * The types which are built in are based on those presented in Built-in datatypes
     * in "XML Schema Part 2: Datatypes, W3C Recommendation 02 May 2001"
     *
     * @see attachGenericSchema()
     */
    FERRISEXP_API void attachStatelessSchema( Context* c, const std::string& rawname, XSDBasic_t t );

    /**
     * Add a basic schema for the ea with name rawname giving the builtin type t
     * rawname is for example "size" and then the schema is bound to "schema:size"
     *
     * This method attaches statefull schema data. For attributes which are bound
     * based on the Context class itself, (ie. stateless ea) see attachStatelessSchema().
     *
     * The types which are built in are based on those presented in Built-in datatypes
     * in "XML Schema Part 2: Datatypes, W3C Recommendation 02 May 2001"
     *
     * @see attachGenericSchema()
     */
    FERRISEXP_API void attachGenericSchema( Context* c, const std::string& rawname, XSDBasic_t t );


    /**
     * Get the enum type for the attribute eaname or sct if no such type exists.
     * @param c Is the context that has the EA we are interested in
     * @param eaname The EA we are wanting to know the schema for
     * @param sct Default if no schema is bound.
     */
    FERRISEXP_API XSDBasic_t getSchemaType( const fh_context& c, const std::string& eaname, XSDBasic_t sct );

    /**
     * Get the justification for the attribute eaname or def if no justification exists.
     *
     * @param c Is the context that has the EA we are interested in
     * @param eaname The EA we are wanting to know the justification for
     * @param sct Default if no schema is bound.
     */
    FERRISEXP_API std::string getSchemaJustification(
        const fh_context& c, const std::string& eaname, const std::string& def = "left" );

    /**
     * Get the display width for the attribute eaname or def if no display width exists.
     *
     * @param c Is the context that has the EA we are interested in
     * @param eaname The EA we are wanting to know the display width for
     * @param sct Default if no schema is bound.
     */
    FERRISEXP_API int getSchemaDisplayWidth(
        const fh_context& c, const std::string& eaname, int def = 0 );
    FERRISEXP_API int getSchemaDisplayPrecision(
        const fh_context& c, const std::string& eaname, int def = 0 );
    
    
    /**
     * Get the enum type for the attribute eaname or sct if no such type exists.
     * @param sc The Schema for the attribute we are interested in.
     *           This will be sc = c->getSchema( eaname ) for the other version of this call.
     * @param sct Default if no schema is bound.
     */
    FERRISEXP_API XSDBasic_t getSchemaType( const fh_context& sc, XSDBasic_t sct );

    namespace Factory 
    {
        typedef std::map< XSDBasic_t, fh_context > xsdtypemap_t;

        /**
         * Get the schema URL for the given schema type.
         */
        std::string getSchemaURLForType( XSDBasic_t sct );
        
        /**
         * creates a map from each XSDBasic_t to a fh_context representing that type
         * note that the return value is the same as 'm'.
         */
        FERRISEXP_API xsdtypemap_t& makeBasicTypeMap( xsdtypemap_t& m );
    }
    
};
#endif

