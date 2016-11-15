// 2016-11-12 16:46:44.497

// =========== PARAMETER ARRAY/TABLE SIZES ============================
// Note: The instances of the same function types should have the same size. 
//        If not, then some is wrong with the PARAMETERS_VAL_INSERT. 

#define CABLE_ANGLE_1_PARAM_SIZE 18
#define CANSENDER_1_PARAM_SIZE 6
#define CANSENDER_2_PARAM_SIZE 6
#define GPS_1_PARAM_SIZE 9
#define GPS_2_PARAM_SIZE 9
#define LOGGER_1_PARAM_SIZE 4
#define LOGGER_2_PARAM_SIZE 4
#define TENSION_a11_PARAM_SIZE 51
#define TENSION_a12_PARAM_SIZE 51
#define TENSION_a21_PARAM_SIZE 51
#define TENSION_a22_PARAM_SIZE 51
#define YOGURT_1_PARAM_SIZE 56


// =====================================================================
#ifdef CABLE_ANGLE_1	// Include following parameters?
const uint32_t paramval[] = {
 CABLE_ANGLE_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6    1 Cable_angle_1: CRC for Cable angle list                                     */
 0x00000001 , /*   2 1                    5    2 Cable_angle_1: Version number for cable angle List                          */
 0x00002000 , /*   3 8192                 6    3 Cable Angle_1: Heart-Beat: Count of time ticks between autonomous msgs      */
 0x00000001 , /*   4 1                    6    4 Cable angle_1: Drum system number for this function instance                */
 0x00000001 , /*   5 1                    6    5 Cable angle_1: f_pollbit: Drum system poll 1st payload byte bit for drum #  */
 0x00000000 , /*   6 0                    6    6 Cable angle_1: p_pollbit: Drum system poll 2nd payload byte bit for this type of function*/
 0x42C83333 , /*   7 100.1               11    7 Cable Angle_1: Minimum tension required (units to match)                    */
 0x00000002 , /*   8 2                    6    8 Cable Angle_1: Rate count: Number of tension readings between cable angle msgs*/
 0x00000009 , /*   9 9                    6    9 Cable Angle_1: Number of times alarm msg is repeated                        */
 0x00000000 , /*  10 0.0                 11   10 Cable Angle_1: Cable angle polynomial coefficient 0                         */
 0x3F800000 , /*  11 1.0                 11   11 Cable Angle_1: Cable angle polynomial coefficient 1                         */
 0x00000000 , /*  12 0.0                 11   12 Cable Angle_1: Cable angle polynomial coefficient 2                         */
 0x00000000 , /*  13 0.0                 11   13 Cable Angle_1: Cable angle polynomial coefficient 3                         */
 0x38200000 , /*  14 CANID_MSG_TENSION_a21  17   14 Cable angle_1: CANID: can msg tension from sheave load-pin                  */
 0x20000000 , /*  15 CANID_MSG_TIME_POLL  17   15 Cable angle_1: CANID: MC: Time msg/Group polling                            */
 0x00400000 , /*  16 CANID_HB_TIMESYNC   17   16 Cable angle_1: CANID: GPS time sync distribution msg                        */
 0xE0A00000 , /*  17 CANID_HB_CABLE_ANGLE_1  17   17 Cable angle_1: CANID: Heartbeat msg                                         */
 0x00000001 , /*  18 1                    6   18 Cable Angle_1: skip or use this function switch                             */
};
#endif

// =====================================================================
#ifdef CANSENDER_1	// Include following parameters?
const uint32_t paramval[] = {
 CANSENDER_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Cansender_1: 1 CRC                                                              */
 0x00000001 , /*   2 1                    5  Cansender_1: 2 Version number                                                   */
 0x000001F4 , /*   3 500                  6  Cansender_1: 3 Heartbeat count of time (ms) between msgs                        */
 0xF0200000 , /*   4 CANID_HB_CANSENDER_1  17  Cansender_1: 4 CANID: Hearbeat sends running count                              */
 0xE2000000 , /*   5 CANID_POLL_CANSENDER  17  Cansender_1: 5 CANID: Poll this cansender                                       */
 0xE2200000 , /*   6 CANID_POLLR_CANSENDER_1  17  Cansender_1: 6 CANID: Response to POLL                                          */
};
#endif

// =====================================================================
#ifdef CANSENDER_2	// Include following parameters?
const uint32_t paramval[] = {
 CANSENDER_2_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Cansender_2: 1 CRC                                                              */
 0x00000001 , /*   2 1                    5  Cansender_2: 2 Version number                                                   */
 0x00000177 , /*   3 375                  6  Cansender_2: 3 Heartbeat count of time (ms) between msgs                        */
 0xF0400000 , /*   4 CANID_HB_CANSENDER_2  17  Cansender_2: 4 CANID: Hearbeat sends running count                              */
 0xE2000000 , /*   5 CANID_POLL_CANSENDER  17  Cansender_2: 5 CANID: Poll this cansender                                       */
 0xE2400000 , /*   6 CANID_POLLR_CANSENDER_2  17  Cansender_2: 6 CANID: Response to POLL                                          */
};
#endif

