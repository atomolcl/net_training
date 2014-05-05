#ifndef LINUX_PLATFORM_H
#define LINUX_PLATFORM_H

namespace net_training {
class linux_mutex_lock {
public:
	linux_mutex_lock() { pthread_mutex_init(&m_mutex, NULL); }
	~linux_mutex_lock() { phtread_mutex_destroy(&m_mutex); }
	void lock() { pthread_mutex_lock(&m_mutex); }
	void unlock() { pthread_mutex_unlock(&m_mutex); }
private:
	pthread_mutex_t m_mutex;
};

typedef linux_mutex_lock mutex_lock;
}
#endif
