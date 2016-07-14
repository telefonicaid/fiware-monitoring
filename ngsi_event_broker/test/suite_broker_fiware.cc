/*
 * Copyright 2013-2016 Telefónica I+D
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


/**
 * @file   suite_broker_fiware.cc
 * @brief  Test suite to verify FIWARE-specific event broker
 *
 * This file defines unit tests to verify FIWARE-specific event broker
 * implementation (see ngsi_event_broker_fiware.c).
 */


#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <climits>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "suite_config.h"
#include "ngsi_event_broker_common.h"
#include "ngsi_event_broker_fiware.h"
#include "neberrors.h"
#include "nebcallbacks.h"
#include "broker.h"
#include "curl/curl.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestFixture.h"
#include "cppunit/TextTestRunner.h"
#include "cppunit/XmlOutputter.h"
#include "cppunit/BriefTestProgressListener.h"
#include "cppunit/extensions/HelperMacros.h"


using CppUnit::TestResult;
using CppUnit::TestFixture;
using CppUnit::TextTestRunner;
using CppUnit::XmlOutputter;
using CppUnit::BriefTestProgressListener;
using namespace std;


// Forward declarations for friend members
extern "C" {
	int			__wrap_gethostname(char*, size_t);
	struct hostent*		__wrap_gethostbyname(const char*);
	int			__wrap_neb_set_module_info(void*, int, char*);
	int			__wrap_neb_register_callback(int, void*, int, int (*)(int, void*));
	host*			__wrap_find_host(char*);
	service*		__wrap_find_service(char*, char*);
	command*		__wrap_find_command(char*);
	int			__wrap_grab_host_macros_r(nagios_macros*, host*);
	int			__wrap_grab_service_macros_r(nagios_macros*, service*);
	int			__wrap_get_raw_command_line_r(nagios_macros*, command*, char*, char**, int);
	int			__wrap_process_macros_r(nagios_macros*, char*, char**, int);
	CURLcode		__wrap_curl_global_init(long);
	void			__wrap_curl_global_cleanup(void);
	CURL*			__wrap_curl_easy_init(void);
	CURLcode		__wrap_curl_easy_setopt(CURL*, CURLoption, ...);
	CURLcode		__wrap_curl_easy_perform(CURL*);
	void			__wrap_curl_easy_cleanup(CURL*);
	const char*		__wrap_curl_easy_strerror(CURLcode);
}


/// FIWARE Broker test suite
class BrokerFiwareTest: public TestFixture
{
	// mocks: return & output values, hit counters, and friend declaration to access static members
	static int		__retval_gethostname;
	friend int		::__wrap_gethostname(char*, size_t);
	friend struct hostent*	::__wrap_gethostbyname(const char*);
	static int		__retval_neb_set_module_info;
	friend int		::__wrap_neb_set_module_info(void*, int, char*);
	static int		__retval_neb_register_callback;
	friend int		::__wrap_neb_register_callback(int, void*, int, int (*)(int, void*));
	static host*		__retval_find_host;
	friend host*		::__wrap_find_host(char*);
	static service*		__retval_find_service;
	friend service*		::__wrap_find_service(char*, char*);
	static command*		__retval_find_command;
	friend command*		::__wrap_find_command(char*);
	static int		__retval_grab_host_macros_r;
	friend int		::__wrap_grab_host_macros_r(nagios_macros*, host*);
	static int		__retval_grab_service_macros_r;
	friend int		::__wrap_grab_service_macros_r(nagios_macros*, service*);
	static char*		__output_get_raw_command_line_r;
	static int		__retval_get_raw_command_line_r;
	friend int		::__wrap_get_raw_command_line_r(nagios_macros*, command*, char*, char**, int);
	static char*		__output_process_macros_r;
	static int		__retval_process_macros_r;
	friend int		::__wrap_process_macros_r(nagios_macros*, char*, char**, int);
	static CURLcode		__retval_curl_global_init;
	friend CURLcode		::__wrap_curl_global_init(long);
	friend void		::__wrap_curl_global_cleanup(void);
	static CURL*		__retval_curl_easy_init;
	friend CURL*		::__wrap_curl_easy_init(void);
	static bool		__header_curl_easy_setopt;
	static CURLcode		__retval_curl_easy_setopt;
	friend CURLcode		::__wrap_curl_easy_setopt(CURL*, CURLoption, ...);
	static size_t		__hitcnt_curl_easy_perform;
	static CURLcode		__retval_curl_easy_perform;
	friend CURLcode		::__wrap_curl_easy_perform(CURL*);
	friend void		::__wrap_curl_easy_cleanup(CURL*);
	static const char*	__retval_curl_easy_strerror;
	friend const char*	::__wrap_curl_easy_strerror(CURLcode);

