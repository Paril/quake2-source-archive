DROP TABLE IF EXISTS abilities;

CREATE TABLE abilities 
(
  char_idx INTEGER,
  aindex INTEGER, 
  level INT, 
  max_level INT, 
  hard_max INT, 
  modifier INT, 
  disable INT, 
  general_skill INT
);

DROP TABLE IF EXISTS character_data;

CREATE TABLE character_data 
(
  char_idx INTEGER PRIMARY KEY,
  respawns INTEGER, 
  health INTEGER, 
  maxhealth INTEGER, 
  armour INTEGER, 
  maxarmour INTEGER, 
  nerfme INTEGER, 
  adminlevel INTEGER, 
  bosslevel INTEGER
);

DROP TABLE IF EXISTS ctf_stats;

CREATE TABLE ctf_stats 
(
  char_idx INTEGER,
  flag_pickups INTEGER, 
  flag_captures INTEGER, 
  flag_returns INTEGER, 
  flag_kills INTEGER, 
  offense_kills INTEGER, 
  defense_kills INTEGER, 
  assists INTEGER
);

DROP TABLE IF EXISTS game_stats;

CREATE TABLE game_stats 
(
  char_idx INTEGER,
  shots INTEGER, 
  shots_hit INTEGER, 
  frags INTEGER, 
  fragged INTEGER, 
  num_sprees INTEGER, 
  max_streak INTEGER, 
  spree_wars INTEGER, 
  broken_sprees INTEGER, 
  broken_spreewars INTEGER, 
  suicides INT, 
  teleports INTEGER, 
  num_2fers INTEGER
);

DROP TABLE IF EXISTS point_data;

CREATE TABLE point_data 
(
  char_idx INTEGER,
  exp INTEGER, 
  exptnl INTEGER, 
  level INTEGER, 
  classnum INTEGER, 
  skillpoints INTEGER, 
  credits INTEGER, 
  weap_points INTEGER, 
  resp_weapon INTEGER, 
  tpoints INTEGER
);

DROP TABLE IF EXISTS runes_meta;

CREATE TABLE runes_meta 
(
  char_idx INTEGER,
  rindex INTEGER, 
  itemtype INTEGER, 
  itemlevel INTEGER, 
  quantity INTEGER, 
  untradeable INTEGER, 
  id CHAR(16), 
  name CHAR(24), 
  nummods INTEGER, 
  setcode INTEGER, 
  classnum INTEGER
);

DROP TABLE IF EXISTS runes_mods;

CREATE TABLE runes_mods 
(
  char_idx INTEGER,
  rune_index INTEGER, 
  rmod INTEGER,
  type INTEGER, 
  mindex INTEGER,
  value INTEGER, 
  rset INTEGER
);

DROP TABLE IF EXISTS talents;

CREATE TABLE talents 
(
  char_idx INTEGER,
  id INTEGER, 
  upgrade_level INTEGER, 
  max_level INTEGER
);

DROP TABLE IF EXISTS userdata;

CREATE TABLE userdata 
(
  char_idx INTEGER,
  title CHAR(24), 
  playername CHAR(64), 
  password CHAR(24), 
  email CHAR(64), 
  owner CHAR(24), 
  member_since CHAR(30), 
  last_played CHAR(30), 
  playtime_total INTEGER, 
  playingtime INTEGER,
  isplaying INTEGER
);

DROP TABLE IF EXISTS weapon_meta;

CREATE TABLE weapon_meta 
(
  char_idx INTEGER,
  windex INTEGER, 
  disable INTEGER
);

DROP TABLE IF EXISTS weapon_mods;

CREATE TABLE weapon_mods 
(
  char_idx INTEGER,
  weapon_index INTEGER, 
  modindex INTEGER,
  level INTEGER,
  soft_max INTEGER, 
  hard_max INTEGER
);

-- Todo: Uniques list in da SQL!

DELIMITER $$

DROP PROCEDURE IF EXISTS FillNewChar; $$

-- We create a new character with this function. 
CREATE PROCEDURE FillNewChar(IN charname VARCHAR(64))
BEGIN

	DECLARE chid INT DEFAULT 0;
	
	START TRANSACTION;

	-- Give us the character id so we generate this new character!
    SELECT COUNT(char_idx) INTO chid FROM character_data WHERE char_idx IS NOT NULL;

	INSERT INTO character_data (char_idx) VALUES(chid);

	INSERT INTO userdata (char_idx, playername) VALUES (chid, charname);
    INSERT INTO point_data (char_idx) VALUES (chid);
    INSERT INTO game_stats (char_idx) VALUES (chid);
    INSERT INTO ctf_stats (char_idx) VALUES (chid);

	COMMIT;

END $$

DROP PROCEDURE IF EXISTS CharacterExists; $$

CREATE PROCEDURE CharacterExists(IN pname varchar(64), OUT doesexist int)
BEGIN

	SELECT EXISTS ( 
		SELECT (char_idx) FROM userdata 
		WHERE userdata.playername = pname 
	) INTO doesexist;

END $$

DROP PROCEDURE IF EXISTS ResetTables; $$

CREATE PROCEDURE ResetTables(IN pname varchar(64))
BEGIN

	DECLARE chid INT DEFAULT 0;

	START TRANSACTION;

	SELECT (char_idx) INTO chid FROM userdata WHERE playername=pname;

	DELETE FROM abilities WHERE char_idx = chid;
	DELETE FROM talents WHERE char_idx = chid;
	DELETE FROM runes_meta WHERE char_idx = chid;
	DELETE FROM runes_mods WHERE char_idx = chid;
	DELETE FROM weapon_meta WHERE char_idx = chid;
	DELETE FROM weapon_mods WHERE char_idx = chid;

	COMMIT;

END $$

DROP PROCEDURE IF EXISTS GetCharID; $$

CREATE PROCEDURE GetCharID(IN pname varchar(64), OUT chidx INT)
BEGIN
    SELECT (char_idx) INTO chidx FROM userdata WHERE playername = pname;
END $$

DROP PROCEDURE IF EXISTS CanPlay; $$

CREATE PROCEDURE CanPlay(IN chidx INT, OUT canplay int)
BEGIN

    DECLARE lp DATE;
	DECLARE str_lp VARCHAR (128);

	SELECT (last_played) INTO str_lp FROM userdata WHERE char_idx = chidx;

	if str_lp != "" THEN
		SELECT DATE (str_lp) INTO lp 
			FROM userdata 
			WHERE char_idx = chidx;
	END IF;

	if lp = current_date AND (select (isplaying) FROM userdata WHERE char_idx = chidx) THEN
		SET canplay = 0;
	else
		SET canplay = 1;
	END IF;


END
-- Saving/Loading procedures are done in-dll, since the code for that is mostly already written. :D
-- That DOES mean the dll MUST be updated before submitting stuff to the database. And the database as well.