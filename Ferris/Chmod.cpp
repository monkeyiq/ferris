/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris
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

    $Id: Chmod.cpp,v 1.3 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * Based on modechange.c from fileutils 4.1 which is GPL and 
 * Written by David MacKenzie <djm@ai.mit.edu> 
 *
 * 
 *
 */
#include "Ferris.hh"
#include "Chmod.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

/* Masks for mode_compile argument. */
# define MODE_MASK_EQUALS 1
# define MODE_MASK_PLUS 2
# define MODE_MASK_MINUS 4
# define MODE_MASK_ALL (MODE_MASK_EQUALS | MODE_MASK_PLUS | MODE_MASK_MINUS)

/* The traditional octal values corresponding to each mode bit.  */
#define SUID 04000
#define SGID 02000
#define SVTX 01000
#define RUSR 00400
#define WUSR 00200
#define XUSR 00100
#define RGRP 00040
#define WGRP 00020
#define XGRP 00010
#define ROTH 00004
#define WOTH 00002
#define XOTH 00001
#define ALLM 07777 /* all octal mode bits */


/* All the mode bits that can be affected by chmod.  */
#define CHMOD_MODE_BITS \
  (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO)

// anything that MakeInitializationMode() should support in octal
#define MakeInitializationMode_ALLPERMS ALLPERMS // perhaps change to ? 0177777

namespace Ferris
{
    
    ChmodOperation::ChmodOperation()
            :
            mode( 0 ),
            XifAnyX( false ),
            CopyExisting( false ),
            AffectedBits( CHMOD_MODE_BITS )
    {
    }
    
    void ChmodOperation::setMode( mode_t v )
    {
        mode = v;
    }

    void ChmodOperation::setModeXifAnyX( bool v )
    {
        XifAnyX = v;
    }

    void ChmodOperation::setCopyExisting( bool v )
    {
        CopyExisting = v;
    }

    void ChmodOperation::setAffectedBits( long v )
    {
        AffectedBits = v;
    }
        

    mode_t
    ChmodOperation::calcMode( mode_t existingmode, mode_t chained )
    {
        mode_t value = 0;
                
        chained = chained & CHMOD_MODE_BITS;
                
        if( CopyExisting )
        {
            /* Isolate in `value' the bits in `chained' to copy, given in
               the mask `mode'. */
            value = chained & mode;

            if ( mode & S_IRWXU )
                /* Copy `u' permissions onto `g' and `o'. */
                value |= ((value & S_IRUSR ? S_IRGRP | S_IROTH : 0)
                          | (value & S_IWUSR ? S_IWGRP | S_IROTH : 0)
                          | (value & S_IXUSR ? S_IXGRP | S_IXOTH : 0));
            else if ( mode & S_IRWXG )
                /* Copy `g' permissions onto `u' and `o'. */
                value |= ((value & S_IRGRP ? S_IRUSR | S_IROTH : 0)
                          | (value & S_IWGRP ? S_IWUSR | S_IROTH : 0)
                          | (value & S_IXGRP ? S_IXUSR | S_IXOTH : 0));
            else
                /* Copy `o' permissions onto `u' and `g'. */
                value |= ((value & S_IROTH ? S_IRUSR | S_IRGRP : 0)
                          | (value & S_IWOTH ? S_IWUSR | S_IRGRP : 0)
                          | (value & S_IXOTH ? S_IXUSR | S_IXGRP : 0));

            /* In order to change only `u', `g', or `o' permissions,
               or some combination thereof, clear unselected bits.
               This cannot be done in mode_compile because the value
               to which the `changes->affected' mask is applied depends
               on the old mode of each file. */
            value &= AffectedBits;
                    
        }
        else
        {
            value = mode;
            /* If `X', do not affect the execute bits if the file is not a
               directory and no execute bits are already set. */
            if ( XifAnyX
                 && !S_ISDIR (existingmode)
                 && (chained & (S_IXUSR | S_IXGRP | S_IXOTH)) == 0)
                /* Clear the execute bits. */
                value &= ~ (S_IXUSR | S_IXGRP | S_IXOTH);
        }
        return value;
    }
    
    
    /********************************************************************************/
    
    struct FERRISEXP_DLLLOCAL ChmodOperationEquals : public ChmodOperation
    {
        ChmodOperationEquals( mode_t v = 0 )
            {
                mode = v;
            }
        
        virtual mode_t apply( mode_t existing, mode_t chained )
            {
                mode_t value = calcMode( existing, chained );
                return (chained & ~AffectedBits) | value;
            }
    };

    struct FERRISEXP_DLLLOCAL ChmodOperationPlus : public ChmodOperation
    {
        ChmodOperationPlus( mode_t v = 0 )
            {
                mode = v;
            }
        
