CFLAGS	+= -W -Wall -g
BINS	 = sqlite2dot sqlite2html
OBJS	 = dot.o html.o parser.o

all: $(BINS)

sqlite2dot: dot.o parser.o
	$(CC) -o $@ dot.o parser.o

sqlite2html: html.o parser.o
	$(CC) -o $@ html.o parser.o

dot.o html.o parser.o: extern.h

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
