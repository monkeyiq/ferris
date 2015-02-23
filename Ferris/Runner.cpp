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

    $Id: Runner.cpp,v 1.11 2010/11/17 21:30:47 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris.hh>
#include <Runner.hh>

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

using namespace std;

namespace Ferris
{
    
    static void childSetupFunctor_null( fh_runner )
    {
    }
    

    void Ferris_GSpawnChildSetupFunc(gpointer user_data)
    {
        ::Ferris::Runner* r = (::Ferris::Runner*)user_data;
        r->SetupFunc();
    }

    
    char**
    stringvec_to_CSTRvec( const vector<string>& v )
    {
        int l = v.size();
        int i = 0;
        
        char** ret = g_new0(char*, l+2 );
        for( i=0; i < l; ++i )
        {
            ret[i] = g_strdup(v[i].c_str());
        }
        
        ret[i] = 0;
        return ret;
    }

    vector<string>
    CSTRvec_to_stringvec( char** cv )
    {
        vector<string> ret;
        
        if( cv )
        {
            char** p = &cv[0];
            while( *p )
            {
                ret.push_back( *p );
                ++p;
            }
        }

        return ret;
    }
    
    static fh_istream null_runner_async_functor( fh_runner r, fh_istream ret )
    {
        return ret;
    }
    
    
    Runner::Runner()
        :
        argvCSTR(0),
        envpCSTR(0),
        SpawnFlags( GSpawnFlags(G_SPAWN_DO_NOT_REAP_CHILD|G_SPAWN_SEARCH_PATH) ),
        ConnectStdIn(false),
        InPipeFd(-1),
        OutPipeFd(-1),
        ErrPipeFd(-1),
        pid(0),
        myerror(0),
        WorkingDir("/tmp"),
        HaveWaitedForProcess( false ),
        InhertENV( true ),
        m_stdoutFunctor( null_runner_async_functor ),
        m_stdoutFunctorUsed( false ),
        m_stdoutAsyncHandler( new AsyncIOHandler() ),
        m_childSetupFunctor( childSetupFunctor_null ),
        m_Connect_ChildStdOut_To_ParentStdOut( false ),
        m_Connect_ChildStdErr_To_ParentStdErr( false )
    {
        setWaitTimeOut( 120 );
    }


    Runner::~Runner()
    {
        LG_EAIDX_D << "~Runner()" << endl;
///        cerr << "~Runner()" << endl;
        m_stdoutAsyncHandler->release();
        clearMyError();
    }

    Runner::Runner( const Runner& r )
    {
        argvCSTR = 0;
        envpCSTR = 0;
        SpawnFlags = r.SpawnFlags;
        ConnectStdIn = r.ConnectStdIn;
        InPipeFd = r.InPipeFd;
        OutPipeFd = r.OutPipeFd;
        ErrPipeFd = r.ErrPipeFd;
        pid = r.pid;
        myerror = 0;
        WorkingDir = r.WorkingDir;
        HaveWaitedForProcess = r.HaveWaitedForProcess;
        InhertENV = r.InhertENV;
        WaitTimeOut = r.WaitTimeOut;
        ExitStatus = r.ExitStatus;
        m_stdoutFunctor = r.m_stdoutFunctor;
        m_stdoutFunctorUsed = r.m_stdoutFunctorUsed;
        m_stdoutAsyncHandler = r.m_stdoutAsyncHandler;
        m_childSetupFunctor = r.m_childSetupFunctor;
        m_Connect_ChildStdOut_To_ParentStdOut = r.m_Connect_ChildStdOut_To_ParentStdOut;
        m_Connect_ChildStdErr_To_ParentStdErr = r.m_Connect_ChildStdErr_To_ParentStdErr;
        
        copy( r.Argv.begin(), r.Argv.end(), back_inserter(Argv) );
        copy( r.Envp.begin(), r.Envp.end(), back_inserter(Envp) );
    }
    
