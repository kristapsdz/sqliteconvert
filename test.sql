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

/* This table has its comments in here. */
/* It references @test2 and @"test2". */
CREATE TABLE "tes't1" (
	/* This is a foreign key. */
	test2id INTEGER NOT NULL,
	/* This is a unique value. */
	foo INTEGER NOT NULL,
	baz INTEGER NOT NULL,
	-- A primary key. 
	-- This is multiple lines long.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	/* Note our foreign key... */
	FOREIGN KEY (test2id) REFERENCES test2(id),
	-- This should be discarded.
	unique (foo)
);

-- This other table has its comments in here.
-- It is also multi-line.
CREATE TABLE test2 (
	/* This column does something. */
	bar INTEGER NOT NULL REFERENCES "tes't1"(baz),
	/* This is a primary key. \\-character---and an \@-sign. */
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);
