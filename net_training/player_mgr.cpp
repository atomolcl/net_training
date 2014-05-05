#include <stdio.h>

#include "player_mgr.h"
#include "session.h"
#include "player.h"

namespace net_training {
player* player_mgr::create_player(session* s) {
	player* p = new player(s);
	m_players.insert(players_container::value_type(s, p));
	return p;
}

void player_mgr::delete_player(session* s) {
	players_container::iterator it = m_players.find(s);
	if (it != m_players.end()) {
		player* p = it->second;
		p->set_status(PS_DELETED);
	} else {
		printf("delete_player error\n");
	}
}

player* player_mgr::find_player(session* s) {
	players_container::iterator it = m_players.find(s);
	if (it != m_players.end()) {
		return it->second;
	} else {
		return NULL;
	}
}

void player_mgr::run() {
	/*last run*/
	deferred_delete();
}

void player_mgr::deferred_delete() {
	players_container::iterator it = m_players.begin();
	player* p;
	while (it != m_players.end()) {
		p = it->second;
		if (p->is_valid() == false) {
			delete p;
			m_players.erase(it++);
		} else {
			++it;
		}
	}
}
}