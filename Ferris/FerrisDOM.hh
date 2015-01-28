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

    $Id: FerrisDOM.hh,v 1.19 2011/11/09 21:31:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_DOM_H_
#define _ALREADY_INCLUDED_FERRIS_DOM_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>

#include <xercesc/sax/InputSource.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <iostream>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisException.hh>

//#include <Ferris/Enamel.hh>

namespace Ferris
{
    using namespace XERCES_CPP_NAMESPACE;
    
    /**
     * Client apps should use one of these to store their DOMDocument
     * pointers in because the document will be reclaimed for them when
     * the last reference goes out of scope. Implicit conversion is allowed
     * so that native xerces-c API calls can be done on the smartptr.
     */
    template <class T>
    class FerrisXercesDOMSmartPtrStorage
    {
    public:
        typedef T* StoredType;    // the type of the pointee_ object
        typedef T* InitPointerType; // 
        typedef T* PointerType;   // type returned by operator->
        typedef T& ReferenceType; // type returned by operator*
        
        FerrisXercesDOMSmartPtrStorage() : pointee_(Default()) 
            {}

        // The storage policy doesn't initialize the stored pointer 
        //     which will be initialized by the OwnershipPolicy's Clone fn
        FerrisXercesDOMSmartPtrStorage(const FerrisXercesDOMSmartPtrStorage&)
            {}

        template <class U>
        FerrisXercesDOMSmartPtrStorage(const FerrisXercesDOMSmartPtrStorage<U>&) 
            {}
        
        FerrisXercesDOMSmartPtrStorage(const StoredType& p)
            : pointee_(p)
            {
            }
        
        PointerType operator->() const { return pointee_; }
        
        ReferenceType operator*() const { return *pointee_; }
        
        void Swap(FerrisXercesDOMSmartPtrStorage& rhs)
            { std::swap(pointee_, rhs.pointee_); }
    
        // Accessors
        friend inline PointerType GetImpl(const FerrisXercesDOMSmartPtrStorage& sp)
            { return sp.pointee_; }

        friend inline PointerType GetImplX(const FerrisXercesDOMSmartPtrStorage& sp)
            { return sp.pointee_; }
        
        friend inline const StoredType& GetImplRef(const FerrisXercesDOMSmartPtrStorage& sp)
            { return sp.pointee_; }

        friend inline StoredType& GetImplRef(FerrisXercesDOMSmartPtrStorage& sp)
            { return sp.pointee_; }

    protected:
        // Destroys the data stored
        // (Destruction might be taken over by the OwnershipPolicy)
        void Destroy()
            {
                if( pointee_ )
                {
//                    LG_DOM_D << "Releasing a DOM:" << (void*)pointee_ << std::endl;
                    pointee_->release();
                }
            }
        
        // Default value to initialize the pointer
        static StoredType Default()
            { return 0; }
    
    private:
        // Data
        StoredType pointee_;
    };
    typedef Loki::SmartPtr< DOMDocument,
                            Loki::RefLinked, 
                            Loki::DisallowConversion,
                            FerrisLoki::FerrisExSmartPointerChecker,
                            FerrisXercesDOMSmartPtrStorage > fh_domdoc;
    typedef Loki::SmartPtr< XMLCh,
                            Loki::RefLinked, 
                            Loki::AllowConversion,
                            FerrisLoki::FerrisExSmartPointerChecker,
                            FerrisLoki::FerrisExArraySmartPtrStorage > fh_xmlch;

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /*** From here to the next star comment block is from DOMTreeErrorReporter.hpp **/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    // ---------------------------------------------------------------------------
    //  This is a simple class that lets us do easy (though not terribly efficient)
    //  trancoding of XMLCh data to local code page for display.
    // ---------------------------------------------------------------------------
    class FERRISEXP_DLLLOCAL StrX
    {
    public :
        // -----------------------------------------------------------------------
        //  Constructors and Destructor
        // -----------------------------------------------------------------------
        StrX(const XMLCh* const toTranscode)
            {
                // Call the private transcoding method
                fLocalForm = XMLString::transcode(toTranscode);
            }

        ~StrX()
            {
                XMLString::release(&fLocalForm);
            }

        StrX( const StrX& ci )
            {
                fLocalForm = XMLString::replicate( ci.fLocalForm );
            }
        
        StrX& operator=( const StrX& ci )
            {
                fLocalForm = XMLString::replicate( ci.fLocalForm );
                return *this;
            }
        
        // -----------------------------------------------------------------------
        //  Getter methods
        // -----------------------------------------------------------------------
        const char* localForm() const
            {
                return fLocalForm;
            }

    private :
        // -----------------------------------------------------------------------
        //  Private data members
        //
        //  fLocalForm
        //      This is the local code page form of the string.
        // -----------------------------------------------------------------------
        char*   fLocalForm;
    };