        virtual mode_t apply( mode_t existing, mode_t chained )
            {
                mode_t value = calcMode( existing, chained );
                return chained |= value;
            }
    };

    struct FERRISEXP_DLLLOCAL ChmodOperationMinus : public ChmodOperation
    {
        ChmodOperationMinus( mode_t v = 0 )
            {
                mode = v;
            }
        
        virtual mode_t apply( mode_t existing, mode_t chained )
            {
                mode_t value = calcMode( existing, chained );
                return chained &= ~value;
            }
    };
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    Chmod::~Chmod()
    {
        for( operations_t::iterator iter = operations.begin(); iter != operations.end(); )
        {
            ChmodOperation* op = *iter;
            ++iter;
            delete op;
        }
    }
    
    mode_t
    Chmod::operator()( mode_t m )
    {
        mode_t chained = m;
        
        for( operations_t::iterator iter = operations.begin();
             iter != operations.end(); ++iter )
        {
            chained = (*iter)->apply( m, chained );
        }

        return chained;
    }

    mode_t
    Chmod::getInitializationMode()
    {
        return operator()( 0 );
    }
    
    void
    Chmod::append( ChmodOperation* o )
    {
        operations.push_back( o );
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Parse ugoa part and move string along to after all effected bits chars
     */
    static mode_t parseAffectedBitsString( fh_iostream& ss )
    {
        mode_t ret = 0;
        char c;

        while( ss >> c )
        {
//            cerr << " parseAffectedBitsString() c:" << c << endl;
            switch( c )
            {
            case ',':
                break;
            case 'u':
                ret |= S_ISUID | S_IRWXU;
                break;
            case 'g':
                ret |= S_ISGID | S_IRWXG;
                break;
            case 'o':
                ret |= S_ISVTX | S_IRWXO;
                break;
            case 'a':
                ret |= CHMOD_MODE_BITS;
                break;
            default:
                ss.unget();
                return ret;
            }
        }
        return ret;
    }


    static ChmodOperation* makeOp( char opcode, mode_t v )
    {
        switch( opcode )
        {
        case '=': return( new ChmodOperationEquals( v ) );
        case '+': return( new ChmodOperationPlus( v )   );
        case '-': return( new ChmodOperationMinus( v )  );
        }
    }
    
    namespace Factory
    {
        /**
         * Parse a ugoa+-rwx style string or a string with 644 octal
         * to a compiled object that can perform the described actions.
         */
        fh_chmod MakeChmod( const std::string& s, long masked_ops )
        {
            fh_chmod ret = new Chmod();

//            cerr << "MakeChmod() s:" << s << " isNumber:" << isNumber( s ) << endl;
            
            /*
             * Test for plain octal first
             */
            if( isNumber( s ) )
            {
//                long octal_value = toType<long>( s );
                long octal_value = 0;
                {
                    fh_stringstream ss;
                    ss << s;
                    ss >> oct >> octal_value;
                }

                octal_value &= (~S_IFMT);

                if( octal_value != (octal_value & MakeInitializationMode_ALLPERMS ))
                {
                    fh_stringstream ss;
                    ss << "Invalid octal mode string given:" << s
                       << " contains bits outside of valid range valid range:"
                       << oct << MakeInitializationMode_ALLPERMS << dec << endl;
                    Throw_InvalidModeString( tostr(ss), 0 );
                }

                /* Help the compiler optimize the usual case where mode_t uses
                   the traditional octal representation.  */
                mode_t mode = 0;
                mode = ((S_ISUID == SUID && S_ISGID == SGID && S_ISVTX == SVTX
                         && S_IRUSR == RUSR && S_IWUSR == WUSR && S_IXUSR == XUSR
                         && S_IRGRP == RGRP && S_IWGRP == WGRP && S_IXGRP == XGRP
                         && S_IROTH == ROTH && S_IWOTH == WOTH && S_IXOTH == XOTH)
                        ? octal_value
                        : ((octal_value & SUID ? S_ISUID : 0)
                           | (octal_value & SGID ? S_ISGID : 0)
                           | (octal_value & SVTX ? S_ISVTX : 0)
                           | (octal_value & RUSR ? S_IRUSR : 0)
                           | (octal_value & WUSR ? S_IWUSR : 0)
                           | (octal_value & XUSR ? S_IXUSR : 0)
                           | (octal_value & RGRP ? S_IRGRP : 0)
                           | (octal_value & WGRP ? S_IWGRP : 0)
                           | (octal_value & XGRP ? S_IXGRP : 0)
                           | (octal_value & ROTH ? S_IROTH : 0)
                           | (octal_value & WOTH ? S_IWOTH : 0)
                           | (octal_value & XOTH ? S_IXOTH : 0)));

                ret->append( new ChmodOperationEquals( mode ) );
//                 cerr << "MakeChmod() s:" << s << " isNumber:" << isNumber( s )
//                      << " octal_value:" << oct << octal_value << dec
//                      << " mode:" << mode
//                      << " u+rwx:" << (S_IRWXU)
//                      << " ug+rwx:" << (S_IRWXU|S_IRWXG)
//                      << endl;
                
                return ret;
            }

            /*
             * Must be of the form "ugoa...=+-rwxXstugo...[=+-rwxXstugo...]".
             */
            mode_t umask_value = umask (0);
            umask (umask_value);		/* Restore the old value. */

            fh_stringstream ss;
            ss << s;

            while( ss.good() )
            {
                
                /* Operators to actually use umask on. */
                long ops_to_mask = 0;
            
                mode_t affectedBitmask = parseAffectedBitsString( ss );
                bool userSpecifiedUGOABits = affectedBitmask > 0;
                if( !affectedBitmask )
                {
                    cerr << "invalid affectedBitmask, setting to default" << endl;
                    affectedBitmask = CHMOD_MODE_BITS;
                    ops_to_mask     = masked_ops;
                }
            

                set< char > opcodes;
                opcodes.insert( '=' );
                opcodes.insert( '+' );
                opcodes.insert( '-' );

                char opcode = 0;
                if( ss >> opcode && opcodes.count( opcode ) )
                {
                    /* Per the Single Unix Spec, if `who' is not specified and the
                       `=' operator is used, then clear all the bits first.  */
                    if (!userSpecifiedUGOABits &&
                        ops_to_mask & (opcode == '=' ? MODE_MASK_EQUALS : 0))
                    {
                        ret->append( new ChmodOperationEquals( 0 ) );
                    }

                    if (ops_to_mask & ( opcode == '=' ? MODE_MASK_EQUALS
                                        : opcode == '+' ? MODE_MASK_PLUS
                                        : MODE_MASK_MINUS))
                        affectedBitmask &= ~umask_value;

                    mode_t value = 0;

//                     cerr << "MakeChmod()"
//                          << " opcode:" << opcode
//                          << " affectedBitmask:" << affectedBitmask
//                          << endl;

                    char ch = 0;
                    bool gettingValues = true;
                    while( gettingValues && ss >> ch )
                    {
                        ChmodOperation* op = 0;
                    
                        switch( ch )
                        {
                        case 'r':
                            op = makeOp( opcode,
                                         ((S_IRUSR | S_IRGRP | S_IROTH) & affectedBitmask));
                            break;
                        case 'w':
                            op = makeOp( opcode,
                                         ((S_IWUSR | S_IWGRP | S_IWOTH) & affectedBitmask));
                            break;
                        case 'x':
                            op = makeOp( opcode,
                                         ((S_IXUSR | S_IXGRP | S_IXOTH) & affectedBitmask));
                            break;

                        case 'X':
                            op = makeOp(
                                opcode, ((S_IXUSR | S_IXGRP | S_IXOTH) & affectedBitmask));
                            op->setModeXifAnyX( true );
                            break;
                        
                        case 's':
                            /* Set the setuid/gid bits if `u' or `g' is selected. */
                            op = makeOp( opcode,
                                         (S_ISUID | S_ISGID) & affectedBitmask);
                            break;
                        case 't':
                            /* Set the "save text image" bit if `o' is selected. */
                            op = makeOp( opcode,
                                         S_ISVTX & affectedBitmask );
                            break;
                        case 'u':
                            /* Set the affected bits to the value of the `u' bits
                               on the same file.  */
                            op = makeOp( opcode, S_IRWXU );
                            op->setCopyExisting( true );
                            break;
                        case 'g':
                            /* Set the affected bits to the value of the `g' bits
                               on the same file.  */
                            op = makeOp( opcode, S_IRWXG );
                            op->setCopyExisting( true );
                            break;
                        case 'o':
                            /* Set the affected bits to the value of the `o' bits
                               on the same file.  */
                            op = makeOp( opcode, S_IRWXO );
                            op->setCopyExisting( true );
                            break;
                        default:
                            gettingValues = false;
                        }

                        if( op )
                        {
                            op->setAffectedBits( affectedBitmask );
                            ret->append( op );
                        }
                    }
                    if( ss.good() && ch != ',' )
                    {
                        fh_stringstream ss;
                        ss << "Invalid octal mode string given:" << s
                           << " Read specifier opcode values but more data follows instead of comma"
                           << endl;
                        Throw_InvalidModeString( tostr(ss), 0 );
                    }
                }
            }
        }

        mode_t MakeInitializationMode( const std::string& s, long masked_ops )
        {
            fh_chmod c = MakeChmod( s, masked_ops );
            mode_t ret = c->getInitializationMode();

//             cerr << "MakeInitializationMode() s:" << s
//                  << " ret.oct:" << oct << ret << dec << endl;
            
            return ret;
        }
            
        
    };
};
