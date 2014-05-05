#include <stdio.h>

#include "player.h"
#include "session.h"
#include "session_mgr.h"

namespace net_training {
player::player(session* s): m_session(s){
	//printf("player login\n");
	m_id = 0;
	const char* string = "wolaile";
	send(string, strlen(string));
}

player::~player() {
	//printf("player logout\n");
	g_sm->delete_session(m_session);
}

bool player::send(const char* data, int length) {
	if (is_valid() == false) return false;
	return m_session->send(data, length);
}
}