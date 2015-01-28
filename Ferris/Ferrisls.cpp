/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferrisls client helper code.

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

    $Id: Ferrisls.cpp,v 1.22 2010/09/24 21:30:49 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <Ferrisls.hh>
#include <FilteredContext.hh>
#include <Context.hh>
#include <iomanip>
#include <ContextSetCompare_private.hh>
#include "Trimming.hh"
#include "ValueRestorer.hh"
#include <FerrisBoost.hh>

using namespace std;


namespace Ferris
{


    StringQuote::StringQuote( QuoteStyle v )
        :
        style(LITERAL_QUOTING),
        backslash_escapes(false),
        quote_string("")
    {
        setStyle( v );
    }



    void
    StringQuote::setStyle( QuoteStyle v )
    {
        style = v;
    
        acceptable_chars.clear();
        backslash_escapes = false;
        quote_string = "";

        /***************************************/
        acceptable_chars.insert('~');

        changelist_shell.push_back( make_pair( "??!", "?\?!" ));
        changelist_shell.push_back( make_pair( "??'", "?\?'" ));
        changelist_shell.push_back( make_pair( "??(", "?\?(" ));
        changelist_shell.push_back( make_pair( "??)", "?\?)" ));
        changelist_shell.push_back( make_pair( "??-", "?\?-" ));
        changelist_shell.push_back( make_pair( "??/", "?\?/" ));
        changelist_shell.push_back( make_pair( "??<", "?\?<" ));
        changelist_shell.push_back( make_pair( "??=", "?\?=" ));
        changelist_shell.push_back( make_pair( "??>", "?\?>" ));

        changelist_dollar_shell.push_back( make_pair( "#", "#" ));
        changelist_dollar_shell.push_back( make_pair( "~", "~" ));
                
        changelist_shell.push_back( make_pair( " ", " " ));
        changelist_shell.push_back( make_pair( "!", "!" ));
        changelist_shell.push_back( make_pair( "\"", "\"" ));
        changelist_shell.push_back( make_pair( "$", "$" ));
        changelist_shell.push_back( make_pair( "&", "&" ));
        changelist_shell.push_back( make_pair( "(", "(" ));
        changelist_shell.push_back( make_pair( ")", ")" ));
        changelist_shell.push_back( make_pair( "*", "*" ));
        changelist_shell.push_back( make_pair( ";", ";" ));
        changelist_shell.push_back( make_pair( "<", "<" ));
        changelist_shell.push_back( make_pair( ">", ">" ));
        changelist_shell.push_back( make_pair( "[", "[" ));
        changelist_shell.push_back( make_pair( "^", "^" ));
        changelist_shell.push_back( make_pair( "`", "`" ));
        changelist_shell.push_back( make_pair( "|", "|" ));
        changelist_shell.push_back( make_pair( "`", "`" ));

        changelist_dollar_shell.push_back( make_pair( "'", "'\\''" ));
        /***************************************/
        /***************************************/

            
        switch( style )
        {
        case LITERAL_QUOTING:
            break;
        case C_QUOTING:
            backslash_escapes = true;
            quote_string = "\"";
            break;
                    
        case SHELL_ALWAYS_QUOTING:
            initShellQuoteString();
            /* Fall */
        case SHELL_QUOTING:

            changelist_shell.push_back( make_pair( "\'", "\'\\\'\'" ));
            changelist_shell.push_back( make_pair( "\\/", "/" ));
            
            break;
                
        case ESCAPE_QUOTING:      
            backslash_escapes = true;
            break;
                
//             case LOCALE_QUOTING:      
//                break;
//             case CLOCALE_QUOTING:     
//                break;
        }

        if( style == C_QUOTING || style == ESCAPE_QUOTING )
        {
            changelist.push_back( make_pair( "\a", "\\a" ));
            changelist.push_back( make_pair( "\b", "\\b" ));
            changelist.push_back( make_pair( "\f", "\\f" ));
            changelist.push_back( make_pair( "\v", "\\v" ));

            changelist_shell.push_back( make_pair( "\n", "\\n" ));
            changelist_shell.push_back( make_pair( "\r", "\\r" ));
            changelist_shell.push_back( make_pair( "\t", "\\t" ));
            changelist_shell.push_back( make_pair( "\\", "\\\\" ));
        }
        

        acceptable_chars.insert('%');
        acceptable_chars.insert('+');
        acceptable_chars.insert(',');
        acceptable_chars.insert('-');
        acceptable_chars.insert('.');
        acceptable_chars.insert('/');
        acceptable_chars.insert('0');
        acceptable_chars.insert('1');
        acceptable_chars.insert('2');
        acceptable_chars.insert('3');
        acceptable_chars.insert('4');
        acceptable_chars.insert('5');
        acceptable_chars.insert('6');
        acceptable_chars.insert('7');
        acceptable_chars.insert('8');
        acceptable_chars.insert('9');
        acceptable_chars.insert(':');
        acceptable_chars.insert('=');
        acceptable_chars.insert('A');
        acceptable_chars.insert('B');
        acceptable_chars.insert('C');
        acceptable_chars.insert('D');
        acceptable_chars.insert('E');
        acceptable_chars.insert('F');
        acceptable_chars.insert('G');
        acceptable_chars.insert('H');
        acceptable_chars.insert('I');
        acceptable_chars.insert('J');
        acceptable_chars.insert('K');
        acceptable_chars.insert('L');
        acceptable_chars.insert('M');
        acceptable_chars.insert('N');
        acceptable_chars.insert('O');
        acceptable_chars.insert('P');
        acceptable_chars.insert('Q');
        acceptable_chars.insert('R');
        acceptable_chars.insert('S');
        acceptable_chars.insert('T');
        acceptable_chars.insert('U');
        acceptable_chars.insert('V');
        acceptable_chars.insert('W');
        acceptable_chars.insert('X');
        acceptable_chars.insert('Y');
        acceptable_chars.insert('Z');
        acceptable_chars.insert(']');
        acceptable_chars.insert('_');
        acceptable_chars.insert('a');
        acceptable_chars.insert('b');
        acceptable_chars.insert('c');
        acceptable_chars.insert('d');
        acceptable_chars.insert('e');
        acceptable_chars.insert('f');
        acceptable_chars.insert('g');
        acceptable_chars.insert('h');
        acceptable_chars.insert('i');
        acceptable_chars.insert('j');
        acceptable_chars.insert('k');
        acceptable_chars.insert('l');
        acceptable_chars.insert('m');
        acceptable_chars.insert('n');
        acceptable_chars.insert('o');
        acceptable_chars.insert('p');
        acceptable_chars.insert('q');
        acceptable_chars.insert('r');
        acceptable_chars.insert('s');
        acceptable_chars.insert('t');
        acceptable_chars.insert('u');
        acceptable_chars.insert('v');
        acceptable_chars.insert('w');
        acceptable_chars.insert('x');
        acceptable_chars.insert('y');
        acceptable_chars.insert('z');
        acceptable_chars.insert('{');
        acceptable_chars.insert('}');
    }


