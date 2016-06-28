CFLAGS	+= -W -Wall -g
BINS	 = sqlite2dot sqlite2html
OBJS	 = dot.o html.o parser.o

all: $(BINS)

sqlite2dot: dot.o parser.o
	$(CC) -o $@ dot.o parser.o

sqlite2html: html.o parser.o
	$(CC) -o $@ html.o parser.o

dot.o html.o parser.o: extern.h

clean:
	rm -f $(BINS) $(OBJS)
	rm -rf sqlite2dot.dSYM sqlite2html.dSYM
