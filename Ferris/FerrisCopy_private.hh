/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris cp
    Copyright (C) 2002 Ben Martin

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

    $Id: FerrisCopy_private.hh,v 1.5 2010/09/24 21:30:35 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_COPY_PRIVATE_H_
#define _ALREADY_INCLUDED_FERRIS_COPY_PRIVATE_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/FerrisCopy.hh>
#include <Ferris/FilteredContext_private.hh>

#include <vector>
#include <string>

namespace Ferris
{

class FERRISEXP_DLLLOCAL ProgressMeter_TTY
    : public Handlable
{
    std::streamsize Min;
    std::streamsize Max;
    std::streamsize Pos;
    std::streamsize ConsoleWidth;
    std::streamsize ConsolePosition;
    
public:
    
    ProgressMeter_TTY()
    {
        reset();
    }

    void update( std::streamsize newpos )
        {
            Pos = newpos;
            double Percent = 1.0 * Pos / Max;
            double ConsolePercent = Percent * ConsoleWidth;
            std::streamsize NewConsolePosition = static_cast<std::streamsize>(ConsolePercent);

//                 LG_COPY_D << " NewConsolePosition:" << NewConsolePosition
//                      << " Pos:" << Pos
//                      << " Max:" << Max
//                      << " ConsoleWidth:" << ConsoleWidth
//                      << " Percent:" << Percent
//                      << " ConsolePosition:" << ConsolePosition
//                      << " ConsolePercent:" << ConsolePercent
//                      << " diff:" << (NewConsolePosition - ConsolePosition)
//                      << std::endl;
            
            if( NewConsolePosition != ConsolePosition )
            {
                std::streamsize diff = NewConsolePosition - ConsolePosition;

                for( ; diff > 0 ; --diff )
                {
                    std::cout << "#" << std::flush;
                }
                ConsolePosition = NewConsolePosition;
            }
        }

    void done()
        {
            std::cout << std::endl;
        }

    void reset( std::streamsize NewMax = 100 )
        {
            ConsoleWidth = 80;
            if( const gchar* p = g_getenv ("COLUMNS") )
            {
                ConsoleWidth = toType<std::streamsize>(p);
            }
            ConsolePosition = 0;
            Max = NewMax;
            Min = 0;
            Pos = 0;

            if( !Max )
                Max = 1;
        }
};

    
    class FerrisCopy_TTY;
    FERRIS_SMARTPTR( FerrisCopy_TTY, fh_cp_tty );


class FERRISEXP_DLLLOCAL FerrisCopy_TTY
    :
    public FerrisCopy
{
    typedef FerrisCopy _Base;
    typedef FerrisCopy_TTY _Self;
    ProgressMeter_TTY Progress;
    bool ShowMeter;

    typedef std::vector<fh_matcher> MatchersList_t;
    MatchersList_t AlwaysPermitMatchers;

    char askQ( std::string prompt )
        {
            LG_COPY_D << "askQ() prompt:" << prompt << std::endl;
            char ch='n';
            std::cout << prompt << std::flush;
            std::cin >> ch;
            LG_COPY_D << "askQ() reply ch:" << ch << " for prompt:" << prompt << std::endl;
            return ch;
        }

    char AskLoop( std::string prompt, int MaxNumericOption = -1 )
        {
            while( char ch = askQ( prompt ))
            {
                switch( ch )
                {
                case 'a':
                case 'A':
                    setInteractive( false );
                    return true;

                case 'n':
                case 'N':
                    return false;

                case 'p':
                case 'P':
                    alwaysPreserveExisting = true;
                    return false;
                    
                case 'S':
                case 's':
                    std::cout << "User requested to stop" << std::endl;
                    exit(0);

                case 'y':
                case 'Y':
                    return true;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    int c = ch - (int)'0';
                    if( c > MaxNumericOption )
                        continue;
                    return ch;
                }
                }
            }
            return true;
        }
    
    

    bool alwaysPreserveExisting;
    
public:

    FerrisCopy_TTY()
        :
        ShowMeter( false ),
        alwaysPreserveExisting( false )
        {
            getCopyStartSignal().connect( sigc::mem_fun( *this,    &_Self::OnCopyStart ));
            getCopyPorgressSignal().connect( sigc::mem_fun( *this, &_Self::OnCopyPorgress ));
            getCopyEndSignal().connect( sigc::mem_fun( *this,      &_Self::OnCopyEnd ));
            getCopyVerboseSignal().connect( sigc::mem_fun( *this,  &_Self::OnCopyVerbose ));
            getSkippingContextSignal().connect( sigc::mem_fun( *this,  &_Self::OnSkippingContext ));
            getAskReplaceContextSignal().connect( sigc::mem_fun( *this,&_Self::OnAskReplaceContext));
            getAskReplaceAttributeSignal().connect( sigc::mem_fun( *this,&_Self::OnAskReplaceAttribute));
        }

