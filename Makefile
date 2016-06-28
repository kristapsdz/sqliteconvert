CFLAGS	+= -W -Wall -g
PREFIX	?= /usr/local
BINS	 = sqlite2dot sqlite2html
MAN1S	 = sqlite2dot.1 sqlite2html.1
OBJS	 = dot.o html.o id.o parser.o
BINDIR	 = $(PREFIX)/bin
MAN1DI	 = $(PREFIX)/man/man1


all: $(BINS)

test: test.html

sqlite2dot: dot.o id.o parser.o
	$(CC) -o $@ dot.o id.o parser.o

sqlite2html: html.o id.o parser.o
	$(CC) -o $@ html.o id.o parser.o

install:
	mkdir -p $(BINDIR)
	mkdir -p $(MAN1DIR)
	install -m 0555 $(BINS) $(BINDIR)
	install -m 0444 $(MAN1S) $(MAN1DIR)

dot.o html.o id.o parser.o: extern.h

test.html: test-top.html test-bottom.html test.png sqlite2dot sqlite2html test.sql
	cat test-top.html >$@
	./sqlite2html test.sql >>$@
	./sqlite2dot test.sql | dot -Tcmapx >>$@
	cat test-bottom.html >>$@

test.png: sqlite2dot test.sql
	./sqlite2dot test.sql | dot -Tpng >$@

clean:
	rm -f $(BINS) $(OBJS)
	rm -rf sqlite2dot.dSYM sqlite2html.dSYM
	rm -f test.png test.html