    string
    StringQuote::quote( const string& s, const string& q )
    {
        if( !q.length() )
        {
            return s;
        }
        string ret = q;
        ret += s;
        ret += q;
        return ret;
    }

    void
    StringQuote::ShellMustBeQuoting()
    {
        if( style == SHELL_QUOTING )
            initShellQuoteString();
    }

    void
    StringQuote::initShellQuoteString()
    {
        backslash_escapes = true;
        quote_string = "'";
    }

    bool
    StringQuote::performSubSt( const changelist_t& cl, const string& s, int& i, string& ret )
    {
        bool performedSubSt = false;
        
        /*
         * Do straight string -> string substitutions.
         */
        for( changelist_t::const_iterator iter = cl.begin(); iter != cl.end(); ++iter )
        {
            const string& fi = iter->first;
            const string& se = iter->second;
            
            if( s.substr(i, fi.length()) == fi )
            {
                i   += fi.length();
                ret += se;
                performedSubSt = true;
                break;
            }
        }
        return performedSubSt;
    }
    
    string
    StringQuote::operator()( const string& s )
    {
        if( style == NO_QUOTING )
        {
            return s;
        }
//             cerr << endl << "StringQuoter:" << s << endl;
//             cerr << "quote_string:" << quote_string << endl;
//             cerr << "backslash_escapes:" << backslash_escapes << endl;
//             cerr << "style:" << style << endl;
        string ret="";

        if( style == SHELL_QUOTING )
        {
            backslash_escapes = false;
            quote_string = "";
        }
            
        for( int i=0; i < s.length(); )
        {
            string::iterator iter;

            if( false )
            {
            }
            /*
             * Must be sure to test for start of line chars first for ~ shell
             */
            else if(!i && performSubSt( changelist_dollar, s, i, ret ))
            {
            }
            else if(!i && performSubSt( changelist_dollar_shell, s, i, ret ))
            {
                ShellMustBeQuoting();
            }
            /*
             * Take care of characters that are always ok first. (speed)
             */
            else if( acceptable_chars.find(s[i]) != acceptable_chars.end() )
            {
                ret += s[i];
                ++i;
                continue;
            }
            else if ( backslash_escapes && quote_string.length()
                      && s.substr( i, quote_string.length() ) == quote_string
                )
            {
                ret += "\\";
                ret += quote_string;
                i   += quote_string.length();
                continue;
            }
            else if(performSubSt( changelist, s, i, ret ))
            {
            }
            else if(performSubSt( changelist_shell, s, i, ret ))
            {
                ShellMustBeQuoting();
            }
            else 
            {
                /* There is a nasty character here */
//                    cerr << "nasty char!\n";
                ShellMustBeQuoting();

                if( style == SHELL_QUOTING || style == SHELL_ALWAYS_QUOTING )
                {
                    char ch = s[i];

                    /*
                     * Most stuff is OK in shell mode, with the exception of the single
                     * quote nested inside a single quoted string! For this case we
                     * must close the single quote, have an escaped single quote and then
                     * reenter single quote mode.
                     */
//                     if( ch == '\'' )
//                     {
//                         ret += "\'\\\'\'";
//                     }
                    if( ch == '\0' || ( ch >= '\a' && ch <= '\r' ))
                    {
                        ret += '?';
                    }
                    else
                    {
                        ret += ch;
                    }
                }
                else if( style == C_QUOTING || style == ESCAPE_QUOTING )
                {
                    int radix = 8;
                    stringstream ss;
                    ss << "\\" << setw(2) << setbase(radix) << setfill('0') << (int)s[i];
                    ret += tostr(ss);
                }
                else
                {
                    ret += "?";
                }
                ++i;
            }

        } // for string traverse.
            
        return(quote(ret, quote_string));
    }

    string
    StringQuote::getStringOptionName()
    {
        switch( style )
        {
        case NO_QUOTING: return "none";
        case LITERAL_QUOTING: return "literal";
        case C_QUOTING: return "c";
        case SHELL_QUOTING: return "shell";
        case SHELL_ALWAYS_QUOTING: return "shell-always";
        case ESCAPE_QUOTING: return "escape";
        }
        return "";
    }


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

    string
    Ferrisls_display::ClassifyIfNeeded( const fh_context& ctx,
                                        const string& attrName,
                                        string s )
    {
        if( ClassifyFileNameStyle != CLASSIFY_NONE && attrName == "name" )
        {
            string fft = getStrAttr( ctx, "filesystem-filetype", "" );

            if( fft == "directory"
                && ClassifyFileNameStyle & CLASSIFY_DIR )
            {
                s+="/";
            }
            else if( fft == "symbolic link"
                     && ClassifyFileNameStyle & CLASSIFY_SYMLINK )
            {
                s+="@";
            }
            else if( fft == "socket"
                     && ClassifyFileNameStyle & CLASSIFY_SOCKET )
            {
                s+="=";
            }
            else if( fft == "fifo"
                     && ClassifyFileNameStyle & CLASSIFY_FIFO )
            {
                s+="|";
            }
            else if( fft == "fifo"
                     && ClassifyFileNameStyle & CLASSIFY_FIFO )
            {
                s+="|";
            }
            else if( ClassifyFileNameStyle & CLASSIFY_EXE )
            {
                if( getStrAttr( ctx, "runable", "0" ) == "1" )
                {
                    s+= "*";
                }
                            
            }
        }
        return s;
    }

    string
    Ferrisls_display::QuoteIfNeeded( const fh_context& ctx,
                                     const string& attrName,
                                     const string& s )
    {
        if( attrName == "name" )
        {
            return Quoter(s);
        }
        return s;
    }
    
    string
    Ferrisls_display::DecorateIfNeeded( const fh_context& ctx,
                                        const string& attrName,
                                        string s )
    {
        s = ClassifyIfNeeded( ctx, attrName, s );
        s = QuoteIfNeeded   ( ctx, attrName, s );

        return s;
    }
    
    
    Ferrisls_display::Ferrisls_display()
        :
        FieldSeperator(","),
        RecordSeperator("\n"),
        EnteredMonitoringLoop(false),
        MonitorCreate(false),
        MonitorDelete(false),
        MonitorChanged(false),
        ShowHeadings(true),
        ShowContextNameInHeading(false),
        NoSuchAttributeIndicator(" "),
        FillChar(' '),
        ClassifyFileNameStyle( CLASSIFY_NONE ),
        Quoter( StringQuote::NO_QUOTING ),
        ShowEAErorrs( false )
    {
    }

    Ferrisls_display::~Ferrisls_display()
    {
        DetachAllSignals( true );
    }

