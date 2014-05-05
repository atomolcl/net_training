#include <stdio.h>

#include "net.h"
#include "game_logic.h"
#include "session_mgr.h"
#include "overlapped_io.h"
// namespace net_training {
// class test_runnable : public runnable {
// public:
// 	virtual void run() {
// 		for (;;) {
// 
// 		}
// 	}
// };
// }

void create_global_obj() {
	net_training::g_gl = new net_training::game_logic();
	net_training::g_sm = new net_training::session_mgr();
	net_training::g_oim = new net_training::overlapped_io_mgr();
	net_training::g_nio = new net_training::net_io();
}

void delete_global_obj() {
	delete net_training::g_gl;
	delete net_training::g_sm;
	delete net_training::g_oim;
	delete net_training::g_nio;
}

int main() {
	create_global_obj();
	net_training::thread gl_thread(net_training::g_gl);
	net_training::g_nio->init();
	gl_thread.join();
	delete_global_obj();
	printf("main thread over\n");
	return 0;
}