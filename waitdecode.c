/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2015, Joyent, Inc.
 */

/*
 * waitdecode.c: print out details about a wait(3C) or bash(1) process exit
 * code.
 */

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define	EXIT_USAGE	2

static void waitdecode_wait(FILE *, unsigned int);
static void waitdecode_bash(FILE *, unsigned int);
static void waitdecode_sig(FILE *, int, const char *);

int
main(int argc, char *argv[])
{
	char *statstr;
	char *endp;
	long statl;
	unsigned int status;
	int asbash;
	int nargs;

	if (argc < 2) {
		errx(EXIT_USAGE, "usage: waitprint [-b] STATUS");
	}

	if (argc > 2 && strcmp(argv[1], "-b") == 0) {
		nargs = 3;
		asbash = 1;
	} else {
		nargs = 2;
		asbash = 0;
	}

	if (argc > nargs) {
		errx(EXIT_USAGE, "unexpected arguments");
	}

	statstr = argv[nargs - 1];
	errno = 0;
	statl = strtol(statstr, &endp, 0);
	if (statl == 0 && errno != 0) {
		err(EXIT_USAGE, "bad value: %s", statstr);
	}

	if (*endp != '\0') {
		errx(EXIT_USAGE, "unexpected characters: %s", endp);
	}

	if (statl < 0) {
		errx(EXIT_USAGE, "value must be non-negative: %s", statstr);
	}

	if (statl > UINT16_MAX) {
		errx(EXIT_USAGE, "value too large: %s", statstr);
	}

	status = (int)statl;
	(void) fprintf(stdout, "status: 0x%x (decimal %d), as %s\n",
	    status, status, asbash ? "bash return code" : "wait(3C) status");

	if (asbash == 0) {
		waitdecode_wait(stdout, status);
	} else {
		waitdecode_bash(stdout, status);
	}

	return (0);
}

/*
 * Decodes the wait status as returned by wait(3C) and similar functions.  See
 * your system's documentation for details, or http://illumos.org/man/wait.3c
 * and http://illumos.org/man/wait.h for a good overview.
 */
static void
waitdecode_wait(FILE *out, unsigned int status)
{
	/*
	 * The next three conditions should be exclusive, but we interpret what
	 * we're given and leave interpretation to the user.
	 */
	if (WIFEXITED(status)) {
		(void) fprintf(out,
		    "    process exited normally with exit status %d\n",
		    WEXITSTATUS(status));
	}

	if (WIFSIGNALED(status)) {
		waitdecode_sig(out, WTERMSIG(status), "terminated");

		if (WCOREDUMP(status)) {
			(void) fprintf(out,
			    "    core file created on termination\n");
		}
	}

	if (WIFSTOPPED(status)) {
		waitdecode_sig(out, WSTOPSIG(status), "stopped");
	}

	if (WIFCONTINUED(status)) {
		(void) fprintf(out, "    process continued\n");
	}
}

/*
 * Decodes the wait status as returned by bash.  This contains less information
 * than wait(3C) provides, but at least tells you whether this was a normal
 * exit or an exit by signal, and which code or signal it was.  See the return
 * value for "Simple Commands" in bash(1).
 */
static void
waitdecode_bash(FILE *out, unsigned int status)
{
	if (status < 128) {
		(void) fprintf(out, "    process terminated normally with "
		    "exit status %d\n", status);
	} else {
		waitdecode_sig(out, status - 128, "terminated");
	}
}

static void
waitdecode_sig(FILE *out, int signum, const char *verb)
{
	char signame[SIG2STR_MAX];

	if (sig2str(signum, signame) == 0)
		(void) fprintf(out, "    process %s on SIG%s\n", verb, signame);
	else
		(void) fprintf(out, "    process %s on unknown signal %d\n",
		    verb, signum);
}
