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
 * @file   suite_argument_parser.cc
 * @brief  Test suite to verify argument parsing
 *
 * This file defines unit tests to verify argument parsing features
 * (see argument_parser.c).
 */


#include <list>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include "argument_parser.h"
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


/// Option string prefix to avoid errors when unknown options are found
#define OPTSTR_PREFIX	":"


/// Initialization of `std::string` to non-null character strings
#define NON_NULL(s)	((s) ? (s):"")


/// C++ version of option_value
struct OptionValue
{
	int		opt;	///< option ('?' unknown, ':' missing value)
	int		err;	///< option that caused error (unknown/missing)
	string		val;	///< option value, or "" if an error is found
	OptionValue(option_value& other): opt(other.opt), err(other.err), val(NON_NULL(other.val)) {}
};


/// Argument parsing test suite
class ArgumentParserTest: public TestFixture
{
	// static methods equivalent to external C functions
	static void parse_args(const string& args, const string& optstr, list<OptionValue>& optlist);

	// internal methods
	void parse_ok_with_valid_argument_opts(const string& separator);

	// tests
	void parse_detects_missing_required_argument_first();
	void parse_detects_missing_required_argument_last();
	void parse_detects_unknown_option();
	void parse_ok_with_valid_argument_opts_space_separation();
	void parse_ok_with_valid_argument_opts_tab_separation();
	void parse_ok_with_valid_argument_opts_no_separation();

public:
	static void suiteSetUp();
	static void suiteTearDown();
	void setUp();
	void tearDown();
	CPPUNIT_TEST_SUITE(ArgumentParserTest);
	CPPUNIT_TEST(parse_detects_missing_required_argument_first);
	CPPUNIT_TEST(parse_detects_missing_required_argument_last);
	CPPUNIT_TEST(parse_detects_unknown_option);
	CPPUNIT_TEST(parse_ok_with_valid_argument_opts_space_separation);
	CPPUNIT_TEST(parse_ok_with_valid_argument_opts_tab_separation);
	CPPUNIT_TEST(parse_ok_with_valid_argument_opts_no_separation);
	CPPUNIT_TEST_SUITE_END();
};


