// Defines from database pcc
// 2016-11-12 16:46:42.271

#define CANID_COUNT 177
#define  CANID_MSG_TENSION_0      0x48000000  // TENSION_a      : Tension_0: Default measurement canid
#define  CANID_MSG_TENSION_a11    0x38000000  // TENSION_a      : Tension_a11: Drum 1 calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_a21    0x38200000  // TENSION_a      : Tension_a12: Drum 1 calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_a12    0x38400000  // TENSION_a      : Tension_a21: Drum 2 calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_a22    0x38600000  // TENSION_a      : Tension_a22: Drum 2 calibrated tension, polled by time msg
#define  CANID_MSG_TENSION_2      0x38800000  // TENSION_2      : Tension_2: calibrated tension, polled by time msg
#define  CANID_TST_TENSION_a11    0xF800010C  // TENSION_a      : Tension_a11: TESTING java program generation of idx_v_val.c
#define  CANID_TST_TENSION_a12    0xF800020C  // TENSION_a      : Tension_a12: TESTING java program generation of idx_v_val.c
#define  CANID_TST_TENSION_a21    0xF800030C  // TENSION_a      : Tension_a21: TESTING java program generation of idx_v_val.c
#define  CANID_TST_TENSION_a22    0xF800040C  // TENSION_a      : Tension_a22: TESTING java program generation of idx_v_val.c
#define  CANID_CMD_TENSION_a1WI   0x05C0003C  // TENSION_a      : Tension_a: I 1W Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a1WR   0x05C0803C  // TENSION_a      : Tension_a: R 1W Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a2WI   0x05C0004C  // TENSION_a      : Tension_a: I 2W Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a2WR   0x05C0804C  // TENSION_a      : Tension_a: R 2W Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a11I   0x05C00004  // TENSION_a      : Tension_a11: I Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a11R   0x05C0000C  // TENSION_a      : Tension_a11: R Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a21I   0x05E00004  // TENSION_a      : Tension_a21: I Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a21R   0x05E0000C  // TENSION_a      : Tension_a21: R Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_a12I   0xF800005C  // TENSION_a      : Tension_a12: I 2 AD7799 VE POD TESTING  3
#define  CANID_CMD_TENSION_a12R   0xF800805C  // TENSION_a      : Tension_a12: R 2 AD7799 VE POD TESTING  3
#define  CANID_CMD_TENSION_a22I   0xF800006C  // TENSION_a      : Tension_a22: I 2 AD7799 VE POD TESTING  3
#define  CANID_CMD_TENSION_a22R   0xF800806C  // TENSION_a      : Tension_a22: R 2 AD7799 VE POD TESTING  3
#define  CANID_CMD_TENSION_a0XI   0xF800001C  // TENSION_a      : Tension_a:  I 1 AD7799 VE POD TESTING (hence 0) 0
#define  CANID_CMD_TENSION_a0XR   0xF800801C  // TENSION_a      : Tension_a:  R 1 AD7799 VE POD TESTING (hence 0) 0
#define  CANID_CMD_TENSION_a1XI   0xF800002C  // TENSION_a      : Tension_a:  I 2 AD7799 VE POD TESTING (hence X) 1
#define  CANID_CMD_TENSION_a1XR   0xF800802C  // TENSION_a      : Tension_a:  R 2 AD7799 VE POD TESTING (hence X) 1
#define  CANID_CMD_TENSION_a2XI   0xF800003C  // TENSION_a      : Tension_a2: I 2 AD7799 VE POD TESTING (hence X) 1
#define  CANID_CMD_TENSION_a2XR   0xF800803C  // TENSION_a      : Tension_a2: R 2 AD7799 VE POD TESTING (hence X) 1
#define  CANID_CMD_TENSION_a1YI   0xF800004C  // TENSION_a      : Tension_a:  I 1 AD7799 VE POD TESTING (hence Y) 2
#define  CANID_CMD_TENSION_a1YR   0xF800804C  // TENSION_a      : Tension_a:  R 1 AD7799 VE POD TESTING (hence Y) 2
#define  CANID_CMD_TENSION_a1GI   0xF800007C  // TENSION_a      : Tension_a:  I 2 AD7799 VE POD GSM 1st board sent (_16)
#define  CANID_CMD_TENSION_a1GR   0xF800807C  // TENSION_a      : Tension_a:  R 2 AD7799 VE POD GSM 1st board sent (_16)
#define  CANID_CMD_TENSION_a2GI   0xF800008C  // TENSION_a      : Tension_a2: I 2 AD7799 VE POD GSM 1st board sent (_16)
#define  CANID_CMD_TENSION_a2GR   0xF800808C  // TENSION_a      : Tension_a2: R 2 AD7799 VE POD GSM 1st board sent (_16)
#define  CANID_CMD_TENSION_2I     0x05C0005C  // TENSION_2      : Tension_2: I Tension_2: Command code: [0] command code, [1]-[8] depends on code
#define  CANID_CMD_TENSION_2R     0x05C0805C  // TENSION_2      : Tension_2: R Tension_2: Command code: [0] command code, [1]-[8] depends on code
#define  CANID_MOTOR_1            0x2D000000  // MOTOR_1        : MOTOR_1: Motor speed
#define  CANID_CMD_CABLE_ANGLE_0I 0x06000000  // CABLE_ANGLE_0  : Cable_Angle0: I Default measurement canid
#define  CANID_CMD_CABLE_ANGLE_0R 0x0600000C  // CABLE_ANGLE_0  : Cable_Angle0: R Default measurement canid
#define  CANID_CMD_CABLE_ANGLE_1I 0x06200000  // CABLE_ANGLE_1  : Cable_Angle1: I [0] command code, [1]-[8] depends on code
#define  CANID_CMD_CABLE_ANGLE_1R 0x0620000C  // CABLE_ANGLE_1  : Cable_Angle1: R [0] command code, [1]-[8] depends on code
#define  CANID_MSG_CABLE_ANGLE_1  0x3A000000  // CABLE_ANGLE_1  : Cable_Angle1: for drum #1
#define  CANID_MSG_CABLE_ANGLE_1_ALARM 0x2B000000  // CABLE_ANGLE_1  : Cable_Angle1: unreliable for drum #1
#define  CANID_CMD_ENGINE_SENSORI 0x80600000  // ENGINE_SENSOR  : Engine: code: I [0] command code, [1]-[8] depends on code
#define  CANID_CMD_ENGINE_SENSORR 0x8060000C  // ENGINE_SENSOR  : Engine: code: R [0] command code, [1]-[8] depends on code
#define  CANID_HB_ENG_RPMMANIFOLD 0x40600000  // ENGINE_SENSOR  : Engine: rpm:manifold pressure
#define  CANID_HB_ENG_TEMP        0x70600000  // ENGINE_SENSOR  : Engine: thermistor converted to temp
#define  CANID_HB_ENG_THERMTHROTL 0x60600000  // ENGINE_SENSOR  : Engine: thermistor:throttle pot
#define  CANID_HB_ENG_THROTTLE    0x50600000  // ENGINE_SENSOR  : Engine: throttle
#define  CANID_HB_FIX_HT_TYP_NSAT 0xB1C00000  // GPS            : GPS: fix: heigth:type fix:number sats
#define  CANID_HB_FIX_LATLON      0xA1C00000  // GPS            : GPS: fix: lattitude:longitude
#define  CANID_HB_LG_ER1          0xD1C00004  // GPS            : GPS: 1st code  CANID-UNITID_CO_OLI GPS checksum error
#define  CANID_HB_LG_ER2          0xD1C00014  // GPS            : GPS: 2nd code  CANID-UNITID_CO_OLI GPS Fix error
#define  CANID_HB_LG_ER3          0xD1C00024  // GPS            : GPS: 3rd code  CANID-UNITID_CO_OLI GPS Time out of step
#define  CANID_HB_TIMESYNC        0x00400000  // GPS            : GPS_1: GPS time sync distribution msg
#define  CANID_HB_TIMESYNC_2      0x00600000  // GPS            : GPS_2: GPS time sync distribution msg
#define  CANID_HB_TIMESYNC_X      0x03000000  // GPS            : GPS_2: Obsolete GPS time sync distribution msg
#define  CANID_HB_UNIVERSAL_RESET 0x00200000  // GPS            : Highest priority: reserved for Universal (if/when implemented)
#define  CANID_CMD_GPS_1I         0xD1C00044  // GPS            : GPS_1: I CANID Command GPS 1
#define  CANID_CMD_GPS_1R         0xD1C0004C  // GPS            : GPS_1: R CANID Command GPS 1
#define  CANID_CMD_GPS_2I         0xD1C00074  // GPS            : GPS_2: I CANID Command GPS 2
#define  CANID_CMD_GPS_2R         0xD1C0007C  // GPS            : GPS_2: R CANID Command GPS 2
#define  CANID_CMD_LOGGER_1I      0xD1C00054  // LOGGER         : Logger_1: I Command Logger 1
#define  CANID_CMD_LOGGER_1R      0xD1C0005C  // LOGGER         : Logger_1: R Command Logger 1
#define  CANID_CMD_LOGGER_2I      0xD1C00064  // LOGGER         : Logger_2: I Command Logger 2
#define  CANID_CMD_LOGGER_2R      0xD1C0006C  // LOGGER         : Logger_2: R Command Logger 2
#define  CANID_MC_SYSTEM_STATE    0x50000000  // MC             : MC: System state msg
#define  CANID_MC_DRUM_SELECT     0xD0800814  // MC             : MC: Drum selection
#define  CANID_HB_MC_MOTOR_1_KPALIVE 0xA0800000  // MC             : MC: Curtis Controller keepalive
#define  CANID_MC_REQUEST_PARAM   0xD0800824  // MC             : MC: Request parameters from HC
#define  CANID_MC_CONTACTOR       0x23000000  // MC             : MC: Contactor OPEN/CLOSE
#define  CANID_MC_BRAKES          0x21000000  // MC             : MC: Brakes APPLY/RELEASE
#define  CANID_MC_GUILLOTINE      0x22000000  // MC             : MC: Fire guillotine
#define  CANID_MC_RQ_LAUNCH_PARAM 0x27000000  // MC             : MC: Fire request launch parameters
#define  CANID_MSG_TIME_POLL      0x20000000  // MC             : MC: Time msg/Group polling
#define  CANID_MC_STATE           0x26000000  // MC             : MC: Launch state msg
#define  CANID_MC_TORQUE          0x25800000  // MC             : MC: Motor torque
#define  CANID_CP_CTL_RMT         0x29000000  // CP             : Control Panel: Control lever remote
#define  CANID_CP_CTL_LCL         0x29200000  // CP             : Control Panel: Control lever local
#define  CANID_CP_CTL_IN_RMT      0x24C00000  // CP             : Control Panel: Control lever remote: input
#define  CANID_CP_CTL_IN_LCL      0x25000000  // CP             : Control Panel: Control lever  local: input
#define  CANID_CP_CTL_OUT_RMT     0x2A000000  // CP             : Control Panel: Control lever output
#define  CANID_SE2H_ADC2_HistA    0xD0800044  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: ADC2 HistogramA tx: request count, switch buffers; rx send count
#define  CANID_SE2H_ADC2_HistB    0xD0800054  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: ADC2 HistogramB tx: bin number, rx: send bin count
#define  CANID_SE2H_ADC3_ADC2_RD  0xD0800064  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: ADC3 ADC2 readings readout
#define  CANID_SE2H_ADC3_HistA    0xD0800024  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: ADC3 HistogramA tx: request count, switch buffers. rx: send count
#define  CANID_SE2H_ADC3_HistB    0xD0800034  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:E2: ADC3 HistogramB tx: bin number, rx: send bin count
#define  CANID_SE2H_COUNTERnSPEED 0x30800000  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: (Lower sheave) Count and speed
#define  CANID_SE2H_ERROR1        0xD0800014  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: error1
#define  CANID_SE2H_ERROR2        0xD0800074  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: error2
#define  CANID_CMD_LOWERSHVI      0xD0800000  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: I Command CAN: send commands to subsystem
#define  CANID_CMD_LOWERSHVR      0xD0800004  // SHAFT_LOWERSHV : Shaft encoder: Lower sheave:SE2: R Command CAN: send commands to subsystem
#define  CANID_SE3H_ADC2_HistA    0xD0A00044  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC2 HistogramA tx: request count, switch buffers; rx send count
#define  CANID_SE3H_ADC2_HistB    0xD0A00054  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC2 HistogramB tx: bin number, rx: send bin count
#define  CANID_SE3H_ADC3_ADC2_RD  0xD0A00064  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC3 ADC2 readings readout
#define  CANID_SE3H_ADC3_HistA    0xD0A00024  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC3 HistogramA tx: request count, switch buffers. rx: send count
#define  CANID_SE3H_ADC3_HistB    0xD0A00034  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: ADC3 HistogramB tx: bin number, rx: send bin count
#define  CANID_SE3H_COUNTERnSPEED 0x30A00000  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: (upper sheave) Count and Speed
#define  CANID_SE3H_ERROR1        0xD0A00014  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: error1
#define  CANID_SE3H_ERROR2        0xD0A00004  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: error2
#define  CANID_CMD_UPPERSHVI      0xD0600000  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: I Command CAN: send commands to subsystem
#define  CANID_CMD_UPPERSHVR      0xD0600004  // SHAFT_UPPERSHV : Shaft encoder: Upper sheave:SE3: R Command CAN: send commands to subsystem
#define  CANID_SE4H_ADC2_HistA    0xD0400044  // DRIVE_SHAFT    : Drive shaft: ADC2 HistogramA tx: request count, switch buffers; rx send count
#define  CANID_SE4H_ADC2_HistB    0xD0400054  // DRIVE_SHAFT    : Drive shaft: ADC2 HistogramB tx: bin number, rx: send bin count
#define  CANID_SE4H_ADC3_ADC2_RD  0xD0400064  // DRIVE_SHAFT    : Drive shaft: ADC3 ADC2 readings readout
#define  CANID_SE4H_ADC3_HistA    0xD0400024  // DRIVE_SHAFT    : Drive shaft: ADC3 HistogramA tx: request count, switch buffers. rx: send count
#define  CANID_SE4H_ADC3_HistB    0xD0400034  // DRIVE_SHAFT    : Drive shaft: ADC3 HistogramB tx: bin number, rx: send bin count
#define  CANID_CMD_DRIVE_SHAFTI   0xD0C00000  // DRIVE_SHAFT    : Drive shaft: I Command CAN: send commands to subsystem
#define  CANID_CMD_DRIVE_SHAFTR   0xD0C00004  // DRIVE_SHAFT    : Drive shaft: R Command CAN: send commands to subsystem
#define  CANID_SE4H_COUNTERnSPEED 0x30400000  // DRIVE_SHAFT    : Drive shaft: (drive shaft) count and speed
#define  CANID_SE4H_ERROR1        0xD0400014  // DRIVE_SHAFT    : Drive shaft: [2]elapsed_ticks_no_adcticks<2000 ct  [3]cic not in sync
#define  CANID_SE4H_ERROR2        0xD0400004  // DRIVE_SHAFT    : Drive shaft: [0]encode_state er ct [1]adctick_diff<2000 ct
#define  CANID_TILT_ALARM         0x04600000  // TILT_SENSE     : Tilt: alarm: Vector angle exceeds limit
#define  CANID_TILT_ANGLE         0x42E00000  // TILT_SENSE     : Tilt: Calibrated angles (X & Y)
#define  CANID_TILT_XYZ           0x42800000  // TILT_SENSE     : Tilt: Calibrated to angle: x,y,z tilt readings
#define  CANID_TILT_XYZ_CAL       0xFFFFFFCC  // TILT_SENSE     : Tilt: CANID: Raw tilt ADC readings
#define  CANID_TILT_XYZ_RAW       0x4280000C  // TILT_SENSE     : Tilt: Tilt:Raw tilt ADC readings
#define  CANID_CMD_TILTI          0x42C00000  // TILT_SENSE     : Tilt: I Command CANID
#define  CANID_CMD_TILTR          0x42C00004  // TILT_SENSE     : Tilt: R Command CANID
#define  CANID_HB_GATEWAY1        0xE0200000  // GATEWAY        : Gateway1: Heartbeat
#define  CANID_HB_GATEWAY2        0xE1200000  // GATEWAY        : Gateway2: Heartbeat
#define  CANID_HB_GATEWAY3        0xE1400000  // GATEWAY        : Gateway3: Heartbeat
#define  CANID_HB_TENSION_0       0xE0400000  // TENSION_0      : Tension_0: Heartbeat
#define  CANID_HB_TENSION_a11     0xE0600000  // TENSION_a      : Tension_a11: Heartbeat
#define  CANID_HB_TENSION_a21     0xE0C00000  // TENSION_a      : Tension_a21: Heartbeat
#define  CANID_HB_TENSION_a12     0xE0800000  // TENSION_a      : Tension_a12: Heartbeat
#define  CANID_HB_TENSION_a22     0xE0E00000  // TENSION_a      : Tension_a22: Heartbeat
#define  CANID_HB_CABLE_ANGLE_1   0xE0A00000  // CABLE_ANGLE_1  : Cable_Angle_1: Heartbeat
#define  CANID_HB_GPS_TIME_1      0xE1000000  // GPS            : GPS_1: Heartbeat unix time
#define  CANID_HB_GPS_TIME_2      0xE1E00000  // GPS            : GPS_2: Heartbeat unix time
#define  CANID_HB_GPS_LLH_1       0xE1C00000  // GPS            : GPS_1: Heartbeat (3 separate msgs) lattitude longitude height
#define  CANID_HB_GPS_LLH_2       0xE2600000  // LOGGER         : Logger_1: Heartbeat logging ctr
#define  CANID_HB_LOGGER_2        0xE1A00000  // LOGGER         : Logger_2: Heartbeat logging ctr
#define  CANID_HB_CANSENDER_1     0xF0200000  // CANSENDER      : Cansender_1: Heartbeat w ctr
#define  CANID_HB_CANSENDER_2     0xF0400000  // CANSENDER      : Cansender_2: Heartbeat w ctr
#define  CANID_CMD_CANSENDER_1I   0xA0200000  // CANSENDER      : Cansender_1: I Command CANID
#define  CANID_CMD_CANSENDER_1R   0xA0200004  // CANSENDER      : Cansender_1: R Command CANID
#define  CANID_CMD_CANSENDER_2I   0xA0400000  // CANSENDER      : Cansender_2: I Command CANID
#define  CANID_CMD_CANSENDER_2R   0xA0400004  // CANSENDER      : Cansender_2: R Command CANID
#define  CANID_POLL_CANSENDER     0xE2000000  // CANSENDER      : Cansender: Poll cansenders
#define  CANID_POLLR_CANSENDER_1  0xE2200000  // CANSENDER      : Cansender_1: Response to POLL
#define  CANID_POLLR_CANSENDER_2  0xE2400000  // CANSENDER      : Cansender_2: Response to POLL
#define  CANID_CMD_SANDBOX_1I     0x28E00000  // SANDBOX_1      : HC: SANDBOX_1: I Launch parameters
#define  CANID_CMD_SANDBOX_1R     0x28E00004  // SANDBOX_1      : HC: SANDBOX_1: R Launch parameters
#define  CANID_CMD_YOGURT_1I      0x29800000  // YOGURT_1       : Yogurt: YOGURT_1: I Yogurt maker parameters
#define  CANID_CMD_YOGURT_1R      0x29800004  // YOGURT_1       : Yogurt: YOGURT_1: R Yogurt maker parameters
#define  CANID_MSG_YOGURT_1       0x29400000  // YOGURT_1       : Yogurt: YOGURT_1: Yogurt maker msgs
#define  CANID_HB_YOGURT_1        0x29600000  // YOGURT_1       : Yogurt: YOGURT_1: Heart-beats
#define  CANID_UNIT_2             0x04000000  // UNIT_2         : Sensor unit: Drive shaft encoder
#define  CANID_UNIT_3             0x03800000  // UNIT_3         : Sensor unit: Engine
#define  CANID_UNIT_4             0x03A00000  // UNIT_4         : Sensor unit: Lower sheave shaft encoder
#define  CANID_UNIT_5             0x03C00000  // UNIT_5         : Sensor unit: Upper sheave shaft encoder
#define  CANID_UNIT_8             0x01000000  // UNIT_8         : Sensor unit: Level wind
#define  CANID_UNIT_9             0x01200000  // UNIT_9         : Sensor unit: XBee receiver #1
#define  CANID_UNIT_A             0x0140000C  // UNIT_A         : Sensor unit: XBee receiver #2
#define  CANID_UNIT_B             0x0160000C  // UNIT_B         : Display driver/console
#define  CANID_UNIT_C             0x0180000C  // UNIT_C         : CAWs Olimex board
#define  CANID_UNIT_D             0x01A0000C  // UNIT_D         : POD board sensor prototype ("6" marked on board)
#define  CANID_UNIT_E             0x01C0000C  // UNIT_E         : Logger1: sensor board w ublox gps & SD card
#define  CANID_UNIT_F             0x01E0000C  // UNIT_F         : Tension_1 & Cable_angle_1 Unit
#define  CANID_UNIT_10            0x0200000C  // UNIT_10        : Gateway1: 2 CAN
#define  CANID_UNIT_19            0x02800000  // UNIT_19        : Master Controller
#define  CANID_UNIT_11            0x02200000  // UNIT_11        : Tension: 1 AD7799 VE POD brd 1
#define  CANID_UNIT_12            0x02400000  // UNIT_12        : Tension: 2 AD7799 VE POD brd 2
#define  CANID_UNIT_13            0x02600000  // UNIT_13        : Yogurt: Olimex board
#define  CANID_UNIT_14            0x02E00000  // UNIT_14        : Tension: 1 AD7799 VE POD brd 3
#define  CANID_UNIT_15            0x02A00000  // UNIT_15        : Tension: 2 AD7799 VE POD brd 4
#define  CANID_UNIT_16            0x02C00000  // UNIT_16        : Tension: 2 AD7799 VE POD brd 5 GSM
#define  CANID_UNIT_17            0x03200000  // UNIT_17        : Gateway2: 1 CAN
#define  CANID_UNIT_18            0x03400000  // UNIT_18        : Gateway3: 2 CAN
#define  CANID_UNIT_1A            0x03600000  // UNIT_1A        : Logger2: sensor board w ublox gps & SD card
#define  CANID_UNIT_1B            0x03E00000  // UNIT_1B        : Sensor board: CAW experiments
#define  CANID_UNIT_1C            0x04200000  // UNIT_1C        : Sensor board: DEH spare 1
#define  CANID_UNIT_1D            0x04400000  // UNIT_1D        : Sensor board: DEH spare 2
#define  CANID_UNIT_99            0xFFFFFF14  // UNIT_99        : Dummy for missing CAN IDs
#define  CANID_DUMMY              0xFFFFFFFC  // UNIT_NU        : Dummy ID: Lowest priority possible (Not Used)
#define  CANID_MSG_DUMMY          0xFFFFFF16  // ANY            : Dummy ID: Polled Msg dummy

