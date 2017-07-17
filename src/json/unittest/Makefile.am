include $(top_srcdir)/Makefile.common
INCLUDES=-I$(top_srcdir)/include

TESTS=test_json
TESTS_ENVIRONMENT=python runjsontests.py

check_PROGRAMS=test_json
test_json_SOURCES=main.cpp
test_json_LDADD=-L.. -ljson

clean-local :
	rm -f *~