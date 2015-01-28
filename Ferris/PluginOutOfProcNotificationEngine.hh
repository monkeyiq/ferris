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

    $Id: PluginOutOfProcNotificationEngine.hh,v 1.6 2010/09/24 21:30:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


#ifndef _ALREADY_INCLUDED_FERRIS_PLUGIN_OUTOFPROC_NE_H_
#define _ALREADY_INCLUDED_FERRIS_PLUGIN_OUTOFPROC_NE_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/FerrisStdHashMap.hh>

namespace Ferris
{
    class PluginOutOfProcNotificationEngine;
    
    /**
     * If a plugin handles notification of contexts created by other processes
     * using the same plugin it should implement this interface.
     *
     * @see IHandleOutOfProcEACreationNotification
     * @see PluginOutOfProcNotificationEngine
     */
    class FERRISEXP_API IHandleOutOfProcContextCreationNotification
    {
    public:
        virtual void OnOutOfProcContextCreationNotification(
            const std::string& rdn,
            bool isDataValid,
            const std::string& data ) = 0;
    };

    /**
     * If a plugin handles notification of contexts being changed (their contents written too)
     * by other processes using the same plugin it should implement this interface.
     *
     * @see IHandleOutOfProcContextCreationNotification
     * @see PluginOutOfProcNotificationEngine
     */
    class FERRISEXP_API IHandleOutOfProcContextChangedNotification
    {
    public:
        virtual void OnOutOfProcContextChangedNotification(
            const std::string& rdn,
            bool isDataValid,
            const std::string& data ) = 0;
    };
    
    /**
     * If a plugin handles notification of contexts created by other processes
     * using the same plugin it should implement this interface.
     *
     * @see IHandleOutOfProcContextCreationNotification
     * @see IHandleOutOfProcEACreationNotification
     * @see PluginOutOfProcNotificationEngine
     */
    class FERRISEXP_API IHandleOutOfProcContextDeletionNotification
    {
    public:
        virtual void OnOutOfProcContextDeletionNotification( const std::string& rdn ) = 0;
    };
    
    /**
     * If a plugin handles notification of new EA created on a context
     * by other processes using the same plugin it should implement this interface.
     *
     * @see IHandleOutOfProcContextCreationNotification
     * @see PluginOutOfProcNotificationEngine
     */
    class FERRISEXP_API IHandleOutOfProcEANotification
    {
    public:
        virtual void OnOutOfProcEACreationNotification( const std::string& eaname,
                                                        bool isDataValid,
                                                        const std::string& data );
        virtual void OnOutOfProcEADeletionNotification( const std::string& eaname );
    };
    
    namespace Factory
    {
        /**
         * Used by context plugins which implement
         * IHandleOutOfProcContextCreationNotification
         * to start monitoring a local tree of contexts
         *
         * Usually called on the root of an overmount.
         **/
        FERRISEXP_API PluginOutOfProcNotificationEngine& getPluginOutOfProcNotificationEngine();

        /**
         * Mainly only useful for ferris-out-of-proc-notification-deamon
         * so that it doesn't try to make connections with itself.
         *
         * Can be used by other apps if they want to ignore out-of-proc
         * update notifications. Must be called before any contexts are
         * created.
         */
        FERRISEXP_API void setDontConnectWithFerrisOutOfProcDeamon( bool v );
    };


    /**
     * Allow sending of any XML messages to many processes in a monitoring
     * group. Subclasses handle explicit cases such as notification of
     * changes in Context trees using PluginOutOfProcNotificationEngine
     *
     * Note that there can be many objects of type OutOfProcNotificationEngine
     * aslong as the deamon that replicates the message is monitoring a different
     * path.
     */
    class FERRISEXP_API OutOfProcNotificationEngine
        :
        public sigc::trackable
    {
        typedef OutOfProcNotificationEngine _Self;
        
    protected:
        
        /**
         * fifo to send data to the multicast server ferris-out-of-proc-notification-deamon
         */
        fh_ostream outgoingss;

        /**
         * handle async io from server (notifications from other processes)
         */
        fh_aiohandler m_fromserv_aio;

        /**
         * XML stream collector for reading msgs from the server
         */
        fh_xstreamcol m_fromserv_xs;

        /**
         * Where to connect the deamon for this notification object.
         */
        std::string m_basedir;
        
        /**
         * handle async io from server (notifications from other processes)
         * callback for when data is received from server
         */
        virtual void xml_msg_arrived( fh_xstreamcol h );


        /**
         * get the proccess ID of the multicast server
         */
        pid_t getServerPID();
        
        /**
         * Make sure the server (ferris-out-of-proc-notification-deamon)
         * is running and attached to its end of the fifo
         */
        void ensureServerRunning();
        
        
        typedef pid_t  clientID_t;

        /**
         * Get the unique ID that identifies this client to the server
         */
        clientID_t getClientID();

    public:

        OutOfProcNotificationEngine();
        virtual ~OutOfProcNotificationEngine();

        /**
         * Where to connect the deamon for this notification object.
         */
        void setBaseDir( const std::string& v );
        std::string getBaseDir();
    };