    void
    Ferrisls_display::DetachAllSignals( bool force )
    {
        typedef std::map< fh_context, SignalCollection > SigCol_t;

        for( SigCol_t::iterator iter = SigCol.begin(); iter != SigCol.end(); ++iter )
        {
            fh_context c         = iter->first;
            SignalCollection& sc = iter->second;

            sc.ExistsConnection.disconnect();
            if( force && MonitorCreate )
            {
                sc.CreatedConnection.disconnect();
            }
            if( force && MonitorChanged )
            {
//                cerr << "disconnecting changed handler. url:" << c->getURL() << endl;
                sc.ChangedConnection.disconnect();
            }
            if( force && MonitorDelete )
            {
                sc.DeletedConnection.disconnect();
            }
        }
        SigCol.clear();
    }
    
        
    
    
    void
    Ferrisls_display::workComplete()
    {
        if( Monitoring() )
        {
            setEnteredMonitoringLoop(true);
            Main::mainLoop();
        }
    }

    void
    Ferrisls_display::workStarting()
    {
    }
    
    void
    Ferrisls_display::PrintEA( fh_context ctx,
                               int i,
                               const string& attr,
                               const string& EA )
    {
        cout << setfill(FillChar) << EA << FieldSeperator;
    }


    void
    Ferrisls_display::setShowEAErrors( bool v )
    {
        ShowEAErorrs = v;
    }
    
    void
    Ferrisls_display::PresentNamedAttribute( fh_context ctx, int i, const string& attr )
    {
        try
        {
//              cerr << "PresentNamedAttribute() ctx:" << ctx->getURL()
//                   << " rdn:" << attr
//                   << " v:" << getStrAttr( ctx, attr, "" )
//                   << endl;
//             cerr << "PresentNamedAttribute() ctx:" << ctx->getURL()
//                  << " i:" << i
//                  << " is-bound:" << ctx->isAttributeBound( attr )
//                  << " attr:" << attr
//                  << endl;
            
            if( !ctx->isAttributeBound( attr ) )
            {
                PrintEA( ctx, i, attr, NoSuchAttributeIndicator );
                return;
            }
        
//             fh_attribute att = ctx->getAttribute( attr );
//             fh_istream   ios = att->getIStream();
//             string s="";
//             getline( ios, s );
            string s = getStrAttr( ctx, attr, "", false, true );
//            cerr << "s:" << s << endl;
            
            s = DecorateIfNeeded( ctx, attr, s );

            PrintEA( ctx, i, attr, s );
        }
        catch( NoSuchAttribute& e )
        {
            LG_ATTR_D << "ferrisls NSA attr:" << attr << " e:" << e.what() << endl;
            PrintEA( ctx, i, attr, NoSuchAttributeIndicator );
        }
        catch( exception& e )
        {
            LG_ATTR_D << "ferrisls general attr:" << attr << " e:" << e.what() << endl;
            fh_stringstream ss;
            if( ShowEAErorrs )
            {
                ss << "<e:" << e.what() << ">";
            }
            PrintEA( ctx, i, attr, tostr(ss) );
        }
        catch( ... )
        {
            PrintEA( ctx, i, attr, "failed-to-read" );
        }
    }

    
    void
    Ferrisls_display::ShowAttributes( fh_context ctx ) 
    {
//        cerr << "Ferrisls_display::ShowAttributes ctx:" << ctx->getURL() << endl;
//         cerr << "  name:" << getStrAttr( ctx, "name", "none" ) << endl;
        PresentNamedAttribute( ctx, 0, "name" );
        cout << RecordSeperator << flush; 
    }

    bool
    Ferrisls_display::ShouldEnterContext(fh_context ctx)
    {
        return true;
    }
    
    void
    Ferrisls_display::EnteringContext(fh_context ctx)
    {
    }

    void
    Ferrisls_display::LeavingContext(fh_context ctx)
    {
    }

    void
    Ferrisls_display::setShowContextNameInHeading( gboolean v )
    {
        ShowContextNameInHeading = v;
    }
    
    void
    Ferrisls_display::setNameClassification( int v )
    {
        ClassifyFileNameStyle = v;
    }

    int
    Ferrisls_display::getNameClassification()
    {
       return ClassifyFileNameStyle;
    }
   
    void
    Ferrisls_display::setNoSuchAttributeIndicator( const string& s )
    {
        NoSuchAttributeIndicator = s;
    }

    void
    Ferrisls_display::setShowHeadings( gboolean v )
    {
        ShowHeadings = v;
    }
    
    void
    Ferrisls_display::setFillChar( char c )
    {
        FillChar = c;
    }

    void
    Ferrisls_display::setQuoteStyle( StringQuote::QuoteStyle v )
    {
        Quoter.setStyle( v );
    }

    void
    Ferrisls_display::setEnteredMonitoringLoop( bool v )
    {
        EnteredMonitoringLoop = v;
    }
    
    void
    Ferrisls_display::setFieldSeperator( const string& c )
    {
        FieldSeperator = c;
    }

    void
    Ferrisls_display::setRecordSeperator( const string& c )
    {
//            cerr << "setRecordSeperator() c:" << c << endl;
        RecordSeperator = c;
    }

    
    void
    Ferrisls_display::setMonitorCreate( gboolean v )
    {
        MonitorCreate = v;
    }

    bool
    Ferrisls_display::getMonitorCreate()
    {
        return MonitorCreate;
    }



    void
    Ferrisls_display::setMonitorDelete( gboolean v )
    {
        MonitorDelete = v;
    }

    void
    Ferrisls_display::setMonitorChanged( gboolean v )
    {
        MonitorChanged = v;
    }

    bool
    Ferrisls_display::Monitoring()
    {
        return MonitorCreate | MonitorDelete | MonitorChanged;
    }


    void
    Ferrisls_display::AttachSignals( fh_context c )
    {
        typedef Ferrisls_display _Self;
    
        SigCol[c].ExistsConnection
            = c->getNamingEvent_Exists_Sig() .connect(sigc::mem_fun( *this, &_Self::OnExists ));

//     cerr << "Ferrisls_display::AttachSignals() MonitorCreate:" << MonitorCreate
//          << " this:" << (void*)this
//          << " c:" << c->getDirPath()
//          << endl;
    
        if( MonitorCreate )
        {
            SigCol[c].CreatedConnection
                = c->getNamingEvent_Created_Sig().connect(sigc::mem_fun( *this, &_Self::OnCreated ));
        }
        if( MonitorDelete )
        {
            SigCol[c].DeletedConnection
                = c->getNamingEvent_Deleted_Sig().connect(sigc::mem_fun( *this, &_Self::OnDeleted ));
        }
        if( MonitorChanged )
        {
//            cerr << "Connecting changed handler. url:" << c->getURL() << endl;
            SigCol[c].ChangedConnection
                = c->getNamingEvent_Changed_Sig().connect(sigc::mem_fun( *this, &_Self::OnChanged ));
        }
    }

