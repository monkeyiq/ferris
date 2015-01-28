/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris ls
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

    $Id: ferris-filesystem-to-xsltfs-sheets.cpp,v 1.5 2010/09/24 21:31:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <Resolver_private.hh>
#include <Ferrisls.hh>
#include <popt.h>
#include <unistd.h>

#include <sys/stat.h>

#include "config.h"


using namespace std;
using namespace Ferris;


const string PROGRAM_NAME = "ferris-filesystem-to-xsltfs-sheets";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

string commandLine = "";

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    static string toOctalString( int v )
    {
        stringstream ss;
        ss << oct << v;
        return ss.str();
    }

string xfragpath( string s )
{
    return AUTOTOOLS_CONFIG_DATADIR + "/ferris/ferris-filesystem-to-xsltfs-sheets/" + s;
}

class SheetCreator
{
    int m_sheetID;

protected:
    stringlist_t attrs;
    int m_UpdateStartCellOffset;


    int getSheetID()
        {
            return m_sheetID;
        }

    string getFilesystemToXSLTfsRootPath()
        {
            string dirPath = "~/.ferris/filesystem-to-xsltfs-sheets/" + tostr(m_sheetID);
            return dirPath;
        }
    
private:

    void setupAttrs( fh_context c )
        {
            m_UpdateStartCellOffset = 0;
            
            string pk = getStrAttr( c, "primary-key", "" );
            Util::parseCommaSeperatedList( pk, attrs );
            m_UpdateStartCellOffset = attrs.size();
            if( !contains( attrs, "name" ) )
                ++m_UpdateStartCellOffset;
            
            {
                stringlist_t tmp = Util::parseCommaSeperatedList( getStrAttr( c, "recommended-ea", "" ) );
                for( stringlist_t::const_iterator si = tmp.begin(); si != tmp.end(); ++si )
                {
                    ensure_back( attrs, *si );
                }
            }

            erase( attrs, "primary-key" );
            erase( attrs, "name" );
            attrs.push_front( "name" );
        }
    
protected:

    void outputFile( fh_ostream oss, const std::string& s )
        {
            fh_ifstream iss( xfragpath( s ) );
            copy( istreambuf_iterator<char>(iss),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(oss));
        }
    
    virtual void outputForwardPrefix( fh_context c, fh_ostream oss )
        {
        }
    virtual void outputForwardBody( fh_context c, fh_ostream oss ) = 0;
    virtual void outputForwardPostfix( fh_context c, fh_ostream oss )
        {
        }

    virtual void outputReversePrefix( fh_context c, fh_ostream oss )
        {
        }
    virtual void outputReverseBody( fh_context c, fh_ostream oss ) = 0;
    virtual void outputReversePostfix( fh_context c, fh_ostream oss )
        {
        }
    
public:
    SheetCreator()
        :
        m_UpdateStartCellOffset( 0 ),
        m_sheetID( 0 )
        {
        }
    virtual ~SheetCreator()
        {
        }

    void setSheetID( int v )
        {
            m_sheetID = v;
        }

    virtual string getAppendExtension()
        {
            return "";
        }
    virtual string getFUSEForceToFileRegex()
        {
            if( !getAppendExtension().empty() )
            {
                stringstream ss;
                ss << ".*" << getAppendExtension() << "$";
                return ss.str();
            }
            return "";
        }
    

    void makeSheets( fh_context c )
        {
            setupAttrs( c );

            string dirPath = getFilesystemToXSLTfsRootPath();
            fh_context   d = Shell::acquireContext( dirPath );

            {
                fh_ofstream oss( d->getURL() + "/base-url.txt" );
                oss << c->getURL() << endl;
            }
            {
                fh_ofstream oss( d->getURL() + "/recreate.sh" );
                oss << "#!/bin/bash" << endl;
                oss << commandLine << endl;
                int newmode = S_IWUSR | S_IRUSR | S_IXUSR;
                setStrAttr( Resolve(d->getURL() + "/recreate.sh"),
                            "mode",
                            toOctalString(newmode) );
            }
            
            {
                fh_ofstream oss( d->getURL() + "/forward-sheet.xsl" );
                outputForwardPrefix( c, oss );
                outputForwardBody( c, oss );
                outputForwardPostfix( c, oss );
            }

            {
                fh_ofstream oss( d->getURL() + "/reverse-sheet.xsl" );
                outputReversePrefix( c, oss );
                outputReverseBody( c, oss );
                outputReversePostfix( c, oss );
            }

            if( !getAppendExtension().empty() )
            {
                fh_context c = Resolve( getFilesystemToXSLTfsRootPath() );
                fh_ofstream extensionss( c->getURL() + "/append-extension" );
                extensionss << getAppendExtension() << flush;
            }
        }
};

