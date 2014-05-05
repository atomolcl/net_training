#ifndef LOCK_H
#define LOCK_H

#ifdef WIN32
#include "win_platform.h"
#else
#include "linux_platform.h"
#endif

#pragma warning(push)
#pragma warning(disable:4512)
namespace net_training {
class scope_lock {
public:
	scope_lock(mutex_lock& lock) : m_lock(lock) { m_lock.lock(); }
	~scope_lock() { m_lock.unlock(); }
private:
	mutex_lock& m_lock;
};
}
#pragma warning(pop)
#endif