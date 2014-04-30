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
 * @file   argument_parser.h
 * @brief  Argument parsing macros and declarations
 *
 * This file defines several macros and declares functions to parse the arguments
 * (initially, from Nagios configuration file) passed to [Event Broker](@NagiosModule_ref)
 * module.
 */


#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H


#ifdef __cplusplus
extern "C" {
#endif


/** Value meaning no option char (therefore, the end of options list) */
#ifndef NO_CHAR
#define NO_CHAR -1
#endif /*NO_CHAR*/


/** Option-value pair */
typedef struct option_value {
	int		opt;	/**< option ('?' unknown, ':' missing value)    */
	int		err;	/**< option that caused error (unknown/missing) */
	const char*	val;	/**< option value, or NULL if an error is found */
} *option_list_t;		/**< Options list as result of argument parsing */


/**
 * Parses module arguments given in configuration file
 *
 * @param[in] args		The module arguments as a space-separated string.
 * @param[in] optstr		The option string as defined for [getopt()](@getopt_ref).
 *
 * @return			The module arguments as options list.
 */
option_list_t parse_args(char* args, const char* optstr);


#ifdef __cplusplus
}
#endif


#endif /*ARGUMENT_PARSER_H*/
