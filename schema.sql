/*	$Id$ */
/*
 * Copyright (c) 2014--2016 Kristaps Dzonsons <kristaps@kcons.eu>
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

-- The winner table consists of rows corresponding to a player and her
-- winner status. This table does not exist until the @experiment.state
-- field is set to 3. This table is deleted when the database is wiped.

CREATE TABLE winner (
	-- Participant.
	playerid INTEGER REFERENCES player(id) NOT NULL,
	-- Boolean value as to whether the player is a winner. If this
	-- is false, then the @winner.winrank and @winner.rnum columns
	-- are undefined.
	winner BOOLEAN NOT NULL,
	-- If the player is a winner, the rank (first, second, third
	-- draw...) of their winning.
	winrank INTEGER NOT NULL,
	-- The random number modulo the total number of tickts used for
	-- this winning draw. In other words, this is the winning
	-- lottery ticket. 
	rnum INTEGER NOT NULL,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	unique (playerid)
);

-- The central player table consists of all participants. We use the
-- term ``participant'' instead of player in the literature to
-- disambiguate a player role and an experiment subject.  The
-- @player.hash variable is redacted when the experiment is wiped. 

CREATE TABLE player (
	-- Player e-mail address or identifier, in the event of a
	-- captive participant. This is unique in the set of all
	-- participants.
	email TEXT NOT NULL,
	-- If the participant joined from Mechanical Turk, the HIT
	-- identifier. Otherwise set to the empty string. 
	hitid TEXT NOT NULL,
	-- If the participant joined from Mechanical Turk, the
	-- assignment identifier. Otherwise set to the empty string.
	assignmentid TEXT NOT NULL,
	-- Whether the Mechanical Turk participant, if applicable, has
	-- indicated that they've finished the experiment. Note that
	-- this is unprotected and can be set at any time: it is only
	-- used to display a message to report to the Mechanical Turk
	-- server.
	mturkdone INTEGER NOT NULL DEFAULT(0),
	-- The state of a participant can be 0, meaning the participant
	-- is newly-added and has no password; 1 when the participant
	-- has been mailed her password; 2, the participant has logged
	-- in; and 3, an error occured when the password e-mail was
	-- attempted.
	state INTEGER NOT NULL DEFAULT(0),
	-- Whether a participant is allowed to login during an experiment.
	enabled INTEGER NOT NULL DEFAULT(1),
	-- A cryptographically random number >0 given to the participant
	-- when created. This is used for many purposes, one of which
	-- being the bonus identifier for Mechanical Turk participants.
	rseed INTEGER NOT NULL,
	-- The player role (0 for row player, 1 for column player) set
	-- when @experiment.state is >0 or when the participant joins.
	role INTEGER NOT NULL DEFAULT(0),
	-- Player was auto-added (i.e., auto-added herself). These
	-- participants are never e-mailed. 
	autoadd INTEGER NOT NULL DEFAULT(0),
	-- If the questionnaire facility has been enabled (via
	-- @experiment.questionnaire), this is non-zero if the
	-- questionnaire has been answered.
	answer INTEGER NOT NULL DEFAULT(0),
	-- Whether the participant should be shown instructions when she
	-- logs in (versus being taken directly to the game-play tab).
	instr INTEGER NOT NULL DEFAULT(1),
	-- The round when the participant joined (i.e., started playing)
	-- the experiment.  This is set to zero when participants join
	-- at the outset.  If -1, the participant exists, but has not
	-- yet started participating in the experiment.
	joined INTEGER NOT NULL DEFAULT(-1),
	-- The minimum of the slot of the participant's tickets among
	-- all participants' tickets. For example, given 100
	-- participants with roughly 10 tickets each, this might be 543
	-- to indicate that slot 543 to 543 plus @player.finalscore are
	-- this participant's slots in the lottery.
	-- Mechanical Turk players are not included in this computation,
	-- so this value is not meaningful for them.
	finalrank INTEGER NOT NULL DEFAULT(0),
	-- Set to the accumulated payoffs from @lottery.aggrpayoff
	-- rounded up to the nearest integer.
	-- Mechanical Turk players are not included in this computation,
	-- so this value is not meaningful for them.
	finalscore INTEGER NOT NULL DEFAULT(0),
	-- A number from zero that indicates the number of time the
	-- @player columns have been updated. This is used to make the
	-- object cachable.
	version INTEGER NOT NULL DEFAULT(0),
	-- The participant's password set when the experiment is started
	-- (i.e., when @experiment.state is >0) or when reset, or when a
	-- captive participant registeres. Note: this is stored in the
	-- clear: it is not a hash!
	hash TEXT,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	UNIQUE (email)
);

-- A lottery is created for an individual @player when the corresponding
-- participant has been granted @payoff for all @game rows in a round. 
-- This doesn't really have anything to do with the ``lottery'' concept
-- of reward: the name is just an historic holdover.

CREATE TABLE lottery (
	-- The round number (starting at zero) for which this lottery
	-- was computed.
	round INTEGER NOT NULL,
	-- The participant owning the lottery.
	playerid INTEGER REFERENCES player(id) NOT NULL,
	-- The player's aggregate payoff (as a rational number)
	-- computing by adding the previous round's aggregate payoff to
	-- the current @"lottery.curpayoff". 
	aggrpayoff TEXT NOT NULL,
	-- The value of @lottery.aggrpayoff represented as a natural
	-- number of tickets. 
	aggrtickets INTEGER NOT NULL DEFAULT(0),
	-- The participant's current payoff (as a rational number)
	-- computing by accumulating her @payoff.payoff for all games in
	-- the experiment. This is set to 0/1 if the participant has not
	-- played all games.
	curpayoff TEXT NOT NULL,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	UNIQUE (round, playerid)
);

-- During a given round, this records a @"player"'s status in terms of
-- number of @choice rows (plays) made.

CREATE TABLE gameplay (
	-- The round number (starting at zero).
	round INTEGER NOT NULL,
	-- The number of games this participant has played, i.e., the
	-- count of @choice rows.
	choices INTEGER NOT NULL DEFAULT(0),
	-- Participant identifier.
	playerid INTEGER REFERENCES player(id) NOT NULL,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	UNIQUE (round, playerid)
);

-- When the given round has completed, this consists of the payoff of
-- the participant's @choice strategy mix for a given @game when played
-- against the average strategy of the opposing player role.

CREATE TABLE payoff (
	-- The round number (starting at zero).
	round INTEGER NOT NULL,
	-- Participant identifier.
	playerid INTEGER REFERENCES player(id) NOT NULL,
	-- Game identifier.
	gameid INTEGER REFERENCES game(id) NOT NULL,
	-- A rational number of the payoff.
	payoff TEXT NOT NULL,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	UNIQUE (round, playerid, gameid)
);

-- A strategy mixture created when a @player plays a round in a @"game". 

CREATE TABLE choice (
	-- The round number (starting at zero). 
	round INTEGER NOT NULL,
	-- A text (space-separated) list of rational numbers of the
	-- strategy mixture ordered from the top if a row-playing role
	-- (i.e., @player.role) or left if a column-player.
	strats TEXT NOT NULL,
	-- Number of entries in @"choice.strats". This obviously equals
	-- the number of strategies available to the participant in that
	-- @game, given the player role.
	stratsz INTEGER NOT NULL,
	-- The participant.
	playerid INTEGER REFERENCES player(id) NOT NULL,
	-- The game.
	gameid INTEGER REFERENCES game(id) NOT NULL,
	-- The session.
	sessid INTEGER REFERENCES sess(id) NOT NULL,
	-- When this was created (epoch).
	created INTEGER NOT NULL,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	UNIQUE (round, playerid, gameid)
);

-- The questionnaire is an optional set of questions given to each
-- participant.  These participants must submit a set number of correct
-- answers before joining the game.  Only a participant's first answer
-- counts.

CREATE TABLE questionnaire (
	-- Participant.
	playerid INTEGER REFERENCES player(id) NOT NULL,
	-- The number of tries, correct or not. This is zero only in the
	-- race after initially creating the row and noting an attempt.
	-- It should not be used and will be removed. 
	tries INTEGER NOT NULL DEFAULT(0),
	-- The question number starting at zero.
	rank INTEGER NOT NULL,
	-- The epoch time when the row is created.
	first INTEGER NOT NULL,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	UNIQUE (playerid, rank)
);

-- This describes an experiment configured by the experimenter (see
-- @admin). There is always one (and only one) row in this table.

CREATE TABLE experiment (
	-- The running state of the experiment, being either 0, for new
	-- (still in the configuration stage); 1, for started
	-- (participants can log in, though the experiment itself may
	-- not be accepting plays yet); 2, where the experiment has
	-- expired, but the winner has not been chosen by the
	-- experimenter; or 3, where the experiment has expired and the
	-- winner has been chosen.
	state INTEGER NOT NULL DEFAULT(0),
	-- Epoch when the experiment when the first round of the
	-- experiment begins.
	start INTEGER DEFAULT(0),
	-- The number of rounds that will be played. Minimum of one.
	rounds INTEGER DEFAULT(0),
	-- The maximum number of participants per player role at any
	-- given time. If zero, the number of participants is unbound. 
	playermax INTEGER DEFAULT(0),
	-- If specified when configuring the experiment, a daemon will
	-- be started that periodically checks to see if the round has
	-- advanced. Its process identifier is stored in this field.
	-- Otherwise, this is zero. 
	roundpid INTEGER DEFAULT(0),
	-- The number of rounds playable by each participant. If zero,
	-- participants will play until the end of the experiment. If
	-- >0, participants will not be allowed to play more than the
	-- given number of rounds within the experiment.
	prounds INTEGER DEFAULT(0),
	-- The current round of the experiment, or -1 if the rounds have
	-- not begun incrementing. This will be set to
	-- @experiment.rounds when the experiment concludes. 
	round INTEGER DEFAULT(-1),
	-- If round is non-negative, then the epoch when the round was
	-- incremented. Otherwise, this is zero. 
	roundbegan INTEGER DEFAULT(0),
	-- If >0, this represents the fraction of participants per
	-- player role who play all games and determine that the round
	-- automatically advances. In other words, if set to 0.5 (50%),
	-- then from both player roles, if 50% or more participants have
	-- played all games, the round advances in advance of the set
	-- round termination time. 
	roundpct REAL DEFAULT(0),
	-- If roundpct is non-zero, this is the ``grace time'' (in
	-- minutes) before which the round will automatically advance
	-- giving the percentage. 
	roundmin INTEGER DEFAULT(0),
	-- The number of minutes per @"experiment.round". Minimum of
	-- one.
	minutes INTEGER DEFAULT(0),
	-- The experiment is currently accepting (or did accept)
	-- auto-added participants.
	autoadd INTEGER NOT NULL DEFAULT(0),
	-- HIT (Mechanical Turk) identifier given by the Mechanical Turk
	-- server for an mturk experiment.  Otherwise, an empty string.
	hitid TEXT NOT NULL DEFAULT(''),
	-- Preserve the @experiment.autoadd state during and after the
	-- experiment start. This is useful for rolling experiments so
	-- as not to have a break between starting the experiment and
	-- re-enabling captive mode. 
	autoaddpreserve INTEGER NOT NULL DEFAULT(0),
	-- A JSON file that provides a ``fake'' history prepended to the
	-- actual experiment history.
	-- This is only shown to users through the browser interface.
	history TEXT NOT NULL DEFAULT(''),
	-- Human-readable amount awarded in lottery, e.g., 10 000 SEK.
	-- If set to the empty string, there's no lottery.
	lottery TEXT NOT NULL DEFAULT(''),
	-- If non-zero, new participants are not directly assigned a
	-- join round and must use the lobby facility. There, they will
	-- be asked questions and cannot proceed until all questions
	-- have been answered.  See @"questionnaire".
	questionnaire INTEGER NOT NULL DEFAULT(0),
	-- Set when the experiment begins (i.e., when @experiment.state
	-- >0), this is the URL given to players in their initial e-mail
	-- for when they log in. This is suffixed by
	-- ``?ident=EMAIL&password=PASSWORD''.
	loginuri TEXT DEFAULT(''),
	-- The instructions shown to participants. This must contain
	-- valid HTML5. It may contain \@\@gamelab-admin-email\@\@, which is
	-- filled in with the experimenter's configured e-mail address;
	-- \@\@gamelab-games\@\@ for the number of games;
	-- \@\@gamelab-rounds\@\@ for the number of rounds; and
	-- \@\@gamelab-round-time\@\@, a decimal number of the number of
	-- hours per round.
	instr TEXT DEFAULT(''),
	-- When the experiment finishes (i.e., when @experiment.state is
	-- set to 2), this is filled in with the total number of lottery
	-- tickets (the @player.finalscore) awarded to all participants.
	total INTEGER DEFAULT(0),
	-- Bit-field. Contains 0x01 if the history is not to be
	-- transmitted to participants, 0x02 if games and rows shouldn't
	-- be randomised prior to display, and 0x04 to show participants
	-- the relative count of rounds---relative to when they started
	-- to play.
	flags INTEGER DEFAULT(0),
	-- If not an empty string, a Mechanical Turk access key.  This
	-- and @experiment.awssecretkey must be set in order to enable a
	-- Mechanical Turk experiment.
	awsaccesskey TEXT NOT NULL DEFAULT(''),
	-- If not an empty string, a Mechanical Turk secret key.  This
	-- and @experiment.awsaccesskey must be set in order to enable a
	-- Mechanical Turk experiment.
	awssecretkey TEXT NOT NULL DEFAULT(''),
	-- If not an empty string, set to an error from a preceeding
	-- Mechanical Turk ``start new HIT'' operation that failed.
	awserror TEXT NOT NULL DEFAULT(''),
	-- How many Mechanical Turk workers to request, if applicable.
	awsworkers INTEGER NOT NULL DEFAULT(2),
	-- What to call this Mechanical Turk experiment, if applicable.
	awsname TEXT NOT NULL DEFAULT(''),
	-- Description of this Mechanical Turk experiment, if
	-- applicable.
	awsdesc TEXT NOT NULL DEFAULT(''),
	-- Keywords (comma-separated) of this Mechanical Turk
	-- experiment, if applicable.
	awskeys TEXT NOT NULL DEFAULT(''),
	-- Whether to use the Mechanical Turk sandbox in all enquiries,
	-- if applicable.
	awssandbox INTEGER NOT NULL DEFAULT(1),
	-- How to convert a Mechanical Turk participant's final score
	-- (tickets) into real currency, if applicable.
	awsconvert REAL NOT NULL DEFAULT(1),
	-- The base reward to give to Mechanical Turk participants, if
	-- applicable.
	awsreward REAL NOT NULL DEFAULT(0.01),
	-- If non-empty and applicable, the locale to request (country
	-- code) for Mechanical Turk participants.
	awslocale TEXT NOT NULL DEFAULT(''),
	-- If >=0 and applicable, the number of HITs for each Mechanical
	-- Turk participant.
	awswhitappr INTEGER NOT NULL DEFAULT(-1),
	-- If >=0 and applicable, the percent (out of 100) of approved
	-- HITs for each Mechanical Turk participant.
	awswpctappr INTEGER NOT NULL DEFAULT(-1),
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);

-- A game is a payoff bimatrix configured by the @"admin". There may be any
-- non-zero number of games in a running @"experiment".

CREATE TABLE game (
	-- Number of strategies for row player.
	p1 INTEGER NOT NULL,
	-- Number of strategies for column player.
	p2 INTEGER NOT NULL,
	-- A list of space-separated payoffs from the top-left to the
	-- bottom-right of the payoff matrix, ordered row-player payoff,
	-- column-player payoff. 
	payoffs TEXT NOT NULL,
	-- The name of the experiment. This is shown only to the
	-- @"admin".
	name TEXT NOT NULL,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);

-- A browser (or otherwise www) session. Sessions are the usual browser
-- session used when participants (or the experimenter) are interacting
-- with the system. 

CREATE TABLE sess (
	-- Epoch when session was created.
	created INTEGER NOT NULL,
	-- Magic random cookie for each session.
	cookie INTEGER NOT NULL,
	-- Participant identifier, if not NULL.  Otherwise, this is the
	-- @admin logged in.
	playerid INTEGER REFERENCES player(id) DEFAULT NULL,
	-- Browser-reported user agent.
	useragent TEXT NOT NULL,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);

-- Experimenter (a.k.a. administrator) credentials. There is always one
-- (and only one) row that exists, initialised to default values.

CREATE TABLE admin (
	-- The experimenter e-mail address. This is used as a predefined
	-- template value for the instructions. It is also used as the
	-- destination for backups of the database. Initialises to
	-- foo\@example.com.
	email TEXT NOT NULL,
	-- The experimenter password. Note: this is stored as cleartext,
	-- so it is not really a hash. Initialises to xyzzy.
	hash TEXT NOT NULL,
	-- Bit-field on whether the email and hash have been set by the
	-- user. Contains 0x01 if the e-mail has been set, 0x02 for the
	-- password. 
	isset INTEGER NOT NULL DEFAULT(0),
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);

-- Records the average strategy mixture of a given player role for a
-- given round and game when a round has concluded. 

CREATE TABLE past (
	-- The round starting at zero. 
	round INTEGER NOT NULL,
	-- Game being referenced.
	gameid INTEGER REFERENCES game(id) NOT NULL,
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	-- For the row player, the average strategy mixture of the
	-- current round. Strategy mixtures are only considered for
	-- @choice rows where the @player played all games for the
	-- round, so this will be a set of zeroes if no participant
	-- completed all games. This is recorded as a space-separated
	-- sequence of rational numbers, one per strategy. 
	currentsp1 TEXT NOT NULL,
	-- Like @past.currentsp1 but for the column player role.
	currentsp2 TEXT NOT NULL,
	-- If zero row or column-player participants played all games,
	-- this is set to 1, else it is 0 (sufficient players played). 
	skip INTEGER NOT NULL,
	-- The total number of plays that were submitted (over all
	-- games) in this round if and only if the participant submitted
	-- for all games in that round. 
	plays INTEGER NOT NULL,
	-- The accumulated count of rounds not skipping, i.e., the count
	-- of zero-valued skip rounds. 
	roundcount INTEGER NOT NULL,
	unique (round, gameid)
);

-- This consists of the SMTP server information used in sending e-mails.
-- There is always only one row set, which defaults to empty values (see
-- @smtp.isset). 

CREATE TABLE smtp (
	-- The SMTP server username (used for logging in).
	user TEXT NOT NULL DEFAULT(''),
	-- The e-mail address used as the ``From'' address in all
	-- communication from the server to participants in the
	-- experimenter. It is usually the same as the experimenter
	-- email set in @"admin", but may be set as a standard
	-- ``No-Reply''. (This is discouraged, as your participants
	-- should be able to reply to you if things go wrong.) 
	email TEXT NOT NULL DEFAULT(''),
	-- The SMTP username's password (used for logging in). Note that
	-- this is stored in cleartext, so make sure that your password
	-- isn't used elsewhere.
	pass TEXT NOT NULL DEFAULT(''),
	-- The SMTP server in smtp://server:port format.
	server TEXT NOT NULL DEFAULT(''),
	-- Whether these entries have been set.
	isset INTEGER NOT NULL DEFAULT(0),
	-- Unique identifier.
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);

INSERT INTO experiment DEFAULT VALUES;
INSERT INTO smtp DEFAULT VALUES;

INSERT INTO admin (email, hash) VALUES ('foo@example.com', 'xyzzy');
