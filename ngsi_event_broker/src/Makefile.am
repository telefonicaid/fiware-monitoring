lib_LTLIBRARIES				= ngsi_event_broker_xifi.la

COMMON_SOURCES				= ngsi_event_broker_common.c ngsi_event_broker_common.h \
					  argument_parser.c argument_parser.h

ngsi_event_broker_xifi_la_SOURCES	= $(COMMON_SOURCES) ngsi_event_broker_xifi.c ngsi_event_broker_xifi.h
ngsi_event_broker_xifi_la_CPPFLAGS	= -DNDEBUG
ngsi_event_broker_xifi_la_CFLAGS	= -Wall
ngsi_event_broker_xifi_la_LDFLAGS	= -module -avoid-version
ngsi_event_broker_xifi_la_LIBADD	= -lcurl

# remove unnecessary files
install-exec-hook:
	@rm -f $(DESTDIR)$(libdir)/ngsi_event_broker_*.la


# reset all execution counts to zero prior coverage check
if GCOV_ENABLED
check-local:
	lcov -d .libs --zerocounters
endif
