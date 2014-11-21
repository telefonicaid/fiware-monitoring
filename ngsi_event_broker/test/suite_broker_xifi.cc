/*
 * Copyright 2013 Telefónica I+D
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
 * @file   suite_broker_xifi.cc
 * @brief  Test suite to verify XIFI-specific event broker
 *
 * This file defines unit tests to verify XIFI-specific event broker
 * implementation (see ngsi_event_broker_xifi.c).
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
#include "config.h"
#include "ngsi_event_broker_common.h"
#include "ngsi_event_broker_xifi.h"
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


/// Some check name
#define SOME_CHECK_NAME		"some_check"


/// Some check output data
#define SOME_CHECK_OUTPUT_DATA	"some_data"


/// Some check output data
#define SOME_CHECK_PERF_DATA	"some_perf_data"


/// Some service description
#define SOME_DESCRIPTION	"some_description"


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


/// Fake cURL handle
#define CURL_HANDLE		((void*) 1)


/// Fake NEB module handle
#define MODULE_HANDLE		((void*) 2)


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
	int			__wrap_neb_set_module_info(void*, int, char*);
	int			__wrap_neb_register_callback(int, void*, int, int (*)(int, void*));
	host*			__wrap_find_host(char*);
	service*		__wrap_find_service(char*, char*);
	command*		__wrap_find_command(char*);
}

/// @}


///
/// @name Mocks for cURL functions
/// @{
///
extern "C" {
	CURLcode		__wrap_curl_global_init(long);
	void			__wrap_curl_global_cleanup(void);
	CURL*			__wrap_curl_easy_init(void);
	CURLcode		__wrap_curl_easy_setopt(CURL*, CURLoption, ...);
	CURLcode		__wrap_curl_easy_perform(CURL*);
	void			__wrap_curl_easy_cleanup(CURL*);
	const char*		__wrap_curl_easy_strerror(CURLcode);
}

/// @}


/// XIFI Broker test suite
class BrokerXifiTest: public TestFixture
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
	static bool		nebmodule_init(int, const string&, void*);
	static bool		nebmodule_deinit(int, int);

	// tests
	void callback_skips_request_if_cannot_find_host();
	void callback_skips_request_if_cannot_find_service();
	void callback_skips_request_if_cannot_find_command();
	void callback_skips_request_if_cannot_initialize_curl();
	void callback_skips_request_if_curl_perform_fails();
	void callback_sends_request_if_curl_perform_succeedes();
	void callback_sends_request_with_txid_and_content_type_headers();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(BrokerXifiTest);
	CPPUNIT_TEST(callback_skips_request_if_cannot_find_host);
	CPPUNIT_TEST(callback_skips_request_if_cannot_find_service);
	CPPUNIT_TEST(callback_skips_request_if_cannot_find_command);
	CPPUNIT_TEST(callback_skips_request_if_cannot_initialize_curl);
	CPPUNIT_TEST(callback_skips_request_if_curl_perform_fails);
	CPPUNIT_TEST(callback_sends_request_if_curl_perform_succeedes);
	CPPUNIT_TEST(callback_sends_request_with_txid_and_content_type_headers);
	CPPUNIT_TEST_SUITE_END();
};


/// Suite startup
int main(int argc, char* argv[])
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(BrokerXifiTest::suite());
	BrokerXifiTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	BrokerXifiTest::suiteTearDown();
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
int BrokerXifiTest::__retval_gethostname = EXIT_SUCCESS;

/// Mock function
int __wrap_gethostname(char* name, size_t len)
{
	memcpy(name, LOCALHOST_NAME, len);
	return BrokerXifiTest::__retval_gethostname;
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
/// @name Mock for neb_set_module_info()
/// @{
///

/// Return value
int BrokerXifiTest::__retval_neb_set_module_info = NEB_OK;

/// Mock function
int __wrap_neb_set_module_info(void* handle, int type, char* data)
{
	return BrokerXifiTest::__retval_neb_set_module_info;
}

/// @}


///
/// @name Mock for neb_register_callback()
/// @{
///

/// Return value
int BrokerXifiTest::__retval_neb_register_callback = NEB_OK;

/// Mock function
int __wrap_neb_register_callback(int type, void* handle, int priority, int (*func)(int, void*))
{
	return BrokerXifiTest::__retval_neb_register_callback;
}

/// @}


///
/// @name Mock for find_host()
/// @{
///

/// Return value
host* BrokerXifiTest::__retval_find_host = NULL;

/// Mock function
host* __wrap_find_host(char* name)
{
	return BrokerXifiTest::__retval_find_host;
}

/// @}


///
/// @name Mock for find_service()
/// @{
///

/// Return value
service* BrokerXifiTest::__retval_find_service = NULL;

/// Mock function
service* __wrap_find_service(char* name, char* svc_desc)
{
	return BrokerXifiTest::__retval_find_service;
}

/// @}


///
/// @name Mock for find_command()
/// @{
///

/// Return value
command* BrokerXifiTest::__retval_find_command = NULL;

/// Mock function
command* __wrap_find_command(char* name)
{
	return BrokerXifiTest::__retval_find_command;
}

/// @}


///
/// @name Mock for curl_global_init()
/// @{
///

/// Return value
CURLcode BrokerXifiTest::__retval_curl_global_init = CURLE_OK;

/// Mock function
CURLcode __wrap_curl_global_init(long flags)
{
	return BrokerXifiTest::__retval_curl_global_init;
}

/// @}


///
/// @name Mock for curl_global_cleanup()
/// @{
///

/// Mock function
void __wrap_curl_global_cleanup(void)
{
}

/// @}


///
/// @name Mock for curl_easy_init()
/// @{
///

/// Return value
CURL* BrokerXifiTest::__retval_curl_easy_init = NULL;

/// Mock function
CURL* __wrap_curl_easy_init(void)
{
	return BrokerXifiTest::__retval_curl_easy_init;
}

/// @}


///
/// @name Mock for curl_easy_setopt()
/// @{
///

/// True if required HTTP headers (`Content-Type` and TXID_HTTP_HEADER) are set
bool BrokerXifiTest::__header_curl_easy_setopt = false;

/// Return value
CURLcode BrokerXifiTest::__retval_curl_easy_setopt = CURLE_OK;

/// Mock function
CURLcode __wrap_curl_easy_setopt(CURL* handle, CURLoption option, ...)
{
	if ((BrokerXifiTest::__retval_curl_easy_setopt == CURLE_OK) && (option == CURLOPT_HTTPHEADER)) {
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

		BrokerXifiTest::__header_curl_easy_setopt = (has_txid && has_type);
	}

	return BrokerXifiTest::__retval_curl_easy_setopt;
}

/// @}


///
/// @name Mock for curl_easy_perform()
/// @{
///

/// Hit counter
size_t BrokerXifiTest::__hitcnt_curl_easy_perform = 0;

/// Return value
CURLcode BrokerXifiTest::__retval_curl_easy_perform = CURLE_OK;

/// Mock function
CURLcode __wrap_curl_easy_perform(CURL* handle)
{
	if (BrokerXifiTest::__retval_curl_easy_perform == CURLE_OK) {
printf("\n*entro ++\n");
		++BrokerXifiTest::__hitcnt_curl_easy_perform;
	}
printf("\n*easy_perform=%d*\n", (int) BrokerXifiTest::__retval_curl_easy_perform);
	return BrokerXifiTest::__retval_curl_easy_perform;
}

/// @}


///
/// @name Mock for curl_easy_cleanup()
/// @{
///

/// Mock function
void __wrap_curl_easy_cleanup(CURL* handle)
{
}

/// @}


///
/// @name Mock for curl_easy_strerror()
/// @{
///

/// Return value
const char* BrokerXifiTest::__retval_curl_easy_strerror = NULL;

/// Mock function
const char* __wrap_curl_easy_strerror(CURLcode errornum)
{
	return BrokerXifiTest::__retval_curl_easy_strerror;
}

/// @}


///
/// Static method for C function ::nebmodule_init()
///
/// @param[in] flags	The initialization flags (ignored).
/// @param[in] args	The module arguments as a space-separated string.
/// @param[in] handle	The module handle passed by Nagios Core server.
///
/// @retval NEB_OK	Successfully initialized.
/// @retval NEB_ERROR	Not successfully initialized.
///
bool BrokerXifiTest::nebmodule_init(int flags, const string& args, void* handle)
{
	char buffer[MAXBUFLEN];
	buffer[args.copy(buffer, MAXBUFLEN-1)] = '\0';
	return (bool) ::nebmodule_init(flags, buffer, handle);
}


///
/// Static method for C function ::nebmodule_deinit()
///
/// @param[in] flags	The deinitialization flags (ignored).
/// @param[in] reason	The reason why this module is being deinitialized.
///
/// @retval NEB_OK	Successfully deinitialized.
///
bool BrokerXifiTest::nebmodule_deinit(int flags, int reason)
{
	return (bool) ::nebmodule_deinit(flags, reason);
}


///
/// Suite setup
///
void BrokerXifiTest::suiteSetUp()
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
void BrokerXifiTest::suiteTearDown()
{
	nebmodule_deinit(0, NEBMODULE_NEB_SHUTDOWN);
}


///
/// Tests setup
///
void BrokerXifiTest::setUp()
{
}


///
/// Tests teardown
///
void BrokerXifiTest::tearDown()
{
	__retval_gethostname			= EXIT_SUCCESS;
	__retval_neb_set_module_info		= NEB_OK;
	__retval_neb_register_callback		= NEB_OK;
	__retval_find_host			= NULL;
	__retval_find_service			= NULL;
	__retval_find_command			= NULL;
	__retval_curl_global_init		= CURLE_OK;
	__retval_curl_easy_init			= NULL;
	__retval_curl_easy_setopt		= CURLE_OK;
	__retval_curl_easy_perform		= CURLE_OK;
	__retval_curl_easy_strerror		= NULL;
	__header_curl_easy_setopt		= false;
	__hitcnt_curl_easy_perform		= 0;
}


/////////////////////////////////////////////////////////////////


void BrokerXifiTest::callback_skips_request_if_cannot_find_host()
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


void BrokerXifiTest::callback_skips_request_if_cannot_find_service()
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


void BrokerXifiTest::callback_skips_request_if_cannot_find_command()
{
	host					check_host;
	service					check_service;
	nebstruct_service_check_data		check_data;

	// given
	__retval_find_command			= NULL;	// no valid command struct
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= NRPE_PLUGIN "!" SOME_CHECK_NAME;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= NULL;
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


void BrokerXifiTest::callback_skips_request_if_cannot_initialize_curl()
{
	host					check_host;
	service					check_service;
	command					check_command;
	nebstruct_service_check_data		check_data;

	// given
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	__retval_curl_easy_init			= NULL;				// initialization fails
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= NRPE_PLUGIN "!" SOME_CHECK_NAME;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= NULL;
	check_command.name			= NRPE_PLUGIN;
	check_command.command_line		= "/usr/local/" NRPE_PLUGIN " -H " REMOTEHOST_ADDR;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerXifiTest::callback_skips_request_if_curl_perform_fails()
{
	host					check_host;
	service					check_service;
	command					check_command;
	nebstruct_service_check_data		check_data;

	// given
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	__retval_curl_easy_init			= CURL_HANDLE;
	__retval_curl_easy_perform		= CURLE_COULDNT_CONNECT;	// easy_perform() fails
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= NRPE_PLUGIN "!" SOME_CHECK_NAME;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= NULL;
	check_command.name			= NRPE_PLUGIN;
	check_command.command_line		= "/usr/local/" NRPE_PLUGIN " -H " REMOTEHOST_ADDR;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 0;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerXifiTest::callback_sends_request_if_curl_perform_succeedes()
{
	host					check_host;
	service					check_service;
	command					check_command;
	nebstruct_service_check_data		check_data;

	// given
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	__retval_curl_easy_init			= CURL_HANDLE;
	__retval_curl_easy_perform		= CURLE_OK;
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= NRPE_PLUGIN "!" SOME_CHECK_NAME;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= NULL;
	check_command.name			= NRPE_PLUGIN;
	check_command.command_line		= "/usr/local/" NRPE_PLUGIN " -H " REMOTEHOST_ADDR;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 1;	// easy_perform() is invoked and succeedes

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
}


void BrokerXifiTest::callback_sends_request_with_txid_and_content_type_headers()
{
	host					check_host;
	service					check_service;
	command					check_command;
	nebstruct_service_check_data		check_data;

	// given
	__retval_find_command			= &check_command;
	__retval_find_service			= &check_service;
	__retval_find_host			= &check_host;
	__retval_curl_easy_init			= CURL_HANDLE;
	__retval_curl_easy_perform		= CURLE_OK;
	__header_curl_easy_setopt		= false;
	check_service.host_name			= REMOTEHOST_ADDR;
	check_service.service_check_command	= NRPE_PLUGIN "!" SOME_CHECK_NAME;
	check_service.description		= SOME_DESCRIPTION;
	check_service.custom_variables		= NULL;
	check_command.name			= NRPE_PLUGIN;
	check_command.command_line		= "/usr/local/" NRPE_PLUGIN " -H " REMOTEHOST_ADDR;
	check_data.host_name			= check_service.host_name;
	check_data.service_description		= check_service.description;
	check_data.output			= SOME_CHECK_OUTPUT_DATA;
	check_data.perf_data			= SOME_CHECK_PERF_DATA;
	check_data.type				= NEBTYPE_SERVICECHECK_PROCESSED;
	int expected_retval			= NEB_OK;
	size_t expected_curl_perform_hitcnt	= 1;

	// when
	int actual_retval = ::callback_service_check(NEBCALLBACK_SERVICE_CHECK_DATA, &check_data);

	// then
	CPPUNIT_ASSERT(expected_retval == actual_retval);
	CPPUNIT_ASSERT(expected_curl_perform_hitcnt == __hitcnt_curl_easy_perform);
	CPPUNIT_ASSERT_EQUAL(BrokerXifiTest::__header_curl_easy_setopt, true);
}
