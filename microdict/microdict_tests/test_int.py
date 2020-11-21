import unittest
import random
from microdict import mdict

def gen_random_list_unique(size, num_range, seed=0):

	numbers = set()
	random.seed(seed)
	while(len(numbers) < size):
		numbers.add(random.randint(*num_range))

	return list(numbers)

def gen_random_list(size, num_range, seed=0):

	numbers = [0] * size
	random.seed(seed)
	for i in range(size):
		numbers[i] = random.randint(*num_range)

	return numbers


class Test_int_int(unittest.TestCase):
	size = 10
	dict_type = None

	def create_dict(self):

		if self.dict_type == 'i32:i32':
			self.key_range = [-2147483648, 2147483647]
			self.val_range = [-2147483648, 2147483647]
			return mdict.create(self.dict_type)

		elif self.dict_type == 'i32:i64':
			self.key_range = [-2147483648, 2147483647]
			self.val_range = [-9223372036854775808, 9223372036854775807]
			return mdict.create(self.dict_type)

		elif self.dict_type == 'i64:i32':
			self.key_range = [-9223372036854775808, 9223372036854775807]
			self.val_range = [-2147483648, 2147483647]
			return mdict.create(self.dict_type)

		elif self.dict_type == 'i64:i64':
			self.key_range = [-9223372036854775808, 9223372036854775807]
			self.val_range = [-9223372036854775808, 9223372036854775807]
			return mdict.create(self.dict_type)


	def test_simple(self):
		d1 = self.create_dict()
		keys = gen_random_list_unique(self.size, self.key_range, seed=23319)
		vals = gen_random_list_unique(self.size, self.val_range, seed=43431313)

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
		keys = gen_random_list_unique(self.size, self.key_range, seed=23319)
		vals = gen_random_list_unique(self.size, self.val_range, seed=43431313)
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
		d1 = self.create_dict()
		partition_size = int(self.size/2)
		keys = gen_random_list_unique(self.size, self.key_range, seed=23319)
		vals = gen_random_list_unique(self.size, self.val_range, seed=43431313)
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
		d1 = self.create_dict()
		partition_size = int(self.size/2)
		keys = gen_random_list_unique(self.size, self.key_range, seed=23319)
		vals = gen_random_list_unique(self.size, self.val_range, seed=43431313)
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
		keys = ['1', '2', '3']
		vals = ['1', '2', '3']
		items = list(zip(keys, vals))
		sorter = lambda x:x[0]

		find_key = lambda d,x: x in d
		self.assertRaises(TypeError, find_key, d1, '1')

		get_val = lambda d,x: d[x]
		self.assertRaises(TypeError, get_val, d1, '1')

		def set_val(d,k,v): d[k]=v
		self.assertRaises(TypeError, set_val, d1, '1', '1')
		self.assertRaises(TypeError, set_val, d1, 5, '1')
		self.assertFalse(5 in d1)

		self.assertRaises(TypeError, d1.pop, '1')
		self.assertRaises(KeyError, d1.pop, 5)
		self.assertRaises(TypeError, d1.clear, {})

		d2 = {'1':1, 2:2, 3:'3'}
		d1.update(d2)
		self.assertListEqual(d1.get_items(), [(2,2)])
		self.assertRaises(TypeError, d1.update, [1])




def runTests_i32_i32():
	runner = unittest.TextTestRunner(verbosity=2)

	Test_int_int.dict_type = 'i32:i32'
	Test_int_int.size = 10
	print("Running i32_i32 tests with number of items set to", Test_int_int.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_int_int)
	runner.run(suite)

	Test_int_int.size = 100000
	print("Running i32_i32 tests with number of items set to", Test_int_int.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_int_int)
	runner.run(suite)


def runTests_i32_i64():
	runner = unittest.TextTestRunner(verbosity=2)

	Test_int_int.dict_type = 'i32:i64'
	Test_int_int.size = 10
	print("Running i32_i64 tests with number of items set to", Test_int_int.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_int_int)
	runner.run(suite)

	Test_int_int.size = 100000
	print("Running i32_i64 tests with number of items set to", Test_int_int.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_int_int)
	runner.run(suite)

def runTests_i64_i32():
	runner = unittest.TextTestRunner(verbosity=2)

	Test_int_int.dict_type = 'i64:i32'
	Test_int_int.size = 10
	print("Running i64_i32 tests with number of items set to", Test_int_int.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_int_int)
	runner.run(suite)

	Test_int_int.size = 100000
	print("Running i64_i32 tests with number of items set to", Test_int_int.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_int_int)
	runner.run(suite)

def runTests_i64_i64():
	runner = unittest.TextTestRunner(verbosity=2)

	Test_int_int.dict_type = 'i64:i64'
	Test_int_int.size = 10
	print("Running i64_i64 tests with number of items set to", Test_int_int.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_int_int)
	runner.run(suite)

	Test_int_int.size = 100000
	print("Running i64_i64 tests with number of items set to", Test_int_int.size)
	suite = unittest.TestLoader().loadTestsFromTestCase(Test_int_int)
	runner.run(suite)


def run_all_int_tests():
	runTests_i32_i32()
	runTests_i32_i64()
	runTests_i64_i32()
	runTests_i64_i64()	

if __name__ == '__main__':
	run_all_int_tests()