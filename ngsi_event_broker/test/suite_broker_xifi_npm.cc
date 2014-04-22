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


// macro stringification
#define _STR(s)				#s
#define STR(s)				_STR(s)


// any nebstruct_service_check_data (ignored)
#define CHECK_DATA			NULL


// any custom entity type
#define SOME_ENTITY_TYPE		"some"


// any remote address and port
#define REMOTE_ADDR			"169.254.0.1"
#define PORT				((PORT_PLUS_1) - 1)
#define PORT_PLUS_1			21


// any adapter URL
#define ADAPTER_URL			"http://localhost:5000"


// any region id
#define REGION_ID			"myregion"


// any timeout
#define TIMEOUT				1000


// any SNMP community
#define SNMP_COMMUNITY			"community"


// any SNMP miblist
#define SNMP_MIBLIST			"miblist"


// any SNMP OID
#define SNMP_OID			".1.3.6.1.2.1.2.2.1.8." STR(PORT_PLUS_1)


// test suite
class BrokerXifiNpmTest: public TestFixture
{
	static string			plugin_name;
	static string			plugin_args;
	static bool			plugin_nrpe;
	static service			plugin_serv;
	static customvariablesmember	custom_vars;

	// C external function wrappers
	static bool init_module_variables(const string& args);
	static bool free_module_variables();
	static bool get_adapter_request(nebstruct_service_check_data* data, string& request);

	// mock for find_plugin_command_name()
	friend char* find_plugin_command_name(nebstruct_service_check_data* data, char** args, int* nrpe, const service** serv);

	// tests
	void get_request_ok_local_snmp_plugin_implicit_type();
	void get_request_ok_local_snmp_plugin_explicit_type();
	void wrong_request_local_snmp_plugin_custom_type();
	void error_getting_request_remote_snmp_plugin_implicit_type();
	void error_getting_request_remote_snmp_plugin_explicit_type();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(BrokerXifiNpmTest);
	CPPUNIT_TEST(get_request_ok_local_snmp_plugin_implicit_type);
	CPPUNIT_TEST(get_request_ok_local_snmp_plugin_explicit_type);
	CPPUNIT_TEST(wrong_request_local_snmp_plugin_custom_type);
	CPPUNIT_TEST(error_getting_request_remote_snmp_plugin_implicit_type);
	CPPUNIT_TEST(error_getting_request_remote_snmp_plugin_explicit_type);
	CPPUNIT_TEST_SUITE_END();
};


int main(void)
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(BrokerXifiNpmTest::suite());
	BrokerXifiNpmTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	BrokerXifiNpmTest::suiteTearDown();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


///////////////////////////////////////////////////////


string			BrokerXifiNpmTest::plugin_name;
string			BrokerXifiNpmTest::plugin_args;
bool			BrokerXifiNpmTest::plugin_nrpe;
service			BrokerXifiNpmTest::plugin_serv;
customvariablesmember	BrokerXifiNpmTest::custom_vars;


char* find_plugin_command_name(nebstruct_service_check_data* data, char** args, int* nrpe, const service** serv)
{
	char* name = NULL;

	name  = strdup(BrokerXifiNpmTest::plugin_name.c_str());
	*args = strdup(BrokerXifiNpmTest::plugin_args.c_str());
	*nrpe = (int) BrokerXifiNpmTest::plugin_nrpe;
	*serv = (const service*) &BrokerXifiNpmTest::plugin_serv;

	return name;
}


bool BrokerXifiNpmTest::init_module_variables(const string& args)
{
	char buffer[MAXBUFLEN];
	buffer[args.copy(buffer, MAXBUFLEN-1)] = '\0';
	return (bool) ::init_module_variables(buffer);
}


bool BrokerXifiNpmTest::free_module_variables()
{
	return (bool) ::free_module_variables();
}


bool BrokerXifiNpmTest::get_adapter_request(nebstruct_service_check_data* data, string& request)
{
	char* result = ::get_adapter_request(data);
	bool  error = (result == NULL);
	request.assign((error) ? "" : result);
	::free(result);
	result = NULL;
	return error;
}


void BrokerXifiNpmTest::suiteSetUp()
{
	string argline	= ((ostringstream&)(ostringstream().flush()
		<<        "-u" << ADAPTER_URL
		<< ' ' << "-r" << REGION_ID
		)).str();

	::adapter_url	= NULL;
	::region_id	= NULL;
	::host_addr	= NULL;
	init_module_variables(argline);
}


