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
 * @file   suite_broker_common.cc
 * @brief  Test suite to verify event broker common features
 *
 * This file defines unit tests to verify features that are common to all
 * specific event broker implementations (see ngsi_event_broker_common.c).
 */


#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "suite_config.h"
#include "ngsi_event_broker_common.h"
#include "neberrors.h"
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


// Stubs for global module constants and variables
extern "C" {
	char* const module_name		= PACKAGE_NAME;
	char* const module_version	= PACKAGE_VERSION;
	void*       module_handle	= NULL;
}


// Stubs for module functions
extern "C" {
	int init_module_handle_info(void* handle, context_t* context)
	{
		return NEB_OK;
	}
	char* get_adapter_request(nebstruct_service_check_data* data, context_t* context)
	{
		return NULL;
	}
}


// Forward declarations for friend members
extern "C" {
	int				__wrap_gethostname(char*, size_t);
	struct hostent*			__wrap_gethostbyname(const char*);
	int				__wrap_neb_set_module_info(void*, int, char*);
	int				__wrap_neb_register_callback(int, void*, int, int (*)(int, void*));
	CURLcode			__wrap_curl_global_init(long);
	void				__wrap_curl_global_cleanup(void);
}


/// Broker common features test suite
class BrokerCommonTest: public TestFixture
{
	// mocks: return values and friend declaration to access static members
	static int			__retval_gethostname;
	friend int			::__wrap_gethostname(char*, size_t);
	static bool			__retval_gethostbyname_is_null;
	friend struct hostent*		::__wrap_gethostbyname(const char*);
	static int			__retval_neb_set_module_info;
	friend int			::__wrap_neb_set_module_info(void*, int, char*);
	static int			__retval_neb_register_callback;
	friend int			::__wrap_neb_register_callback(int, void*, int, int (*)(int, void*));
	static CURLcode			__retval_curl_global_init;
	friend CURLcode			::__wrap_curl_global_init(long);
	friend void			::__wrap_curl_global_cleanup(void);

	// static methods equivalent to external C functions
	static int			nebmodule_init(int, const string&, void*);
	static int			nebmodule_deinit(int, int);

	// tests
	void init_fails_wrong_nagios_object_version();
	void init_fails_with_unknown_args_option();
	void init_fails_with_missing_adapter_url_option();
	void init_fails_with_missing_adapter_url_value();
	void init_fails_with_missing_region_option();
	void init_fails_with_missing_region_value();
	void init_fails_when_cannot_get_hostname();
	void init_fails_when_cannot_resolve_host_address();
	void init_fails_when_curl_cannot_be_initialized();
	void init_fails_when_callback_cannot_be_registered();
	void init_ok_with_valid_mandatory_args();
	void init_ok_with_optional_logging_arg();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(BrokerCommonTest);
	CPPUNIT_TEST(init_fails_wrong_nagios_object_version);
	CPPUNIT_TEST(init_fails_with_unknown_args_option);
	CPPUNIT_TEST(init_fails_with_missing_adapter_url_option);
	CPPUNIT_TEST(init_fails_with_missing_adapter_url_value);
	CPPUNIT_TEST(init_fails_with_missing_region_option);
	CPPUNIT_TEST(init_fails_with_missing_region_value);
	CPPUNIT_TEST(init_fails_when_cannot_get_hostname);
	CPPUNIT_TEST(init_fails_when_cannot_resolve_host_address);
	CPPUNIT_TEST(init_fails_when_curl_cannot_be_initialized);
	CPPUNIT_TEST(init_fails_when_callback_cannot_be_registered);
	CPPUNIT_TEST(init_ok_with_valid_mandatory_args);
	CPPUNIT_TEST(init_ok_with_optional_logging_arg);
	CPPUNIT_TEST_SUITE_END();
};


/// Suite startup
int main(int argc, char* argv[])
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(BrokerCommonTest::suite());
	BrokerCommonTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	BrokerCommonTest::suiteTearDown();
	ofstream xmlFileOut((string(argv[0]) + "-cppunit-results.xml").c_str());
	XmlOutputter xmlOut(&runner.result(), xmlFileOut);
	xmlOut.write();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


/// Return value from ::__wrap_gethostname
int BrokerCommonTest::__retval_gethostname = EXIT_SUCCESS;


/// Mock for ::gethostname
int __wrap_gethostname(char* name, size_t len)
{
	memcpy(name, LOCALHOST_NAME, len);
	return BrokerCommonTest::__retval_gethostname;
}


/// Return value from ::__wrap_gethostbyname
bool BrokerCommonTest::__retval_gethostbyname_is_null = false;


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
	} else {
		return NULL;	// host not found
	}

	host.h_addr_list = (char**) list;
	inet_pton(AF_INET, hostaddr, host.h_addr_list[0]);
	return (BrokerCommonTest::__retval_gethostbyname_is_null) ? NULL : &host;
}


/// Return value from ::__wrap_neb_set_module_info
int BrokerCommonTest::__retval_neb_set_module_info = NEB_OK;