// =====================================================================
#ifdef GPS_1	// Include following parameters?
const uint32_t paramval[] = {
 GPS_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  GPS_1: 1 CRC                                                                    */
 0x00000001 , /*   2 1                    5  GPS_1: 2 Version number                                                         */
 0x000003E8 , /*   3 1000                 6  GPS_1: 3 Time (ms) between unix time HB msgs                                    */
 0x00002710 , /*   4 10000                6  GPS_1: 4 Time (ms) between burst of lat lon height HB msgs                      */
 0x0000044C , /*   5 1100                 6  GPS_1: 5 Time (ms) between lat/lon and lon/ht msgs                              */
 0xE1000000 , /*   6 CANID_HB_GPS_TIME_1  17  GPS_1: 6 Heartbeat unix time                                                    */
 0xE1C00000 , /*   7 CANID_HB_GPS_LLH_1  17  GPS_1: 7 Heartbeat (3 separate msgs) lattitude longitude height                 */
 0x00000000 , /*   8 0                    6  GPS_1: 8 time sync msgs: 0 = enable  1 = disable                                */
 0x00400000 , /*   9 CANID_HB_TIMESYNC   17  GPS_1: 9 Time sync msg                                                          */
};
#endif

// =====================================================================
#ifdef GPS_2	// Include following parameters?
const uint32_t paramval[] = {
 GPS_2_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  GPS_2: 1 CRC                                                                    */
 0x00000001 , /*   2 1                    5  GPS_2: 2 Version number                                                         */
 0x000003E8 , /*   3 1000                 6  GPS_2: 3 Time (ms) between unix time HB msgs                                    */
 0x00002710 , /*   4 10000                6  GPS_2: 4 Time (ms) between burst of lat lon height HB msgs                      */
 0x0000041A , /*   5 1050                 6  GPS_2: 5 Time (ms) between lat/lon and lon/ht msgs                              */
 0xE1E00000 , /*   6 CANID_HB_GPS_TIME_2  17  GPS_2: 6 Heartbeat unix time                                                    */
 0xE2600000 , /*   7 CANID_HB_GPS_LLH_2  17  GPS_2: 7 Heartbeat (3 separate msgs) lattitude longitude height                 */
 0x00000000 , /*   8 0                    6  GPS_2: 8 time sync msgs: 0 = enable  1 = disable                                */
 0x00600000 , /*   9 CANID_HB_TIMESYNC_2  17  GPS_2: 9 Time sync msg                                                          */
};
#endif

// =====================================================================
#ifdef LOGGER_1	// Include following parameters?
const uint32_t paramval[] = {
 LOGGER_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Logger_1: 1 CRC                                                                 */
 0x00000001 , /*   2 1                    5  Logger_1: 2 Version number                                                      */
 0x00001F40 , /*   3 8000                 6  Logger_1: 3 Heartbeat count of time (ms) between msgs                           */
 0x## CAN ID NOT FOUND ## , /*   4 CANID_HB_LOGGER_1   17  Logger_1: 4 CANID: Hearbeat sends running count of logged msgs                  */
};
#endif

// =====================================================================
#ifdef LOGGER_2	// Include following parameters?
const uint32_t paramval[] = {
 LOGGER_2_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Logger_2: 1 CRC                                                                 */
 0x00000001 , /*   2 1                    5  Logger_2: 2 Version number                                                      */
 0x00001F40 , /*   3 8000                 6  Logger_2: 3 Heartbeat count of time (ms) between msgs                           */
 0xE1A00000 , /*   4 CANID_HB_LOGGER_2   17  Logger_2: 4 CANID: Hearbeat sends running count of logged msgs                  */
};
#endif

// =====================================================================
#ifdef TENSION_a11	// Include following parameters?
const uint32_t paramval[] = {
 TENSION_a11_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a11: 1 CRC for tension list                                             */
 0x00000001 , /*   2 1                    5  Tension_a11: 2 Version number for Tension List                                  */
 0x0001273C , /*   3 75580                5  Tension_a11: 3 AD7799 final offset                                              */
 0x3980F990 , /*   4 0.246E-3            11  Tension_a11: 4 AD7799 final Scale (convert to kgf)                              */
 0x45534000 , /*   5 3380.0              11  Tension_a11:  5 Thermistor1 param: constant B                                   */
 0x41200000 , /*   6 10.0                11  Tension_a11:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a11:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a11:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a11:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a11: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a11: 11 Thermistor2 param: constant B                                   */
 0x41200000 , /*  12 10.0                11  Tension_a11: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a11: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a11: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a11: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a11: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a11: 17 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a11: 18 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a11: 19 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 2 (x^2)*/
 0x00000000 , /*  20 0.0                 11  Tension_a11: 20 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 3 (x^3)*/
 0x40A00000 , /*  21 5.0                 11  Tension_a11: 21 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a11: 22 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a11: 23 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 2 (x^2)*/
 0x00000000 , /*  24 0.0                 11  Tension_a11: 24 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 3 (x^3)*/
 0x000000FA , /*  25 250                  6  Tension_a11: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs*/
 0x00000001 , /*  26 1                    6  Tension_a11: 26 Drum system number for this function instance                   */
 0x00000001 , /*  27 1                    6  Tension_a11: 27 Drum system poll 2nd payload byte bit for this type of function */
 0x00000001 , /*  28 1                    6  Tension_a11: 28 Drum system poll 1st payload byte bit for drum # (function instance)*/
 0x38000000 , /*  29 CANID_MSG_TENSION_a11  17  Tension_a11: 29 CANID: can msg tension for AD7799 #1                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a11: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a11: 31 CANID: GPS time sync distribution msg                           */
 0xE0600000 , /*  32 CANID_HB_TENSION_a11  17  Tension_a11: 32 CANID: Heartbeat msg                                            */
 0xF800010C , /*  33 CANID_TST_TENSION_a11  17  Tension_a11: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a11: 34 IIR Filter factor: divisor sets time constant: reading for polled msg*/
 0x00000080 , /*  35 128                  6  Tension_a11: 35 Filter scale : upscaling (due to integer math): for polled msg  */
 0x00000064 , /*  36 100                  6  Tension_a11: 36 IIR Filter factor: divisor sets time constant: reading for heart-beat  msg*/
 0x00000080 , /*  37 128                  6  Tension_a11: 37 Filter scale : upscaling (due to integer math): for heart-beat  msg*/
 0x00000003 , /*  38 3                    6  Tension_a11: 38 skip or use this function swit ch                               */
 0x0000000A , /*  39 10                   6  Tension_a11: 39 IIR Filter factor: zero recalibration                           */
 0x00000040 , /*  40 64                   6  Tension_a11: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a11: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a11: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a11: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a11: 44 CANID 1 added to CAN hardware filter to allow incoming msg      */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a11: 45 CANID 2 added to CAN hardware filter to allow incoming msg      */
 0xF800010C , /*  46 CANID_TST_TENSION_a11  17  Tension_a11: 46 CANID 3 added to CAN hardware filter to allow incoming msg      */
 0x05C00004 , /*  47 CANID_CMD_TENSION_a11I  17  Tension_a11: 47 CANID 4 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a11: 48 CANID 5 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a11: 49 CANID 6 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a11: 50 CANID 7 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a11: 51 CANID 8 added to CAN hardware filter to allow incoming msg      */
};
#endif

