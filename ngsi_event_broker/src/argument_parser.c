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
 * @file   argument_parser.c
 * @brief  Argument parsing implementation
 *
 * This file consists of the implementation of the argument parsing component.
 */


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "argument_parser.h"


/* maximum number of args */
#define MAX_ARGS	255


/* parses an argument string using getopt() */
option_list_t parse_args(const char* args, const char* optstr)
{
	extern int	optind;
	extern int	optopt;
	extern char*	optarg;
	option_list_t	optlist	= NULL;
	size_t		optsize	= 0;
	int		opt;

	int		argc = 0;
	char*		argv[MAX_ARGS] = { "" };

	/* possible opt values if parsing fails */
	char		optfail[] = { UNKNOWN_OPTION, MISSING_VALUE, '\0' };

	/* split a non-null copy of args string into char* array */
	char* last;
	char* copy = strdup((args) ? args : "");
	for (args = copy; (argv[argc] != NULL) && (argc < MAX_ARGS-1);) {
		argv[++argc] = strtok_r((char*) args, " \t", &last);
		args = NULL;
	}

	/* fill option_value structs */
	optind  = 0;
	optlist = (option_list_t) malloc(argc * sizeof(struct option_value));
	while ((opt = getopt(argc, argv, optstr)) != -1) {
		const int dash = (optarg != NULL && optarg[0] == '-');
		const int fail = (strchr(optfail, opt) != NULL) || dash;
		const struct option_value optval = {
			.opt = (dash) ? MISSING_VALUE : opt,
			.err = (fail) ? ((dash) ? opt : optopt) : NO_CHAR,
			.val = (fail) ? NULL : strdup(optarg)
		};
		optlist[optsize++] = optval;
		if (fail) break;
	}

	/* end of list */
	optlist[optsize].opt = NO_CHAR;

	free(copy);
	copy = NULL;
	return optlist;
}


/* releases resources for given options list */
void free_option_list(option_list_t optlist)
{
	if (optlist != NULL) {
		size_t	i;
		for (i = 0; optlist[i].opt != NO_CHAR; i++) {
			free(optlist[i].val);
		}
		free(optlist);
		optlist = NULL;
	}
}
