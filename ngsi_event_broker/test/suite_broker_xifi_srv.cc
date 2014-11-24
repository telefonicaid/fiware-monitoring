/*
 * Copyright 2013 Telefónica I+D
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


/**
 * @file   suite_broker_xifi_srv.cc
 * @brief  Test suite to verify host service monitoring features from XIFI-specific
 *         event broker
 *
 * This file defines unit tests to verify host service monitoring features from
 * the XIFI-specific event broker implementation (see ngsi_event_broker_xifi.c).
 */


#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "ngsi_event_broker_common.h"
#include "ngsi_event_broker_xifi.h"
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


/// Some custom entity type
#define SOME_ENTITY_TYPE	"some_type"


/// Some check name
#define SOME_CHECK_NAME		"some_check"


/// Some check arguments
#define SOME_CHECK_ARGS		"-w 10 -c 20"


/// Some service description
#define SOME_DESCRIPTION	"nova-scheduler"


/// Some region id
#define REGION_ID		"some_region"


/// Fake adapter URL
#define ADAPTER_URL		"http://adapter_host:5000"


/// Fake local host address
#define LOCALHOST_ADDR		"10.95.0.6"


/// Fake local host name
#define LOCALHOST_NAME		"my_local_host"


/// Fake remote address
#define REMOTEHOST_ADDR		"169.254.0.1"


/// Fake remote host name
#define REMOTEHOST_NAME		"my_remote_host"


///
/// @name Mocks for system calls
/// @{
///
extern "C" {
	int			__wrap_gethostname(char*, size_t);
	struct hostent*		__wrap_gethostbyname(const char*);
}

/// @}


///
/// @name Mocks for Nagios functions
/// @{
///
extern "C" {
	host*			__wrap_find_host(char*);
	service*		__wrap_find_service(char*, char*);
	command*		__wrap_find_command(char*);
	int			__wrap_grab_host_macros_r(nagios_macros*, host*);
	int			__wrap_grab_service_macros_r(nagios_macros*, service*);
	int			__wrap_get_raw_command_line_r(nagios_macros*, command*, char*, char**, int);
	int			__wrap_process_macros_r(nagios_macros*, char*, char**, int);
}

/// @}


/// XIFI Broker (host service monitoring features) test suite
class BrokerXifiSrvTest: public TestFixture
{
	// mocks: return & output values, and friend declaration to access static members
	static int		__retval_gethostname;
	friend int		::__wrap_gethostname(char*, size_t);
	friend struct hostent*	::__wrap_gethostbyname(const char*);
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

	// static methods equivalent to external C functions
	static int		init_module_variables(const string&);
	static int		free_module_variables();
	static const char*	get_adapter_request(nebstruct_service_check_data*, string&);

	// tests
	void wrong_request_local_plugin_implicit_entity_type();
	void get_request_ok_local_plugin_explicit_entity_type();
	void wrong_request_remote_plugin_implicit_entity_type();
	void get_request_ok_remote_plugin_explicit_entity_type();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(BrokerXifiSrvTest);
	CPPUNIT_TEST(wrong_request_local_plugin_implicit_entity_type);
	CPPUNIT_TEST(get_request_ok_local_plugin_explicit_entity_type);
	CPPUNIT_TEST(wrong_request_remote_plugin_implicit_entity_type);
	CPPUNIT_TEST(get_request_ok_remote_plugin_explicit_entity_type);
	CPPUNIT_TEST_SUITE_END();
};


/// Suite startup
int main(int argc, char* argv[])
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(BrokerXifiSrvTest::suite());
	BrokerXifiSrvTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	BrokerXifiSrvTest::suiteTearDown();
	ofstream xmlFileOut((string(argv[0]) + "-cppunit-results.xml").c_str());
	XmlOutputter xmlOut(&runner.result(), xmlFileOut);
	xmlOut.write();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


///
/// @name Mock for gethostname()
/// @{
///

/// Return value
int BrokerXifiSrvTest::__retval_gethostname = EXIT_SUCCESS;

/// Mock function
int __wrap_gethostname(char* name, size_t len)
{
	memcpy(name, LOCALHOST_NAME, len);
	return BrokerXifiSrvTest::__retval_gethostname;
}

/// @}


///
/// @name Mock for gethostbyname()
/// @{
///

/// Mock function
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

/// @}


///
/// @name Mock for find_host()
/// @{
///

/// Return value
host* BrokerXifiSrvTest::__retval_find_host = NULL;

/// Mock function
host* __wrap_find_host(char* name)
{
	return BrokerXifiSrvTest::__retval_find_host;
}

/// @}


///
/// @name Mock for find_service()
/// @{
///

/// Return value
service* BrokerXifiSrvTest::__retval_find_service = NULL;

/// Mock function
service* __wrap_find_service(char* name, char* svc_desc)
{
	return BrokerXifiSrvTest::__retval_find_service;
}

