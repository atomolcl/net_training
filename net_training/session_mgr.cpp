#include "session_mgr.h"
#include "session.h"

namespace net_training {
session_mgr* g_sm = NULL;

session* session_mgr::create_session() {
	return new session();
}

void session_mgr::delete_session(session* s) {
	delete s;
}
}