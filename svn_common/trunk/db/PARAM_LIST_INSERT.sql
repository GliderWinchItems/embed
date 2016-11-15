-- PARAM_LIST_INSERT.sql  Defines the parameters for each different type of function.
-- 04/11/2015
--
-- PARAM_NAME (Unique name for parameter)
-- PARAM_CODE (Used to generate a "#define PARAM_NAME	PARAM_CODE" for use in C programs)
-- TYPE_NAME (join with NUMBER_TYPE table to get a numeric code for the type of number (e.g. unit32_t = 6)
--    See NUMBER_TYPE table.
-- FORMAT (Format to use for displaying the number)
-- FUNCTION_TYPE (join to FUNCTIONS.)  See FUNCTIONS table.
-- DESCRIPTION10 (Nonsense for the hapless Op and a crutch for the Wizard programmer; but must be unique)
--
--  A single *program* may be used for multiple instances of the same function, and the
--    parameter *list* will be identical.  However, the *values* may differ.  These  
--    values are specified in PARAM_VAL table.
--
-- PARAM_LIST table differs from READINGS_LIST table in how it is handled.
--
-- Readings are a retrieval of four bytes from some *fixed* memory location.
--   These locations typically have a counter, or some value being updated periodically
--   e.g. ADC readings.
--
-- Parameters are retrieved and set using a pointer to a struct.  This pointer might change dynamically
--   as parameters are updated, tested, saved, reverted.
--
-- For transport the parameters and readings are handled the same way, and the lists have the same
--   structure.
--
-- Code 0 reserved for SIZE
--
DELETE from PARAM_LIST;
--
-- Cansender 
--                              Parameter name                 Code     Type  format Function_type       Description10
INSERT INTO PARAM_LIST VALUES ('CANSENDER_LIST_CRC'	   	, 1, 'TYP_U32','%08X', 	'CANSENDER',	'Cansender: CRC ');
INSERT INTO PARAM_LIST VALUES ('CANSENDER_LIST_VERSION'      	, 2, 'TYP_S32','%d', 	'CANSENDER',	'Cansender: Version number');
INSERT INTO PARAM_LIST VALUES ('CANSENDER_HEARTBEAT_CT'		, 3, 'TYP_U32','%d',	'CANSENDER',	'Cansender: Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_LIST VALUES ('CANSENDER_HEARTBEAT_MSG'    	, 4, 'TYP_CANID','%x', 	'CANSENDER',  	'Cansender: CANID: Hearbeat sends running count');
INSERT INTO PARAM_LIST VALUES ('CANSENDER_POLL'	    		, 5, 'TYP_CANID','%x', 	'CANSENDER',  	'Cansender: CANID: Poll this cansender');
INSERT INTO PARAM_LIST VALUES ('CANSENDER_POLL_R'	  	, 6, 'TYP_CANID','%x', 	'CANSENDER',  	'Cansender: CANID: Response to POLL');

--
-- AD7799 Tension
--                              Parameter name                Code     Type  format     Function_type            Description
INSERT INTO PARAM_LIST VALUES ('TENSION_a_LIST_CRC'	     ,	 1, 'TYP_U32','%08X', 	'TENSION_a',	'Tension_a: crc: CRC for tension list');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_LIST_VERSION'      ,	 2, 'TYP_S32','%d', 	'TENSION_a',	'Tension_a: version: Version number for Tension List');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_AD7799_1_OFFSET'   ,	 3, 'TYP_S32','%d', 	'TENSION_a',	'Tension_a: offset: AD7799 #1 offset');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_AD7799_1_SCALE'    ,	 4, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: scale: AD7799 #1 Scale (convert to kgf)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_CONST_B'    ,	 5, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor1 param: B: constant B');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_R_SERIES'   ,	 6, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor1 param: RS: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_R_ROOMTMP'  ,	 7, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor1 param: R0: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_REF_TEMP'   ,	 8, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor1 param: TREF: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_TEMP_OFFSET',	 9, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor1 param: offset: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_TEMP_SCALE' ,	10, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor1 param: B: scale:  Thermistor temp scale correction');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_CONST_B'    ,	11, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor2 param: RS: constant B');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_R_SERIES'   ,	12, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor2 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_R_ROOMTMP'  ,	13, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor2 param: R0: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_REF_TEMP'   ,	14, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor2 param: TREF: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_TEMP_OFFSET',	15, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor2 param: offset: hermistor temp offset correction (deg C)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_TEMP_SCALE' ,	16, 'TYP_FLT','%f', 	'TENSION_a',	'Tension_a: Thermistor2 param: scale: Thermistor temp scale correction');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_COEF_0'     ,	17, 'TYP_FLT','%f',	'TENSION_a',	'Tension_a: Thermistor1 param: comp_t1[0]: Load-Cell polynomial coefficient 0 (offset)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_COEF_1'     ,	18, 'TYP_FLT','%f',	'TENSION_a',	'Tension_a: Thermistor1 param: comp_t1[1]: Load-Cell polynomial coefficient 1 (scale)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_COEF_2'     ,	19, 'TYP_FLT','%f',	'TENSION_a',	'Tension_a: Thermistor1 param: comp_t1[2]: Load-Cell polynomial coefficient 2 (x^2)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM1_COEF_3'     ,	20, 'TYP_FLT','%f',	'TENSION_a',	'Tension_a: Thermistor1 param: comp_t1[3]: Load-Cell polynomial coefficient 3 (x^3)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_COEF_0'     ,	21, 'TYP_FLT','%f',	'TENSION_a',	'Tension_a: Thermistor2 param: comp_t2[0]: Load-Cell polynomial coefficient 0 (offset)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_COEF_1'     ,	22, 'TYP_FLT','%f',	'TENSION_a',	'Tension_a: Thermistor2 param: comp_t2[1]: Load-Cell polynomial coefficient 1 (scale)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_COEF_2'     ,	23, 'TYP_FLT','%f',	'TENSION_a',	'Tension_a: Thermistor2 param: comp_t2[2]: Load-Cell polynomial coefficient 2 (x^2)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_THERM2_COEF_3'     ,	24, 'TYP_FLT','%f',	'TENSION_a',	'Tension_a: Thermistor2 param: comp_t2[3]: Load-Cell polynomial coefficient 3 (x^3)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_HEARTBEAT_CT'      ,	25, 'TYP_U32','%u', 	'TENSION_a',	'Tension_a: hbct: Heart-Beat Count of time (milliseconds) between autonomous msgs');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_DRUM_NUMBER'       ,	26, 'TYP_U32','%u', 	'TENSION_a',	'Tension_a: drum: Drum system number for this function instance');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_DRUM_FUNCTION_BIT' ,	27, 'TYP_U32','%02x', 	'TENSION_a',	'Tension_a: f_pollbit: Drum system poll 1st payload byte bit for drum # (function instance)');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_DRUM_POLL_BIT'     ,	28, 'TYP_U32','%02x',   'TENSION_a',	'Tension_a: p_pollbit: Drum system poll 2nd payload byte bit for this type of function');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANPRM_TENSION'    ,	29, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID: cid_ten_msg:  canid msg Tension');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_MSG_TIME_POLL'     ,	30, 'TYP_CANID','%x', 	'TENSION_a',  	'Tension_a: CANID: cid_ten_poll:  canid MC: Time msg/Group polling');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_TIMESYNC'          ,	31, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID: cid_gps_sync: canid time: GPS time sync distribution');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_HEARTBEAT'         ,	32, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID: heartbeat');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANIDTEST'         ,	33, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID: testing java program');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_IIR_POLL_K'        ,	34, 'TYP_U32','%u',  	'TENSION_a',  	'Tension_a: IIR Filter factor: divisor sets time constant: reading for polled msg');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_IIR_POLL_SCALE'    ,	35, 'TYP_U32','%u',  	'TENSION_a',  	'Tension_a: IIR Filter scale : upscaling (due to integer math): for polled msg');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_IIR_HB_K'          ,	36, 'TYP_U32','%u',  	'TENSION_a',  	'Tension_a: IIR Filter factor: divisor sets time constant: reading for heart-beat msg');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_IIR_HB_SCALE'      ,	37, 'TYP_U32','%u',  	'TENSION_a',  	'Tension_a: IIR Filter scale : upscaling (due to integer math): for heart-beat msg');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_USEME'   	     ,	38, 'TYP_U32','%08X',  	'TENSION_a',  	'Tension_a: skip or use this function switch');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_IIR_Z_RECAL_K'     ,	39, 'TYP_U32','%u',  	'TENSION_a',  	'Tension_a: IIR Filter factor: divisor sets time constant: zero recalibration');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_IIR_Z_RECAL_SCALE' ,	40, 'TYP_U32','%u',  	'TENSION_a',  	'Tension_a: IIR Filter scale : upscaling (due to integer math): zero recalibration');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_Z_RECAL_CT'        ,	41, 'TYP_U32','%u',  	'TENSION_a',  	'Tension_a: ADC conversion counts between zero recalibrations');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_LIMIT_HI'          ,	42, 'TYP_FLT','%f',  	'TENSION_a',  	'Tension_a: Exceeding this calibrated limit (+) means invalid reading');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_LIMIT_LO'          ,	43, 'TYP_FLT','%f', 	'TENSION_a',  	'Tension_a: Exceeding this calibrated limit (-) means invalid reading');

INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANID_HW_FILT1'    ,	44, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID1 parameter in this list for CAN hardware filter to allow');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANID_HW_FILT2'    ,	45, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID2 CANID parameter in this list for CAN hardware filter to allow');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANID_HW_FILT3'    ,	46, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID3 CANID parameter in this list for CAN hardware filter to allow');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANID_HW_FILT4'    ,	47, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID4 CANID parameter in this list for CAN hardware filter to allow');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANID_HW_FILT5'    ,	48, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID5 CANID parameter in this list for CAN hardware filter to allow');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANID_HW_FILT6'    ,	49, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID6 CANID parameter in this list for CAN hardware filter to allow');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANID_HW_FILT7'    ,	50, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID7 CANID parameter in this list for CAN hardware filter to allow');
INSERT INTO PARAM_LIST VALUES ('TENSION_a_CANID_HW_FILT8'    ,	51, 'TYP_CANID','%x',  	'TENSION_a',  	'Tension_a: CANID8 CANID parameter in this list for CAN hardware filter to allow');
--
-- Cable angle 
--                              Parameter name                 Code     Type  format Function_type                   Description
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_LIST_CRC'	   	, 1, 'TYP_U32','%08X', 	'CABLE_ANGLE',	'Cable Angle: CRC for cable angle list');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_LIST_VERSION'      	, 2, 'TYP_S32','%d', 	'CABLE_ANGLE',	'Cable Angle: Version number for Cable Angle List');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_HEARTBEAT_CT'      	, 3, 'TYP_U32','%u', 	'CABLE_ANGLE',	'Cable Angle: Heart-Beat: Count of time ticks between autonomous msgs');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_DRUM_NUMBER'        , 4, 'TYP_U32','%u', 	'CABLE_ANGLE',	'Cable Angle: drum: Drum system number for this function instance');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_DRUM_FUNCTION_BIT'  , 5, 'TYP_U32','%02x', 	'CABLE_ANGLE',	'Cable Angle: f_pollbit: Drum system poll 1st payload byte bit for drum # (function instance)');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_DRUM_POLL_BIT'      , 6, 'TYP_U32','%02x',  'CABLE_ANGLE',	'Cable Angle: p_pollbit: Drum system poll 2nd payload byte bit for this type of function');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_MIN_TENSION'	, 7, 'TYP_FLT','%0.3f',	'CABLE_ANGLE',	'Cable Angle: Minimum tension required (units to match)');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_RATE_CT'		, 8, 'TYP_U32','%u',   	'CABLE_ANGLE',	'Cable Angle: Rate count: Number of tension readings between cable angle msgs');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_ALARM_REPEAT'	, 9, 'TYP_U32','%u',   	'CABLE_ANGLE',	'Cable Angle: Number of times alarm msg is repeated');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_CALIB_COEF_0'	,10, 'TYP_FLT','%f',   	'CABLE_ANGLE',	'Cable Angle: Cable angle polynomial coefficient 0');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_CALIB_COEF_1'	,11, 'TYP_FLT','%f',   	'CABLE_ANGLE',	'Cable Angle: Cable angle polynomial coefficient 1');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_CALIB_COEF_2'	,12, 'TYP_FLT','%f',   	'CABLE_ANGLE',	'Cable Angle: Cable angle polynomial coefficient 2');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_CALIB_COEF_3'	,13, 'TYP_FLT','%f',   	'CABLE_ANGLE',	'Cable Angle: Cable angle polynomial coefficient 3');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_CANPRM_TENSION'     ,14, 'TYP_CANID','%x',  'CABLE_ANGLE',  'Cable Angle: CANID: can msg tension for sheave load-pin');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_MSG_TIME_POLL'      ,15, 'TYP_CANID','%x', 	'CABLE_ANGLE',  'Cable Angle: CANID: cid_ten_poll: canid MC: Time msg/Group polling');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_TIMESYNC'           ,16, 'TYP_CANID','%x',  'CABLE_ANGLE',  'Cable Angle: CANID: cid_gps_sync: canid time: GPS time sync distribution');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_HEARTBEAT_MSG'     	,17, 'TYP_CANID','%x', 	'CABLE_ANGLE',	'Cable Angle: CANID: Heart-Beat: msg');
INSERT INTO PARAM_LIST VALUES ('CABLE_ANGLE_USEME'   	        ,18, 'TYP_U32','%08X', 	'CABLE_ANGLE',	'Cable Angle:  skip or use this function switch');
--
-- GPS
--                              Parameter name                 Code     Type  format Function_type       Description
INSERT INTO PARAM_LIST VALUES ('GPS_LIST_CRC'	   		, 1, 'TYP_U32','%08X', 	'GPS',		'GPS: CRC ');
INSERT INTO PARAM_LIST VALUES ('GPS_LIST_VERSION'      		, 2, 'TYP_S32','%d', 	'GPS',		'GPS: Version number');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_TIME_CT'		, 3, 'TYP_U32','%d',	'GPS',		'GPS: Time (ms) between unix time msgs');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_LLH_CT'		, 4, 'TYP_U32','%d',	'GPS',		'GPS: Time (ms) between burst of lat lon height msgs');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_LLH_DELAY_CT'	, 5, 'TYP_U32','%d',	'GPS',		'GPS: Time (ms) between lat/lon and lon/ht msgs');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_TIME'    		, 6, 'TYP_CANID','%x', 	'GPS',  	'GPS: Heartbeat unix time');
INSERT INTO PARAM_LIST VALUES ('GPS_HEARTBEAT_LLH'  		, 7, 'TYP_CANID','%x', 	'GPS',  	'GPS: Heartbeat (3 separate msgs) lattitude longitude height');
INSERT INTO PARAM_LIST VALUES ('GPS_DISABLE_SYNCMSGS'		, 8, 'TYP_U32','%d',	'GPS',		'GPS: time sync msgs; 0 = enable  1 = disable');
INSERT INTO PARAM_LIST VALUES ('GPS_TIME_SYNC_MSG'    		, 9, 'TYP_CANID','%x', 	'GPS',  	'GPS: Time sync msg');

