#ifndef SESSION_MGR_H
#define SESSION_MGR_H

//#include "lock.h"

/*memory optimization*/
namespace net_training {
class session;
class session_mgr {
public:
	session* create_session();

	void delete_session(session* s);
private:
	//mutex_lock m_mutex;
};
extern session_mgr* g_sm;
}
#endif