typedef Loki::SingletonHolder<
    Loki::Factory< SheetCreator, std::string > >
SheetCreatorFactory;

template < class Base,class Sub >
struct MakeObject
{
    static Base* Create()
        { return new Sub(); }
};
stringset_t Plugins;
template< class T >
bool Register( const std::string& n )
{
    bool b = SheetCreatorFactory::Instance().Register( n, &MakeObject<SheetCreator,T>::Create );
    Plugins.insert( n );
    return b;
}


class ExcelSheetCreator : public SheetCreator
{
    string getReverseMappingURL()
        {
            fh_context c = Resolve( getFilesystemToXSLTfsRootPath() );
            return c->getURL() + "/reverse-mapping-associative-array.xsl";
        }
protected:

    virtual void outputForwardPrefix( fh_context c, fh_ostream oss )
        {
            outputFile( oss, "ferris-filesystem-to-xsltfs-excel-sheet-prefix.xfragment" );
        }

    virtual void outputForwardBody( fh_context c, fh_ostream oss )
        {
            c->read();
            
            typedef stringmap_t schemaMap_t;
            schemaMap_t schemaMap;
            for( stringlist_t::iterator si = attrs.begin(); si != attrs.end(); ++si )
            {
                string schemaURL = getStrAttr( c, (string)"schema:" + *si,
                                               "schema://xsd/attributes/string" );
                cerr << "c:" << c->getURL() << endl;
                cerr << "si:" << *si << " schema:" << schemaURL << endl;
                
                string t = "String";

                if( schemaURL == "schema://xsd/attributes/decimal/integer/long/int" )
                    t = "Number";
                if( schemaURL == "schema://xsd/attributes/bool" )
                    t = "Boolean";

                schemaMap[ *si ] = t;
            }

            for( int i=0; i < m_UpdateStartCellOffset; ++i )
                oss << "<Column ss:StyleID=\"readonlycell\" />  \n";
            for( int i = attrs.size() - m_UpdateStartCellOffset; i > 0; --i )
                oss << "<Column />  \n";
                
            oss << "<Row ss:Height=\"16.0\">" << endl;
            int idx = 1;
            for( stringlist_t::iterator si = attrs.begin(); si != attrs.end(); ++si, ++idx )
            {
                string vtype = "String";
        
                oss << "<Cell ss:StyleID=\"headercell\">" << endl;
                oss << "  <Data ss:Type=\"" << vtype << "\">"
                    << *si
                    << "</Data>" << endl;
                oss << "</Cell>" << endl;
            }
            oss << "</Row>" << endl;
            
            oss << "  <xsl:apply-templates/>\n"
                << "    </Table> \n"
                << "    <x:WorksheetOptions/> \n"
                << "  </ss:Worksheet> \n"
                << "</Workbook>\n"
                << "</xsl:template>\n\n";
            
            oss << " <xsl:template match=\"//*/*\">" << endl;
            oss << "<Row ss:Height=\"12.1039\">" << endl;
            for( stringlist_t::iterator si = attrs.begin(); si != attrs.end(); ++si )
            {
                string vtype = schemaMap[ *si ];
        
                oss << "<Cell>" << endl;
                oss << "  <Data ss:Type=\"" << vtype << "\">"
                     << "<xsl:value-of select=\"@" + *si + "\"/>"
                     << "</Data>" << endl;
                oss << "</Cell>" << endl;
            }
            oss << "</Row>" << endl;
            oss << " </xsl:template>" << endl;
        }
    
