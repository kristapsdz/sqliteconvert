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
#ifndef EXTERN_H
#define EXTERN_H

struct	col {
	char		*name;
	char		*comment;
	struct tab	*tab;
	size_t		 idx;
	struct col	*fkey;
	TAILQ_ENTRY(col) entry;
};

TAILQ_HEAD(colq, col);

#define	TAB_TEMP	 0x01
#define	TAB_IF_NOT_EXIST 0x02

struct	tab {
	char		*name;
	char		*comment;
	size_t		 ncol;
	unsigned int	 flags;
	size_t		 idx;
	struct colq	 colq;
	TAILQ_ENTRY(tab) entry;
};

TAILQ_HEAD(tabq, tab);

struct	fkey {
	struct col	*col;
	char		*rtab;
	char		*rcol;
	TAILQ_ENTRY(fkey) entry;
};

TAILQ_HEAD(fkeyq, fkey);

struct	parse {
	const char	*map;
	size_t		 i;
	size_t		 len;
	size_t		 line;
	size_t		 col;
	size_t		 ntab;
	const char	*fname;
	struct tabq	 tabq;
	struct fkeyq	 fkeyq;
	int		 verbose;
};

__BEGIN_DECLS

void	 sqlite_schema_free(struct parse *);
char	*sqlite_schema_id(const char *, const char *);
char	*sqlite_schema_idbuf(const char *, size_t);
int	 sqlite_schema_parsebuf(const char *, const char *, size_t, struct parse *);
int	 sqlite_schema_parsefd(const char *, int, struct parse *);
int	 sqlite_schema_parsefile(const char *, struct parse *);
int	 sqlite_schema_parsestdin(struct parse *);

__END_DECLS

#endif /*!EXTERN_H*/