// =====================================================================
#ifdef TENSION_a12	// Include following parameters?
const uint32_t paramval[] = {
 TENSION_a12_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a12:   1 CRC for tension list                                           */
 0x00000001 , /*   2 1                    5  Tension_a12:   2 Version number for Tension List                                */
 0x000012B5 , /*   3 4789                 5  Tension_a12:   3 AD7799 offset                                                  */
 0x3B39D2A6 , /*   4 0.283543155E-2      11  Tension_a12: AD7799 #1 Scale (convert to kgf)                                   */
 0x45534000 , /*   5 3380.0              11  Tension_a12:  5 Thermistor1 param: constant B                                   */
 0x41200000 , /*   6 10.0                11  Tension_a12:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a12:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a12:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a12:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a12: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a12: 11 Thermistor2 param: constant B                                   */
 0x41200000 , /*  12 10.0                11  Tension_a12: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a12: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a12: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a12: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a12: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a12: 17 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a12: 18 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a12: 19 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 2 (x^2)*/
 0x00000000 , /*  20 0.0                 11  Tension_a12: 20 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 3 (x^3)*/
 0x40A00000 , /*  21 5.0                 11  Tension_a12: 21 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a12: 22 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a12: 23 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 2 (x^2)*/
 0x00000000 , /*  24 0.0                 11  Tension_a12: 24 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 3 (x^3)*/
 0x000007D0 , /*  25 2000                 6  Tension_a12: 25 Heart-Beat: Count of time ticks between autonomous msgs         */
 0x00000002 , /*  26 2                    6  Tension_a12: 26 Drum system number for this function instance                   */
 0x00000001 , /*  27 1                    6  Tension_a12: 27 Drum system poll 2nd payload byte bit for this type of function */
 0x00000002 , /*  28 2                    6  Tension_a12: 28 Drum system poll 1st payload byte bit for drum # (function instance)*/
 0x38400000 , /*  29 CANID_MSG_TENSION_a12  17  Tension_a12: 29 CANID: can msg tension for AD7799 #2                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a12: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a12: 31 CANID: GPS time sync distribution msg                           */
 0xE0800000 , /*  32 CANID_HB_TENSION_a12  17  Tension_a12: 32 CANID: Heartbeat msg                                            */
 0xF800020C , /*  33 CANID_TST_TENSION_a12  17  Tension_a12: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a12: 34 IIR Filter factor: divisor sets time constant: reading for polled msg*/
 0x00000080 , /*  35 128                  6  Tension_a12: 35 Filter scale : upscaling (due to integer math): for polled msg  */
 0x00000200 , /*  36 512                  6  Tension_a12: 36 IIR Filter factor: divisor sets time constant: reading for heart-beat msg*/
 0x00000080 , /*  37 128                  6  Tension_a12: 37 Filter scale : upscaling (due to integer math): for heart-beat  msg*/
 0x00000003 , /*  38 3                    6  Tension_a12: 38 skip or use this function switch                                */
 0x0000000A , /*  39 10                   6  Tension_a12: 39 IIR Filter factor: zero recalibration                           */
 0x00000080 , /*  40 128                  6  Tension_a12: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a12: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a12: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a12: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a12: 44 CANID 1 added to CAN hardware filter to allow incoming msg      */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a12: 45 CANID 2 added to CAN hardware filter to allow incoming msg      */
 0xF800020C , /*  46 CANID_TST_TENSION_a12  17  Tension_a12: 46 CANID 3 added to CAN hardware filter to allow incoming msg      */
 0xF800005C , /*  47 CANID_CMD_TENSION_a12I  17  Tension_a12: 47 CANID 4 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a12: 48 CANID 5 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a12: 49 CANID 6 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a12: 50 CANID 7 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a12: 51 CANID 8 added to CAN hardware filter to allow incoming msg      */
};
#endif

