import unittest
import random
from microdict import mdict
import string

def randStr(chars = string.ascii_uppercase + string.digits, N=10):
	str_len = random.randint(1, N-1)
	return ''.join(random.choice(chars) for _ in range(str_len)) + '!'


def randStr_UTF(UTF_size=1, N=10):
	if UTF_size == 1:
		char_upper = 127
	elif UTF_size == 2:
		char_upper = 2047
	elif UTF_size == 3:
		char_upper = 65535
	elif UTF_size == 4:
		char_upper = 1114111
	else:
		return None

	str_len = random.randint(1, N)
	return ''.join(chr(random.randint(1, char_upper)) for _ in range(str_len)) 
	# str_len = random.randint(1, N-1)
	# return ''.join(chr(random.randint(1, char_upper)) for _ in range(str_len)) + '!'

def gen_random_str_list(size, str_len, UTF_size, seed=None):
	s = set()
	if seed is not None:
		random.seed(seed)

	while(len(s) < size):
		s.add(randStr_UTF(UTF_size, N=str_len))

	return list(s)

class Test_str_str(unittest.TestCase):
	size = 10
	key_len = None
	val_len = None
	UTF_size = None

	def create_dict(self):
		return mdict.create("str:str", self.key_len * self.UTF_size, self.val_len * self.UTF_size)


	def test_simple(self):
		d1 = self.create_dict()
		keys = gen_random_str_list(self.size, self.key_len, self.UTF_size, seed=23319)
		vals = gen_random_str_list(self.size, self.val_len, self.UTF_size, seed=43431313)

		for i in range(self.size):
			d1[keys[i]] = vals[i]

		key_found = []
		for k in keys:
			key_found.append(k in d1)
		self.assertListEqual(key_found, [True]*self.size)

		key_results = []
		for k in d1:
			key_results.append(k)
		self.assertListEqual(sorted(keys), sorted(key_results))

		val_results = []
		for k in keys:
			val_results.append(d1[k])
		self.assertListEqual(sorted(vals), sorted(val_results))

	def test_iterators(self):
		d1 = self.create_dict()
		keys = gen_random_str_list(self.size, self.key_len, self.UTF_size, seed=23319)
		vals = gen_random_str_list(self.size, self.val_len, self.UTF_size, seed=43431313)
		items = list(zip(keys, vals)) 
		sorter = lambda x:x[0]

		for i in range(self.size):
			d1[keys[i]] = vals[i]

		val_results = []
		for v in d1.values():
			val_results.append(v)
		self.assertListEqual(sorted(val_results), sorted(vals))

		item_results = []
		for k,v in d1.items():
			item_results.append((k,v))

		self.assertListEqual(sorted(item_results, key=sorter), sorted(items, key=sorter))
		self.assertListEqual(sorted(keys), sorted(d1.get_keys()))
		self.assertListEqual(sorted(vals), sorted(d1.get_values()))
		self.assertListEqual(sorted(items, key=sorter), sorted(d1.get_items(), key=sorter))

	def test_updating_conversion(self):
		partition_size = int(self.size/2)
		d1 = self.create_dict()
		keys = gen_random_str_list(self.size, self.key_len, self.UTF_size, seed=23319)
		vals = gen_random_str_list(self.size, self.val_len, self.UTF_size, seed=43431313)
		items = list(zip(keys, vals)) 
		sorter = lambda x:x[0]

		d2 = {}
		for i in range(partition_size):
			d1[keys[i]] = vals[i]
			d2[keys[i]] = vals[i]

		self.assertDictEqual(d2, d1.to_Pydict())

		for i in range(partition_size, self.size):
			d2[keys[i]] = vals[i]

		d1.update(d2)
		self.assertDictEqual(d2, d1.to_Pydict())

		d3 = self.create_dict()
		d3.update(d1)

		self.assertListEqual(sorted(list(d3.items()), key=sorter), sorted(list(d1.items()), key=sorter)) 


	def test_misc(self):
		partition_size = int(self.size/2)
		d1 = self.create_dict()
		keys = gen_random_str_list(self.size, self.key_len, self.UTF_size, seed=23319)
		vals = gen_random_str_list(self.size, self.val_len, self.UTF_size, seed=43431313)
		items = list(zip(keys, vals)) 
		sorter = lambda x:x[0]

		for i in range(self.size):
			d1[keys[i]] = vals[i]

		partial_vals = []
		for i in range(partition_size, self.size):
			partial_vals.append(d1.pop(keys[i]))

		self.assertListEqual(vals[partition_size:self.size], partial_vals)
		self.assertListEqual(sorted(items[:partition_size], key=sorter), sorted(d1.items(), key=sorter))

		d2 = d1.copy()
		self.assertListEqual(sorted(d1.items(), key=sorter), sorted(d2.items(), key=sorter))

		self.assertEqual(len(d1), partition_size)
		d1.clear([keys[0],keys[1]])
		self.assertEqual(len(d1), partition_size-2)

		d1.clear(keys[2:partition_size])
		self.assertEqual(len(d1), 0)
		self.assertEqual(list(d1), [])
		self.assertEqual(list(d1.values()), [])
		self.assertEqual(list(d1.items()), [])

		d2.clear()
		self.assertEqual(len(d2), 0)
		self.assertEqual(list(d2), [])
		self.assertEqual(list(d2.values()), [])
		self.assertEqual(list(d2.items()), [])


	def test_exceptions(self):
		d1 = self.create_dict()
		keys = [1, 2, 3]
		vals = [1, 2, 3]
		items = list(zip(keys, vals))
		sorter = lambda x:x[0]

		find_key = lambda d,x: x in d
		self.assertRaises(TypeError, find_key, d1, 1)

		get_val = lambda d,x: d[x]
		self.assertRaises(TypeError, get_val, d1, 1)

		def set_val(d,k,v): d[k]=v
		self.assertRaises(TypeError, set_val, d1, 1, 1)
		self.assertRaises(TypeError, set_val, d1, '5', 1)
		self.assertFalse('5' in d1)

		self.assertRaises(TypeError, d1.pop, 1)
		self.assertRaises(KeyError, d1.pop, '5')
		self.assertRaises(TypeError, d1.clear, {})

		d2 = {'1':1, '2':'2', 3:'3'}
		d1.update(d2)
		self.assertListEqual(d1.get_items(), [('2','2')])
		self.assertRaises(TypeError, d1.update, ['1'])



def runTests_str_str():
	runner = unittest.TextTestRunner(verbosity=2)

	Test_str_str.key_len = 7
	Test_str_str.val_len = 8
	Test_str_str.UTF_size = 1
	Test_str_str.size = 10
	print("Running str_str tests with number of items set to", Test_str_str.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_str_str)
	runner.run(suite)

	Test_str_str.size = 100000
	print("Running str_str tests with number of items set to", Test_str_str.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_str_str)
	runner.run(suite)


if __name__ == '__main__':
	runTests_str_str()