/// Suite startup
int main(int argc, char* argv[])
{
	TextTestRunner runner;
	BriefTestProgressListener progress;
	runner.eventManager().addListener(&progress);
	runner.addTest(ArgumentParserTest::suite());
	ArgumentParserTest::suiteSetUp();
	cout << endl << endl;
	bool success = runner.run("", false, true, false);
	ArgumentParserTest::suiteTearDown();
	ofstream xmlFileOut((string(argv[0]) + "-cppunit-results.xml").c_str());
	XmlOutputter xmlOut(&runner.result(), xmlFileOut);
	xmlOut.write();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


///
/// Static method wrapping C function ::parse_args from module
///
/// @param[in]  args		The module arguments as a space-separated string.
/// @param[in]  optstr		The option string as defined for ::getopt.
/// @param[out] optlist		The module arguments as options list.
///
void ArgumentParserTest::parse_args(const string& args, const string& optstr, list<OptionValue>& optlist)
{
	option_list_t opts = ::parse_args(args.c_str(), optstr.c_str());
	if (opts) {
		for (size_t i = 0; opts[i].opt != NO_CHAR; i++) {
			optlist.push_back(OptionValue(opts[i]));
		}
		::free_option_list(opts);
		opts = NULL;
	}
}


///
/// Suite setup
///
void ArgumentParserTest::suiteSetUp()
{
}


///
/// Suite teardown
///
void ArgumentParserTest::suiteTearDown()
{
}


///
/// Tests setup
///
void ArgumentParserTest::setUp()
{
}


///
/// Tests teardown
///
void ArgumentParserTest::tearDown()
{
}


///////////////////////////////////


void ArgumentParserTest::parse_detects_missing_required_argument_first()
{
	char	opt_no_arg = 'a',
		opt_valid  = 'b';

	// given
	string optstr = ((ostringstream&)(ostringstream(OPTSTR_PREFIX, ios_base::ate).flush()
		<< opt_no_arg << ':'
		<< opt_valid  << ':'
		)).str();
	string argline = ((ostringstream&)(ostringstream().flush()
		<<        '-' << opt_no_arg << ""
		<< ' ' << '-' << opt_valid  << "value"
		)).str();

	// when
	list<OptionValue> optlist;
	parse_args(argline, optstr, optlist);

	// then
	bool found_opt_valid  = false;
	bool found_opt_no_arg = false;
	bool detected_missing = false;
	for (list<OptionValue>::iterator iter = optlist.begin(); iter != optlist.end(); iter++) {
		found_opt_no_arg = found_opt_no_arg || (iter->opt == opt_no_arg);
		found_opt_valid  = found_opt_valid  || (iter->opt == opt_valid);
		detected_missing = (iter->opt == MISSING_VALUE) && (iter->err == opt_no_arg);
	}
	CPPUNIT_ASSERT(!found_opt_no_arg);	// opt_no_arg not found because it has no value
	CPPUNIT_ASSERT(!found_opt_valid);	// args parsing aborts prior reaching opt_valid
	CPPUNIT_ASSERT(detected_missing);
}


void ArgumentParserTest::parse_detects_missing_required_argument_last()
{
	char	opt_valid  = 'a',
		opt_no_arg = 'b';

	// given
	string optstr = ((ostringstream&)(ostringstream(OPTSTR_PREFIX, ios_base::ate).flush()
		<< opt_valid  << ':'
		<< opt_no_arg << ':'
		)).str();
	string argline = ((ostringstream&)(ostringstream().flush()
		<<        '-' << opt_valid  << "value"
		<< ' ' << '-' << opt_no_arg << ""
		)).str();

	// when
	list<OptionValue> optlist;
	parse_args(argline, optstr, optlist);

	// then
	bool found_opt_valid  = false;
	bool found_opt_no_arg = false;
	bool detected_missing = false;
	for (list<OptionValue>::iterator iter = optlist.begin(); iter != optlist.end(); iter++) {
		found_opt_valid  = found_opt_valid  || (iter->opt == opt_valid);
		found_opt_no_arg = found_opt_no_arg || (iter->opt == opt_no_arg);
		detected_missing = (iter->opt == MISSING_VALUE) && (iter->err == opt_no_arg);
	}
	CPPUNIT_ASSERT(found_opt_valid);	// first valid option is reached
	CPPUNIT_ASSERT(!found_opt_no_arg);	// opt_no_arg not found because it has no value
	CPPUNIT_ASSERT(detected_missing);
}


void ArgumentParserTest::parse_detects_unknown_option()
{
	char	opt_valid   = 'a',
		opt_unknown = 'b';

	// given
	string optstr = ((ostringstream&)(ostringstream(OPTSTR_PREFIX, ios_base::ate).flush()
		<< opt_valid << ':'
		)).str();
	string argline = ((ostringstream&)(ostringstream().flush()
		<<        '-' << opt_valid   << "value"
		<< ' ' << '-' << opt_unknown << "value"
		)).str();

	// when
	list<OptionValue> optlist;
	parse_args(argline, optstr, optlist);

	// then
	bool found_opt_valid   = false;
	bool found_opt_unknown = false;
	bool detected_unknown  = false;
	for (list<OptionValue>::iterator iter = optlist.begin(); iter != optlist.end(); iter++) {
		found_opt_valid   = found_opt_valid   || (iter->opt == opt_valid);
		found_opt_unknown = found_opt_unknown || (iter->opt == opt_unknown);
		detected_unknown  = (iter->opt == UNKNOWN_OPTION) && (iter->err == opt_unknown);
	}
	CPPUNIT_ASSERT(found_opt_valid);
	CPPUNIT_ASSERT(!found_opt_unknown);
	CPPUNIT_ASSERT(detected_unknown);
}


void ArgumentParserTest::parse_ok_with_valid_argument_opts_space_separation()
{
	parse_ok_with_valid_argument_opts(" ");
}


void ArgumentParserTest::parse_ok_with_valid_argument_opts_tab_separation()
{
	parse_ok_with_valid_argument_opts("\t");
}


void ArgumentParserTest::parse_ok_with_valid_argument_opts_no_separation()
{
	parse_ok_with_valid_argument_opts("");
}


void ArgumentParserTest::parse_ok_with_valid_argument_opts(const string& separator)
{
	char	opt_valid_1 = 'a',
		opt_valid_2 = 'b';

	// given
	string optstr = ((ostringstream&)(ostringstream(OPTSTR_PREFIX, ios_base::ate).flush()
		<< opt_valid_1 << ':'
		<< opt_valid_2 << ':'
		)).str();
	string argline = ((ostringstream&)(ostringstream().flush()
		<<        '-' << opt_valid_1 << separator << "value"
		<< ' ' << '-' << opt_valid_2 << separator << "value"
		)).str();

	// when
	list<OptionValue> optlist;
	parse_args(argline, optstr, optlist);

	// then
	bool found_opt_valid_1 = false;
	bool found_opt_valid_2 = false;
	for (list<OptionValue>::iterator iter = optlist.begin(); iter != optlist.end(); iter++) {
		found_opt_valid_1 = found_opt_valid_1 || (iter->opt == opt_valid_1);
		found_opt_valid_2 = found_opt_valid_2 || (iter->opt == opt_valid_2);
	}
	CPPUNIT_ASSERT(found_opt_valid_1);
	CPPUNIT_ASSERT(found_opt_valid_2);
	CPPUNIT_ASSERT(optlist.size() == 2);
}
