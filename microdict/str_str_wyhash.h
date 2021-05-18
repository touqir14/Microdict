#include "wyhash.h"
#include "_string.h"

static inline uint64_t _hash_func(h_t *h, kbox_t key_box) {
	return wyhash(key_box.str, key_box.len, h->seed, _wyp);
}


#include "mdict_ht.h"