// =====================================================================
#ifdef TENSION_a21	// Include following parameters?
const uint32_t paramval[] = {
 TENSION_a21_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a21:   1 CRC for tension list                                           */
 0x00000001 , /*   2 1                    5  Tension_a21:   2 Version number for Tension List                                */
 0x0001456E , /*   3 83310                5  Tension_a21:   3 AD7799 offset                                                  */
 0x3980F990 , /*   4 0.246E-3            11  Tension_a21: AD7799 #1 Scale (convert to kgf)                                   */
 0x45534000 , /*   5 3380.0              11  Tension_a21:  5 Thermistor1 param: constant B                                   */
 0x41200000 , /*   6 10.0                11  Tension_a21:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a21:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a21:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a21:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a21: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a21: 11 Thermistor2 param: constant B                                   */
 0x41200000 , /*  12 10.0                11  Tension_a21: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a21: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a21: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a21: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a21: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a21: 17 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a21: 18 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a21: 19 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 2 (x^2)*/
 0x00000000 , /*  20 0.0                 11  Tension_a21: 20 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 3 (x^3)*/
 0x40A00000 , /*  21 5.0                 11  Tension_a21: 21 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a21: 22 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a21: 23 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 2 (x^2)*/
 0x00000000 , /*  24 0.0                 11  Tension_a21: 24 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 3 (x^3)*/
 0x000003E8 , /*  25 1000                 6  Tension_a21: 25 Heart-Beat: Count of time ticks (milliseconds) between autonomous msgs*/
 0x00000001 , /*  26 1                    6  Tension_a21: 26 Drum system number for this function instance                   */
 0x00000001 , /*  27 1                    6  Tension_a21: 27 Drum system poll 2nd payload byte bit for this type of function */
 0x00000001 , /*  28 1                    6  Tension_a21: 28 Drum system poll 1st payload byte bit for drum # (function instance)*/
 0x38200000 , /*  29 CANID_MSG_TENSION_a21  17  Tension_a21: 29 CANID: can msg tension for AD7799 #2                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a21: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a21: 31 CANID: GPS time sync distribution msg                           */
 0xE0C00000 , /*  32 CANID_HB_TENSION_a21  17  Tension_a21: 32 CANID: Heartbeat msg                                            */
 0xF800030C , /*  33 CANID_TST_TENSION_a21  17  Tension_a21: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a21: 34 IIR Filter factor: divisor sets time constant: reading for polled msg*/
 0x00000080 , /*  35 128                  6  Tension_a21: 35 Filter scale : upscaling (due to integer math): for polled msg  */
 0x00000200 , /*  36 512                  6  Tension_a21: 36 IIR Filter factor: divisor sets time constant: reading for heart-beat msg*/
 0x00000080 , /*  37 128                  6  Tension_a21: 37 Filter scale : upscaling (due to integer math): for heart-beat  msg*/
 0x00000003 , /*  38 3                    6  Tension_a: 38 skip or use this function switch                                  */
 0x0000000A , /*  39 10                   6  Tension_a21: 39 IIR Filter factor: zero recalibration                           */
 0x00000080 , /*  40 128                  6  Tension_a21: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a21: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a21: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a21: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a21: 44 CANID 1 added to CAN hardware filter to allow incoming msg      */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a21: 45 CANID 2 added to CAN hardware filter to allow incoming msg      */
 0xF800030C , /*  46 CANID_TST_TENSION_a21  17  Tension_a21: 46 CANID 3 added to CAN hardware filter to allow incoming msg      */
 0x05E00004 , /*  47 CANID_CMD_TENSION_a21I  17  Tension_a21: 47 CANID 4 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a21: 48 CANID 5 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a21: 49 CANID 6 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a21: 50 CANID 7 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a21: 51 CANID 8 added to CAN hardware filter to allow incoming msg      */
};
#endif