    Runner&
    Runner::operator=( const Runner& r )
    {
        if( this != &r )
        {
            argvCSTR = 0;
            envpCSTR = 0;
            SpawnFlags = r.SpawnFlags;
            ConnectStdIn = r.ConnectStdIn;
            InPipeFd = r.InPipeFd;
            OutPipeFd = r.OutPipeFd;
            ErrPipeFd = r.ErrPipeFd;
            pid = r.pid;
            myerror = 0;
            WorkingDir = r.WorkingDir;
            HaveWaitedForProcess = r.HaveWaitedForProcess;
            InhertENV = r.InhertENV;
            WaitTimeOut = r.WaitTimeOut;
            ExitStatus = r.ExitStatus;
            m_stdoutFunctor = r.m_stdoutFunctor;
            m_stdoutFunctorUsed = r.m_stdoutFunctorUsed;
            m_stdoutAsyncHandler = r.m_stdoutAsyncHandler;
            m_childSetupFunctor = r.m_childSetupFunctor;
            m_Connect_ChildStdOut_To_ParentStdOut = r.m_Connect_ChildStdOut_To_ParentStdOut;
            m_Connect_ChildStdErr_To_ParentStdErr = r.m_Connect_ChildStdErr_To_ParentStdErr;
            
            copy( r.Argv.begin(), r.Argv.end(), back_inserter(Argv) );
            copy( r.Envp.begin(), r.Envp.end(), back_inserter(Envp) );
        }
        return *this;
    }

    
    void
    Runner::setCommandLine( const std::string& cmdline )
    {
        gint    argc = 0;
        gchar** c_argv = 0;
        GError *err = 0;

        gboolean rc = g_shell_parse_argv( cmdline.c_str(), &argc, &c_argv, &err );
        if( !rc )
        {
            fh_stringstream ss;
            ss << "Error passing cmdline:" << cmdline << " msg:" << err->message;
            Throw_ParseError( tostr(ss), 0 );
        }

//         cerr << "cmdline:" << cmdline << endl;
//         cerr << "argc:" << argc << endl;
        
        Argv.clear();
        Argv = CSTRvec_to_stringvec( c_argv );
        g_strfreev( c_argv );
    }

    void
    Runner::pushCommandLineArg( const std::string& a )
    {
        Argv.push_back( a );
    }
    

    
    Runner::ArgArray_t&
    Runner::getArgv()
    {
        return Argv;
    }
    
    Runner::ArgArray_t&
    Runner::getEnvp()
    {
        return Envp;
    }


    void
    Runner::argvPushPath( const std::string& v )
    {
//         if( contains( v, " " ) )
//         {
//             stringstream ss;
//             ss << "\"" << v << "\"";
//             getArgv().push_back( ss.str() );
//         }
//         else
        {
            getArgv().push_back( v );
        }
    }
    
    

    void
    Runner::SetupFunc()
    {
        fh_runner r = this;
        m_childSetupFunctor( r );
    }

    void
    Runner::setWorkingDir( std::string v )
    {
        WorkingDir = v;
    }
    
    void
    Runner::setInheritENV( bool v )
    {
        InhertENV = v;
    }
    

    void
    Runner::setSpawnFlags( GSpawnFlags v )
    {
        SpawnFlags = v;
    }

    GSpawnFlags
    Runner::getSpawnFlags()
    {
        return SpawnFlags;
    }
    

    
    fh_ostream
    Runner::getStdIn( bool AutoCloseFDOnStreamBufDtor )
    {
        return ::Ferris::Factory::MakeFdOStream( InPipeFd, AutoCloseFDOnStreamBufDtor );
    }
    
    fh_istream
    Runner::getStdOut( bool AutoCloseFDOnStreamBufDtor )
    {
        return ::Ferris::Factory::MakeFdIStream( OutPipeFd, AutoCloseFDOnStreamBufDtor );
    }
    
    fh_istream
    Runner::getStdErr( bool AutoCloseFDOnStreamBufDtor )
    {
        return ::Ferris::Factory::MakeFdIStream( ErrPipeFd, AutoCloseFDOnStreamBufDtor );
    }

    int
    Runner::getStdInFd()
    {
        return InPipeFd;
    }
    
    int
    Runner::getStdOutFd()
    {
        return OutPipeFd;
    }

    int
    Runner::getStdErrFd()
    {
        return ErrPipeFd;
    }
    
    pid_t
    Runner::getChildProcessID()
    {
        return pid;
    }
    
    void
    Runner::signalChild( int sig )
    {
        kill( pid, sig );
    }
    
