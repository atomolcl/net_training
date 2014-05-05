#ifndef INTERNAL_MSG_H
#define INTERNAL_MSG_H

#include <stdint.h>

namespace net_training {
#define  INTERNAL_MSG_TYPE_SHIFT 24
#define  make_msg_type(t, s) (t << INTERNAL_MSG_TYPE_SHIFT | s)
#define mask_msg_type_high(t) ((t >> INTERNAL_MSG_TYPE_SHIFT) & ((1 << (sizeof(t)*8 - INTERNAL_MSG_TYPE_SHIFT))-1))
#define  mask_msg_type_low(t) (t & (1 << INTERNAL_MSG_TYPE_SHIFT) - 1)

enum internal_msg_type {
	IMT_SYS = 1,
	IMT_NET = 2
};

enum imt_sys_sub {
	ISS_CONNECT = 1,
	ISS_DISCONNECT = 2
};

struct internal_msg {
	int type;
	void* dst;
	void* data;
	//size_t size;

	internal_msg() {
		data = 0;
	}
};

struct net_msg_head {
	int size;
	int op_type;
};
}
#endif