include $(top_srcdir)/Makefile.common
SUBDIRS=json iscsi scsi
noinst_HEADERS=common.h logger.h configurator.h configbinder.h configdefault.hpp \
	targetportconfigurator.h \
	targetconfigurator.h initiatorconfigurator.h volumeconfigurator.h \
	inetutils.h limitter.h port.h portlistener.h filehandle.h acceptor.h \
	signallistener.h manager.h session.h reactor.h writecache.h diskwriter.h

configdefault.hpp : $(top_srcdir)/man/sstoraged.conf.pod $(top_srcdir)/script/podconf2default.pl
	$(top_srcdir)/script/podconf2default.pl -f $< > $@

clean-local :
	rm -f *~