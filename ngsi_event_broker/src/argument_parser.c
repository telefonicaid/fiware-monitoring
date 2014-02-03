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


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "argument_parser.h"


/* maximum number of args */
#define MAX_ARGS	255


/* parses a command line string using getopt() */
option_list_t parse_args(char* args, const char* optstr)
{
	extern char*	optarg;
	option_list_t	optlist	= NULL;
	size_t		optsize	= 0;
	int		opt;

	int		argc = 0;
	char*		argv[MAX_ARGS] = { "" };
	char*		last;

	char* ptr = args;
	while (ptr && argc < MAX_ARGS-1) {
		argv[++argc] = ptr = strtok_r(args, " ", &last);
		args = NULL;
	}

	optlist = (option_list_t) malloc(argc * sizeof(struct option_value));
	while ((opt = getopt(argc, argv, optstr)) != -1) {
		const struct option_value optval = {
			.opt = opt,
			.val = optarg
		};
		if (opt != '?') {
			optlist[optsize++] = optval;
		}
	}

	optlist[optsize].opt = -1;
	return optlist;
}
