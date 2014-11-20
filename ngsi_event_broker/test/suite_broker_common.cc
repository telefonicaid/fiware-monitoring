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
 * @file   suite_broker_common.cc
 * @brief  Test suite to verify event broker common features
 *
 * This file defines unit tests to verify features that are common to all
 * specific event broker implementations (see ngsi_event_broker_common.c).
 */


#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include "config.h"
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


///
/// @name Stubs for global module constants and variables
/// @{
///
extern "C" {
	char* const module_name		= PACKAGE_NAME;
	char* const module_version	= PACKAGE_VERSION;
	void*       module_handle	= NULL;
}

/// @}


///
/// @name Stubs for module functions
/// @{
///
extern "C" {
	int init_module_handle_info(void* handle, context_t* context)
	{
		return 0;
	}
	char* get_adapter_request(nebstruct_service_check_data* data, context_t* context)
	{
		return NULL;
	}
}

/// @}


///
/// @name Mocks for cURL functions
/// @{
///
extern "C" {
	CURLcode curl_global_init_result = CURLE_OK;
	CURLcode curl_global_init(long flags)
	{
		return curl_global_init_result;
	}

	void curl_global_cleanup(void)
	{
	}

	CURL* curl_easy_init_result = NULL;
	CURL* curl_easy_init(void)
	{
		return curl_easy_init_result;
	}

	CURLcode curl_easy_setopt_result = CURLE_OK;
	CURLcode curl_easy_setopt(CURL* curl, CURLoption option, ...)
	{
		return curl_easy_setopt_result;
	}

	CURLcode curl_easy_perform_result = CURLE_OK;
	CURLcode curl_easy_perform(CURL*curl)
	{
		return curl_easy_perform_result;
	}

	void curl_easy_cleanup(CURL* curl)
	{
	}

	const char* curl_easy_strerror_result = NULL;
	const char* curl_easy_strerror(CURLcode)
	{
		return curl_easy_strerror_result;
	}

	struct curl_slist* curl_slist_append_result = NULL;
	struct curl_slist* curl_slist_append(struct curl_slist* list, const char* string)
	{
		return curl_slist_append_result;
	}

	void curl_slist_free_all(struct curl_slist* list)
	{
	}
}

/// @}


///
/// @name Mocks for Nagios functions
/// @{
///
extern "C" {
	int neb_register_callback_result = NEB_OK;
	int neb_register_callback(int callback_type, void* handle, int priority, int (*callback_func)(int, void*))
	{
		return neb_register_callback_result;
	}
}

/// @}


/// Broker common features test suite
class BrokerCommonTest: public TestFixture
{
	// C function wrappers
	static bool nebmodule_init(int flags, const string& args, void* handle);
	static bool nebmodule_deinit(int flags, int reason);

	// tests
	void init_fails_with_unknown_args_option();
	void init_fails_with_missing_adapter_url_option();
	void init_fails_with_missing_adapter_url_value();
	void init_fails_with_missing_region_option();
	void init_fails_with_missing_region_value();
	void init_fails_when_curl_cannot_be_initialized();
	void init_fails_when_callback_cannot_be_registered();
	void init_ok_with_valid_args();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(BrokerCommonTest);
	CPPUNIT_TEST(init_fails_with_unknown_args_option);
	CPPUNIT_TEST(init_fails_with_missing_adapter_url_option);
	CPPUNIT_TEST(init_fails_with_missing_adapter_url_value);
	CPPUNIT_TEST(init_fails_with_missing_region_option);
	CPPUNIT_TEST(init_fails_with_missing_region_value);
	CPPUNIT_TEST(init_fails_when_curl_cannot_be_initialized);
	CPPUNIT_TEST(init_fails_when_callback_cannot_be_registered);
	CPPUNIT_TEST(init_ok_with_valid_args);
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


///
/// C++ wrapper for function ::nebmodule_init()
///
/// @param[in] flags	The initialization flags (ignored).
/// @param[in] args	The module arguments as a space-separated string.
/// @param[in] handle	The module handle passed by Nagios Core server.
///
/// @retval NEB_OK	Successfully initialized.
/// @retval NEB_ERROR	Not successfully initialized.
///
bool BrokerCommonTest::nebmodule_init(int flags, const string& args, void* handle)
{
	char buffer[MAXBUFLEN];
	buffer[args.copy(buffer, MAXBUFLEN-1)] = '\0';
	return (bool) ::nebmodule_init(flags, buffer, handle);
}


///
/// C++ wrapper for function ::nebmodule_deinit()
///
/// @param[in] flags	The deinitialization flags (ignored).
/// @param[in] reason	The reason why this module is being deinitialized.
///
/// @retval NEB_OK	Successfully deinitialized.
///
bool BrokerCommonTest::nebmodule_deinit(int flags, int reason)
{
	return (bool) ::nebmodule_deinit(flags, reason);
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
	neb_register_callback_result	= NEB_OK;
	curl_global_init_result		= CURLE_OK;
	curl_easy_setopt_result		= CURLE_OK;
	curl_easy_perform_result	= CURLE_OK;
	curl_easy_init_result		= NULL;
	curl_easy_strerror_result	= NULL;
	curl_slist_append_result	= NULL;
}


/////////////////////////////////


void BrokerCommonTest::init_fails_with_unknown_args_option()
{
	// given
	int	flags	= 0;
	string	value	= "value",
		argline	= ((ostringstream&)(ostringstream().flush()
		<< "-Z" << value
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle);

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_missing_adapter_url_option()
{
	// given
	int	flags	= 0;
	string	region	= "region",
		argline	= ((ostringstream&)(ostringstream().flush()
		<< "-r" << region
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle);

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_missing_adapter_url_value()
{
	// given
	int	flags	= 0;
	string	region	= "region",
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << ""
		<< ' ' << "-r" << region
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle);

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_missing_region_option()
{
	// given
	int	flags	= 0;
	string	url	= "url",
		argline	= ((ostringstream&)(ostringstream().flush()
		<< "-u" << url
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle);

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_missing_region_value()
{
	// given
	int	flags	= 0;
	string	url	= "url",
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << ""
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle);

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_when_curl_cannot_be_initialized()
{
	// given
	int	flags	= 0;
	string	url	= "url",
		region	= "region",
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		)).str();
	curl_global_init_result = CURLE_FAILED_INIT;

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle);

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_when_callback_cannot_be_registered()
{
	// given
	int	flags	= 0;
	string	url	= "url",
		region	= "region",
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		)).str();
	neb_register_callback_result = NEB_ERROR;

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle);

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_ok_with_valid_args()
{
	// given
	int	flags	= 0;
	string	url	= "url",
		region	= "region",
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		)).str();

	// when
	bool init_error = nebmodule_init(flags, argline, module_handle);

	// then
	CPPUNIT_ASSERT(!init_error);
	CPPUNIT_ASSERT(url == ::adapter_url);
	CPPUNIT_ASSERT(region == ::region_id);
	CPPUNIT_ASSERT(::host_addr != NULL);
}