// =====================================================================
#ifdef TENSION_a22	// Include following parameters?
const uint32_t paramval[] = {
 TENSION_a22_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Tension_a22:   1 CRC for tension list                                           */
 0x00000001 , /*   2 1                    5  Tension_a22:   2 Version number for Tension List                                */
 0x000012B5 , /*   3 4789                 5  Tension_a22:   3 AD7799 offset                                                  */
 0x3B39D2A6 , /*   4 0.283543155E-2      11  Tension_a22: AD7799 #1 Scale (convert to kgf)                                   */
 0x45534000 , /*   5 3380.0              11  Tension_a22:  5 Thermistor1 param: constant B                                   */
 0x41200000 , /*   6 10.0                11  Tension_a22:  6 Thermistor1 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*   7 10.0                11  Tension_a22:  7 Thermistor1 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*   8 290.0               11  Tension_a22:  8 Thermistor1 param: Reference temp for thermistor                */
 0x00000000 , /*   9 0.0                 11  Tension_a22:  9 Thermistor1 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  10 1.0                 11  Tension_a22: 10 Thermistor1 param: Thermistor temp scale correction             */
 0x45534000 , /*  11 3380.0              11  Tension_a22: 11 Thermistor2 param: constant B                                   */
 0x41200000 , /*  12 10.0                11  Tension_a22: 12 Thermistor2 param: Series resistor, fixed (K ohms)              */
 0x41200000 , /*  13 10.0                11  Tension_a22: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)     */
 0x43910000 , /*  14 290.0               11  Tension_a22: 14 Thermistor2 param: Reference temp for thermistor                */
 0x00000000 , /*  15 0.0                 11  Tension_a22: 15 Thermistor2 param: Thermistor temp offset correction (deg C)    */
 0x3F800000 , /*  16 1.0                 11  Tension_a22: 16 Thermistor2 param: Thermistor temp scale correction             */
 0x40A00000 , /*  17 5.0                 11  Tension_a22: 17 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 0 (offset)*/
 0x3F90A3D7 , /*  18 1.13                11  Tension_a22: 18 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 1 (scale)*/
 0x00000000 , /*  19 0.0                 11  Tension_a22: 19 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 2 (x^2)*/
 0x00000000 , /*  20 0.0                 11  Tension_a22: 20 Thermistor1 param: Load-Cell temp compensation polynomial coefficient 3 (x^3)*/
 0x40A00000 , /*  21 5.0                 11  Tension_a22: 21 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 0 (offset)*/
 0x3F90A3D7 , /*  22 1.13                11  Tension_a22: 22 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 1 (scale)*/
 0x00000000 , /*  23 0.0                 11  Tension_a22: 23 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 2 (x^2)*/
 0x00000000 , /*  24 0.0                 11  Tension_a22: 24 Thermistor2 param: Load-Cell temp compensation polynomial coefficient 3 (x^3)*/
 0x00002000 , /*  25 8192                 6  Tension_a22: 25 Heart-Beat: Count of time ticks between autonomous msgs         */
 0x00000002 , /*  26 2                    6  Tension_a22: 26 Drum system number for this function instance                   */
 0x00000001 , /*  27 1                    6  Tension_a22: 27 Drum system poll 2nd payload byte bit for this type of function */
 0x00000000 , /*  28 0                    6  Tension_a22: 28 Drum system poll 1st payload byte bit for drum # (function instance)*/
 0x38600000 , /*  29 CANID_MSG_TENSION_a22  17  Tension_a22: 29 CANID: can msg tension for AD7799 #2                            */
 0x20000000 , /*  30 CANID_MSG_TIME_POLL  17  Tension_a22: 30 CANID: MC: Time msg/Group polling                               */
 0x00400000 , /*  31 CANID_HB_TIMESYNC   17  Tension_a22: 31 CANID: GPS time sync distribution msg                           */
 0xE0E00000 , /*  32 CANID_HB_TENSION_a22  17  Tension_a22: 32 CANID: Heartbeat msg                                            */
 0xF800020C , /*  33 CANID_TST_TENSION_a12  17  Tension_a22: 33 Test                                                            */
 0x00000004 , /*  34 04                   6  Tension_a22: 34 IIR Filter factor: divisor sets time constant: reading for polled msg*/
 0x00000080 , /*  35 128                  6  Tension_a22: 35 Filter scale : upscaling (due to integer math): for polled msg  */
 0x00000200 , /*  36 512                  6  Tension_a22: 36 IIR Filter factor: divisor sets time constant: reading for heart-beat msg*/
 0x00000080 , /*  37 128                  6  Tension_a22: 37 Filter scale : upscaling (due to integer math): for heart-beat  msg*/
 0x00000003 , /*  38 3                    6  Tension_a22: 38 skip or use this function switch                                */
 0x0000000A , /*  39 10                   6  Tension_a22: 39 IIR Filter factor: zero recalibration                           */
 0x00000080 , /*  40 128                  6  Tension_a22: 40 IIR Filter scale : zero recalibration                           */
 0x000001D6 , /*  41 470                  6  Tension_a22: 41 ADC conversion counts between zero recalibrations               */
 0x44960000 , /*  42 1200.0              11  Tension_a22: 42 Exceeding this calibrated limit (+) means invalid reading       */
 0xC42F0000 , /*  43 -700.0              11  Tension_a22: 43 Exceeding this calibrated limit (-) means invalid reading       */
 0x00400000 , /*  44 CANID_HB_TIMESYNC   17  Tension_a22: 44 CANID 1 added to CAN hardware filter to allow incoming msg      */
 0x20000000 , /*  45 CANID_MSG_TIME_POLL  17  Tension_a22: 45 CANID 2 added to CAN hardware filter to allow incoming msg      */
 0xF800040C , /*  46 CANID_TST_TENSION_a22  17  Tension_a22: 46 CANID 3 added to CAN hardware filter to allow incoming msg      */
 0xF800006C , /*  47 CANID_CMD_TENSION_a22I  17  Tension_a22: 47 CANID 4 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  48 CANID_DUMMY         17  Tension_a22: 48 CANID 5 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  49 CANID_DUMMY         17  Tension_a22: 49 CANID 6 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  50 CANID_DUMMY         17  Tension_a22: 50 CANID 7 added to CAN hardware filter to allow incoming msg      */
 0xFFFFFFFC , /*  51 CANID_DUMMY         17  Tension_a22: 51 CANID 8 added to CAN hardware filter to allow incoming msg      */
};
#endif