	// static methods equivalent to external C functions
	static int		nebmodule_init(int, const string&, void*);
	static int		nebmodule_deinit(int, int);

	// tests
	void callback_skips_request_if_invoked_before_plugin_exec_ends();
	void callback_skips_request_if_cannot_find_host();
	void callback_skips_request_if_cannot_find_service();
	void callback_skips_request_if_cannot_find_command();
	void callback_skips_request_if_cannot_find_command_arguments();
	void callback_skips_request_if_cannot_find_custom_variables();
	void callback_skips_request_if_cannot_find_custom_variable_entity_type();
	void callback_skips_request_if_unknown_entity_type();
	void callback_skips_request_if_curl_initialization_fails();
	void callback_skips_request_if_curl_perform_fails();
	void callback_sends_request_if_curl_perform_succeeds();
	void callback_sends_request_with_txid_and_content_type_headers();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(BrokerFiwareTest);
	CPPUNIT_TEST(callback_skips_request_if_invoked_before_plugin_exec_ends);
	CPPUNIT_TEST(callback_skips_request_if_cannot_find_host);
	CPPUNIT_TEST(callback_skips_request_if_cannot_find_service);
	CPPUNIT_TEST(callback_skips_request_if_cannot_find_command);
	CPPUNIT_TEST(callback_skips_request_if_cannot_find_command_arguments);
	CPPUNIT_TEST(callback_skips_request_if_cannot_find_custom_variables);
	CPPUNIT_TEST(callback_skips_request_if_cannot_find_custom_variable_entity_type);
	CPPUNIT_TEST(callback_skips_request_if_unknown_entity_type);
	CPPUNIT_TEST(callback_skips_request_if_curl_initialization_fails);
	CPPUNIT_TEST(callback_skips_request_if_curl_perform_fails);
	CPPUNIT_TEST(callback_sends_request_if_curl_perform_succeeds);
	CPPUNIT_TEST(callback_sends_request_with_txid_and_content_type_headers);
	CPPUNIT_TEST_SUITE_END();
};


/// Suite startup
int main(int argc, char* argv[])
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(BrokerFiwareTest::suite());
	BrokerFiwareTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	BrokerFiwareTest::suiteTearDown();
	ofstream xmlFileOut((string(argv[0]) + "-cppunit-results.xml").c_str());
	XmlOutputter xmlOut(&runner.result(), xmlFileOut);
	xmlOut.write();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


/// Return value from ::__wrap_gethostname
int BrokerFiwareTest::__retval_gethostname = EXIT_SUCCESS;


/// Mock for ::gethostname
int __wrap_gethostname(char* name, size_t len)
{
	memcpy(name, LOCALHOST_NAME, len);
	return BrokerFiwareTest::__retval_gethostname;
}


/// Mock for ::gethostbyname
struct hostent* __wrap_gethostbyname(const char* name)
{
	static struct hostent	host;
	static struct in_addr	addr;
	static char*		list[] = { (char*) &addr, NULL };

	string			hostname(name);
	const char*		hostaddr;

