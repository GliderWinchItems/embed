-- READINGS_BOARD_INSERT.sql 
-- 06/08/2016
--
-- Note: primary key is a combination
--
-- Defines readings that are common to all functions on the board, and 
--    are not function-type specific, e.g. CAN driver error counts.
--
-- READINGS_BOARDNAME varchar(48) PRIMARY KEY,
--   Unique name for value.  This name is used in the pointer table for the STM32 programs,
--   so it is best to use a prefix on the name that makes it easy to find in the list.
--
-- READINGS_BOARDCODE numeric(4),
--   Unique number for each PROG_NAME that can be used in place of the name (via #define)
--
-- TYPE_NAME varchar(24),
--   Type of number in payload
-- 
-- FORMAT varchar(24) NOT NULL,
--   Format for the receiving end to display the payload
--
-- PROG_NAME varchar(48),
--   Program that supports this reading.  (May not need this, but it could be useful
--   for sorting.)
--
-- FUNCTION_TYPE (join to FUNCTIONS.)  See FUNCTIONS table.
--
-- DESCRIPTION15 varchar(128) NOT NULL UNIQUE
--   Nonsense for the hapless Op and a crutch for the Wizard programmer; but must be unique
--
-- Readings are a retrieval of four bytes from some fixed memory location.
--   These locations are typically a counter, or some value being updated periodically.
--
DELETE FROM READINGS_BOARD;
--                                    Reading name                                Code   Type     format   Prog name     Function_type  Description15
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_NUM_AD7799'        , 1, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Number of AD7799 that successfully initialized');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_TXERR'    	   , 2, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: total number of msgs returning a TERR flags (including retries)');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_TX_BOMBED'	   , 3, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: number of times msgs failed due to too many TXERR');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_ALST0_ERR' 	   , 4, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: arbitration failure total');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_ALST0_NART_ERR', 5, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: arbitration failure when NART is on');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_MSGOVRFLO'	   , 6, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: Buffer overflow when adding a msg');		
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_SPURIOUS_INT'  , 7, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: TSR had no RQCPx bits on (spurious interrupt)');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_NO_FLAGGED'	   , 8, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count:');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_PFOR_BK_ONE'   , 9, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: Instances that pfor was adjusted in TX interrupt');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_PXPRV_FWD_ONE' ,10, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: Instances that pxprv was adjusted in for loop');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_RX0ERR_CT'	   ,11, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: FIFO 0 overrun');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_RX1ERR_CT'	   ,12, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: FIFO 1 overrun');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN_CP1CP2'	   ,13, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: (RQCP1 | RQCP2) unexpectedly ON');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_TXINT_EMPTYLIST'   ,14, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: TX interrupt with pending list empty');
INSERT INTO READINGS_BOARD VALUES ('PROG_TENSION_READINGS_BOARD_CAN1_BOGUS_CT'     ,15, 'TYP_U32' ,'%u',  'PROG_TENSION', 'TENSION_a', 'Count: bogus CAN1 IDs rejected');

