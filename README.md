# Microdict
A high performance python hash table library that is generally faster and consumes significantly less memory than Python Dictionaries. It currently supports Python 3.5+.

### Why Microdict? Why create another Hash table library when there is the builtin Python Dictionary?
Python Dictionaries are fast but their memory consumption can also be high at the same time. This is partly due to the nature of Python keeping data, within RAM, in the form of PyObjects, which consume far more memory than native types such as Integers and Character Arrays. As a result, Python Dictionaries can be prohibitive in many cases while building memory intensive python applications. This motivated me to develop a typed python hash table library that consumes significantly (upto 7 times) less memory compared to python dictionaries. It is also faster than python dictionaries. Moreover, it's underlying C implementation can also outperform Google's highly optimized [Swiss Table](https://abseil.io/blog/20180927-swisstables) and Facebook's [F14](https://engineering.fb.com/2019/04/25/developer-tools/f14/) hash tables. See the [Performance Section](#performance).

### Installation
Microdict is absolutely built using C extensions and as such building it will require Python C api header files. Build and install the package using 
```python setup.py install``` from the terminal after cloning the repository. Microdict is tested to work on Linux, Mac OSX, and Windows systems. You will need GCC 7+ on linux/mac osx systems and Visual C++ 14+ compiler on Windows systems to build the package.

### Usage
The following code snippet shows common uses of the library.

```python
from microdict import mdict

dict_i32 = mdict.create("i32:i32") # Generates a dictionary with key and value type of signed 32 bit integer.
dict_i32[1] = 2 # Just like python dictionaries, setting a key value pair.
try:
   print(4 in dict_i32) # prints False after catching a KeyError exception.
except KeyError:
   pass

print(dict_i32[1]) # prints 2
try:
   print(dict_i32.pop(4)) # prints None.
except KeyError:
   pass
print(dict_i32.pop(1)) # Removes [1,2] from the hashtable and prints 2.

dict_i32[10] = 0
dict_i32[5] = 1
dict_i32[6] = 8

for k in dict_i32:
   print(k) # Will print 10, 5, 6
  
for v in dict_i32.values():
   print(v) # Will print 0, 1, 8
  
for k,v in dict_i32.items():
   print(k,v) # Will print all the items.

d2 = dict_i32.copy() # Creates a deep copy of dict_i32.
dict_i32.clear([10,6]) # Removes the pairs [10,0] and [6,8]
dict_i32.clear() # Makes the dictionary empty.

pydict = d2.to_Pydict() # Returns a python dictionary containing all the items in d2
pydict[120] = 5
pydict[42] = 9

d2.update(pydict) # d2 now will additionally have the pairs [120, 5] and [42, 9]

dict_i32[111] = 89
dict_i32[123] = 1
d2.update(dict_i32) # d2 now will additionally have the pairs [111, 89] and [123, 1].

print(list(d2.items())) # prints all d2 items
"""
Faster approach shown below. d2.get_items() creates and returns a list of all items. 
So if you don't need a list container, iterate using d2.items() for memory efficiency. 
Same applies for other d2.get_* methods shown below.
"""
print(d2.get_items()) # 
print(list(d2.values())) # prints all d2 values
print(d2.get_values()) # Same but faster approach
print(list(d2)) # prints all d2 keys
print(d2.get_keys()) # Same but faster approach

```
#### Hash Table types
Currently, Microdict includes 5 types of dictionaries:
* ```"i32:i32"``` -> 32 bit signed keys and 32 bit signed values
* ```"i32:i64"``` -> 32 bit signed keys and 64 bit signed values
* ```"i64:i32"``` -> 64 bit signed keys and 32 bit signed values
* ```"i64:i64"``` -> 64 bit signed keys and 64 bit signed values
* ```"str:str"``` -> string keys and string values.

#### Method Documentations
* **microdict.mdict.create** (*dtype, key_len=None, val_len=None*)

   : Returns a Microdict hash table of any of the types given [above](#hash-table-types).
   
   **Parameters:**
   
   * *dtype:*  A python string type (```str```) that sets the hash table type to be created. It can be any one of the above [types](#hash-table-types).
   * *key_len:*  A python Integer type (```int```). It sets the maximum number of bytes the characters of a key (UTF-8 string) requires. Passing a UTF-8 encoded string key which consumes more bytes than *key_len* will not be accepted. This argument is only applicable when ```dtype="str:str"```. It only accepts a value of at most 65355 and a larger value will raise a ```TypeError```.
   * *val_len:* A python Integer type(```int```). It sets the maximum number of bytes the characters of a value (UTF-8 string) requires. Passing a UTF-8 encoded string value which consumes more bytes than *val_len* will not be accepted. This argument is only applicable when ```dtype="str:str"```. It only accepts a value of at most 65355 and a larger value will raise a ```TypeError```.
   
* **microdict.mdict.listDictionaryTypes** ()

   : Prints a series of lines of the form : ```Key Type: key_t . Value Type: val_t```, where ```key_t:val_t``` forms a type given [above](#hash-table-types).
   
The following are the methods that are common to all hash table types returned by ```mdict.create```.
* **clear** (*key_list = None*)

   : Returns None. If *key_list* is not provided, the **clear** method will delete all items from the hash table.
   
   **Parameters:**
   
   * *key_list:* It is an optional argument but not a keyword optional argument (keyword must not be provided). So, it suffices: ```keys = [1,2]; d.clear(keys)```. If provided, it must be of type ```list```. The entries within *key_list* are the keys that will be removed from the hash table. By default, any entry that is not present in the hash table will be skipped. For any of the integer hash table types, any non python ```int``` entry will be skipped. It is upto the programmer to pass the proper sized integer to prevent overflows. For ```str:str``` type, the entries must be python UTF-8 strings with UTF-8 character bytes upto *key_len* as set by ```mdict.create``` and otherwise, that entry will be skipped.
   
* **copy** ()

   : Returns a deep copy of the Microdict Hash table of the same type as the caller Hash table object.
   
* **get_items** ()

   : Creates and returns a python ```list``` containing all the items (key, value) in the hash table.
   
* **get_keys** ()
   
   : Creates and returns a python ```list``` containing all the keys in the hash table.
   
* **get_values** ()

   : Creates and returns a python ```list``` containing all the values in the hash table.
   
* **items** ()

   : Used to iterate over items using a ```for``` loop. Example : ```for k,v in d.items() : print(k, v)```
   
* **pop** (*key*)

   : Deletes a *key* from the hash table and returns its corresponding value. If the *key* is not present, then a ```KeyError``` is raised.
   
   **Parameters:**
   
   * *key:* For any of the integer hash table types, any non python ```int``` entry will raise a ```TypeError```. It is upto the programmer to pass the proper sized integer to prevent overflows. For ```str:str``` type, the entries must be python UTF-8 strings with UTF-8 character bytes upto *key_len* as set by ```mdict.create``` and otherwise, a ```TypeError``` will be raised.
   
* **to_Pydict** ()

   : Creates and returns a python dictionary containing all items present in the Microdict hash table.
   
* **update** (*dictionary*)

   : Inserts all the items present in the dictionary into the Microdict hash table.
   
   **Parameters:**
   
   * *dictionary:* Must be either a python dictionary or a Microdict hash table. If it is a python dictionary, then all its items that are of the same type as the Microdict hash table will be inserted. The rest will be skipped by default. The conditions given in the method documentation of **clear** regarding type constraints apply here too.   
   
* **values** ()

   : Used to iterate over values using a ```for``` loop. Example : ```for v in d.values() : print(v)```.


### Performance
#### Competing with Python Dictionary
Each of the cells in the tables below are of the format (Speed Gain, Space Gain). 

* Speed Gain is defined as : <img src="https://render.githubusercontent.com/render/math?math=\dfrac{\text{Average execution time for competing hash table}}{\text{Average execution time for Microdict hash table}}">. 

* Similarly, Space Gain : <img src="https://render.githubusercontent.com/render/math?math=\dfrac{\text{Average memory consumed by competing hash table}}{\text{Average memory consumed by Microdict hash table}}">

Experiments were carried out for the types ```"i32:i32"```, ```"i64:64"```, ```"str:str"``` (key and value length kept to 8 characters). Speed Gain and Space Gain are computed by averaging over the results of 30 experiments carried out using (unique) randomly generated data. Space consumption was computed using the [psutil](https://github.com/giampaolo/psutil) library. Time consumption was computed using the ```time.perf_counter``` method. All the experiments were conducted with Python 3.8.2. The following table shows the benchmarks against Python Dictionary for retrieving all values using the full set of keys and the ```[]``` operator.

| #Items     | ```i32:i32``` | ```i64:i64```| ```str:str```|
|------------|---------------|--------------|--------------|
| 100000     | 1.48x, 7.13x  | 1.29x, 4.67x | 1.43x, 5.19x |
| 1000000    | 1.47x, 4.23x  | 1.48x, 2.64x | 1.46x, 4.46x |
| 10000000   | 1.44x, 4.77x  | 1.48x, 3.02x | 1.53x, 4.97x |
| 3x10000000 | 1.55x, 4.19x  | 1.3x , 2.57x | 1.36x, 3.93x |

#### Competing with Google's Swiss Table and Facebook's F14
Microdict's underlying C implementation was benchmarked against Swiss Table```->```[abseil::flat_hash_map](https://abseil.io/docs/cpp/guides/container) and F14```->```[folly::F14FastMap](https://github.com/facebook/folly/blob/master/folly/container/F14.md) to further test its capabilities. The data types were set as above using the same settings. The following tables show the results for retrieving all values using the keys.

<table>
<tr><th>abseil::flat_hash_map </th><th>folly::F14FastMap </th></tr>
<tr><td>

| #Items     | ```i32:i32``` | ```i64:i64```| ```str:str```|   
|------------|---------------|--------------|--------------|
| 1000000    | 1.22x, 1.56x  | 1.02x, 1.08x | 1.33x, 1.82x |
| 10000000   | 1.14x, 1x     | 1.09x, 1.33x | 1.73x, 1.67x |
| 3x10000000 | 1.11x, 1.27x  | 1.15x, 1.03x | 1.68x, 1.65x |

</td><td>

| #Items     | ```i32:i32``` | ```i64:i64```| ```str:str```|   
|------------|---------------|--------------|--------------|
| 1000000    | 2.96x, 1.56x  | 2.74x, 1x    | 3.12x, 1.39x |
| 10000000   | 2.11x, 1x     | 2.12x, 1.29x | 3.45x, 1.4x  |
| 3x10000000 | 1.84x, 1.22x  | 1.97x, 1x    | 3.36x, 1.21x |

</td></tr> </table>

