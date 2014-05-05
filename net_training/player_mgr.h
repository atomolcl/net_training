#ifndef PLAYER_MGR_H
#define PLAYER_MGR_H

#include <map>

/*for sample*/
namespace net_training {
class player;
class session;

class player_mgr {
public:
	player* create_player(session* s);

	void delete_player(session* s);
	
	player* find_player(session* s);

	void run();
private:
	typedef std::map<session*, player*> players_container;
	void deferred_delete();
	players_container m_players;
};
}
#endif