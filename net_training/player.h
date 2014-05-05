#ifndef PLAYER_H
#define PLAYER_H

/*for sample*/
namespace net_training {
class session;

enum player_status {
	PS_CREATE = 0,
	PS_DELETED = 1
};

class player {
public:
	player(session* session);
	~player();

	void set_status(int status) { m_status = status; }

	bool is_valid()const { return m_status != PS_DELETED; }

	bool send(const char* data, int length);
private:
	int m_id;
	int m_status;
	session* m_session;
};
}
#endif