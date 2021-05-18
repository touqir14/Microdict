#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

/* 
	Microdictionary hashtable implementation.

*/

void rehash_int(h_t* h, i_t* new_flags, i_t* new_psl, i_t new_num_buckets);
void rehash_str(h_t* h, i_t* new_flags, i_t* new_psl, i_t new_num_buckets);
int mdict_resize(h_t *h, bool to_expand);


h_t *mdict_create(ht_param* param) {
	h_t* h = (h_t*)calloc(1, sizeof(h_t));
	h->is_map = true;							

	if (param){
		if (param->key_type == 5){
			h->key_str_len = param->key_size;
			h->k_t_size = param->key_size + 2; // 2 comes due to size of character array stored.
			h->k_step_increment = param->key_step_increment;
			srand(time(NULL));
			h->seed = (i_t) rand();
		}
		else {
			h->k_t_size = sizeof(k_t);
			h->k_step_increment = 1;
		}

		if (param->val_type == 5){
			h->val_str_len = param->val_size;
			h->v_t_size = param->val_size + 2; // 2 comes due to size of character array stored.
			h->v_step_increment = param->val_step_increment;
		}
		else {
			h->v_t_size = sizeof(v_t);
			h->v_step_increment = 1;
		}
	} else {
		h->k_t_size = sizeof(k_t);
		h->v_t_size = sizeof(v_t);
		h->k_step_increment = 1;
		h->v_step_increment = 1;
	}

	mdict_resize(h, true);
	return h;		
}																	


void mdict_delete_ht(h_t *h)						
{																	
	if (h) {					
		free((void *)h->keys); 
		free(h->flags);					
		free((void *)h->vals);
		free(h->psl);										
		free(h);													
	}																
}


inline vbox_t mdict_get_map(h_t *h, kbox_t key_box, i_t *ret_idx) 	
{																	
	i_t idx, ptr, last, mask, step = 0, k_step_inc = h->k_step_increment, v_step_inc = h->v_step_increment; 
	mask = h->num_buckets - 1;
	idx = _hash_func(h, key_box) & mask; 
	last = idx;
	i_t psl_val = _get_psl(h->psl, last);		
	vbox_t val;					
	ptr = GET_PTR(idx, k_step_inc);

	while (!(!_flags_isempty(h->flags, idx) && _key_equal(h, ptr, key_box))) { 
		idx = (idx + (++step)) & mask;
		ptr = GET_PTR(idx, k_step_inc);
		if (step > psl_val) {
			*ret_idx = h->num_buckets;
			return val;
		}						
	}

	*ret_idx = idx;
	val = _get_val(h, GET_PTR(idx, v_step_inc));

	return val;
}															



int mdict_resize(h_t *h, bool to_expand) 
{
	i_t *new_flags, *new_psl;										
	i_t j = 1;				
	
	i_t new_num_buckets;
	if (to_expand)
		new_num_buckets = h->num_buckets << 1;
	else
		new_num_buckets = h->num_buckets >> 1;		
	
	if (new_num_buckets < 32) 
		new_num_buckets = 32;			

	new_flags = (i_t*) malloc(_flags_size(new_num_buckets) * sizeof(i_t));	
	new_psl = (i_t*) malloc(_flags_size(new_num_buckets) * sizeof(i_t));	

	if (!new_flags || !new_psl) 
		return -1;	

	memset(new_flags, 0xff, _flags_size(new_num_buckets) * sizeof(i_t)); 
	memset(new_psl, 0, _flags_size(new_num_buckets) * sizeof(i_t)); 

	i_t k_t_size = h->k_t_size;
	i_t v_t_size = h->v_t_size;

	if (h->num_buckets < new_num_buckets) {		
		k_t *new_keys = (k_t*)realloc((void *)h->keys, new_num_buckets * k_t_size); 
		if (!new_keys) { 
			free(new_flags); 
			return -1; 
		}		
		h->keys = new_keys;									
		if (h->is_map) {									
			v_t *new_vals = (v_t*)realloc((void *)h->vals, new_num_buckets * v_t_size); 
			if (!new_vals) { 
				free(new_flags); 
				return -1; 
			}	
			h->vals = new_vals;								
		}													
	} 								

	_rehash_func(h, new_flags, new_psl, new_num_buckets);

	if (h->num_buckets > new_num_buckets) {
		h->keys = (k_t*)realloc((void *)h->keys, new_num_buckets * k_t_size); 

		if (h->is_map) { 
			h->vals = (v_t*)realloc((void *)h->vals, new_num_buckets * v_t_size); 
		}
	}

	free(h->flags); 		
	free(h->psl);		
	h->flags = new_flags;			
	h->psl = new_psl;							
	h->num_buckets = new_num_buckets;								
	h->upper_bound = (i_t)(h->num_buckets * PEAK_LOAD); 
															
	return 0;														
}											

