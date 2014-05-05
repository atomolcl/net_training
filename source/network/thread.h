#ifndef THREAD_H
#define THREAD_H

#include <Windows.h>

namespace net_training {
/*implement run method to a thread*/
class runnable {
public:
	virtual void run() = 0;
};


class thread {
public:
	thread(runnable* task);
	~thread();

	HANDLE id() const { return m_id; }
	
	bool start();

	void join();

	void deattach();

	bool joinable() const;

private:
	static  unsigned WINAPI spawn_thread(void* param);
	runnable* m_task;
	bool m_join;
	HANDLE m_id;
};
}
#endif