include $(top_srcdir)/Makefile.common
INCLUDES=-I$(top_srcdir)/include
check_PROGRAMS= test_configurator test_configbinder test_targetconfigurator \
	test_targetportconfigurator \
	test_initiatorconfigurator test_volumeconfigurator test_inetutils \
	test_validator test_rootvalidator test_send \
	test_writecache test_readcache test_sendcache test_diskwriter \
	test_limitter
TESTS= test_configurator test_configbinder test_targetconfigurator \
	test_targetportconfigurator \
	test_initiatorconfigurator test_volumeconfigurator test_inetutils \
	test_validator test_rootvalidator test_send \
	test_writecache test_readcache test_sendcache test_diskwriter \
	test_limitter

.h.cpp:
	cxxtestgen --error-printer -o $@ $<

test_configurator_SOURCES=test_configurator.cpp  ../configurator.cpp
test_configurator_LDADD=-ljson \
	../validator.o ../rootvalidator.o \
	../targetportconfigurator.o ../targetportvalidator.o \
	../targetconfigurator.o ../targetvalidator.o \
	../initiatorconfigurator.o ../initiatorvalidator.o \
	../volumeconfigurator.o ../volumevalidator.o
test_configurator_LDFLAGS=-L../json

test_configbinder_SOURCES=test_configbinder.cpp ../configurator.cpp
test_configbinder_LDADD=-ljson  ../configbinder.o \
	../validator.o ../rootvalidator.o \
	../targetportconfigurator.o ../targetportvalidator.o \
	../targetconfigurator.o ../targetvalidator.o \
	../initiatorconfigurator.o ../initiatorvalidator.o \
	../volumeconfigurator.o ../volumevalidator.o
test_configbinder_LDFLAGS=-L../json

test_targetconfigurator_SOURCES=test_targetconfigurator.cpp ../configurator.cpp
test_targetconfigurator_LDADD=-ljson \
	../validator.o ../rootvalidator.o \
	../targetportconfigurator.o ../targetportvalidator.o \
	../targetconfigurator.o ../targetvalidator.o \
	../initiatorconfigurator.o ../initiatorvalidator.o \
	../volumeconfigurator.o ../volumevalidator.o
test_targetconfigurator_LDFLAGS=-L../json

test_targetportconfigurator_SOURCES=test_targetportconfigurator.cpp ../configurator.cpp
test_targetportconfigurator_LDADD=-ljson \
	../validator.o ../rootvalidator.o \
	../targetportconfigurator.o ../targetportvalidator.o \
	../targetconfigurator.o ../targetvalidator.o \
	../initiatorconfigurator.o ../initiatorvalidator.o \
	../volumeconfigurator.o ../volumevalidator.o
test_targetportconfigurator_LDFLAGS=-L../json

test_initiatorconfigurator_SOURCES=test_initiatorconfigurator.cpp ../configurator.cpp
test_initiatorconfigurator_LDADD=-ljson ../configbinder.o \
	../validator.o ../rootvalidator.o \
	../targetportconfigurator.o ../targetportvalidator.o \
	../targetconfigurator.o ../targetvalidator.o \
	../initiatorconfigurator.o ../initiatorvalidator.o \
	../volumeconfigurator.o ../volumevalidator.o
test_initiatorconfigurator_LDFLAGS=-L../json

test_inetutils_SOURCES=test_inetutils.cpp

test_volumeconfigurator_SOURCES=test_volumeconfigurator.cpp ../configurator.cpp
test_volumeconfigurator_LDADD=-ljson ../configbinder.o \
	../validator.o ../rootvalidator.o \
	../targetportconfigurator.o ../targetportvalidator.o \
	../targetconfigurator.o ../targetvalidator.o \
	../initiatorconfigurator.o ../initiatorvalidator.o \
	../volumeconfigurator.o ../volumevalidator.o
test_volumeconfigurator_LDFLAGS=-L../json

test_validator_SOURCES=test_validator.cpp
test_validator_LDADD=../validator.o
test_validator_CXXFLAGS=-std=c++0x

test_rootvalidator_SOURCES=test_rootvalidator.cpp ../configurator.cpp
test_rootvalidator_LDADD=-ljson ../validator.o ../rootvalidator.o \
	../targetportconfigurator.o ../targetportvalidator.o \
	../targetconfigurator.o ../targetvalidator.o \
	../initiatorconfigurator.o ../initiatorvalidator.o \
	../volumeconfigurator.o ../volumevalidator.o
test_rootvalidator_LDFLAGS=-L../json

test_send_SOURCES=test_send.cpp ../send.cpp ../port.cpp ../ioveccontainer.cpp
test_send_CXXFLAGS=-DLOG4CXX_COUT -DIOVEC_DEBUG

test_writecache_SOURCES=test_writecache.cpp ../writecache.cpp
test_writecache_CXXFLAGS=-DLOG4CXX_COUT

test_readcache_SOURCES=test_readcache.cpp ../writecache.cpp ../scsi/read.cpp
test_readcache_CXXFLAGS=-DLOG4CXX_COUT

test_sendcache_SOURCES=test_sendcache.cpp ../sendcache.cpp ../writecache.cpp \
	../port.cpp ../ioveccontainer.cpp
test_sendcache_LDADD=../common.o
test_sendcache_CXXFLAGS=-DLOG4CXX_COUT -DIOVEC_DEBUG

test_diskwriter_SOURCES=test_diskwriter.cpp ../diskwriter.cpp ../filehandle.cpp \
	../writecache.cpp
test_diskwriter_LDADD=../common.o
test_diskwriter_CXXFLAGS=-DLOG4CXX_COUT -DIOVEC_DEBUG
test_diskwriter_LDFLAGS=-laio

test_limitter_SOURCES=test_limitter.cpp

clean-local :
	rm -f *.cpp *~