/// Mock for ::neb_set_module_info
int __wrap_neb_set_module_info(void* handle, int type, char* data)
{
	return BrokerCommonTest::__retval_neb_set_module_info;
}


/// Return value from ::__wrap_neb_register_callback
int BrokerCommonTest::__retval_neb_register_callback = NEB_OK;


/// Mock for ::neb_register_callback
int __wrap_neb_register_callback(int type, void* handle, int priority, int (*func)(int, void*))
{
	return BrokerCommonTest::__retval_neb_register_callback;
}


/// Return value from ::__wrap_curl_global_init
CURLcode BrokerCommonTest::__retval_curl_global_init = CURLE_OK;


/// Mock for ::curl_global_init
CURLcode __wrap_curl_global_init(long flags)
{
	return BrokerCommonTest::__retval_curl_global_init;
}


/// Mock for ::curl_global_cleanup
void __wrap_curl_global_cleanup(void)
{
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
int BrokerCommonTest::nebmodule_init(int flags, const string& args, void* handle)
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
int BrokerCommonTest::nebmodule_deinit(int flags, int reason)
{
	return ::nebmodule_deinit(flags, reason);
}


///
/// Suite setup
///
void BrokerCommonTest::suiteSetUp()
{
}


///
/// Suite teardown
///
void BrokerCommonTest::suiteTearDown()
{
}


///
/// Tests setup
///
void BrokerCommonTest::setUp()
{
	::adapter_url = NULL;
	::region_id   = NULL;
	::host_addr   = NULL;
}


///
/// Tests teardown
///
void BrokerCommonTest::tearDown()
{
	nebmodule_deinit(0, NEBMODULE_NEB_SHUTDOWN);
	__nagios_object_structure_version	= CURRENT_OBJECT_STRUCTURE_VERSION;
	__retval_gethostname			= EXIT_SUCCESS;
	__retval_gethostbyname_is_null		= false;
	__retval_neb_set_module_info		= NEB_OK;
	__retval_neb_register_callback		= NEB_OK;
	__retval_curl_global_init		= CURLE_OK;
}


///////////////////////////////////////////////////////////////


void BrokerCommonTest::init_fails_wrong_nagios_object_version()
{
	// given
	int	flags	= 0;
	string	argline	= "";
	__nagios_object_structure_version = 0;

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_unknown_args_option()
{
	// given
	int	flags	= 0;
	string	value	= "value",
		argline	= ((ostringstream&)(ostringstream().flush()
		<< "-Z" << value
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_missing_adapter_url_option()
{
	// given
	int	flags	= 0;
	string	region	= REGION_ID,
		argline	= ((ostringstream&)(ostringstream().flush()
		<< "-r" << region
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_missing_adapter_url_value()
{
	// given
	int	flags	= 0;
	string	region	= REGION_ID,
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << ""
		<< ' ' << "-r" << region
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_missing_region_option()
{
	// given
	int	flags	= 0;
	string	url	= ADAPTER_URL,
		argline	= ((ostringstream&)(ostringstream().flush()
		<< "-u" << url
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_missing_region_value()
{
	// given
	int	flags	= 0;
	string	url	= ADAPTER_URL,
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << ""
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_when_cannot_get_hostname()
{
	// given
	int	flags	= 0;
	string	url	= ADAPTER_URL,
		region	= REGION_ID,
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		)).str();
	__retval_gethostname = EXIT_FAILURE;

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_when_cannot_resolve_host_address()
{
	// given
	int	flags	= 0;
	string	url	= ADAPTER_URL,
		region	= REGION_ID,
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		)).str();
	__retval_gethostbyname_is_null = true;

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_when_curl_cannot_be_initialized()
{
	// given
	int	flags	= 0;
	string	url	= ADAPTER_URL,
		region	= REGION_ID,
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		)).str();
	__retval_curl_global_init = CURLE_FAILED_INIT;

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_when_callback_cannot_be_registered()
{
	// given
	int	flags	= 0;
	string	url	= ADAPTER_URL,
		region	= REGION_ID,
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		)).str();
	__retval_neb_register_callback = NEB_ERROR;

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_ok_with_valid_mandatory_args()
{
	// given
	int	flags	= 0;
	string	url	= ADAPTER_URL,
		region	= REGION_ID,
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(!init_error);
	CPPUNIT_ASSERT(url == ::adapter_url);
	CPPUNIT_ASSERT(region == ::region_id);
	CPPUNIT_ASSERT(::host_addr != NULL);
}


void BrokerCommonTest::init_ok_with_optional_logging_arg()
{
	// given
	int	flags	= 0,
		level	= LOG_DEBUG;
	string	url	= ADAPTER_URL,
		region	= REGION_ID,
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		<< ' ' << "-l" << loglevel_names[level]
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle) == NEB_ERROR;

	// then
	CPPUNIT_ASSERT(!init_error);
	CPPUNIT_ASSERT(url == ::adapter_url);
	CPPUNIT_ASSERT(region == ::region_id);
	CPPUNIT_ASSERT(::log_level == level);
}
