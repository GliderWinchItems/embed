-- SYSFUNCTIONS table
-- 05/20/2016
--
--  Associates "functions" as defined at the overall system level with
--  "generic function."  Specific functions that are provided by a program and CAN unit
--  implement the generic function.  E.g. 'TENSION_DRUM_1' at the overall level is unique
--  drum #1 and the 'TENSION' function.  The CAN msg for drum #1 is constant when implemented
--  with different boards and/or programs that support the generic function type of TENSION.
--
-- SYSFUNCTION_NAME varchar(24) PRIMARY KEY,
--  This is the name assigned to the overall system view of a "function," e.g.
--  the tension on drum #1 is such a function.  Implementation of this function
--  might take different forms at the CAN unit(node) level, but it still 
--
-- SYSFUNCTION_CODE NUMERIC UNIQUE,
--  Numeric code to associate name with a number for programs that cannot afford to deal with strings.
--
-- GENERIC_FUNC_TYPE varchar(24),
--  This the name assigned to a specific implementation *type*.  There may
--  be multiple instances of this particular type.
-- 
-- DESCRIPTION18 varchar(128) NOT NULL UNIQUE
--  Good words that might mean something to someone. 
--
DELETE from SYSFUNCTIONS;
--                              SYSFUNCTION_NAME     SYSFUNCTION_CODE   GENERIC_FUNC_TYPE     DESCRIPTION18
INSERT INTO SYSFUNCTIONS VALUES ('TENSION_DRUM_1',	1,		'TENSION',	'Drum 1 tension' );
INSERT INTO SYSFUNCTIONS VALUES ('TENSION_DRUM_2',	2,		'TENSION',	'Drum 2 tension' );
INSERT INTO SYSFUNCTIONS VALUES ('CABLE_ANGLE_1',	3,		'CABLE_ANGLE',	'Drum 1 cable Angle' );
INSERT INTO SYSFUNCTIONS VALUES ('CABLE_ANGLE_2',	4,		'CABLE_ANGLE',	'Drum 2 cable Angle' );
INSERT INTO SYSFUNCTIONS VALUES ('MOTOR_ENCODE_1',	5,		'MOTOR_ENCODE',	'Motor 1 shaft encoder' );
INSERT INTO SYSFUNCTIONS VALUES ('MOTOR_ENCODE_2',	6,		'MOTOR_ENCODE',	'Motor 2 shaft encoder' );
INSERT INTO SYSFUNCTIONS VALUES ('SHAFT_UPPERSHV',	7,		'SHAFT_ENCODER','Upper sheave Count and speed' );
INSERT INTO SYSFUNCTIONS VALUES ('SHAFT_LOWERSHV',	8,		'SHAFT_ENCODER','Lower sheave Count and speed' );
INSERT INTO SYSFUNCTIONS VALUES ('DRIVE_SHAFT',		9,		'SHAFT_ENCODER','Drive shaft encoder count and speed' );
INSERT INTO SYSFUNCTIONS VALUES ('ENGINE_SENSOR',	10,		'ENGINE_SENSOR','Sensor, engine: rpm, manifold pressure, throttle setting, temperature' );
INSERT INTO SYSFUNCTIONS VALUES ('TILT_SENSE',		11,		'TILT_SENSE',	'Tilt sensor' );
INSERT INTO SYSFUNCTIONS VALUES ('YOGURT_1',		12,		'YOGURT_1',	'Yogurt_1: Ver 1 of maker' );
INSERT INTO SYSFUNCTIONS VALUES ('HC_SANDBOX_A',	13,		'HC_SANDBOX_a',	'Host Controller: sandbox function #1 (or call it a)' );