/// @}


///
/// @name Mock for find_command()
/// @{
///

/// Return value
command* BrokerXifiSrvTest::__retval_find_command = NULL;

/// Mock function
command* __wrap_find_command(char* name)
{
	return BrokerXifiSrvTest::__retval_find_command;
}

/// @}


///
/// @name Mock for grab_host_macros_r()
/// @{
///

/// Return value
int BrokerXifiSrvTest::__retval_grab_host_macros_r = EXIT_SUCCESS;

/// Mock function
int __wrap_grab_host_macros_r(nagios_macros* mac, host* hst)
{
	return BrokerXifiSrvTest::__retval_grab_host_macros_r;
}

/// @}


///
/// @name Mock for grab_service_macros_r()
/// @{
///

/// Return value
int BrokerXifiSrvTest::__retval_grab_service_macros_r = EXIT_SUCCESS;

/// Mock function
int __wrap_grab_service_macros_r(nagios_macros* mac, service* svc)
{
	return BrokerXifiSrvTest::__retval_grab_service_macros_r;
}

/// @}


///
/// @name Mock for get_raw_command_line_r()
/// @{
///

/// Output value for `full_command`
char* BrokerXifiSrvTest::__output_get_raw_command_line_r = NULL;

/// Return value
int BrokerXifiSrvTest::__retval_get_raw_command_line_r = EXIT_SUCCESS;

/// Mock function
int __wrap_get_raw_command_line_r(nagios_macros* mac, command* ptr, char* cmd, char** full_command, int macro_options)
{
	if (full_command) {
		*full_command = (BrokerXifiSrvTest::__output_get_raw_command_line_r) ?
			strdup(BrokerXifiSrvTest::__output_get_raw_command_line_r) :
			NULL;
	}
	return BrokerXifiSrvTest::__retval_get_raw_command_line_r;
}

/// @}


///
/// @name Mock for process_macros_r()
/// @{
///

/// Output value for `output_buffer`
char* BrokerXifiSrvTest::__output_process_macros_r = NULL;

/// Return value
int BrokerXifiSrvTest::__retval_process_macros_r = EXIT_SUCCESS;

/// Mock function
int __wrap_process_macros_r(nagios_macros* mac, char* input_buffer, char** output_buffer, int options)
{
	if (output_buffer) {
		*output_buffer = strdup(BrokerXifiSrvTest::__output_process_macros_r);
	}
	return BrokerXifiSrvTest::__retval_process_macros_r;
}


///
/// Static method for C function ::init_module_variables()
///
/// @param[in] args			The module arguments as a space-separated string.
///
/// @retval NEB_OK			Successfully initialized.
/// @retval NEB_ERROR			Not successfully initialized.
///
int BrokerXifiSrvTest::init_module_variables(const string& args)
{
	char buffer[MAXBUFLEN];
	buffer[args.copy(buffer, MAXBUFLEN-1)] = '\0';
	context_t* context = NULL;
	return ::init_module_variables(buffer, context);
}


///
/// Static method for C function ::free_module_variables()
///
/// @retval NEB_OK			Success.
///
int BrokerXifiSrvTest::free_module_variables()
{
	return ::free_module_variables();
}


///
/// Static method for C function ::get_adapter_request()
///
/// @param[in]  data			The plugin data passed by Nagios to the registered callback_service_check().
/// @param[out] request			String storing the request URL to invoke NGSI Adapter (including query string).
///
/// @return				Pointer to the string storing the request.
/// @retval ADAPTER_REQUEST_INVALID	An error ocurred and no request could be composed.
/// @retval ADAPTER_REQUEST_IGNORE	Skip request to NGSI Adapter.
///
const char* BrokerXifiSrvTest::get_adapter_request(nebstruct_service_check_data* data, string& request)
{
	context_t* context = NULL;
	char* adapter_request_str = ::get_adapter_request(data, context);
	request.assign((adapter_request_str == ADAPTER_REQUEST_INVALID) ? "{invalid}" : adapter_request_str);
	const char* result = (adapter_request_str == ADAPTER_REQUEST_INVALID) ? adapter_request_str : request.c_str();
	::free(adapter_request_str);
	adapter_request_str = NULL;
	return result;
}


///
/// Suite setup
///
void BrokerXifiSrvTest::suiteSetUp()
{
	// Setup broker arguments
	string argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << ADAPTER_URL
		<< ' ' << "-r" << REGION_ID
		)).str();

	::adapter_url	= NULL;
	::region_id	= NULL;
	::host_addr	= NULL;
	init_module_variables(argline);
}


///
/// Suite teardown
///
void BrokerXifiSrvTest::suiteTearDown()
{
	free_module_variables();
}


///
/// Tests setup
///
void BrokerXifiSrvTest::setUp()
{
}


