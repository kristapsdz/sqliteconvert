#include <sys/queue.h>

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

static void
safe_fmtstring(char *p)
{

	for ( ; '\0' != *p; p++) {
		if (isalnum((int)*p) || '-' == *p ||
		    '_' == *p || '.' == *p)
			continue;
		*p = '_';
	}
}

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
			if (isspace((int)*p))
				putchar(' ');
			else
				putchar(*p);
			break;
		}
}

static void
output(struct parse *p)
{
	struct tab	*tab;
	struct col	*col;
	char		*cp;
	int		 rc;

	puts("<dl class=\"tabs\">");
	TAILQ_FOREACH(tab, &p->tabq, entry) {
		if (NULL == (cp = strdup(tab->name)))
			err(EXIT_FAILURE, "strdup");
		safe_fmtstring(cp);
		printf("\t<dt id=\"tab-%s\">", cp);
		free(cp);
		safe_putstring(tab->name);
		puts("</dt>");
		puts("\t<dd>");
		if (NULL != tab->comment) {
			puts("\t\t<div class=\"comment\">");
			fputs("\t\t\t", stdout);
			safe_putstring(tab->comment);
			puts("\n\t\t</div>");
		}
		puts("\t\t<dl class=\"cols\">");
		TAILQ_FOREACH(col, &tab->colq, entry) {
			rc = asprintf(&cp, "%s.%s",
				col->tab->name, col->name);
			if (rc < 0)
				err(EXIT_FAILURE, "asprintf");
			safe_fmtstring(cp);
			printf("\t\t\t<dt id=\"col-%s\">", cp);
			free(cp);
			safe_putstring(col->name);
			puts("</dt>");
			puts("\t\t\t<dd>");
			if (NULL != col->comment) {
				puts("\t\t\t\t<div class=\"comment\">");
				fputs("\t\t\t\t\t", stdout);
				safe_putstring(col->comment);
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

	memset(&p, 0, sizeof(struct parse));
	rc = 0;

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

	if (1 != argc)
		goto usage;

	if ((rc = sqlite_schema_parse(argv[0], &p)) > 0)
		output(&p);

	sqlite_schema_free(&p);
	return(rc ? EXIT_SUCCESS : EXIT_FAILURE);

usage:
	fprintf(stderr, "usage: %s [-v] file\n", getprogname());
	return(EXIT_FAILURE);
}
