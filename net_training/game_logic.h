#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "thread.h"
#include "lock.h"
#include "dequeue.h"

namespace net_training {
struct internal_msg;
class player_mgr;

class game_logic : public runnable {
public:
	game_logic();
	~game_logic();

	virtual void run();

	void post_msg(const internal_msg& msg);
private:
	void process_msg();
	void process_sys_msg(const internal_msg& msg);
	void process_net_msg(const internal_msg& msg);
	mutex_lock m_mutex;
	dequeue m_contain;
	player_mgr* m_player_mgr;
};
extern game_logic* g_gl;
}

#endif