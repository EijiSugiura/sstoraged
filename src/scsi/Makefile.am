include $(top_srcdir)/Makefile.common
SUBDIRS=
INCLUDES=-I$(top_srcdir)/include
noinst_LIBRARIES=libscsi.a
libscsi_a_SOURCES=reportluns.cpp ../iscsi/datain.cpp ../iscsi/r2t.cpp inquiry.cpp readcapacity.cpp \
	readcapacity16.cpp modesense.cpp read.cpp write.cpp dummy.cpp maintenancein.cpp

clean-local :
	rm -f *~