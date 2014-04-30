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
#include <cstdlib>
#include "config.h"
#include "ngsi_event_broker_common.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestFixture.h"
#include "cppunit/TextTestRunner.h"
#include "cppunit/BriefTestProgressListener.h"
#include "cppunit/extensions/HelperMacros.h"


using CppUnit::TestResult;
using CppUnit::TestFixture;
using CppUnit::TextTestRunner;
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
	int init_module_handle_info(void* handle)
	{
		return 0;
	}
	char* get_adapter_request(nebstruct_service_check_data* data)
	{
		return NULL;
	}
}

/// @}


/// Broker common features test suite
class BrokerCommonTest: public TestFixture
{
	// C function wrappers
	static bool init_module_variables(const string& args);
	static bool free_module_variables();

	// tests
	void init_fails_with_missing_adapter_url();
	void init_fails_with_missing_region();
	void init_ok_with_valid_args();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(BrokerCommonTest);
	CPPUNIT_TEST(init_fails_with_missing_adapter_url);
	CPPUNIT_TEST(init_fails_with_missing_region);
	CPPUNIT_TEST(init_ok_with_valid_args);
	CPPUNIT_TEST_SUITE_END();
};


/// Suite startup
int main(void)
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(BrokerCommonTest::suite());
	BrokerCommonTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	BrokerCommonTest::suiteTearDown();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


///
/// C++ wrapper for function ::init_module_variables()
///
/// @param[in] args	The module arguments as a space-separated string.
/// @return		Successful initialization.
///
bool BrokerCommonTest::init_module_variables(const string& args)
{
	char buffer[MAXBUFLEN];
	buffer[args.copy(buffer, MAXBUFLEN-1)] = '\0';
	return (bool) ::init_module_variables(buffer);
}


///
/// C++ wrapper for function ::free_module_variables()
///
/// @return		Successful resources release.
///
bool BrokerCommonTest::free_module_variables()
{
	return (bool) ::free_module_variables();
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
	free_module_variables();
}


/////////////////////////////////


void BrokerCommonTest::init_fails_with_missing_adapter_url()
{
	// given
	string	region	= "region";
	string	argline	= ((ostringstream&)(ostringstream().flush()
		<< "-r" << region
		)).str();

	// when
	bool init_error = init_module_variables(argline);

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_fails_with_missing_region()
{
	// given
	string	url	= "url";
	string	argline	= ((ostringstream&)(ostringstream().flush()
		<< "-u" << url
		)).str();

	// when
	bool init_error = init_module_variables(argline);

	// then
	CPPUNIT_ASSERT(init_error);
}


void BrokerCommonTest::init_ok_with_valid_args()
{
	// given
	string	url	= "url",
		region	= "region",
		argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << url
		<< ' ' << "-r" << region
		)).str();

	// when
	bool init_error = init_module_variables(argline);

	// then
	CPPUNIT_ASSERT(!init_error);
	CPPUNIT_ASSERT(url == ::adapter_url);
	CPPUNIT_ASSERT(region == ::region_id);
	CPPUNIT_ASSERT(::host_addr != NULL);
}
