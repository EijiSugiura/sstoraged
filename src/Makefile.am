include $(top_srcdir)/Makefile.common
SUBDIRS=json iscsi scsi unittest
INCLUDES=-I$(top_srcdir)/include
sbin_PROGRAMS=sstoraged
sstoraged_SOURCES=main.cpp common.cpp parse_args.cpp configurator.cpp configbinder.cpp \
	targetportconfigurator.cpp \
	targetconfigurator.cpp initiatorconfigurator.cpp volumeconfigurator.cpp \
	validator.cpp rootvalidator.cpp targetvalidator.cpp initiatorvalidator.cpp \
	volumevalidator.cpp targetportvalidator.cpp \
	port.cpp portlistener.cpp filehandle.cpp signallistener.cpp logger.cpp \
	loop.cpp acceptor.cpp manager.cpp session.cpp reactor.cpp connection.cpp \
	ioveccontainer.cpp task.cpp initiator.cpp volume.cpp writecache.cpp \
	send.cpp sendfile.cpp sendcache.cpp diskwriter.cpp
sstoraged_LDADD=-ljson ./iscsi/libiscsi.a ./scsi/libscsi.a
sstoraged_LDFLAGS=-L./json

clean-local :
	rm -f *~