    void
    Runner::killChild()
    {
        signalChild( 9 );
    }
    
    
    void
    Runner::setExitStatus( gint status )
    {
//        cerr << "Runner::setExitStatus() st:" << status << endl;
        
        if(!(ConnectStdIn == false)) close(InPipeFd);
        if(!(SpawnFlags & G_SPAWN_STDOUT_TO_DEV_NULL)) close(OutPipeFd);
        if(!(SpawnFlags & G_SPAWN_STDERR_TO_DEV_NULL)) close(ErrPipeFd);
        m_stdoutAsyncHandler->disconnect();
        
        ExitStatus           = status;
        HaveWaitedForProcess = true;
    }

    void
    Runner::closeChildStdOut()
    {
        if( OutPipeFd < 0 )
            return;
        
        if(!(SpawnFlags & G_SPAWN_STDOUT_TO_DEV_NULL)) close(OutPipeFd);
        OutPipeFd = -1;
    }
    void
    Runner::closeChildStdErr()
    {
        if( ErrPipeFd < 0 )
            return;
        
        if(!(SpawnFlags & G_SPAWN_STDERR_TO_DEV_NULL)) close(ErrPipeFd);
        ErrPipeFd = -1;
    }
    void
    Runner::closeChildStdIn()
    {
        if( InPipeFd < 0 )
            return;

        if(!(ConnectStdIn == false)) close(InPipeFd);
        InPipeFd = -1;
    }
    
    
    
    gint
    Runner::getExitStatus()
    {
        LG_RUNNER_D << "Runner::getExitStatus()"
                    << " HaveWaitedForProcess:" << HaveWaitedForProcess
                    << " WaitTimeOut:" << WaitTimeOut
                    << " pid:" << pid
                    << endl;

//         /* Try to force the local filehandles out of scope */
//         if(!(ConnectStdIn == false)) close(InPipeFd);
//         if(!(SpawnFlags & G_SPAWN_STDOUT_TO_DEV_NULL)) close(OutPipeFd);
//         if(!(SpawnFlags & G_SPAWN_STDERR_TO_DEV_NULL)) close(ErrPipeFd);
        /* Try to force the local filehandles out of scope */
        if(!(ConnectStdIn == false)) close(InPipeFd);
        if(!(SpawnFlags & G_SPAWN_STDOUT_TO_DEV_NULL)) fsync(OutPipeFd);
        if(!(SpawnFlags & G_SPAWN_STDERR_TO_DEV_NULL)) fsync(ErrPipeFd);

        /* wait for the subprocess */
        if( !HaveWaitedForProcess )
        {
            int status  = 0;
            bool virgin = true;
            pid_t rc    = -1;
            int WaitTimeOutRemaining = WaitTimeOut;

            LG_RUNNER_D << "About to wait for child pid:" << pid << endl;
            HaveWaitedForProcess = true;
            ExitStatus = 0;
            errno      = 0;
            
            while( rc < 0 && (virgin || errno == EINTR || WaitTimeOutRemaining) )
            {
                virgin = false;

                LG_RUNNER_D << "calling waitpid() pid:" << pid << endl;

                pid_t rc = waitpid( pid, &status, WNOHANG );
//                pid_t rc = waitpid( pid, &status, 0 );
                int errnocache = errno;
                
                LG_RUNNER_D << "waitpid() status:" << status 
                            << " rc:" << rc 
                            << " exit:" << WEXITSTATUS(status)
                            << " errno:" << errno
                            << " errstr:" << errnum_to_string("", errnocache )
                            << endl;
                
                if( rc == 0 )
                {
                    /* The child is not ready yet */
                    if( !WaitTimeOutRemaining )
                    {
                        stringstream ss;
                        ss << "Runner::getExitStatus(), time ran out waiting for the subprocess"
                           << " pid:" << pid
                           << endl;
                        Throw_FerrisWaitTimedOut( tostr(ss), 0 );
                    }
                    
                    sleep( 1 );
                    --WaitTimeOutRemaining;
                    rc = -1;

                    LG_RUNNER_D << "About to wait again after decrementing the"
                                << " total time left:" << WaitTimeOutRemaining << endl;
                    continue;
                }
                else if( rc < 0 )
                {
                    /* Why did we fail? */
                    LG_RUNNER_D << "waiting failed! errnocache:" << errnocache << endl;
                    
                    if (errnocache == EINTR)
                    {
                        LG_RUNNER_D << "due to interupt during system call, retrying" << endl;
                        continue;
                    }
                    else if (errnocache == ECHILD)
                    {
                        LG_RUNNER_ER << "getExitStatus(), exit status of a child process was requested but SIGCHLD action was set to SIG_IGN and ECHILD was received by waitpid(), so exit status can't be returned. This is a bug in the program using class Runner; either don't request the exit status, or don't set the SIGCHLD action." << endl;
                    }
                    else
                    {
                        LG_RUNNER_ER << "getExitStatus(), Unexpected error." << endl;
                    }
                }
                else
                {
                    /* waitpid was ok, get the exit status for a normal exit or
                     * return 1 for a strange client exit.
                     */
                    if( WIFEXITED( status ) )
                    {
                        ExitStatus = WEXITSTATUS(status);
                    }
                    else
                    {
                        ExitStatus = 1;
                    }
                    LG_RUNNER_D << "waited, exit status:" << ExitStatus << endl;
                    break;
                }
            }
        }

        m_stdoutAsyncHandler->disconnect();
        
        LG_RUNNER_D << "exit status:" << ExitStatus << endl;
        return ExitStatus;
    }

