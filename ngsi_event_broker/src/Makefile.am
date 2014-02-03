lib_LTLIBRARIES				= ngsi_event_broker_snmp.la ngsi_event_broker_host.la

COMMON_SOURCES				= ngsi_event_broker_common.c ngsi_event_broker_common.h \
					  argument_parser.c argument_parser.h

METADATA_SOURCES			= metadata_parser.c metadata_parser.h
METADATA_LIBADD				= -lyajl

ngsi_event_broker_snmp_la_SOURCES	= $(COMMON_SOURCES) ngsi_event_broker_snmp.c
ngsi_event_broker_snmp_la_CPPFLAGS	= -DNDEBUG
ngsi_event_broker_snmp_la_CFLAGS	= -Wall
ngsi_event_broker_snmp_la_LDFLAGS	= -module -avoid-version
ngsi_event_broker_snmp_la_LIBADD	= -lcurl

ngsi_event_broker_host_la_SOURCES	= $(COMMON_SOURCES) ngsi_event_broker_host.c $(METADATA_SOURCES)
ngsi_event_broker_host_la_CPPFLAGS	= -DNDEBUG
ngsi_event_broker_host_la_CFLAGS	= -Wall
ngsi_event_broker_host_la_LDFLAGS	= -module -avoid-version
ngsi_event_broker_host_la_LIBADD	= -lcurl $(METADATA_LIBADD)

install-exec-hook:
	@rm -f $(DESTDIR)$(libdir)/ngsi_event_broker_*.la
