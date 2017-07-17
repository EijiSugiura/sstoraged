1. QUICKLY build from source 

$ ./configure
$ make

2. Build step by step

2.1. Setup initial source tree
$ autoscan
$ mv configure.scan configure.in
# Edit configure.in...
# Create Makefile.am...

2.2. Rebuild after file/directory added
# Edit Makefile.am...
$ aclocal
$ autoheader
$ automake --add-missing
$ autoconf
$ ./configure
$ make

3. Install

$ make install