#define NUMBER_TYPE_COUNT 17
#define  TYP_S8                  1         // 1             int8_t,   signed char, 1 byte
#define  TYP_U8                  2         // 1            uint8_t, unsigned char, 1 byte
#define  TYP_S16                 3         // 2            int16_t,   signed short, 2 bytes
#define  TYP_U16                 4         // 2           uint16_t, unsigned short, 2 bytes
#define  TYP_S32                 5         // 4            int32_t,   signed int, 4 bytes
#define  TYP_U32                 6         // 4           uint32_t, unsigned int, 4 bytes
#define  TYP_S64_L               7         // 4            int64_t,   signed long long, low  order 4 bytes
#define  TYP_S64_H               8         // 4            int64_t,   signed long long, high order 4 bytes
#define  TYP_U64_L               9         // 4           uint64_t, unsigned long long, low  order 4 bytes
#define  TYP_U64_H               10        // 4           uint64_t, unsigned long long, high order 4 bytes
#define  TYP_FLT                 11        // 4           float, 4 bytes
#define  TYP_12FLT               12        // 2           half-float, 2 bytes
#define  TYP_34FLT               13        // 3           3/4-float, 3 bytes
#define  TYP_DBL_L               14        // 4           double, low  order 4 bytes
#define  TYP_DBL_H               15        // 4           double, high order 4 bytes
#define  TYP_ASC                 16        // 4           ascii chars
#define  TYP_CANID               17        // 1           CANID (handled differently than a U32)

