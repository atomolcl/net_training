#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "session.h"
#include "net.h"
#include "internal_msg.h"
#include "game_logic.h"

namespace net_training {
int io_ctl::m_offset = 0;
int io_ctl::m_offset_size = 0;

void io_ctl::set_packet_head(int offset, int offset_size) {
	m_offset = offset;
	m_offset_size = offset_size;
}

io_ctl::io_ctl()
	: m_write(NULL),
	m_read_pos(0),
	m_write_pos(0)
{
	m_datas = new io_buff();
	m_read = m_write = m_datas;
}

io_ctl::~io_ctl() {
	io_buff* cur = m_datas;
	io_buff* next;
	while (cur) {
		next = cur->m_next;
		delete cur;
		cur = next;
	}
}

io_buff* io_ctl::create_io_buff() {
	if (m_write->m_next == NULL) {
		m_write->m_next = new io_buff();
	}
	m_write = m_write->m_next;

	return m_write;
}

void io_ctl::free_io_buff(io_buff* b) {
	b->m_next = NULL;
	m_write->m_next = b;
}

bool io_ctl::write_to(session* s, const char* data, int length) {
	copy_data(m_write->m_data, m_write_pos, data, length);
	return complete_one_packet(s);
}

bool io_ctl::complete_one_packet(session* s) {
	char* begin = &m_read->m_data[m_read_pos] + m_offset;
	int sizes; /*the size in packet head*/
	if (m_offset_size == 4)
		sizes = *(int*)begin;
	else if (m_offset_size == 2)
		sizes = *(short*)begin;
	else {
		printf("error packet head sizes\n");
		return false;
	}

	if (sizes <= 0 || sizes > IO_BUFF_DATA_SIZE) return false;

	int readed_sizes = 0;
	io_buff* begin_read = m_read;
	while (begin_read != m_write) {
		readed_sizes =  IO_BUFF_DATA_SIZE - begin_read->m_data[m_read_pos];
		begin_read = begin_read->m_next;
	}

	if (m_read == m_write) {
		readed_sizes += m_write_pos - m_read_pos;
	} else {
		readed_sizes += m_write_pos;
	}

	/*a complete packet is recived, so copy it to extra buff when extra buff is readed*/
	if (readed_sizes >= sizes) {
		scope_lock lock(m_recv_mutex);
		if (m_complete_data.m_next == NULL) {
			begin_read = m_read;
			int offset = 0;
			for (;;) {
				/*only one loop*/
				if (m_read_pos + sizes < IO_BUFF_DATA_SIZE) {
					copy_data(m_complete_data.m_data, offset, begin_read->m_data + m_read_pos, sizes);
					m_read_pos += sizes;
					m_read = begin_read;
					break;
				} else {
					int little_done = IO_BUFF_DATA_SIZE - m_read_pos;
					copy_data(m_complete_data.m_data, offset, begin_read->m_data + m_read_pos, little_done);
					sizes -= little_done;
					m_read_pos = 0;
					io_buff* next = begin_read->m_next;
					assert(next);
					free_io_buff(begin_read);
					begin_read = next;
				}
			}

			m_complete_data.m_next = (io_buff*)0xcc; /*not null is prepare to read*/
			/*to send a msg to game logic*/
			internal_msg msg;
			msg.type = make_msg_type(IMT_NET, 0);
			msg.dst = s;
			msg.data = m_complete_data.m_data;
			g_gl->post_msg(msg);
		}
	}

	return true;
}

char* io_ctl::begin_read() {
	scope_lock lock(m_recv_mutex);
	return m_complete_data.m_next ? m_complete_data.m_data : NULL;
}

void io_ctl::finish_read() {
	scope_lock lock(m_recv_mutex);
	m_complete_data.m_next = NULL;
}

void io_ctl::copy_data(char* dst, int& offset, const char* src, int src_length) {
	int done = 0, little_done;
	for (;;) {
		if (offset + src_length < IO_BUFF_DATA_SIZE) {
			memcpy(dst + offset, src, src_length);
			offset += src_length;
			return;
		} else {
			little_done = IO_BUFF_DATA_SIZE - offset;
			memcpy(dst + offset, src + done, little_done);
			done += little_done;
			src_length -= little_done;
			dst = create_io_buff()->m_data;
			offset = 0;
		}
	}
}

void session::set_packet_head(int offset, int offset_size) {
	io_ctl::set_packet_head(offset, offset_size);
}

session::~session() {
}

void session::init(SOCKET s) {
	m_socket = s;
}

bool session::write_to(const char* data, int length) {
	return m_io.write_to(this, data, length);
}

char* session::begin_read() {
	return m_io.begin_read();
}

void session::finish_read() {
	m_io.finish_read();
}

bool session::send(const char* data, int length) {
	return g_nio->send(m_socket, data, length);
}
}