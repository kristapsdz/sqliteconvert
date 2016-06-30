/*	$Id$ */
/*
 * Copyright (c) 2016 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/queue.h>

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

static void
safe_putstring(const char *p)
{

	for ( ; '\0' != *p; p++)
		switch (*p) {
		case ('<'):
			fputs("&gt;", stdout);
			break;
		case ('>'):
			fputs("&lt;", stdout);
			break;
		case ('"'):
			fputs("&quot;", stdout);
			break;
		case ('&'):
			fputs("&amp;", stdout);
			break;
		default:
			putchar(*p);
			break;
		}
}

static void
output(struct parse *p, const char *prefix, const char *topts, 
	const char *fopts, const char *ropts)
{
	struct tab	*tab;
	struct col	*col;
	char		*cp;

	if (NULL == fopts)
		fopts = ropts;

	puts("digraph G {");
	TAILQ_FOREACH(tab, &p->tabq, entry) {
		printf("\ttable%zu [shape=none; label=<"
			"<TABLE%s%s>\n",
		       tab->idx, NULL == topts ? "" : " ",
		       NULL == topts ? "" : topts);
		cp = sqlite_schema_id(tab->name, NULL);
		printf("\t\t\t<TR><TD %s%sHREF=\"#%s-%s\">", 
			NULL == fopts ? "" : fopts,
			NULL == fopts ? "" : " ", prefix, cp);
		free(cp);
		safe_putstring(tab->name);
		puts("</TD></TR>");
		TAILQ_FOREACH(col, &tab->colq, entry) {
			cp = sqlite_schema_id
				(col->tab->name, col->name);
			printf("\t\t\t<TR><TD %s%sHREF=\"#%s-%s\" "
				"PORT=\"f%zu\">", 
				NULL == ropts ? "" : ropts,
				NULL == ropts ? "" : " ",
				prefix, cp, col->idx);
			free(cp);
			safe_putstring(col->name);
			puts("</TD></TR>");
		}
		puts("\t\t</TABLE>>];");
		TAILQ_FOREACH(col, &tab->colq, entry) {
			if (NULL == col->fkey)
				continue;
			printf("\ttable%zu:f%zu -> table%zu:f%zu;\n",
				col->tab->idx, col->idx,
				col->fkey->tab->idx, col->fkey->idx);
		}
	}
	puts("}");
}

static int
append(char **val, const char *cp)
{
	size_t	 sz, tsz;

	if (0 == strncasecmp(cp, "href=", 5))
		return(0);

	if (NULL != *val) {
		sz = strlen(*val);
		tsz = sz + strlen(cp) + 2;
		if (NULL == (*val = realloc(*val, tsz)))
			err(EXIT_FAILURE, "realloc");
		strlcat(*val, " ", tsz);
		strlcat(*val, cp, tsz);
	} else
		if (NULL == (*val = strdup(cp)))
			err(EXIT_FAILURE, "strdup");

	return(1);
}

int
main(int argc, char *argv[])
{
	int	 	 rc, c;
	char		*topts, *fopts, *ropts;
	struct parse	 p;
	const char	*prefix;

	memset(&p, 0, sizeof(struct parse));
	topts = ropts = fopts = NULL;
	prefix = "sql";

	while (-1 != (c = getopt(argc, argv, "h:c:t:p:v"))) 
		switch (c) {
		case ('p'):
			prefix = optarg;
			break;
		case ('t'):
			if ( ! append(&topts, optarg))
				warnx("-%c %s: ignoring", c, optarg);
			break;
		case ('h'):
			if ( ! append(&fopts, optarg)) 
				warnx("-%c %s: ignoring", c, optarg);
			break;
		case ('c'):
			if ( ! append(&ropts, optarg))
				warnx("-%c %s: ignoring", c, optarg);
			break;
		case ('v'):
			p.verbose = 1;
			break;
		default:
			goto usage;
		}

	argc -= optind;
	argv += optind;

	if (0 == argc)
		rc = sqlite_schema_parsestdin(&p);
	else 
		rc = sqlite_schema_parsefile(argv[0], &p);

	if (rc > 0)
		output(&p, prefix, topts, fopts, ropts);

	sqlite_schema_free(&p);
	return(rc ? EXIT_SUCCESS : EXIT_FAILURE);

usage:
	fprintf(stderr, "usage: %s [-v] "
		"[-c attrs] "
		"[-h attrs] "
		"[-t attrs] "
		"file\n", getprogname());
	return(EXIT_FAILURE);
}