#define CMD_CODES_COUNT 32
#define  LDR_SET_ADDR            1         // 5 Set address pointer (not FLASH) (bytes 2-5):  Respond with last written address.
#define  LDR_SET_ADDR_FL         2         // 5 Set address pointer (FLASH) (bytes 2-5):  Respond with last written address.
#define  LDR_CRC                 3         // 8 Get CRC: 2-4 = count; 5-8 = start address; Reply CRC 2-4 na, 5-8 computed CRC 
#define  LDR_ACK                 4         // 1 ACK: Positive acknowledge (Get next something)
#define  LDR_NACK                5         // 1 NACK: Negative acknowledge (So? How do we know it is wrong?)
#define  LDR_JMP                 6         // 5 Jump: to address supplied (bytes 2-5)
#define  LDR_WRBLK               7         // 1 Done with block: write block with whatever you have.
#define  LDR_RESET               8         // 1 RESET: Execute a software forced RESET
#define  LDR_XON                 9         // 1 Resume sending
#define  LDR_XOFF                10        // 1 Stop sending
#define  LDR_FLASHSIZE           11        // 1 Get flash size; bytes 2-3 = flash block size (short)
#define  LDR_ADDR_OOB            12        // 1 Address is out-of-bounds
#define  LDR_DLC_ERR             13        // 1 Unexpected DLC
#define  LDR_FIXEDADDR           14        // 5 Get address of flash with fixed loader info (e.g. unique CAN ID)
#define  LDR_RD4                 15        // 5 Read 4 bytes at address (bytes 2-5)
#define  LDR_APPOFFSET           16        // 5 Get address where application begins storing.
#define  LDR_HIGHFLASHH          17        // 5 Get address of beginning of struct with crc check and CAN ID info for app
#define  LDR_HIGHFLASHP          18        // 8 Get address and size of struct with app calibrations, parameters, etc.
#define  LDR_ASCII_SW            19        // 2 Switch mode to send printf ASCII in CAN msgs
#define  LDR_ASCII_DAT           20        // 3-8 [1]=line position;[2]-[8]=ASCII chars
#define  LDR_WRVAL_PTR           21        // 2-8 Write: 2-8=bytes to be written via address ptr previous set.
#define  LDR_WRVAL_PTR_SIZE      22        // Write data payload size
#define  LDR_WRVAL_AI            23        // 8 Write: 2=memory area; 3-4=index; 5-8=one 4 byte value
#define  LDR_SQUELCH             24        // 8 Send squelch sending tick ct: 2-8 count
#define  CMD_GET_IDENT           30        // Get parameter using indentification name/number in byte [1]
#define  CMD_PUT_IDENT           31        // Put parameter using indentification name/number in byte [1]
#define  CMD_GET_INDEX           32        // Get parameter using index name/number in byte [1]
#define  CMD_PUT_INDEX           33        // Put parameter using index name/number in byte [1]
#define  CMD_REVERT              34        // Revert (re-initialize) working parameters/calibrations/CANIDs back to stored non-volatile values
#define  CMD_SAVE                35        // Write current working parameters/calibrations/CANIDs to non-volatile storage
#define  CMD_GET_READING         36        // Send a reading for the code specified in byte [1] specific to function
#define  CMD_GET_READING_BRD     37        // Send a reading for the code specified in byte [1] for board; common to functions

