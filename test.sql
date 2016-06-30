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

-- sqliteconvert is a set of tools to extract documentation from SQLite
-- database schemas. Documentation, in this regard, consists of the
-- schema itself and comments prior to each table and comment.  What
-- you're reading right now is part of the comment above the table
-- called test't1.  (The funny name is demonstrate table names from
-- literals.)
/* This table references @test2 and @"test2". */
CREATE TABLE "tes't1" (
	-- To see the original SQL file, click on the ``SQL'' link in
	-- the header of this file.
	foo INTEGER NOT NULL,
	/* This is a foreign key. */
	-- By the way, here I have some ``fancy quotes''.
	test2id INTEGER NOT NULL,
	-- Some sort of comment---with a nice em-dash there.
	moo INTEGER NOT NULL,
	-- To see the source code, click on the GitHub icon.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	/* Note our foreign key... */
	FOREIGN KEY (test2id) REFERENCES test2(id),
	-- This should be discarded.
	unique (foo)
);

-- This other table has its comments in here.
-- It is also multi-line.
-- It's just used to flesh out the example.
CREATE TABLE test2 (
	-- This column references another table and column.
	-- You can also use foreign key constraints.
	bar INTEGER NOT NULL REFERENCES "tes't1"(foo),
	-- This is a primary key and shows escaping of special
	-- characters, for example, \\-character---and an \@-sign.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
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

CREATE TABLE test3 (
	-- Nam liber tempor cum soluta nobis eleifend option congue
	-- nihil imperdiet doming id quod mazim placerat facer possim
	-- assum. Typi non habent claritatem insitam; est usus legentis
	-- in iis qui facit eorum claritatem.
	col1 TEXT NOT NULL,
	-- Investigationes demonstraverunt lectores legere me lius quod
	-- ii legunt saepius.
	col2 TEXT NOT NULL REFERENCES test2(bar),
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