// =====================================================================
#ifdef YOGURT_1	// Include following parameters?
const uint32_t paramval[] = {
 YOGURT_1_PARAM_SIZE,	/* Number of param entries that follow */
 0x00000000 , /*   1 0                    6  Yogurt_1:  1 CRC for this list                                                  */
 0x00000001 , /*   2 1                    5  Yogurt_1:  2 Version number for Tension List                                    */
 0x45520000 , /*   3 3360.0              11  Yogurt_1:  3 Thermistor1 param: constant B                                      */
 0x41200000 , /*   4 10.0                11  Yogurt_1:  4 Thermistor1 param: Series resistor, fixed (K ohms)                 */
 0x41200000 , /*   5 10.0                11  Yogurt_1:  5 Thermistor1 param: Thermistor room temp resistance (K ohms)        */
 0x43910000 , /*   6 290.0               11  Yogurt_1:  6 Thermistor1 param: Reference temp for thermistor                   */
 0x40C00000 , /*   7 6.0                 11  Yogurt_1:  7 Thermistor1 param: polynomial coeff 0 (offset)                     */
 0x3F800000 , /*   8 1.00                11  Yogurt_1:  8z Thermistor1 param: polynomial coeff 1 (scale)                     */
 0x00000000 , /*   9 0.0                 11  Yogurt_1:  9 Thermistor1 param: polynomial coeff 2 (x^2)                        */
 0x00000000 , /*  10 0.0                 11  Yogurt_1: 10 Thermistor1 param: polynomial coeff 3 (x^3)                        */
 0x4553E000 , /*  11 3390.0              11  Yogurt_1: 11 Thermistor2 param: constant B                                      */
 0x41200000 , /*  12 10.0                11  Yogurt_1: 12 Thermistor2 param: Series resistor, fixed (K ohms)                 */
 0x41200000 , /*  13 10.0                11  Yogurt_1: 13 Thermistor2 param: Thermistor room temp resistance (K ohms)        */
 0x43910000 , /*  14 290.0               11  Yogurt_1: 14 Thermistor2 param: Reference temp for thermistor                   */
 0x40A9999A , /*  15 5.3                 11  Yogurt_1: 15 Thermistor2 param: polynomial coeff 0 (offset)                     */
 0x3F83D70A , /*  16 1.03                11  Yogurt_1: 16 Thermistor2 param: polynomial coeff 1 (scale)                      */
 0x00000000 , /*  17 0.0                 11  Yogurt_1: 17 Thermistor2 param: polynomial coeff 2 (x^2)                        */
 0x00000000 , /*  18 0.0                 11  Yogurt_1: 18 Thermistor2 param: polynomial coeff 3 (x^3)                        */
 0x4550C000 , /*  19 3340.0              11  Yogurt_1: 19 Thermistor3 param: constant B                                      */
 0x41200000 , /*  20 10.0                11  Yogurt_1: 20 Thermistor3 param: Series resistor, fixed (K ohms)                 */
 0x41200000 , /*  21 10.0                11  Yogurt_1: 21 Thermistor3 param: Thermistor room temp resistance (K ohms)        */
 0x43910000 , /*  22 290.0               11  Yogurt_1: 22 Thermistor3 param: Reference temp for thermistor                   */
 0x40B9999A , /*  23 5.8                 11  Yogurt_1: 23 Thermistor3 param: polynomial coeff 0 (offset)                     */
 0x3F80E560 , /*  24 1.007               11  Yogurt_1: 24 Thermistor3 param: polynomial coeff 1 (scale)                      */
 0x00000000 , /*  25 0.0                 11  Yogurt_1: 25 Thermistor3 param: polynomial coeff 2 (x^2)                        */
 0x00000000 , /*  26 0.0                 11  Yogurt_1: 26 Thermistor3 param: polynomial coeff 3 (x^3)                        */
 0x4550C000 , /*  27 3340.0              11  Yogurt_1: 27 Thermistor4 param: constant B                                      */
 0x41200000 , /*  28 10.0                11  Yogurt_1: 28 Thermistor4 param: Series resistor, fixed (K ohms)                 */
 0x41200000 , /*  29 10.0                11  Yogurt_1: 29 Thermistor4 param: Thermistor room temp resistance (K ohms)        */
 0x43910000 , /*  30 290.0               11  Yogurt_1: 30 Thermistor4 param: Reference temp for thermistor                   */
 0x40A66666 , /*  31 5.2                 11  Yogurt_1: 31 Thermistor4 param: polynomial coeff 0 (offset)                     */
 0x3F851EB8 , /*  32 1.04                11  Yogurt_1: 32 Thermistor4 param: polynomial coeff 1 (scale)                      */
 0x00000000 , /*  33 0.0                 11  Yogurt_1: 33 Thermistor4 param: polynomial coeff 2 (x^2)                        */
 0x00000000 , /*  34 0.0                 11  Yogurt_1: 34 Thermistor4 param: polynomial coeff 3 (x^3)                        */
 0x43200000 , /*  35 160.0               11  Yogurt_1: 35 Pasteur: Control set-point temperature (deg F) heat to this temp   */
 0x3F000000 , /*  36 0.5                 11  Yogurt_1: 36 Pasteur: Time duration at temp (hours.frac_hours)                  */
 0x42DC0000 , /*  37 110.0               11  Yogurt_1: 37 Pasteur: Control end-point temperature (deg F) cool to this temp   */
 0x42DC0000 , /*  38 110.0               11  Yogurt_1: 38 Ferment: Control set-point temperature (deg F) heat to this temp   */
 0x41500000 , /*  39 13.0                11  Yogurt_1: 39 Ferment: Time duration at temp (hours.frac_hours)                  */
 0x42340000 , /*  40 45.0                11  Yogurt_1: 40 Ferment: Control end-point temperature (deg F) cool to this temp   */
 0x00000003 , /*  41 3                    6  Yogurt_1: 41 Thermistor number for shell temp (0 - 3)                           */
 0x00000002 , /*  42 2                    6  Yogurt_1: 42 Thermistor number for center of pot temp (0 - 3)                   */
 0x00000000 , /*  43 0                    6  Yogurt_1: 43 Thermistor number for air inlet to fan temp (0 - 3)                */
 0x00000001 , /*  44 1                    6  Yogurt_1: 44 Thermistor number for air coming out of holes (0 - 3)              */
 0x463B8000 , /*  45 12000.0             11  Yogurt_1: 45 Control loop: Proportional coefficient                             */
 0x40C00000 , /*  46 6.0                 11  Yogurt_1: 46 Control loop: Integral coefficient                                 */
 0x486A6000 , /*  47 240.0E3             11  Yogurt_1: 47 Control loop: Derivative coefficient                               */
 0x## CAN ID NOT FOUND ## , /*  48 CANID_CMD_YOGURT_1  17  Yogurt_1: 48 CANID: cid_yog_cmd: Yogurt maker parameters                        */
 0x29400000 , /*  49 CANID_MSG_YOGURT_1  17  Yogurt_1: 49 CANID: cid_yog_msg: Yogurt maker msgs                              */
 0x29600000 , /*  50 CANID_HB_YOGURT_1   17  Yogurt_1: 50 CANID: cid_yog_hb: Yogurt maker heart-beats                        */
 0x3C23D70A , /*  51 0.0100              11  Yogurt_1: 51  Control, stored heat constant Pasteur phase                       */
 0x3CA3D70A , /*  52 0.0200              11  Yogurt_1: 52  Control, stored heat constant Ferment phase                       */
 0xC57A0000 , /*  53 -4000.0             11  Yogurt_1: 53  Control, integrator initialization, a of  a + b*x                 */
 0x43D90000 , /*  54 434.0               11  Yogurt_1: 54  Control, integrator initialization, b of  a + b*x                 */
 0x000000C8 , /*  55 200                  6  Yogurt_1: 55  Control, time delay for temperature stabilization, Pasteur        */
 0x0000042E , /*  56 1070                 6  Yogurt_1: 56  Control, time delay for temperature stabilization, Ferment        */
};
#endif