#define PAYLOAD_TYPE_COUNT 29
#define  NONE                    0         //  No payload bytes                               
#define  FF                      1         //  [0]-[3]: Full Float                            
#define  FF_FF                   2         //  [0]-[3]: Full Float[0]; [4]-[7]: Full Float[1] 
#define  U32                     3         //  [0]-[3]: uint32_t                              
#define  U32_U32                 4         //  [0]-[3]: uint32_t[0]; [4]-[7]: uint32_t[1]     
#define  U8_U32                  5         //  [0]: uint8_t; [1]-[4]: uint32_t                
#define  S32                     6         //  [0]-[3]: int32_t                               
#define  S32_S32                 7         //  [0]-[3]: int32_t[0]; [4]-[7]: int32_t[1]       
#define  U8_S32                  8         //  [0]: int8_t; [4]-[7]: int32_t                  
#define  HF                      9         //  [0]-[1]: Half-Float                            
#define  F34F                    10        //  [0]-[2]: 3/4-Float                             
#define  xFF                     11        //  [0]:[1]-[4]: Full-Float, first   byte  skipped 
#define  xxFF                    12        //  [0]:[1]:[2]-[5]: Full-Float, first 2 bytes skipped
#define  xxU32                   13        //  [0]:[1]:[2]-[5]: uint32_t, first 2 bytes skipped
#define  xxS32                   14        //  [0]:[1]:[2]-[5]: int32_t, first 2 bytes skipped
#define  U8_U8_U32               15        //  [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1],uint32_t,
#define  U8_U8_S32               16        //  [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], int32_t,
#define  U8_U8_FF                17        //  [0]:[1]:[2]-[5]: uint8_t[0],uint8_t[1], Full Float,
#define  U16                     18        //  [0]-[2]uint16_t                                
#define  S16                     19        //  [0]-[2] int16_t                                
#define  LAT_LON_HT              20        //  [0]:[1]:[2]-[5]: Fix type, bits fields, lat/lon/ht
#define  U8_FF                   21        //  [0]:[1]-[4]: uint8_t, Full Float               
#define  U8_HF                   22        //  [0]:[1]-[2]: uint8_t, Half Float               
#define  U8                      23        //  [0]: uint8_t                                   
#define  UNIXTIME                24        //  [0]: U8_U32 with U8 bit field stuff            
#define  U8_U8                   25        //  [0]:[1]: uint8_t[0],uint8[1]                   
#define  LVL2B                   249       //  [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Board code),[2]-[5]see table
#define  LVL2R                   250       //  [2]-[5]: (uint8_t[0],uint8_t[1] cmd:Readings code),[2]-[5]see table
#define  UNDEF                   255       //  Undefined                                      

#define PARAM_LIST_COUNT 154	// TOTAL COUNT OF PARAMETER LIST

#define  CANSENDER_LIST_CRC      	1         // Cansender: CRC                                  
#define  CANSENDER_LIST_VERSION  	2         // Cansender: Version number                       
#define  CANSENDER_HEARTBEAT_CT  	3         // Cansender: Heartbeat count of time (ms) between msgs
#define  CANSENDER_HEARTBEAT_MSG 	4         // Cansender: CANID: Hearbeat sends running count  
#define  CANSENDER_POLL          	5         // Cansender: CANID: Poll this cansender           
#define  CANSENDER_POLL_R        	6         // Cansender: CANID: Response to POLL              

#define PARAM_LIST_CT_CANSENDER	6	// Count of same FUNCTION_TYPE in preceding list

