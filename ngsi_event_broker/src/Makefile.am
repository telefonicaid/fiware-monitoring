lib_LTLIBRARIES			= ngsi_event_broker.la

ngsi_event_broker_la_SOURCES	= ngsi_event_broker.c metadata_parser.c metadata_parser.h
ngsi_event_broker_la_CPPFLAGS	= -DNDEBUG
ngsi_event_broker_la_CFLAGS	= -Wall
ngsi_event_broker_la_LDFLAGS	= -module -avoid-version
ngsi_event_broker_la_LIBADD	= -lcurl -lyajl

install-exec-hook:
	@rm -f $(DESTDIR)$(libdir)/ngsi_event_broker.la
