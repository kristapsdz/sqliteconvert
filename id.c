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

#include <err.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

char *
sqlite_schema_id(const char *tab, const char *col)
{
	char	*p, *op;
	int	 rc;

	if (NULL != col) {
		rc = asprintf(&p, "%s.%s", tab, col);
		if (-1 == rc)
			err(EXIT_FAILURE, "asprintf");
	} else {
		p = strdup(tab);
		if (NULL == p)
			err(EXIT_FAILURE, "strdup");
	}

	for (op = p; '\0' != *p; p++) {
		if (isalnum((int)*p) || '-' == *p ||
		    '_' == *p || '.' == *p)
			continue;
		*p = '_';
	}

	return(op);
}
