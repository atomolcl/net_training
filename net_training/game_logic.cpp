#include "session.h"
#include "game_logic.h"
#include "player.h"
#include "player_mgr.h"

#include <assert.h>
#include <stdio.h>


namespace net_training {
game_logic* g_gl = NULL;

game_logic::game_logic() {
	m_player_mgr = new player_mgr();
}

game_logic::~game_logic() {
	delete m_player_mgr;
}

void game_logic::run() {
	for (;;) {
		process_msg();
		m_player_mgr->run();
		Sleep(1);
	}
}

void game_logic::post_msg(const internal_msg& msg) {
	scope_lock lock(m_mutex);
	m_contain.push(msg);
}

void game_logic::process_msg() {
	scope_lock lock(m_mutex);

	internal_msg msg;
	for (;;) {
		if (m_contain.pop(msg)) {
			switch (mask_msg_type_high(msg.type)) {
			case IMT_SYS:
				process_sys_msg(msg);
				break;
			case IMT_NET:
				process_net_msg(msg);
				break;
			default:
				printf("wrong mask_msg_type_high msg\n");
				assert(0);
			}
		} else {
			Sleep(50);
		}
	}
}

void game_logic::process_sys_msg(const internal_msg& msg) {
	switch (mask_msg_type_low(msg.type)) {
	case ISS_CONNECT:
		{
			session* s = (session*)msg.dst;
			player* p = m_player_mgr->create_player(s);
			p->set_status(PS_CREATE);
			break;
		}
	case ISS_DISCONNECT:
		{
			session* s = (session*)msg.dst;
			m_player_mgr->delete_player(s);
			break;
		}
	default:
		printf("wrong mask_msg_type_low msg\n");
		assert(0);
	}
}

void game_logic::process_net_msg(const internal_msg& msg) {
	session* s = (session*)msg.dst;
	net_msg_head* head = (net_msg_head*)msg.data;
	if (head->op_type != 10)
		printf("size=[%d], op_type=[%d]\n", head->size, head->op_type);
	s->finish_read();
}
}