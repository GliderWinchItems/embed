-- WINCHSTATE table
-- 05/31/2016
--
-- WINCHSTATE_NAME varchar(24) PRIMARY KEY,
--  Name of winch state
--
-- WINCHSTATE_CODE	NUMERIC UNIQUE,
--  Code that matches the name
--  Naming convention:
--  'SUPER_STATE_' = Upper 4 bits of a byte
--  'SUB_STATE_' = Lower 4 bits of a byte
--
-- DESCRIPTION19 varchar(128) NOT NULL UNIQUE
--  Good words that might mean something to someone. 
--
DELETE from WINCHSTATE;
--                 
--                              WINCHSTATE_NAME      WINCHSTATE_CODE   DESCRIPTION19
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_SAFE',	 0,	' Winchstate:  0 Safe (idle) ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_PREP', 	 1,	' Winchstate:  1 Prepartory somethings ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_ARM' , 	 2,	' Winchstate:  2 Motor controller connected to battery string ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_PROFILE',  	 3,	' Winchstate:  3 Early launch tension profile used ' );
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_TAPER', 	 4,	' Winchstate:  4 Tapering tension to limit speed before rotation ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_CLIMB', 	 5,	' Winchstate:  5 Glider in climb state ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_RECOVERY', 	 6,	' Winchstate:  6 Glider released and chute recovery ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_RETRIEVE',	 7,	' Winchstate:  7 Cable being towed back to launch position ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_SPARE_8',	 8,	' Winchstate:  8 unassigned ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_SPARE_9',	 9,	' Winchstate:  9 unassigned ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_ABORT', 	10,	' Winchstate: 10 Immediate shutdown of current launch ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_STOP', 	11,	' Winchstate: 11 Stopped??? ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_SPARE_12',	12,	' Winchstate: 12 unassigned ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_SPARE_13',	13,	' Winchstate: 13 unassigned ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_SPARE_14',	14,	' Winchstate: 14 unassigned ');
INSERT INTO WINCHSTATE VALUES ('SUPER_STATE_CAL', 	15,	' Winchstate: 15 Calibration ');

