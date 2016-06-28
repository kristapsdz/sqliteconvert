#ifndef SQLITE_SCHEMA_H
#define SQLITE_SCHEMA_H

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

int
sqlite_schema_parse(const char *fname, struct parse *p);

void
sqlite_schema_free(struct parse *p);

__END_DECLS

#endif