// ================= COMMAND CANID TABLES ========================

#define CAN_UNIT_11_CMDID_TABLE_SIZE 2	// Tension_a2: R 2 AD7799 VE POD Test (hence X) 1

#define CAN_UNIT_12_CMDID_TABLE_SIZE 4	// Yogurt_1: R Ver 1 of maker

#define CAN_UNIT_13_CMDID_TABLE_SIZE 2	// Tension_a:  R 1 AD7799 VE POD Test (hence X) 2

#define CAN_UNIT_14_CMDID_TABLE_SIZE 2	// Tension_a2: R 2 AD7799 VE POD Test (hence X) 3

#define CAN_UNIT_15_CMDID_TABLE_SIZE 4	// Tension_a2: R 2 AD7799 VE POD GSM 1

#define CAN_UNIT_16_CMDID_TABLE_SIZE 4	// Logger_2: R 

#define CAN_UNIT_1A_CMDID_TABLE_SIZE 4	// Cansender_1: R Sensor board test

#define CAN_UNIT_1B_CMDID_TABLE_SIZE 2	// Sensor, engine: R rpm, manifold pressure, throttle setting, temperature

#define CAN_UNIT_3_CMDID_TABLE_SIZE 2	// Logger_1: R 

#define CAN_UNIT_E_CMDID_TABLE_SIZE 4	// Cable angle: R AD7799 #2 drum #1

#define CAN_UNIT_F_CMDID_TABLE_SIZE 4


// =====================================================================
#ifdef CAN_UNIT_11	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x02200000, CAN_UNIT_11_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1003,  0x## CAN ID NOT FOUND ## }, /*   1 CAN_UNIT_11          CANID_CMD_TENSION_0R	 Tension_a: R 1 AD7799 VE POD Test (hence X) 0*/
{   3,  0x## CAN ID NOT FOUND ## }, /*   2 CAN_UNIT_11          CANID_CMD_TENSION_0I	 Tension_a: I 1 AD7799 VE POD Test (hence X) 0*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_12	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x02400000, CAN_UNIT_12_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1004,  0x05E0000C }, /*   1 CAN_UNIT_12          CANID_CMD_TENSION_a21R	 Tension_a2: R 2 AD7799 VE POD Test (hence X) 1*/
{   4,  0x05E00004 }, /*   2 CAN_UNIT_12          CANID_CMD_TENSION_a21I	 Tension_a2: I 2 AD7799 VE POD Test (hence X) 1*/
{1003,  0x05C0000C }, /*   3 CAN_UNIT_12          CANID_CMD_TENSION_a11R	 Tension_a: R  2 AD7799 VE POD Test (hence X) 1*/
{   3,  0x05C00004 }, /*   4 CAN_UNIT_12          CANID_CMD_TENSION_a11I	 Tension_a: I  2 AD7799 VE POD Test (hence X) 1*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_13	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x02600000, CAN_UNIT_13_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1011,  0x29800004 }, /*   1 CAN_UNIT_13          CANID_CMD_YOGURT_1R	 Yogurt_1: R Ver 1 of maker    */
{  11,  0x29800000 }, /*   2 CAN_UNIT_13          CANID_CMD_YOGURT_1I	 Yogurt_1: I Ver 1 of maker    */
};
#endif