    virtual ~FerrisCopy_TTY() throw ()
        {
        }
    
    void setShowMeter( bool v )
        {
            ShowMeter = v;
        }
    
    void OnCopyStart( FerrisCopy& thisobj,
                      std::streamsize CurrentPosition,
                      std::streamsize BlockSize,
                      std::streamsize FinalSize )
        {
            if( !ShowMeter ) return;
            
            if( FinalSize != -1 )
                Progress.reset( FinalSize );
        }

    void OnCopyPorgress( FerrisCopy& thisobj,
                      std::streamsize CurrentPosition,
                      std::streamsize BlockSize,
                      std::streamsize FinalSize )
        {
            if( !ShowMeter ) return;

            if( FinalSize != -1 )
                Progress.update( CurrentPosition );
        }
    
    void OnCopyEnd( FerrisCopy& thisobj,
                      std::streamsize CurrentPosition,
                      std::streamsize BlockSize,
                      std::streamsize FinalSize )
        {
            if( !ShowMeter ) return;

            Progress.done();
        }
    

    void OnCopyVerbose( FerrisCopy& thisobj,
                        fh_context src,
                        fh_context dst,
                        std::string srcDescription,
                        std::string dstDescription )
        {
            std::cout << "cp "  << getSourceDescription()
                 << " to " << getDestinationDescription() << std::endl;
        }

    void OnSkippingContext( FerrisCopy& thisobj, std::string srcDescription, std::string reason )
        {
            std::cerr << reason << ":" << srcDescription << std::endl;
        }

    
    bool OnAskReplaceContext( FerrisCopy& thisobj,
                              fh_context src,
                              fh_context dst,
                              std::string srcDescription,
                              std::string dstDescription )
        {
//             std::cerr << "OnAskReplaceContext() "
//                  << " srcdesc:" << srcDescription
//                  << " dstdesc:" << dstDescription
//                  << std::endl;

            if( alwaysPreserveExisting )
            {
                return false;
            }

            if( !AlwaysPermitMatchers.empty() )
            {
                for( MatchersList_t::iterator iter = AlwaysPermitMatchers.begin();
                     iter != AlwaysPermitMatchers.end(); ++iter )
                {
                    fh_matcher m = *iter;
                    if( m( dst ) )
                    {
                        return true;
                    }
                }
            }
            
            
            fh_stringstream pss;
            pss << "Replace " << dstDescription << std::endl
                << " (a)lways, (n)o, (p)reserve all existing, (s)top everything, (y)es" << std::endl;

            int MaxNumericOption = -1;
            std::string extension = getStrAttr( dst, "name-extension", "" );
            MaxNumericOption++;
            pss << "Always for: (0)-with extension:" << extension;

            std::string mimetype = getStrAttr( dst, "mimetype", "" );
            MaxNumericOption++;
            pss << " (1)-with mimetype:" << mimetype;

            std::string filetype = getStrAttr( dst, "filetype", "" );
            MaxNumericOption++;
            pss << " (2)-with filetype:" << filetype;

            

            pss << "? " << std::flush;
            char ch = AskLoop( tostr(pss), 1 );
            if( ch == '0' )
            {
                static Factory::EndingList r;
                r.push_back( make_pair( std::string("name-extension"), std::string( extension )));
                fh_matcher m = Factory::ComposeEqualsMatcher( r );
                AlwaysPermitMatchers.push_back( m );
            }
            else if( ch == '1' )
            {
                static Factory::EndingList r;
                r.push_back( make_pair( std::string("mimetype"), std::string( mimetype )));
                fh_matcher m = Factory::ComposeEqualsMatcher( r );
                AlwaysPermitMatchers.push_back( m );
            }
            else if( ch == '2' )
            {
                static Factory::EndingList r;
                r.push_back( make_pair( std::string("filetype"), std::string( filetype )));
                fh_matcher m = Factory::ComposeEqualsMatcher( r );
                AlwaysPermitMatchers.push_back( m );
            }
            return ch;
        }

    bool OnAskReplaceAttribute( FerrisCopy& thisobj,
                                fh_context src,
                                fh_context dst,
                                std::string srcDescription,
                                std::string dstDescription,
                                fh_attribute dstattr )
        {
            if( alwaysPreserveExisting )
            {
                return false;
            }
            
            fh_stringstream pss;
            pss << "Replace " << dstDescription << std::endl
                << " (a)lways, (n)o, (p)reserve all existing, (s)top everything, (y)es? " << std::flush;
            return AskLoop( tostr(pss) );
        }
    
};

    
};


#endif

