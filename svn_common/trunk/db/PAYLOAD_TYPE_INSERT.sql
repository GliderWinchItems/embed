-- PAYLOAD_TYPE Name of payload arrangement versus code number for calling a routine
--   CANID.CAN_MSG_FMT specifies the PAYLOAD_TYPE_NAME from the following list, and 
--   provides a code that can be used to call a routine to parse the payload.
--
-- PAYLOAD_TYPE_NAME varchar(24) PRIMARY KEY,
--  Name assigned to this payload layout
--
-- PAYLOAD_TYPE_CODE numeric(4) NOT NULL UNIQUE,
--  Code that associates with the name. 
--
-- PAYLOAD_TYPE_DLC numeric(4) NOT NULL,
--  Number of bytes in the payload
-- DESCRIPTION12 varchar(128) NOT NULL UNIQUE
--  Helpful words for the puzzled programmer
--
DELETE from PAYLOAD_TYPE;
--   payload -- [0] - [7]	(-- = conversion routine written for to/from payload: 'pay_type_cnvt.c')
--                                               PAYLOAD_TYPE_DLC
--                      PAYLOAD_TYPE_NAME  PAYLOAD_TYPE_CODE    DESCRIPTION12  ([] = payload array index)                   
INSERT INTO PAYLOAD_TYPE VALUES ('NONE',	 0,	0, ' No payload bytes');
INSERT INTO PAYLOAD_TYPE VALUES ('FF',		 1,	4, ' [0]-[3]: Full Float');			--
INSERT INTO PAYLOAD_TYPE VALUES ('FF_FF',	 2,	8, ' [0]-[3]: Full Float[0]; [4]-[7]: Full Float[1]');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U32',		 3,	4, ' [0]-[3]: uint32_t');				--
INSERT INTO PAYLOAD_TYPE VALUES ('U32_U32',	 4,	8, ' [0]-[3]: uint32_t[0]; [4]-[7]: uint32_t[1]');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U8_U32',	 5,	5, ' [0]: uint8_t; [1]-[4]: uint32_t');		--
INSERT INTO PAYLOAD_TYPE VALUES ('S32',		 6,	4, ' [0]-[3]: int32_t');				--
INSERT INTO PAYLOAD_TYPE VALUES ('S32_S32',	 7,	8, ' [0]-[3]: int32_t[0]; [4]-[7]: int32_t[1]');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U8_S32',	 8,	5, ' [0]: int8_t; [4]-[7]: int32_t');		--
INSERT INTO PAYLOAD_TYPE VALUES ('HF',		 9,	2, ' [0]-[1]: Half-Float');			--
INSERT INTO PAYLOAD_TYPE VALUES ('F34F',	10,	3, ' [0]-[2]: 3/4-Float');				--
INSERT INTO PAYLOAD_TYPE VALUES ('xFF',		11,	5, ' [0]:[1]-[4]: Full-Float, first   byte  skipped');	--
INSERT INTO PAYLOAD_TYPE VALUES ('xxFF',	12,	6, ' [0]:[1]:[2]-[5]: Full-Float, first 2 bytes skipped');	--
INSERT INTO PAYLOAD_TYPE VALUES ('xxU32',	13,	6, ' [0]:[1]:[2]-[5]: uint32_t, first 2 bytes skipped');	--
INSERT INTO PAYLOAD_TYPE VALUES ('xxS32',	14,	6, ' [0]:[1]:[2]-[5]: int32_t, first 2 bytes skipped');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U8_U8_U32',	15,	6, ' [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1],uint32_t,');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U8_U8_S32',	16,	6, ' [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], int32_t,');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U8_U8_FF',	17,	6, ' [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], Full Float,');--
INSERT INTO PAYLOAD_TYPE VALUES ('U16',		18,	2, ' [0]-[2]uint16_t');			--
INSERT INTO PAYLOAD_TYPE VALUES ('S16',		19,	2, ' [0]-[2] int16_t');			--
INSERT INTO PAYLOAD_TYPE VALUES ('LAT_LON_HT',	20,	6, ' [0]:[1]:[2]-[5]: Fix type, bits fields, lat/lon/ht');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U8_FF',	21,	5, ' [0]:[1]-[4]: uint8_t, Full Float');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U8_HF',	22,	3, ' [0]:[1]-[2]: uint8_t, Half Float');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U8',		23,	1, ' [0]: uint8_t');	--
INSERT INTO PAYLOAD_TYPE VALUES ('UNIXTIME',	24,	5, ' [0]: U8_U32 with U8 bit field stuff');	--
INSERT INTO PAYLOAD_TYPE VALUES ('U8_U8',	25,	2, ' [0]:[1]: uint8_t[0],uint8[1]');	--

INSERT INTO PAYLOAD_TYPE VALUES ('LVL2B',	249,	6, ' [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Board code),[2]-[5]see table');	--
INSERT INTO PAYLOAD_TYPE VALUES ('LVL2R',	250,	6, ' [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Readings code),[2]-[5]see table');	--
INSERT INTO PAYLOAD_TYPE VALUES ('UNDEF',	255,	8, ' Undefined');			--