	if (hostname == LOCALHOST_NAME || hostname == LOCALHOST_ADDR) {
		hostaddr = LOCALHOST_ADDR;
	} else if (hostname == REMOTEHOST_NAME || hostname == REMOTEHOST_ADDR) {
		hostaddr = REMOTEHOST_ADDR;
	} else {
		return NULL;	// host not found
	}

	host.h_addr_list = (char**) list;
	inet_pton(AF_INET, hostaddr, host.h_addr_list[0]);
	return &host;
}


/// Return value from ::__wrap_neb_set_module_info
int BrokerFiwareTest::__retval_neb_set_module_info = NEB_OK;


/// Mock for ::neb_set_module_info
int __wrap_neb_set_module_info(void* handle, int type, char* data)
{
	return BrokerFiwareTest::__retval_neb_set_module_info;
}


/// Return value from ::__wrap_neb_register_callback
int BrokerFiwareTest::__retval_neb_register_callback = NEB_OK;


/// Mock for ::neb_register_callback
int __wrap_neb_register_callback(int type, void* handle, int priority, int (*func)(int, void*))
{
	return BrokerFiwareTest::__retval_neb_register_callback;
}


/// Return value from ::__wrap_find_host
host* BrokerFiwareTest::__retval_find_host = NULL;


/// Mock for ::find_host
host* __wrap_find_host(char* name)
{
	return BrokerFiwareTest::__retval_find_host;
}


/// Return value from ::__wrap_find_service
service* BrokerFiwareTest::__retval_find_service = NULL;


/// Mock for ::find_service
service* __wrap_find_service(char* name, char* svc_desc)
{
	return BrokerFiwareTest::__retval_find_service;
}


/// Return value from ::__wrap_find_command
command* BrokerFiwareTest::__retval_find_command = NULL;


/// Mock for ::find_command
command* __wrap_find_command(char* name)
{
	return BrokerFiwareTest::__retval_find_command;
}


/// Return value from ::__wrap_grab_host_macros_r
int BrokerFiwareTest::__retval_grab_host_macros_r = EXIT_SUCCESS;


/// Mock for ::grab_host_macros_r
int __wrap_grab_host_macros_r(nagios_macros* mac, host* hst)
{
	return BrokerFiwareTest::__retval_grab_host_macros_r;
}


/// Return value from ::__wrap_grab_service_macros_r
int BrokerFiwareTest::__retval_grab_service_macros_r = EXIT_SUCCESS;


/// Mock for ::grab_service_macros_r
int __wrap_grab_service_macros_r(nagios_macros* mac, service* svc)
{
	return BrokerFiwareTest::__retval_grab_service_macros_r;
}


/// Output value for `full_command` from ::__wrap_get_raw_command_line_r
char* BrokerFiwareTest::__output_get_raw_command_line_r = NULL;


/// Return value from ::__wrap_get_raw_command_line_r
int BrokerFiwareTest::__retval_get_raw_command_line_r = EXIT_SUCCESS;


/// Mock for ::get_raw_command_line_r
int __wrap_get_raw_command_line_r(nagios_macros* mac, command* ptr, char* cmd, char** full_command, int macro_options)
{
	if (full_command) {
		*full_command = STRDUP(BrokerFiwareTest::__output_get_raw_command_line_r);
	}
	return BrokerFiwareTest::__retval_get_raw_command_line_r;
}


/// Output value for `output_buffer` from ::__wrap_process_macros_r
char* BrokerFiwareTest::__output_process_macros_r = NULL;


/// Return value from ::__wrap_process_macros_r
int BrokerFiwareTest::__retval_process_macros_r = EXIT_SUCCESS;


/// Mock for ::process_macros_r
int __wrap_process_macros_r(nagios_macros* mac, char* input_buffer, char** output_buffer, int options)
{
	if (output_buffer) {
		*output_buffer = STRDUP(BrokerFiwareTest::__output_process_macros_r);
	}
	return BrokerFiwareTest::__retval_process_macros_r;
}


