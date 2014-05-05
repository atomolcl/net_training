#include <Winsock2.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_SOCKET 1000
SOCKET s[MAX_SOCKET];
struct net_msg_head {
	int size;
	int op_type;
};

struct message:public net_msg_head {
	char a[4];
	int time;

	message() {
		size = sizeof(*this);
	}
};

int main() {
	WSAData data;
	WSAStartup(MAKEWORD(2, 2), &data);
	for (int i = 0; i < MAX_SOCKET; ++i) {
		s[i] = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr("127.0.0.1");
		address.sin_port = htons(12345);
		if (WSAConnect(s[i], (sockaddr*)&address, sizeof(address), 0, 0, 0, 0) == SOCKET_ERROR) {
			printf("WSAConnect error=[%d]\n", WSAGetLastError());
			printf("i=[%d]", i);
			getchar();
			return -1;
		}
	}

	printf("create done\n");


	int i = 0;
	message msg;
	msg.op_type = 10;
	strcpy(msg.a, "abc");
	msg.time = i;
	char buffer[1024];
	WSABUF buff;
	buff.buf = buffer;
	buff.len = sizeof(msg);
	DWORD btyes;
	for (;;) {
		for (int j = 0; j < MAX_SOCKET; ++j) {
			msg.time = ++i;
			memcpy(buffer, &msg, sizeof(msg));
			WSASend(s[j], &buff, 1, &btyes, 0, NULL, NULL);
		}
		Sleep(500);
	}
	for (int i = 0; i < MAX_SOCKET; ++i)
		closesocket(s[i]);
	
	WSACleanup();
	return 0;
}