DELETE FROM NUMBER_TYPE;
-- 05/10/2016
--
-- TYPE_NAME varchar(24) PRIMARY KEY,
--   Arbitrary name for the number (unique)
--
-- TYPE_CODE numeric(4) UNIQUE,
--   Simple numeric code associated with the name
--  NOTE: code must not be zero as that is used for "NULL code"
--
-- TYPE_CT   numeric(4),
--   Number of bytes (except for CANID which is a special case)
--
-- DESCRIPTION9 varchar(128) NOT NULL UNIQUE
--   A few to help clarify
--
--                             TYPE_NAME TYPE_CODE TYPE_CT DESCRIPTION9
INSERT INTO NUMBER_TYPE VALUES ('TYP_S8',    1,1,'  int8_t,   signed char, 1 byte');
INSERT INTO NUMBER_TYPE VALUES ('TYP_U8',    2,1,' uint8_t, unsigned char, 1 byte');
INSERT INTO NUMBER_TYPE VALUES ('TYP_S16',   3,2,' int16_t,   signed short, 2 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_U16',   4,2,'uint16_t, unsigned short, 2 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_S32',   5,4,' int32_t,   signed int, 4 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_U32',   6,4,'uint32_t, unsigned int, 4 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_S64_L', 7,4,' int64_t,   signed long long, low  order 4 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_S64_H', 8,4,' int64_t,   signed long long, high order 4 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_U64_L', 9,4,'uint64_t, unsigned long long, low  order 4 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_U64_H',10,4,'uint64_t, unsigned long long, high order 4 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_FLT',  11,4,'float, 4 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_12FLT',12,2,'half-float, 2 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_34FLT',13,3,'3/4-float, 3 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_DBL_L',14,4,'double, low  order 4 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_DBL_H',15,4,'double, high order 4 bytes');
INSERT INTO NUMBER_TYPE VALUES ('TYP_ASC',  16,4,'ascii chars');
-- CAUTION: 17 for TYP_CANID is hard coded into a the java program that generates the command CAN ID table.
INSERT INTO NUMBER_TYPE VALUES ('TYP_CANID',17,1,'CANID (handled differently than a U32)');