    virtual void outputForwardPostfix( fh_context c, fh_ostream oss )
        {
            outputFile( oss, "ferris-filesystem-to-xsltfs-excel-sheet-postfix.xfragment" );
        }

    virtual void outputReversePrefix( fh_context c, fh_ostream oss )
        {
            outputFile( oss, "ferris-filesystem-to-xsltfs-excel-sheet-rev-prefix.xfragment" );
        }
    virtual void outputReverseBody( fh_context c, fh_ostream oss )
        {
            string reverseMappingDocument = getReverseMappingURL();
            
            oss << "<xsl:variable name=\"update-start-cell-offset\">"
                << m_UpdateStartCellOffset << "</xsl:variable>\n\n";
            
            oss << "  <xsl:template match=\"ss:Row\" > \n";
            oss << "    <xsl:variable name=\"rdn\"><xsl:value-of select=\"./ss:Cell/ss:Data[1]/.\"/></xsl:variable>\n";
            oss << "    <xsl:element name=\"context\" >\n" 
                << "       <xsl:attribute name=\"name\">\n"
                << "          <xsl:value-of select=\"$rdn\"/>\n"
                << "       </xsl:attribute>\n";
            oss << "      <xsl:for-each select=\"./ss:Cell/ss:Data\">\n";
            oss << "        <xsl:if test=\"position() > $update-start-cell-offset\">      \n";
            oss << "          <xsl:call-template name=\"mycell\">\n";
            oss << "            <xsl:with-param name=\"pos\" select=\"position()\" />\n";
            oss << "          </xsl:call-template>\n";
            oss << "        </xsl:if>\n";
            oss << "\n";
            oss << "      </xsl:for-each>\n";
            oss << "\n";
            oss << "    </xsl:element>\n";
            oss << "\n";
            oss << "  </xsl:template>\n";


            oss << "  <xsl:template name=\"mycell\">\n"
                << "    <xsl:param name=\"pos\"/>\n"
                << "    <xsl:variable name=\"eaname\">\n"
                << "      <xsl:value-of select=\"document('"
                << reverseMappingDocument
                << "')/mapping/e[@idx=$pos]/.\"/>\n"
                << "    </xsl:variable>\n"
                << "\n"
                << "    <xsl:attribute name=\"{$eaname}\">\n"
                << "      <xsl:value-of select=\".\"/>\n"
                << "    </xsl:attribute>\n"
                << "  </xsl:template>\n";

            {
                fh_ofstream revmapss( reverseMappingDocument );

                revmapss << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                         << "<mapping>\n";

                int idx = 1;
                for( stringlist_t::iterator si = attrs.begin(); si != attrs.end(); ++si, ++idx )
                {
                    revmapss << "<e idx=\"" << idx << "\">" << *si << "</e>\n";
                }
                
                revmapss << "</mapping>\n";
            }
        }
    virtual string getAppendExtension()
        {
            return ".xml";
        }
//     virtual string getFUSEForceToFileRegex()
//         {
//             return ".*xml$";
//         }
    
    
    virtual void outputReversePostfix( fh_context c, fh_ostream oss )
        {
            outputFile( oss, "ferris-filesystem-to-xsltfs-excel-sheet-rev-postfix.xfragment" );
        }
    
public:
    ExcelSheetCreator()
        :
        SheetCreator()
        {
        }
};



