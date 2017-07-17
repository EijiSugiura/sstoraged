include $(top_srcdir)/Makefile.common
SUBDIRS=
INCLUDES=-I$(top_srcdir)/include
noinst_LIBRARIES=libiscsi.a
libiscsi_a_SOURCES=tokenizer.cpp login.cpp keyvalue.cpp text.cpp logout.cpp \
	command.cpp datain.cpp r2t.cpp dataout.cpp nopin.cpp nopout.cpp taskmng.cpp

clean-local :
	rm -f *~