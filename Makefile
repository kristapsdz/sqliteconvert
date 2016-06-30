.SUFFIXES: .1 .1.html

CFLAGS		+= -W -Wall -g
PREFIX		?= /usr/local
BINS		 = sqlite2dot sqlite2html
MAN1S		 = sqlite2dot.1 sqlite2html.1
OBJS		 = dot.o html.o id.o parser.o
BINDIR		 = $(PREFIX)/bin
MAN1DIR		 = $(PREFIX)/man/man1
DOTFLAGS	 = -h "BGCOLOR=\"red\"" \
		   -t "CELLBORDER=\"0\"" \
		   -t "CELLSPACING=\"0\""
WWWPREFIX	 = /var/www/vhosts/kristaps.bsd.lv/htdocs/sqliteconvert
HTMLS		 = index.html test.sql.html sqlite2dot.1.html sqlite2html.1.html
BUILT		 = imageMapResizer.min.js index.css mandoc.css test.sql

all: $(BINS)

www: $(HTMLS) test.png

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
	install -m 0444 $(HTMLS) $(BUILT) test.png $(WWWPREFIX)

dot.o html.o id.o parser.o: extern.h

index.html: index.xml test.png sqlite2dot sqlite2html test.sql
	( sed -n '1,/@SCHEMA@/p' index.xml ; \
	  ./sqlite2html test.sql ; \
	  ./sqlite2dot $(DOTFLAGS) test.sql | dot -Tcmapx ; \
	  sed -n '/@SCHEMA@/,$$p' index.xml ; ) >$@

test.png: sqlite2dot test.sql
	./sqlite2dot $(DOTFLAGS) test.sql | dot -Tpng >$@

test.sql.html: test.sql
	highlight -I -l test.sql > $@

.1.1.html:
	mandoc -Thtml -Ostyle=mandoc.css $< >$@

clean:
	rm -f $(BINS) $(OBJS)
	rm -rf sqlite2dot.dSYM sqlite2html.dSYM
	rm -f test.png $(HTMLS)
