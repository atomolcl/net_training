#ifndef NET_H
#define NET_H

#include "winsock2.h"
#include "mswsock.h"

namespace net_training {
class overlapped_io_mgr;
struct overlapped_io;

class net_io {
public:
	net_io();
	~net_io();

	int init();

	HANDLE iocp()const { return m_iocp; }
	
	SOCKET listen_socket()const { return m_listen; }
	
	void recv(overlapped_io* o, unsigned length);

	bool send(SOCKET s, const char* data, int length);

	void send_complete(overlapped_io* o, unsigned length);
	
	void close_socket(overlapped_io* o);
private:
	int create_listen();
	int create_iocp();
	int create_acceptex_handle();
	int push_acceptexs();
	int push_a_acceptex(overlapped_io* o);
	SOCKET m_listen;
	HANDLE m_iocp;
	HANDLE* m_threads;
	int m_thread_nums;
	LPFN_ACCEPTEX m_acceptex_fnt;
	static const int BACK_LOG = 128;
	static const int MAX_SESSION = 1024;
};

extern net_io* g_nio;
}

#endif