    /**
     * Used to debug the notification engine. Allows one to save incomming
     * data to a stream and send messages
     */
    class FERRISEXP_API OutOfProcNotificationEngineDebug
        :
        public OutOfProcNotificationEngine
    {
        fh_ostream m_messageLogStream;
        int m_messageCount;
        
    protected:

        virtual void xml_msg_arrived( fh_xstreamcol h );
        
    public:

        OutOfProcNotificationEngineDebug();
        virtual ~OutOfProcNotificationEngineDebug();

        void connect();
        void setOutputStream( fh_ostream oss );
        void sendMessage( stringmap_t& m );
    };
    
    
    
    /**
     * Monitor the watching server for changes and tell each plugin module
     * about the changes that other procs have made to the underlying data.
     *
     * eg. When another app creates a new context in a db4 file we are notified
     * and should tell any db4 overmounts in this proc to update their state to
     * reflect the new context that the other proc has made.
     */
    class FERRISEXP_API PluginOutOfProcNotificationEngine
        :
        public OutOfProcNotificationEngine
    {
        typedef OutOfProcNotificationEngine _Base;
        typedef PluginOutOfProcNotificationEngine _Self;

        /**
         * Avoid the senario:
         * ProcA creates
         * ProcA notifies daemon which notifies ProcB
         * ProcB creates
         * ProcB notifies daemon
         * by setting m_ignoreCreatedSignals temporarily when calling
         * OnOutOfProcContextCreationNotification()
         */
        bool m_ignoreCreatedSignals;
        bool m_ignoreDeletedSignals;
        bool m_ignoreEASignals;
        
        /**
         * Monitor each new context for exists/created signals too
         */
        void OnExists( NamingEvent_Exists* ev,
                       const fh_context& newc,
                       std::string olddn, std::string newdn );
        
        /**
         * When an object is created we monitor it too and tell the server that
         * there is a new context that all interested procs should know about
         */
        void OnCreated( NamingEvent_Created*,
                        const fh_context& newc,
                        std::string, std::string );

        /**
         * Notify when contexts are deleted
         */
        void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );
        

        /**
         * We keep a collection of which contexts we are monitoring so that we
         * dont monitor something twice.
         */
        typedef FERRIS_STD_HASH_SET< Context*,
                                     f_hash<Context* const>,
                                     f_equal_to<Context* const> > m_monitoredContexts_t;
        m_monitoredContexts_t m_monitoredContexts;

        /**
         * Connect to exists and created signals for context
         */
        void connectSignals( fh_context c );

        
        virtual void xml_msg_arrived( fh_xstreamcol h );
        
    public:

        /**
         * Use getPluginOutOfProcNotificationEngine() instead of creating one!
         */
        PluginOutOfProcNotificationEngine();

        virtual ~PluginOutOfProcNotificationEngine();
        
        
        /**
         * Start monitoring create events on base and all its children
         * and reporting that data to the watcher server
         */
        virtual void watchTree( fh_context base );

        /**
         * Undo anything that was done during watchTree()
         */
        virtual void forgetContext( Context* base );
        
        void ensureServerRunning();

        /**
         * Send notifications to other processes that a new EA now exists
         */
        void signalEACreated( fh_context c, const std::string& eaname );
        void signalEACreated( fh_context c, const std::string& eaname,
                              const std::string& data, bool isDataValid = true );

        /**
         * Send notifications to other processes that an EA has been deleted
         */
        void signalEADeleted( fh_context c, const std::string& eaname );

        /**
         * Send notifications that a single context was created. Do not use
         * this method if you have called watchTree().
         */
        void signalContextCreated( fh_context subc );
        void signalContextDeleted( fh_context p, const std::string& olddn );

        void signalContextCreated( fh_context subc,
                                   const std::string& data, bool isDataValid = true );

        void signalContextChanged( fh_context c, const std::string& data, bool isDataValid = true );
        
        /**
         * Tell other processes that a medallion has been updated.
         */
        void signalMedallionUpdated( fh_context c );

        /**
         * Tell other processes that a new item was created in the etagere
         */
        void signalEtagereNewEmblems( fh_etagere et, std::set< emblemID_t >& eset );
        
        /***************************/
        /*** singals ***************/
        /***************************/

        typedef sigc::signal1< void, std::string > MedallionUpdated_Sig_t;
        MedallionUpdated_Sig_t& getMedallionUpdated_Sig();

        
    private:
        MedallionUpdated_Sig_t MedallionUpdated_Sig;
    };
};
#endif