    void
    Runner::setConnectStdIn( bool v )
    {
        ConnectStdIn = v;
    }

    void
    Runner::setWaitTimeOut( int v )
    {
        WaitTimeOut = v;
    }
    
    void
    Runner::clearMyError()
    {
        if( myerror != NULL )
        {
            g_error_free( myerror );
            myerror = NULL;
        }
    }
    
    string 
    Runner::getErrorString()
    {
        if( myerror )
        {
            stringstream ss;
            ss << "GError, " << myerror->message << endl;
            return tostr(ss);
        }
        return "";
    }

    void
    Runner::setConnect_ChildStdOut_To_ParentStdOut( bool v )
    {
        m_Connect_ChildStdOut_To_ParentStdOut = v;
    }
    
    void
    Runner::setConnect_ChildStdErr_To_ParentStdErr( bool v )
    {
        m_Connect_ChildStdErr_To_ParentStdErr = v;
    }
    
    
    
    void
    Runner::operator()()
    {
        argvCSTR = stringvec_to_CSTRvec( getArgv() );
        envpCSTR = stringvec_to_CSTRvec( getEnvp() );

        clearMyError();

        HaveWaitedForProcess = false;
        ExitStatus = 0;

        LG_RUNNER_D << "Runner::operator() wd:" << WorkingDir
                    << " SpawnFlags:" << SpawnFlags
                    << " pid:" << pid
                    << " ConnectStdIn:" << ConnectStdIn
                    << " InhertENV:" << InhertENV
                    << endl;
        LG_RUNNER_D << "Runner::operator() desc:" << getDescription() << endl;
        {
            char** p = argvCSTR;
            while( *p )
            {
                LG_RUNNER_D << "Runner::operator() p:" << *p << endl;
                ++p;
            }
        }
        
        bool ret = g_spawn_async_with_pipes(
            WorkingDir.c_str(),
            argvCSTR,
            InhertENV ? NULL : envpCSTR,
            SpawnFlags,
            Ferris_GSpawnChildSetupFunc, (void*)this,
            &pid,
            (ConnectStdIn == false)                   ? NULL : &InPipeFd,
            m_Connect_ChildStdOut_To_ParentStdOut |
            (SpawnFlags & G_SPAWN_STDOUT_TO_DEV_NULL) ? NULL : &OutPipeFd,
            m_Connect_ChildStdErr_To_ParentStdErr |
            (SpawnFlags & G_SPAWN_STDERR_TO_DEV_NULL) ? NULL : &ErrPipeFd,
            &myerror);
        
        if(argvCSTR) g_strfreev( argvCSTR );
        if(envpCSTR) g_strfreev( envpCSTR );
        
        if( !ret )
        {
            fh_stringstream ss;
            ss << "Error running command:" << getDescription();
            Throw_ProgramSpawn( tostr(ss), 0 );
        }

        if( m_stdoutFunctorUsed )
        {
            m_stdoutAsyncHandler->release();
            m_stdoutAsyncHandler->attach( getStdOutFd() );
            m_stdoutAsyncHandler->setFunctor(
                AsyncIOHandler::AsyncIOFunctor_t( this, &_Self::stdoutAsyncFunctor ));
        }
    }