--
-- Logger
--                              Parameter name                 Code     Type  format Function_type       Description
INSERT INTO PARAM_LIST VALUES ('LOGGER_LIST_CRC'	   	, 1, 'TYP_U32','%08X', 	'LOGGER',	'Logger: CRC ');
INSERT INTO PARAM_LIST VALUES ('LOGGER_LIST_VERSION'      	, 2, 'TYP_S32','%d', 	'LOGGER',	'Logger: Version number');
INSERT INTO PARAM_LIST VALUES ('LOGGER_HEARTBEAT1_CT'		, 3, 'TYP_U32','%d',	'LOGGER',	'Logger: Heartbeat count of time (ms) between msgs');
INSERT INTO PARAM_LIST VALUES ('LOGGER_HEARTBEAT_MSG'    	, 4, 'TYP_CANID','%x', 	'LOGGER',  	'Logger: CANID: Hearbeat sends running count of logged msgs');

---
-- Engine sensor
--                              Parameter name            Code     Type  format Function_type            Description
INSERT INTO PARAM_LIST VALUES ('ENGINE__CRC',			 2, 'TYP_U32','%08X', 'ENGINE_SENSOR',	'Engine_sensor: CRC for cable angle list');
INSERT INTO PARAM_LIST VALUES ('ENGINE__VERSION', 		 3, 'TYP_S32',  '%d', 'ENGINE_SENSOR',	'Engine_sensor: Version number for Cable Angle List');
INSERT INTO PARAM_LIST VALUES ('ENGINE_SENSOR_SEG_CT',		 4, 'TYP_U32',  '%u', 'ENGINE_SENSOR',	'Engine_sensor: Number of black (or white) segments');
INSERT INTO PARAM_LIST VALUES ('ENGINE_SENSOR_PRESS_OFFSET',	 5, 'TYP_FLT',  '%f', 'ENGINE_SENSOR',	'Engine_sensor: Manifold pressure offset');
INSERT INTO PARAM_LIST VALUES ('ENGINE_SENSOR_PRESS_SCALE',	 6, 'TYP_FLT',  '%f', 'ENGINE_SENSOR',	'Engine_sensor: Manifold pressure  scale (inch Hg)');
INSERT INTO PARAM_LIST VALUES ('ENGINE_THERM1_CONST_B' ,         7, 'TYP_FLT',  '%f', 'ENGINE_SENSOR',	'Engine_sensor: Thermistor param: constant B');
INSERT INTO PARAM_LIST VALUES ('ENGINE_THERM1_R_SERIES' ,        8, 'TYP_FLT',  '%f', 'ENGINE_SENSOR',	'Engine_sensor: Thermistor param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('ENGINE_THERM1_R_ROOMTMP',        9, 'TYP_FLT',  '%f', 'ENGINE_SENSOR',	'Engine_sensor: Thermistor param: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('ENGINE_THERM1_REF_TEMP' ,       10, 'TYP_FLT',  '%f', 'ENGINE_SENSOR',	'Engine_sensor: Thermistor param: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('ENGINE_THERM1_TEMP_OFFSET',     11, 'TYP_FLT',  '%f', 'ENGINE_SENSOR',	'Engine_sensor: Thermistor param: Thermistor temp offset correction (deg C)');

-- Yogurt maker
--                              Parameter name            Code     Type  format Function_type            Description
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_LIST_CRC'	  , 1, 'TYP_U32','%08X','YOGURT_1',	'Yogurt: crc: CRC for this list');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_LIST_VERSION'      , 2, 'TYP_S32','%d','YOGURT_1',	'Yogurt: version: Version number yogurt');

INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM1_CONST_B'    , 3, 'TYP_FLT','%f','YOGURT_1',	'Yogurt: Thermistor1 param: B: constant B');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM1_R_SERIES'   , 4, 'TYP_FLT','%f','YOGURT_1',	'Yogurt: Thermistor1 param: RS: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM1_R_ROOMTMP'  , 5, 'TYP_FLT','%f','YOGURT_1',	'Yogurt: Thermistor1 param: R0: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM1_REF_TEMP'   , 6, 'TYP_FLT','%f','YOGURT_1',	'Yogurt: Thermistor1 param: TREF: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM1_COEF_0',    7, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor1 param: poly[0]:  polynomial coefficient 0 (offset)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM1_COEF_1',    8, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor1 param: poly[1]:  polynomial coefficient 1 (scale)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM1_COEF_2',    9, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor1 param: poly[2]:  polynomial coefficient 2 (x^2)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM1_COEF_3',   10, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor1 param: poly[3]:  polynomial coefficient 3 (x^3)');

INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM2_CONST_B'    ,11, 'TYP_FLT','%f','YOGURT_1',	'Yogurt: Thermistor2 param: B: constant B');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM2_R_SERIES'   ,12, 'TYP_FLT','%f','YOGURT_1',	'Yogurt: Thermistor2 param: RS: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM2_R_ROOMTMP'  ,13, 'TYP_FLT','%f','YOGURT_1',	'Yogurt: Thermistor2 param: R0: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM2_REF_TEMP'   ,14, 'TYP_FLT','%f','YOGURT_1',	'Yogurt: Thermistor2 param: TREF: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM2_COEF_0',   15, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor2 param: poly[0]:  polynomial coefficient 0 (offset)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM2_COEF_1',   16, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor2 param: poly[1]:  polynomial coefficient 1 (scale)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM2_COEF_2',   17, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor2 param: poly[2]:  polynomial coefficient 2 (x^2)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM2_COEF_3',   18, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor2 param: poly[3]:  polynomial coefficient 3 (x^3)');

INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM3_CONST_B'   ,19, 'TYP_FLT','%f', 'YOGURT_1',	'Yogurt: Thermistor3 param: B: constant B');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM3_R_SERIES'  ,20, 'TYP_FLT','%f', 'YOGURT_1',	'Yogurt: Thermistor3 param: RS: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM3_R_ROOMTMP' ,21, 'TYP_FLT','%f', 'YOGURT_1',	'Yogurt: Thermistor3 param: R0: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM3_REF_TEMP'  ,22, 'TYP_FLT','%f', 'YOGURT_1',	'Yogurt: Thermistor3 param: TREF: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM3_COEF_0',   23, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor3 param: poly[0]:  polynomial coefficient 0 (offset)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM3_COEF_1',   24, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor3 param: poly[1]:  polynomial coefficient 1 (scale)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM3_COEF_2',   25, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor3 param: poly[2]:  polynomial coefficient 2 (x^2)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM3_COEF_3',   26, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor3 param: poly[3]:  polynomial coefficient 3 (x^3)');

INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM4_CONST_B'   ,27, 'TYP_FLT','%f', 'YOGURT_1',	'Yogurt: Thermistor4 param: B: constant B');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM4_R_SERIES'  ,28, 'TYP_FLT','%f', 'YOGURT_1',	'Yogurt: Thermistor4 param: RS: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM4_R_ROOMTMP' ,29, 'TYP_FLT','%f', 'YOGURT_1',	'Yogurt: Thermistor4 param: R0: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM4_REF_TEMP'  ,30, 'TYP_FLT','%f', 'YOGURT_1',	'Yogurt: Thermistor4 param: TREF: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM4_COEF_0',   31, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor4 param: poly[0]:  polynomial coefficient 0 (offset)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM4_COEF_1',   32, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor4 param: poly[1]:  polynomial coefficient 1 (scale)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM4_COEF_2',   33, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor4 param: poly[2]:  polynomial coefficient 2 (x^2)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_THERM4_COEF_3',   34, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Thermistor4 param: poly[3]:  polynomial coefficient 3 (x^3)');

INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTLTEMP_HEAT_PAST',   35, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Pasteur: Control set-point temperature (deg F) heat to this temp');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTLTEMP_DUR_PAST',    36, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Pasteur: Time duration at temp (hours.frac_hours)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTLTEMP_COOL_PAST',   37, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Pasteur: Control end-point temperature (deg F) cool to this temp');

INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTLTEMP_HEAT_FERM',   38, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Ferment: Control set-point temperature (deg F) heat to this temp');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTLTEMP_DUR_FERM',    39, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Ferment: Time duration at temp (hours.frac_hours)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTLTEMP_COOL_FERM',   40, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Ferment: Control end-point temperature (deg F) cool to this temp');

INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTL_THERM_SHELL', 	41, 'TYP_U32','%d', 	'YOGURT_1',	'Yogurt: Thermistor number for shell temp (0 - 3)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTL_THERM_POT',	42, 'TYP_U32','%d', 	'YOGURT_1',	'Yogurt: Thermistor number for center of pot temp (0 - 3)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTL_THERM_AIRIN',	43, 'TYP_U32','%d', 	'YOGURT_1',	'Yogurt: Thermistor number for air inlet to fan temp (0 - 3)');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTL_THERM_AIROUT',	44, 'TYP_U32','%d', 	'YOGURT_1',	'Yogurt: Thermistor number for air coming out of holes (0 - 3)');


INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTL_THERM_LOOP_P',	45, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Control loop: Proportional coefficient');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTL_THERM_LOOP_I',	46, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Control loop: Integral coefficient');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CTL_THERM_LOOP_D',	47, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Control loop: Derivative coefficient');

INSERT INTO PARAM_LIST VALUES ('YOGURT_1_CMD'      ,48,'TYP_CANID','%x', 'YOGURT_1',  	'Yogurt: CANID: cid_yog_cmd: Yogurt maker parameters');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_MSG'      ,49,'TYP_CANID','%x', 'YOGURT_1',  	'Yogurt: CANID: cid_yog_msg: Yogurt maker msgs');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_HB'       ,50,'TYP_CANID','%x', 'YOGURT_1',  	'Yogurt: CANID: cid_yog_hb: Yogurt maker heart-beats');

INSERT INTO PARAM_LIST VALUES ('YOGURT_1_HEATCONSTANT_KM_P',   	51, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Control, stored heat constant Pasteur phase');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_HEATCONSTANT_KM_M',   	52, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Control, stored heat constant Ferment phase');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_INTEGRATEINIT_A',     	53, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Control, integrator initialization, a of  a + b*x ');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_INTEGRATEINIT_B',     	54, 'TYP_FLT','%f', 	'YOGURT_1',	'Yogurt: Control, integrator initialization, b of  a + b*x ');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_STABILIZETIMEDELAY_P',	55, 'TYP_U32','%d', 	'YOGURT_1',	'Yogurt: Control, time delay for temperature stabilization, Pasteur');
INSERT INTO PARAM_LIST VALUES ('YOGURT_1_STABILIZETIMEDELAY_F',	56, 'TYP_U32','%d', 	'YOGURT_1',	'Yogurt: Control, time delay for temperature stabilization, Ferment');

