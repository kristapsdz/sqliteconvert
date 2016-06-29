## Synopsis

sqliteconvert is a set of tools to convert
[sqlite3](https://www.sqlite.org/) schema files into documentation.
It includes sqlite2dot, which converts into a
[graphviz](http://www.graphviz.org) file; and sqlite2html, which
converts into an HTML5 fragment.

These tools are designed to be used together: sqlite2dot first converts
(via [dot](http://graphviz.org/pdf/dotguide.pdf)) into a PNG (-Tpng),
then into a client-side image map (-Tcmapx).  These are referenced from
an HTML5 page, which also includes the output of sqlite2html.  Readers
can then click on columns in the graphical output.

**These tools are still under development**.

## Installation

Compile with `make`, then `sudo make install` (or `doas make install`,
whatever the case may be).
There are no dependencies.

## License

All sources use the ISC (like OpenBSD) license.
See the [LICENSE.md](LICENSE.md) file for details.
