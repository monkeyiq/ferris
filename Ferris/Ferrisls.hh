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

    $Id: Ferrisls.hh,v 1.11 2010/09/24 21:30:49 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FERRISLS_H_
#define _ALREADY_INCLUDED_FERRIS_FERRISLS_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <math.h>

#include <Ferris/Ferris.hh>
#include <set>

namespace Ferris
{

/**
 *
 * Functor that adds quoting to internal contents of a string and quotes the
 * string itself depending on the QuoteStyle.
 *
 * This functor is a mapping
 * std::string -> std::string
 *
 * FIXME LOCALE_QUOTING and other LOCALE stuff doesn't work
 *
 */
    class FERRISEXP_API StringQuote
    {
    public:

        /**
         * The type of quoting/escaping required.
         *
         * NO_QUOTING       does absolutely nothing and is designed to be very fast.
         *                  this is done to allow this functor to remain in use even
         *                  if it is impotent.
         * LITERAL_QUOTING  This mode changes strange characters to a ?
         * SHELL_QUOTING    This subclass of LITERAL places ' quotes around the entire
         *                  string if it contains characters that a shell may interpret.
         * SHELL_ALWAYS_QUOTING This is a subclass of SHELL_QUOTING that always places
         *                  quotes (') around the given string.
         *
         * C_QUOTING        This mode places double quotes around the string and escapes
         *                  any special chars like newline as \n and other characters as
         *                  octal
         * ESCAPE_QUOTING   This subclass of C_QUOTING works exactly like C_QUOTING except
         *                  no double quotes are added before and after the string.
         * 
         */
        enum QuoteStyle
        {
            NO_QUOTING            = 1<<0,
            LITERAL_QUOTING       = 1<<1,
            C_QUOTING             = 1<<2,
            SHELL_QUOTING         = 1<<3,
            SHELL_ALWAYS_QUOTING  = 1<<4,
            ESCAPE_QUOTING        = 1<<5,
//         LOCALE_QUOTING        = 1<<6,
//         CLOCALE_QUOTING       = 1<<7
        };

        /**
         * Quote style in use
         */
        QuoteStyle style;

        /**
         * Create a quoting functor with no quoting as the default.
         */
        StringQuote( QuoteStyle v = NO_QUOTING );

        /**
         * These are characters that do not need to be treated specially.
         * This is here so that the functor can operate fast on strings
         * that do not contain heaps of troublesome characters.
         */
        typedef std::set<char> acceptable_chars_t;
        acceptable_chars_t acceptable_chars;

        /**
         * A mapping string -> string that defines a string to find and what to
         * replace it with.
         */
        typedef std::pair< std::string, std::string > change_t;

        /**
         * A change group
         */
        typedef std::vector< change_t > changelist_t;
    
        /**
         * String translations that are to be performed on every character of the string.
         * changelist_shell is the same as changelist except that a match in changelist_shell
         * indicates that the string needs to be quoted for the shell.
         */
        changelist_t changelist;
        changelist_t changelist_shell;

        /**
         * String translations that are to be performed only on strings starting at the
         * first character of the given input string.
         * changelist_shell is the same as changelist except that a match in changelist_shell
         * indicates that the string needs to be quoted for the shell.
         */
        changelist_t changelist_dollar;
        changelist_t changelist_dollar_shell;
    
        /**
         * String used to quote the string pre/postfix. This is needed because the
         * quote string may need to be escaped in the output.
         */
        std::string quote_string;
        bool backslash_escapes;


        /**
         * Set the quoting style. Refer to the definition of QuoteStyle for the
         * meanings of the various quote methods.
         */
        void setStyle( QuoteStyle v );

        /**
         * Quotes the string s using the pre/postfix of q
         */
        std::string quote( const std::string& s, const std::string& q );

        /**
         * If in shell mode make sure that this entry is pre/postfix quoted.
         */
        void ShellMustBeQuoting();

        /**
         * Setup for shell mode.
         */
        void initShellQuoteString();

        /**
         * Look through cl trying to match each key in cl with the string starting at
         * offset i in string s, changing ret is needed.
         *
         * @return true if a change was made.
         */
        bool performSubSt( const changelist_t& cl, const std::string& s, int& i, std::string& ret );
    
        /**
         * Perform a string -> string translation
         */
        std::string operator()( const std::string& s );