    void
    Ferrisls_display::DetachSignals( fh_context c )
    {
        if( SigCol.find(c) != SigCol.end() )
        {
            SigCol[c].ExistsConnection.disconnect();
//             if( MonitorCreate )
//             {
//                 SigCol[c].CreatedConnection.disconnect();
//             }
//             if( MonitorChanged )
//             {
//                 SigCol[c].ChangedConnection.disconnect();
//             }
//             if( MonitorDelete )
//             {
//                 SigCol[c].DeletedConnection.disconnect();
//             }
        }
    }



    void
    Ferrisls_display::OnExists ( NamingEvent_Exists* ev,
                                 const fh_context& subc,
                                 string olddn, string newdn )
    {
        LG_CTXREC_D << "Ferrisls_display::OnExists() " << endl;
        LG_CTXREC_D << "Ferrisls_display::OnExists() olddn:" << olddn << endl;
        LG_CTXREC_D << "Ferrisls_display::OnExists() newdn:" << newdn << endl;
        LG_CTXREC_D << "Ferrisls_display::OnExists() subc:" << toVoid(subc) << endl;
        LG_CTXREC_D << "Ferrisls_display::OnExists() subc.url:" << subc->getURL() << endl;
    
        fh_context c    = ev->getSource();
//        fh_context subc = c->getSubContext(newdn);

//         cerr << "Ferrisls_display::OnExists() c:" << toVoid(c)
//              << " c.url:" << c->getURL()
//              << " subc:" << toVoid( subc )
//              << " subc.url:" << subc->getURL()
//              << " getAttr( subc, url ):" << getStrAttr( subc, "url", "none" )
//              << endl;
        
        if( EnteredMonitoringLoop )
        {
            cout << "Discovered" << FieldSeperator;
//            cout << "earl:" << subc->getURL() << endl;
        }
        ShowAttributes( subc );
        AttachSignals(  subc );
    }

    void
    Ferrisls_display::OnCreated( NamingEvent_Created* ev,
                                 const fh_context& subc,
                                 string olddn, string newdn )
    {
        fh_context c    = ev->getSource();
//        fh_context subc = c->getSubContext(newdn);
    
        cout << "Created" << FieldSeperator;
    
        ShowAttributes( subc );
        AttachSignals(  subc );
//    cerr << "Ferrisls_display::OnCreated(exit) " << endl;
    }

    void
    Ferrisls_display::OnChanged( NamingEvent_Changed* ev, string olddn, string newdn )
    {
//        cout << "--------OnChanged( enter ) -----------------------------" << endl;
//      cerr << "OnChanged() old:" << olddn << endl;
//      cerr << "OnChanged() new:" << newdn << endl;

        fh_context c = ev->getSource();
//     cout << "Path:" << c->getDirPath() << " c:" << (void*)c << endl;
        cout << "Changed c:" << toVoid(c) << " " << FieldSeperator;
        ShowAttributes( c );
//      cout << "--------OnChanged( exit ) -----------------------------" << endl;
    }

    void Ferrisls_display::OnDeleted( NamingEvent_Deleted* ev, string olddn, string newdn )
    {
        cout << "Deleted" << FieldSeperator << olddn << RecordSeperator;
    }


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

    void
    Ferrisls_long_display::StreamPrintEA( fh_context ctx,
                                          int i,
                                          const string& attr,
                                          const string& EA,
                                          ostream* ss )
    {
        
//         if( HaveColumnWidths )
//         {
//             (*ss) << setw(ColumnWidths[i])
//                   << setfill(FillChar)
//                   << justificationC( ctx, attr )
//                   << EA;
//         }
//         else
//         {
//             (*ss) << widthC( ctx, attr )
//                   << setfill(FillChar)
//                   << justificationC( ctx, attr )
//                   << EA;
//         }
//         (*ss) << FieldSeperator;


        
        fh_context sc = 0;
        try
        {
            fh_context sc = ctx->getSchema( attr );
        }
        catch( ... ) {}

//        cerr << "m_outputPrecision:" << m_outputPrecision << endl;
        
        if( m_outputPrecision )
            (*ss).precision( m_outputPrecision );
        
//         {
//             double v = 66.123456789;
//             (*ss) << v << endl;
//         }
        
        if( HaveColumnWidths )
        {
            (*ss) << setw(ColumnWidths[i]) << setfill(FillChar) << justification(sc) << EA;
        }
        else
        {
//            cerr << "HaveColumnWidths:" << HaveColumnWidths << " sc:" << isBound(sc) << " attr:" << attr << endl;
            
            if( sc )
            {
                (*ss) << width(sc) << setfill(FillChar) << justification(sc) << EA;
            }
            else
            {
                (*ss) << setfill(FillChar) << justification(sc) << EA;
            }
        }
        (*ss) << FieldSeperator;
    }
    

    Ferrisls_long_display::Ferrisls_long_display()
        :
        HaveColumnWidths(   false ),
        ShowAllAttributes(  false ),
        HumanReadableSizes( false ),
        m_hideXMLDecl( false ),
        m_outputPrecision( 0 )
    {
    }

    
    

    void
    Ferrisls_long_display::PrintEA( fh_context ctx,
                                    int i,
                                    const string& attr,
                                    const string& EA )
    {
        StreamPrintEA( ctx, i, attr, EA, &cout );
    }
    

    void
    Ferrisls_long_display::ShowAttributes( fh_context ctx ) 
    {
        int i=0;
//        cerr << "Ferrisls_long_display::ShowAttributes ctx:" << ctx->getURL() << endl;

        stringset_t alreadyDone;
        
        for( Columns_t::iterator iter = Columns.begin();
             iter != Columns.end(); iter++, i++ )
        {
            if( "recommended-ea" == *iter )
            {
                string recEA = getStrAttr( ctx, *iter, "" );
                fh_stringstream ss( recEA );
                string s;
                while( getline( ss, s, ',' ) )
                {
                    if( s.empty() )
                        continue;
                    
                    s = modifyForHumanReadableSizes( s );
                    PresentNamedAttribute( ctx, i++, s );
                    alreadyDone.insert( s );
                }
            }
            else
            {
//                cerr << "Ferrisls_long_display::ShowAttributes a:" << *iter << endl;
                PresentNamedAttribute( ctx, i, *iter );
                alreadyDone.insert( *iter );
            }
        }

        if( fh_rex r = m_showColumnRegex.getRegex() )
        {
            typedef AttributeCollection::AttributeNames_t ant;
            ant an;
            ctx->getAttributeNames( an );

            for( ant::iterator anit = an.begin(); anit!=an.end(); ++anit )
            {
                string eaname = *anit;
                if( regex_match( eaname, r, boost::match_any ) )
                {
                    PresentNamedAttribute( ctx, i++, eaname );
                }
            }
        }
        
        cout << RecordSeperator; 
    }

