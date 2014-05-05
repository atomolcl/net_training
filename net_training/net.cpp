#include <process.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "net.h"
#include "session.h"
#include "session_mgr.h"
#include "overlapped_io.h"
#include "game_logic.h"
#include "internal_msg.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

static  unsigned WINAPI thread_function(void* param) {
	net_training::net_io* task = (net_training::net_io*)param;
	DWORD length, key;
	OVERLAPPED* overlapped = NULL;
	net_training::overlapped_io* o;
	BOOL ret;

	for (;;) {
		ret = GetQueuedCompletionStatus(task->iocp(), &length, &key, &overlapped, INFINITE);
		o = (net_training::overlapped_io*)overlapped;
		if (ret) {
			if (length > 0) {
				switch (o->m_op_type) {
				case net_training::OOT_ACCEPT:
					{
						CreateIoCompletionPort((HANDLE)o->m_socket, task->iocp(), 0, 0);
						SOCKET s = task->listen_socket();
						setsockopt(o->m_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&s, sizeof(task->listen_socket()));
						o->m_session = net_training::g_sm->create_session();
						o->m_session->init(o->m_socket);
						task->recv(o, length);		
						net_training::internal_msg msg;
						msg.type = make_msg_type(net_training::IMT_SYS, net_training::ISS_CONNECT);
						msg.dst = o->m_session;
						net_training::g_gl->post_msg(msg);
						break;
					}
				case  net_training::OOT_RECEIVE:
					{
						task->recv(o, length);
						break;
					}
				case  net_training::OOT_SEND:
					{
						task->send_complete(o, length);
						break;
					}
				default:
					assert(false);
					printf("OVERLAPPED_OP_TYPE is invalid\n");
				}
			} else if (length == 0) {
				//client is closed
				//printf("client graceful closed\n");
				task->close_socket(o);
			}
		} else {
			if (length == 0) {
				//printf("clinet closed[%d],[%p]\n", WSAGetLastError(), o->m_session);
				task->close_socket(o);
			} else {
				printf("GetQueuedCompletionStatus error[%d]", WSAGetLastError());
			}
		}
	}
	return 0;
}
namespace net_training {
net_io* g_nio = NULL;
net_io::net_io() {
	m_listen = INVALID_SOCKET;
	m_iocp = INVALID_HANDLE_VALUE;
	m_acceptex_fnt = NULL;
}

net_io::~net_io() {
	CloseHandle(m_iocp);
	closesocket(m_listen);
	WSACleanup();
	delete[] m_threads;
}

int net_io::init() {
	WSAData data;
	if (WSAStartup(MAKEWORD(2, 2), &data)) {
		printf("WSAStartup error\n");
		return -1;
	}
	/*set how to resolve packet*/
	session::set_packet_head(0, sizeof(int));

	if (create_iocp() == -1)	 return -1;
	if (create_listen() == -1) return -1;
	if (create_acceptex_handle() == -1) return -1;

	printf("server success!\n");
	for (int i = 0; i < m_thread_nums; ++i)
		WaitForSingleObject(m_threads[i], INFINITE);

	return 0;
}

void net_io::recv(overlapped_io* o, unsigned length) {
	/*fisrt step: receive data*/
	if (o->m_session->write_to(o->m_buff.buf, length) == false) {
		printf("WSARecv error=[%d]\n", WSAGetLastError());
		close_socket(o);
	}

	/*second step: post another WSARecv opt*/
	o->m_op_type = OOT_RECEIVE;
	DWORD btyes, flags = 0;
	int ret = WSARecv(o->m_socket, &o->m_buff, 1, &btyes, &flags, &o->m_lapped, NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
		printf("WSARecv error=[%d]\n", WSAGetLastError());
		close_socket(o);
	}
}

bool net_io::send(SOCKET s, const char* data, int length) {
	overlapped_io* o = net_training::g_oim->create_free_overlapped();
	if (o == NULL) return false;
	o->m_buff.buf = o->m_internal;
	o->m_op_type = OOT_SEND;

	memcpy(o->m_buff.buf, data, length);
	o->m_buff.len = length;
	DWORD bytes;
	int ret = WSASend(s, &o->m_buff, 1, &bytes, 0, &o->m_lapped, NULL);

	if (ret == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
		printf("send error[%d]", WSAGetLastError());
		return false;
	}

	return true;
}

void net_io::send_complete(overlapped_io* o, unsigned length) {
	if (o->m_buff.len == length) {
		g_oim->delete_free_overlapped(o);
	} else {
		printf("send_complete failed\n");
		assert(0);
	}
}

int net_io::create_listen() {
	m_listen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_listen == INVALID_SOCKET) {
		printf("create listen socket error\n");
		return -1;
	}
	
