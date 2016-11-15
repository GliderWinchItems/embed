-- READINGS_LIST_INSERT.sql  Defines the readings by type of function.
-- 04/11/2015
--
-- READINGS_NAME (Unique name for value to be xmitted)
-- READINGS_CODE (Used to generate a "#define READINGS_NAME READINGS_CODE" for use in the command msg)
-- TYPE_NAME (join with NUMBER_TYPE table to get a numeric code for the type of number (e.g. unit32_t = 6)
--    See NUMBER_TYPE table.
-- FORMAT (Format to use for displaying the number)
-- FUNCTION_TYPE (join to FUNCTIONS.)  See FUNCTIONS table.
-- DESCRIPTION16 (Nonsense for the hapless Op and a crutch for the Wizard programmer; but must be unique)
--
-- READINGS_LIST differs from PARAM_LIST in how it is handled.
--
-- Readings are a retrieval of four bytes from some fixed memory location.
-- These locations typically have a counter, or some value being updated periodically.
--
-- The retrieval program uses a *fixed* pointer to scattered locations, whereas,
--   the PARAM_LIST will be accessed by a pointer to various locations where a table
--   of the values are stored.  The location of the pointer may change when parameters are updated, whereas,
--   the locations of the readings do not change.
--
-- Note: Some parameters may be changed in SRAM, e.g. offset and scale for one of the AD7799s, however the current
--   value being used in SRAM can be retrieved via the parameter list that loads the SRAM at startup or "revert".
--
DELETE FROM READINGS_LIST;
--                                    Reading name              Code   Type     format Function_type   Description16
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_FILTADC_THERM1' , 1, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: double thrm[0]; Filtered ADC for Thermistor on AD7799');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_FILTADC_THERM2' , 2, 'TYP_U32' ,'%u',	 'TENSION_a', 'Tension: READING: double thrm[1]; Filtered ADC for Thermistor external');
-- code #3 was skipped for no good reason!
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_FORMCAL_THERM1' , 4, 'TYP_FLT' ,'%0.2f', 'TENSION_a', 'Tension: READING: double degX[0]; Formula computed thrm for Thermistor on AD7799');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_FORMCAL_THERM2' , 5, 'TYP_FLT' ,'%0.2f', 'TENSION_a', 'Tension: READING: double degX[1]; Formula computed thrm for Thermistor external');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_POLYCAL_THERM1' , 6, 'TYP_FLT' ,'%0.2f', 'TENSION_a', 'Tension: READING: double degC[0]; Polynomial adjusted degX for Thermistor on AD7799');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_POLYCAL_THERM2' , 7, 'TYP_FLT' ,'%0.2f', 'TENSION_a', 'Tension: READING: double degC[1]; Polynomial adjusted degX for Thermistor external');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_AD7799_LGR'     , 8, 'TYP_S32' ,'%u', 	 'TENSION_a', 'Tension: READING: int32_t lgr; last_good_reading (no filtering or adjustments)');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_AD7799_CALIB_1' , 9, 'TYP_FLT' ,'0.3f',  'TENSION_a', 'Tension: READING: ten_iircal[0];  AD7799 filtered (fast) and calibrated');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_AD7799_CALIB_2' ,10, 'TYP_FLT' ,'0.3f',  'TENSION_a', 'Tension: READING: ten_iircal[1];  AD7799 filtered (slow) and calibrated');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_CIC_RAW'        ,11, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: int32_t cicraw; cic before averaging');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_CIC_AVE'        ,12, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: int32_t cicave; cic averaged for determining offset');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_CIC_AVE_CT'     ,13, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: int32_t ave.n;  current count for above average');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_OFFSET_REG'	,14, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: Last reading of AD7799 offset register');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_OFFSET_REG_FILT',15, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: Last filtered AD7799 offset register');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_OFFSET_REG_RDBK',16, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: Last filtered AD7799 offset register set read-back');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_FULLSCALE_REG'	,17, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: Last reading of AD7799 fullscale register');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_POLL_MASK'	,18, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: Mask for first two bytes of a poll msg (necessary?)');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_READINGSCT'	,19, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: Running count of readings (conversions completed)');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_READINGSCT_LASTPOLL',20, 'TYP_U32' ,'%u','TENSION_a', 'Tension: READING: Reading count the last time a poll msg sent');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_OFFSET_CT'	,21, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: Running ct of offset updates');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_ZERO_FLAG'	,22, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: 1 = zero-calibration operation competed');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_STATUS_BYTE'	,23, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: Reading status byte');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_IIR_OFFSET_K'	,24, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: IIR filter for offsets: parameter for setting time constant');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_IIR_OFFSET_SCL'	,25, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: IIR filter for offsets: Scaling to improve spare bits with integer math');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_IIR_Z_RECAl_W_K',26, 'TYP_U32' ,'%u', 	 'TENSION_a', 'Tension: READING: IIR filter for zeroing: parameter for setting time constant');
INSERT INTO READINGS_LIST VALUES ('TENSION_READ_IIR_Z_RECAl_W_SCL',27, 'TYP_U32' ,'%u',  'TENSION_a', 'Tension: READING: IIR filter for zeroing: Scaling to improve spare bits with integer math');


