#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define dtype_key 5
#define dtype_val 5
#define str_len_SIZE 2 
#define str_len_MAX 65535

#define GET_PTR(idx, step_inc) idx*step_inc


#define _rehash_func rehash_str

typedef char k_t;
typedef char v_t;

typedef uint16_t str_len_t;
typedef struct kbox_t
{
	char* str;
	int len;
} kbox_t;

typedef kbox_t vbox_t;


#include "hash_funcs.h"

inline uint64_t _hash_func(h_t *h, kbox_t key_box);

void print_str(char* str, int len){
    printf("Str of len: %d ---", len);
    for (int i=0; i<len; ++i)
        printf("%c",str[i]);
    printf("\n");
}

inline void _strncpy(char* dest, char* src, int len){
	for (int i=0; i<len; ++i){
		dest[i] = src[i];
	}
}

inline bool _strncmp(char* s1, char* s2, int len) {
	for (int i=0; i<len; ++i){
		if (s1[i] != s2[i]) {
			return false;
		}
	}
	return true;
}


inline str_len_t _get_str_len(char* block_ptr){
	return ((str_len_t*) block_ptr)[0]; 
}

inline void _set_str_len(char* block_ptr, str_len_t len){
	((str_len_t*) block_ptr)[0] = len;
}


inline bool _key_equal(h_t *h, i_t idx, kbox_t key_box) {
	str_len_t h_key_len = _get_str_len(&h->keys[idx]);
	bool found;
	idx += str_len_SIZE;

	if (h_key_len != key_box.len || h_key_len == 0)
		found = false;
	else
		found = _strncmp(&h->keys[idx], key_box.str, h_key_len);

	return found;
}


inline kbox_t _get_key(h_t *h, i_t idx) {
	str_len_t len = _get_str_len(&h->keys[idx]);
	idx += str_len_SIZE;
	return (kbox_t) {&h->keys[idx], len};   
}  


inline vbox_t _get_val(h_t *h, i_t idx) {  
	str_len_t len = _get_str_len(&h->vals[idx]);
	idx += str_len_SIZE;
	return (vbox_t) {&h->vals[idx], len};   
}




inline int _set_key(h_t *h, i_t idx, kbox_t key) {
	/*
	Returns 0 on success. -1 if the string provided is larger than str_len_MAX.
	*/

	if (key.len <= str_len_MAX) {
		_set_str_len(&h->keys[idx], key.len);
		idx += str_len_SIZE;
		_strncpy(&h->keys[idx], key.str, key.len);
		return 0; 
	} else {
		return -1;
	}
} 


inline int _set_val(h_t *h, i_t idx, vbox_t val) {
	/*
	Returns 0 on success. -1 if the string provided is larger than str_len_MAX.
	*/

	if (val.len <= str_len_MAX) {
		_set_str_len(&h->vals[idx], val.len);
		idx += str_len_SIZE;
		_strncpy(&h->vals[idx], val.str, val.len);
		return 0; 
	} else {
		return -1;
	}
}  


void rehash_str(h_t* h, i_t* new_flags, i_t* new_psl, i_t new_num_buckets) {
	i_t new_mask = new_num_buckets - 1;		
	i_t i_ptr, k_step_inc = h->k_step_increment, v_step_inc = h->v_step_increment;
	i_t* visit_array = (i_t*) calloc(h->num_buckets, sizeof(i_t));
	i_t last_visited;
	bool loop_present=false;
	kbox_t temp_k_box;
	vbox_t temp_v_box;

	temp_k_box.str = (char*) malloc(sizeof(char)*h->k_t_size);
	temp_v_box.str = (char*) malloc(sizeof(char)*h->v_t_size);

	for (i_t j = 0; j < h->num_buckets; ++j) {						
		if (!_flags_isempty(h->flags, j)) {					
			kbox_t key = _get_key(h, GET_PTR(j, k_step_inc));
			_flags_setTrue_isempty(h->flags, j);				
			last_visited = 0;
			visit_array[last_visited] = j;

			// Forward pass
			while (1) { 
				i_t i, step = 0;
				i = _hash_func(h, key) & new_mask;							
				i_t last = i;
				i_t psl_val = _get_psl(new_psl, last);								
				
				while (!_flags_isempty(new_flags, i)) {
					i = (i + (++step)) & new_mask; 
				}
				_flags_setFalse_isempty(new_flags, i);			
				if (step > psl_val)
					_set_psl(new_psl, last, step);

				last_visited += 1;
				visit_array[last_visited] = i;
				if (i < h->num_buckets && !_flags_isempty(h->flags, i)) { 
					i_ptr = GET_PTR(i, k_step_inc);
					key = _get_key(h, i_ptr);
					_flags_setTrue_isempty(h->flags, i); 
				} else {  
					break;										
				}												
			}				

			if ((last_visited > 1) && (visit_array[0] == visit_array[last_visited])) {
				loop_present = true;
				i_t i = visit_array[0];				
				
				i_ptr = GET_PTR(i, k_step_inc);
				kbox_t key = _get_key(h, i_ptr);
				strncpy(temp_k_box.str, key.str, key.len);
				temp_k_box.len = key.len;
				
				i_ptr = GET_PTR(i, v_step_inc);
				vbox_t val = _get_val(h, i_ptr);
				strncpy(temp_v_box.str, val.str, val.len);
				temp_v_box.len = val.len;
			}

			// Backward pass
			while (last_visited > 1){
				kbox_t src_key = _get_key(h, GET_PTR(visit_array[last_visited-1], k_step_inc));
				_set_key(h, GET_PTR(visit_array[last_visited], k_step_inc), src_key);
				vbox_t src_val = _get_val(h, GET_PTR(visit_array[last_visited-1], v_step_inc));
				_set_val(h, GET_PTR(visit_array[last_visited], v_step_inc), src_val);
				last_visited -= 1;
			}

			kbox_t src_key;
			vbox_t src_val;
			if (!loop_present){
				src_key = _get_key(h, GET_PTR(visit_array[0], k_step_inc));
				src_val = _get_val(h, GET_PTR(visit_array[0], v_step_inc));
			} else {
				src_key = temp_k_box;
				src_val = temp_v_box;
				loop_present = false;
			}

			_set_key(h, GET_PTR(visit_array[1], k_step_inc), src_key);
			_set_val(h, GET_PTR(visit_array[1], v_step_inc), src_val);

		}														
	}			

	free(temp_k_box.str);
	free(temp_v_box.str);
	free(visit_array);
}