/// Return value from ::__wrap_curl_global_init
CURLcode BrokerFiwareTest::__retval_curl_global_init = CURLE_OK;


/// Mock for ::curl_global_init
CURLcode __wrap_curl_global_init(long flags)
{
	return BrokerFiwareTest::__retval_curl_global_init;
}


/// Mock for ::curl_global_cleanup
void __wrap_curl_global_cleanup(void)
{
}


/// Return value from ::__wrap_curl_easy_init
CURL* BrokerFiwareTest::__retval_curl_easy_init = NULL;


/// Mock for ::curl_easy_init
CURL* __wrap_curl_easy_init(void)
{
	return BrokerFiwareTest::__retval_curl_easy_init;
}


/// Flag variable for ::__wrap_curl_easy_setopt
/// @brief True if required HTTP headers (`Content-Type` and ::TXID_HTTP_HEADER) are set
bool BrokerFiwareTest::__header_curl_easy_setopt = false;


/// Return value from ::__wrap_curl_easy_setopt
CURLcode BrokerFiwareTest::__retval_curl_easy_setopt = CURLE_OK;


/// Mock for ::curl_easy_setopt
CURLcode __wrap_curl_easy_setopt(CURL* handle, CURLoption option, ...)
{
	if ((BrokerFiwareTest::__retval_curl_easy_setopt == CURLE_OK) && (option == CURLOPT_HTTPHEADER)) {
		bool		has_txid = false;
		bool		has_type = false;
		curl_slist*	headers  = NULL;

		va_list ap;
		va_start(ap, option);
		headers = va_arg(ap, curl_slist*);
		va_end(ap);

		string txid(TXID_HTTP_HEADER);
		string type("Content-Type");
		for (curl_slist* ptr = headers; ptr; ptr = ptr->next) {
			string header(ptr->data);
			has_txid = has_txid || (header.compare(0, txid.size(), txid) == 0);
			has_type = has_type || (header.compare(0, type.size(), type) == 0);
		}

		BrokerFiwareTest::__header_curl_easy_setopt = (has_txid && has_type);
	}

	return BrokerFiwareTest::__retval_curl_easy_setopt;
}


/// Hit counter for ::__wrap_curl_easy_perform
size_t BrokerFiwareTest::__hitcnt_curl_easy_perform = 0;


/// Return value from ::__wrap_curl_easy_perform
CURLcode BrokerFiwareTest::__retval_curl_easy_perform = CURLE_OK;


/// Mock for ::curl_easy_perform
CURLcode __wrap_curl_easy_perform(CURL* handle)
{
	if (BrokerFiwareTest::__retval_curl_easy_perform == CURLE_OK) {
		++BrokerFiwareTest::__hitcnt_curl_easy_perform;
	}
	return BrokerFiwareTest::__retval_curl_easy_perform;
}


/// Mock for ::curl_easy_cleanup
void __wrap_curl_easy_cleanup(CURL* handle)
{
}


/// Return value from ::__wrap_curl_easy_strerror
const char* BrokerFiwareTest::__retval_curl_easy_strerror = NULL;


/// Mock for ::curl_easy_strerror
const char* __wrap_curl_easy_strerror(CURLcode errornum)
{
	return BrokerFiwareTest::__retval_curl_easy_strerror;
}


///
/// Static method wrapping C function ::nebmodule_init from Nagios
///
/// @param[in] flags	The initialization flags (ignored).
/// @param[in] args	The module arguments as a space-separated string.
/// @param[in] handle	The module handle passed by Nagios Core server.
///
/// @retval NEB_OK	Successfully initialized.
/// @retval NEB_ERROR	Not successfully initialized.
///
int BrokerFiwareTest::nebmodule_init(int flags, const string& args, void* handle)
{
	char buffer[MAXBUFLEN];
	buffer[args.copy(buffer, MAXBUFLEN-1)] = '\0';
	return ::nebmodule_init(flags, buffer, handle);
}


