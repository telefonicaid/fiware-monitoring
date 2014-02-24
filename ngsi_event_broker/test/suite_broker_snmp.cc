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


// test suite
class SNMPBrokerTest: public TestFixture
{
	static string valid_adapter_url;
	static string valid_region_id;
	static string valid_snmp_parameter;
	static string valid_interface_host;
	static int    valid_interface_port;
	static string valid_plugin_name;
	static string valid_plugin_args;

	// C external function wrappers
	static bool init_module_variables(const string& args);
	static bool free_module_variables();
	static bool get_adapter_request(nebstruct_service_check_data* data, string& request);

	// mock for find_plugin_name()
	friend char* find_plugin_name(nebstruct_service_check_data* data, char** args);

	// tests
	void request_ok_with_valid_plugin_args();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(SNMPBrokerTest);
	CPPUNIT_TEST(request_ok_with_valid_plugin_args);
	CPPUNIT_TEST_SUITE_END();
};


int main(void)
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(SNMPBrokerTest::suite());
	SNMPBrokerTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	SNMPBrokerTest::suiteTearDown();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


///////////////////////////////////////////////////////


string	SNMPBrokerTest::valid_adapter_url;
string	SNMPBrokerTest::valid_region_id;
string	SNMPBrokerTest::valid_snmp_parameter;
string	SNMPBrokerTest::valid_interface_host;
int	SNMPBrokerTest::valid_interface_port;
string	SNMPBrokerTest::valid_plugin_name;
string	SNMPBrokerTest::valid_plugin_args;


char* find_plugin_name(nebstruct_service_check_data* data, char** args)
{
	char* result = NULL;
	if (args) {
		*args  = strdup(SNMPBrokerTest::valid_plugin_args.c_str());
		result = strdup(SNMPBrokerTest::valid_plugin_name.c_str());
	}
	return result;
}


bool SNMPBrokerTest::init_module_variables(const string& args)
{
	char buffer[MAXBUFLEN];
	buffer[args.copy(buffer, MAXBUFLEN-1)] = '\0';
	return (bool) ::init_module_variables(buffer);
}


bool SNMPBrokerTest::free_module_variables()
{
	return (bool) ::free_module_variables();
}


bool SNMPBrokerTest::get_adapter_request(nebstruct_service_check_data* data, string& request)
{
	char* result = ::get_adapter_request(data);
	bool  error = (result == NULL);
	request.assign((error) ? "" : result);
	::free(result);
	return error;
}


void SNMPBrokerTest::suiteSetUp()
{
	ostringstream buffer;

	valid_adapter_url    = "http://localhost:5000";
	valid_region_id      = "myregion";
	valid_snmp_parameter = ".1.3.6.1.2.1.2.2.1.8";
	valid_interface_host = "10.11.100.80";
	valid_interface_port = 20;
	valid_plugin_name    = "check_snmp";
	valid_plugin_args    =((ostringstream&)(buffer
		<<        "-H " << valid_interface_host
		<< ' ' << "-o " << valid_snmp_parameter << '.' << (valid_interface_port + 1)
	)).str();
}


void SNMPBrokerTest::suiteTearDown()
{
}


void SNMPBrokerTest::setUp()
{
	::adapter_url = NULL;
	::region_id   = NULL;
	::host_addr   = NULL;
}


void SNMPBrokerTest::tearDown()
{
	free_module_variables();
}


void SNMPBrokerTest::request_ok_with_valid_plugin_args()
{
	ostringstream buffer;

	string expected_request, actual_request;

	string argline = ((ostringstream&)(buffer
		<<        "-u" << valid_adapter_url
		<< ' ' << "-r" << valid_region_id
	)).str();

	buffer.str("");
	expected_request = ((ostringstream&)(buffer
		<< valid_adapter_url << '/' << valid_plugin_name
		<< '?'
		<< "id="
			<< valid_region_id
			<< ':'
			<< valid_interface_host << '/' << valid_interface_port
		<< '&'
		<< "type=" << "interface"
	)).str();

	bool error = false;
	if (!(error = init_module_variables(argline))) {
		error = get_adapter_request(NULL, actual_request);
	}
	
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT(valid_adapter_url == ::adapter_url);
	CPPUNIT_ASSERT(valid_region_id == ::region_id);
	CPPUNIT_ASSERT(::host_addr != NULL);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}
