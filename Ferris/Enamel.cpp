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

    $Id: Enamel.cpp,v 1.48 2011/10/22 21:30:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris.hh>
#include <Ferris_private.hh>
#include <Enamel.hh>
#include <Enamel_priv.hh>
#include <General.hh>
#include <list>
#include <string>


#include <FerrisPopt.hh>
// #ifdef HAVE_EDB
// /*
//  * We need the raw interface here because we are logging the very baseline of
//  * context functionality, thus we can not use contexts in loading/saving of
//  * loggging metadata.
//  */
// #include <Edb.h>
// #endif

#include <General.hh>


using namespace std;

namespace Ferris
{
    

    namespace Enamel 
    {

        Timber& get__t_l0em()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L0, Timber::_SBufT::PRI_EMERG);
            return x;
        }
    
        Timber& get__t_l0a()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L0, Timber::_SBufT::PRI_ALERT);
            return x;
        }

        Timber& get__t_l0er()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L0, Timber::_SBufT::PRI_ERR);
            return x;
        }

        Timber& get__t_l0w()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L0, Timber::_SBufT::PRI_WARNING);
            return x;
        }

    
        Timber& get__t_l0n()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L0, Timber::_SBufT::PRI_NOTICE);
            return x;
        }
    
        Timber& get__t_l0i()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L0, Timber::_SBufT::PRI_INFO);
            return x;
        }
    
        Timber& get__t_l0d()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L0, Timber::_SBufT::PRI_DEBUG);
            return x;
        }


        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////


        Timber& get__t_l1em()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L1, Timber::_SBufT::PRI_EMERG);
            return x;
        }
    
        Timber& get__t_l1a()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L1, Timber::_SBufT::PRI_ALERT);
            return x;
        }

        Timber& get__t_l1er()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L1, Timber::_SBufT::PRI_ERR);
            return x;
        }

        Timber& get__t_l1w()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L1, Timber::_SBufT::PRI_WARNING);
            return x;
        }

    
        Timber& get__t_l1n()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L1, Timber::_SBufT::PRI_NOTICE);
            return x;
        }
    
        Timber& get__t_l1i()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L1, Timber::_SBufT::PRI_INFO);
            return x;
        }
    
        Timber& get__t_l1d()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L1, Timber::_SBufT::PRI_DEBUG);
            return x;
        }


        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////


        Timber& get__t_l2em()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L2, Timber::_SBufT::PRI_EMERG);
            return x;
        }
    
        Timber& get__t_l2a()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L2, Timber::_SBufT::PRI_ALERT);
            return x;
        }

        Timber& get__t_l2er()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L2, Timber::_SBufT::PRI_ERR);
            return x;
        }

        Timber& get__t_l2w()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L2, Timber::_SBufT::PRI_WARNING);
            return x;
        }

    
        Timber& get__t_l2n()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L2, Timber::_SBufT::PRI_NOTICE);
            return x;
        }
    
        Timber& get__t_l2i()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L2, Timber::_SBufT::PRI_INFO);
            return x;
        }
    
        Timber& get__t_l2d()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L2, Timber::_SBufT::PRI_DEBUG);
            return x;
        }
    
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////


        Timber& get__t_l3em()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L3, Timber::_SBufT::PRI_EMERG);
            return x;
        }
    
        Timber& get__t_l3a()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L3, Timber::_SBufT::PRI_ALERT);
            return x;
        }

        Timber& get__t_l3er()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L3, Timber::_SBufT::PRI_ERR);
            return x;
        }

        Timber& get__t_l3w()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L3, Timber::_SBufT::PRI_WARNING);
            return x;
        }

    
        Timber& get__t_l3n()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L3, Timber::_SBufT::PRI_NOTICE);
            return x;
        }
    
        Timber& get__t_l3i()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L3, Timber::_SBufT::PRI_INFO);
            return x;
        }
    
        Timber& get__t_l3d()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L3, Timber::_SBufT::PRI_DEBUG);
            return x;
        }
    

        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////


        Timber& get__t_l4em()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L4, Timber::_SBufT::PRI_EMERG);
            return x;
        }
    
        Timber& get__t_l4a()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L4, Timber::_SBufT::PRI_ALERT);
            return x;
        }

        Timber& get__t_l4er()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L4, Timber::_SBufT::PRI_ERR);
            return x;
        }

        Timber& get__t_l4w()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L4, Timber::_SBufT::PRI_WARNING);
            return x;
        }

    
        Timber& get__t_l4n()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L4, Timber::_SBufT::PRI_NOTICE);
            return x;
        }
    
        Timber& get__t_l4i()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L4, Timber::_SBufT::PRI_INFO);
            return x;
        }
    
        Timber& get__t_l4d()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L4, Timber::_SBufT::PRI_DEBUG);
            return x;
        }
    

        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////


        Timber& get__t_l5em()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L5, Timber::_SBufT::PRI_EMERG);
            return x;
        }
    
        Timber& get__t_l5a()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L5, Timber::_SBufT::PRI_ALERT);
            return x;
        }

        Timber& get__t_l5er()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L5, Timber::_SBufT::PRI_ERR);
            return x;
        }

        Timber& get__t_l5w()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L5, Timber::_SBufT::PRI_WARNING);
            return x;
        }

    
        Timber& get__t_l5n()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L5, Timber::_SBufT::PRI_NOTICE);
            return x;
        }
    
        Timber& get__t_l5i()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L5, Timber::_SBufT::PRI_INFO);
            return x;
        }
    
        Timber& get__t_l5d()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L5, Timber::_SBufT::PRI_DEBUG);
            return x;
        }

        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////


        Timber& get__t_l6em()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L6, Timber::_SBufT::PRI_EMERG);
            return x;
        }
    
        Timber& get__t_l6a()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L6, Timber::_SBufT::PRI_ALERT);
            return x;
        }

        Timber& get__t_l6er()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L6, Timber::_SBufT::PRI_ERR);
            return x;
        }

        Timber& get__t_l6w()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L6, Timber::_SBufT::PRI_WARNING);
            return x;
        }

    
        Timber& get__t_l6n()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L6, Timber::_SBufT::PRI_NOTICE);
            return x;
        }
    
        Timber& get__t_l6i()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L6, Timber::_SBufT::PRI_INFO);
            return x;
        }
    
        Timber& get__t_l6d()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L6, Timber::_SBufT::PRI_DEBUG);
            return x;
        }

        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////


        Timber& get__t_l7em()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L7, Timber::_SBufT::PRI_EMERG);
            return x;
        }
    
        Timber& get__t_l7a()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L7, Timber::_SBufT::PRI_ALERT);
            return x;
        }

        Timber& get__t_l7er()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L7, Timber::_SBufT::PRI_ERR);
            return x;
        }

        Timber& get__t_l7w()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L7, Timber::_SBufT::PRI_WARNING);
            return x;
        }

    
        Timber& get__t_l7n()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L7, Timber::_SBufT::PRI_NOTICE);
            return x;
        }
    
        Timber& get__t_l7i()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L7, Timber::_SBufT::PRI_INFO);
            return x;
        }
    
        Timber& get__t_l7d()
        {
            static Timber x( "libferris.so", Timber::_SBufT::OPT_PID, Timber::_SBufT::FAC_L7, Timber::_SBufT::PRI_DEBUG);
            return x;
        }
    
    
    
    
    };

    namespace Logging
    {
        bool operator<( const realStreamsKey& k1, const realStreamsKey& k2 )
        {
            if( k1.F == k2.F )
                return k1.S < k2.S;
            return k1.F < k2.F;
        }

        bool operator==( const realStreamsKey& k1, const realStreamsKey& k2 )
        {
            return ( k1.F == k2.F && k1.S == k2.S );
        }
    
        string
        LogStateBase::getName()
        {
            return "base";
        }
    
        state_t
        LogStateBase::getDefaultState()
        {
            int edbstate  = 0;
            string strkey = getName();
            char* key     = (char*)strkey.c_str();

            string v = getEDBString( FDB_LOGGING, strkey, "" );
            if( v.length() )
            {
                std::stringstream ss;
                ss << v;
                ss >> edbstate;
                
//                cerr << "LOGGING n:" << getName() << "  state:" << edbstate << endl;
                
                return state_t(edbstate);
            }

//             BackTrace();
//             cerr << "LOGGING n:" << getName() << " using default state" << endl;
            
            return state_t(
                Timber::_SBufT::PRI_EMERG |
                Timber::_SBufT::PRI_ALERT |
                Timber::_SBufT::PRI_CRIT  |
                Timber::_SBufT::PRI_ERR   |
                0
//                 Timber::_SBufT::PRI_WARNING |
//                 Timber::_SBufT::PRI_NOTICE  |
//                 Timber::_SBufT::PRI_INFO    |
//                 Timber::_SBufT::PRI_DEBUG                      
                );
        }



    
        void
        LogStateBase::setState( state_t v )
        {
            m_state = v;
        }
    
        LogStateBase::LogStateBase()
            :
            m_state( state_t(
                         Timber::_SBufT::PRI_EMERG |
                         Timber::_SBufT::PRI_ALERT |
                         Timber::_SBufT::PRI_CRIT  |
                         Timber::_SBufT::PRI_ERR
                         ))
        {
        }
    
        LogStateBase::~LogStateBase()
        {
        }

        f_ostream&
        LogStateBase::getNullStream()
        {
            static NullStream<> null_stream;
            return null_stream;
        }

        NullStream<>&
        LogStateBase::getNullStreamWithType()
        {
            static NullStream<> null_stream;
            return null_stream;
        }
    

        fh_ostream&
        LogStateBase::getStream( Timber::Facility_t f, state_t d )
        {
            string lfn = getEDBString( FDB_GENERAL, LOGGING_TO_FILENAME_KEY, "" );
            if( !lfn.empty() )
            {
                static fh_fstream fs = fh_fstream( lfn, ios::out | ios::app );
                return fs;
            }
            
            static fh_timber tim( "libferris.so",
                                  Timber::_SBufT::OPT_PID, f, d );

            tim << flush;
            tim.setPriority( d );
            return tim;
            
            

            
// //        cerr << "getStream() " << endl;
// //                cerr << "getStream() f:" << f << " d:" << d << endl;
        
//             realStreamsKey key = realStreamsKey( f, d );
//             realStreams_t::iterator foundobj = realStreams.find( key );
                
//             if( realStreams.end() == foundobj )
//             {
// //            cerr << "getStream() NO KEY! f:" << f << " d:" << d << endl;
            
// //             fh_timber& tim = new Timber("libferris.so",
// //                                        Timber::_SBufT::OPT_PID,
// //                                        f, d);
// //             realStreams[key] = tim;

//                 fh_timber tim( "libferris.so",
//                                Timber::_SBufT::OPT_PID,
//                                f, d);
            

// //             cerr << "About to write to tim...1" << endl;
// //             tim << "test1" << endl;
// //             cerr << "About to write to tim...2" << endl;
// //             cerr << "About to write to tim...1" << endl;
// //             tim << "test1.b" << endl;
// //             cerr << "About to write to tim...2" << endl;
// //             cerr << "About to write to tim...1" << endl;
// //             tim << "test1.c" << endl;
// //             cerr << "About to write to tim...2" << endl;
            
//                 realStreams.insert( realStreams_t::value_type( key, tim ));
//                 //realStreams[key] = tim;

// //             cerr << "About to write to tim...3" << endl;
// //             realStreams[key] << "test2" << endl;
// //             cerr << "About to write to tim...4" << endl;
// //             cerr << "About to write to tim...5" << endl;
// //             tim << "test2.b" << endl;
// //             cerr << "About to write to tim...6" << endl;
            
//                 return realStreams[key];
//             }
// //        cerr << "getStream() FOUND KEY1! f:" << f << " d:" << d << endl;

//             fh_timber& fht = foundobj->second;
// //                cerr << "getStream() FOUND KEY2! fht:" << toVoid(fht) << endl;
                
// //                *GetImpl(fht) << "hello\n";
// //                cerr << "getStream() FOUND KEY3! f:" << f << " d:" << d << endl;

// //        cerr << "getStream() FOUND KEY1! object:" << toVoid(foundobj->second) << endl;
                    
//             return fht;
        }

        void
        LogStateBase::clear()
        {
            m_state = state_t(0);
        }
    
        void
        LogStateBase::reset()
        {
            m_state = getDefaultState();
        }
    
        void
        LogStateBase::unset( state_t s )
        {
//        cerr << "unset() old:" << m_state << endl;
            m_state = state_t( m_state ^ s );
//        cerr << "unset() new:" << m_state << endl;
        }
    
        void
        LogStateBase::add( state_t s )
        {
            m_state = state_t( m_state | s );
        }
    
        void
        LogStateBase::set( state_t s )
        {
            m_state = state_t( s );
        }
    
        state_t&
        LogStateBase::state()
        {
            return m_state;
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
    
//        typedef map< string, LogStateBase* > Loggers_t;
//        Loggers_t Loggers;

        Loggers_t& getLoggers()
        {
            static Loggers_t c;
            static bool virgin = true;

            if( virgin )
            {
                virgin = false;

//            c.insert(make_pair(LG_STRF::Instance().getName(), &LG_STRF::Instance()));
                c.insert(make_pair(LG_STRF::Instance().getName(), &LG_STRF::Instance()));
                c.insert(make_pair(LG_ATTR::Instance().getName(), &LG_ATTR::Instance()));
                c.insert(make_pair(LG_OVERMOUNT::Instance().getName(), &LG_OVERMOUNT::Instance()));
                c.insert(make_pair(LG_SORT::Instance().getName(), &LG_SORT::Instance()));
                c.insert(make_pair(LG_IMAGE::Instance().getName(), &LG_IMAGE::Instance()));
                c.insert(make_pair(LG_PNGIMAGE::Instance().getName(), &LG_PNGIMAGE::Instance()));
                c.insert(make_pair(LG_JASPERIMAGE::Instance().getName(), &LG_JASPERIMAGE::Instance()));
                c.insert(make_pair(LG_JPEGIMAGE::Instance().getName(), &LG_JPEGIMAGE::Instance()));
                c.insert(make_pair(LG_DJVUIMAGE::Instance().getName(), &LG_DJVUIMAGE::Instance()));
                c.insert(make_pair(LG_GIMPIMAGE::Instance().getName(), &LG_GIMPIMAGE::Instance()));
                c.insert(make_pair(LG_MAGIKIMAGE::Instance().getName(), &LG_MAGIKIMAGE::Instance()));
                c.insert(make_pair(LG_IMLIB2IMAGE::Instance().getName(),&LG_IMLIB2IMAGE::Instance()));
                c.insert(make_pair(LG_STRATTR::Instance().getName(), &LG_STRATTR::Instance()));
                c.insert(make_pair(LG_BYTEATTR::Instance().getName(), &LG_BYTEATTR::Instance()));
                c.insert(make_pair(LG_CTX::Instance().getName(), &LG_CTX::Instance()));
                c.insert(make_pair(LG_VM::Instance().getName(), &LG_VM::Instance()));
                c.insert(make_pair(LG_NATIVE::Instance().getName(), &LG_NATIVE::Instance()));
                c.insert(make_pair(LG_FACTORY::Instance().getName(), &LG_FACTORY::Instance()));
                c.insert(make_pair(LG_ID3::Instance().getName(), &LG_ID3::Instance()));
                c.insert(make_pair(LG_MPG::Instance().getName(), &LG_MPG::Instance()));
                c.insert(make_pair(LG_PLUGIN::Instance().getName(), &LG_PLUGIN::Instance()));
                c.insert(make_pair(LG_FFILTER::Instance().getName(), &LG_FFILTER::Instance()));
                c.insert(make_pair(LG_MBOX::Instance().getName(), &LG_MBOX::Instance()));
                c.insert(make_pair(LG_EDB::Instance().getName(), &LG_EDB::Instance()));
                c.insert(make_pair(LG_PATHS::Instance().getName(), &LG_PATHS::Instance()));
                c.insert(make_pair(LG_PCCTS::Instance().getName(), &LG_PCCTS::Instance()));
                c.insert(make_pair(LG_FILTERPARSE::Instance().getName(), &LG_FILTERPARSE::Instance()));
                c.insert(make_pair(LG_XFS::Instance().getName(), &LG_XFS::Instance()));
                c.insert(make_pair(LG_SOCKET::Instance().getName(), &LG_SOCKET::Instance()));
                c.insert(make_pair(LG_SQLDB::Instance().getName(), &LG_SQLDB::Instance()));
                c.insert(make_pair(LG_SQLPLUS::Instance().getName(), &LG_SQLPLUS::Instance()));
                c.insert(make_pair(LG_DTL::Instance().getName(), &LG_DTL::Instance()));
                c.insert(make_pair(LG_CURL::Instance().getName(), &LG_CURL::Instance()));
                c.insert(make_pair(LG_GTKFERRIS::Instance().getName(), &LG_GTKFERRIS::Instance()));
                c.insert(make_pair(LG_FERRISCREATE::Instance().getName(), &LG_FERRISCREATE::Instance()));
                c.insert(make_pair(LG_CREATE::Instance().getName(), &LG_CREATE::Instance()));
                c.insert(make_pair(LG_SCHEMA::Instance().getName(), &LG_SCHEMA::Instance()));
                c.insert(make_pair(LG_EMBLEM::Instance().getName(), &LG_EMBLEM::Instance()));
                c.insert(make_pair(LG_TIME::Instance().getName(), &LG_TIME::Instance()));
                c.insert(make_pair(LG_COPY::Instance().getName(), &LG_COPY::Instance()));
                c.insert(make_pair(LG_MOVE::Instance().getName(), &LG_MOVE::Instance()));
                c.insert(make_pair(LG_RM::Instance().getName(), &LG_RM::Instance()));
                c.insert(make_pair(LG_DVDREAD::Instance().getName(), &LG_DVDREAD::Instance()));
                c.insert(make_pair(LG_IPCCTX::Instance().getName(), &LG_IPCCTX::Instance()));
                c.insert(make_pair(LG_FCLIP::Instance().getName(), &LG_FCLIP::Instance()));
                c.insert(make_pair(LG_DB4::Instance().getName(), &LG_DB4::Instance()));
                c.insert(make_pair(LG_GDBM::Instance().getName(), &LG_GDBM::Instance()));
                c.insert(make_pair(LG_TDB::Instance().getName(), &LG_TDB::Instance()));
                c.insert(make_pair(LG_COMMONDB::Instance().getName(), &LG_COMMONDB::Instance()));
                c.insert(make_pair(LG_EET::Instance().getName(), &LG_EET::Instance()));
                c.insert(make_pair(LG_FNEWS::Instance().getName(), &LG_FNEWS::Instance()));
                c.insert(make_pair(LG_JOURNAL::Instance().getName(), &LG_JOURNAL::Instance()));
                c.insert(make_pair(LG_LDAP::Instance().getName(), &LG_LDAP::Instance()));
                c.insert(make_pair(LG_EXCEPTIONS::Instance().getName(), &LG_EXCEPTIONS::Instance()));
                c.insert(make_pair(LG_FTXLEXI::Instance().getName(), &LG_FTXLEXI::Instance()));
                c.insert(make_pair(LG_IDX::Instance().getName(), &LG_IDX::Instance()));
                c.insert(make_pair(LG_EAIDX::Instance().getName(), &LG_EAIDX::Instance()));
                c.insert(make_pair(LG_IOSTREAM::Instance().getName(), &LG_IOSTREAM::Instance()));
                c.insert(make_pair(LG_XML::Instance().getName(), &LG_XML::Instance()));
                c.insert(make_pair(LG_DOM::Instance().getName(), &LG_DOM::Instance()));
                c.insert(make_pair(LG_EXTFS::Instance().getName(), &LG_EXTFS::Instance()));
                c.insert(make_pair(LG_RUNNER::Instance().getName(), &LG_RUNNER::Instance()));
                c.insert(make_pair(LG_A52::Instance().getName(), &LG_A52::Instance()));
                c.insert(make_pair(LG_MPEG2::Instance().getName(), &LG_MPEG2::Instance()));
                c.insert(make_pair(LG_RDF::Instance().getName(), &LG_RDF::Instance()));
                c.insert(make_pair(LG_EXIF::Instance().getName(), &LG_EXIF::Instance()));
                c.insert(make_pair(LG_CTXREC::Instance().getName(), &LG_CTXREC::Instance()));
                c.insert(make_pair(LG_RTIMEPARSE::Instance().getName(), &LG_RTIMEPARSE::Instance()));
                c.insert(make_pair(LG_LIBEXTRACTOR::Instance().getName(), &LG_LIBEXTRACTOR::Instance()));
                c.insert(make_pair(LG_GPHOTO::Instance().getName(), &LG_GPHOTO::Instance()));
                c.insert(make_pair(LG_FCA::Instance().getName(), &LG_FCA::Instance()));
                c.insert(make_pair(LG_EVO::Instance().getName(), &LG_EVO::Instance()));
                c.insert(make_pair(LG_SAMBA::Instance().getName(), &LG_SAMBA::Instance()));
                c.insert(make_pair(LG_PG::Instance().getName(), &LG_PG::Instance()));
                c.insert(make_pair(LG_BIBTEX::Instance().getName(), &LG_BIBTEX::Instance()));
                c.insert(make_pair(LG_KDE::Instance().getName(), &LG_KDE::Instance()));
                c.insert(make_pair(LG_ANNODEX::Instance().getName(), &LG_ANNODEX::Instance()));
                c.insert(make_pair(LG_ASTEXT::Instance().getName(), &LG_ASTEXT::Instance()));
                c.insert(make_pair(LG_FILEACTIONS::Instance().getName(), &LG_FILEACTIONS::Instance()));
                c.insert(make_pair(LG_EMACS::Instance().getName(), &LG_EMACS::Instance()));
                c.insert(make_pair(LG_FIREFOX::Instance().getName(), &LG_FIREFOX::Instance()));
                c.insert(make_pair(LG_ICONSRV::Instance().getName(), &LG_ICONSRV::Instance()));
                c.insert(make_pair(LG_XINE::Instance().getName(), &LG_XINE::Instance()));
                c.insert(make_pair(LG_OBBY::Instance().getName(), &LG_OBBY::Instance()));
                c.insert(make_pair(LG_XSLTFS::Instance().getName(), &LG_XSLTFS::Instance()));
                c.insert(make_pair(LG_RDFATTRCACHE::Instance().getName(), &LG_RDFATTRCACHE::Instance()));
                c.insert(make_pair(LG_FFUSE::Instance().getName(), &LG_FFUSE::Instance()));
                c.insert(make_pair(LG_BEAGLE::Instance().getName(), &LG_BEAGLE::Instance()));
                c.insert(make_pair(LG_GLOB::Instance().getName(), &LG_GLOB::Instance()));
                c.insert(make_pair(LG_AMAROK::Instance().getName(), &LG_AMAROK::Instance()));
                c.insert(make_pair(LG_XMMS::Instance().getName(), &LG_XMMS::Instance()));
                c.insert(make_pair(LG_PRIVATE::Instance().getName(), &LG_PRIVATE::Instance()));
                c.insert(make_pair(LG_DBUS::Instance().getName(), &LG_DBUS::Instance()));
                c.insert(make_pair(LG_XQILLA::Instance().getName(), &LG_XQILLA::Instance()));
                c.insert(make_pair(LG_WEBPHOTO::Instance().getName(), &LG_WEBPHOTO::Instance()));
                c.insert(make_pair(LG_FSPOT::Instance().getName(), &LG_FSPOT::Instance()));
                c.insert(make_pair(LG_STRIGI::Instance().getName(), &LG_STRIGI::Instance()));
                c.insert(make_pair(LG_MDSERV::Instance().getName(), &LG_MDSERV::Instance()));
                c.insert(make_pair(LG_VOLMAN::Instance().getName(), &LG_VOLMAN::Instance()));
                c.insert(make_pair(LG_HAL::Instance().getName(), &LG_HAL::Instance()));
                c.insert(make_pair(LG_EGO::Instance().getName(), &LG_EGO::Instance()));
                c.insert(make_pair(LG_BGPROC::Instance().getName(), &LG_BGPROC::Instance()));
                c.insert(make_pair(LG_FLAC::Instance().getName(), &LG_FLAC::Instance()));
                c.insert(make_pair(LG_OGGZ::Instance().getName(), &LG_OGGZ::Instance()));
                c.insert(make_pair(LG_TAGLIB::Instance().getName(), &LG_TAGLIB::Instance()));
                c.insert(make_pair(LG_FFIND::Instance().getName(), &LG_FFIND::Instance()));
                c.insert(make_pair(LG_QTSQL::Instance().getName(), &LG_QTSQL::Instance()));
                c.insert(make_pair(LG_SQLITE::Instance().getName(), &LG_SQLITE::Instance()));
                c.insert(make_pair(LG_RECORDFILE::Instance().getName(), &LG_RECORDFILE::Instance()));
                c.insert(make_pair(LG_GOOGLE::Instance().getName(), &LG_GOOGLE::Instance()));
                c.insert(make_pair(LG_BOXCOM::Instance().getName(), &LG_BOXCOM::Instance()));
                c.insert(make_pair(LG_FACEBOOK::Instance().getName(), &LG_FACEBOOK::Instance()));
                c.insert(make_pair(LG_VIMEO::Instance().getName(), &LG_VIMEO::Instance()));
                c.insert(make_pair(LG_IDENTICA::Instance().getName(), &LG_IDENTICA::Instance()));
                c.insert(make_pair(LG_ZONEMINDER::Instance().getName(), &LG_ZONEMINDER::Instance()));
                c.insert(make_pair(LG_FERRISREST::Instance().getName(), &LG_FERRISREST::Instance()));
                c.insert(make_pair(LG_KIO::Instance().getName(), &LG_KIO::Instance()));
                c.insert(make_pair(LG_GDRIVE::Instance().getName(), &LG_GDRIVE::Instance()));
                c.insert(make_pair(LG_QIO::Instance().getName(), &LG_QIO::Instance()));
                c.insert(make_pair(LG_WEBSERVICE::Instance().getName(), &LG_WEBSERVICE::Instance()));
                c.insert(make_pair(LG_GSTREAMER::Instance().getName(), &LG_GSTREAMER::Instance()));
                c.insert(make_pair(LG_UPNP::Instance().getName(), &LG_UPNP::Instance()));
                c.insert(make_pair(LG_USEROVERLAY::Instance().getName(), &LG_USEROVERLAY::Instance()));
                c.insert(make_pair(LG_PLASMA::Instance().getName(), &LG_PLASMA::Instance()));
                c.insert(make_pair(LG_SANE::Instance().getName(), &LG_SANE::Instance()));
                c.insert(make_pair(LG_PRINTER::Instance().getName(), &LG_PRINTER::Instance()));
                c.insert(make_pair(LG_PULSEAUDIO::Instance().getName(), &LG_PULSEAUDIO::Instance()));
                c.insert(make_pair(LG_WIKI::Instance().getName(), &LG_WIKI::Instance()));
                c.insert(make_pair(LG_SPIRITCONTEXT::Instance().getName(), &LG_SPIRITCONTEXT::Instance()));
                c.insert(make_pair(LG_PASTEBIN::Instance().getName(), &LG_PASTEBIN::Instance()));
                c.insert(make_pair(LG_LIBMEDIAINFO::Instance().getName(), &LG_LIBMEDIAINFO::Instance()));
                c.insert(make_pair(LG_SUBTITLES::Instance().getName(), &LG_SUBTITLES::Instance()));
                c.insert(make_pair(LG_FERRISQTGUI::Instance().getName(), &LG_FERRISQTGUI::Instance()));
            }
            return c;
        }

//     namespace Private
//     {
//         class LoggCreator
//         {
//         public:
//             LoggCreator()
//                 {
//                     cout << "LoggCreator: getting loggers" << endl;
                    
//                     Loggers_t& l =  ::Logging::getLoggers();
//                 }
//         };

//         static LoggCreator LoggCreatorObj;
//     };
    
    


        const string DISABLE_ALL_LOGGING = "ferris-logging-off";
        template <class T>
        class PopTableCollector : public basic_PopTableCollector
        {
        public:

            virtual void poptCallback(poptContext con,
                                      enum poptCallbackReason reason,
                                      const struct poptOption * opt,
                                      const char * arg,
                                      const void * data)
                {
                    const string key = opt->longName;

                    if( key == DISABLE_ALL_LOGGING )
                    {
                        for( Loggers_t::const_iterator iter = getLoggers().begin();
                             iter != getLoggers().end();
                             iter++ )
                        {
                            iter->second->clear();
                        }
                        return;
                    }
        
                    string argstr(arg);
                    int argi;
        
//                 cerr << "Logging::poptCallbackType()  opt " << opt << endl;
//                 cerr << "Logging::poptCallbackType()  name:" << opt->longName  << endl;
//                 cerr << "Logging::poptCallbackType() arg:" << arg << endl;

                    Loggers_t::const_iterator iter = getLoggers().find(key);
                    istringstream ss(argstr);
                    ss >> argi;
        
                    if( iter != getLoggers().end() )
                    {
                        iter->second->set( Timber::Priority_t(argi) );
                    }
                }


        
            struct ::poptOption* getTable(
                const string& desc,
                const list<string>& names )
                {
                    int extraArgInfo = 0;
                    
                    if( !isTrue(
                            getEDBString( FDB_GENERAL,
                                          SHOW_LOGGING_POPT_OPTIONS_BY_DEFAULT, "0" )))
                    {
                        extraArgInfo |= POPT_ARGFLAG_DOC_HIDDEN;
                    }
                    
                    int extraTableLines = 5;

                    allocTable( names.size() + extraTableLines );
                
                    int i=0;
                    setToCallbackEntry( &table[i] );
                    ++i;

                    setEntry( &table[i], DISABLE_ALL_LOGGING, 0,
                              POPT_ARG_NONE | extraArgInfo, 0,
                              "Disable all logging" );
                    ++i;
                
                
                    for(list<string>::const_iterator iter = names.begin();
                        iter != names.end(); iter++, i++ )
                    {
                        setEntry( &table[i], *iter, 0,
                                  POPT_ARG_INT | extraArgInfo, 0,
                                  desc, "255" );
                    }

                    clearEntry( &table[i] );
                    return table;
                }
        
        
        };
    
    
    
        struct ::poptOption*
        getPopTable()
        {
            const char* Desc = "Bitmask of range 0-255 "
                "to set if syslog is informed of debug";
        
            Loggers_t& lg = getLoggers();
            list<string> sl;

            for( Loggers_t::const_iterator iter = lg.begin();
                 iter != lg.end();
                 iter++ )
            {
                sl.push_back( iter->first );
            }

            typedef Loki::SingletonHolder<PopTableCollector<LogStateBase>,
            Loki::CreateUsingNew, Loki::NoDestroy > LoggPopt;
            return LoggPopt::Instance().getTable( Desc, sl );
        }
    

    
        char const lg_strf_class_name[]      = "ferris-logging-strf";
        char const lg_attr_class_name[]      = "ferris-logging-attr";
        char const lg_overmount_class_name[] = "ferris-logging-overmount";
        char const lg_sort_class_name[]      = "ferris-logging-sort";
        char const lg_image_class_name[]     = "ferris-logging-image";
        char const lg_pngimage_class_name[]  = "ferris-logging-pngimage";
        char const lg_jasperimage_class_name[]  = "ferris-logging-jasperimage";
        char const lg_jpegimage_class_name[] = "ferris-logging-jpegimage";
        char const lg_djvuimage_class_name[] = "ferris-logging-djvuimage";
        char const lg_gimpimage_class_name[] = "ferris-logging-gimpimage";
        char const lg_magikimage_class_name[]= "ferris-logging-magickimage";
        char const lg_imlib2image_class_name[]= "ferris-logging-imlib2image";
        char const lg_strattr_class_name[]   = "ferris-logging-strattr";
        char const lg_byteattr_class_name[]  = "ferris-logging-byteattr";
        char const lg_ctx_class_name[]       = "ferris-logging-ctx";
        char const lg_vm_class_name[]        = "ferris-logging-vm";
        char const lg_native_class_name[]    = "ferris-logging-native";
        char const lg_factory_class_name[]   = "ferris-logging-factory";
        char const lg_id3_class_name[]       = "ferris-logging-id3";
        char const lg_mpg_class_name[]       = "ferris-logging-mpg";
        char const lg_plugin_class_name[]    = "ferris-logging-plugin";
        char const lg_ffilter_class_name[]   = "ferris-logging-ffilter";
        char const lg_mbox_class_name[]      = "ferris-logging-mbox";
        char const lg_edb_class_name[]       = "ferris-logging-edb";
        char const lg_paths_class_name[]     = "ferris-logging-paths";
        char const lg_pccts_class_name[]     = "ferris-logging-pccts";
        char const lg_filterparse_class_name[]="ferris-logging-filterparse";
        char const lg_xfs_class_name[]       ="ferris-logging-xfs";
        char const lg_rdf_class_name[]       ="ferris-logging-rdf";
        char const lg_exif_class_name[]       ="ferris-logging-exif";
        char const lg_ctxrec_class_name[]       ="ferris-logging-ctxrec";
        char const lg_rtimeparse_class_name[]       ="ferris-logging-rtimeparse";
        char const lg_libextractor_class_name[]       ="ferris-logging-libextractor";
        char const lg_gphoto_class_name[]       ="ferris-logging-gphoto";
        char const lg_fca_class_name[]       ="ferris-logging-fca";
        char const lg_evo_class_name[]       ="ferris-logging-evo";
        char const lg_samba_class_name[]       ="ferris-logging-samba";
        char const lg_pg_class_name[]       ="ferris-logging-pg";
        char const lg_bibtex_class_name[]       ="ferris-logging-bibtex";
        char const lg_kde_class_name[]       ="ferris-logging-kde";
        char const lg_annodex_class_name[]       ="ferris-logging-annodex";
        char const lg_obby_class_name[]       ="ferris-logging-obby";
        char const lg_xsltfs_class_name[]       ="ferris-logging-xsltfs";
        char const lg_rdfattrcache_class_name[]       ="ferris-logging-rdfattrcache";
        char const lg_ffuse_class_name[]       ="ferris-logging-ffuse";
        char const lg_beagle_class_name[]       ="ferris-logging-beagle";
        char const lg_glob_class_name[]       ="ferris-logging-glob";
        char const lg_amarok_class_name[]       ="ferris-logging-amarok";
        char const lg_xmms_class_name[]       ="ferris-logging-xmms";
        char const lg_private_class_name[]       ="ferris-logging-private";
        char const lg_dbus_class_name[]       ="ferris-logging-dbus";
        char const lg_xqilla_class_name[]       ="ferris-logging-xqilla";
        char const lg_webphoto_class_name[]       ="ferris-logging-webphoto";
        char const lg_fspot_class_name[]       ="ferris-logging-fspot";
        char const lg_strigi_class_name[]       ="ferris-logging-strigi";
        char const lg_mdserv_class_name[]       ="ferris-logging-mdserv";
        char const lg_volman_class_name[]       ="ferris-logging-volman";
        char const lg_hal_class_name[]       ="ferris-logging-hal";
        char const lg_ego_class_name[]       ="ferris-logging-ego";
        char const lg_bgproc_class_name[]       ="ferris-logging-bgproc";
        char const lg_flac_class_name[]       ="ferris-logging-flac";
        char const lg_oggz_class_name[]       ="ferris-logging-oggz";
        char const lg_taglib_class_name[]       ="ferris-logging-taglib";
        char const lg_ffind_class_name[]       ="ferris-logging-ffind";
        char const lg_qtsql_class_name[]       ="ferris-logging-qtsql";
        char const lg_sqlite_class_name[]       ="ferris-logging-sqlite";
        char const lg_recordfile_class_name[]       ="ferris-logging-recordfile";
        char const lg_google_class_name[]       ="ferris-logging-google";
        char const lg_boxcom_class_name[]       ="ferris-logging-boxcom";
        char const lg_facebook_class_name[]       ="ferris-logging-facebook";
        char const lg_vimeo_class_name[]       ="ferris-logging-vimeo";
        char const lg_identica_class_name[]       ="ferris-logging-identica";
        char const lg_zoneminder_class_name[]       ="ferris-logging-zoneminder";
        char const lg_ferrisrest_class_name[]       ="ferris-logging-ferrisrest";
        char const lg_kio_class_name[]       ="ferris-logging-kio";
        char const lg_gdrive_class_name[]       ="ferris-logging-gdrive";
        char const lg_qio_class_name[]       ="ferris-logging-qio";
        char const lg_webservice_class_name[]       ="ferris-logging-webservice";
        char const lg_gstreamer_class_name[]       ="ferris-logging-gstreamer";
        char const lg_upnp_class_name[]       ="ferris-logging-upnp";
        char const lg_useroverlay_class_name[]       ="ferris-logging-useroverlay";
        char const lg_plasma_class_name[]       ="ferris-logging-plasma";
        char const lg_sane_class_name[]       ="ferris-logging-sane";
        char const lg_printer_class_name[]       ="ferris-logging-printer";
        char const lg_pulseaudio_class_name[]       ="ferris-logging-pulseaudio";
        char const lg_wiki_class_name[]       ="ferris-logging-wiki";
        char const lg_spiritcontext_class_name[]       ="ferris-logging-spiritcontext";
        char const lg_pastebin_class_name[]       ="ferris-logging-pastebin";
        char const lg_libmediainfo_class_name[]       ="ferris-logging-libmediainfo";
        char const lg_subtitles_class_name[]       ="ferris-logging-subtitles";
        char const lg_ferrisqtgui_class_name[]       ="ferris-logging-ferrisqtgui";
        char const lg_astext_class_name[]       ="ferris-logging-astext";
        char const lg_fileactions_class_name[]       ="ferris-logging-fileactions";
        char const lg_emacs_class_name[]       ="ferris-logging-emacs";
        char const lg_firefox_class_name[]       ="ferris-logging-firefox";
        char const lg_iconsrv_class_name[]       ="ferris-logging-iconsrv";
        char const lg_xine_class_name[]       ="ferris-logging-xine";
        char const lg_socket_class_name[]    ="ferris-logging-socket";
        char const lg_sqldb_class_name[]   ="ferris-logging-sqldb";
        char const lg_sqlplus_class_name[]   ="ferris-logging-sqlplus";
        char const lg_dtl_class_name[]   ="ferris-logging-dtl";
        char const lg_curl_class_name[]      ="ferris-logging-curl";
        char const lg_gtkferris_class_name[] ="ferris-logging-gtkferris";
        char const lg_ferriscreate_class_name[] ="ferris-logging-ferriscreate";
        char const lg_create_class_name[] ="ferris-logging-create";
        char const lg_schema_class_name[] ="ferris-logging-schema";
        char const lg_emblem_class_name[] ="ferris-logging-emblem";
        char const lg_time_class_name[]      ="ferris-logging-time";
        char const lg_copy_class_name[]      ="ferris-logging-copy";
        char const lg_move_class_name[]      ="ferris-logging-move";
        char const lg_rm_class_name[]        ="ferris-logging-rm";
        char const lg_dvdread_class_name[]   ="ferris-logging-dvdread";
        char const lg_ipcctx_class_name[]    ="ferris-logging-ipcctx";
        char const lg_fclip_class_name[]     ="ferris-logging-fclip";
        char const lg_db4_class_name[]       ="ferris-logging-db4";
        char const lg_gdbm_class_name[]       ="ferris-logging-gdbm";
        char const lg_tdb_class_name[]       ="ferris-logging-tdb";
        char const lg_commondb_class_name[]       ="ferris-logging-commondb";
        char const lg_eet_class_name[]       ="ferris-logging-eet";
        char const lg_fnews_class_name[]     ="ferris-logging-fnews";
        char const lg_journal_class_name[]     ="ferris-logging-journal";
        char const lg_ldap_class_name[]     ="ferris-logging-ldap";
        char const lg_exceptions_class_name[]     ="ferris-logging-exceptions";
        char const lg_ftxlexi_class_name[]     ="ferris-logging-fulltext-lexicon";
        char const lg_idx_class_name[]       ="ferris-logging-index";
        char const lg_eaidx_class_name[]     ="ferris-logging-ea-index";
        char const lg_iostream_class_name[]  ="ferris-logging-iostream";
        char const lg_xml_class_name[]       ="ferris-logging-xml";
        char const lg_dom_class_name[]       ="ferris-logging-dom";
        char const lg_extfs_class_name[]     ="ferris-logging-extfs";
        char const lg_runner_class_name[]    ="ferris-logging-runner";
        char const lg_a52_class_name[]       ="ferris-logging-a52";
        char const lg_mpeg2_class_name[]     ="ferris-logging-mpeg2";







    
    
    };
 
};
