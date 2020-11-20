from _mdict_c import i32_i32, i32_i64, i64_i32, i64_i64, str_str

str_len_MAX = 65355
DICT_TYPES = {('i32', 'i32'):i32_i32, ('i64', 'i64'):i64_i64, ('i32', 'i64'):i32_i64, ('i64', 'i32'):i64_i32, ('str', 'str'):str_str}


def create(dtype, key_len=None, val_len=None):
	"""
	Input : Example input dtype=i32:i32 will create an i32_i32 instance.  
	"""

	k_ty, v_type = None, None

	try:
		splitted_words = dtype.split(':')
	except AttributeError as e:
		raise TypeError("dtype must be a string")

	if len(splitted_words) != 2:
		raise ValueError("dtype string must be of format : 'key_type:value_type' ")
	else:
		k_type = splitted_words[0].strip()
		v_type = splitted_words[1].strip()
		if (k_type, v_type) not in DICT_TYPES:
			raise ValueError("Make sure dtype string contains valid key and value types")

	if (k_type, v_type) != ('str', 'str'):
		myDict = DICT_TYPES[(k_type, v_type)].create()
		return myDict
	else:
		if not(type(key_len) == int and type(val_len) == int):
			raise TypeError("Both key_len and val_len must be int")

		if not (0<key_len<=str_len_MAX and 0<val_len<=str_len_MAX):
			raise ValueError("Both key_len and val_len must be in between 0 and 65355")

		myDict = DICT_TYPES[(k_type, v_type)].create(key_len, val_len)
		return myDict		


def listDictionaryTypes():
	for key in DICT_TYPES:
		print("Key Type:", key[0], ". Value Type:", key[1])