    fh_istream
    Runner::stdoutAsyncFunctor( fh_aiohandler aio, fh_istream iss )
    {
        fh_runner r = this;
        return m_stdoutFunctor( r, iss );
    }
    

    
    void
    Runner::Run()
    {
        operator()();
    }

    fh_istream
    Runner::RunWithStdoutAsReadablePipe()
    {
        setConnectStdIn( false );
        
        GSpawnFlags spawnflags = getSpawnFlags();
        spawnflags = GSpawnFlags( spawnflags | G_SPAWN_STDERR_TO_DEV_NULL );
        spawnflags = GSpawnFlags( spawnflags  & (~G_SPAWN_DO_NOT_REAP_CHILD) );
        setSpawnFlags( spawnflags );
        
        Run();
        fh_istream ret = getStdOut();
        return ret;
    }
    
    
    
    std::string
    Runner::getDescription()
    {
        int i=0;
        fh_stringstream ss;
        
        ss << " WorkingDir:" << WorkingDir
           << " WaitTimeOut:" << WaitTimeOut
           << " SpawnFlags:" << SpawnFlags
           << " HaveWaitedForProcess:" << HaveWaitedForProcess
           << " ExitStatus:" << ExitStatus
           << " ConnectStdIn:" << ConnectStdIn
           << " InhertENV:" << InhertENV
           << " InPipeFd:" << InPipeFd
           << " OutPipeFd:" << OutPipeFd
           << " ErrPipeFd:" << ErrPipeFd
           << " pid:" << pid
           << " ";
        ss << getErrorString();

        i=0;
        for( ArgArray_t::iterator iter = Argv.begin(); iter != Argv.end(); ++iter, ++i )
        {
            ss << " argv[" << i << "]:" << *iter;
        }
        i=0;
        for( ArgArray_t::iterator iter = Envp.begin(); iter != Envp.end(); ++iter, ++i )
        {
            ss << " env[" << i << "]:" << *iter;
        }
        return tostr(ss);
    }

    void
    Runner::setAsyncStdOutFunctor( Runner_AsyncIOFunctor_t x )
    {
        m_stdoutFunctor     = x;
        m_stdoutFunctorUsed = true;
    }

    void
    Runner::setChildSetupFunctor( const ChildSetupFunctor_t& f )
    {
        m_childSetupFunctor = f;
    }


    template< class T >
    T copyTo( fh_istream iss, T oss )
    {
        copy( istreambuf_iterator<char>(iss),
              istreambuf_iterator<char>(),
              ostreambuf_iterator<char>(oss));
        oss << flush;
        return oss;
    }
    
    int
    Runner::executeAndReturnStdOut( std::string& ret )
    {
        LG_RUNNER_D << "executeAndReturnStdOut()" << endl;

        setSpawnFlags( GSpawnFlags( getSpawnFlags()
                                    | G_SPAWN_STDERR_TO_DEV_NULL
                                    | G_SPAWN_DO_NOT_REAP_CHILD
                                    | G_SPAWN_SEARCH_PATH) );
        
        Run();
        fh_stringstream ss;
        LG_RUNNER_D << "OutPipeFd:" << OutPipeFd << endl;

        fh_istream iss = getStdOut();
        copyTo( iss, ss );
        // copy( istreambuf_iterator<char>(iss),
        //       istreambuf_iterator<char>(),
        //       ostreambuf_iterator<char>(ss));

        ret = ss.str();
        LG_RUNNER_D << "executeAndReturnStdOut() ret.sz:" << ret.length() << endl;

        gint e = getExitStatus();
        LG_RUNNER_D << "executeAndReturnStdOut() e:" << e << endl;
        return e;
    }
    
    namespace Private
    {
        void attachStreamCollector( const fh_runner& r,
                                    const Runner_AsyncIOFunctor_t& f )
        {
            r->setAsyncStdOutFunctor( f );
        }
    };
    
};
