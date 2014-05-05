#include "thread.h"

#include <assert.h>
#include <process.h>

namespace net_training {
thread::thread(runnable* task)
	: m_task(task),
	m_id(0),
	m_join(true)
{
	assert(m_task);
	start();
}

thread::~thread() {
	CloseHandle(m_id);
}

bool thread::start() {
	m_id = (HANDLE)_beginthreadex(NULL, 0, &thread::spawn_thread, m_task, 0, NULL);
	if (m_id == (HANDLE)-1 || m_id == (HANDLE)0)
		return false;
	return true;
}

void thread::join() {
	m_join = false;
	WaitForSingleObject(m_id, INFINITE);
}

void thread::deattach() {
	m_join = false;
}

bool thread::joinable() const {
	return m_join;
}

unsigned  WINAPI thread::spawn_thread(void* param) {
	runnable* task = (runnable*)param;
	task->run();
	return 0;
}
}