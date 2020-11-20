#include <stdbool.h>
#include <stdint.h>

#define FLAG_POP_ARG_EXC 1 // Controls whether the (pop function) key length check failure issues an exception (if set) or just returns None (if unset). Default: set
#define FLAG_POP_RET_EXC (1 << 1) // Controls whether the (pop function) issues an exception (if set) or just returns None (if unset) when a key is not found. Default: set
#define FLAG_CLEAR_RET_EXC (1 << 2) // Controls whether the (clear function) issues an exception (if set) or just skips when a key is not of the right type. Default: unset
#define FLAG_UPDATE_ARG_EXC (1 << 3) // Controls whether the (_update_from_Pydict function) issues an exception (if set) or just skips when a key or value of the Python Dictionary is not of the right type. Default: unset
#define FLAG_PYDICT_ARG_EXC (1 << 4) // If set it raises an exception and deletes the dictionary before returning whenever there is any error. Default: unset
#define FLAG_GET_RET_EXC (1 << 5) // If set then invoking d[k] from python (where d is a microdict) will raise an exception whenever k is not inside d. Otherwise, a None will be returned. Default: unset

bool _get_flag(uint32_t flags, uint32_t flag_code) {
    return (bool) (flags & flag_code);
}

void _set_flag(uint32_t* flags, uint32_t flag_code, bool flag_value) {
    *flags = *flags & (~flag_code);
    if (flag_value)
        *flags = *flags | flag_code;
}