    void
    Ferrisls_long_display::EnteringContext(fh_context ctx)
    {
        if( ShowContextNameInHeading )
        {
            cout << "Entering Context:" << ctx->getDirPath()
//             << " size:" << ctx->SubContextCount()
                 << endl;
        }

        if( ShowAllAttributes )
        {
            set<string> z;

                
            Context::SubContextNames_t ls = ctx->getSubContextNames();
            for( Context::SubContextNames_t::iterator iter = ls.begin();
                 iter != ls.end(); iter++ )
            {
                fh_context subc = ctx->getSubContext( *iter );
                Context::AttributeNames_t an;
                subc->getAttributeNames( an );
                    
                for( Context::AttributeNames_t::iterator iter = an.begin();
                     iter != an.end(); iter++ )
                {
                    z.insert( *iter );
                }
            }

            Columns.clear();
            copy( z.begin(), z.end(), back_inserter(Columns));
        }

//    cerr << "Ferrisls_long_display::EnteringContext() ShowHeadings:" << ShowHeadings << endl;
    
        if( ShowHeadings )
        {
            int i=0;

            for( Columns_t::iterator iter = Columns.begin();
                 iter != Columns.end(); iter++, i++ )
            {
                if( "recommended-ea" == *iter )
                {
                    string recEA = getStrAttr( ctx, *iter, "" );
                    fh_stringstream ss( recEA );
                    string s;
                    while( getline( ss, s, ',' ) )
                    {
                        s = modifyForHumanReadableSizes( s );
                        PrintEA( ctx, i++, s, s );
                    }
                }
                else
                {
                    PrintEA( ctx, i, *iter, *iter );
                }
            }

            cout << RecordSeperator;
        }
    }
    
    void
    Ferrisls_long_display::setShowAllAttributes( gboolean v )
    {
        ShowAllAttributes = v;
    }
    
    void
    Ferrisls_long_display::setColumns( const string& c )
    {
        istringstream iss( c );
        string x;
        while(getline( iss, x, ',' ))
        {
            Columns.push_back( x );
        }
    }

    void
    Ferrisls_long_display::appendShowColumnsRegex( const std::string& c )
    {
        m_showColumnRegex.append( c );
    }
    
    
    int
    Ferrisls_long_display::setColumnWidths( const string& c )
    {
        istringstream iss( c );
        int x;
        char comma;

        while( iss >> x )
        {
            ColumnWidths.push_back( x );
//                cerr << "Adding x:" << x << endl;
            iss >> comma;
            if( comma != ',' )
                return 0;
        }
        HaveColumnWidths = 1;
        return 1;
    }

    string
    Ferrisls_long_display::modifyForHumanReadableSizes( const std::string& s )
    {
        if( s == "size" )
        {
            return "size-human-readable";
        }
        return s;
    }
    
    void
    Ferrisls_long_display::setHumanReadableSizes( bool v )
    {
        HumanReadableSizes = v;
    }
    
    void
    Ferrisls_long_display::setHideXMLDeclaration( bool v )
    {
        m_hideXMLDecl = v;
    }

    void
    Ferrisls_long_display::setOutputPrecision( int v )
    {
        m_outputPrecision = v;
    }
    
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    void
    Ferrisls_dired_display::PrintEA( fh_context ctx,
                                     int i,
                                     const string& attr,
                                     const string& EA )
    {
        stringstream ss;
        ostream* oss = &ss;
        StreamPrintEA( ctx, i, attr, EA, oss );
        string s = tostr(ss);
        int len  = s.length();

        if( attr == "name" )
        {
            Coords.push_back( make_pair(runningOffset, runningOffset+len) );
        }
            
        cout << s;
        runningOffset += len;
    }

    void
    Ferrisls_dired_display::workComplete()
    {
        cout << "//DIRED//";

        for( Coords_t::iterator iter = Coords.begin();
             iter != Coords.end(); ++iter )
        {
            cout << " " << iter->first << " " << iter->second;
        }
        cout << nl;

        cout << "//DIRED-OPTIONS// --quoting-style="
             << Quoter.getStringOptionName()
             << endl;
            
        _Base::workComplete();
    }
    



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


    void
    Ferrisls_widthneeding_display::puts( const string& s )
    {
        ConsolePosition += s.length();
        cout << s;
    }

    void
    Ferrisls_widthneeding_display::putnl()
    {
        cout << nl;
        ConsolePosition = 0;
    }
    
    
    Ferrisls_widthneeding_display::Ferrisls_widthneeding_display()
        :
        ConsoleWidth(80),
        ConsolePosition(0)
    {
        if( const gchar* p = g_getenv ("COLUMNS") )
        {
            stringstream ss;
            ss << p;
            if(!(ss >> ConsoleWidth))
            {
                cerr << "Ignoring bad COLUMNS environment variable" << endl;
                ConsoleWidth = 80;
            }
        }
    }

    void Ferrisls_widthneeding_display::setConsoleWidth( int v )
    {
        ConsoleWidth = v;
    }
    


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    Ferrisls_csv_display::Ferrisls_csv_display()
        :
        firstEntry(true)
    {
    }

    

    void
    Ferrisls_csv_display::PrintEA( fh_context ctx,
                                   int i,
                                   const string& attr,
                                   const string& EA )
    {
        if( firstEntry )
        {
            EffectiveWidth = 0;
            firstEntry = false;
            puts(EA);
        }
        else
        {
            int NeededEndPos = ConsolePosition
                + FieldSeperator.length() + EA.length();
                
            if( NeededEndPos <= ConsoleWidth )
            {
                puts(FieldSeperator);
                puts(EA);
            }
            else
            {
                puts(",");
                if( !EffectiveWidth )
                {
                    EffectiveWidth = ConsolePosition;
                }
                putnl();
                puts(EA);
            }
        }
    }

    void Ferrisls_csv_display::workComplete()
    {
        putnl();
        _Base::workComplete();
    }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


    int
    Ferrisls_nameCollector_display::GetColumnLeftMostCharPos( int idx )
    {
        int ret = 0;
            
        for( int i=0; i < idx; ++i )
        {
            ret += ColW[i];
        }
        return ret;
    }

    int
    Ferrisls_nameCollector_display::GetColumnRightMostCharPos( int idx )
    {
        return ColW[idx] + GetColumnLeftMostCharPos(idx);
    }

    int
    Ferrisls_nameCollector_display::GetColumnSize( int idx )
    {
        return ColW[idx];
    }
    
    
    void
    Ferrisls_nameCollector_display::SetColumnSize( int idx, int v )
    {
        ColW[idx] = v;
    }

    
    void
    Ferrisls_nameCollector_display::initColW( int max )
    {
        int i=0;

        MaxCol = max;
        ColW.clear();
        for( i=0; i<=MaxCol; ++i )
        {
            ColW.push_back( 0 );
        }
    }

