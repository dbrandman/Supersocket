# SWIG Python Wrapper for Supersocket



## Basics

Please install SWIG first:

```shell
sudo apt install swig
sudo apt install python2.7-dev
```

Then, build the Python library:

```shell
make
```

Now, try running the tests in the `test/` directory.



## Using Supersocket in your own project

To include / access the Supersocket Python extension in your own code, you must 
copy both *supersocket.py* and *_supersocket.so* into your project directory.

Supersocket can now be imported using the Python `import` directive like so:

```python
import supersocket
```

For an example of how to create a new Supersocket address and send a message, 
please see the documentation in the Supersocket extension:

```python
help(supersocket)
```

Most functions have been ported over into the Python extension and their usage
is almost identical, except you never have to worry about pointers! For some
use-cases check out Supersocket/SWIG/test.

NOTE: In Python, `from` is a reserved keyword, so the Message object has that
      property renamed to `_from`.



## Additional notes

To force rebuild the project, run:

```shell
make remake
```

To clean the project, run:

```shell
make clean
```



## About SWIG

SWIG stands for *Simplified Wrapper and Interface Generator*. It generates C 
and C++ software wrappers in a variety of higher-level, often interpreted, 
languages such as Python. SWIG is basically another C preprocessor, in front of
the standard one. More information can be found [here](http://www.swig.org/).

To generate a new wrapper, SWIG requires an interface (\*.i) file. This file
defines which functions and data structures are to be exposed in the generated
wrapper. SWIG also lets you write additional wrapper functions that can make
an interface more intuitive and usable. The SWIG interface file is written in C
with SWIG-specific tags (starting with percent (%) signs mixed in).

For a quick demo, see [here](demo).



## Modifying the API

Whenever either of the APIs are changed, check the sections below titled
'Data structures and functions to expose...'. These sections contain 
declarations copied verbatim from the C header files. They define which 
functions and data structures SWIG will expose in the generated code (i.e.
Python). If SWIG is still failing but the APIs match, check the functions
and struct extensions ('%extend') to make sure that all methods contained
within are valid and reference existent data structure fields or functions.
