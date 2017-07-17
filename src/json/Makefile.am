include $(top_srcdir)/Makefile.common
SUBDIRS=unittest
INCLUDES=-I$(top_srcdir)/include
noinst_LIBRARIES=libjson.a
libjson_a_SOURCES=json_reader.cpp json_writer.cpp json_value.cpp

clean-local :
	rm -f *~