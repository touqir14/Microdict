#include <string.h>
#include <stdbool.h>
#include <math.h>

/*
dtype : 1 refers to int32
dtype : 2 refers to int64
dtype : 3 refers to float32
dtype : 4 refers to float64
dtype : 5 refers to string
*/


typedef int32_t i_t;

#if dtype_key == 1
    typedef int32_t k_t;

    #if dtype_val == 1
        typedef int32_t v_t;

    #elif dtype_val == 2
        typedef int64_t v_t;

    #elif dtype_val == 3
        typedef float v_t;

    #elif dtype_val == 4
        typedef double v_t;

    #endif

    typedef k_t kbox_t;
    typedef v_t vbox_t;

    #define _hash_func int_hash
    #define _key_equal int_key_equal

    #define _get_key(h, idx) h->keys[idx]  
    #define _get_val(h, idx) h->vals[idx]  

    #define _set_key(h, idx, key) {h->keys[idx] = key;} 
    #define _set_val(h, idx, val) {h->vals[idx] = val;}  

    #define GET_PTR(idx, step_increment) idx

    #define _rehash_func rehash_int

#elif dtype_key == 2
    typedef int64_t k_t;

    #if dtype_val == 1
        typedef int32_t v_t;

    #elif dtype_val == 2
        typedef int64_t v_t;

    #elif dtype_val == 3
        typedef float v_t;

    #elif dtype_val == 4
        typedef double v_t;

    #endif

    typedef k_t kbox_t;
    typedef v_t vbox_t;

    #define _hash_func int_hash
    #define _key_equal int_key_equal

    #define _get_key(h, idx) h->keys[idx]  
    #define _get_val(h, idx) h->vals[idx]  

    #define _set_key(h, idx, key) {h->keys[idx] = key;} 
    #define _set_val(h, idx, val) {h->vals[idx] = val;}  

    #define GET_PTR(idx, step_increment) idx

    #define _rehash_func rehash_int

#endif

const double PEAK_LOAD = 0.79;

typedef struct
{
  int key_type;
  int key_size;
  int val_type;
  int val_size;
  i_t key_step_increment;
  i_t val_step_increment;
} ht_param;

typedef struct
{
    i_t num_buckets, size, upper_bound, k_t_size, v_t_size, key_str_len, val_str_len, k_step_increment, v_step_increment, seed;
    i_t *flags;
    i_t *psl;
    k_t *keys;
    v_t *vals;
    bool is_map;
} h_t;


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define int_hash(h, key_box) key_box
#define int_key_equal(h, i, key_box) (h->keys[i] == key_box)


inline bool _flags_isempty(i_t *flag, i_t i){

    return ((flag[i>>5]>>(i&0x1fU))&1);
}

inline void _flags_setFalse_isempty(i_t *flag, i_t i){

    flag[i>>5]&=~(1ul<<(i&0x1fU));
    return;
}


inline void _flags_setTrue_isempty(i_t *flag, i_t i){

    flag[i>>5]|=(1ul<<(i&0x1fU));
    return;
}


inline i_t _flags_size(i_t num_buckets){

    return (i_t) ceil(num_buckets / 32.0);
}

inline i_t _get_psl(i_t* psl_array, i_t i){
    return psl_array[i>>5];
}

inline void _set_psl(i_t* psl_array, i_t i, i_t psl_val){
    psl_array[i>>5] = psl_val;
}


void _print_psl_array(h_t* h) {
    printf("printing psl array..\n");
    int factor = 32;
    for (i_t i=0; i<(h->num_buckets >> 5); ++i){
        printf("idx:%d, psl_val:%d\n", i*factor, i*_get_psl(h->psl, i*factor));
    }
}

void _print_keys_int(h_t* h) {
    printf("printing keys....\n");
    for (i_t i=0; i<h->num_buckets; ++i){
        printf("Key idx:%d and key:%d\n", i, h->keys[i]);
    }
}