    inline std::ostream& operator<<(std::ostream& target, const StrX& toDump)
    {
        target << toDump.localForm();
        return target;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /*** From here to the next star comment block is from CreateDOMDocument.cpp    **/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    // ---------------------------------------------------------------------------
    //  This is a simple class that lets us do easy (though not terribly efficient)
    //  trancoding of char* data to XMLCh data.
    // ---------------------------------------------------------------------------
    class FERRISEXP_DLLLOCAL XStr
    {
    public :
        // -----------------------------------------------------------------------
        //  Constructors and Destructor
        // -----------------------------------------------------------------------
        XStr(const char* const toTranscode)
            {
                // Call the private transcoding method
                fUnicodeForm = XMLString::transcode(toTranscode);
            }
 
        ~XStr()
            {
                XMLString::release(&fUnicodeForm); 
            }

        XStr( const XStr& ci )
            {
                fUnicodeForm = XMLString::replicate( ci.fUnicodeForm );
            }
        
        XStr& operator=( const XStr& ci )
            {
                fUnicodeForm = XMLString::replicate( ci.fUnicodeForm );
                return *this;
            }
        
        // -----------------------------------------------------------------------
        //  Getter methods
        // -----------------------------------------------------------------------
        const XMLCh* unicodeForm() const
            {
                return fUnicodeForm;
            }
    private :
        // -----------------------------------------------------------------------
        //  Private data members
        //
        //  fUnicodeForm
        //      This is the Unicode XMLCh format of the string.
        // -----------------------------------------------------------------------
        XMLCh*   fUnicodeForm;
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /*** And now we start with ferris code ******************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_EXCEPTION XMLParse : public FerrisVFSExceptionBase
    {
        int errorCount;
        fh_domdoc doc;
    
    public:

        inline XMLParse(
            const FerrisException_CodeState& state,
            fh_ostream log,
            const std::string& e,
            int errorCount,
            fh_domdoc doc,
            Attribute* a=0)
            :
            FerrisVFSExceptionBase( state, log, e.c_str(), a ),
            errorCount( errorCount ),
            doc( doc )
            {
                setExceptionName("XMLParse");
            }

        virtual ~XMLParse() throw() 
            {}
    
        inline fh_domdoc getDoc()
            { return doc; }

        inline int getErrorCount()
            { return errorCount; }
    
    
    };
#define Throw_XMLParse(e,ecount,doc) \
throw XMLParse( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (ecount), (doc))
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_API EAUpdateErrorHandler
        :
        public Handlable
    {
    public:
        virtual void handleError( fh_context,
                                  const std::string& eaname,
                                  const std::string& what ) = 0;
    };
    FERRIS_SMARTPTR( EAUpdateErrorHandler, fh_EAUpdateErrorHandler );

    /**
     * Dont do anything during errors
     */
    class FERRISEXP_API EAUpdateErrorHandler_Null
        :
        public EAUpdateErrorHandler
    {
    public:
        virtual void handleError( fh_context,
                                  const std::string& eaname,
                                  const std::string& what )
            {
            }
    };

    /**
     * print error info to cerr and do nothing
     */
    class FERRISEXP_API EAUpdateErrorHandler_cerr
        :
        public EAUpdateErrorHandler
    {
    public:
        virtual void handleError( fh_context c,
                                  const std::string& eaname,
                                  const std::string& what );
    };
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * if (trim) then remove the initial <?xml...?> declaration
     */
    fh_stringstream& trimXMLDeclaration( fh_stringstream& ret, bool trim = true );
    
    FERRISEXP_API fh_stringstream tostream( fh_domdoc doc, bool gFormatPrettyPrint = true );
    FERRISEXP_API fh_stringstream tostream( DOMNode& n, bool gFormatPrettyPrint = true );
    
    FERRISEXP_API std::string tostr( const XMLCh* xc );
    FERRISEXP_API std::string XMLToString( const XMLCh* xc, const std::string def = "" );

    FERRISEXP_API void setAttribute( DOMElement* e, const std::string& k, const std::string& v );
    FERRISEXP_API void ensureAttribute( DOMDocument* dom,
                                        DOMElement* e, const std::string& k, const std::string& v );

    FERRISEXP_API std::string getAttribute(       DOMElement* e, const std::string& k );
    FERRISEXP_API std::string getAttribute( const DOMElement* e, const std::string& k );

    FERRISEXP_API void setAttributeNS( DOMElement* e,
                         const std::string& ns,
                         const std::string& k,
                         const std::string& v );
    FERRISEXP_API std::string getAttributeNS( const DOMElement* e,
                                const std::string& ns,
                                const std::string& k );

    FERRISEXP_API std::string getStrSubCtx( const DOMNode* c,
                                            std::string subname,
                                            std::string def = "",
                                            bool getAllLines = false,
                                            bool throw_for_errors = false );
    
    
    namespace XML
    {
        typedef std::list< const DOMNode* >    DOMNodeList_t;
        typedef std::list< const DOMElement* > DOMElementList_t;
        FERRISEXP_API std::list< const DOMNode* > evalXPath( fh_domdoc doc, std::string expression );
        FERRISEXP_API std::list< const DOMElement* > evalXPathToElements( fh_domdoc doc, std::string expression );

        FERRISEXP_API std::string escapeToXMLAttr( const std::string& s );

        FERRISEXP_API void writeMessage( fh_ostream oss, const stringmap_t& m );
        FERRISEXP_API stringmap_t& readMessage( fh_stringstream& iss, stringmap_t& ret );
        FERRISEXP_API stringmap_t& readMessage( fh_istream iss, stringmap_t& ret );

        FERRISEXP_API void writeList( fh_ostream, const stringlist_t& l );
        FERRISEXP_API stringlist_t& readList( fh_istream iss, stringlist_t& ret );
        
        FERRISEXP_API std::string contextToXML( fh_context c,
                                                bool recurse = false,
                                                bool includeContent = true );
        FERRISEXP_API std::string contextToXML( fh_context c,
                                                stringlist_t eatoinclude,
                                                int recurse = 0,
                                                bool includeContent = true );
        FERRISEXP_API std::string stringmapToXML( const stringmap_t& sm );

        FERRISEXP_API void        updateFromXML( fh_context c,
                                   const std::string& xml_data,
                                   bool recurse = false,
                                   fh_EAUpdateErrorHandler eh = 0 );

        typedef std::list< DOMNode* > domnode_list_t;
        FERRISEXP_API domnode_list_t& getChildren( domnode_list_t& nl, DOMElement* element );
        FERRISEXP_API domnode_list_t  getChildren( DOMElement* element );
        FERRISEXP_API bool allWhitespaceNodes( DOMElement* element );
        
        /**
         * Get the contents of the first TEXT node under the given element
         */
        FERRISEXP_API std::string getChildText( DOMElement* n );

        /**
         * Set the contents of the first TEXT node under the given element
         */
        FERRISEXP_API const std::string& setChildText( fh_domdoc doc, DOMElement* n, const std::string& );
        FERRISEXP_API const std::string& setChildText( DOMDocument* doc, DOMElement* n, const std::string& s );

        /**
         * Clear away all TEXT type child nodes of a given XML element
         */
        FERRISEXP_API void removeAllTextChildren( DOMElement* element );
        
        /**
         * Add a new element to the document with the given parent element
         * @return the new element
         */
        FERRISEXP_API DOMElement* createElement( fh_domdoc doc,
                                                 DOMElement* parent,
                                                 const std::string& childname );

        /**
         * Get the child element with the given name or null
         */
        FERRISEXP_API DOMElement* getChildElement( const DOMNode* node, const std::string& name );
        FERRISEXP_API std::string getChildElementText( DOMNode* node, const std::string& name );

        /**
         * Get all the children elements with the given name
         */
        FERRISEXP_API std::list< DOMElement* >
        getAllChildrenElements( DOMNode* node,
                                const std::string& name,
                                bool recurse = true );
        FERRISEXP_API std::list< DOMElement* >&
        getAllChildrenElements( DOMNode* node,
                                const std::string& name,
                                std::list< DOMElement* >& ret,
                                bool recurse = true );

        /**
         * Like getAllChildrenElements() but only get the first child element
         */
        FERRISEXP_API DOMElement*
        firstChild( DOMNode* node, const std::string& name );
        
        
    };
    
    namespace Factory
    {
        FERRISEXP_API void ensureXMLPlatformInitialized();
        FERRISEXP_API fh_domdoc    makeDOM( fh_context c,
                                            bool hideXMLAttribute = true,
                                            bool hideEmblemAttributes = true,
                                            bool hideSchemaAttributes = true );
        FERRISEXP_API fh_context   mountDOM( fh_domdoc dom );
        FERRISEXP_API fh_context   mountDOM( fh_domdoc dom, const std::string& forcedURLPrefix );
        FERRISEXP_API fh_domdoc    StreamToDOM( fh_istream iss );
        FERRISEXP_API fh_domdoc    StreamToDOM( fh_stringstream& iss );
        FERRISEXP_API fh_domdoc    StringToDOM( const std::string& s );

        FERRISEXP_API DOMImplementation* getDefaultDOMImpl();

        /**
         * Create a new empty document
         */
        FERRISEXP_API fh_domdoc    makeDOM( const std::string& rootname );
        FERRISEXP_API InputSource* makeInputSource( const fh_istream& iss,
                                                   const char* const bufId,
                                                   MemoryManager* const
                                                   manager = XMLPlatformUtils::fgMemoryManager );
        FERRISEXP_API InputSource* makeInputSource( const fh_context& c );

        FERRISEXP_API XMLCh* makeXMLBase64encoded( const std::string& s );
    };
};
#endif