#define  TENSION_a_LIST_CRC      	1         // Tension_a: crc: CRC for tension list            
#define  TENSION_a_LIST_VERSION  	2         // Tension_a: version: Version number for Tension List
#define  TENSION_a_AD7799_1_OFFSET	3         // Tension_a: offset: AD7799 #1 offset             
#define  TENSION_a_AD7799_1_SCALE	4         // Tension_a: scale: AD7799 #1 Scale (convert to kgf)
#define  TENSION_a_THERM1_CONST_B	5         // Tension_a: Thermistor1 param: B: constant B     
#define  TENSION_a_THERM1_R_SERIES	6         // Tension_a: Thermistor1 param: RS: Series resistor, fixed (K ohms)
#define  TENSION_a_THERM1_R_ROOMTMP	7         // Tension_a: Thermistor1 param: R0: Thermistor room temp resistance (K ohms)
#define  TENSION_a_THERM1_REF_TEMP	8         // Tension_a: Thermistor1 param: TREF: Reference temp for thermistor
#define  TENSION_a_THERM1_TEMP_OFFSET	9         // Tension_a: Thermistor1 param: offset: Thermistor temp offset correction (deg C)
#define  TENSION_a_THERM1_TEMP_SCALE	10        // Tension_a: Thermistor1 param: B: scale:  Thermistor temp scale correction
#define  TENSION_a_THERM2_CONST_B	11        // Tension_a: Thermistor2 param: RS: constant B    
#define  TENSION_a_THERM2_R_SERIES	12        // Tension_a: Thermistor2 param: Series resistor, fixed (K ohms)
#define  TENSION_a_THERM2_R_ROOMTMP	13        // Tension_a: Thermistor2 param: R0: Thermistor room temp resistance (K ohms)
#define  TENSION_a_THERM2_REF_TEMP	14        // Tension_a: Thermistor2 param: TREF: Reference temp for thermistor
#define  TENSION_a_THERM2_TEMP_OFFSET	15        // Tension_a: Thermistor2 param: offset: hermistor temp offset correction (deg C)
#define  TENSION_a_THERM2_TEMP_SCALE	16        // Tension_a: Thermistor2 param: scale: Thermistor temp scale correction
#define  TENSION_a_THERM1_COEF_0 	17        // Tension_a: Thermistor1 param: comp_t1[0]: Load-Cell polynomial coefficient 0 (offset)
#define  TENSION_a_THERM1_COEF_1 	18        // Tension_a: Thermistor1 param: comp_t1[1]: Load-Cell polynomial coefficient 1 (scale)
#define  TENSION_a_THERM1_COEF_2 	19        // Tension_a: Thermistor1 param: comp_t1[2]: Load-Cell polynomial coefficient 2 (x^2)
#define  TENSION_a_THERM1_COEF_3 	20        // Tension_a: Thermistor1 param: comp_t1[3]: Load-Cell polynomial coefficient 3 (x^3)
#define  TENSION_a_THERM2_COEF_0 	21        // Tension_a: Thermistor2 param: comp_t2[0]: Load-Cell polynomial coefficient 0 (offset)
#define  TENSION_a_THERM2_COEF_1 	22        // Tension_a: Thermistor2 param: comp_t2[1]: Load-Cell polynomial coefficient 1 (scale)
#define  TENSION_a_THERM2_COEF_2 	23        // Tension_a: Thermistor2 param: comp_t2[2]: Load-Cell polynomial coefficient 2 (x^2)
#define  TENSION_a_THERM2_COEF_3 	24        // Tension_a: Thermistor2 param: comp_t2[3]: Load-Cell polynomial coefficient 3 (x^3)
#define  TENSION_a_HEARTBEAT_CT  	25        // Tension_a: hbct: Heart-Beat Count of time (milliseconds) between autonomous msgs
#define  TENSION_a_DRUM_NUMBER   	26        // Tension_a: drum: Drum system number for this function instance
#define  TENSION_a_DRUM_FUNCTION_BIT	27        // Tension_a: f_pollbit: Drum system poll 1st payload byte bit for drum # (function instance)
#define  TENSION_a_DRUM_POLL_BIT 	28        // Tension_a: p_pollbit: Drum system poll 2nd payload byte bit for this type of function
#define  TENSION_a_CANPRM_TENSION	29        // Tension_a: CANID: cid_ten_msg:  canid msg Tension
#define  TENSION_a_MSG_TIME_POLL 	30        // Tension_a: CANID: cid_ten_poll:  canid MC: Time msg/Group polling
#define  TENSION_a_TIMESYNC      	31        // Tension_a: CANID: cid_gps_sync: canid time: GPS time sync distribution
#define  TENSION_a_HEARTBEAT     	32        // Tension_a: CANID: heartbeat                     
#define  TENSION_a_CANIDTEST     	33        // Tension_a: CANID: testing java program          
#define  TENSION_a_IIR_POLL_K    	34        // Tension_a: IIR Filter factor: divisor sets time constant: reading for polled msg
#define  TENSION_a_IIR_POLL_SCALE	35        // Tension_a: IIR Filter scale : upscaling (due to integer math): for polled msg
#define  TENSION_a_IIR_HB_K      	36        // Tension_a: IIR Filter factor: divisor sets time constant: reading for heart-beat msg
#define  TENSION_a_IIR_HB_SCALE  	37        // Tension_a: IIR Filter scale : upscaling (due to integer math): for heart-beat msg
#define  TENSION_a_USEME         	38        // Tension_a: skip or use this function switch     
#define  TENSION_a_IIR_Z_RECAL_K 	39        // Tension_a: IIR Filter factor: divisor sets time constant: zero recalibration
#define  TENSION_a_IIR_Z_RECAL_SCALE	40        // Tension_a: IIR Filter scale : upscaling (due to integer math): zero recalibration
#define  TENSION_a_Z_RECAL_CT    	41        // Tension_a: ADC conversion counts between zero recalibrations
#define  TENSION_a_LIMIT_HI      	42        // Tension_a: Exceeding this calibrated limit (+) means invalid reading
#define  TENSION_a_LIMIT_LO      	43        // Tension_a: Exceeding this calibrated limit (-) means invalid reading
#define  TENSION_a_CANID_HW_FILT1	44        // Tension_a: CANID1 parameter in this list for CAN hardware filter to allow
#define  TENSION_a_CANID_HW_FILT2	45        // Tension_a: CANID2 CANID parameter in this list for CAN hardware filter to allow
#define  TENSION_a_CANID_HW_FILT3	46        // Tension_a: CANID3 CANID parameter in this list for CAN hardware filter to allow
#define  TENSION_a_CANID_HW_FILT4	47        // Tension_a: CANID4 CANID parameter in this list for CAN hardware filter to allow
#define  TENSION_a_CANID_HW_FILT5	48        // Tension_a: CANID5 CANID parameter in this list for CAN hardware filter to allow
#define  TENSION_a_CANID_HW_FILT6	49        // Tension_a: CANID6 CANID parameter in this list for CAN hardware filter to allow
#define  TENSION_a_CANID_HW_FILT7	50        // Tension_a: CANID7 CANID parameter in this list for CAN hardware filter to allow
#define  TENSION_a_CANID_HW_FILT8	51        // Tension_a: CANID8 CANID parameter in this list for CAN hardware filter to allow

#define PARAM_LIST_CT_TENSION_a	51	// Count of same FUNCTION_TYPE in preceding list

