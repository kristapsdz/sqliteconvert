.SUFFIXES: .1 .1.html

CFLAGS		+= -W -Wall -g
PREFIX		?= /usr/local
BINS		 = sqlite2dot sqlite2html sqliteconvert
MAN1S		 = sqlite2dot.1 sqlite2html.1 sqliteconvert.1
OBJS		 = dot.o html.o id.o parser.o
BINDIR		 = $(PREFIX)/bin
MAN1DIR		 = $(PREFIX)/man/man1
SHAREDIR	 = $(PREFIX)/share/sqliteconvert
WWWPREFIX	 = /var/www/vhosts/kristaps.bsd.lv/htdocs/sqliteconvert
HTMLS		 = index.html test.sql.html sqlite2dot.1.html sqlite2html.1.html sqliteconvert.1.html schema.html
PNGS		 = test.png schema.png
BUILT		 = imageMapResizer.min.js index.css mandoc.css test.sql

all: $(BINS) sqliteconvert.1

www: $(HTMLS) $(PNGS)

sqlite2dot: dot.o id.o parser.o
	$(CC) -o $@ dot.o id.o parser.o

sqlite2html: html.o id.o parser.o
	$(CC) -o $@ html.o id.o parser.o

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MAN1DIR)
	install -m 0555 $(BINS) $(DESTDIR)$(BINDIR)
	install -m 0444 $(MAN1S) $(DESTDIR)$(MAN1DIR)

installwww: www
	mkdir -p $(WWWPREFIX)
	install -m 0444 $(HTMLS) $(BUILT) $(PNGS) $(WWWPREFIX)

dot.o html.o id.o parser.o: extern.h

index.html: test.sql index.xml sqliteconvert
	sh sqliteconvert -f index.xml test.sql >$@

test.png: test.sql sqliteconvert
	sh sqliteconvert -i test.sql >$@

test.sql.html: test.sql
	highlight -I -l test.sql > $@

.1.1.html:
	mandoc -Thtml -Ostyle=mandoc.css $< >$@

schema.html: schema.sql schema.xml sqliteconvert
	sh sqliteconvert -f schema.xml schema.sql >$@

schema.png: schema.sql schema.xml sqliteconvert
	sh sqliteconvert -i schema.sql >$@

sqliteconvert: sqliteconvert.sh sqlite2dot sqlite2html
	sed "s!@SHAREDIR@!$(SHAREDIR)!g" sqliteconvert.sh >$@

sqliteconvert.1: sqliteconvert.in.1
	sed "s!@SHAREDIR@!$(SHAREDIR)!g" sqliteconvert.in.1 >$@

clean:
	rm -f $(BINS) $(OBJS) $(HTMLS) $(PNGS) sqliteconvert.1
	rm -rf sqlite2dot.dSYM sqlite2html.dSYM
