UNITTESTS_PROGS			   = suite_metadata_parser \
				     suite_argument_parser \
				     suite_broker_common   \
				     suite_broker_host     \
				     suite_broker_snmp

UNITTEST_NAGIOS_MAIN		   = nagios_main.c
UNITTEST_BROKER_MOCK		   = broker_common_mock.c

noinst_PROGRAMS			   = $(UNITTESTS_PROGS)
check_PROGRAMS			   = $(UNITTESTS_PROGS)
TESTS				   = $(check_PROGRAMS)

NAGIOS_BASE_OBJECTS		   = $(wildcard @NAGIOS_SRCDIR@/base/*.o)
NAGIOS_COMMON_OBJECTS		   = $(wildcard @NAGIOS_SRCDIR@/common/*.o)
NAGIOS_OBJECTS			   = $(filter-out */nagios.o, $(NAGIOS_BASE_OBJECTS) $(NAGIOS_COMMON_OBJECTS))

suite_metadata_parser_SOURCES	   = suite_metadata_parser.cc
suite_metadata_parser_CXXFLAGS	   = -Wall @CPPUNIT_CFLAGS@
suite_metadata_parser_LDADD	   = -lcppunit -lyajl @CPPUNIT_LIBS@ \
				     $(top_builddir)/src/ngsi_event_broker_host_la-metadata_parser.lo

suite_argument_parser_SOURCES	   = suite_argument_parser.cc
suite_argument_parser_CXXFLAGS	   = -Wall @CPPUNIT_CFLAGS@
suite_argument_parser_LDADD	   = -lcppunit @CPPUNIT_LIBS@ \
				     $(top_builddir)/src/ngsi_event_broker_host_la-argument_parser.lo

suite_broker_common_SOURCES	   = suite_broker_common.cc
nodist_suite_broker_common_SOURCES = $(UNITTEST_NAGIOS_MAIN)
suite_broker_common_CXXFLAGS	   = -fpermissive -w @CPPUNIT_CFLAGS@
suite_broker_common_LDADD	   = -lcppunit -lcurl -lm -lpthread -lltdl @CPPUNIT_LIBS@ \
				     $(top_builddir)/src/ngsi_event_broker_host_la-ngsi_event_broker_common.lo \
				     $(top_builddir)/src/ngsi_event_broker_host_la-argument_parser.lo \
				     $(NAGIOS_OBJECTS)

suite_broker_host_SOURCES	   = suite_broker_host.cc
nodist_suite_broker_host_SOURCES   = $(UNITTEST_NAGIOS_MAIN) $(UNITTEST_BROKER_MOCK)
suite_broker_host_CXXFLAGS	   = -fpermissive -w @CPPUNIT_CFLAGS@
suite_broker_host_LDADD		   = -lcppunit -lcurl -lm -lpthread -lltdl @CPPUNIT_LIBS@ \
				     $(top_builddir)/src/ngsi_event_broker_host_la-ngsi_event_broker_host.lo \
				     $(top_builddir)/src/ngsi_event_broker_host_la-argument_parser.lo \
				     $(NAGIOS_OBJECTS)

suite_broker_snmp_SOURCES	   = suite_broker_snmp.cc
nodist_suite_broker_snmp_SOURCES   = $(UNITTEST_NAGIOS_MAIN) $(UNITTEST_BROKER_MOCK)
suite_broker_snmp_CXXFLAGS	   = -fpermissive -w @CPPUNIT_CFLAGS@
suite_broker_snmp_LDADD		   = -lcppunit -lcurl -lm -lpthread -lltdl @CPPUNIT_LIBS@ \
				     $(top_builddir)/src/ngsi_event_broker_snmp_la-ngsi_event_broker_snmp.lo \
				     $(top_builddir)/src/ngsi_event_broker_snmp_la-argument_parser.lo \
				     $(NAGIOS_OBJECTS)

$(UNITTEST_NAGIOS_MAIN): @NAGIOS_SRCDIR@/base/nagios.c
	$(SED) 's/\(int[ \t]*\)main\([ \t]*(\)/\1nagios_main\2/' $< > $@

$(UNITTEST_BROKER_MOCK): $(top_builddir)/src/ngsi_event_broker_common.c
	$(SED) '/char\*[ \t]*find_plugin_name/,/^}/ { d; }' $< > $@
