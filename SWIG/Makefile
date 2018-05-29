# SWIG Makefile
#
# Build for Linux:
#		$ make
#
# Build for Windows (requires Cygwin):
# 		$ make os=windows

all: build
remake: clean build


ifeq ($(os), windows)
COMPILER = --compiler=mingw32
else
# leave blank (default)
COMPILER = 
endif


build:
	CFLAGS="-DNOFPRINTF" python setup.py build_ext --inplace $(COMPILER)
	# python setup.py build_ext --inplace $(COMPILER)
	# cp _supersocket.dll _supersocket.pyd  # rename for Windows
	# cp supersocket.py _supersocket.dll _supersocket.pyd test/
	cp supersocket.py _supersocket.so test/
	
clean-test:
	rm -rf test/_supersocket.* test/supersocket.py test/*.pyc

clean: clean-test
	rm -rf build *.pyc *.so *.pyd *.dll *.c supersocket.py