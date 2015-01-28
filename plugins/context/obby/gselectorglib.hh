#include <map>
#include <net6/socket.hpp>

#include <sigc++/sigc++.h>
#include <sigc++/slot.h>
#include <sigc++/object.h>
#include <sigc++/object_slot.h>

class GSelectorGlib: public sigc::trackable
{
public:
    struct ChannelWrapper 
    {
        GIOChannel* m_chan;
        int m_channelID;
        
        ChannelWrapper( GIOChannel* chan = 0 )
            :
            m_chan( chan ),
            m_channelID( 0 )
            {
            }
        ~ChannelWrapper()
            {
                free();
            }
        void free()
            {
                if( m_channelID )
                {
                    g_source_remove( m_channelID );
                }
                m_channelID = 0;
                m_chan = 0;
            }
        void takeOwnership( GIOChannel* c, int channelID )
            {
                free();
                m_chan = c;
                m_channelID = channelID;
            }
    };
    
	struct SelectedSocket {
		sigc::connection conn;
		const net6::socket* sock;
//		Glib::RefPtr<Glib::IOChannel> chan;
        ChannelWrapper chan;
		net6::io_condition cond;
        GSelectorGlib* parent;
	};

	~GSelectorGlib();

	net6::io_condition get(const net6::socket& sock) const;
	void set(const net6::socket& sock, net6::io_condition cond);
	net6::io_condition check(const net6::socket& sock,
	                         net6::io_condition mask) const;

protected:
	typedef std::map<const net6::socket*, SelectedSocket> map_type;

	void add_socket(const net6::socket& sock, net6::io_condition cond);
	void modify_socket(map_type::iterator iter, net6::io_condition cond);
	void delete_socket(map_type::iterator iter);

    map_type m_map;
public:
	bool on_io( GIOCondition cond, const net6::socket* sock) const;
	unsigned long get_timeout(const net6::socket& sock) const
        {
            return 0;
        }
    void set_timeout(const net6::socket& sock, unsigned long timeout)
        {
        }
    
};


	inline GIOCondition gcond(net6::io_condition cond)
	{
// G_IO_IN | G_IO_ERR | G_IO_PRI        
		GIOCondition g_cond = GIOCondition(0);
		if(cond & net6::IO_INCOMING)
			g_cond = (GIOCondition)( g_cond | G_IO_IN );
		if(cond & net6::IO_OUTGOING)
			g_cond = (GIOCondition)( g_cond | G_IO_OUT);
		if(cond & net6::IO_ERROR)
			g_cond = (GIOCondition)( g_cond | (G_IO_HUP | G_IO_NVAL | G_IO_ERR));
		return g_cond;
	}

	inline net6::io_condition ncond(GIOCondition cond)
	{
		net6::io_condition n_cond = net6::IO_NONE;
		if(cond & G_IO_IN)
			n_cond |= net6::IO_INCOMING;
		if(cond & G_IO_OUT)
			n_cond |= net6::IO_OUTGOING;
		if(cond & (G_IO_HUP | G_IO_NVAL | G_IO_ERR) )
			n_cond |= net6::IO_ERROR;
		return n_cond;
	}


GSelectorGlib::~GSelectorGlib()
{
	// Should already be performed by sigc::trackable...
	for(map_type::iterator it = m_map.begin(); it != m_map.end(); ++ it)
		it->second.conn.disconnect();
}

net6::io_condition GSelectorGlib::get(const net6::socket& sock) const
{
	map_type::const_iterator iter = m_map.find(&sock);

	if(iter == m_map.end() )
		return net6::IO_NONE;
	else
		return iter->second.cond;
}

gboolean
data_available_cb( GIOChannel *source,
                         GIOCondition condition,
                         gpointer user_data )
{
    GSelectorGlib::SelectedSocket* g = (GSelectorGlib::SelectedSocket*)user_data;
    g->parent->on_io( condition, g->sock );
}

void GSelectorGlib::add_socket(const net6::socket& sock,
                                  net6::io_condition cond)
{
    cerr << "GSelectorGlib::add_socket()" << endl;
	SelectedSocket& sel = m_map[&sock];

	net6::socket::socket_type fd = sock.cobj();

    GIOChannel* chan = g_io_channel_unix_new( fd );
    int conn = g_io_add_watch( chan, gcond(cond),
                               data_available_cb, &sel );
    sel.chan.takeOwnership( chan, conn );
    
    sel.sock = &sock;
	sel.cond = cond;
    sel.parent = this;
}

void GSelectorGlib::modify_socket(map_type::iterator iter,
                                     net6::io_condition cond)
{
	// Flags are already set
	if(iter->second.cond == cond)
		return;
    cerr << "GSelectorGlib::modify_socket()" << endl;

	iter->second.cond = cond;
    
    SelectedSocket& sel = iter->second;
    GIOChannel* chan = sel.chan.m_chan;
    int conn = g_io_add_watch( chan, gcond(cond),
                               data_available_cb, &sel );
    sel.chan.takeOwnership( chan, conn );
}

void GSelectorGlib::delete_socket(map_type::iterator iter)
{
	iter->second.conn.disconnect();
	m_map.erase(iter);
}

void GSelectorGlib::set(const net6::socket& sock, net6::io_condition cond)
{
	map_type::iterator iter = m_map.find(&sock);

	if(cond != net6::IO_NONE)
		if(iter == m_map.end() )
			add_socket(sock, cond);
		else
			modify_socket(iter, cond);
	else if(iter != m_map.end() )
		delete_socket(iter);
}

bool GSelectorGlib::on_io( GIOCondition cond,
                           const net6::socket* sock) const
{
	map_type::const_iterator iter = m_map.find(sock);

	// Has been removed by previous handler
	if(iter == m_map.end() ) return false;

	// Occured condition has been removed by previous handler
	if( (gcond(iter->second.cond) & cond) == gcond(net6::IO_NONE))
		return true;

	sock->io_event().emit(ncond(cond) );
	return true;
}