        std::string getStringOptionName();
    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    class Ferrisls;

    class FERRISEXP_API Ferrisls_display
        :
        public Handlable
    {
    public:

        enum {
            CLASSIFY_NONE    =1<<0,
            CLASSIFY_EXE     =1<<1,
            CLASSIFY_DIR     =1<<2,
            CLASSIFY_SOCKET  =1<<3,
            CLASSIFY_FIFO    =1<<4,
            CLASSIFY_SYMLINK =1<<5
        };

    protected:

        /*
         * Maybe we should append chars to the name? (*|/=@)
         */
        std::string ClassifyIfNeeded( const fh_context& ctx,
                                      const std::string& attrName,
                                      std::string s );

        std::string QuoteIfNeeded( const fh_context& ctx,
                                   const std::string& attrName,
                                   const std::string& s );

        virtual std::string DecorateIfNeeded( const fh_context& ctx,
                                              const std::string& attrName,
                                              std::string s );
    
        gboolean MonitorCreate;
        gboolean MonitorDelete;
        gboolean MonitorChanged;
        gboolean EnteredMonitoringLoop;
        gboolean ShowHeadings;
        gboolean ShowContextNameInHeading;

        std::string FieldSeperator;
        std::string RecordSeperator;

        std::string NoSuchAttributeIndicator;
    
        char FillChar;

        int ClassifyFileNameStyle;
        StringQuote Quoter;

        bool ShowEAErorrs;
        
    public:

        Ferrisls_display();
        virtual ~Ferrisls_display();

        /**
         * Called once at the start of a listing. Not called for each dir in a
         * recursive listing.
         */
        virtual void workStarting();
        
        /**
         * Called once at the end of a listing. Not called for each dir in a
         * recursive listing.
         */
        virtual void workComplete();

        /**
         * Called by PresentNamedAttribute with the logical equal of
         * EA==getStrAttr(ctx,attr) but EA might be modified for quoting
         * or encoded for shell display.
         */
        virtual void PrintEA( fh_context ctx,
                              int i,
                              const std::string& attr,
                              const std::string& EA );
        void PresentNamedAttribute( fh_context ctx, int i, const std::string& attr );
        virtual void ShowAttributes( fh_context ctx );

        /**
         * Called before EnteringContext() to tell if we should actaully visit the context or not.
         * if this method returns false then EnteringContext()/tree traversal/LeavingContext()
         * are not preformed for ctx.
         */
        virtual bool ShouldEnterContext(fh_context ctx);
        
        /**
         * Called before listing a context. Paired with a LeavingContext() call
         */
        virtual void EnteringContext(fh_context ctx);
        
        /**
         * Called after listing a context before listing another.
         */
        virtual void LeavingContext(fh_context ctx);

        void setShowContextNameInHeading( gboolean v );;
        void setNameClassification( int v );
        int  getNameClassification();
        void setNoSuchAttributeIndicator( const std::string& s );
        void setShowHeadings( gboolean v );
        void setFillChar( char c );
        void setQuoteStyle( StringQuote::QuoteStyle v );
        void setEnteredMonitoringLoop( bool v = true );
        void setFieldSeperator( const std::string& c );
        void setRecordSeperator( const std::string& c );
        void setMonitorCreate( gboolean v );
        bool getMonitorCreate();
        void setMonitorDelete( gboolean v );
        void setMonitorChanged( gboolean v );
        bool Monitoring();

        void setShowEAErrors( bool v );
    
        struct SignalCollection
        {
            sigc::connection ExistsConnection;
            sigc::connection CreatedConnection;
            sigc::connection ChangedConnection;
            sigc::connection DeletedConnection;
        };
        typedef std::map< fh_context, SignalCollection > SigCol_t;
        SigCol_t SigCol;
    
        void AttachSignals( fh_context c );
        void DetachSignals( fh_context c );
        /**
         * Detaches from exists signals, and if force from all other signals.
         */
        void DetachAllSignals( bool force = true );
        
        virtual void OnExists ( NamingEvent_Exists* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        virtual void OnCreated( NamingEvent_Created* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        virtual void OnChanged( NamingEvent_Changed* ev, std::string olddn, std::string newdn );
        virtual void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );

    };

    FERRIS_SMARTPTR( Ferrisls_display, fh_lsdisplay );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    class FERRISEXP_API Ferrisls_long_display
        :
        public Ferrisls_display
    {
        gboolean ShowAllAttributes;

        /**
         * Names of the attributes to show.
         */
        typedef std::list<std::string> Columns_t;
        Columns_t Columns;

        RegexCollection m_showColumnRegex;

        bool HaveColumnWidths;
        typedef std::vector<int> ColumnWidths_t;
        ColumnWidths_t ColumnWidths;

        bool HumanReadableSizes;

        int m_outputPrecision;

    protected:

        bool m_hideXMLDecl;

        void StreamPrintEA( fh_context ctx,
                            int i,
                            const std::string& attr,
                            const std::string& EA,
                            std::ostream* ss );
        std::string modifyForHumanReadableSizes( const std::string& s );

    public:

        Ferrisls_long_display();

        virtual void PrintEA( fh_context ctx,
                              int i,
                              const std::string& attr,
                              const std::string& EA );
        virtual void ShowAttributes( fh_context ctx );
        void EnteringContext(fh_context ctx);
        void setShowAllAttributes( gboolean v );
        void setColumns( const std::string& c );
        void appendShowColumnsRegex( const std::string& c );
        int setColumnWidths( const std::string& c );
        void setHumanReadableSizes( bool v );

        void setHideXMLDeclaration( bool v );
        void setOutputPrecision( int v );
    };


    FERRIS_SMARTPTR( Ferrisls_long_display, fh_lsdisplay_long );


    class FERRISEXP_API Ferrisls_dired_display
        :
        public Ferrisls_long_display
    {
        typedef Ferrisls_long_display _Base;
    
        typedef std::pair< int, int > StartEnd_t;
        typedef std::vector<StartEnd_t> Coords_t;
        Coords_t Coords;

        int runningOffset;
    
    public:
 
        virtual void PrintEA( fh_context ctx,
                              int i,
                              const std::string& attr,
                              const std::string& EA );
        virtual void workComplete();
    
    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_widthneeding_display
        :
        public Ferrisls_display
    {
    protected:

        int ConsoleWidth;
        int ConsolePosition;

        void puts( const std::string& s );
        void putnl();
    
    public:

        Ferrisls_widthneeding_display();

        void setConsoleWidth( int v );
    };

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_csv_display
        :
        public Ferrisls_widthneeding_display
    {
        typedef Ferrisls_widthneeding_display _Base;
    
        bool firstEntry;
        int EffectiveWidth;
    
    public:

        Ferrisls_csv_display();

        virtual void PrintEA( fh_context ctx,
                              int i,
                              const std::string& attr,
                              const std::string& EA );
        virtual void workComplete();
    };


    FERRIS_SMARTPTR( Ferrisls_csv_display, fh_lsdisplay_csv );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_nameCollector_display
        :
        public Ferrisls_widthneeding_display
    {
        typedef Ferrisls_widthneeding_display _Base;
        
        typedef std::vector<int> ColW_t;
        ColW_t ColW;
        int MaxCol;
        bool m_initialPrintComplete;

    protected:

        typedef std::list<std::string> NameCollection_t;
        NameCollection_t NameCollection;

        int GetColumnLeftMostCharPos( int idx );
        int GetColumnRightMostCharPos( int idx );
        int GetColumnSize( int idx );
        void SetColumnSize( int idx, int v );
        void initColW( int max );
        int getMaxCol();
    
    public:

        Ferrisls_nameCollector_display();
    
        virtual void PrintEA( fh_context ctx,
                              int i,
                              const std::string& attr,
                              const std::string& EA );
        virtual void workComplete();
    };

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_hv_display
        :
        public Ferrisls_nameCollector_display
    {
        typedef Ferrisls_nameCollector_display _Base;
    
    protected:

        virtual void Init();
        virtual void FindColumnSizes() = 0;
        virtual void PrintColumns() = 0;
    
    public:

        virtual void workComplete();
    };

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
    

class FERRISEXP_API Ferrisls_horiz_display
        :
        public Ferrisls_hv_display
    {
        typedef Ferrisls_hv_display _Base;

    protected:

        void FindColumnSizes();
        void PrintColumns();
    };

    FERRIS_SMARTPTR( Ferrisls_horiz_display, fh_lsdisplay_horiz );


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_vert_display
        :
        public Ferrisls_hv_display
    {
        typedef Ferrisls_hv_display _Base;

        int GetRowCount();
        bool checkfit();
    
    protected:

        void FindColumnSizes();
        void PrintColumns();
    };

    FERRIS_SMARTPTR( Ferrisls_vert_display, fh_lsdisplay_vert );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_xmlraw_display
        :
        public Ferrisls_long_display
    {
        typedef Ferrisls_long_display _Base;

        virtual void workStarting();
        virtual void workComplete();
        virtual void EnteringContext(fh_context ctx);
        virtual void LeavingContext(fh_context ctx);
        
    protected:

        struct Ferrisls_xmlraw_display_private* P;
        
        virtual void ShowAttributes( fh_context ctx );
        virtual void PrintEA( fh_context ctx,
                              int i,
                              const std::string& attr,
                              const std::string& EA );

        
    public:
        Ferrisls_xmlraw_display();
        virtual ~Ferrisls_xmlraw_display();

    };

    FERRIS_SMARTPTR( Ferrisls_xmlraw_display, fh_lsdisplay_xmlraw );
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_xml_display
        :
        public Ferrisls_long_display
    {
        typedef Ferrisls_long_display _Base;

        virtual void workStarting();
        virtual void workComplete();
        virtual void EnteringContext(fh_context ctx);
        virtual void LeavingContext(fh_context ctx);
        
    protected:

        struct Ferrisls_xml_display_private* P;
        
        virtual void ShowAttributes( fh_context ctx );
        virtual void PrintEA( fh_context ctx,
                              int i,
                              const std::string& attr,
                              const std::string& EA );

        
    public:
        Ferrisls_xml_display();
        virtual ~Ferrisls_xml_display();

    };

    FERRIS_SMARTPTR( Ferrisls_xml_display, fh_lsdisplay_xml );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_xmle_display
        :
        public Ferrisls_xml_display
{
    typedef Ferrisls_long_display _Base;
    
protected:
    
    virtual void PrintEA( fh_context ctx,
                          int i,
                          const std::string& attr,
                          const std::string& EA );
    
public:
    Ferrisls_xmle_display();
    virtual ~Ferrisls_xmle_display();
};
    
    FERRIS_SMARTPTR( Ferrisls_xmle_display, fh_lsdisplay_xmle );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_xml_xsltfs_debug_display
        :
        public Ferrisls_xml_display
{
    typedef Ferrisls_long_display _Base;

    void* ensureElementCreated( fh_context ctx );
    
protected:

    virtual void EnteringContext(fh_context ctx);
    virtual void LeavingContext(fh_context ctx);
    
    virtual void ShowAttributes( fh_context ctx );
    virtual void PrintEA( fh_context ctx,
                          int i,
                          const std::string& attr,
                          const std::string& EA );
    
public:
    Ferrisls_xml_xsltfs_debug_display();
    virtual ~Ferrisls_xml_xsltfs_debug_display();
};
    
    FERRIS_SMARTPTR( Ferrisls_xml_xsltfs_debug_display, fh_lsdisplay_xml_xsltfs_debug );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_json_display
        :
        public Ferrisls_long_display
    {
        typedef Ferrisls_long_display _Base;

        virtual void workStarting();
        virtual void workComplete();
        virtual void EnteringContext(fh_context ctx);
        virtual void LeavingContext(fh_context ctx);
        
    protected:

        struct Ferrisls_json_display_private* P;
        
        virtual void ShowAttributes( fh_context ctx );
        virtual void PrintEA( fh_context ctx,
                              int i,
                              const std::string& attr,
                              const std::string& EA );

        
    public:
        Ferrisls_json_display();
        virtual ~Ferrisls_json_display();

    };

    FERRIS_SMARTPTR( Ferrisls_json_display, fh_lsdisplay_json );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Ferrisls_rdf_display
        :
        public Ferrisls_long_display
    {
        typedef Ferrisls_long_display _Base;

        struct Ferrisls_rdf_display_private* P;

        virtual void workStarting();
        virtual void workComplete();
        virtual void EnteringContext(fh_context ctx);
        virtual void LeavingContext(fh_context ctx);
        
    protected:

        virtual void ShowAttributes( fh_context ctx );
        virtual void PrintEA( fh_context ctx,
                              int i,
                              const std::string& attr,
                              const std::string& EA );
        
    public:
        Ferrisls_rdf_display();
        virtual ~Ferrisls_rdf_display();
        

    };

    FERRIS_SMARTPTR( Ferrisls_rdf_display, fh_lsdisplay_rdf );

    
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


class FERRISEXP_API Ferrisls
        :
        public Handlable
    {
        typedef Ferrisls _Self;
        
    protected:

        std::string theURL;
//        RootContextFactory Factory;
        fh_context rctx;

        gboolean ListDirectoryNodeOnly;
        gboolean ForceReadRootDirectoryNodes;
        gboolean ImplyListDirectoryNodeOnly;
        gboolean RecursiveList;

        gboolean usingFilter;
        std::string FilterString;
        std::string SortString;

        fh_lsdisplay Display;

        bool HadErrors;

    public:

        typedef sigc::signal2< void, Ferrisls&, fh_context > DiskReadStarted_Sig_t;
        DiskReadStarted_Sig_t& getDiskReadStarted_Sig();

        typedef sigc::signal2< void, Ferrisls&, fh_context > DiskReadDone_Sig_t;
        DiskReadDone_Sig_t& getDiskReadDone_Sig();

        typedef sigc::signal3< void, Ferrisls&, fh_context, long > DiskReadProgress_Sig_t;
        DiskReadProgress_Sig_t& getDiskReadProgress_Sig();

        typedef sigc::signal1< void, Ferrisls& > FilterStarted_Sig_t;
        FilterStarted_Sig_t& getFilterStarted_Sig();

        typedef sigc::signal1< void, Ferrisls& > SortStarted_Sig_t;
        SortStarted_Sig_t& getSortStarted_Sig();

        typedef sigc::signal2< void, Ferrisls&, fh_context > ContextPropergationStarted_Sig_t;
        ContextPropergationStarted_Sig_t& getContextPropergationStarted_Sig();
        
        Ferrisls();

        void setDisplay( fh_lsdisplay v );
        void setFilterString( const std::string& v );
        void setSortString( const std::string& v );
        void setRecursiveList( gboolean x );
        void setListDirectoryNodeOnly( gboolean x );
        void setForceReadRootDirectoryNodes( gboolean x );
        void setImplyListDirectoryNodeOnly( gboolean x );
        void setDontDescendRegex( const std::string& s );

//        void setContextClass( const std::string& c ) throw( NoSuchContextClass );
//        void setRoot( const std::string& c );
        void setURL( const std::string& earl );
        void operator()( fh_context rctx );
        int operator()();

        void workComplete();

        
        /* For broadcasting disk read update msgs */
        long OnExistsDiskReadContextsProcessed;
        void OnExistsDiskRead( NamingEvent_Exists* ev,
                               const fh_context& subc,
                               std::string olddn, std::string newdn );

        bool hadErrors();

        void setDontFollowLinks( bool v );
        void setDontDecendIntoFiles( bool v );
        void usePreorderTraversal( bool v );
        bool usePreorderTraversal();

        typedef sigc::signal2< void, Ferrisls&, fh_context > EnteringContext_Sig_t;
        EnteringContext_Sig_t& getEnteringContext_Sig();
        typedef sigc::signal2< void, Ferrisls&, fh_context > LeavingContext_Sig_t;
        LeavingContext_Sig_t& getLeavingContext_Sig();
        
    private:
        EnteringContext_Sig_t m_EnteringContext_Sig;
        LeavingContext_Sig_t  m_LeavingContext_Sig;
        
        void MaybeRecurse( fh_context rctx );

        bool PreorderTraversal;
        bool DontFollowLinks;
        bool DontDecendIntoFiles;
        
        void hadErrors( bool v );
        
        DiskReadStarted_Sig_t DiskReadStarted_Sig;
        DiskReadProgress_Sig_t DiskReadProgress_Sig;
        DiskReadDone_Sig_t DiskReadDone_Sig;
        FilterStarted_Sig_t FilterStarted_Sig;
        SortStarted_Sig_t SortStarted_Sig;
        ContextPropergationStarted_Sig_t ContextPropergationStarted_Sig;

        std::string DontDescendRegexString;
        fh_rex DontDescendRegex;
};


    FERRIS_SMARTPTR( Ferrisls, fh_ls );
    
};


#endif
