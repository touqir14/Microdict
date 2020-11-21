from setuptools import setup, Extension, find_packages
import os
import sys

parent_dir = 'microdict'


def read(filename):
    return open(os.path.join(os.path.dirname(__file__), filename)).read()

if os.name != 'nt':
    if sys.platform == 'darwin' and 'APPVEYOR' in os.environ:
        os.environ['CC'] = 'gcc-8'

    module_i32_i32 = Extension('i32_i32', sources = [os.path.join(parent_dir, 'int32_int32_Py.c')], extra_compile_args = ["-O3", "-w"])
    module_i32_i64 = Extension('i32_i64', sources = [os.path.join(parent_dir, 'int32_int64_Py.c')], extra_compile_args = ["-O3", "-w"])
    module_i64_i32 = Extension('i64_i32', sources = [os.path.join(parent_dir, 'int64_int32_Py.c')], extra_compile_args = ["-O3", "-w"])
    module_i64_i64 = Extension('i64_i64', sources = [os.path.join(parent_dir, 'int64_int64_Py.c')], extra_compile_args = ["-O3", "-w"])
    module_str_str = Extension('str_str', sources = [os.path.join(parent_dir, 'str_str_wyhash_Py.c')], extra_compile_args = ["-O3", "-w"])

    os.system('gcc -v')
else:
    # If windows:
    module_i32_i32 = Extension('i32_i32', sources = [os.path.join(parent_dir, 'int32_int32_Py.c')], extra_compile_args = ["/O2", "/w"])
    module_i32_i64 = Extension('i32_i64', sources = [os.path.join(parent_dir, 'int32_int64_Py.c')], extra_compile_args = ["/O2", "/w"])
    module_i64_i32 = Extension('i64_i32', sources = [os.path.join(parent_dir, 'int64_int32_Py.c')], extra_compile_args = ["/O2", "/w"])
    module_i64_i64 = Extension('i64_i64', sources = [os.path.join(parent_dir, 'int64_int64_Py.c')], extra_compile_args = ["/O2", "/w"])
    module_str_str = Extension('str_str', sources = [os.path.join(parent_dir, 'str_str_wyhash_Py.c')], extra_compile_args = ["/O2", "/w"])


setup (name = 'microdict',
        version = '0.1',
        author = 'Touqir Sajed',
        author_email = 'shuhash6@gmail.com',
        description = 'The Microdict library - a high performance Python hashtable implementation',
        long_description = read('README.md'),
        license = 'MIT',
        url = 'https://github.com/touqir14/Microdict',
        ext_package = '_mdict_c',
        py_modules = [os.path.join(parent_dir, 'mdict'), os.path.join(parent_dir, 'run_tests'), os.path.join(parent_dir, 'microdict_tests')],
        ext_modules = [module_i32_i32, module_i32_i64, module_i64_i32, module_i64_i64, module_str_str],
        packages = find_packages(),
        classifiers = ['Development Status :: 4 - Beta',
          'Intended Audience :: Developers',
          'License :: OSI Approved :: MIT License',
          'Operating System :: MacOS',
          'Operating System :: Microsoft :: Windows',
          'Operating System :: POSIX :: Linux',
          'Programming Language :: Python',
          'Programming Language :: Python :: 3',
          'Programming Language :: Python :: 3.5',
          'Programming Language :: Python :: 3.6',
          'Programming Language :: Python :: 3.7',
          'Programming Language :: Python :: 3.8',
          'Programming Language :: Python :: 3.9',
          'Programming Language :: Python :: Implementation :: CPython',
          'Programming Language :: C',
          'Topic :: Software Development',
          'Topic :: Utilities',],
        )


