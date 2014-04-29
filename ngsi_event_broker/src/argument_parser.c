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
 * @file   argument_parser.c
 * @brief  Argument parsing implementation
 *
 * This file consists of the implementation of the argument parsing component.
 */


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "argument_parser.h"


/* POSIX compliance */
#ifndef POSIXLY_CORRECT
#define POSIXLY_CORRECT
#endif /*POSIXLY_CORRECT*/


/* maximum number of args */
#define MAX_ARGS	255


/* parses an argument string using getopt() */
option_list_t parse_args(char* args, const char* optstr)
{
	extern int	optind;
	extern int	optopt;
	extern char*	optarg;
	option_list_t	optlist	= NULL;
	size_t		optsize	= 0;
	int		opt;

	int		argc = 0;
	char*		argv[MAX_ARGS] = { "" };

	/* split string into char* array */
	char* last;
	char* next = args;
	while (next && (argc < MAX_ARGS-1)) {
		argv[++argc] = next = strtok_r(args, " \t", &last);
		args = NULL;
	}

	/* fill option_value structs */
	optind  = 1;
	optlist = (option_list_t) malloc(argc * sizeof(struct option_value));
	while ((opt = getopt(argc, argv, optstr)) != -1) {
		const int fail = (strchr("?:", opt) != NULL);
		const struct option_value optval = {
			.opt = opt,
			.err = (fail) ? optopt : NO_CHAR,
			.val = (fail) ? NULL   : optarg
		};
		optlist[optsize++] = optval;
		if (fail) break;
	}

	/* end of list */
	optlist[optsize].opt = NO_CHAR;

	return optlist;
}