	int opt = 1;
	if (setsockopt(m_listen, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
		printf("setsockopt error\n");
		return -1;
	}

	sockaddr_in  address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = ADDR_ANY;
	address.sin_port = htons(12345);
	if (bind(m_listen, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR) {
		printf("bind error\n");
		return -1;
	}

	if (listen(m_listen, BACK_LOG) == SOCKET_ERROR) {
		printf("listen error\n");
		return -1;
	}

	if (CreateIoCompletionPort((HANDLE)m_listen, m_iocp, 0, 0) == NULL) {
		printf("create iocp bind listen error\n");
		return -1;
	}

	return 0;
}

int net_io::create_iocp() {
	m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_iocp == NULL) {
		printf("create iocp error\n");
		return -1;
	}

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	m_thread_nums = info.dwNumberOfProcessors;
	m_threads = new HANDLE[m_thread_nums];
	for (size_t i = 0; i < info.dwNumberOfProcessors; ++i) {
		m_threads[i] = (HANDLE)_beginthreadex(NULL, 0, thread_function, this, 0, NULL);
		CloseHandle(m_threads[i]);
	}

	return 0;
}

int net_io::create_acceptex_handle() {
	GUID acceptex = WSAID_ACCEPTEX;
	DWORD ret;
	if (WSAIoctl(	m_listen,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&acceptex,
		sizeof(acceptex),
		&m_acceptex_fnt,
		sizeof(m_acceptex_fnt),
		&ret,
		NULL,
		NULL) == SOCKET_ERROR) {
			printf("wsaioctl error=[%i]\n", WSAGetLastError());
			return -1;
	}

	return push_acceptexs();
}

int net_io::push_acceptexs() {
	for (size_t i = 0; i < MAX_SESSION; ++i) {
		overlapped_io* o = g_oim->create_free_overlapped();
		if (o == NULL) return -1;
		if (push_a_acceptex(o) == -1)
			return -1;
	}
	return 0;
}

int net_io::push_a_acceptex(overlapped_io* o) {
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (s == SOCKET_ERROR) {
		printf("create acceptex socket error\n");
		return -1;
	}

	o->m_socket = s;
	o->m_op_type = OOT_ACCEPT;
	o->m_buff.buf = o->m_internal;
	o->m_buff.len = sizeof(o->m_internal);
	/*situation that client not connect but iocp listened disconnect*/
	o->m_session = NULL;
	DWORD btyes = 0;
	BOOL ret = m_acceptex_fnt(m_listen, s, o->m_buff.buf, o->m_buff.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		&btyes, &o->m_lapped);
	if (ret == false) {
		int error = WSAGetLastError();
		if (error != ERROR_IO_PENDING) {
			printf("call acceptex_fnt error [%i]\n", error);
			g_oim->delete_free_overlapped(o);
			return -1;
		}
	}

	return 0;
}

void net_io::close_socket(overlapped_io* o) {
	if (o) {
		closesocket(o->m_socket);
		if (o->m_session) {
			/*situation that not accept, but iocp listened disconnect*/
			internal_msg msg;
			msg.type = make_msg_type(IMT_SYS, ISS_DISCONNECT);
			msg.dst = o->m_session;
			g_gl->post_msg(msg);
		}
		push_a_acceptex(o);
	}
}
}