-- 04/12/2015, 05/21/2016
-- Define bits in second byte of group polling msg
-- ALSO, define bits in first byte of group polling msg
--
-- Each FUNCTION_TYPE has a bit assigned.
-- When the bit matches the bit in the group poll msg
--  and the the drum/group number matches in the first
--  byte, the function sends it measurement.
--
-- FUNC_BIT_PARAM_NAME varchar(48) PRIMARY KEY,
--   A unique name to refer to the bit/function
--
-- FUNC_BIT_PARAM_VAL varchar(24),
--   Bit specifies the function to respond to a poll msg
--
-- FUNCTION_TYPE varchar(24),
--
-- FORMAT varchar(24),
--  Format type for display purposes, e.g. java
--
-- DESCRIPTION5 varchar(128) NOT NULL UNIQUE
--  Unique description 
--
DELETE FROM FUNC_BIT_PARAM;
--
--                                 FUNC_BIT_PARAM_NAME      FUNC_BIT_PARAM_VAL FUNCTION_TYPE   FORMAT      DESCRIPTION5 
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_FUNC_BIT_TENSION', 		'0x1',	'TENSION',	'%x',	'Function bit: 2nd byte of poll msg: TENSION');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_FUNC_BIT_CABLE_ANGLE', 	'0x2',	'CABLE_ANGLE',	'%x',	'Function bit: 2nd byte of poll msg: CABLE_ANGLE');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_FUNC_BIT_SHAFT_ODO_SPD', 	'0x4',	'SHAFT_ENCODER','%x',	'Function bit: 2nd byte of poll msg: shaft odometer & speed');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_FUNC_BIT_TILT', 		'0x8',	'TILT_SENSE', 	'%x',	'Function bit: 2nd byte of poll msg: TILT');
--
-- First byte of group poll msg 
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_DO_NOT', 			'0x1',	'DUMMY'        ,'%x',	'Selection bit: No reply sent to poll msg');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_DRUM_BIT_2', 			'0x2',	'SELECT_DRUM_1','%x',	'Selection bit: 1st byte of poll msg Drum #1');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_DRUM_BIT_3', 			'0x4',	'SELECT_DRUM_2','%x',	'Selection bit: 1st byte of poll msg Drum #2');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_DRUM_BIT_4', 			'0x8',	'SELECT_DRUM_3','%x',	'Selection bit: 1st byte of poll msg Drum #3');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_DRUM_BIT_5', 			'0x10',	'SELECT_DRUM_4','%x',	'Selection bit: 1st byte of poll msg Drum #4');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_DRUM_BIT_6', 			'0x20',	'SELECT_DRUM_5','%x',	'Selection bit: 1st byte of poll msg Drum #5');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_DRUM_BIT_7', 			'0x40',	'SELECT_DRUM_6','%x',	'Selection bit: 1st byte of poll msg Drum #6');
INSERT INTO FUNC_BIT_PARAM VALUES ('POLL_DRUM_BIT_8', 			'0x80',	'SELECT_DRUM_7','%x',	'Selection bit: 1st byte of poll msg Drum #7');
--
-- Status byte of polled readings
INSERT INTO FUNC_BIT_PARAM VALUES ('STATUS_TENSION_BIT_NONEW',		'0x1',	'TENSION'	,'%x',	'status: No new reading since last poll msg sent');
INSERT INTO FUNC_BIT_PARAM VALUES ('STATUS_TENSION_BIT_EXCEEDHI',	'0x2',	'TENSION'	,'%x',	'status: Reading limit hi exceed (open (-) connection?)');
INSERT INTO FUNC_BIT_PARAM VALUES ('STATUS_TENSION_BIT_EXCEEDLO',	'0x4',	'TENSION'	,'%x',	'status: Reading limit lo exceed (open (+ or both) connection?)');
INSERT INTO FUNC_BIT_PARAM VALUES ('STATUS_TENSION_BIT_4',		'0x8',	'TENSION'	,'%x',	'status: spare 0x8');
INSERT INTO FUNC_BIT_PARAM VALUES ('STATUS_TENSION_BIT_5',		'0x10',	'TENSION'	,'%x',	'status: spare 0x10');
INSERT INTO FUNC_BIT_PARAM VALUES ('STATUS_TENSION_BIT_6',		'0x20',	'TENSION'	,'%x',	'status: spare 0x20');
INSERT INTO FUNC_BIT_PARAM VALUES ('STATUS_TENSION_BIT_7',		'0x40',	'TENSION'	,'%x',	'status: spare 0x40');
INSERT INTO FUNC_BIT_PARAM VALUES ('STATUS_TENSION_BIT_DONOTUSE',	'0x80',	'TENSION'	,'%x',	'status: spare 0x80');
--
-- USEME bits for which tension function on a board to use
INSERT INTO FUNC_BIT_PARAM VALUES ('USEME_TENSION_BIT_AD7799_1',	'0x1',	'TENSION'	,'%x',	'useme: 1st AD7799');
INSERT INTO FUNC_BIT_PARAM VALUES ('USEME_TENSION_BIT_AD7799_2',	'0x2',	'TENSION'	,'%x',	'useme: 2nd AD7799');
INSERT INTO FUNC_BIT_PARAM VALUES ('USEME_TENSION_BIT_3',		'0x4',	'TENSION'	,'%x',	'useme: spare 0x4');
INSERT INTO FUNC_BIT_PARAM VALUES ('USEME_TENSION_BIT_4',		'0x8',	'TENSION'	,'%x',	'useme: spare 0x8');
INSERT INTO FUNC_BIT_PARAM VALUES ('USEME_TENSION_BIT_5',		'0x10',	'TENSION'	,'%x',	'useme: spare 0x10');
INSERT INTO FUNC_BIT_PARAM VALUES ('USEME_TENSION_BIT_6',		'0x20',	'TENSION'	,'%x',	'useme: spare 0x20');
INSERT INTO FUNC_BIT_PARAM VALUES ('USEME_TENSION_BIT_7',		'0x40',	'TENSION'	,'%x',	'useme: spare 0x40');
INSERT INTO FUNC_BIT_PARAM VALUES ('USEME_TENSION_BIT_8',		'0x80',	'TENSION'	,'%x',	'useme: spare 0x80');

