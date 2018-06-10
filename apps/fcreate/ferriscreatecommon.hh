/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of FerrisCreate.

    FerrisCreate is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FerrisCreate is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FerrisCreate.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: createcommon.hh,v 1.3 2010/09/24 21:37:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>

#include "config.h"
#ifdef HAVE_GTK2
#include <gtk/gtk.h>
#include <glade/glade.h>
#endif

#include <string>

#include <string.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <XalanTransformer/XalanTransformer.hpp>
#include <XercesParserLiaison/XercesDOMSupport.hpp>
#include <XercesParserLiaison/XercesParserLiaison.hpp>
#include <XalanTransformer/XercesDOMWrapperParsedSource.hpp>
#include <XalanDOM/XalanDOMException.hpp>
#include <PlatformSupport/XSLException.hpp>


namespace Ferris
{
    using namespace XERCES_CPP_NAMESPACE;
    using namespace XALAN_CPP_NAMESPACE;

    const char* getSchemaToGladeXSLTFileNameDefault();
    const char* SchemaToListXSLTFileNameDefault();
    const char* getGladeToDocumentXSLTFileNameDefault();
    

    
    #define DumpFerrisXSDFileName    "/tmp/FerrisXSD.xml"
    #define DumpToGladeXMLFileName   "/tmp/ToGlade.xml"
    #define DumpFromGladeXMLFileName "/tmp/FromGlade.xml"
    #define DumpToFerrisXMLFileName  "/tmp/ToFerris.xml"
    
    class FerrisCreate
    {
        std::string SchemaString;
        fh_stringstream createDocumentStream;
        
#ifdef HAVE_GTK2
        void writeProps( GtkWidget* w );
        void writeWidget( GtkWidget* w, bool beforeChildren );
        friend void gcont_fe ( GtkWidget *widget, gpointer udata);
#endif
        
        std::string SchemaToGladeXSLTFileName;
        std::string GladeToDocumentXSLTFileName;

        const XalanCompiledStylesheet* SchemaToGladeXSLT;
        const XalanCompiledStylesheet* GladeToDocumentXSLT;

        DOMDocument* GladeDoc;

        bool DumpFerrisXSD;
        bool DumpToGladeXML;
        bool DumpFromGladeXML;
        bool DumpToFerrisXML;

    public:

        FerrisCreate();
        ~FerrisCreate();
        
        void setSchema( fh_istream ss );
        fh_istream getGlade2Document();

#ifdef HAVE_GTK2
        fh_istream createDocument( GladeXML *xml, GtkWindow* win );
#endif
        
        void setSchemaToGladeXSLTFileName( const std::string& s );
        void setGladeToDocumentXSLTFileName( const std::string& s );
        
        void setDumpFerrisXSD( bool v );
        void setDumpToGladeXML( bool v );
        void setDumpFromGladeXML( bool v );
        void setDumpToFerrisXML( bool v );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    /*
     * Make sure that this user is setup.
     */
    void ensureDotFilesCreated( const std::string& SchemaToGladeXSLTFileName,
                                const std::string& GladeToDocumentXSLTFileName );
    
};

