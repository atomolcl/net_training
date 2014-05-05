#include "dequeue.h"

namespace net_training {
#define DEQUEQUE_MAX 4096

internal_dequeque::internal_dequeque()
{
	m_msgs = new internal_msg[DEQUEQUE_MAX];
	m_head = 0;
	m_tail = 0;
	m_cap = DEQUEQUE_MAX;
}

internal_dequeque::~internal_dequeque() {
	delete[] m_msgs;
}

void internal_dequeque::push_back(const internal_msg& msg) {
	m_msgs[m_tail++] = msg;
	if (m_tail >= m_cap)
		m_tail = 0;

	if (m_head == m_tail)
		expand();
}

void internal_dequeque::expand() {
	internal_msg* new_q = new internal_msg[m_cap * 2];
	for (int i = 0; i < m_cap; ++i)
		new_q[i] = m_msgs[(m_head + i) % m_cap];

	delete[] m_msgs;
	m_head = 0;
	m_tail = m_cap;
	m_cap *= 2;
	m_msgs = new_q;
}

internal_msg internal_dequeque::front() const{
	return m_msgs[m_head];
}

void internal_dequeque::pop_front() {
	++m_head;
	if (m_head >= m_cap)
		m_head = 0;
}

bool internal_dequeque::empty()const {
	return m_head == m_tail;
}

void dequeue::push(const internal_msg& msg) {
	m_contain.push_back(msg);
}

bool dequeue::pop(internal_msg& msg) {
	if (m_contain.empty()) return false;
	msg = m_contain.front();
	m_contain.pop_front();
	return true;
}
}