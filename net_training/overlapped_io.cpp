#include <limits.h>
#include <assert.h>
#include <stdio.h>

#include "overlapped_io.h"
#include "session.h"

namespace net_training {
overlapped_io_mgr* g_oim = NULL;

overlapped_io_mgr::overlapped_io_mgr() {
	m_overlappeds = new overlapped_io[OVERLAPPED_MAX_NUM];

	for (size_t i = 0; i < OVERLAPPED_MAX_NUM - 1; ++i) {
		m_overlappeds[i].m_socket = i + 1;
	}

	m_overlappeds[OVERLAPPED_MAX_NUM-1].m_socket = UINT_MAX;
	m_free_overlapped = m_overlappeds;
}

overlapped_io* overlapped_io_mgr::create_free_overlapped() {
	scope_lock lock(m_mutex);
	overlapped_io* ret = NULL;
	if (m_free_overlapped) {
		ret = m_free_overlapped;
		overlapped_io* next = &m_overlappeds[m_free_overlapped->m_socket];
		m_free_overlapped = next->m_socket == UINT_MAX ? NULL : next;
		memset(&ret->m_lapped, 0, sizeof(ret->m_lapped));
	} else {
		printf("not enough overlappeds\n");
		assert(0);
	}

	return ret;
}

void overlapped_io_mgr::delete_free_overlapped(overlapped_io* o) {
	scope_lock lock(m_mutex);
	overlapped_io* pref = m_free_overlapped;
	m_free_overlapped = o;
	m_free_overlapped->m_socket = pref ? pref - m_overlappeds : UINT_MAX;
}
}