#define  CABLE_ANGLE_LIST_CRC    	1         // Cable Angle: CRC for cable angle list           
#define  CABLE_ANGLE_LIST_VERSION	2         // Cable Angle: Version number for Cable Angle List
#define  CABLE_ANGLE_HEARTBEAT_CT	3         // Cable Angle: Heart-Beat: Count of time ticks between autonomous msgs
#define  CABLE_ANGLE_DRUM_NUMBER 	4         // Cable Angle: drum: Drum system number for this function instance
#define  CABLE_ANGLE_DRUM_FUNCTION_BIT	5         // Cable Angle: f_pollbit: Drum system poll 1st payload byte bit for drum # (function instance)
#define  CABLE_ANGLE_DRUM_POLL_BIT	6         // Cable Angle: p_pollbit: Drum system poll 2nd payload byte bit for this type of function
#define  CABLE_ANGLE_MIN_TENSION 	7         // Cable Angle: Minimum tension required (units to match)
#define  CABLE_ANGLE_RATE_CT     	8         // Cable Angle: Rate count: Number of tension readings between cable angle msgs
#define  CABLE_ANGLE_ALARM_REPEAT	9         // Cable Angle: Number of times alarm msg is repeated
#define  CABLE_ANGLE_CALIB_COEF_0	10        // Cable Angle: Cable angle polynomial coefficient 0
#define  CABLE_ANGLE_CALIB_COEF_1	11        // Cable Angle: Cable angle polynomial coefficient 1
#define  CABLE_ANGLE_CALIB_COEF_2	12        // Cable Angle: Cable angle polynomial coefficient 2
#define  CABLE_ANGLE_CALIB_COEF_3	13        // Cable Angle: Cable angle polynomial coefficient 3
#define  CABLE_ANGLE_CANPRM_TENSION	14        // Cable Angle: CANID: can msg tension for sheave load-pin
#define  CABLE_ANGLE_MSG_TIME_POLL	15        // Cable Angle: CANID: cid_ten_poll: canid MC: Time msg/Group polling
#define  CABLE_ANGLE_TIMESYNC    	16        // Cable Angle: CANID: cid_gps_sync: canid time: GPS time sync distribution
#define  CABLE_ANGLE_HEARTBEAT_MSG	17        // Cable Angle: CANID: Heart-Beat: msg             
#define  CABLE_ANGLE_USEME       	18        // Cable Angle:  skip or use this function switch  

#define PARAM_LIST_CT_CABLE_ANGLE	18	// Count of same FUNCTION_TYPE in preceding list

#define  GPS_LIST_CRC            	1         // GPS: CRC                                        
#define  GPS_LIST_VERSION        	2         // GPS: Version number                             
#define  GPS_HEARTBEAT_TIME_CT   	3         // GPS: Time (ms) between unix time msgs           
#define  GPS_HEARTBEAT_LLH_CT    	4         // GPS: Time (ms) between burst of lat lon height msgs
#define  GPS_HEARTBEAT_LLH_DELAY_CT	5         // GPS: Time (ms) between lat/lon and lon/ht msgs  
#define  GPS_HEARTBEAT_TIME      	6         // GPS: Heartbeat unix time                        
#define  GPS_HEARTBEAT_LLH       	7         // GPS: Heartbeat (3 separate msgs) lattitude longitude height
#define  GPS_DISABLE_SYNCMSGS    	8         // GPS: time sync msgs; 0 = enable  1 = disable    
#define  GPS_TIME_SYNC_MSG       	9         // GPS: Time sync msg                              

#define PARAM_LIST_CT_GPS	9	// Count of same FUNCTION_TYPE in preceding list

#define  LOGGER_LIST_CRC         	1         // Logger: CRC                                     
#define  LOGGER_LIST_VERSION     	2         // Logger: Version number                          
#define  LOGGER_HEARTBEAT1_CT    	3         // Logger: Heartbeat count of time (ms) between msgs
#define  LOGGER_HEARTBEAT_MSG    	4         // Logger: CANID: Hearbeat sends running count of logged msgs

#define PARAM_LIST_CT_LOGGER	4	// Count of same FUNCTION_TYPE in preceding list

#define  ENGINE__CRC             	2         // Engine_sensor: CRC for cable angle list         
#define  ENGINE__VERSION         	3         // Engine_sensor: Version number for Cable Angle List
#define  ENGINE_SENSOR_SEG_CT    	4         // Engine_sensor: Number of black (or white) segments
#define  ENGINE_SENSOR_PRESS_OFFSET	5         // Engine_sensor: Manifold pressure offset         
#define  ENGINE_SENSOR_PRESS_SCALE	6         // Engine_sensor: Manifold pressure  scale (inch Hg)
#define  ENGINE_THERM1_CONST_B   	7         // Engine_sensor: Thermistor param: constant B     
#define  ENGINE_THERM1_R_SERIES  	8         // Engine_sensor: Thermistor param: Series resistor, fixed (K ohms)
#define  ENGINE_THERM1_R_ROOMTMP 	9         // Engine_sensor: Thermistor param: Thermistor room temp resistance (K ohms)
#define  ENGINE_THERM1_REF_TEMP  	10        // Engine_sensor: Thermistor param: Reference temp for thermistor
#define  ENGINE_THERM1_TEMP_OFFSET	11        // Engine_sensor: Thermistor param: Thermistor temp offset correction (deg C)

#define PARAM_LIST_CT_ENGINE_SENSOR	10	// Count of same FUNCTION_TYPE in preceding list

