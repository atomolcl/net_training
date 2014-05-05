#ifndef WIN_PLATFORM_H
#define WIN_PLATFORM_H

#include "windows.h"

namespace net_training {
class win_mutex_lock {
public:
	win_mutex_lock() { InitializeCriticalSection(&m_mutex); }
	~win_mutex_lock() { DeleteCriticalSection(&m_mutex); }
	void lock() { EnterCriticalSection(&m_mutex); }
	void unlock() { LeaveCriticalSection(&m_mutex); }
private:
	CRITICAL_SECTION m_mutex;
};

typedef win_mutex_lock mutex_lock;
}
#endif