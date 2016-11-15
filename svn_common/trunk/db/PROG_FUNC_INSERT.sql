-- PROG_FUNC table
-- 05/20/2016
--
--  Associates a program (path/name) with function types supported
--
-- PROG_NAME varchar(24),
--  Program names used in PROGRAMS
--
-- FUNCTION_TYPE varchar(24),
--  The table FUNCTIONS.FUNCTION_TYPE assigns a *specific instance* of FUNCTION_TYPE
--  to this program, and this program is assigned to a specific CAN unit
--  with the table CAN_UNIT.PROG_NAME.
--
-- PRIMARY KEY(PROG_NAME,FUNCTION_TYPE),
--  A program supports multiple FUNCTION_TYPE(s), but the same program does not support 
--  multiple instances of the same program & function_type, e.g. PROG_TENSION and TENSION_a
--  can only appear once.  A different program, e.g. one using a different amp/adc would have a 
--  different program name and would most likely use a different parameter/calibration table which
--  would be a different FUNCTION_TYPE, but it could is theorectically possible that it was written
--  to use the an existing parameter/calibration table layout, e.g. TENSION_a.

-- DESCRIPTION13 varchar(128) NOT NULL UNIQUE
--
--
DELETE from PROG_FUNC;
--                             PROG_NAME       FUNCTION_TYPE            DESCRIPTION13
INSERT INTO PROG_FUNC VALUES ('PROG_TENSION',	'TENSION_a',		'POD prog: AD7799 #1' );
INSERT INTO PROG_FUNC VALUES ('PROG_TENSION',	'TENSION_a2',		'POD prog: AD7799 #2' );
INSERT INTO PROG_FUNC VALUES ('PROG_TENSION',	'CABLE_ANGLE',		'POD prog: cable angle from combined tension readings' );

INSERT INTO PROG_FUNC VALUES ('PROG_LOGGER',	'LOGGER',		'CO1 prog: sensor board logger w SD card' );
INSERT INTO PROG_FUNC VALUES ('PROG_LOGGER',	'GPS',			'CO1 prog: sensor board GPS w ublox module' );

INSERT INTO PROG_FUNC VALUES ('PROG_CANSENDER',	'CANSENDER',		'cansender prog: sensor board' );