    int
    Ferrisls_nameCollector_display::getMaxCol()
    {
        return MaxCol;
    }
    


    Ferrisls_nameCollector_display::Ferrisls_nameCollector_display()
        :
        m_initialPrintComplete( false )
    {
    }


    void
    Ferrisls_nameCollector_display::PrintEA( fh_context ctx,
                                             int i,
                                             const string& attr,
                                             const string& EA )
    {
//         cerr << "Ferrisls_nameCollector_display::PrintEA() ea:" << EA << endl;
//         cerr << "    m_initialPrintComplete:" << m_initialPrintComplete << endl;
        if( m_initialPrintComplete )
        {
            _Base::PrintEA( ctx, i, attr, EA );
        }
        else
        {
            NameCollection.push_back( EA );
        }
    }
    
    void
    Ferrisls_nameCollector_display::workComplete()
    {
        m_initialPrintComplete = true;
        /* Upcall */
        _Base::workComplete();
    }
    


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    void
    Ferrisls_hv_display::Init()
    {
        int i             = 0;
        int max           = ConsoleWidth;
        int RunningLength = 0; 

        /*
         * Trim MaxCol based on the first line, this can be a large
         * saving in terms of time.
         */
        for( NameCollection_t::iterator iter = NameCollection.begin();
             iter != NameCollection.end(); ++iter, ++i )
        {
            RunningLength+=iter->length();
            if( RunningLength > ConsoleWidth )
            {
                max = i;
                break;
            }
        }
        initColW(max);
    }

    void
    Ferrisls_hv_display::workComplete()
    {
        int i;

        /* init */
        Init();

//        cerr << "Ferrisls_hv_display::workComplete()" << endl;
        /* Find the number of columns to use and their sizes */
        FindColumnSizes();
            
        /* print output */
        PrintColumns();
            
        /* Upcall */
        _Base::workComplete();
    }
    
    


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
    

    void
    Ferrisls_horiz_display::FindColumnSizes()
    {
        int SepLen = FieldSeperator.length();
        int fileNum = 0;
        for( NameCollection_t::iterator iter = NameCollection.begin();
             iter != NameCollection.end();
            )
        {
            int colidx = fileNum % getMaxCol();
            int curLen = iter->length()+SepLen;


            /*
             * Check if this column is to long
             */
            if( GetColumnRightMostCharPos( colidx ) > ConsoleWidth )
            {
                /*
                 * Trim column count to make this column one past the last one.
                 * not that colidx is % getMaxCol(), thus it is always in the range
                 * 0 - (getMaxCol()-1) inclusive.
                 */
                initColW( colidx );

                /*
                 * Start again.
                 */
                iter = NameCollection.begin();
                fileNum = 0;
            
                continue;
            }

            if( curLen > GetColumnSize( colidx ) )
            {
//                     cerr << "grow fileNum:" << fileNum
//                          << " colidx:" << colidx
//                          << " name:" << *iter
//                          << " lm:" << GetColumnLeftMostCharPos( colidx )
//                          << " curLen:" << curLen
//                          << " CW:" << ConsoleWidth
//                          << endl;

                /*
                 * If we are making a column larger, then any of the
                 * columns from this one to getMaxCol()-1 may have become to large.
                 * so we must test all of them starting from this column
                 * that we are making larger. The reason that the test is
                 * outside the loop is to allow columns after this one to
                 * still be tested, as we are not resizing them but they
                 * may exceed the Console width due to a earlier column
                 * having been expanded.
                 */
                SetColumnSize( colidx, curLen );
                    
                // Check this columns length again //
                continue;
            }
            ++iter;
            ++fileNum;
        }
    }
    

    void
    Ferrisls_horiz_display::PrintColumns()
    {
        int fileNum=0;
        for( NameCollection_t::iterator iter = NameCollection.begin();
             iter != NameCollection.end(); ++iter, ++fileNum )
        {
            int i = fileNum % getMaxCol();

            cout
                << left
                << setfill(FillChar)
                << setw( GetColumnSize(i)- FieldSeperator.length() ) << *iter
                << FieldSeperator;

            if( i == (getMaxCol()-1) )
            {
                cout << nl;
            }
        }
            
        cout << nl;
    }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <math.h>

    int
    Ferrisls_vert_display::GetRowCount()
    {
        if( getMaxCol() <= 1 )
            return NameCollection.size();
        
        double x = NameCollection.size();
        x = x / (getMaxCol()-1);
        double c = ceil( x );
        return (int)c;
    }

    bool Ferrisls_vert_display::checkfit()
    {
        int rows   = GetRowCount();
        int r      = 0;
        int SepLen = FieldSeperator.length();

//             cerr << "checkfit() maxcol:" << getMaxCol() << endl;
//             cerr << "checkfit()   rows:" << rows << endl;

        int NameCollection_size = NameCollection.size();
        
        for( r=0; r < rows; ++r )
        {
            int total = 0;

            for( int c=0; c < (getMaxCol()); ++c )
            {
                NameCollection_t::iterator iter = NameCollection.begin();
                if( r + c*rows >= NameCollection_size )
                {
                    break;
                }
                advance( iter, r + c*rows );
                int curLen = iter->length()+SepLen;
                    
                total += curLen;

                if( GetColumnSize( c ) < curLen )
                {
                    SetColumnSize(c, curLen );
                }
            }

//                 cerr << "row:" << r << " total:" << total << endl;
                
            if( total > ConsoleWidth )
            {
                return false;
            }
        }

        return true;
    }
    
    


    void
    Ferrisls_vert_display::FindColumnSizes()
    {
        int SepLen = FieldSeperator.length();
        int rows   = GetRowCount();

//             cerr << "FindColumnSizes() maxcol:" << getMaxCol() << endl;
//             cerr << "FindColumnSizes()   rows:" << rows << endl;

        /*
         * Keep scaling it down until it fits.
         */
        while( !checkfit() )
        {
//                 cerr << "FindColumnSizes() maxcol:" << getMaxCol() << endl;
//                 cerr << "FindColumnSizes()   rows:" << rows << endl;

            initColW( getMaxCol() - 1 );
        }
        if( getMaxCol() < 1 )
            initColW( 1 );
    }
    