#define  YOGURT_1_LIST_CRC       	1         // Yogurt: crc: CRC for this list                  
#define  YOGURT_1_LIST_VERSION   	2         // Yogurt: version: Version number yogurt          
#define  YOGURT_1_THERM1_CONST_B 	3         // Yogurt: Thermistor1 param: B: constant B        
#define  YOGURT_1_THERM1_R_SERIES	4         // Yogurt: Thermistor1 param: RS: Series resistor, fixed (K ohms)
#define  YOGURT_1_THERM1_R_ROOMTMP	5         // Yogurt: Thermistor1 param: R0: Thermistor room temp resistance (K ohms)
#define  YOGURT_1_THERM1_REF_TEMP	6         // Yogurt: Thermistor1 param: TREF: Reference temp for thermistor
#define  YOGURT_1_THERM1_COEF_0  	7         // Yogurt: Thermistor1 param: poly[0]:  polynomial coefficient 0 (offset)
#define  YOGURT_1_THERM1_COEF_1  	8         // Yogurt: Thermistor1 param: poly[1]:  polynomial coefficient 1 (scale)
#define  YOGURT_1_THERM1_COEF_2  	9         // Yogurt: Thermistor1 param: poly[2]:  polynomial coefficient 2 (x^2)
#define  YOGURT_1_THERM1_COEF_3  	10        // Yogurt: Thermistor1 param: poly[3]:  polynomial coefficient 3 (x^3)
#define  YOGURT_1_THERM2_CONST_B 	11        // Yogurt: Thermistor2 param: B: constant B        
#define  YOGURT_1_THERM2_R_SERIES	12        // Yogurt: Thermistor2 param: RS: Series resistor, fixed (K ohms)
#define  YOGURT_1_THERM2_R_ROOMTMP	13        // Yogurt: Thermistor2 param: R0: Thermistor room temp resistance (K ohms)
#define  YOGURT_1_THERM2_REF_TEMP	14        // Yogurt: Thermistor2 param: TREF: Reference temp for thermistor
#define  YOGURT_1_THERM2_COEF_0  	15        // Yogurt: Thermistor2 param: poly[0]:  polynomial coefficient 0 (offset)
#define  YOGURT_1_THERM2_COEF_1  	16        // Yogurt: Thermistor2 param: poly[1]:  polynomial coefficient 1 (scale)
#define  YOGURT_1_THERM2_COEF_2  	17        // Yogurt: Thermistor2 param: poly[2]:  polynomial coefficient 2 (x^2)
#define  YOGURT_1_THERM2_COEF_3  	18        // Yogurt: Thermistor2 param: poly[3]:  polynomial coefficient 3 (x^3)
#define  YOGURT_1_THERM3_CONST_B 	19        // Yogurt: Thermistor3 param: B: constant B        
#define  YOGURT_1_THERM3_R_SERIES	20        // Yogurt: Thermistor3 param: RS: Series resistor, fixed (K ohms)
#define  YOGURT_1_THERM3_R_ROOMTMP	21        // Yogurt: Thermistor3 param: R0: Thermistor room temp resistance (K ohms)
#define  YOGURT_1_THERM3_REF_TEMP	22        // Yogurt: Thermistor3 param: TREF: Reference temp for thermistor
#define  YOGURT_1_THERM3_COEF_0  	23        // Yogurt: Thermistor3 param: poly[0]:  polynomial coefficient 0 (offset)
#define  YOGURT_1_THERM3_COEF_1  	24        // Yogurt: Thermistor3 param: poly[1]:  polynomial coefficient 1 (scale)
#define  YOGURT_1_THERM3_COEF_2  	25        // Yogurt: Thermistor3 param: poly[2]:  polynomial coefficient 2 (x^2)
#define  YOGURT_1_THERM3_COEF_3  	26        // Yogurt: Thermistor3 param: poly[3]:  polynomial coefficient 3 (x^3)
#define  YOGURT_1_THERM4_CONST_B 	27        // Yogurt: Thermistor4 param: B: constant B        
#define  YOGURT_1_THERM4_R_SERIES	28        // Yogurt: Thermistor4 param: RS: Series resistor, fixed (K ohms)
#define  YOGURT_1_THERM4_R_ROOMTMP	29        // Yogurt: Thermistor4 param: R0: Thermistor room temp resistance (K ohms)
#define  YOGURT_1_THERM4_REF_TEMP	30        // Yogurt: Thermistor4 param: TREF: Reference temp for thermistor
#define  YOGURT_1_THERM4_COEF_0  	31        // Yogurt: Thermistor4 param: poly[0]:  polynomial coefficient 0 (offset)
#define  YOGURT_1_THERM4_COEF_1  	32        // Yogurt: Thermistor4 param: poly[1]:  polynomial coefficient 1 (scale)
#define  YOGURT_1_THERM4_COEF_2  	33        // Yogurt: Thermistor4 param: poly[2]:  polynomial coefficient 2 (x^2)
#define  YOGURT_1_THERM4_COEF_3  	34        // Yogurt: Thermistor4 param: poly[3]:  polynomial coefficient 3 (x^3)
#define  YOGURT_1_CTLTEMP_HEAT_PAST	35        // Yogurt: Pasteur: Control set-point temperature (deg F) heat to this temp
#define  YOGURT_1_CTLTEMP_DUR_PAST	36        // Yogurt: Pasteur: Time duration at temp (hours.frac_hours)
#define  YOGURT_1_CTLTEMP_COOL_PAST	37        // Yogurt: Pasteur: Control end-point temperature (deg F) cool to this temp
#define  YOGURT_1_CTLTEMP_HEAT_FERM	38        // Yogurt: Ferment: Control set-point temperature (deg F) heat to this temp
#define  YOGURT_1_CTLTEMP_DUR_FERM	39        // Yogurt: Ferment: Time duration at temp (hours.frac_hours)
#define  YOGURT_1_CTLTEMP_COOL_FERM	40        // Yogurt: Ferment: Control end-point temperature (deg F) cool to this temp
#define  YOGURT_1_CTL_THERM_SHELL	41        // Yogurt: Thermistor number for shell temp (0 - 3)
#define  YOGURT_1_CTL_THERM_POT  	42        // Yogurt: Thermistor number for center of pot temp (0 - 3)
#define  YOGURT_1_CTL_THERM_AIRIN	43        // Yogurt: Thermistor number for air inlet to fan temp (0 - 3)
#define  YOGURT_1_CTL_THERM_AIROUT	44        // Yogurt: Thermistor number for air coming out of holes (0 - 3)
#define  YOGURT_1_CTL_THERM_LOOP_P	45        // Yogurt: Control loop: Proportional coefficient  
#define  YOGURT_1_CTL_THERM_LOOP_I	46        // Yogurt: Control loop: Integral coefficient      
#define  YOGURT_1_CTL_THERM_LOOP_D	47        // Yogurt: Control loop: Derivative coefficient    
#define  YOGURT_1_CMD            	48        // Yogurt: CANID: cid_yog_cmd: Yogurt maker parameters
#define  YOGURT_1_MSG            	49        // Yogurt: CANID: cid_yog_msg: Yogurt maker msgs   
#define  YOGURT_1_HB             	50        // Yogurt: CANID: cid_yog_hb: Yogurt maker heart-beats
#define  YOGURT_1_HEATCONSTANT_KM_P	51        // Yogurt: Control, stored heat constant Pasteur phase
#define  YOGURT_1_HEATCONSTANT_KM_M	52        // Yogurt: Control, stored heat constant Ferment phase
#define  YOGURT_1_INTEGRATEINIT_A	53        // Yogurt: Control, integrator initialization, a of  a + b*x 
#define  YOGURT_1_INTEGRATEINIT_B	54        // Yogurt: Control, integrator initialization, b of  a + b*x 
#define  YOGURT_1_STABILIZETIMEDELAY_P	55        // Yogurt: Control, time delay for temperature stabilization, Pasteur
#define  YOGURT_1_STABILIZETIMEDELAY_F	56        // Yogurt: Control, time delay for temperature stabilization, Ferment

#define PARAM_LIST_CT_YOGURT_1	56	// Count of same FUNCTION_TYPE in preceding list


#define READINGS_LIST_COUNT 26
#define  TENSION_READ_FILTADC_THERM1	1         // Tension: READING: double thrm[0]; Filtered ADC for Thermistor on AD7799
#define  TENSION_READ_FILTADC_THERM2	2         // Tension: READING: double thrm[1]; Filtered ADC for Thermistor external
#define  TENSION_READ_FORMCAL_THERM1	4         // Tension: READING: double degX[0]; Formula computed thrm for Thermistor on AD7799
#define  TENSION_READ_FORMCAL_THERM2	5         // Tension: READING: double degX[1]; Formula computed thrm for Thermistor external
#define  TENSION_READ_POLYCAL_THERM1	6         // Tension: READING: double degC[0]; Polynomial adjusted degX for Thermistor on AD7799
#define  TENSION_READ_POLYCAL_THERM2	7         // Tension: READING: double degC[1]; Polynomial adjusted degX for Thermistor external
#define  TENSION_READ_AD7799_LGR 	8         // Tension: READING: int32_t lgr; last_good_reading (no filtering or adjustments)
#define  TENSION_READ_AD7799_CALIB_1	9         // Tension: READING: ten_iircal[0];  AD7799 filtered (fast) and calibrated
#define  TENSION_READ_AD7799_CALIB_2	10        // Tension: READING: ten_iircal[1];  AD7799 filtered (slow) and calibrated
#define  TENSION_READ_CIC_RAW    	11        // Tension: READING: int32_t cicraw; cic before averaging
#define  TENSION_READ_CIC_AVE    	12        // Tension: READING: int32_t cicave; cic averaged for determining offset
#define  TENSION_READ_CIC_AVE_CT 	13        // Tension: READING: int32_t ave.n;  current count for above average
#define  TENSION_READ_OFFSET_REG 	14        // Tension: READING: Last reading of AD7799 offset register
#define  TENSION_READ_OFFSET_REG_FILT	15        // Tension: READING: Last filtered AD7799 offset register
#define  TENSION_READ_OFFSET_REG_RDBK	16        // Tension: READING: Last filtered AD7799 offset register set read-back
#define  TENSION_READ_FULLSCALE_REG	17        // Tension: READING: Last reading of AD7799 fullscale register
#define  TENSION_READ_POLL_MASK  	18        // Tension: READING: Mask for first two bytes of a poll msg (necessary?)
#define  TENSION_READ_READINGSCT 	19        // Tension: READING: Running count of readings (conversions completed)
#define  TENSION_READ_READINGSCT_LASTPOLL	20        // Tension: READING: Reading count the last time a poll msg sent
#define  TENSION_READ_OFFSET_CT  	21        // Tension: READING: Running ct of offset updates  
#define  TENSION_READ_ZERO_FLAG  	22        // Tension: READING: 1 = zero-calibration operation competed
#define  TENSION_READ_STATUS_BYTE	23        // Tension: READING: Reading status byte           
#define  TENSION_READ_IIR_OFFSET_K	24        // Tension: READING: IIR filter for offsets: parameter for setting time constant
#define  TENSION_READ_IIR_OFFSET_SCL	25        // Tension: READING: IIR filter for offsets: Scaling to improve spare bits with integer math
#define  TENSION_READ_IIR_Z_RECAl_W_K	26        // Tension: READING: IIR filter for zeroing: parameter for setting time constant
#define  TENSION_READ_IIR_Z_RECAl_W_SCL	27        // Tension: READING: IIR filter for zeroing: Scaling to improve spare bits with integer math

