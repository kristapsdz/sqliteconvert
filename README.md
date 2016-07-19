## Synopsis

sqliteconvert is a set of tools to convert
[sqlite3](https://www.sqlite.org/) schema files into documentation.
It includes
[sqlite2dot(1)](https://kristaps.bsd.lv/sqliteconvert/sqlite2dot.1.html),
which converts into a [graphviz](http://www.graphviz.org) file;
[sqlite2html(1)](https://kristaps.bsd.lv/sqliteconvert/sqlite2html.1.html),
which converts into an HTML5 fragment; and
[sqliteconvert(1)](https://kristaps.bsd.lv/sqliteconvert/sqliteconvert.1.html),
which pulls these tools together with some sane default templates.

For a full-fledged example, see
[schema.html](https://kristaps.bsd.lv/sqliteconvert/schema.html).

## Installation

Compile with `make`, then `sudo make install` (or `doas make install`,
whatever the case may be).
There are no dependencies.

## License

All sources use the ISC (like OpenBSD) license.
See the [LICENSE.md](LICENSE.md) file for details.
