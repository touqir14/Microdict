import microdict.microdict_tests.test_int as mtest_int 
import microdict.microdict_tests.test_str as mtest_str 

def run():
	mtest_int.run_all_int_tests()
	mtest_str.runTests_str_str()

if __name__ == "__main__":
	run()