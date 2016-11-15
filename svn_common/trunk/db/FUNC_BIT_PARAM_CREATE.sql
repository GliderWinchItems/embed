-- 04/12/2015
-- Define bits in second byte of group polling msg
-- Each FUNCTION_TYPE has a bit assigned.
-- When the bit matches the bit in the group poll msg
--  and the the drum/group number matches in the first
--  byte, the function sends it measurement.
--
DROP TABLE FUNC_BIT_PARAM;
--
CREATE TABLE FUNC_BIT_PARAM
(
FUNC_BIT_PARAM_NAME varchar(48) PRIMARY KEY,
FUNC_BIT_PARAM_VAL varchar(24),
FUNCTION_TYPE varchar(24),
FORMAT varchar(24),
DESCRIPTION5 varchar(128) NOT NULL UNIQUE
);