namespace 
{
    bool b = Register<ExcelSheetCreator>( "excel2003" );
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


int main( int argc, const char** argv )
{
    const char* ShowColumnsDefault= "name";
    const char* PluginNameCSTR    = "help";
    unsigned long ImplicitSheetID    = 0;
    unsigned long ShowVersion        = 0;
    const char*   FUSENameCSTR       = 0;

    struct poptOption optionsTable[] = {

//         { "show-columns", 0, POPT_ARG_STRING, &ShowColumnsCSTR, 0,
//           "Same as --show-ea",
//           "size,mime-type,name" },

        { "implicit-sheet-id", 0, POPT_ARG_INT, &ImplicitSheetID, 0,
          "ID number to use for this sheet set", 0 },

        { "plugin", 'p', POPT_ARG_STRING, &PluginNameCSTR, 0,
          "Name of plugin to use", "help" },

        { "fuse", 'F', POPT_ARG_STRING, &FUSENameCSTR, 0,
          "Add a new FUSE script and mountpoint in ~/ferrisfuse/ with given name", "" },
        
        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        
        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS
        
//         { "show-all-attributes", 0, POPT_ARG_NONE,
//           &ShowAllAttributes, 0,
//           "Show every available bit of metadata about each object in context",
//           0 },
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* RootName1 ...");

    if (argc < 1) {
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }

    if( ShowVersion )
    {
        cout << PROGRAM_NAME << " version: $Id: ferris-filesystem-to-xsltfs-sheets.cpp,v 1.5 2010/09/24 21:31:20 ben Exp $\n"
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2001-2006 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    string PluginName = PluginNameCSTR;
    if( PluginName == "help" )
    {
        cerr << "Please select from plugin list using -p option:" << endl;
        for( stringset_t::iterator si = Plugins.begin(); si != Plugins.end(); ++si )
        {
            cerr << *si << endl;
        }
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }
    
    
    if( !ImplicitSheetID )
    {
        string fsToXSLTFSNextIDPath = "~/.ferris/filesystem-to-xsltfs-sheets-next-id";
        fh_context c = Shell::acquireContext( fsToXSLTFSNextIDPath, 0, false );
        int   nextID = 0;

        fh_istream iss = c->getIStream();
        if( !(iss >> nextID) )
            nextID = 1;
        
        ImplicitSheetID = nextID;
        ++nextID;
        setStrAttr( c, "content", tostr(nextID) );
    }


    {
        stringstream ss;
        ss << argv[0] << " ";
        string isid = "--implicit-sheet-id";
        bool isid_was_used = false;
        for( int i = 1; i<argc; ++i )
        {
            if( isid == argv[i] )
            {
                isid_was_used = true;
                break;
            }
        }
        if( !isid_was_used )
            ss << " --implicit-sheet-id " << ImplicitSheetID << " ";
        
        for( int i = 1; i<argc; ++i )
                ss << "'" << argv[i] << "' ";
        commandLine = tostr(ss);
    }
    
    
    try
    {

        ctxlist_t selection;
        stringlist_t srcs;
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            string srcURL = tmpCSTR;
            fh_context c = Resolve( srcURL );
            selection.push_back( c );
        }

        SheetCreator* creator = SheetCreatorFactory::Instance().CreateObject( PluginName );
        creator->setSheetID( ImplicitSheetID );
        
        for( ctxlist_t::const_iterator ci = selection.begin(); ci != selection.end(); ++ci )
        {
            fh_context c = *ci;

            cerr << "Making transformation for c:" << c->getURL() << endl;

            creator->makeSheets( c );

            FerrisURL fu = FerrisURL::fromString( c->getURL() );

            stringstream earlss;
            earlss << "xsltfs?implicit-sheets=" << ImplicitSheetID
                   << "://context/" << fu.getScheme()
                   << c->getDirPath();
            cout << tostr(earlss) << endl;

            if( FUSENameCSTR )
            {
                
                fh_context ofusebase = Shell::acquireContext( "~/ferrisfuse" );
                fh_context ofusemountpoint = Shell::acquireContext( (string)"~/ferrisfuse/" + FUSENameCSTR );

                stringstream ss;
                ss << "#!/bin/bash" << endl;
                ss << "ferrisfs -u "
                   << "'single-file-filesystem:" << tostr(earlss) << creator->getAppendExtension() << "' ";
                if( !creator->getFUSEForceToFileRegex().empty() )
                    ss << " -F '" << creator->getFUSEForceToFileRegex() << "' ";
                ss << " " << FUSENameCSTR << endl;
                {
                    string rdn = (string)"mount-" + FUSENameCSTR + ".sh";
                    fh_context c = Shell::acquireSubContext( ofusebase, rdn, false, 700 );
                    setStrAttr( c, "content", tostr(ss) );
                }
            }
            
            break;
        }
    }
    catch( NoSuchContextClass& e )
    {
        cerr << "e:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        cerr << "cought:" << e.what() << endl;
        exit(1);
    }

    return(0);
}
