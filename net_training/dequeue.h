#ifndef DEQUEUE_H
#define DEQUEUE_H

#include "internal_msg.h"

namespace net_training { 
	class internal_dequeque; 
}

#ifdef USE_INTERNAL_DEQUEUE
typedef net_training::internal_dequeque deque_c;
#else
#include <deque>
typedef std::deque<net_training::internal_msg> deque_c;
#endif

namespace net_training {

class internal_dequeque {
public:
	internal_dequeque();
	~internal_dequeque();

	void push_back(const internal_msg& msg);

	internal_msg front()const;

	void pop_front();

	bool empty()const;
private:
	void expand();
	int m_head;
	int m_tail;
	int m_cap;
	internal_msg* m_msgs;
};

class dequeue {
public:
	void push(const internal_msg& msg);

	bool pop(internal_msg& msg);
private:
	deque_c m_contain;
};
}
#endif