///
/// Static method wrapping C function ::nebmodule_deinit from Nagios
///
/// @param[in] flags	The deinitialization flags (ignored).
/// @param[in] reason	The reason why this module is being deinitialized.
///
/// @retval NEB_OK	Successfully deinitialized.
///
int BrokerFiwareTest::nebmodule_deinit(int flags, int reason)
{
	return ::nebmodule_deinit(flags, reason);
}


///
/// Suite setup
///
void BrokerFiwareTest::suiteSetUp()
{
	// Setup broker arguments
	string argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << ADAPTER_URL
		<< ' ' << "-r" << REGION_ID
		)).str();

	::adapter_url	= NULL;
	::region_id	= NULL;
	::host_addr	= NULL;
	nebmodule_init(0, argline, MODULE_HANDLE);
}


///
/// Suite teardown
///
void BrokerFiwareTest::suiteTearDown()
{
	nebmodule_deinit(0, NEBMODULE_NEB_SHUTDOWN);
}


///
/// Tests setup
///
void BrokerFiwareTest::setUp()
{
}


///
/// Tests teardown
///
void BrokerFiwareTest::tearDown()
{
	nebmodule_deinit(0, NEBMODULE_NEB_SHUTDOWN);
	__retval_gethostname			= EXIT_SUCCESS;
	__retval_neb_set_module_info		= NEB_OK;
	__retval_neb_register_callback		= NEB_OK;
	__retval_find_host			= NULL;
	__retval_find_service			= NULL;
	__retval_find_command			= NULL;
	__retval_grab_host_macros_r		= EXIT_SUCCESS;
	__retval_grab_service_macros_r		= EXIT_SUCCESS;
	__output_get_raw_command_line_r		= NULL;
	__retval_get_raw_command_line_r		= EXIT_SUCCESS;
	__output_process_macros_r		= NULL;
	__retval_process_macros_r		= EXIT_SUCCESS;
	__retval_curl_global_init		= CURLE_OK;
	__retval_curl_easy_init			= NULL;
	__retval_curl_easy_setopt		= CURLE_OK;
	__retval_curl_easy_perform		= CURLE_OK;
	__retval_curl_easy_strerror		= NULL;
	__header_curl_easy_setopt		= false;
	__hitcnt_curl_easy_perform		= 0;
}


////////////////////////////////////////////////////////////////////////////////


