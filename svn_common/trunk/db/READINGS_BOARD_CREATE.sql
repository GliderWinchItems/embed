--
DROP TABLE READINGS_BOARD;
--
CREATE TABLE READINGS_BOARD 
(
READINGS_BOARDNAME varchar(48),
READINGS_BOARDCODE numeric(4),
TYPE_NAME varchar(24),
FORMAT varchar(24) NOT NULL,
PROG_NAME varchar(48),
FUNCTION_TYPE varchar(24),
DESCRIPTION15 varchar(128) NOT NULL UNIQUE,
PRIMARY KEY(READINGS_BOARDNAME,FUNCTION_TYPE)
);


