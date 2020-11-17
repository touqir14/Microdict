from distutils.core import setup, Extension
import os

parent_dir = 'Microdict'

module_i32_i32 = Extension('i32_i32', sources = [os.path.join(parent_dir, 'int32_int32_Py.c')], extra_compile_args = ["-O4", "-w"])
module_i32_i64 = Extension('i32_i64', sources = [os.path.join(parent_dir, 'int32_int64_Py.c')], extra_compile_args = ["-O4", "-w"])
module_i64_i32 = Extension('i64_i32', sources = [os.path.join(parent_dir, 'int64_int32_Py.c')], extra_compile_args = ["-O4", "-w"])
module_i64_i64 = Extension('i64_i64', sources = [os.path.join(parent_dir, 'int64_int64_Py.c')], extra_compile_args = ["-O4", "-w"])
module_str_str = Extension('str_str', sources = [os.path.join(parent_dir, 'str_str_wyhash_Py.c')], extra_compile_args = ["-O4", "-w"])

setup (name = 'mdict',
        version = '0.1',
        author = 'Touqir Sajed',
        author_email = 'shuhash6@gmail.com',
        description = 'The microdict library - a high performance hashtable implementation',
        ext_package = '_mdict_c',
        # py_modules = ['mdict'],
        py_modules = [os.path.join(parent_dir, 'mdict')],
        ext_modules = [module_i32_i32, module_i32_i64, module_i64_i32, module_i64_i64, module_str_str])


