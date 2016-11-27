-- MISC_SYS: System definitions used by both java and C programs
--
-- Delete all the current values:
DELETE from MISC_SYS;
-- Then add everything that follows:
--   

-- MISC_SYS_NAME varchar(48) PRIMARY KEY,
--  Unqiue name used by programs
--
-- MISC_SYS_VAL varchar(32) NOT NULL UNIQUE,
--  String with the ascii value assigned to the name
--
-- MISC_SYS_FMT varchar(16) NOT NULL,
--  String with NUMBER_TYPE_NAME (from NUMBER_TYPE table)
--
-- DESCRIPTION_MISC_SYS varchar(128) NOT NULL UNIQUE 
--  Some words to help understand what this entry is about
--
--                                                        MISC_SYS_FMT
--                             MISC_SYS_NAME    MISC_SYS_VAL                DESCRIPTION_MISC_SYS
INSERT INTO MISC_SYS VALUES ('LAUNCH_PARAM_BURST_SIZE', '16',   	'TYPE_U32',	'Maximum number of CAN msgs in a burst when sending launch parameters');
INSERT INTO MISC_SYS VALUES ('LAUNCH_PARAM_RETRY_CT',   '3',   	'TYPE_U32',	'Number of error retries when sending launch parameters');
INSERT INTO MISC_SYS VALUES ('LAUNCH_PARAM_RETRY_TIMEOUT', '500','TYPE_U32',	'Number of milliseconds to wait for a response when sending launch parameters');
INSERT INTO MISC_SYS VALUES ('VER_MCL', 		'1','TYPE_U32',	'Version: Master Controller Launch parameters database table PARAM_LIST');
INSERT INTO MISC_SYS VALUES ('VER_TENSION_a', 		'1','TYPE_U32',	'Version: Tension_a: parameters database table PARAM_LIST');
INSERT INTO MISC_SYS VALUES ('VER_LOGGER', 		'1','TYPE_U32',	'Version: Logger: parameters database table PARAM_LIST');
INSERT INTO MISC_SYS VALUES ('VER_GPS', 		'1','TYPE_U32',	'Version: GPS: parameters database table PARAM_LIST');

