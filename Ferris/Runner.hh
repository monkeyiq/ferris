/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: Runner.hh,v 1.10 2010/11/17 21:30:47 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_RUNNER_H_
#define _ALREADY_INCLUDED_FERRIS_RUNNER_H_

#include <glib.h>

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/FerrisException.hh>
#include <Ferris/FerrisHandle.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/Runner_FunctorType.hh>

namespace Ferris
{
    namespace Private
    {
        FERRISEXP_API void attachStreamCollector( const fh_runner& r,
                                    const Runner_AsyncIOFunctor_t& x );
    };
};

#include <Ferris/AsyncIO.hh>

#include <glib.h>

#include <sigc++/sigc++.h>
#include <SmartPtr.h>


namespace Ferris
{
    FERRISEXP_API char** stringvec_to_CSTRvec( const std::vector<std::string>& v );

    
    class FERRISEXP_API Runner
        :
        public Handlable
    {
        typedef Runner _Self;
        
    public:
        
        typedef std::vector<std::string> ArgArray_t;
//         typedef Loki::Functor< fh_istream,
//                                LOKI_TYPELIST_2( fh_runner, fh_istream ) > AsyncIOFunctor_t;
#ifndef LIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER

    private:

        
        std::string WorkingDir;
        ArgArray_t Argv;
        ArgArray_t Envp;
        gchar **argvCSTR;
        gchar **envpCSTR;

        int WaitTimeOut;
        GSpawnFlags SpawnFlags;
        bool HaveWaitedForProcess;
        gint ExitStatus;
        bool ConnectStdIn;
        GError* myerror;
        bool InhertENV;

        gint InPipeFd;
        gint OutPipeFd;
        gint ErrPipeFd;
        
        gint pid;

        bool m_Connect_ChildStdOut_To_ParentStdOut;
        bool m_Connect_ChildStdErr_To_ParentStdErr;
        
        Runner_AsyncIOFunctor_t m_stdoutFunctor;
        bool                    m_stdoutFunctorUsed;
        fh_aiohandler           m_stdoutAsyncHandler;
        fh_istream              stdoutAsyncFunctor( fh_aiohandler aio, fh_istream iss );
        

        friend void Ferris_GSpawnChildSetupFunc(gpointer user_data);
        void SetupFunc();

        void clearMyError();

#endif
    public:
        
        Runner();
        virtual ~Runner();
        Runner( const Runner& );
        Runner& operator=( const Runner& );

        void setCommandLine( const std::string& cmdline );
        void pushCommandLineArg( const std::string& a );
        ArgArray_t& getArgv();
        ArgArray_t& getEnvp();

        void argvPushPath( const std::string& v );
        

        void setSpawnFlags( GSpawnFlags v );
        GSpawnFlags getSpawnFlags();
        void setWorkingDir( std::string v );
        
        fh_ostream getStdIn( bool AutoCloseFDOnStreamBufDtor = true );
        fh_istream getStdOut( bool AutoCloseFDOnStreamBufDtor = true );
        fh_istream getStdErr( bool AutoCloseFDOnStreamBufDtor = true );

        int getStdInFd();
        int getStdOutFd();
        int getStdErrFd();

        pid_t getChildProcessID();
        void signalChild( int sig = 9 );
        void killChild();
        
        gint getExitStatus();

        /**
         * Close access to the child's stdout and stderr if they
         * were not set to NULL
         */
        void closeChildStdOut();
        void closeChildStdErr();
        void closeChildStdIn();

        /**
         * Connect the child stdout/err to the parents stream.
         */
        void  setConnect_ChildStdOut_To_ParentStdOut( bool v );
        void  setConnect_ChildStdErr_To_ParentStdErr( bool v );
        

        /*
         * if the user calls waitpid() etc then they should inform the runner
         * that they have discovered it has finished and set its exit status
         */
        void setExitStatus( gint status );

        void setConnectStdIn( bool v );
        void setWaitTimeOut( int v );
        void setInheritENV( bool v );

        std::string getErrorString();
        
        void operator()();
        void Run();

        /**
         * Sets the flags G_SPAWN_DO_NOT_REAP_CHILD
         * and G_SPAWN_STDERR_TO_DEV_NULL etc and calls Run();
         * This method sets up the command to give you a pipe to the
         * new child's stdout which you can read until EOF (you
         * consumed all the child's output) or you can just drop the
         * streambuf out of scope which will close the child with SIGPIPE.
         *
         * @returns A readable pipe to the subprocesses stdout. You can close
         *          this pipe early to have the child terminated if you don't want
         *          to read the entire subprocess' output.
         */
        fh_istream RunWithStdoutAsReadablePipe();

        std::string getDescription();

        void setAsyncStdOutFunctor( Runner_AsyncIOFunctor_t x );

        typedef Loki::Functor< void, LOKI_TYPELIST_1( fh_runner ) > ChildSetupFunctor_t;

        /*
         * A function to be called after fork() just before exec()
         */
        void setChildSetupFunctor( const ChildSetupFunctor_t& f );
        

        // private.
        gboolean internal_async_cb( GIOChannel *source, GIOCondition condition );

        /*
         * Execute the runner and return it's stdout as a string.
         */
        int executeAndReturnStdOut( std::string& ret );
        
        
    private:
        ChildSetupFunctor_t m_childSetupFunctor;
    };

};

#endif
