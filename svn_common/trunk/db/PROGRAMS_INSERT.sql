-- PROGRAMS
-- 05/15/2016
-- 
-- PROG_NAME varchar(24) PRIMARY KEY,
--  A unique name for each program and program variant
--
-- PROG_CODE NUMERIC UNIQUE,
--  A unique code to associate numerically with the program.
--
-- PROG_PATH varchar(128),
--  A partial path to locate the program.
--
-- DESCRIPTION14 varchar(128) NOT NULL UNIQUE
--  Words that amplify the capability of this program
--
--   NOTE: To use the PROG_PATH it needs a prefix added, e.g. '/home/deh/'.
--
DELETE from PROGRAMS;
--                           PROG_NAME   PROG_CODE                   PROG_PATH                                            	DESCRIPTION14
INSERT INTO PROGRAMS VALUES ('PROG_TENSION'	, 1,	'gitrepo/svn_sensor/sensor/tension/trunk/tension.srec', 	'Tension, POD board, two AD7799, four thermistors' );
INSERT INTO PROGRAMS VALUES ('PROG_ENGINE'	, 2,	'gitrepo/svn_sensor/sensor/se1/trunk/se1.srec', 		'Engine,  Manifold press, RPM, ' );
INSERT INTO PROGRAMS VALUES ('PROG_ENCODER'	, 3,	'gitrepo/svn_sensor/sensor/se4_h/trunk/se4_h.srec', 		'Shaft encode: Reflective foto, count & speed');
INSERT INTO PROGRAMS VALUES ('PROG_LOGGER'	, 4,	'gitrepo/svn_sensor/sensor/co1_sensor_ublox/trunk/co1.srec', 	'Logger: ublox GPS w SD card' );
INSERT INTO PROGRAMS VALUES ('PROG_YOGURT'	, 5,	'gitrepo/svn_sensor/sensor/yogurt/trunk/yogurt.srec', 		'Yogurt_1: OlimexL version 1 yogurt maker' );
INSERT INTO PROGRAMS VALUES ('PROG_ACCESSORIES'	, 6,	'gitrepo/svn_sensor/sensor/TODO_1',		 		'Accessories: on/off stuff' );

