-- CANID table associates a CANID "name" with a hex value and usage type
-- 07/10/2015
--
-- CANID_NAME is used rather than a hex value, allowing changes to hex assignments without changing a program.
-- CANID_NAME varchar(48) PRIMARY KEY,
--  Each CAN ID is given a unique name.  A Java program produces a .h file with #define that connects the name with 
--  the hex code.  Programs are written using the name so that if there are changes in the hex code the source
--  code does not need to be changed and only recompiling is required.
--
--  Naming convention:
--  Prefix: CANID_
--  2nd group: 
--    MSG = Response to a poll
--    CMD = Function instance Command
--       Suffix: I or R (interrogate or response) 
--    HB  = Heartbeat (autonomous sending)
--    variable = Function instance name
--
-- CANID_HEX varchar(8) NOT NULL UNIQUE,
--  This is the hex code for the CAN ID left justified as defined in the STM32 Reference Manual, and as follows--
--
--  Bits in 32b word for CAN ID--
--
--   If a standard 11 bit CAN ID:
--      31:21 And, IDE (bit 2) is zero
--   If an extended CAN ID:
--      31:03 And, IDE (bit 2) is one
--
--   Lower value CAN IDs have higher CAN priority
--
--   Bit 1 RTR: Remote transmission request
--      0: Data frame
--      1: Remote frame
--
--   Bit 0 Reserved.  Used for triggering hardware to xmit
--
-- CANID_IR varchar(2) NOT NULL,
--  Designate a response or interrogate CAN ID
--  I = interrogate
--  R = response
--  ' ' = no designation
--
-- CANID_TYPE varchar(24) NOT NULL,
--  A clue as to where this CAN ID might be used
--  (This column may not be needed, i.e. redundant.)
--
-- CAN_MSG_FMT varchar(16) NOT NULL,
--  PAYLOAD_TYPE_NAME: Name for payload layout
--  'UNDEF' is valid name for "not defined" (i.e. default)
--
-- DESCRIPTION varchar(128) NOT NULL UNIQUE
--  Clever words for further eludication and identification (must be unique).  Since the description field is unique
--  a search can be made using the field.
--
-- Rebuild the table by--
-- Delete all the current values:
DELETE from CANID;
-- Then add everything that follows:                            CANID_IR
--                         CANID_NAME                    CANID_HEX   CANID_TYPE    CAN_MSG_FMT     DESCRIPTION
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_0',	'48000000', 'TENSION_a', 'U8_FF',	'Tension_0: Default measurement canid');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a11',	'38000000', 'TENSION_a', 'U8_FF','Tension_a11: Drum 1 calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a21',	'38200000', 'TENSION_a', 'U8_FF','Tension_a12: Drum 1 calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a12',	'38400000', 'TENSION_a', 'U8_FF','Tension_a21: Drum 2 calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_a22',	'38600000', 'TENSION_a', 'U8_FF','Tension_a22: Drum 2 calibrated tension, polled by time msg');
INSERT INTO CANID VALUES ('CANID_MSG_TENSION_2',	'38800000', 'TENSION_2', 'U8_FF','Tension_2: calibrated tension, polled by time msg');

INSERT INTO CANID VALUES ('CANID_TST_TENSION_a11',	'F800010C', 'TENSION_a', 'U8_FF','Tension_a11: TESTING java program generation of idx_v_val.c');
INSERT INTO CANID VALUES ('CANID_TST_TENSION_a12',	'F800020C', 'TENSION_a', 'U8_FF','Tension_a12: TESTING java program generation of idx_v_val.c');
INSERT INTO CANID VALUES ('CANID_TST_TENSION_a21',	'F800030C', 'TENSION_a', 'U8_FF','Tension_a21: TESTING java program generation of idx_v_val.c');
INSERT INTO CANID VALUES ('CANID_TST_TENSION_a22',	'F800040C', 'TENSION_a', 'U8_FF','Tension_a22: TESTING java program generation of idx_v_val.c');
--
-- Command CAN IDs for each function
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1WI',	'05C0003C', 'TENSION_a', 'U8_U8_U32','Tension_a: I 1W Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1WR',	'05C0803C', 'TENSION_a', 'U8_U8_U32','Tension_a: R 1W Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2WI',	'05C0004C', 'TENSION_a', 'U8_U8_U32','Tension_a: I 2W Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2WR',	'05C0804C', 'TENSION_a', 'U8_U8_U32','Tension_a: R 2W Command code: [0] command code, [1]-[8] depends on code');

INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a11I',	'05C00004', 'TENSION_a', 'U8_U8_U32','Tension_a11: I Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a11R',	'05C0000C', 'TENSION_a', 'U8_U8_U32','Tension_a11: R Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a21I',	'05E00004', 'TENSION_a', 'U8_U8_U32','Tension_a21: I Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a21R',	'05E0000C', 'TENSION_a', 'U8_U8_U32','Tension_a21: R Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a12I',	'F800005C', 'TENSION_a', 'U8_U8_U32','Tension_a12: I 2 AD7799 VE POD TESTING  3' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a12R',	'F800805C', 'TENSION_a', 'U8_U8_U32','Tension_a12: R 2 AD7799 VE POD TESTING  3' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a22I',	'F800006C', 'TENSION_a', 'U8_U8_U32','Tension_a22: I 2 AD7799 VE POD TESTING  3' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a22R',	'F800806C', 'TENSION_a', 'U8_U8_U32','Tension_a22: R 2 AD7799 VE POD TESTING  3' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a0XI',	'F800001C', 'TENSION_a', 'U8_U8_U32','Tension_a:  I 1 AD7799 VE POD TESTING (hence 0) 0' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a0XR',	'F800801C', 'TENSION_a', 'U8_U8_U32','Tension_a:  R 1 AD7799 VE POD TESTING (hence 0) 0' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1XI',	'F800002C', 'TENSION_a', 'U8_U8_U32','Tension_a:  I 2 AD7799 VE POD TESTING (hence X) 1' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1XR',	'F800802C', 'TENSION_a', 'U8_U8_U32','Tension_a:  R 2 AD7799 VE POD TESTING (hence X) 1' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2XI',	'F800003C', 'TENSION_a', 'U8_U8_U32','Tension_a2: I 2 AD7799 VE POD TESTING (hence X) 1' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2XR',	'F800803C', 'TENSION_a', 'U8_U8_U32','Tension_a2: R 2 AD7799 VE POD TESTING (hence X) 1' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1YI',	'F800004C', 'TENSION_a', 'U8_U8_U32','Tension_a:  I 1 AD7799 VE POD TESTING (hence Y) 2' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1YR',	'F800804C', 'TENSION_a', 'U8_U8_U32','Tension_a:  R 1 AD7799 VE POD TESTING (hence Y) 2' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1GI',	'F800007C', 'TENSION_a', 'U8_U8_U32','Tension_a:  I 2 AD7799 VE POD GSM 1st board sent (_16)' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a1GR',	'F800807C', 'TENSION_a', 'U8_U8_U32','Tension_a:  R 2 AD7799 VE POD GSM 1st board sent (_16)' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2GI',	'F800008C', 'TENSION_a', 'U8_U8_U32','Tension_a2: I 2 AD7799 VE POD GSM 1st board sent (_16)' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_a2GR',	'F800808C', 'TENSION_a', 'U8_U8_U32','Tension_a2: R 2 AD7799 VE POD GSM 1st board sent (_16)' );
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_2I',	'05C0005C', 'TENSION_2', 'U8_U8_U32','Tension_2: I Tension_2: Command code: [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_TENSION_2R',	'05C0805C', 'TENSION_2', 'U8_U8_U32','Tension_2: R Tension_2: Command code: [0] command code, [1]-[8] depends on code');

INSERT INTO CANID VALUES ('CANID_MOTOR_1',		'2D000000', 'MOTOR_1', 		'UNDEF','MOTOR_1: Motor speed');

INSERT INTO CANID VALUES ('CANID_CMD_CABLE_ANGLE_0I',	'06000000', 'CABLE_ANGLE_0', 	'UNDEF','Cable_Angle0: I Default measurement canid');
INSERT INTO CANID VALUES ('CANID_CMD_CABLE_ANGLE_0R',	'0600000C', 'CABLE_ANGLE_0', 	'UNDEF','Cable_Angle0: R Default measurement canid');
INSERT INTO CANID VALUES ('CANID_CMD_CABLE_ANGLE_1I',	'06200000', 'CABLE_ANGLE_1', 	'UNDEF','Cable_Angle1: I [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_CABLE_ANGLE_1R',	'0620000C', 'CABLE_ANGLE_1', 	'UNDEF','Cable_Angle1: R [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_MSG_CABLE_ANGLE_1',	'3A000000', 'CABLE_ANGLE_1', 	'UNDEF','Cable_Angle1: for drum #1');
INSERT INTO CANID VALUES ('CANID_MSG_CABLE_ANGLE_1_ALARM','2B000000','CABLE_ANGLE_1',	'UNDEF','Cable_Angle1: unreliable for drum #1');

INSERT INTO CANID VALUES ('CANID_CMD_ENGINE_SENSORI',	'80600000', 'ENGINE_SENSOR', 	'UNDEF','Engine: code: I [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_CMD_ENGINE_SENSORR',	'8060000C', 'ENGINE_SENSOR', 	'UNDEF','Engine: code: R [0] command code, [1]-[8] depends on code');
INSERT INTO CANID VALUES ('CANID_HB_ENG_RPMMANIFOLD',	'40600000', 'ENGINE_SENSOR', 	'UNDEF','Engine: rpm:manifold pressure');
INSERT INTO CANID VALUES ('CANID_HB_ENG_TEMP',		'70600000', 'ENGINE_SENSOR', 	'UNDEF','Engine: thermistor converted to temp');
INSERT INTO CANID VALUES ('CANID_HB_ENG_THERMTHROTL',	'60600000', 'ENGINE_SENSOR', 	'UNDEF','Engine: thermistor:throttle pot');
INSERT INTO CANID VALUES ('CANID_HB_ENG_THROTTLE',	'50600000', 'ENGINE_SENSOR', 	'UNDEF','Engine: throttle');

INSERT INTO CANID VALUES ('CANID_HB_FIX_HT_TYP_NSAT',	'B1C00000', 'GPS', 	'UNDEF','GPS: fix: heigth:type fix:number sats');
INSERT INTO CANID VALUES ('CANID_HB_FIX_LATLON',	'A1C00000', 'GPS', 	'UNDEF','GPS: fix: lattitude:longitude');
INSERT INTO CANID VALUES ('CANID_HB_LG_ER1',		'D1C00004', 'GPS', 	'UNDEF','GPS: 1st code  CANID-UNITID_CO_OLI GPS checksum error');
INSERT INTO CANID VALUES ('CANID_HB_LG_ER2',		'D1C00014', 'GPS', 	'UNDEF','GPS: 2nd code  CANID-UNITID_CO_OLI GPS Fix error');
INSERT INTO CANID VALUES ('CANID_HB_LG_ER3',		'D1C00024', 'GPS', 	'UNDEF','GPS: 3rd code  CANID-UNITID_CO_OLI GPS Time out of step');
INSERT INTO CANID VALUES ('CANID_HB_TIMESYNC',		'00400000', 'GPS', 	'UNDEF','GPS_1: GPS time sync distribution msg');
INSERT INTO CANID VALUES ('CANID_HB_TIMESYNC_2',	'00600000', 'GPS', 	'UNDEF','GPS_2: GPS time sync distribution msg');
INSERT INTO CANID VALUES ('CANID_HB_TIMESYNC_X',	'03000000', 'GPS', 	'UNDEF','GPS_2: Obsolete GPS time sync distribution msg');
INSERT INTO CANID VALUES ('CANID_HB_UNIVERSAL_RESET',	'00200000', 'GPS', 	'UNDEF','Highest priority: reserved for Universal (if/when implemented)');

INSERT INTO CANID VALUES ('CANID_CMD_GPS_1I',		'D1C00044', 'GPS',	'UNDEF','GPS_1: I CANID Command GPS 1');
INSERT INTO CANID VALUES ('CANID_CMD_GPS_1R',		'D1C0004C', 'GPS',	'UNDEF','GPS_1: R CANID Command GPS 1');
INSERT INTO CANID VALUES ('CANID_CMD_GPS_2I',		'D1C00074', 'GPS',	'UNDEF','GPS_2: I CANID Command GPS 2');
INSERT INTO CANID VALUES ('CANID_CMD_GPS_2R',		'D1C0007C', 'GPS',	'UNDEF','GPS_2: R CANID Command GPS 2');
INSERT INTO CANID VALUES ('CANID_CMD_LOGGER_1I',	'D1C00054', 'LOGGER','UNDEF','Logger_1: I Command Logger 1');
INSERT INTO CANID VALUES ('CANID_CMD_LOGGER_1R',	'D1C0005C', 'LOGGER','UNDEF','Logger_1: R Command Logger 1');
INSERT INTO CANID VALUES ('CANID_CMD_LOGGER_2I',	'D1C00064', 'LOGGER','UNDEF','Logger_2: I Command Logger 2');
INSERT INTO CANID VALUES ('CANID_CMD_LOGGER_2R',	'D1C0006C', 'LOGGER','UNDEF','Logger_2: R Command Logger 2');

INSERT INTO CANID VALUES ('CANID_MC_SYSTEM_STATE',	'50000000', 'MC',	'UNDEF','MC: System state msg');
INSERT INTO CANID VALUES ('CANID_MC_DRUM_SELECT',	'D0800814', 'MC', 	'UNDEF','MC: Drum selection');
INSERT INTO CANID VALUES ('CANID_HB_MC_MOTOR_1_KPALIVE','A0800000', 'MC', 	'UNDEF','MC: Curtis Controller keepalive');
INSERT INTO CANID VALUES ('CANID_MC_REQUEST_PARAM',	'D0800824', 'MC', 	'UNDEF','MC: Request parameters from HC');
INSERT INTO CANID VALUES ('CANID_MC_CONTACTOR',		'23000000', 'MC',	'UNDEF','MC: Contactor OPEN/CLOSE');
INSERT INTO CANID VALUES ('CANID_MC_BRAKES',		'21000000', 'MC',	'UNDEF','MC: Brakes APPLY/RELEASE');
INSERT INTO CANID VALUES ('CANID_MC_GUILLOTINE',	'22000000', 'MC',	'UNDEF','MC: Fire guillotine');
INSERT INTO CANID VALUES ('CANID_MC_RQ_LAUNCH_PARAM',	'27000000', 'MC',	'UNDEF','MC: Fire request launch parameters');
INSERT INTO CANID VALUES ('CANID_MSG_TIME_POLL',	'20000000', 'MC', 	'UNDEF','MC: Time msg/Group polling');
INSERT INTO CANID VALUES ('CANID_MC_STATE',		'26000000', 'MC', 	'UNDEF','MC: Launch state msg');
INSERT INTO CANID VALUES ('CANID_MC_TORQUE',		'25800000', 'MC', 	'UNDEF','MC: Motor torque');
INSERT INTO CANID VALUES ('CANID_CMD_MCLI',		'FFE00000', 'MCL', 	'UNDEF','MCL: Master Controller Launch parameters I (HC)');
INSERT INTO CANID VALUES ('CANID_CMD_MCLR',		'FFE00004', 'MCL', 	'UNDEF','MCL: Master Controller Launch parameters R (MC)');


INSERT INTO CANID VALUES ('CANID_CP_CTL_RMT',		'29000000', 'CP', 	'UNDEF','Control Panel: Control lever remote');
INSERT INTO CANID VALUES ('CANID_CP_CTL_LCL',		'29200000', 'CP', 	'UNDEF','Control Panel: Control lever local');
INSERT INTO CANID VALUES ('CANID_CP_CTL_IN_RMT',	'24C00000', 'CP', 	'UNDEF','Control Panel: Control lever remote: input');
INSERT INTO CANID VALUES ('CANID_CP_CTL_IN_LCL',	'25000000', 'CP', 	'UNDEF','Control Panel: Control lever  local: input');
INSERT INTO CANID VALUES ('CANID_CP_CTL_OUT_RMT',	'2A000000', 'CP', 	'UNDEF','Control Panel: Control lever output');

INSERT INTO CANID VALUES ('CANID_SE2H_ADC2_HistA',	'D0800044', 'SHAFT_LOWERSHV',	'UNDEF','Shaft encoder: Lower sheave:SE2: ADC2 HistogramA tx: request count, switch buffers; rx send count');
INSERT INTO CANID VALUES ('CANID_SE2H_ADC2_HistB',	'D0800054', 'SHAFT_LOWERSHV', 	'UNDEF','Shaft encoder: Lower sheave:SE2: ADC2 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_SE2H_ADC3_ADC2_RD',	'D0800064', 'SHAFT_LOWERSHV', 	'UNDEF','Shaft encoder: Lower sheave:SE2: ADC3 ADC2 readings readout');
INSERT INTO CANID VALUES ('CANID_SE2H_ADC3_HistA',	'D0800024', 'SHAFT_LOWERSHV', 	'UNDEF','Shaft encoder: Lower sheave:SE2: ADC3 HistogramA tx: request count, switch buffers. rx: send count');
INSERT INTO CANID VALUES ('CANID_SE2H_ADC3_HistB',	'D0800034', 'SHAFT_LOWERSHV', 	'UNDEF','Shaft encoder: Lower sheave:E2: ADC3 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_SE2H_COUNTERnSPEED',	'30800000', 'SHAFT_LOWERSHV', 	'UNDEF','Shaft encoder: Lower sheave:SE2: (Lower sheave) Count and speed');
INSERT INTO CANID VALUES ('CANID_SE2H_ERROR1',		'D0800014', 'SHAFT_LOWERSHV', 	'UNDEF','Shaft encoder: Lower sheave:SE2: error1');
INSERT INTO CANID VALUES ('CANID_SE2H_ERROR2',		'D0800074', 'SHAFT_LOWERSHV', 	'UNDEF','Shaft encoder: Lower sheave:SE2: error2');
INSERT INTO CANID VALUES ('CANID_CMD_LOWERSHVI',	'D0800000', 'SHAFT_LOWERSHV', 	'UNDEF','Shaft encoder: Lower sheave:SE2: I Command CAN: send commands to subsystem');
INSERT INTO CANID VALUES ('CANID_CMD_LOWERSHVR',	'D0800004', 'SHAFT_LOWERSHV', 	'UNDEF','Shaft encoder: Lower sheave:SE2: R Command CAN: send commands to subsystem');

INSERT INTO CANID VALUES ('CANID_SE3H_ADC2_HistA',	'D0A00044', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: ADC2 HistogramA tx: request count, switch buffers; rx send count');
INSERT INTO CANID VALUES ('CANID_SE3H_ADC2_HistB',	'D0A00054', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: ADC2 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_SE3H_ADC3_ADC2_RD',	'D0A00064', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: ADC3 ADC2 readings readout');
INSERT INTO CANID VALUES ('CANID_SE3H_ADC3_HistA',	'D0A00024', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: ADC3 HistogramA tx: request count, switch buffers. rx: send count');
INSERT INTO CANID VALUES ('CANID_SE3H_ADC3_HistB',	'D0A00034', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: ADC3 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_SE3H_COUNTERnSPEED',	'30A00000', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: (upper sheave) Count and Speed');
INSERT INTO CANID VALUES ('CANID_SE3H_ERROR1',		'D0A00014', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: error1');
INSERT INTO CANID VALUES ('CANID_SE3H_ERROR2',		'D0A00004', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: error2');
INSERT INTO CANID VALUES ('CANID_CMD_UPPERSHVI',	'D0600000', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: I Command CAN: send commands to subsystem');
INSERT INTO CANID VALUES ('CANID_CMD_UPPERSHVR',	'D0600004', 'SHAFT_UPPERSHV', 	'UNDEF','Shaft encoder: Upper sheave:SE3: R Command CAN: send commands to subsystem');

INSERT INTO CANID VALUES ('CANID_SE4H_ADC2_HistA',	'D0400044', 'DRIVE_SHAFT', 	'UNDEF','Drive shaft: ADC2 HistogramA tx: request count, switch buffers; rx send count');
INSERT INTO CANID VALUES ('CANID_SE4H_ADC2_HistB',	'D0400054', 'DRIVE_SHAFT', 	'UNDEF','Drive shaft: ADC2 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_SE4H_ADC3_ADC2_RD',	'D0400064', 'DRIVE_SHAFT', 	'UNDEF','Drive shaft: ADC3 ADC2 readings readout');
INSERT INTO CANID VALUES ('CANID_SE4H_ADC3_HistA',	'D0400024', 'DRIVE_SHAFT', 	'UNDEF','Drive shaft: ADC3 HistogramA tx: request count, switch buffers. rx: send count');
INSERT INTO CANID VALUES ('CANID_SE4H_ADC3_HistB',	'D0400034', 'DRIVE_SHAFT', 	'UNDEF','Drive shaft: ADC3 HistogramB tx: bin number, rx: send bin count');
INSERT INTO CANID VALUES ('CANID_CMD_DRIVE_SHAFTI',	'D0C00000', 'DRIVE_SHAFT', 	'UNDEF','Drive shaft: I Command CAN: send commands to subsystem');
INSERT INTO CANID VALUES ('CANID_CMD_DRIVE_SHAFTR',	'D0C00004', 'DRIVE_SHAFT', 	'UNDEF','Drive shaft: R Command CAN: send commands to subsystem');
INSERT INTO CANID VALUES ('CANID_SE4H_COUNTERnSPEED',	'30400000', 'DRIVE_SHAFT', 	'UNDEF','Drive shaft: (drive shaft) count and speed');
INSERT INTO CANID VALUES ('CANID_SE4H_ERROR1',		'D0400014', 'DRIVE_SHAFT',	'UNDEF','Drive shaft: [2]elapsed_ticks_no_adcticks<2000 ct  [3]cic not in sync');
INSERT INTO CANID VALUES ('CANID_SE4H_ERROR2',		'D0400004', 'DRIVE_SHAFT', 	'UNDEF','Drive shaft: [0]encode_state er ct [1]adctick_diff<2000 ct');

INSERT INTO CANID VALUES ('CANID_TILT_ALARM',		'04600000', 'TILT_SENSE', 	'UNDEF','Tilt: alarm: Vector angle exceeds limit');
INSERT INTO CANID VALUES ('CANID_TILT_ANGLE',		'42E00000', 'TILT_SENSE', 	'UNDEF','Tilt: Calibrated angles (X & Y)');
INSERT INTO CANID VALUES ('CANID_TILT_XYZ',		'42800000', 'TILT_SENSE', 	'UNDEF','Tilt: Calibrated to angle: x,y,z tilt readings');
INSERT INTO CANID VALUES ('CANID_TILT_XYZ_CAL',		'FFFFFFCC', 'TILT_SENSE', 	'UNDEF','Tilt: CANID: Raw tilt ADC readings');
INSERT INTO CANID VALUES ('CANID_TILT_XYZ_RAW',		'4280000C', 'TILT_SENSE', 	'UNDEF','Tilt: Tilt:Raw tilt ADC readings');
INSERT INTO CANID VALUES ('CANID_CMD_TILTI',		'42C00000', 'TILT_SENSE', 	'UNDEF','Tilt: I Command CANID');
INSERT INTO CANID VALUES ('CANID_CMD_TILTR',		'42C00004', 'TILT_SENSE', 	'UNDEF','Tilt: R Command CANID');

INSERT INTO CANID VALUES ('CANID_HB_GATEWAY1',		'E0200000', 'GATEWAY',	 	'NONE','Gateway1: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_GATEWAY2',		'E1200000', 'GATEWAY',	 	'NONE','Gateway2: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_GATEWAY3',		'E1400000', 'GATEWAY',	 	'NONE','Gateway3: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_0',		'E0400000', 'TENSION_0', 	'U8_FF','Tension_0: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a11',	'E0600000', 'TENSION_a', 	'U8_FF','Tension_a11: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a21',	'E0C00000', 'TENSION_a', 	'U8_FF','Tension_a21: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a12',	'E0800000', 'TENSION_a', 	'U8_FF','Tension_a12: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_TENSION_a22',	'E0E00000', 'TENSION_a', 	'U8_FF','Tension_a22: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_CABLE_ANGLE_1',	'E0A00000', 'CABLE_ANGLE_1', 	'UNDEF','Cable_Angle_1: Heartbeat');
INSERT INTO CANID VALUES ('CANID_HB_GPS_TIME_1',	'E1000000', 'GPS', 	'UNIXTIME',	'GPS_1: Heartbeat unix time');
INSERT INTO CANID VALUES ('CANID_HB_GPS_TIME_2',	'E1E00000', 'GPS', 	'UNIXTIME',	'GPS_2: Heartbeat unix time');
INSERT INTO CANID VALUES ('CANID_HB_GPS_LLH_1',		'E1C00000', 'GPS', 	'LAT_LON_HT',	'GPS_1: Heartbeat (3 separate msgs) lattitude longitude height');
INSERT INTO CANID VALUES ('CANID_HB_GPS_LLH_2',		'E2600000', 'LOGGER', 	'U8_U32',	'Logger_1: Heartbeat logging ctr');
INSERT INTO CANID VALUES ('CANID_HB_LOGGER_2',		'E1A00000', 'LOGGER', 	'U8_U32',	'Logger_2: Heartbeat logging ctr');

INSERT INTO CANID VALUES ('CANID_HB_CANSENDER_1',	'F0200000', 'CANSENDER','U8_U32',	'Cansender_1: Heartbeat w ctr');
INSERT INTO CANID VALUES ('CANID_HB_CANSENDER_2',	'F0400000', 'CANSENDER','U8_U32',	'Cansender_2: Heartbeat w ctr');
INSERT INTO CANID VALUES ('CANID_CMD_CANSENDER_1I',	'A0200000', 'CANSENDER','UNDEF',	'Cansender_1: I Command CANID');
INSERT INTO CANID VALUES ('CANID_CMD_CANSENDER_1R',	'A0200004', 'CANSENDER','UNDEF',	'Cansender_1: R Command CANID');
INSERT INTO CANID VALUES ('CANID_CMD_CANSENDER_2I',	'A0400000', 'CANSENDER','UNDEF',	'Cansender_2: I Command CANID');
INSERT INTO CANID VALUES ('CANID_CMD_CANSENDER_2R',	'A0400004', 'CANSENDER','UNDEF',	'Cansender_2: R Command CANID');
INSERT INTO CANID VALUES ('CANID_POLL_CANSENDER',	'E2000000', 'CANSENDER','U8_U32',	'Cansender: Poll cansenders');
INSERT INTO CANID VALUES ('CANID_POLLR_CANSENDER_1',	'E2200000', 'CANSENDER','U8_U32',	'Cansender_1: Response to POLL');
INSERT INTO CANID VALUES ('CANID_POLLR_CANSENDER_2',	'E2400000', 'CANSENDER','U8_U32',	'Cansender_2: Response to POLL');


INSERT INTO CANID VALUES ('CANID_CMD_SANDBOX_1I',	'28E00000', 'SANDBOX_1',	'UNDEF','HC: SANDBOX_1: I Launch parameters');
INSERT INTO CANID VALUES ('CANID_CMD_SANDBOX_1R',	'28E00004', 'SANDBOX_1',	'UNDEF','HC: SANDBOX_1: R Launch parameters');

INSERT INTO CANID VALUES ('CANID_CMD_YOGURT_1I',	'29800000', 'YOGURT_1',	'UNDEF','Yogurt: YOGURT_1: I Yogurt maker parameters');
INSERT INTO CANID VALUES ('CANID_CMD_YOGURT_1R',	'29800004', 'YOGURT_1',	'UNDEF','Yogurt: YOGURT_1: R Yogurt maker parameters');
INSERT INTO CANID VALUES ('CANID_MSG_YOGURT_1',		'29400000', 'YOGURT_1',	'UNDEF','Yogurt: YOGURT_1: Yogurt maker msgs');
INSERT INTO CANID VALUES ('CANID_HB_YOGURT_1',		'29600000', 'YOGURT_1',	'UNDEF','Yogurt: YOGURT_1: Heart-beats');

INSERT INTO CANID VALUES ('CANID_UNIT_2',		'04000000', 'UNIT_2', 	'U8','Sensor unit: Drive shaft encoder');
INSERT INTO CANID VALUES ('CANID_UNIT_3',		'03800000', 'UNIT_3', 	'U8','Sensor unit: Engine');
INSERT INTO CANID VALUES ('CANID_UNIT_4',		'03A00000', 'UNIT_4', 	'U8','Sensor unit: Lower sheave shaft encoder');
INSERT INTO CANID VALUES ('CANID_UNIT_5',		'03C00000', 'UNIT_5', 	'U8','Sensor unit: Upper sheave shaft encoder');
INSERT INTO CANID VALUES ('CANID_UNIT_8',		'01000000', 'UNIT_8', 	'U8','Sensor unit: Level wind');
INSERT INTO CANID VALUES ('CANID_UNIT_9',		'01200000', 'UNIT_9', 	'U8','Sensor unit: XBee receiver #1');
INSERT INTO CANID VALUES ('CANID_UNIT_A',		'0140000C', 'UNIT_A', 	'U8','Sensor unit: XBee receiver #2');
INSERT INTO CANID VALUES ('CANID_UNIT_B',		'0160000C', 'UNIT_B', 	'U8','Display driver/console');
INSERT INTO CANID VALUES ('CANID_UNIT_C',		'0180000C', 'UNIT_C', 	'U8','CAWs Olimex board');
INSERT INTO CANID VALUES ('CANID_UNIT_D',		'01A0000C', 'UNIT_D', 	'U8','POD board sensor prototype ("6" marked on board)');
INSERT INTO CANID VALUES ('CANID_UNIT_E',		'01C0000C', 'UNIT_E', 	'U8','Logger1: sensor board w ublox gps & SD card');
INSERT INTO CANID VALUES ('CANID_UNIT_F',		'01E0000C', 'UNIT_F', 	'U8','Tension_1 & Cable_angle_1 Unit');
INSERT INTO CANID VALUES ('CANID_UNIT_10',		'0200000C', 'UNIT_10',	'U8','Gateway1: 2 CAN');
INSERT INTO CANID VALUES ('CANID_UNIT_19',		'02800000', 'UNIT_19',	'U8','Master Controller');
INSERT INTO CANID VALUES ('CANID_UNIT_11',		'02200000', 'UNIT_11',	'U8','Tension: 1 AD7799 VE POD brd 1');
INSERT INTO CANID VALUES ('CANID_UNIT_12',		'02400000', 'UNIT_12',	'U8','Tension: 2 AD7799 VE POD brd 2');
INSERT INTO CANID VALUES ('CANID_UNIT_13',		'02600000', 'UNIT_13',	'U8','Yogurt: Olimex board');
INSERT INTO CANID VALUES ('CANID_UNIT_14',		'02E00000', 'UNIT_14',	'U8','Tension: 1 AD7799 VE POD brd 3');
INSERT INTO CANID VALUES ('CANID_UNIT_15',		'02A00000', 'UNIT_15',	'U8','Tension: 2 AD7799 VE POD brd 4');
INSERT INTO CANID VALUES ('CANID_UNIT_16',		'02C00000', 'UNIT_16',	'U8','Tension: 2 AD7799 VE POD brd 5 GSM');
INSERT INTO CANID VALUES ('CANID_UNIT_17',		'03200000', 'UNIT_17',	'U8','Gateway2: 1 CAN');
INSERT INTO CANID VALUES ('CANID_UNIT_18',		'03400000', 'UNIT_18',	'U8','Gateway3: 2 CAN');
INSERT INTO CANID VALUES ('CANID_UNIT_1A',		'03600000', 'UNIT_1A',	'U8','Logger2: sensor board w ublox gps & SD card');
INSERT INTO CANID VALUES ('CANID_UNIT_1B',		'03E00000', 'UNIT_1B',	'U8','Sensor board: CAW experiments');
INSERT INTO CANID VALUES ('CANID_UNIT_1C',		'04200000', 'UNIT_1C',	'U8','Sensor board: DEH spare 1');
INSERT INTO CANID VALUES ('CANID_UNIT_1D',		'04400000', 'UNIT_1D',	'U8','Sensor board: DEH spare 2');

INSERT INTO CANID VALUES ('CANID_UNIT_99',		'FFFFFF14', 'UNIT_99',	'UNDEF','Dummy for missing CAN IDs');
INSERT INTO CANID VALUES ('CANID_DUMMY',		'FFFFFFFC', 'UNIT_NU',	'UNDEF','Dummy ID: Lowest priority possible (Not Used)');
INSERT INTO CANID VALUES ('CANID_MSG_DUMMY',		'FFFFFF16', 'ANY', 		'UNDEF','Dummy ID: Polled Msg dummy');

