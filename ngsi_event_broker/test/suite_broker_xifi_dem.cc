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
 * @file   suite_broker_xifi_dem.cc
 * @brief  Test suite to verify DEM features from XIFI-specific event broker
 *
 * This file defines unit tests to verify DEM features from the XIFI-specific
 * event broker implementation (see ngsi_event_broker_xifi.c).
 */


#include <string>
#include <sstream>
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
#include "cppunit/BriefTestProgressListener.h"
#include "cppunit/extensions/HelperMacros.h"


using CppUnit::TestResult;
using CppUnit::TestFixture;
using CppUnit::TextTestRunner;
using CppUnit::BriefTestProgressListener;
using namespace std;


/// Any `nebstruct_service_check_data` (ignored)
#define CHECK_DATA			NULL


/// Any custom entity type
#define SOME_ENTITY_TYPE		"some"


/// Any remote address
#define REMOTEHOST_ADDR			"169.254.0.1"


/// Any adapter URL
#define ADAPTER_URL			"http://localhost:5000"


/// Any region id
#define REGION_ID			"myregion"


/// Any timeout
#define TIMEOUT				1000


/// XIFI Broker (DEM features) test suite
class BrokerXifiDemTest: public TestFixture
{
	static string			localhost_addr;
	static string			plugin_name;
	static string			plugin_args;
	static bool			plugin_nrpe;
	static service			plugin_serv;
	static customvariablesmember	custom_vars;

	// C function wrappers
	static bool init_module_variables(const string& args);
	static bool free_module_variables();
	static bool get_adapter_request(nebstruct_service_check_data* data, string& request);

	// mock for function ::find_plugin_command_name()
	friend char* find_plugin_command_name(nebstruct_service_check_data* data, char** args, int* nrpe, const service** serv);

	// tests
	void wrong_request_remote_physical_host_implicit_type();
	void get_request_ok_remote_physical_host_explicit_type();
	void get_request_ok_remote_entity_explicit_type();
	void get_request_ok_local_physical_host_implicit_type();
	void get_request_ok_local_physical_host_explicit_type();
	void get_request_ok_local_entity_explicit_type();
	void get_request_ok_remote_vm_implicit_type();
	void get_request_ok_remote_vm_explicit_type();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(BrokerXifiDemTest);
	CPPUNIT_TEST(wrong_request_remote_physical_host_implicit_type);
	CPPUNIT_TEST(get_request_ok_remote_physical_host_explicit_type);
	CPPUNIT_TEST(get_request_ok_remote_entity_explicit_type);
	CPPUNIT_TEST(get_request_ok_local_physical_host_implicit_type);
	CPPUNIT_TEST(get_request_ok_local_physical_host_explicit_type);
	CPPUNIT_TEST(get_request_ok_local_entity_explicit_type);
	CPPUNIT_TEST(get_request_ok_remote_vm_implicit_type);
	CPPUNIT_TEST(get_request_ok_remote_vm_explicit_type);
	CPPUNIT_TEST_SUITE_END();
};


/// Suite startup
int main(void)
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(BrokerXifiDemTest::suite());
	BrokerXifiDemTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	BrokerXifiDemTest::suiteTearDown();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


string			BrokerXifiDemTest::localhost_addr;
string			BrokerXifiDemTest::plugin_name;
string			BrokerXifiDemTest::plugin_args;
bool			BrokerXifiDemTest::plugin_nrpe;
service			BrokerXifiDemTest::plugin_serv;
customvariablesmember	BrokerXifiDemTest::custom_vars;


///
/// Mock for function ::find_plugin_command_name()
/// @memberof BrokerXifiDemTest
///
char* find_plugin_command_name(nebstruct_service_check_data* data, char** args, int* nrpe, const service** serv)
{
	char* name = NULL;

	name  = strdup(BrokerXifiDemTest::plugin_name.c_str());
	*args = strdup(BrokerXifiDemTest::plugin_args.c_str());
	*nrpe = (int) BrokerXifiDemTest::plugin_nrpe;
	*serv = (const service*) &BrokerXifiDemTest::plugin_serv;

	return name;
}


///
/// C++ wrapper for function ::init_module_variables()
///
/// @param[in] args	The module arguments as a space-separated string.
/// @return		Successful initialization.
///
bool BrokerXifiDemTest::init_module_variables(const string& args)
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
bool BrokerXifiDemTest::free_module_variables()
{
	return (bool) ::free_module_variables();
}


///
/// C++ wrapper for function ::get_adapter_request()
///
/// @param[in]  data	The plugin data passed by Nagios to the registered callback_service_check().
/// @param[out] request	The request URL to invoke NGSI Adapter (including query string).
/// @return		Request successfully generated.
///
bool BrokerXifiDemTest::get_adapter_request(nebstruct_service_check_data* data, string& request)
{
	char* result = ::get_adapter_request(data);
	bool  error = (result == NULL);
	request.assign((error) ? "" : result);
	::free(result);
	result = NULL;
	return error;
}


