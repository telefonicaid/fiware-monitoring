UNITTESTS_PROGS			= suite_metadata_parser
noinst_PROGRAMS			= $(UNITTESTS_PROGS)
check_PROGRAMS			= $(UNITTESTS_PROGS)
TESTS				= $(check_PROGRAMS)

suite_metadata_parser_SOURCES	= suite_metadata_parser.cc
suite_metadata_parser_CXXFLAGS	= -Wall @CPPUNIT_CFLAGS@
suite_metadata_parser_LDADD	= -lcppunit -lyajl @CPPUNIT_LIBS@ \
				  $(top_builddir)/src/ngsi_event_broker_la-metadata_parser.lo
