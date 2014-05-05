#ifndef OVERLAPPED_IO_H
#define OVERLAPPED_IO_H

#include <stdint.h>
#include <winsock2.h>

#include "lock.h"

namespace net_training {
class session;
enum OVERLAPPED_OP_TYPE {
	OOT_ACCEPT,
	OOT_RECEIVE,
	OOT_SEND,
	OOT_INVALID
};

struct overlapped_io {
	WSAOVERLAPPED m_lapped;
	WSABUF m_buff;
	char m_internal[4096];
	int8_t m_op_type;
	SOCKET m_socket;
	session* m_session;
};

class overlapped_io_mgr {
public:
	overlapped_io_mgr();

	overlapped_io* create_free_overlapped();
	void delete_free_overlapped(overlapped_io* o);
private:
	static const int OVERLAPPED_MAX_NUM = 0x1000;
	overlapped_io* m_overlappeds;
	overlapped_io* m_free_overlapped;
	mutex_lock m_mutex;
};

extern overlapped_io_mgr* g_oim;
}
#endif