    void
    Ferrisls_vert_display::PrintColumns()
    {
//        fh_context sc = ctx->getSchema( attr );
        int rows = GetRowCount();
        int r    = 0;

//         cerr << "Ferrisls_vert_display::PrintColumns() rows:" << rows << endl;
//         cerr << "Ferrisls_vert_display::PrintColumns() getMaxCol():" << getMaxCol() << endl;

        int NameCollection_size = NameCollection.size();
        
        for( r=0; r < rows; ++r )
        {
            for( int c=0; c < (getMaxCol()); ++c )
            {
                NameCollection_t::iterator iter = NameCollection.begin();
//                 cerr << "Ferrisls_vert_display::PrintColumns(iter) r:" << r
//                      << " c:" << c << " (r + c*rows):" << (r + c*rows)
//                      << " nc.sz:" << NameCollection.size()
//                      << endl;

                if( r + c*rows >= NameCollection_size )
                {
                    break;
                }

                
                advance( iter, r + c*rows );

                cout << left // justification(sc)
                     << setfill(FillChar)
                     << setw( GetColumnSize(c)- FieldSeperator.length() )
                     << *iter
                     << FieldSeperator;
            }
            cout << nl;
        }
    }





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

Ferrisls::Ferrisls()
    :
    rctx(0),
    ListDirectoryNodeOnly(0),
    ForceReadRootDirectoryNodes(0),
    ImplyListDirectoryNodeOnly( true ),
    RecursiveList(0),
    usingFilter(false),
    HadErrors(false),
    PreorderTraversal( false ),
    DontFollowLinks( false ),
    DontDecendIntoFiles( false ),
    DontDescendRegexString(""),
    DontDescendRegex(0)
{
}

    void
    Ferrisls::setDisplay( fh_lsdisplay v )
    {
        Display = v;
    }
    

    void
    Ferrisls::setFilterString( const string& v )
    {
        usingFilter = true;
        FilterString = v;
    }

    void
    Ferrisls::setSortString( const string& v )
    {
        SortString = v;
    }

    
    
    void
    Ferrisls::setRecursiveList( gboolean x )
    {
        RecursiveList = x;
    }
    
    
    void
    Ferrisls::setListDirectoryNodeOnly( gboolean x )
    {
        ListDirectoryNodeOnly = x;
    }

    void
    Ferrisls::setImplyListDirectoryNodeOnly( gboolean x )
    {
        ImplyListDirectoryNodeOnly = x;
    }
    
    void
    Ferrisls::setForceReadRootDirectoryNodes( gboolean x )
    {
        ForceReadRootDirectoryNodes = x;
    }

    void
    Ferrisls::setDontDescendRegex( const std::string& s )
    {
        DontDescendRegexString = s;
        DontDescendRegex = toregexh( s );
    }
    
    

// void
// Ferrisls::setContextClass( const string& c ) throw( NoSuchContextClass )
// {
//     Factory.setContextClass( c );
// }

// void
// Ferrisls::setRoot( const string& c )
// {
//     Factory.AddInfo( RootContextFactory::ROOT, c );
// }

    void
    Ferrisls::setURL( const std::string& earl )
    {
        theURL = earl;
    }

    bool
    Ferrisls::hadErrors()
    {
        return HadErrors;
    }

    void 
    Ferrisls::hadErrors( bool v )
    {
        HadErrors = v;
    }
    
    void
    Ferrisls::usePreorderTraversal( bool v )
    {
        PreorderTraversal = v;
    }
    
    bool
    Ferrisls::usePreorderTraversal()
    {
        return PreorderTraversal;
    }

    void
    Ferrisls::setDontFollowLinks( bool v )
    {
        DontFollowLinks = v;
    }

    void
    Ferrisls::setDontDecendIntoFiles( bool v )
    {
        DontDecendIntoFiles = v;
    }
    
    void
    Ferrisls::MaybeRecurse( fh_context rctx )
    {
        LG_CTXREC_D << "Ferrisls::MaybeRecurse() RecursiveList:" << RecursiveList << endl;
        
        if( RecursiveList )
        {
            try
            {
                Context::SubContextNames_t ls = rctx->getSubContextNames();
                for( Context::SubContextNames_t::iterator iter = ls.begin();
                     iter != ls.end(); iter++ )
                {
                    fh_context c = rctx->getSubContext( *iter );
                    LG_CTXREC_D << "Ferrisls::MaybeRecurse() dfl:" << DontFollowLinks
                                << " c:" << c->getDirPath()
                                << endl;

                    if( DontDescendRegex )
                    {
                        if( regex_search( c->getURL(),
                                          DontDescendRegex,
                                          boost::match_any))
                        {
                            LG_CTXREC_D << "Ferrisls::MaybeRecurse() matches dont descend regex."
                                        << endl;
                            continue;
                        }
                    }
                    
                    if( DontFollowLinks )
                    {
                        LG_CTXREC_D << "+++ Ferrisls::MaybeRecurse() c:" << c->getDirPath()
                                    << " df-size:" << getStrAttr( c, "dontfollow-size", "0" )
                                    << " df-islink:" << getStrAttr( c, "dontfollow-is-link", "0" )
                                    << " islink:" << getStrAttr( c, "is-link", "0" )
                                    << endl;
                        if( int islink = toint(getStrAttr( c, "dontfollow-is-link", "0" )))
                        {
                            continue;
                        }
                    }

                    if( DontDecendIntoFiles )
                    {
//                        cerr << "MaybeRecurse is-file:" << getStrAttr( c, "is-file", "0" ) << " url:" << c->getURL() << endl;
                        if( isTrue( getStrAttr( c, "is-file", "0" ) ) )
                            continue;
                    }
                    
                    operator()(c);
                }
            }
            catch( FerrisNotReadableAsContext& e )
            {
                LG_CTXREC_D << "Ferrisls::MaybeRecurse() not a context rctx:" << rctx->getURL()
                         << " e:" << e.what()
                         << endl;
                hadErrors( true );
            }
        }
    }
    