// =====================================================================
#ifdef CAN_UNIT_14	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x02E00000, CAN_UNIT_14_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1003,  0xF800804C }, /*   1 CAN_UNIT_14          CANID_CMD_TENSION_a1YR	 Tension_a:  R 1 AD7799 VE POD Test (hence X) 2*/
{   3,  0xF800004C }, /*   2 CAN_UNIT_14          CANID_CMD_TENSION_a1YI	 Tension_a:  I 1 AD7799 VE POD Test (hence X) 2*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_15	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x02A00000, CAN_UNIT_15_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1004,  0xF800806C }, /*   1 CAN_UNIT_15          CANID_CMD_TENSION_a22R	 Tension_a2: R 2 AD7799 VE POD Test (hence X) 3*/
{   4,  0xF800006C }, /*   2 CAN_UNIT_15          CANID_CMD_TENSION_a22I	 Tension_a2: I 2 AD7799 VE POD Test (hence X) 3*/
{1003,  0xF800805C }, /*   3 CAN_UNIT_15          CANID_CMD_TENSION_a12R	 Tension_a:  R 2 AD7799 VE POD Test (hence X) 3*/
{   3,  0xF800005C }, /*   4 CAN_UNIT_15          CANID_CMD_TENSION_a12I	 Tension_a:  I 2 AD7799 VE POD Test (hence X) 3*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_16	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x02C00000, CAN_UNIT_16_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1004,  0xF800808C }, /*   1 CAN_UNIT_16          CANID_CMD_TENSION_a2GR	 Tension_a2: R 2 AD7799 VE POD GSM 1*/
{   4,  0xF800008C }, /*   2 CAN_UNIT_16          CANID_CMD_TENSION_a2GI	 Tension_a2: I 2 AD7799 VE POD GSM 1*/
{1003,  0xF800807C }, /*   3 CAN_UNIT_16          CANID_CMD_TENSION_a1GR	 Tension_a:  R 2 AD7799 VE POD GSM 1*/
{   3,  0xF800007C }, /*   4 CAN_UNIT_16          CANID_CMD_TENSION_a1GI	 Tension_a:  I 2 AD7799 VE POD GSM 1*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_1A	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x03600000, CAN_UNIT_1A_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1013,  0xD1C0006C }, /*   1 CAN_UNIT_1A          CANID_CMD_LOGGER_2R	 Logger_2: R                   */
{  13,  0xD1C00064 }, /*   2 CAN_UNIT_1A          CANID_CMD_LOGGER_2I	 Logger_2: I                   */
{1012,  0xD1C0007C }, /*   3 CAN_UNIT_1A          CANID_CMD_GPS_2R	 GPS_2: R time sync distribution msg */
{  12,  0xD1C00074 }, /*   4 CAN_UNIT_1A          CANID_CMD_GPS_2I	 GPS_2: I  time sync distribution msg */
};
#endif

// =====================================================================
#ifdef CAN_UNIT_1B	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x03E00000, CAN_UNIT_1B_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1014,  0xA0200004 }, /*   1 CAN_UNIT_1B          CANID_CMD_CANSENDER_1R	 Cansender_1: R Sensor board test*/
{  14,  0xA0200000 }, /*   2 CAN_UNIT_1B          CANID_CMD_CANSENDER_1I	 Cansender_1: I Sensor board test*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_3	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x03800000, CAN_UNIT_3_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1002,  0x8060000C }, /*   1 CAN_UNIT_3           CANID_CMD_ENGINE_SENSORR	 Sensor, engine: R rpm, manifold pressure, throttle setting, temperature*/
{   2,  0x80600000 }, /*   2 CAN_UNIT_3           CANID_CMD_ENGINE_SENSORI	 Sensor, engine: I rpm, manifold pressure, throttle setting, temperature*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_E	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x01C0000C, CAN_UNIT_E_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1013,  0xD1C0005C }, /*   1 CAN_UNIT_E           CANID_CMD_LOGGER_1R	 Logger_1: R                   */
{  13,  0xD1C00054 }, /*   2 CAN_UNIT_E           CANID_CMD_LOGGER_1I	 Logger_1: I                   */
{1012,  0xD1C0004C }, /*   3 CAN_UNIT_E           CANID_CMD_GPS_1R	 GPS_1: R time sync distribution msg*/
{  12,  0xD1C00044 }, /*   4 CAN_UNIT_E           CANID_CMD_GPS_1I	 GPS_1: I time sync distribution msg*/
};
#endif

// =====================================================================
#ifdef CAN_UNIT_F	// a #define is used to select the following
const struct FUNC_CANID func_canid[] = {
{   0x01E0000C, CAN_UNIT_F_CMDID_TABLE_SIZE },	/* {Unit CAN ID, Number of CAN IDs that follow} */
{1005,  0x0620000C }, /*   1 CAN_UNIT_F           CANID_CMD_CABLE_ANGLE_1R	 Cable angle: R AD7799 #2 drum #1*/
{   5,  0x06200000 }, /*   2 CAN_UNIT_F           CANID_CMD_CABLE_ANGLE_1I	 Cable angle: I AD7799 #2 drum #1*/
{1003,  0x05C0803C }, /*   3 CAN_UNIT_F           CANID_CMD_TENSION_a1WR	 Tension_a: R Tension drum #1 AD7799 #1*/
{   3,  0x05C0003C }, /*   4 CAN_UNIT_F           CANID_CMD_TENSION_a1WI	 Tension_a: I Tension drum #1 AD7799 #1*/
};
#endif
