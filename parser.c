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
#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

enum	tokent {
	TOK_EOF,
	TOK_COMMENT,
	TOK_LITERAL,
	TOK_IDENT
};

struct	token {
	const char	*start;
	size_t		 sz;
	int		 eof;
	enum tokent	 type;
};

static	void dowarnx(const struct parse *, const char *, ...)
	__attribute__((format(printf, 2, 3)));
static	void dogwarnx(const struct parse *, const char *, ...)
	__attribute__((format(printf, 2, 3)));
static	void domsg(const struct parse *, const char *, ...)
	__attribute__((format(printf, 2, 3)));

/*
 * Emit some debugging information.
 * This will only work if we're in verbose mode.
 */
static void
domsg(const struct parse *p, const char *fmt, ...)
{
	va_list	 ap;

	if ( ! p->verbose)
		return;
	fprintf(stderr, "%s:%zu:%zu: ", 
		p->fname, p->line + 1, p->col);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

/*
 * Equivalent to warnx(3) but also showing our file.
 */
static void
dogwarnx(const struct parse *p, const char *fmt, ...)
{
	va_list	 ap;

	fprintf(stderr, "%s: ", p->fname);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

/*
 * Equivalent to warnx(3) but also showing our file, line number, and
 * current column number.
 */
static void
dowarnx(const struct parse *p, const char *fmt, ...)
{
	va_list	 ap;

	fprintf(stderr, "%s:%zu:%zu: ", 
		p->fname, p->line + 1, p->col);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

static size_t
tok_nextchar(struct parse *p, size_t n)
{
	size_t	 j;

	for (j = 0; j < n && p->i < p->len; j++, p->i++, p->col++)
		if ('\n' == p->map[p->i]) {
			p->line++;
			p->col = 0;
		}

	return(j);
}

static void
tok_init(struct token *tok, struct parse *p)
{

	tok->sz = 0;
	tok->start = &p->map[p->i];
}

/*
 * Skip white-space in input buffer.
 * This can go all the way to the eof.
 */
static void
tok_skipws(struct parse *p)
{

	while (p->i < p->len && isspace((int)p->map[p->i])) 
		tok_nextchar(p, 1);
}

/*
 * Returns zero on EOF, non-zero on next word.
 * If "eofok" is non-zero, reports the end-of-file, if found.
 * FIXME: this will get a lot of work.
 */
static int
tok_next(struct token *tok, struct parse *p, int eofok)
{
	int	 quot;

	memset(tok, 0, sizeof(struct token));
	tok->type = TOK_EOF;

	tok_skipws(p);
	if (p->i == p->len) {
		tok->eof = 1;
		if ( ! eofok)
			dowarnx(p, "unexpected eof");
		return(0);
	}


	if ('/' == p->map[p->i] && p->i < p->len - 2 && 
	    '*' == p->map[p->i + 1]) {
		/* 
		 * Are we in a multi-line comment?
		 * If so, read until we hit the end of comment.
		 * Suck down everything but the leading white-space.
		 */
		tok_nextchar(p, 2);
		tok_skipws(p);
		tok_init(tok, p);
		while (p->i < p->len - 2) {
			if ('*' == p->map[p->i] &&
			    '/' == p->map[p->i + 1])
				break;
			tok_nextchar(p, 1);
			tok->sz++;
		}
		if (p->i == p->len - 2) {
			tok->eof = 1;
			if ( ! eofok)
				dowarnx(p, "unexpected eof");
			return(0);
		}
		tok_nextchar(p, 2);
		tok->type = TOK_COMMENT;
		return(1);
	} else if ('-' == p->map[p->i] && p->i < p->len - 2 && 
	           '-' == p->map[p->i + 1]) {
		/*
		 * Are we in a single-line comment?
		 * If so, read only the current line.
		 */
		tok_nextchar(p, 2);
		tok_init(tok, p);
		while (p->i < p->len && '\n' != p->map[p->i])  {
			tok_nextchar(p, 1);
			tok->sz++;
		}
		if (p->i == p->len) {
			tok->eof = 1;
			if ( ! eofok)
				dowarnx(p, "unexpected eof");
			return(0);
		}
		tok_nextchar(p, 1);
		tok->type = TOK_COMMENT;
		return(1);
	} else if (('"' == p->map[p->i] || '\'' == p->map[p->i]) && 
		   p->i && '\\' != p->map[p->i - 1]) {
		/*
		 * Are we quoting?
		 */
		quot = p->map[p->i];
		tok_nextchar(p, 1);
		tok_init(tok, p);
		while (p->i < p->len && quot != p->map[p->i] && 
		       '\\' != p->map[p->i - 1]) {
			tok_nextchar(p, 1);
			tok->sz++;
		}
		if (p->i == p->len) {
			tok->eof = 1;
			if ( ! eofok)
				dowarnx(p, "unexpected eof");
			return(0);
		}
		tok_nextchar(p, 1);
		tok->type = TOK_LITERAL;
		return(1);
	}

	tok_init(tok, p);
	tok_nextchar(p, 1);
	tok->sz++;
	tok->type = TOK_IDENT;

	if ('(' == p->map[p->i - 1] ||
	    ',' == p->map[p->i - 1] ||
	    ')' == p->map[p->i - 1] ||
	    ';' == p->map[p->i - 1])
		return(1);

	while (p->i < p->len && ! isspace((int)p->map[p->i])) {
		if ('(' == p->map[p->i] ||
		    ',' == p->map[p->i] ||
		    ')' == p->map[p->i] ||
		    ';' == p->map[p->i])
			break;
		tok_nextchar(p, 1);
		tok->sz++;
	}

	return(1);
}

static int
tok_strsame(const struct token *tok, const char *str)
{

	if (0 == tok->sz)
		return(0);
	return(0 == strncasecmp(str, tok->start, tok->sz));
}

static void
tok_skipstmt(struct parse *p)
{
	struct token	 tok;

	do {
		do if ( ! tok_next(&tok, p, 0)) 
			return;
		while (TOK_COMMENT == tok.type);
	} while ( ! tok_strsame(&tok, ";"));
}

static int
tok_nextsame(struct token *tok, struct parse *p, 
	const char *str, int eofok)
{

	do if ( ! tok_next(tok, p, eofok)) 
		return(-1);
	while (TOK_COMMENT == tok->type);

	if (TOK_LITERAL == tok->type)
		return(0);
	return(tok_strsame(tok, str));
}

static int
tok_nextexpect(struct token *tok, struct parse *p, const char *str)
{
	int	 c;

	if ((c = tok_nextsame(tok, p, str, 0)) > 0)
		return(1);
	else if (c < 0)
		return(0);

	dowarnx(p, "unexpected: %.*s (wanted \"%s\")", 
		(int)tok->sz, tok->start, str);
	return(0);
}

static int
comment_append(struct token *tok, struct parse *p, 
	int eofok, char **outp)
{
	char	*comment;
	size_t	 sz;

	comment = NULL;
	sz = 0;

	for (;;) {
		if ( ! tok_next(tok, p, eofok)) {
			free(comment);
			return(0);
		} else if (TOK_COMMENT != tok->type)
			break;

		if (NULL == comment) {
			comment = strndup(tok->start, tok->sz);
			if (NULL == comment)
				err(EXIT_FAILURE, "strndup");
			sz = strlen(comment);
		} else {
			comment = realloc(comment, sz + tok->sz + 1);
			if (NULL == comment)
				err(EXIT_FAILURE, "realloc");
			memcpy(comment + sz, tok->start, tok->sz);
			sz += tok->sz;
			comment[sz] = '\0';
		}
	}

	*outp = comment;
	return(1);
}

/*
 * Process what comes after "references" within a column declaration.
 * Return zero on failure and non-zero on success.
 */
static int
schema_column_references(struct token *tok, 
	struct parse *p, struct col *col)
{
	struct fkey	*fkey;

	do if ( ! tok_next(tok, p, 0))
		return(0);
	while (TOK_COMMENT == tok->type);

	fkey = calloc(1, sizeof(struct fkey));
	if (NULL == fkey)
		err(EXIT_FAILURE, "calloc");
	fkey->col = col;
	fkey->rtab = strndup(tok->start, tok->sz);
	if (NULL == fkey->rtab)
		err(EXIT_FAILURE, "strndup");
	TAILQ_INSERT_TAIL(&p->fkeyq, fkey, entry);

	if ( ! tok_nextexpect(tok, p, "("))
		return(0);
	do if ( ! tok_next(tok, p, 0))
		return(0);
	while (TOK_COMMENT == tok->type);

	fkey->rcol = strndup(tok->start, tok->sz);
	if (NULL == fkey->rcol)
		err(EXIT_FAILURE, "strndup");

	domsg(p, "added reference to %s.%s: %s.%s",
		col->tab->name, col->name,
		fkey->rtab, fkey->rcol);
	return(tok_nextexpect(tok, p, ")"));
}

/*
 * Process what follows "foreign" as a column constraint, e.g., "foreign
 * key (moop) references foo(bar)".
 * Return zero on failure and non-zero on success.
 */
static int
schema_foreign(struct token *tok, struct parse *p, struct tab *tab)
{
	struct col	*tcol;
	struct fkey	*fkey = NULL;

	if ( ! tok_nextexpect(tok, p, "key"))
		return(0);
	if ( ! tok_nextexpect(tok, p, "("))
		return(0);

	/* Get and look up column name. */

	do if ( ! tok_next(tok, p, 0))
		return(0);
	while (TOK_COMMENT == tok->type);

	TAILQ_FOREACH(tcol, &tab->colq, entry)
		if ( ! strncmp(tcol->name, tok->start, tok->sz))
			break;

	if (NULL != tcol) {
		fkey = calloc(1, sizeof(struct fkey));
		if (NULL == fkey)
			err(EXIT_FAILURE, "calloc");
		fkey->col = tcol;
		TAILQ_INSERT_TAIL(&p->fkeyq, fkey, entry);
	} else
		dowarnx(p, "cannot find column: %.*s",
			(int)tok->sz, tok->start);

	if ( ! tok_nextexpect(tok, p, ")"))
		return(0);
	if ( ! tok_nextexpect(tok, p, "references"))
		return(0);

	/* Table name: keep if we have our column. */

	do if ( ! tok_next(tok, p, 0)) 
		return(0);
	while (TOK_COMMENT == tok->type);

	if (NULL != fkey) {
		fkey->rtab = strndup(tok->start, tok->sz);
		if (NULL == fkey->rtab)
			err(EXIT_FAILURE, "strndup");
	}

	if ( ! tok_nextexpect(tok, p, "("))
		return(0);

	/* Column name: keep if we have our column. */

	do if ( ! tok_next(tok, p, 0))
		return(0);
	while (TOK_COMMENT == tok->type);

	if (NULL != fkey) {
		fkey->rcol = strndup(tok->start, tok->sz);
		if (NULL == fkey->rcol)
			err(EXIT_FAILURE, "strndup");
	}

	if (NULL != tcol)
		domsg(p, "added foreign key to %s.%s: %s.%s",
			tcol->tab->name, tcol->name,
			fkey->rtab, fkey->rcol);
	return(tok_nextexpect(tok, p, ")"));
}

/*
 * Returns <0 on failure, 0 if no more columns, 1 if more columns.
 */
static int
schema_column(struct token *tok, struct parse *p, struct tab *tab)
{
	size_t	 	 nest;
	struct col	*col, *ncol;
	char		*comment;

	if ( ! comment_append(tok, p, 0, &comment))
		return(-1);

	/* Check sub-clauses. */

	if ( ! tok_strsame(tok, "unique") &&
	     ! tok_strsame(tok, "foreign")) {
		col = calloc(1, sizeof(struct col));
		if (NULL == col)
			err(EXIT_FAILURE, "calloc");
		col->name = strndup(tok->start, tok->sz);
		if (NULL == col->name)
			err(EXIT_FAILURE, "strndup");
		col->tab = tab;
		col->idx = tab->ncol++;
		col->comment = comment;
		TAILQ_FOREACH(ncol, &tab->colq, entry)
			if (strcmp(ncol->name, col->name) > 0)
				break;
		if (NULL != ncol)
			TAILQ_INSERT_BEFORE(ncol, col, entry);
		else
			TAILQ_INSERT_TAIL(&tab->colq, col, entry);
		domsg(p, "added column: %s.%s", 
			col->tab->name, col->name);
	} else {
		free(comment);
		col = NULL;
	}

	if (tok_strsame(tok, "foreign"))
		if ( ! schema_foreign(tok, p, tab))
			return(0);

	for (nest = 1; nest > 0; ) {
		do if ( ! tok_next(tok, p, 0))
			return(-1);
		while (TOK_COMMENT == tok->type);

		if (NULL != col && tok_strsame(tok, "references")) {
			if ( ! schema_column_references(tok, p, col))
				return(0);
			continue;
		}
		if (tok_strsame(tok, "("))
			nest++;
		else if (tok_strsame(tok, ",") && 1 == nest)
			break;
		else if (tok_strsame(tok, ")"))
			nest--;
	}

	if (tok_strsame(tok, ","))
		return(1);
	if (tok_strsame(tok, ")"))
		return(0);

	dowarnx(p, "syntax error trailing column");
	return(-1);
}

/*
 * Parse a table definition.
 * Returns zero on failure, non-zero on success.
 */
static int
schema_table(struct token *tok, struct parse *p, 
	char **comment, unsigned int flags)
{
	int	 	 c;
	struct tab	*tab, *ntab;

	/* Start trying to get the table identifier. */

	do if ( ! tok_next(tok, p, 0))
		return(0);
	while (TOK_COMMENT == tok->type);

	if (tok_strsame(tok, "if")) {
		if ( ! tok_nextsame(tok, p, "not", 0))
			return(0);
		if ( ! tok_nextsame(tok, p, "exists", 0))
			return(0);
		do if ( ! tok_next(tok, p, 0))
			return(0);
		while (TOK_COMMENT == tok->type);
		flags |= TAB_IF_NOT_EXIST;
	}

	/* Allocate table in queue. */

	tab = calloc(1, sizeof(struct tab));
	if (NULL == tab)
		err(EXIT_FAILURE, "calloc");
	tab->name = strndup(tok->start, tok->sz);
	if (NULL == tab->name)
		err(EXIT_FAILURE, "strndup");
	tab->idx = p->ntab++;
	tab->comment = *comment;
	*comment = NULL;
	tab->flags = flags;
	TAILQ_INIT(&tab->colq);

	TAILQ_FOREACH(ntab, &p->tabq, entry)
		if (strcmp(ntab->name, tab->name) > 0)
			break;
	if (NULL != ntab)
		TAILQ_INSERT_BEFORE(ntab, tab, entry);
	else
		TAILQ_INSERT_TAIL(&p->tabq, tab, entry);

	domsg(p, "added table: %s", tab->name);

	/* Parse through all of our columns. */

	if (0 == (c = tok_nextsame(tok, p, "(", 0))) {
		dowarnx(p, "syntax error leading columns");
		return(0);
	} else if (c < 0)
		return(0);

	while ((c = schema_column(tok, p, tab)) > 0) 
		continue;
	if (c < 0)
		return(0);

	if ( ! tok_strsame(tok, ")")) {
		dowarnx(p, "syntax error trailing columns");
		return(0);
	}

	/* 
	 * See if we're at the end of the table statement, which can
	 * also have a "without rowid" clause.
	 */

	do if ( ! tok_next(tok, p, 0))
		return(0);
	while (TOK_COMMENT == tok->type);

	if (tok_strsame(tok, "without")) {
		if (0 == (c = tok_nextsame(tok, p, "rowid", 0))) {
			dowarnx(p, "syntax error leading columns");
			return(0);
		} else if (c < 0)
			return(0);
		do if ( ! tok_next(tok, p, 0))
			return(0);
		while (TOK_COMMENT == tok->type);
	}

	if (tok_strsame(tok, ";"))
		return(1);

	dowarnx(p, "syntax error at end of table statement");
	return(0);
}

/* 
 * Processes a "create xxxx" statement.
 * Returns zero on failure, <0 on end of file, or >0 at the end of the
 * creation.
 * This should be at a semicolon.
 */
static int
schema_create(struct token *tok, struct parse *p, char **comment)
{
	unsigned int	 flags = 0;

	do if ( ! tok_next(tok, p, 0))
		return(-1);
	while (TOK_COMMENT == tok->type);

	/* Pass over "temp" and "temporary" statements. */

	if (tok_strsame(tok, "temp") ||
	    tok_strsame(tok, "temporary")) {
		flags = TAB_TEMP;
		do if ( ! tok_next(tok, p, 0))
			return(-1);
		while (TOK_COMMENT == tok->type);
	}

	/* Try to read the "table", if it exists. */

	if ( ! tok_strsame(tok, "table")) {
		dowarnx(p, "ignoring non-table creation");
		return(0);
	} 

	return(schema_table(tok, p, comment, flags) ? 1 : -1);
}

/*
 * Cross-reference foreign key entries.
 * Skips all non-existent references.
 */
static void
foreign_keys(struct parse *p)
{
	struct tab	*tab;
	struct col	*col;
	struct fkey	*fkey;

	TAILQ_FOREACH(fkey, &p->fkeyq, entry) {
		if (NULL == fkey->col)
			continue;
		TAILQ_FOREACH(tab, &p->tabq, entry)
			if (0 == strcmp(tab->name, fkey->rtab))
				break;
		if (NULL == tab) {
			dogwarnx(p, "unknown foreign key "
				"table on %s.%s: %s.%s", 
				fkey->col->tab->name, 
				fkey->col->name, 
				fkey->rtab, fkey->rcol);
			continue;
		}
		TAILQ_FOREACH(col, &tab->colq, entry)
			if (0 == strcmp(col->name, fkey->rcol))
				break;
		if (NULL == tab) {
			dogwarnx(p, "unknown foreign key "
				"column on %s.%s: %s.%s", 
				fkey->col->tab->name,
				fkey->col->name, 
				fkey->rtab, fkey->rcol);
			continue;
		}
		if (NULL != fkey->col->fkey) {
			dogwarnx(p, "foreign key exists: %s.%s", 
				fkey->col->fkey->tab->name, 
				fkey->col->fkey->name);
			continue;
		}
		fkey->col->fkey = col;
	}
}

void
sqlite_schema_free(struct parse *p)
{
	struct fkey	*fkey;
	struct col	*col;
	struct tab	*tab;

	while (NULL != (fkey = TAILQ_FIRST(&p->fkeyq))) {
		TAILQ_REMOVE(&p->fkeyq, fkey, entry);
		free(fkey->rtab);
		free(fkey->rcol);
		free(fkey);
	}

	while (NULL != (tab = TAILQ_FIRST(&p->tabq))) {
		TAILQ_REMOVE(&p->tabq, tab, entry);
		while (NULL != (col = TAILQ_FIRST(&tab->colq))) {
			TAILQ_REMOVE(&tab->colq, col, entry);
			free(col->name);
			free(col->comment);
			free(col);
		}
		free(tab->name);
		free(tab->comment);
		free(tab);
	}
}

int
sqlite_schema_parsefile(const char *fname, struct parse *p) 
{
	int	 	 fd, rc;

	if (-1 == (fd = open(fname, O_RDONLY, 0))) {
		warn("%s", fname);
		return(0);
	}

	rc = sqlite_schema_parsefd(fname, fd, p);
	if (-1 == close(fd)) {
		warn("%s", fname);
		rc = 0;
	}

	return(rc);
}

int
sqlite_schema_parsefd(const char *fname, int fd, struct parse *p) 
{
	int	 	 rc;
	void		*map;
	struct stat	 st;

	if (-1 == fstat(fd, &st)) {
		warn("%s", fname);
		return(0);
	} 
	
	map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (MAP_FAILED == map) {
		warn("%s", fname);
		return(0);
	}

	rc = sqlite_schema_parsebuf(fname, map, st.st_size, p);
	if (-1 == munmap(map, st.st_size)) {
		warn("%s", fname);
		rc = 0;
	}

	return(rc);
}

int
sqlite_schema_parsestdin(struct parse *p) 
{
	char	 sbuf[BUFSIZ];
	char	*buf;
	size_t	 bufsz;
	ssize_t	 ssz;
	int	 rc;

	for (rc = 0, buf = NULL, bufsz = 0; ; bufsz += ssz) {
		ssz = read(STDIN_FILENO, sbuf, sizeof(sbuf));
		if (ssz < 0) {
			warn("<stdin>");
			break;
		} else if (0 == ssz) {
			rc = sqlite_schema_parsebuf
				("<stdin>", buf, bufsz, p);
			break;
		}
		buf = realloc(buf, bufsz + ssz);
		if (NULL == buf)
			err(EXIT_FAILURE, "realloc");
		memcpy(buf + bufsz, sbuf, ssz);
	}

	free(buf);
	return(rc);
}

int
sqlite_schema_parsebuf(const char *fname, 
	const char *map, size_t mapsz, struct parse *p) 
{
	int	 	 rc, c;
	struct token	 tok;
	char		*comment;

	if (0 == mapsz) {
		warnx("%s: empty file", fname);
		return(0);
	}

	/* Initialise relevant parts of the parser. */

	TAILQ_INIT(&p->tabq);
	TAILQ_INIT(&p->fkeyq);
	p->map = map;
	p->i = p->line = p->col = p->ntab = 0;
	p->len = mapsz;
	p->fname = fname;
	
	/*
	 * Top-level of parse.
	 * Look for a statement that begins with "create", which will
	 * indicate that we may have a table.
	 * (It might start with a comment, which we should keep track of
	 * to pump into any new table.)
	 * Ignore all other statements by continuing til the semicolon.
	 */

	comment = NULL;
	rc = 0;

	while (p->i < p->len) {
		free(comment);
		if ( ! comment_append(&tok, p, 1, &comment)) {
			rc = 1;
			break;
		} else if ( ! tok_strsame(&tok, "create")) {
			domsg(p, "ignoring top-level statement");
			tok_skipstmt(p);
			continue;
		} 
		
		if (0 == (c = schema_create(&tok, p, &comment))) {
			tok_skipstmt(p);
			continue;
		} else if (c < 0)
			break;

		if (tok_strsame(&tok, ";"))
			continue;
		dowarnx(p, "bad token at end of statement");
		break;
	}

	free(comment);

	/* On success, compute foreign keys. */

	if (1 == rc)
		foreign_keys(p);

	return(rc);
}
