#ifndef SESSION_H
#define SESSION_H

#include "winsock2.h"
#include "lock.h"

namespace net_training {
#define  IO_BUFF_DATA_SIZE 4096
struct io_buff {
	char m_data[IO_BUFF_DATA_SIZE];
	io_buff* m_next;

	io_buff() {
		m_next = NULL;
	}
};

struct overlapped_io;
class session;

class io_ctl {
public:
	io_ctl();
	~io_ctl();

	io_buff* create_io_buff();

	void free_io_buff(io_buff* b);

	bool write_to(session* s, const char* data, int length);

	bool complete_one_packet(session* s);

	char* begin_read();

	void finish_read();

	static void set_packet_head(int offset, int offset_size);
private:  
	/*@offset is the position of packet head size
		@offset_size is the length of @offset*/
	static int m_offset, m_offset_size;

	void copy_data(char* dst, int& offset, const char* src, int src_length);

	overlapped_io* create_send_io(SOCKET s);

	io_buff* m_datas;
	io_buff* m_write; /*point to write*/
	int m_write_pos; /*m_write position's offset*/
	io_buff* m_read; /*point to read*/
	int m_read_pos;  /*m_read position's offset*/
	io_buff m_complete_data; /*to up logic*/
	mutex_lock m_recv_mutex;
};


class session {
public:
	~session();

	void init(SOCKET s);
	/*write from iocp to session's buffer*/
	bool write_to(const char* data, int length);
	/*game logic app level's read api*/
	char* begin_read();

	void finish_read();
	/*player call it*/
	bool send(const char* data, int length);
	/*set how to resolve packet*/
	static void set_packet_head(int offset, int offset_size);
private:
	io_ctl m_io;
	SOCKET m_socket;
};
}
#endif