/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2007 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libferrisstrigi.cpp,v 1.1 2007/08/15 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <FerrisBoost.hh>

#include <strigi/indexwriter.h>
#include <strigi/streamanalyzer.h>
#include <strigi/analyzerconfiguration.h>
#include <strigi/analysisresult.h>
#include <strigi/bufferedstream.h>
#include <strigi/fieldtypes.h>

using namespace std;

namespace Ferris
{
    
    class FERRISEXP_DLLLOCAL EAGenerator_Strigi : public MatchedEAGeneratorFactory
    {
        bool shouldOverlayStrigiForURL( fh_context a );
        
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_Strigi();
        virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef class EA_Atom_Static StrigiByteArrayAttribute;
    typedef class EA_Atom_Static StrigiByteArrayAttributeSchema;
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



    bool
    EAGenerator_Strigi::shouldOverlayStrigiForURL( fh_context a )
    {
        LG_STRIGI_D << "shouldOverlayStrigiForURL() earl:" << a->getURL() << endl;

        bool ret = false;
        string earl = a->getURL();

        static fh_rex r = 0;

        if( !r )
        {
            string regexstr = getConfigString( FDB_GENERAL,
                                               CFG_STRIGI_POSITIVE_OVERLAY_REGEX_K,
                                               CFG_STRIGI_POSITIVE_OVERLAY_REGEX_DEF );
            if( !regexstr.empty() )
            {
                r = toregexh( regexstr );
            }
        }

        if( !r )
        {
            ret = true;
        }
        else
        {
            if( regex_match( earl, r ) )
            {
                ret = true;
            }
        }
        
        return ret;
    }
    
    

    

EAGenerator_Strigi::EAGenerator_Strigi()
    :
    MatchedEAGeneratorFactory()
{
}


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FerrisStrigiIndexWriter
        :
        public Strigi::IndexWriter
    {
    private:
        
    protected:
        void startAnalysis(const Strigi::AnalysisResult* ar)
            {
            }
        void printValue(const Strigi::RegisteredField* name, std::string& value)
            {
                LG_STRIGI_D << "printValue() val:" << value << endl;
            }
        void finishAnalysis(const Strigi::AnalysisResult* ar)
            {
            }
        void addText(const Strigi::AnalysisResult* ar, const char* text,
                     int32_t length)
            {
                LG_STRIGI_D << "addText() len:" << length << endl;
            }
        void addValue(const Strigi::AnalysisResult* ar,
                      const Strigi::RegisteredField* field, const std::string& value)
            {
                LG_STRIGI_D << "addValue(1) key:" << field->key() << " v:" << value << endl;
                string rdn = field->key();
                if( !theContext->isAttributeBound( rdn, false ) )
                {
                    theContext->addAttribute( rdn, value, XSD_BASIC_STRING, false );
                }
            }
        void addValue(const Strigi::AnalysisResult* ar,
                      const Strigi::RegisteredField* field,
                      const unsigned char* data, uint32_t size)
            {
                LG_STRIGI_D << "addValue(2) sz:" << size << endl;
            }
        void addValue(const Strigi::AnalysisResult* ar,
                      const Strigi::RegisteredField* field, uint32_t value)
            {
                LG_STRIGI_D << "addValue(3) key:" << field->key() << " val:" << value << endl;
                string rdn = field->key();
                if( !theContext->isAttributeBound( rdn, false ) )
                {
                    theContext->addAttribute( rdn, tostr(value), XSD_BASIC_INT, false );
                }
            }
        void addValue(const Strigi::AnalysisResult* ar,
                      const Strigi::RegisteredField* field, int32_t value)
            {
                LG_STRIGI_D << "addValue(4) val:" << value << endl;
                string rdn = field->key();
                if( !theContext->isAttributeBound( rdn, false ) )
                {
                    theContext->addAttribute( rdn, tostr(value), XSD_BASIC_INT, false );
                }
            }
        void addValue(const Strigi::AnalysisResult* ar,
                      const Strigi::RegisteredField* field, double value)
            {
                LG_STRIGI_D << "addValue(5) val:" << value << endl;
                string rdn = field->key();
                if( !theContext->isAttributeBound( rdn, false ) )
                {
                    theContext->addAttribute( rdn, tostr(value), XSD_BASIC_DOUBLE, false );
                }
            }
        void addTriplet(const std::string& subject,
                        const std::string& predicate, const std::string& object) {}
        void addValue(const Strigi::AnalysisResult*,
                      const Strigi::RegisteredField* field, const std::string& name,
                      const std::string& value) {}
        void initWriterData(const Strigi::FieldRegister& f)
            {
//                 map<string, Strigi::RegisteredField*>::const_iterator i;
//                 map<string, Strigi::RegisteredField*>::const_iterator end(f.fields().end());
//                 for (i = f.fields().begin(); i != end; ++i)
//                 {
//                     LG_STRIGI_D << "n:" << i->first << endl;
//                 }
            }
        void releaseWriterData(const Strigi::FieldRegister&)
            {
            }
    public:
        const fh_context& theContext;
    
        explicit FerrisStrigiIndexWriter( const fh_context& a )
            :
            theContext( a )
            {
            }
        ~FerrisStrigiIndexWriter()
            {
            }
        void commit() {}
        void deleteEntries(const std::vector<std::string>& entries) {}
        void deleteAllEntries() {}
};

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    class FerrisStrigiInputStream
        :
        public Strigi::BufferedInputStream
    {
    private:
        fh_istream iss;

        int32_t fillBuffer(char* start, int32_t space)
            {
                if( iss.eof() )
                    return -1;
                if( !iss.good() )
                    return -1;

                iss.read( start, space );
                int32_t ret = iss.gcount();
//                LG_STRIGI_D << "fillBuffer() space:" << space << " gcount:" << ret << endl;
                return ret;
            }
        

    public:

        explicit FerrisStrigiInputStream( fh_istream iss )
            :
            iss( iss )
            {}
        ~FerrisStrigiInputStream()
            {}
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


void
EAGenerator_Strigi::Brew( const fh_context& a )
{
    if( !shouldOverlayStrigiForURL( a ) )
        return;

    LG_STRIGI_D << "EAGenerator_Strigi::Brew() url:" << a->getURL() << endl;

    static bool brewing = false;
    if( brewing )
        return;
    Util::ValueRestorer< bool > dummy1( brewing, true );
    
    vector<pair<bool,string> >filters;
    filters.push_back(make_pair<bool,string>(false,".*/"));
    filters.push_back(make_pair<bool,string>(false,".*"));
    Strigi::AnalyzerConfiguration ac;
    ac.setFilters(filters);

    string earl = a->getURL();
    time_t mtime = time(0);
    Strigi::StreamAnalyzer sa( ac );
    FerrisStrigiIndexWriter writer( a );
    sa.setIndexWriter( writer );
    fh_istream ferris_iss = a->getIStream();
    FerrisStrigiInputStream iss( ferris_iss );
    Strigi::AnalysisResult result( earl, mtime, writer, sa );
    sa.analyze(result, &iss);

    
    
// //    Time::Benchmark("Strigi::Brew() " + a->getURL() );
//     LG_STRIGI_D << "EAGenerator_Strigi::Brew() url:" << a->getURL() << endl;
    
//     static bool brewing = false;
//     if( brewing )
//         return;
//     Util::ValueRestorer< bool > dummy1( brewing, true );
    
//     try
//     {
//         stringlist_t strigi_ea_names;

//         LG_STRIGI_D << "EAGenerator_Strigi::Brew(2) url:" << a->getURL() << endl;
//         LG_STRIGI_D << "is-native:" << a->getIsNativeContext() << endl;
//         LG_STRIGI_D << "has-parent:" << a->isParentBound() << endl;

//         if( a->getIsNativeContext() && a->isParentBound() )
//         {
//             fh_StrigiDirInfo di = getDirInfo( a->getParent() );
//             LG_STRIGI_D << "EAGenerator_Strigi::Brew(3) di:" << toVoid(di) << endl;
//             stringset_t& sl = di->getFileTags( a->getDirName() );

//             LG_STRIGI_D << "EAGenerator_Strigi::Brew(4) sl.sz:" << sl.size() << endl;
//             stringset_t::iterator si = sl.begin();
//             stringset_t::iterator se = sl.end();
//             for( ; si != se; ++si )
//             {
//                 string rdn = *si;
//                 string qrdn = (string)"strigi:" + rdn;

//                 LG_STRIGI_D << "EAGenerator_Strigi::Brew(add) a:" << a->getURL()
//                            << " rdn:" << rdn
//                            << " qrdn:" << qrdn
//                            << endl;
                
//                 a->addAttribute( qrdn, "1", XSD_BASIC_BOOL, false );
//                 strigi_ea_names.push_back( qrdn );
//             }

            
//             {
//                 stringset_t neg;
//                 di->getFileTagsNegative( a->getDirName(), neg );
//                 stringset_t::iterator si = neg.begin();
//                 stringset_t::iterator se = neg.end();
//                 for( ; si != se; ++si )
//                 {
//                     string rdn = *si;
//                     string qrdn = (string)"strigi:" + rdn;

//                     LG_STRIGI_D << "EAGenerator_Strigi::Brew(add neg) a:" << a->getURL()
//                                << " rdn:" << rdn
//                                << " qrdn:" << qrdn
//                                << endl;
                
//                     a->addAttribute( qrdn, "0", XSD_BASIC_BOOL, false );
//                     strigi_ea_names.push_back( qrdn );
//                 }
//             }
            
            
//             a->addAttribute( "strigi-ea-names",
//                              Util::createCommaSeperatedList( strigi_ea_names ),
//                              FXD_EANAMES,
//                              true );
//         }
//     }
//     catch( exception& e )
//     {
//         LG_RDF_ER << "Failed to load RDF EA, error:" << e.what() << endl;
//     }

    LG_STRIGI_D << "Brew() "
                << " url:" << a->getURL()
                << " complete"
                << endl;
}

bool
EAGenerator_Strigi::tryBrew( const fh_context& ctx, const std::string& eaname )
{
    LG_STRIGI_D << "tryBrew() "
                << " url:" << ctx->getURL()
                << endl;

    if( !shouldOverlayStrigiForURL( ctx ) )
        return false;
    
    Brew( ctx );
    return ctx->isAttributeBound( eaname, false );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C"
{
    FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
    {
        LG_STRIGI_D << "EAGenerator_Strigi::CreateRealFactory()" << endl;
        return new EAGenerator_Strigi();
    }
};



 
};
