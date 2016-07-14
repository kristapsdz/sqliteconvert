/*	$Id$ */
/*
 * Copyright (c) 2016 Kristaps Dzonsons <kristaps@kcons.eu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

PRAGMA journal_mode=WAL;

-- sqliteconvert is a set of tools to extract documentation from
-- [SQLite](https://www.sqlite.org)
-- database schemas. Documentation, in this regard, consists of the
-- schema itself and comments prior to each table and comment.  What
-- you're reading right now is part of the comment above the table
-- called ``INTRODUCTION''.
-- The result of this extraction is this HTML5 page.
-- It consists of an image showing the schema, the documentation, and an
-- image map linking the two.
-- All of these are contained on this page.
CREATE TABLE "INTRODUCTION" (
	-- The tools generating this page are
	-- [sqliteconvert(1)](sqliteconvert.1.html), 
	-- [sqlite2dot(1)](sqlite2dot.1.html), and
	-- [sqlite2html(1)](sqlite2html.1.html).
	-- The first, [sqliteconvert(1)](sqliteconvert.1.html), is
	-- simply a shell script that pulls together the latter two.
	"1. Tools" INTEGER NOT NULL,
	-- The file generating this text is [test.sql.html].
	-- You can see how I generate the links (Markdown-style) from
	-- the comments.
	"2. Input" INTEGER NOT NULL,
	-- To see the source code, head to
	-- [github.com/kristapsdz/sqliteconvert](https://github.com/kristapsdz/sqliteconvert)
	-- and check it out yourself.
	-- It consists of the (minimal) SQLite parser and its output
	-- modes.
	"3. Source Code" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	-- To see a larger example, visit [schema.html].
	-- This is created from the
	-- [Gamelab](http://www.kcons.eu/gamelab) schema by using the
	-- default 
	-- [sqliteconvert(1)](sqliteconvert.1.html) invocation.
	"4. Larger Example" TEXT,
	/* Note our foreign key... */
	FOREIGN KEY ("2. Input") REFERENCES "SYNTAX"("1. Comments"),
	-- This should be discarded.
	unique ("1. Tools")
);

/*
 * In this ``table'', I show how to document your SQLite schema so that
 * it shows up nicely---just like this here.
 * You can read about it in detail in
 * [sqlite2html(1)](sqlite2html.1.html).

 * On the other hand, you can just infer all this from [test.sql.html].
 */
CREATE TABLE SYNTAX (
	-- The comments themselves can be in SQLite single-line format
	-- (two dashes), multi-line (slash-asterisk until another
	-- slash-asterisk), or fancy multi-line (slash-asterisk until
	-- another slash-asterisk, but ignoring ``*'' at the start of
	-- individual lines).
	"1. Comments" INTEGER NOT NULL,
	-- sqliteconvert recognises ``CREATE TABLE'' statements and the
	-- columns described by that, including foreign key references
	-- and unique constraints.
	-- At this time, the unique constraints are not used in any way.
	"2. Schema" REAL NOT NULL,
	-- Beyond the schema itself and the comments, you can have
	-- generic links and link to tables/columns.
 	-- Quotes and dashes are also nice-ified.
	"3. Specials" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);

-- Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam
-- nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat
-- volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation
-- ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo
-- consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate
-- velit esse molestie consequat, vel illum dolore eu feugiat nulla
-- facilisis at vero eros et accumsan et iusto odio dignissim qui
-- blandit praesent luptatum zzril delenit augue duis dolore te feugait
-- nulla facilisi.

CREATE TABLE Lorem (
	-- Nam liber tempor cum soluta nobis eleifend option congue
	-- nihil imperdiet doming id quod mazim placerat facer possim
	-- assum. Typi non habent claritatem insitam; est usus legentis
	-- in iis qui facit eorum claritatem.
	col1 TEXT NOT NULL,
	-- Investigationes demonstraverunt lectores legere me lius quod
	-- ii legunt saepius.
	col2 TEXT NOT NULL REFERENCES SYNTAX("1. Comments"),
	-- Claritas est etiam processus dynamicus, qui sequitur
	-- mutationem consuetudium lectorum. Mirum est notare quam
	-- littera gothica, quam nunc putamus parum claram, anteposuerit
	-- litterarum formas humanitatis per seacula quarta decima et
	-- quinta decima.
	col3 INTEGER NOT NULL DEFAULT(0),
	-- Eodem modo typi, qui nunc nobis videntur parum clari, fiant
	-- sollemnes in futurum.
	col4 INTEGER NOT NULL,
	-- And lastly, here's the primary key.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);