void BrokerFiwareTest::callback_skips_request_if_invoked_before_plugin_exec_ends()
{
	nebstruct_service_check_data		check_data;

	// given
	check_data.host_name			= REMOTEHOST_ADDR;
	check_data.type				= NEBTYPE_SERVICECHECK_INITIATE;	// plugin execution initiated
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_skips_request_if_cannot_find_host()
{
	nebstruct_service_check_data		check_data;

	// given
	__retval_find_host			= NULL;	// no valid host struct
	check_data.host_name			= REMOTEHOST_ADDR;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_skips_request_if_cannot_find_service()
{
	host					check_host;
	nebstruct_service_check_data		check_data;

	// given
	__retval_find_service			= NULL;	// no valid service struct
	__retval_find_host			= &check_host;
	check_data.host_name			= REMOTEHOST_ADDR;
	check_data.service_description		= SOME_DESCRIPTION;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_skips_request_if_cannot_find_command()
{
	host					check_host;
	service					check_service;
	nebstruct_service_check_data		check_data;

	// given
	__retval_find_command			= NULL;	// no valid command struct
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME "!" SOME_CHECK_ARGS;
	check_service.description		= SOME_DESCRIPTION;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_skips_request_if_cannot_find_command_arguments()
{
	host					check_host;
	service					check_service;
	command					check_command;
	nebstruct_service_check_data		check_data;

	// given
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME "!" SOME_CHECK_ARGS;
	check_service.description		= SOME_DESCRIPTION;
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/bin/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	__output_get_raw_command_line_r		= NULL;		// get command line fails
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_skips_request_if_cannot_find_custom_variables()
{
	host					check_host;
	service					check_service;
	command					check_command;
	nebstruct_service_check_data		check_data;

	// given
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME "!" SOME_CHECK_ARGS;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= NULL;
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/bin/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_skips_request_if_cannot_find_custom_variable_entity_type()
{
	host					check_host;
	service					check_service;
	command					check_command;
	customvariablesmember			check_vars;
	nebstruct_service_check_data		check_data;

	// given
	check_vars = {
		variable_name:			"some_variable",
		variable_value:			"some_value"
	};
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME "!" SOME_CHECK_ARGS;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= &check_vars;
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/bin/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_skips_request_if_unknown_entity_type()
{
	host					check_host;
	service					check_service;
	command					check_command;
	customvariablesmember			check_vars;
	nebstruct_service_check_data		check_data;

	// given
	check_vars = {
		variable_name:			CUSTOM_VAR_ENTITY_TYPE,
		variable_value:			"unknown_value"
	};
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME "!" SOME_CHECK_ARGS;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= &check_vars;
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/bin/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_skips_request_if_curl_initialization_fails()
{
	host					check_host;
	service					check_service;
	command					check_command;
	customvariablesmember			check_vars;
	nebstruct_service_check_data		check_data;

	// given
	check_vars = {
		variable_name:			CUSTOM_VAR_ENTITY_TYPE,
		variable_value:			GE_ENTITY_TYPE
	};
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME "!" SOME_CHECK_ARGS;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= &check_vars;
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/bin/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	__retval_curl_easy_init			= NULL;		// initialization fails
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_skips_request_if_curl_perform_fails()
{
	host					check_host;
	service					check_service;
	command					check_command;
	customvariablesmember			check_vars;
	nebstruct_service_check_data		check_data;

	// given
	check_vars = {
		variable_name:			CUSTOM_VAR_ENTITY_TYPE,
		variable_value:			GE_ENTITY_TYPE
	};
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME "!" SOME_CHECK_ARGS;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= &check_vars;
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/bin/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	__retval_curl_easy_init			= CURL_HANDLE;
	__retval_curl_easy_perform		= CURLE_COULDNT_CONNECT;	// easy_perform() fails
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_sends_request_if_curl_perform_succeeds()
{
	host					check_host;
	service					check_service;
	command					check_command;
	customvariablesmember			check_vars;
	nebstruct_service_check_data		check_data;

	// given
	check_vars = {
		variable_name:			CUSTOM_VAR_ENTITY_TYPE,
		variable_value:			GE_ENTITY_TYPE
	};
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME "!" SOME_CHECK_ARGS;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= &check_vars;
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/bin/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	__retval_curl_easy_init			= CURL_HANDLE;
	__retval_curl_easy_perform		= CURLE_OK;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 1;	// easy_perform() is invoked and succeedes

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerFiwareTest::callback_sends_request_with_txid_and_content_type_headers()
{
	host					check_host;
	service					check_service;
	command					check_command;
	customvariablesmember			check_vars;
	nebstruct_service_check_data		check_data;

	// given
	check_vars = {
		variable_name:			CUSTOM_VAR_ENTITY_TYPE,
		variable_value:			GE_ENTITY_TYPE
	};
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME "!" SOME_CHECK_ARGS;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= &check_vars;
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/bin/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	__retval_curl_easy_init			= CURL_HANDLE;
	__retval_curl_easy_perform		= CURLE_OK;
	__header_curl_easy_setopt		= false;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 1;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
	CPPUNIT_ASSERT_EQUAL(BrokerFiwareTest::__header_curl_easy_setopt, true);
}