#define FUNC_BIT_PARAM_COUNT 28
#define  POLL_FUNC_BIT_TENSION   	0x1       // TENSION             Function bit: 2nd byte of poll msg: TENSION     
#define  POLL_FUNC_BIT_CABLE_ANGLE	0x2       // CABLE_ANGLE         Function bit: 2nd byte of poll msg: CABLE_ANGLE 
#define  POLL_FUNC_BIT_SHAFT_ODO_SPD	0x4       // SHAFT_ENCODER       Function bit: 2nd byte of poll msg: shaft odometer & speed
#define  POLL_FUNC_BIT_TILT      	0x8       // TILT_SENSE          Function bit: 2nd byte of poll msg: TILT        
#define  POLL_DO_NOT             	0x1       // DUMMY               Selection bit: No reply sent to poll msg        
#define  POLL_DRUM_BIT_2         	0x2       // SELECT_DRUM_1       Selection bit: 1st byte of poll msg Drum #1     
#define  POLL_DRUM_BIT_3         	0x4       // SELECT_DRUM_2       Selection bit: 1st byte of poll msg Drum #2     
#define  POLL_DRUM_BIT_4         	0x8       // SELECT_DRUM_3       Selection bit: 1st byte of poll msg Drum #3     
#define  POLL_DRUM_BIT_5         	0x10      // SELECT_DRUM_4       Selection bit: 1st byte of poll msg Drum #4     
#define  POLL_DRUM_BIT_6         	0x20      // SELECT_DRUM_5       Selection bit: 1st byte of poll msg Drum #5     
#define  POLL_DRUM_BIT_7         	0x40      // SELECT_DRUM_6       Selection bit: 1st byte of poll msg Drum #6     
#define  POLL_DRUM_BIT_8         	0x80      // SELECT_DRUM_7       Selection bit: 1st byte of poll msg Drum #7     
#define  STATUS_TENSION_BIT_NONEW	0x1       // TENSION             status: No new reading since last poll msg sent 
#define  STATUS_TENSION_BIT_EXCEEDHI	0x2       // TENSION             status: Reading limit hi exceed (open (-) connection?)
#define  STATUS_TENSION_BIT_EXCEEDLO	0x4       // TENSION             status: Reading limit lo exceed (open (+ or both) connection?)
#define  STATUS_TENSION_BIT_4    	0x8       // TENSION             status: spare 0x8                               
#define  STATUS_TENSION_BIT_5    	0x10      // TENSION             status: spare 0x10                              
#define  STATUS_TENSION_BIT_6    	0x20      // TENSION             status: spare 0x20                              
#define  STATUS_TENSION_BIT_7    	0x40      // TENSION             status: spare 0x40                              
#define  STATUS_TENSION_BIT_DONOTUSE	0x80      // TENSION             status: spare 0x80                              
#define  USEME_TENSION_BIT_AD7799_1	0x1       // TENSION             useme: 1st AD7799                               
#define  USEME_TENSION_BIT_AD7799_2	0x2       // TENSION             useme: 2nd AD7799                               
#define  USEME_TENSION_BIT_3     	0x4       // TENSION             useme: spare 0x4                                
#define  USEME_TENSION_BIT_4     	0x8       // TENSION             useme: spare 0x8                                
#define  USEME_TENSION_BIT_5     	0x10      // TENSION             useme: spare 0x10                               
#define  USEME_TENSION_BIT_6     	0x20      // TENSION             useme: spare 0x20                               
#define  USEME_TENSION_BIT_7     	0x40      // TENSION             useme: spare 0x40                               
#define  USEME_TENSION_BIT_8     	0x80      // TENSION             useme: spare 0x80                               

#define FUNCTION_TYPE_COUNT 14
#define  FUNCTION_TYPE_SHAFT_ENCODER           	1         // Sensor, shaft: Drive shaft encoder              
#define  FUNCTION_TYPE_ENGINE_SENSOR           	2         // Sensor, engine: rpm, manifold pressure, throttle setting, temperature
#define  FUNCTION_TYPE_TENSION_a               	3         // Tension_a: Tension AD7799 #1                    
#define  FUNCTION_TYPE_TENSION_a2              	4         // Tension_a: Tension AD7799 #2                    
#define  FUNCTION_TYPE_CABLE_ANGLE             	5         // Cable angle AD7799                              
#define  FUNCTION_TYPE_TENSION_c               	6         // Tension_c: Tension op-amp                       
#define  FUNCTION_TYPE_TIMESYNC                	7         // GPS time sync distribution msg                  
#define  FUNCTION_TYPE_HC_SANDBOX_1            	8         // Host Controller: sandbox function 1             
#define  FUNCTION_TYPE_MC                      	9         // Master Controller                               
#define  FUNCTION_TYPE_TILT_SENSE              	10        // Tilt sensor                                     
#define  FUNCTION_TYPE_YOGURT_1                	11        // Yogurt_1: Ver 1 of maker                        
#define  FUNCTION_TYPE_GPS                     	12        // GPS                                             
#define  FUNCTION_TYPE_LOGGER                  	13        // Logger                                          
#define  FUNCTION_TYPE_CANSENDER               	14        // Cansender                                       

#define READINGS_BOARD_COUNT 15
#define  PROG_TENSION_READINGS_BOARD_NUM_AD7799	1         // Number of AD7799 that successfully initialized  
#define  PROG_TENSION_READINGS_BOARD_CAN_TXERR	2         // Count: total number of msgs returning a TERR flags (including retries)
#define  PROG_TENSION_READINGS_BOARD_CAN_TX_BOMBED	3         // Count: number of times msgs failed due to too many TXERR
#define  PROG_TENSION_READINGS_BOARD_CAN_ALST0_ERR	4         // Count: arbitration failure total                
#define  PROG_TENSION_READINGS_BOARD_CAN_ALST0_NART_ERR	5         // Count: arbitration failure when NART is on      
#define  PROG_TENSION_READINGS_BOARD_CAN_MSGOVRFLO	6         // Count: Buffer overflow when adding a msg        
#define  PROG_TENSION_READINGS_BOARD_CAN_SPURIOUS_INT	7         // Count: TSR had no RQCPx bits on (spurious interrupt)
#define  PROG_TENSION_READINGS_BOARD_CAN_NO_FLAGGED	8         // Count:                                          
#define  PROG_TENSION_READINGS_BOARD_CAN_PFOR_BK_ONE	9         // Count: Instances that pfor was adjusted in TX interrupt
#define  PROG_TENSION_READINGS_BOARD_CAN_PXPRV_FWD_ONE	10        // Count: Instances that pxprv was adjusted in for loop
#define  PROG_TENSION_READINGS_BOARD_CAN_RX0ERR_CT	11        // Count: FIFO 0 overrun                           
#define  PROG_TENSION_READINGS_BOARD_CAN_RX1ERR_CT	12        // Count: FIFO 1 overrun                           
#define  PROG_TENSION_READINGS_BOARD_CAN_CP1CP2	13        // Count: (RQCP1 | RQCP2) unexpectedly ON          
#define  PROG_TENSION_READINGS_BOARD_TXINT_EMPTYLIST	14        // Count: TX interrupt with pending list empty     
#define  PROG_TENSION_READINGS_BOARD_CAN1_BOGUS_CT	15        // Count: bogus CAN1 IDs rejected                  

#define MISC_SYS_COUNT 1
#define  LAUNCH_PARAM_BURST_SIZE 		8	// ASCII value 8         Number of CAN msgs in a burst when sending launch parameters

/* TOTAL COUNT OF #defines = 493  */
/* Test 2016/06/12 */