///
/// Suite setup
///
void BrokerXifiDemTest::suiteSetUp()
{
	char name[HOST_NAME_MAX];
	char addr[INET_ADDRSTRLEN];

	if (gethostname(name, HOST_NAME_MAX) || resolve_address(name, addr, INET_ADDRSTRLEN)) {
		localhost_addr.assign("127.0.0.1");
	} else {
		localhost_addr.assign(addr);
	}

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
void BrokerXifiDemTest::suiteTearDown()
{
	free_module_variables();
}


///
/// Tests setup
///
void BrokerXifiDemTest::setUp()
{
}


///
/// Tests teardown
///
void BrokerXifiDemTest::tearDown()
{
}


//////////////////////////////////


void BrokerXifiDemTest::wrong_request_remote_physical_host_implicit_type()
{
	string expected_request, actual_request;

	// given
	plugin_serv.custom_variables = NULL;	// implicit entity_type
	plugin_nrpe = true;			// remote monitored host
	plugin_name = "check_load";
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTEHOST_ADDR
		<< ' ' << "-c" << plugin_name
		<< ' ' << "-t" << TIMEOUT
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTEHOST_ADDR
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << DEM_ENTITY_TYPE_PHYSICAL
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);
	bool wrong = (expected_request != actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT(wrong);
}


void BrokerXifiDemTest::get_request_ok_remote_physical_host_explicit_type()
{
	string expected_request, actual_request;

	// given
	custom_vars.variable_name	= CUSTOM_VAR_ENTITY_TYPE;
	custom_vars.variable_value	= DEM_ENTITY_TYPE_PHYSICAL;
	plugin_serv.custom_variables	= &custom_vars;
	plugin_nrpe = true;
	plugin_name = "check_load";
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTEHOST_ADDR
		<< ' ' << "-c" << plugin_name
		<< ' ' << "-t" << TIMEOUT
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTEHOST_ADDR
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << DEM_ENTITY_TYPE_PHYSICAL
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}


void BrokerXifiDemTest::get_request_ok_remote_entity_explicit_type()
{
	string expected_request, actual_request;

	// given
	custom_vars.variable_name	= CUSTOM_VAR_ENTITY_TYPE;
	custom_vars.variable_value	= SOME_ENTITY_TYPE;
	plugin_serv.custom_variables	= &custom_vars;
	plugin_nrpe = true;
	plugin_name = "check_load";
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTEHOST_ADDR
		<< ' ' << "-c" << plugin_name
		<< ' ' << "-t" << TIMEOUT
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTEHOST_ADDR
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << SOME_ENTITY_TYPE
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}


void BrokerXifiDemTest::get_request_ok_local_physical_host_implicit_type()
{
	string expected_request, actual_request;

	// given
	plugin_serv.custom_variables = NULL;
	plugin_nrpe = false;
	plugin_name = "check_load";
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-w" << 1 << ',' << 5 << ',' << 15
		<< ' ' << "-c" << 1 << ',' << 5 << ',' << 15
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << localhost_addr
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << DEM_ENTITY_TYPE_PHYSICAL
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}


void BrokerXifiDemTest::get_request_ok_local_physical_host_explicit_type()
{
	string expected_request, actual_request;

	// given
	custom_vars.variable_name	= CUSTOM_VAR_ENTITY_TYPE;
	custom_vars.variable_value	= DEM_ENTITY_TYPE_PHYSICAL;
	plugin_serv.custom_variables	= &custom_vars;
	plugin_nrpe = false;
	plugin_name = "check_load";
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-w" << 1 << ',' << 5 << ',' << 15
		<< ' ' << "-c" << 1 << ',' << 5 << ',' << 15
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << localhost_addr
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << DEM_ENTITY_TYPE_PHYSICAL
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}


void BrokerXifiDemTest::get_request_ok_local_entity_explicit_type()
{
	string expected_request, actual_request;

	// given
	custom_vars.variable_name	= CUSTOM_VAR_ENTITY_TYPE;
	custom_vars.variable_value	= SOME_ENTITY_TYPE;
	plugin_serv.custom_variables	= &custom_vars;
	plugin_nrpe = false;
	plugin_name = "check_load";
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-w" << 1 << ',' << 5 << ',' << 15
		<< ' ' << "-c" << 1 << ',' << 5 << ',' << 15
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << localhost_addr
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << SOME_ENTITY_TYPE
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}


void BrokerXifiDemTest::get_request_ok_remote_vm_implicit_type()
{
	string expected_request, actual_request;

	// given
	plugin_serv.custom_variables = NULL;
	plugin_nrpe = true;
	plugin_name = "check_load";
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTEHOST_ADDR
		<< ' ' << "-c" << plugin_name
		<< ' ' << "-t" << TIMEOUT
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTEHOST_ADDR
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << DEM_ENTITY_TYPE_VIRTUAL
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}


void BrokerXifiDemTest::get_request_ok_remote_vm_explicit_type()
{
	string expected_request, actual_request;

	// given
	custom_vars.variable_name	= CUSTOM_VAR_ENTITY_TYPE;
	custom_vars.variable_value	= DEM_ENTITY_TYPE_VIRTUAL;
	plugin_serv.custom_variables	= &custom_vars;
	plugin_nrpe = true;
	plugin_name = "check_load";
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTEHOST_ADDR
		<< ' ' << "-c" << plugin_name
		<< ' ' << "-t" << TIMEOUT
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTEHOST_ADDR
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << DEM_ENTITY_TYPE_VIRTUAL
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}