///
/// Tests teardown
///
void BrokerXifiSrvTest::tearDown()
{
	__retval_gethostname			= EXIT_SUCCESS;
	__retval_find_host			= NULL;
	__retval_find_service			= NULL;
	__retval_find_command			= NULL;
	__retval_grab_host_macros_r		= EXIT_SUCCESS;
	__retval_grab_service_macros_r		= EXIT_SUCCESS;
	__output_get_raw_command_line_r		= NULL;
	__retval_get_raw_command_line_r		= EXIT_SUCCESS;
	__output_process_macros_r		= NULL;
	__retval_process_macros_r		= EXIT_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////


void BrokerXifiSrvTest::wrong_request_local_plugin_implicit_entity_type()
{
	string					request;
	host					check_host;
	service					check_service;
	command					check_command;
	nebstruct_service_check_data		check_data;

	// given
	check_service.host_name			= LOCALHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= NULL;			// no explicit entity type given
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/local/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;

	string expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << SOME_CHECK_NAME
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << LOCALHOST_ADDR << ':' << SOME_DESCRIPTION
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << SRV_DEFAULT_ENTITY_TYPE
		)).str();

	// when
	const char* actual_request = get_adapter_request(&check_data, request);

	// then
	CPPUNIT_ASSERT(actual_request != ADAPTER_REQUEST_INVALID);
	CPPUNIT_ASSERT(expected_request != actual_request);
}


void BrokerXifiSrvTest::get_request_ok_local_plugin_explicit_entity_type()
{
	string					request;
	host					check_host;
	service					check_service;
	command					check_command;
	customvariablesmember			check_vars;
	nebstruct_service_check_data		check_data;

	// given
	check_vars = {
		variable_name:			CUSTOM_VAR_ENTITY_TYPE,
		variable_value:			SRV_DEFAULT_ENTITY_TYPE
	};
	check_service.host_name			= LOCALHOST_ADDR;
	check_service.service_check_command	= SOME_CHECK_NAME;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= &check_vars;
	check_command.name			= SOME_CHECK_NAME;
	check_command.command_line		= "/usr/local/" SOME_CHECK_NAME " " SOME_CHECK_ARGS;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;

	string expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << SOME_CHECK_NAME
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << LOCALHOST_ADDR << ':' << SOME_DESCRIPTION
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << SRV_DEFAULT_ENTITY_TYPE
		)).str();

	// when
	const char* actual_request = get_adapter_request(&check_data, request);

	// then
	CPPUNIT_ASSERT(actual_request != ADAPTER_REQUEST_INVALID);
	CPPUNIT_ASSERT(expected_request == actual_request);
}



void BrokerXifiSrvTest::wrong_request_remote_plugin_implicit_entity_type()
{
	string					request;
	host					check_host;
	service					check_service;
	command					check_command;
	nebstruct_service_check_data		check_data;

	// given
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= NRPE_PLUGIN "!" SOME_CHECK_NAME;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= NULL;			// no explicit entity type given
	check_command.name			= NRPE_PLUGIN;
	check_command.command_line		= "/usr/local/" NRPE_PLUGIN " -H " REMOTEHOST_ADDR " -c arguments";
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;

	string expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << SOME_CHECK_NAME
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTEHOST_ADDR << ':' << SOME_DESCRIPTION
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << SRV_DEFAULT_ENTITY_TYPE
		)).str();

	// when
	const char* actual_request = get_adapter_request(&check_data, request);

	// then
	CPPUNIT_ASSERT(actual_request != ADAPTER_REQUEST_INVALID);
	CPPUNIT_ASSERT(expected_request != actual_request);
}


void BrokerXifiSrvTest::get_request_ok_remote_plugin_explicit_entity_type()
{
	string					request;
	host					check_host;
	service					check_service;
	command					check_command;
	customvariablesmember			check_vars;
	nebstruct_service_check_data		check_data;

	// given
	check_vars = {
		variable_name:			CUSTOM_VAR_ENTITY_TYPE,
		variable_value:			SRV_DEFAULT_ENTITY_TYPE
	};
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= NRPE_PLUGIN "!" SOME_CHECK_NAME;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= &check_vars;
	check_command.name			= NRPE_PLUGIN;
	check_command.command_line		= "/usr/local/" NRPE_PLUGIN " -H " REMOTEHOST_ADDR " -c arguments";
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	__output_get_raw_command_line_r		= check_command.command_line;
	__output_process_macros_r		= check_command.command_line;
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;

	string expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << SOME_CHECK_NAME
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTEHOST_ADDR << ':' << SOME_DESCRIPTION
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << SRV_DEFAULT_ENTITY_TYPE
		)).str();

	// when
	const char* actual_request = get_adapter_request(&check_data, request);

	// then
	CPPUNIT_ASSERT(actual_request != ADAPTER_REQUEST_INVALID);
	CPPUNIT_ASSERT(expected_request == actual_request);
}
