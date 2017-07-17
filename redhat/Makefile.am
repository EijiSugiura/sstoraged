include $(top_srcdir)/Makefile.common
TARGETS=sstoraged.conf

all:$(TARGETS)

sstoraged.conf : $(top_srcdir)/man/sstoraged.conf.pod $(top_srcdir)/script/podconf2example.pl
	$(top_srcdir)/script/podconf2example.pl -f $< > $@

clean-local :
	rm -f *~ $(TARGETS)