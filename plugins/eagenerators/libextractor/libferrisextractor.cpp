/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

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

    $Id: libferrisextractor.cpp,v 1.2 2010/09/24 21:31:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
extern "C" {
#include <extractor.h>
};


using namespace std;

namespace Ferris
{
    /************************************************************/
    /************************************************************/
    /************************************************************/

    
    class FERRISEXP_DLLLOCAL EAGenerator_libExtractor
        :
        public MatchedEAGeneratorFactory
    {
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_libExtractor();

        fh_attribute CreateAttr(
            const fh_context& a,
            const string& rdn,
            fh_context md = 0 )
            throw(
                FerrisCreateAttributeFailed,
                FerrisCreateAttributeNotSupported
                );
        
        virtual bool isDynamic()
            {
                return false;
            }
    
        bool
        supportsCreateForContext( fh_context c )
            {
                return false;
            }
    };

    class FERRISEXP_DLLLOCAL ExtractorListHolder
    {
        EXTRACTOR_ExtractorList* m_el;
        
    public:
        ExtractorListHolder()
            :
            m_el( 0 )
            {
                m_el = EXTRACTOR_loadDefaultLibraries();
            }
        ~ExtractorListHolder()
            {
                if( m_el )
                    EXTRACTOR_removeAll( m_el );
            }
        
        EXTRACTOR_ExtractorList* l()
            {
                return m_el;
            }
    };
    
    
    static
    EXTRACTOR_ExtractorList*
    getExtractorList()
    {
        static ExtractorListHolder h;
        return h.l();
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    EAGenerator_libExtractor::EAGenerator_libExtractor()
        :
        MatchedEAGeneratorFactory()
    {
    }

    void
    EAGenerator_libExtractor::Brew( const fh_context& a )
    {
        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );

        LG_LIBEXTRACTOR_D << "EAGenerator_libExtractor::Brew() a:'" << a->getDirPath() << "'" << endl;
        
        try
        {
            string                 earl = a->getDirPath();
            EXTRACTOR_ExtractorList* el = getExtractorList();
            EXTRACTOR_KeywordList*   kw = EXTRACTOR_getKeywords( el, earl.c_str() );

            LG_LIBEXTRACTOR_D << "EAGenerator_libExtractor::Brew() el:" << toVoid(el)
                              << " kw:" << toVoid(kw) << endl;

            typedef map< string, XSDBasic_t > EASchema_t;
            stringmap_t EA;
            EASchema_t EASchema;
            
            while (kw != NULL)
            {
                if (kw->keywordType <= EXTRACTOR_getHighestKeywordTypeNumber() )
                {
                    string eaname  = EXTRACTOR_getKeywordTypeAsString( kw->keywordType );
                    string eavalue = kw->keyword;

                    LG_LIBEXTRACTOR_D << "Another keyword for earl:" << earl
                                      << " eaname:" << eaname
                                      << " eavalue:" << eavalue
                                      << endl;

                    stringmap_t::iterator mi = EA.find( eaname );
                    if( mi == EA.end() )
                    {
                        EA[ eaname ] = eavalue;
                        EASchema[ eaname ] = XSD_BASIC_STRING;
                    }
                    else
                    {
                        string t = mi->second + "," + eavalue;
                        EA[ eaname ] = t;
                        EASchema[ eaname ] = FXD_STRINGLIST;
                    }
                }
                
                kw = kw->next;
            }

            for( stringmap_t::const_iterator mi = EA.begin(); mi!=EA.end(); ++mi )
            {
                string eaname  = mi->first;
                string eavalue = mi->second;
                bool rc = a->addAttribute( eaname, eavalue,
                                           EASchema[ eaname ] );
            }
            
            EXTRACTOR_freeKeywords( kw );
        }
        catch( exception& e )
        {
            LG_LIBEXTRACTOR_D << "Failed to load EAs, error:" << e.what() << endl;
        }
    }
    
    fh_attribute
    EAGenerator_libExtractor::CreateAttr( const fh_context& a,
                                 const string& rdn,
                                 fh_context md )
        throw(
            FerrisCreateAttributeFailed,
            FerrisCreateAttributeNotSupported
            )
    {
        fh_stringstream ss;
        ss << "Creating EA is not supported for libextractor ferris plugin."
           << " context:" << a->getURL()
           << " rdn:" << rdn
           << endl;
        Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
    }

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            return new EAGenerator_libExtractor();
        }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

};