void BrokerXifiNpmTest::suiteTearDown()
{
	free_module_variables();
}


void BrokerXifiNpmTest::setUp()
{
}


void BrokerXifiNpmTest::tearDown()
{
}


void BrokerXifiNpmTest::get_request_ok_local_snmp_plugin_implicit_type()
{
	string expected_request, actual_request;

	// given
	plugin_serv.custom_variables = NULL;
	plugin_nrpe = false;
	plugin_name = SNMP_PLUGIN;
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTE_ADDR
		<< ' ' << "-C" << plugin_name
		<< ' ' << "-o" << SNMP_OID
		<< ' ' << "-m" << SNMP_MIBLIST
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTE_ADDR << '/' << PORT
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << NPM_DEFAULT_ENTITY_TYPE
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}


void BrokerXifiNpmTest::get_request_ok_local_snmp_plugin_explicit_type()
{
	string expected_request, actual_request;

	// given
	custom_vars.variable_name	= CUSTOM_VAR_ENTITY_TYPE;
	custom_vars.variable_value	= NPM_DEFAULT_ENTITY_TYPE;
	plugin_serv.custom_variables	= &custom_vars;
	plugin_nrpe = false;
	plugin_name = SNMP_PLUGIN;
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTE_ADDR
		<< ' ' << "-C" << plugin_name
		<< ' ' << "-o" << SNMP_OID
		<< ' ' << "-m" << SNMP_MIBLIST
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTE_ADDR << '/' << PORT
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << NPM_DEFAULT_ENTITY_TYPE
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}


void BrokerXifiNpmTest::wrong_request_local_snmp_plugin_custom_type()
{
	string expected_request, actual_request;

	// given
	custom_vars.variable_name	= CUSTOM_VAR_ENTITY_TYPE;
	custom_vars.variable_value	= SOME_ENTITY_TYPE;
	plugin_serv.custom_variables	= &custom_vars;
	plugin_nrpe = false;
	plugin_name = SNMP_PLUGIN;
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTE_ADDR
		<< ' ' << "-C" << plugin_name
		<< ' ' << "-o" << SNMP_OID
		<< ' ' << "-m" << SNMP_MIBLIST
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTE_ADDR << '/' << PORT
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << NPM_DEFAULT_ENTITY_TYPE
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);
	bool wrong = (expected_request != actual_request);

	// then
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT(wrong);
}


void BrokerXifiNpmTest::error_getting_request_remote_snmp_plugin_implicit_type()
{
	string expected_request, actual_request;

	// given
	plugin_serv.custom_variables = NULL;
	plugin_nrpe = true;
	plugin_name = SNMP_PLUGIN;
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTE_ADDR
		<< ' ' << "-c" << plugin_name
		<< ' ' << "-t" << TIMEOUT
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTE_ADDR << '/' << PORT
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << NPM_DEFAULT_ENTITY_TYPE
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(error);
}


void BrokerXifiNpmTest::error_getting_request_remote_snmp_plugin_explicit_type()
{
	string expected_request, actual_request;

	// given
	custom_vars.variable_name	= CUSTOM_VAR_ENTITY_TYPE;
	custom_vars.variable_value	= NPM_DEFAULT_ENTITY_TYPE;
	plugin_serv.custom_variables	= &custom_vars;
	plugin_nrpe = true;
	plugin_name = SNMP_PLUGIN;
	plugin_args =((ostringstream&)(ostringstream().flush()
		<<        "-H" << REMOTE_ADDR
		<< ' ' << "-c" << plugin_name
		<< ' ' << "-t" << TIMEOUT
		)).str();
	expected_request = ((ostringstream&)(ostringstream().flush()
		<< ADAPTER_URL << '/' << plugin_name
		<< '?' << ADAPTER_QUERY_FIELD_ID
		<< '=' << REGION_ID << ':' << REMOTE_ADDR << '/' << PORT
		<< '&' << ADAPTER_QUERY_FIELD_TYPE
		<< '=' << NPM_DEFAULT_ENTITY_TYPE
		)).str();

	// when
	bool error = get_adapter_request(CHECK_DATA, actual_request);

	// then
	CPPUNIT_ASSERT(error);
}
