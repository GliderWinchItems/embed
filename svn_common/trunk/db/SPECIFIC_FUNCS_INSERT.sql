-- SPECIFIC_FUNCS table
-- 05/20/2016
--
--  Associates "functions" as defined at the overall system level with
--  "functions" defined at the CAN unit(node) implementation level
--
-- FUNCTION_NAME  varchar(24) PRIMARY KEY,
--  Specific instance of a FUNCTIONS.FUNCTION_TYPE
--
-- FUNCTION_TYPE varchar(24),
--  Type of function for which this is an instance
--
-- DESCRIPTION17 varchar(128) NOT NULL UNIQUE
--  Good words that might mean something to someone. 
--
DELETE from SPECIFIC_FUNCS;
--                                FUNCTION_NAME          FUNCTION_TYPE    DESCRIPTION17
INSERT INTO SPECIFIC_FUNCS VALUES ('DRIVE_SHAFT',	'SHAFT_ENCODER','Sensor, shaft: Drive shaft encoder' );
INSERT INTO SPECIFIC_FUNCS VALUES ('SHAFT_UPPERSHV',	'SHAFT_ENCODER','(Upper sheave)' );
INSERT INTO SPECIFIC_FUNCS VALUES ('SHAFT_LOWERSHV',	'SHAFT_ENCODER','(Lower sheave) Count and speed' );
INSERT INTO SPECIFIC_FUNCS VALUES ('ENGINE_SENSOR',	'ENGINE_SENSOR','Sensor, engine: rpm, manifold pressure, throttle setting, temperature' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_a12',	'TENSION_a',	'Tension_a: Tension drum #2 AD7799 #1' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_a22',	'TENSION_a2',	'Tension_a2: Tension drum #2 AD7799 #2' );
INSERT INTO SPECIFIC_FUNCS VALUES ('CABLE_ANGLE_1',	'CABLE_ANGLE', 	'Cable angle AD7799 #2 drum #1' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_2',		'TENSION_c',	'Tension_2: Tension AD7799 #1 drum #2' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_CAL_1',	'TENSION_a',	'Calibration tension: S-load-cell #1' );
INSERT INTO SPECIFIC_FUNCS VALUES ('CABLE_ANGLE_2',	'CABLE_ANGLE',	'Cable angle AD7799 #2 drum #2' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TIMESYNC',		'TIMESYNC',	'GPS time sync distribution msg' );
INSERT INTO SPECIFIC_FUNCS VALUES ('HC_SANDBOX_1',	'HC_SANDBOX_1',	'Host Controller: sandbox function 1' );
INSERT INTO SPECIFIC_FUNCS VALUES ('MC_a',		'MC',		'Master Controller' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TILT_SENSE_a',	'TILT_SENSE',	'Tilt sensor' );
INSERT INTO SPECIFIC_FUNCS VALUES ('YOGURT_1',		'YOGURT_1',	'Yogurt_1: Ver 1 of maker' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_a0X',	'TENSION_a',	'Tension_a1: 1 AD7799 VE POD Test (hence X) 0' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_a11',	'TENSION_a',	'Tension_a1: 2 AD7799 VE POD Test (hence X) 1' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_a21',	'TENSION_a2',	'Tension_a2: 2 AD7799 VE POD Test (hence X) 1' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_a1Y',	'TENSION_a',	'Tension_a1: 1 AD7799 VE POD Test (hence X) 2' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_a1Z',	'TENSION_a',	'Tension_a1: 2 AD7799 VE POD Test (hence X) 3' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_a2Z',	'TENSION_a2',	'Tension_a2: 2 AD7799 VE POD Test (hence X) 3' );
INSERT INTO SPECIFIC_FUNCS VALUES ('TENSION_a1G',	'TENSION_a',	'Tension_a1: 2 AD7799 VE POD GSM 1' );
INSERT INTO SPECIFIC_FUNCS VALUES ('GPS',		'GPS',		'GPS: ublox gps.  Only one GPS function on a system');
INSERT INTO SPECIFIC_FUNCS VALUES ('LOGGER_1',		'LOGGER',	'Logger 1: SD card logger');


