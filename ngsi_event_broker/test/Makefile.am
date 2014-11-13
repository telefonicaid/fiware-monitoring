UNITTESTS_PROGS				= suite_argument_parser \
					  suite_broker_common   \
					  suite_broker_xifi_dem \
					  suite_broker_xifi_npm \
					  suite_broker_xifi_srv

UNITTESTS_NAGIOS_MAIN			= nagios_main.c
UNITTESTS_NAGIOS_NEBMODS		= nagios_nebmods.c
UNITTESTS_BROKER_COMMON_STUB		= broker_common_stub.c

noinst_PROGRAMS				= $(UNITTESTS_PROGS)
check_PROGRAMS				= $(UNITTESTS_PROGS)
TESTS					= $(check_PROGRAMS)

NAGIOS_BASE_OBJECTS			= $(wildcard @NAGIOS_SRCDIR@/base/*.o)
NAGIOS_COMMON_OBJECTS			= $(wildcard @NAGIOS_SRCDIR@/common/*.o)
NAGIOS_OBJECTS				= $(filter-out %/nagios.o, $(NAGIOS_BASE_OBJECTS) $(NAGIOS_COMMON_OBJECTS))
NAGIOS_LIBS				= $(shell awk '/^(MATH|PERL|SOCKET|THREAD|BROKER)LIBS/ {sub(/.*=/,""); print}' \
					  @NAGIOS_SRCDIR@/base/Makefile)

suite_argument_parser_SOURCES		= suite_argument_parser.cc
suite_argument_parser_CXXFLAGS		= -Wall @CPPUNIT_CFLAGS@
suite_argument_parser_LDADD		= @CPPUNIT_LIBS@ \
					  $(top_builddir)/src/ngsi_event_broker_xifi_la-argument_parser.lo

suite_broker_common_SOURCES		= suite_broker_common.cc
nodist_suite_broker_common_SOURCES	= $(UNITTESTS_NAGIOS_MAIN) $(UNITTESTS_NAGIOS_NEBMODS)
suite_broker_common_CXXFLAGS		= -fpermissive -w @CPPUNIT_CFLAGS@
suite_broker_common_LDADD		= $(NAGIOS_LIBS) @CPPUNIT_LIBS@ \
					  $(top_builddir)/src/ngsi_event_broker_xifi_la-ngsi_event_broker_common.lo \
					  $(top_builddir)/src/ngsi_event_broker_xifi_la-argument_parser.lo \
					  $(filter-out %/nebmods.o, $(NAGIOS_OBJECTS))

suite_broker_xifi_dem_SOURCES		= suite_broker_xifi_dem.cc
nodist_suite_broker_xifi_dem_SOURCES	= $(UNITTESTS_NAGIOS_MAIN) $(UNITTESTS_BROKER_COMMON_STUB)
suite_broker_xifi_dem_CXXFLAGS		= -fpermissive -w @CPPUNIT_CFLAGS@
suite_broker_xifi_dem_LDADD		= -lcurl $(NAGIOS_LIBS) @CPPUNIT_LIBS@ \
					  $(top_builddir)/src/ngsi_event_broker_xifi_la-ngsi_event_broker_xifi.lo \
					  $(top_builddir)/src/ngsi_event_broker_xifi_la-argument_parser.lo \
					  $(NAGIOS_OBJECTS)

suite_broker_xifi_npm_SOURCES		= suite_broker_xifi_npm.cc
nodist_suite_broker_xifi_npm_SOURCES	= $(UNITTESTS_NAGIOS_MAIN) $(UNITTESTS_BROKER_COMMON_STUB)
suite_broker_xifi_npm_CXXFLAGS		= -fpermissive -w @CPPUNIT_CFLAGS@
suite_broker_xifi_npm_LDADD		= -lcurl $(NAGIOS_LIBS) @CPPUNIT_LIBS@ \
					  $(top_builddir)/src/ngsi_event_broker_xifi_la-ngsi_event_broker_xifi.lo \
					  $(top_builddir)/src/ngsi_event_broker_xifi_la-argument_parser.lo \
					  $(NAGIOS_OBJECTS)

suite_broker_xifi_srv_SOURCES		= suite_broker_xifi_srv.cc
nodist_suite_broker_xifi_srv_SOURCES	= $(UNITTESTS_NAGIOS_MAIN) $(UNITTESTS_BROKER_COMMON_STUB)
suite_broker_xifi_srv_CXXFLAGS		= -fpermissive -w @CPPUNIT_CFLAGS@
suite_broker_xifi_srv_LDADD		= -lcurl $(NAGIOS_LIBS) @CPPUNIT_LIBS@ \
					  $(top_builddir)/src/ngsi_event_broker_xifi_la-ngsi_event_broker_xifi.lo \
					  $(top_builddir)/src/ngsi_event_broker_xifi_la-argument_parser.lo \
					  $(NAGIOS_OBJECTS)

$(UNITTESTS_NAGIOS_MAIN): @NAGIOS_SRCDIR@/base/nagios.c
	$(SED) 's/\(int[ \t]*\)main\([ \t]*(\)/\1nagios_main\2/' $< > $@

$(UNITTESTS_NAGIOS_NEBMODS): @NAGIOS_SRCDIR@/base/nebmods.c
	$(SED) '/int neb_register_callback/,/return OK;/ c\void foo(){' $< > $@

$(UNITTESTS_BROKER_COMMON_STUB): $(top_builddir)/src/ngsi_event_broker_common.c
	$(SED) '/char\*[ \t]*find_plugin_command_name/,/^}/ { d; }' $< > $@

# remove gcov and cppunit files
clean-local:
	rm -f *.gcda *.gcno *-cppunit-results.xml