    void
    Ferrisls::operator()( fh_context rctx )
    {
        bool read=0;

        try
        {
//                  cerr << "op() rctx:" << rctx << endl; 
//                  cerr << "op() rctx:" << isBound(rctx) << endl;
//                 cerr << "op() rctx->impl:" << toVoid(rctx)) << endl;
//                 cerr << "op() path:" << rctx->getDirPath() << endl;

            LG_CTXREC_D << "Ferrisls::operator() rctx:" << rctx->getURL() << endl;
            
            if( ForceReadRootDirectoryNodes )
                rctx->read();

            // This will regress in copy/move.
//             if( !ListDirectoryNodeOnly && !isTrue( getStrAttr( rctx, "is-dir-try-automounting", "1" ) ) )
//                 ListDirectoryNodeOnly = true;
            
            LG_CTXREC_D << "Ferrisls::operator() ListDirectoryNodeOnly:" << ListDirectoryNodeOnly << endl;
            if( ListDirectoryNodeOnly )
            {
                if( Display->ShouldEnterContext( rctx ) )
                {
                    getEnteringContext_Sig().emit( *this, rctx );
                    Display->EnteringContext( rctx );
                    Display->ShowAttributes( rctx );
                    Display->LeavingContext( rctx );
                    getLeavingContext_Sig().emit( *this, rctx );
                }
            }
            else
            {
                Display->DetachSignals( rctx );

                if( !Display->ShouldEnterContext( rctx ) )
                    return;
                
                getDiskReadStarted_Sig().emit( *this, rctx );
                OnExistsDiskReadContextsProcessed = 0;
                sigc::connection DiskReadConnection =
                    rctx->getNamingEvent_Exists_Sig().connect(
                        sigc::mem_fun( *this, &_Self::OnExistsDiskRead ));
                LG_CTXREC_D << "Ferrisls::operator()...calling read()" << endl;
                rctx->read();
                DiskReadConnection.disconnect();
                getDiskReadDone_Sig().emit( *this, rctx );

                /* We should only fire this when we know
                 * we are able to read the context */
                getEnteringContext_Sig().emit( *this, rctx );
                Display->EnteringContext( rctx );
                fh_context original_rtx = rctx;
                
                if( usePreorderTraversal() )
                {
                    MaybeRecurse( rctx );
                }

                
                /* Filtering */
                if( usingFilter )
                {
                    fh_context filter = Factory::MakeFilter( FilterString );
                    fh_context fc     = Factory::MakeFilteredContext( rctx, filter );

                    /*
                     * We have to read the base context before we drop our handle
                     * to it
                     */
                    getFilterStarted_Sig().emit( *this );
                    rctx->read();
                    rctx = fc;
                }

                /* Sorting */
                if( SortString.length() )
                {
//                     cerr << "ferrisls: SortString:" << SortString
//                          << " rctx:" << rctx->getURL()
//                          << endl;
                    
                    rctx->read();
                    fh_sorter sorter = Factory::MakeSorter( SortString );
                    getSortStarted_Sig().emit( *this );
                    fh_context sc    = Factory::MakeSortedContext( rctx, sorter );

//                         if( DEBUG )
//                         {
//                             cout << "ls::op() listing in order" << endl;
//                             Context::SubContextNames_t na = sc->getSubContextNames();

//                             for( Context::SubContextNames_t::iterator iter = na.begin();
//                                  iter != na.end(); ++iter )
//                             {
//                                 cout << "iter:" << sc->getSubContext(*iter)->getDirPath() << endl;
//                             }
//                         }

                    rctx = sc;
                }
                    

                Display->AttachSignals( rctx );
                getContextPropergationStarted_Sig().emit( *this, rctx );

//                 cerr << "Ferrisls.cpp dumping out contexts.:" << endl;
//                 rctx->dumpOutItems();
//                cerr << "Ferrisls.cpp rctx->read(1); url:" << rctx->getURL() << endl;
                LG_CTXREC_D << "Ferrisls::operator()...calling read(2)" << endl;
                rctx->read();
//                cerr << "Ferrisls.cpp rctx->read(2); url:" << rctx->getURL() << endl;
                read=1;

                if( !usePreorderTraversal() )
                {
                    MaybeRecurse( original_rtx );
                }
                Display->LeavingContext( rctx );
                getLeavingContext_Sig().emit( *this, rctx );
            }
        }
        catch( FerrisNotReadableAsContext& e )
        {
            if( RecursiveList )
            {
            }
            else
            {
                LG_CTXREC_D << "Not readable as a context:" << e.what() << endl;
                cerr << "Not readable as a context:" << e.what() << endl;
                hadErrors( true );
            }
        }
        catch( CanNotReadContext& e )
        {
            LG_CTXREC_D << "Can not read context:" << rctx->getDirPath()
                        << " reason:" << e.what()
                        << endl;
            cerr << "Can not read context:" << rctx->getDirPath()
                 << " reason:" << e.what()
                 << endl;
            hadErrors( true );
        }
    }
    
    void
    Ferrisls::workComplete()
    {
        Display->workComplete();
    }

    int
    Ferrisls::operator()()
    {
        Display->setEnteredMonitoringLoop(false);
        Display->workStarting();

        try
        {
//        rctx = Factory.resolveContext();
            rctx = Resolve( theURL );

            // Make sure we revert this value before the next file.
            Util::ValueRestorer< gboolean > _ListDirectoryNodeOnlyObj( ListDirectoryNodeOnly );
            if( !ListDirectoryNodeOnly
                && ImplyListDirectoryNodeOnly
                && isFalse( getStrAttr( rctx, "is-dir-try-automounting", "1" ) ) )
            {
                ListDirectoryNodeOnly = true;
            }
            
            operator()(rctx);

            if( Display->Monitoring() && rctx )
            {
                if( !isTrue( getStrAttr( rctx, "is-active-view", "1" ) ) )
                {
                    cerr << "Warning, monitoring for events on a context which you have"
                         << " forced to be passive.\n No events will ever come!\n"
                         << " url:" << rctx->getURL()
                         << endl;
                }
            }
            
            workComplete();
            Display->DetachAllSignals( false );
        }
        catch( RootContextCreationFailed& e )
        {
            cerr << "Can not open root context. error: " << e.what() << endl;
            throw e;
        }

        return 0;
    }
    
    void
    Ferrisls::OnExistsDiskRead( NamingEvent_Exists* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn )
    {
        fh_context c    = ev->getSource();
//        fh_context subc = c->getSubContext(newdn);

        ++OnExistsDiskReadContextsProcessed;

        if( (OnExistsDiskReadContextsProcessed % 20) == 0 )
        {
            getDiskReadProgress_Sig().emit( *this, c, OnExistsDiskReadContextsProcessed );
//         cerr << "Ferrisls::OnExistsDiskRead() context:" << subc->getDirPath() << endl;
//         cerr << "Ferrisls::OnExistsDiskRead() n:"<< OnExistsDiskReadContextsProcessed << endl;
        }
    
    }



    Ferrisls::FilterStarted_Sig_t&
    Ferrisls::getFilterStarted_Sig()
    {
        return FilterStarted_Sig;
    }

    Ferrisls::SortStarted_Sig_t&
    Ferrisls::getSortStarted_Sig()
    {
        return SortStarted_Sig;
    }

    Ferrisls::ContextPropergationStarted_Sig_t&
    Ferrisls::getContextPropergationStarted_Sig()
    {
        return ContextPropergationStarted_Sig;
    }

    Ferrisls::DiskReadStarted_Sig_t&
    Ferrisls::getDiskReadStarted_Sig()
    {
        return DiskReadStarted_Sig;
    }

    Ferrisls::DiskReadProgress_Sig_t&
    Ferrisls::getDiskReadProgress_Sig()
    {
        return DiskReadProgress_Sig;
    }

    Ferrisls::DiskReadDone_Sig_t&
    Ferrisls::getDiskReadDone_Sig()
    {
        return DiskReadDone_Sig;
    }

    Ferrisls::EnteringContext_Sig_t&
    Ferrisls::getEnteringContext_Sig()
    {
        return m_EnteringContext_Sig;
    }
    
    Ferrisls::LeavingContext_Sig_t&
    Ferrisls::getLeavingContext_Sig()
    {
        return m_LeavingContext_Sig;
    }


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

