-- CAN_UNIT table
-- 07/10/2015; 05/10/2016
--
-- Each unit has a unique CAN_UNIT_NAME, but different units might use the same application program
-- SKIP has bits for doing such things as skipping the loading
-- CANID_NAME is the name of the hex CAN id assignment that is the CANID table, which doesn't have to be the 
--   the same name as the UNIT...
-- Each unit has a loader program in a section of low flash that uses a unique CANID for loading application programs.
-- The application program is found via the PROG_PATH (at the moment an absolute path)
-- Functions that are implemented by the application program are specified in the FUNCTIONS_CODES table
--
-- CAN_UNIT_NAME varchar(32) PRIMARY KEY,
--  Unique name for this CAN unit(node)
--
-- CAN_UNIT_CODE NUMERIC UNIQUE,
--  Unique numeric code for this CAN_UNIT_NAME
--
-- PROG_NAME varchar(48),
--  Unique name for the App program in this CAN unit
--
-- SKIP NUMERIC,
--  Skip automatic loading
--
-- CANID_NAME varchar(48) UNIQUE,
--  CANID used by this CAN UNIT
--
-- DESCRIPTION3 varchar(128) NOT NULL UNIQUE
--  Words to identify this CAN unit(node)
--
DELETE from CAN_UNIT;
--                                       CAN_UNIT_CODE
--                           CAN_UNIT_NAME  ||   PROG_NAME      SKIP   CANID_NAME             DESCRIPTION3
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_2' ,  2,'PROG_ENCODER'	, 0,'CANID_UNIT_2' ,'Shaft encoder: Drive shaft encoder');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_3' ,  3,'PROG_ENGINE' 	, 0,'CANID_UNIT_3' ,'Engine: ');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_4' ,  4,'PROG_ENCODER'	, 1,'CANID_UNIT_4' ,'Shaft encoder: Lower sheave shaft encoder');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_5' ,  5,'PROG_ENCODER'	, 1,'CANID_UNIT_5' ,'Shaft encoder: Upper sheave shaft encoder');

INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_8' ,  6,'PROG_ENCODER'	, 1,'CANID_UNIT_8' ,'Level Wind: ');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_9' ,  7,'PROG_ENCODER'	, 1,'CANID_UNIT_9' ,'XBee receiver #1');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_A' ,  8,'PROG_ENCODER'	, 1,'CANID_UNIT_A' ,'XBee receiver #2');

INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_E' ,  9,'PROG_LOGGER' 	, 1,'CANID_UNIT_E' ,'Logger1: sensor board w ublox gps & SD card');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_1A', 17,'PROG_LOGGER' 	, 1,'CANID_UNIT_1A','Logger2: sensor board w ublox gps & SD card');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_F' , 10,'PROG_TENSION'	, 1,'CANID_UNIT_F' ,'Tension & cable angle: 2 load-cell w temp comp');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_11', 11,'PROG_TENSION'	, 1,'CANID_UNIT_11','Tension: 1 AD7799 VE POD brd 1');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_12', 12,'PROG_TENSION'	, 1,'CANID_UNIT_12','Tension: 1 AD7799 VE POD brd 2');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_13', 13,'PROG_YOGURT' 	, 1,'CANID_UNIT_13','Yogurt_1: version 1 yogurt maker');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_14', 14,'PROG_TENSION'	, 1,'CANID_UNIT_14','Tension: 1 AD7799 VE POD brd 3');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_15', 15,'PROG_TENSION'	, 1,'CANID_UNIT_15','Tension: 1 AD7799 VE POD brd 4');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_16', 16,'PROG_TENSION'	, 1,'CANID_UNIT_16','Tension: 2 AD7799 VE POD brd 5 GSM');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_1B', 18,'PROG_CANSENDER'	, 1,'CANID_UNIT_1B','Cansender: CAW sensor test board');
INSERT INTO CAN_UNIT VALUES ('CAN_UNIT_1C', 19,'PROG_CANSENDER'	, 1,'CANID_UNIT_1C','Cansender: DEH sensor test board');