inline void rehash_int(h_t* h, i_t* new_flags, i_t* new_psl, i_t new_num_buckets) {
	i_t new_mask = new_num_buckets - 1;		
	i_t i_ptr, k_step_inc = h->k_step_increment, v_step_inc = h->v_step_increment;

	for (i_t j = 0; j < h->num_buckets; ++j) {						
		if (!_flags_isempty(h->flags, j)) {					
			kbox_t key = _get_key(h, GET_PTR(j, k_step_inc));
			vbox_t val;
			if (h->is_map) {
				val = _get_val(h, GET_PTR(j, v_step_inc));
			}
			_flags_setTrue_isempty(h->flags, j);				

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

				if (i < h->num_buckets && !_flags_isempty(h->flags, i)) { 
					{ 
						i_ptr = GET_PTR(i, k_step_inc);
						kbox_t tmp = _get_key(h, i_ptr);
						_set_key(h, i_ptr, key);
						key = tmp; 
					} 
					if (h->is_map) { 
						i_ptr = GET_PTR(i, v_step_inc);
						vbox_t tmp = _get_val(h, i_ptr);
						_set_val(h, i_ptr, val);						
						val = tmp; 
					} 
					_flags_setTrue_isempty(h->flags, i); 
				} else { 
					_set_key(h, GET_PTR(i, k_step_inc), key);
					if (h->is_map) {
						_set_val(h, GET_PTR(i, v_step_inc), val);
					}
					break;										
				}												
			}													
		}														
	}			
}



static inline int mdict_set(h_t *h, kbox_t key_box, vbox_t val_box)
{																	
	i_t x;														

	if (h->size >= h->upper_bound) {
		if (mdict_resize(h, true) < 0) {   
			return -1;
		}											
	}

	i_t idx, last, mask = h->num_buckets - 1, step = 0, k_step_inc = h->k_step_increment, v_step_inc = h->v_step_increment; 
	x = h->num_buckets; 
	idx = _hash_func(h, key_box) & mask; 
	last = idx;
	i_t psl_val = _get_psl(h->psl, last);
	
	i_t ptr = GET_PTR(idx, k_step_inc); 

	if (_flags_isempty(h->flags, idx)) 
		x = ptr; 
	else {														
		while (!(_flags_isempty(h->flags, idx) || _key_equal(h, ptr, key_box))) { 
			idx = (idx + (++step)) & mask; 
			ptr = GET_PTR(idx, k_step_inc);

			if (idx == last) { 
				return -2;
			}					
		}														
		x = ptr;														
	}															

	int ret_val;

	if (_flags_isempty(h->flags, idx)) {	
		_set_key(h, x, key_box);										
		_flags_setFalse_isempty(h->flags, idx);							
		++h->size; 
		ret_val = 1;
	} else 
		ret_val = 0;  

	if (h->is_map) {
		_set_val(h, GET_PTR(idx, v_step_inc), val_box);		
	}

	if (step > psl_val)
		_set_psl(h->psl, last, step);

	return ret_val;														
}					



static inline int mdict_del_map(h_t *h, kbox_t key_box, vbox_t* val_box) {
	i_t idx;

	if (val_box == NULL)
		mdict_get_map(h, key_box, &idx);
	else
		*val_box = mdict_get_map(h, key_box, &idx);

	if (idx != h->num_buckets) {
		_flags_setTrue_isempty(h->flags, idx);							
		--h->size;
	} else {
		return -2;
	}

	if (h->size <= (h->num_buckets >> 2) && h->num_buckets > 32) {
		if (mdict_resize(h, false) < 0) {  
			return -1;
		}														
	}

	return 0;
} 


