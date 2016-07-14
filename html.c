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

struct	opts {
	const char	*prefix;
};

/*
 * Put a single character into an HTML stream on stdout.
 * Beyond the usual, this also normalises spaces into white-space.
 */
static void
safe_putchar(char c)
{

	switch (c) {
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
		putchar(isspace((int)c) ? ' ' : c);
		break;
	}
}

/*
 * Safely put a buffer of characters into an HTML stream to stdout.
 */
static void
safe_putbuf(const char *p, size_t sz)
{
	size_t	 i;

	for (i = 0; i < sz; i++)
		safe_putchar(p[i]);
}

/*
 * See safe_putbuf().
 */
static void
safe_putstr(const char *p)
{

	safe_putbuf(p, strlen(p));
}

static int
escaped_streq(const char *op, const char *p, const char *str)
{

	if (op != p && '\\' == p[-1])
		return(0);
	return(0 == strncmp(p, str, strlen(str)));
}

/*
 * Put a comment into the stdout HTML stream.
 * This will automatically convert @-references into links.
 */
static void
safe_putcomment(const struct opts *opts, const char *p)
{
	const char	*op, *link;
	char		*cp;
	size_t		 sz, linksz;

	for (op = p; '\0' != *p; ) {
		if ('\\' == *p) {
			if (op != p && '\\' == p[-1])
				safe_putchar(*p);
			p++;
			continue;
		} else if (escaped_streq(op, p, "\n")) {
			fputs("<p></p>", stdout);
			p += 1;
			continue;
		} else if (escaped_streq(op, p, "``")) {
			fputs("&#x201c;", stdout);
			p += 2;
			continue;
		} else if (escaped_streq(op, p, "\'\'")) {
			fputs("&#x201d;", stdout);
			p += 2;
			continue;
		} else if (escaped_streq(op, p, "---")) {
			fputs("&#8212;", stdout);
			p += 3;
			continue;
		} else if (escaped_streq(op, p, "--")) {
			fputs("&#8211;", stdout);
			p += 2;
			continue;
		} 
		
		/* 
		 * Now we catch our links: '@' for an in-document
		 * reference and '[' (Markdown style) for a general
		 * reference.
		 */

		if ( ! escaped_streq(op, p, "@") &&
		     ! escaped_streq(op, p, "[")) {
			safe_putchar(*p++);
			continue;
		}

		link = op = NULL;

		if ('[' == *p)
			link = ++p;
		else
			op = ++p;

		sz = linksz = 0;

		if (NULL != link) {
			for ( ; '\0' != *p && ']' != *p; p++)
				linksz++;
			if ('\0' != *p)
				p++;
			if ('(' == *p) {
				op = ++p;
				for ( ; '\0' != *p && ')' != *p; p++)
					sz++;
				if ('\0' != *p)
					p++;
			}
		} else if ('"' == *p) {
			/* Quote-escaped @-reference. */
			for (op = ++p; '\0' != *p && '"' != *p; p++)
				sz++;
			if ('\0' != *p)
				p++;
		} else
			for ( ; '\0' != *p && ! isspace((int)*p); p++)
				if ('(' == *p || ')' == *p || 
				    ';' == *p || ',' == *p)
					break;
				else
					sz++;

		if ((NULL != link && 0 == linksz) || 
		    (NULL == link && 0 == sz)) {
			putchar('@');
			continue;
		} 

		if (NULL != link) {
			if (NULL != op)
				printf("<a href=\"%.*s\">", (int)sz, op);
			else
				printf("<a href=\"%.*s\">", (int)linksz, link);
			safe_putbuf(link, linksz);
		} else {
			cp = sqlite_schema_idbuf(op, sz);
			printf("<a href=\"#%s-%s\">", opts->prefix, cp);
			free(cp);
			safe_putbuf(op, sz);
		}
		fputs("</a>", stdout);
	}
}

static void
output(const struct opts *opts, struct parse *p)
{
	struct tab	*tab;
	struct col	*col;
	char		*cp;

	puts("<dl class=\"tabs\">");
	TAILQ_FOREACH(tab, &p->tabq, entry) {
		cp = sqlite_schema_id(tab->name, NULL);
		printf("\t<dt id=\"%s-%s\">", opts->prefix, cp);
		free(cp);
		safe_putstr(tab->name);
		puts("</dt>");
		puts("\t<dd>");
		if (NULL != tab->comment) {
			puts("\t\t<div class=\"comment\">");
			fputs("\t\t\t", stdout);
			safe_putcomment(opts, tab->comment);
			puts("\n\t\t</div>");
		}
		puts("\t\t<dl class=\"cols\">");
		TAILQ_FOREACH(col, &tab->colq, entry) {
			cp = sqlite_schema_id
				(col->tab->name, col->name);
			printf("\t\t\t<dt id=\"%s-%s\">", 
				opts->prefix, cp);
			free(cp);
			safe_putstr(col->name);
			puts("</dt>");
			puts("\t\t\t<dd>");
			if (NULL != col->fkey) {
				fputs("\t\t\t\t<div "
					"class=\"foreign\">", stdout);
				cp = sqlite_schema_id
					(col->fkey->tab->name, 
					 col->fkey->name);
				printf("<a href=\"#%s-%s\">", 
					opts->prefix, cp);
				free(cp);
				safe_putstr(col->fkey->tab->name);
				safe_putstr(".");
				safe_putstr(col->fkey->name);
				puts("</a></div>");
			}
			if (NULL != col->comment) {
				puts("\t\t\t\t<div class=\"comment\">");
				fputs("\t\t\t\t\t", stdout);
				safe_putcomment(opts, col->comment);
				puts("\n\t\t\t\t</div>");
			}
			puts("\t\t\t</dd>");
		}
		puts("\t\t</dl>");
		puts("\t</dd>");
	}
	puts("</dl>");
}

int
main(int argc, char *argv[])
{
	int	 	 rc, c;
	struct parse	 p;
	struct opts	 opts;

	memset(&opts, 0, sizeof(struct opts));
	memset(&p, 0, sizeof(struct parse));
	opts.prefix = "sql";

	while (-1 != (c = getopt(argc, argv, "v"))) 
		switch (c) {
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
		output(&opts, &p);

	sqlite_schema_free(&p);
	return(rc ? EXIT_SUCCESS : EXIT_FAILURE);

usage:
	fprintf(stderr, "usage: %s [-v] file\n", getprogname());
	return(EXIT_FAILURE);
}
