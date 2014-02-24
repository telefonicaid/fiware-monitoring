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
class HostBrokerTest: public TestFixture
{
	static string localhost_addr;
	static string valid_adapter_url;
	static string valid_region_id;
	static string valid_plugin_name;
	static string valid_plugin_args;

	// C external function wrappers
	static bool init_module_variables(const string& args);
	static bool free_module_variables();
	static bool get_adapter_request(nebstruct_service_check_data* data, string& request);

	// mock for find_plugin_name()
	friend char* find_plugin_name(nebstruct_service_check_data* data, char** args);

	// tests
	void request_ok_with_valid_host_plugin_args();
	void request_ok_with_valid_nrpe_plugin_args();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(HostBrokerTest);
	CPPUNIT_TEST(request_ok_with_valid_host_plugin_args);
	CPPUNIT_TEST(request_ok_with_valid_nrpe_plugin_args);
	CPPUNIT_TEST_SUITE_END();
};


int main(void)
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(HostBrokerTest::suite());
	HostBrokerTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	HostBrokerTest::suiteTearDown();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


///////////////////////////////////////////////////////


string	HostBrokerTest::localhost_addr;
string	HostBrokerTest::valid_adapter_url;
string	HostBrokerTest::valid_region_id;
string	HostBrokerTest::valid_plugin_name;
string	HostBrokerTest::valid_plugin_args;


char* find_plugin_name(nebstruct_service_check_data* data, char** args)
{
	char* result = NULL;
	if (args) {
		*args  = strdup(HostBrokerTest::valid_plugin_args.c_str());
		result = strdup(HostBrokerTest::valid_plugin_name.c_str());
	}
	return result;
}


bool HostBrokerTest::init_module_variables(const string& args)
{
	char buffer[MAXBUFLEN];
	buffer[args.copy(buffer, MAXBUFLEN-1)] = '\0';
	return (bool) ::init_module_variables(buffer);
}


bool HostBrokerTest::free_module_variables()
{
	return (bool) ::free_module_variables();
}


bool HostBrokerTest::get_adapter_request(nebstruct_service_check_data* data, string& request)
{
	char* result = ::get_adapter_request(data);
	bool  error = (result == NULL);
	request.assign((error) ? "" : result);
	::free(result);
	return error;
}


void HostBrokerTest::suiteSetUp()
{
	char name[HOST_NAME_MAX];
	char addr[INET_ADDRSTRLEN];

	valid_adapter_url = "http://localhost:5000";
	valid_region_id   = "myregion";
	if (gethostname(name, HOST_NAME_MAX) || resolve_address(name, addr, INET_ADDRSTRLEN)) {
		localhost_addr.assign("127.0.0.1");
	} else {
		localhost_addr.assign(addr);
	}
}


void HostBrokerTest::suiteTearDown()
{
}


void HostBrokerTest::setUp()
{
	::adapter_url = NULL;
	::region_id   = NULL;
	::host_addr   = NULL;
}


void HostBrokerTest::tearDown()
{
	free_module_variables();
}


void HostBrokerTest::request_ok_with_valid_host_plugin_args()
{
	ostringstream buffer;

	string expected_request, actual_request;

	valid_plugin_name = "check_load";
	valid_plugin_args =((ostringstream&)(buffer
		<<        "-w " << 1 << ',' << 5 << ',' << 15
		<< ' ' << "-c " << 1 << ',' << 5 << ',' << 15
	)).str();

	buffer.str("");
	string argline = ((ostringstream&)(buffer
		<<        "-u" << valid_adapter_url
		<< ' ' << "-r" << valid_region_id
	)).str();

	buffer.str("");
	expected_request = ((ostringstream&)(buffer
		<< valid_adapter_url << '/' << valid_plugin_name
		<< '?'
		<< "id=" << valid_region_id << ':' << localhost_addr
		<< '&'
		<< "type=" << "host"
	)).str();

	bool error = false;
	if (!(error = init_module_variables(argline))) {
		error = get_adapter_request(NULL, actual_request);
	}
	
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT(valid_adapter_url == ::adapter_url);
	CPPUNIT_ASSERT(valid_region_id == ::region_id);
	CPPUNIT_ASSERT(localhost_addr == ::host_addr);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}


void HostBrokerTest::request_ok_with_valid_nrpe_plugin_args()
{
	ostringstream buffer;

	string expected_request, actual_request;

	string remote_addr   = "10.11.100.80";
	string remote_plugin = "check_load";

	valid_plugin_name = "check_nrpe";
	valid_plugin_args =((ostringstream&)(buffer
		<<        "-H " << remote_addr
		<< ' ' << "-c " << remote_plugin
	)).str();

	buffer.str("");
	string argline = ((ostringstream&)(buffer
		<<        "-u" << valid_adapter_url
		<< ' ' << "-r" << valid_region_id
	)).str();

	buffer.str("");
	expected_request = ((ostringstream&)(buffer
		<< valid_adapter_url << '/' << remote_plugin
		<< '?'
		<< "id=" << valid_region_id << ':' << remote_addr
		<< '&'
		<< "type=" << "vm"
	)).str();

	bool error = false;
	if (!(error = init_module_variables(argline))) {
		error = get_adapter_request(NULL, actual_request);
	}
	
	CPPUNIT_ASSERT(!error);
	CPPUNIT_ASSERT(valid_adapter_url == ::adapter_url);
	CPPUNIT_ASSERT(valid_region_id == ::region_id);
	CPPUNIT_ASSERT_EQUAL(expected_request, actual_request);
}
