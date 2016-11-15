/* comp.h - COMP registers & bits
10/06/2012 deh

#ifndef LIBOPENCM3_COMP_H
#define LIBOPENCM3_COMP_H

#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/common.h>

/* COMP register base simplification */
#define COMP1				COMP1_BASE	
#define COMP2				COMP2_BASE

/* --- COMP register p 279 ------------------------------------------------- */

/* COMP control and status register (COMP_CSR) p 279 */
#define DAC1_CR				MMIO32(DAC1_BASE + 0x1C)	
#define DAC2_CR				MMIO32(DAC2_BASE + 0x1C)

/* --- Bit fields ---------------------------------------------------------- */
#define COMP2LOCK	(1 << 31)	// Comparator 2 lock
/* This bit is write-once. It is set by software. It can only be cleared by a system reset.
It allows to have all control bits of comparator 2 as read-only.
0: COMP_CSR[31:16] bits are read-write.
1: COMP_CSR[31:16] bits are read-only. */

#define COMP2OUT	(1 << 30)	// Comparator 2 output
/*           This read-only bit is a copy of comparator 2 output state.
             0: Output is low (non-inverting input below inverting input).
             1: Output is high (non-inverting input above inverting input). */

#define COMP2HYST	(1 << 28)	// Comparator 2 hysteresis
/*             These bits control the hysteresis level.
             00: No hysteresis
             01: Low hysteresis
             10: Medium hysteresis
             11: High hysteresis
             Please refer to the electrical characteristics for the hysteresis values. */
#define COMP2POL	(1 << 27)	// Comparator 2 output polarity
/*             This bit is used to invert the comparator 2 output.
             0: Output is not inverted
             1: Output is inverted */

#define COMP2OUTSEL	(1 << 24)	// Comparator 2 output selection
/*             These bits select the destination of the comparator output.
             000: No selection
             001: Timer 16 break input
             010: Timer 4 Input capture 1
             011: Timer 4 OCrefclear input
             100: Timer 2 input capture 4
             101: Timer 2 OCrefclear input
             110: Timer 3 input capture 1
             111: Timer 3 OCrefclear input */

#define WNDWEN		(1 << 23)	// Window mode enable
/*        This bit connects the non-inverting input of COMP2 to COMP1’s non-inverting input,
        which is simultaneously disconnected from PA3.
        0: Window mode disabled
        1: Window mode enabled	*/

#define COMP2INSEL	(1 << 20)	// Comparator 2 inverting input selection
/*            These bits allows to select the source connected to the inverting input of the
            comparator 2.
            000: 1/4 of Vrefint
            001: 1/2 of Vrefint
            010: 3/4 of Vrefint
            011: Vrefint
            100: DAC1_OUT1 output (and PA4 output)
            101: Input from PA5 (and DAC1_OUT2 output)
            110: Input from PA2
            111: Input from PA6 (and DAC2_OUT1 output)	*/

#define COMP2MODE	(1 << 18)	// Comparator 2 mode
/*             These bits control the operating mode of the comparator2 and allows to adjust the
             speed/consumption.
             00: High speed/
             01: Medium speed
             10: Low power
             11: Ultra-low power	*/

/*     Bit 17 Reserved, must be kept at reset value.	*/

#define COMP2EN		(1 << 16)	// Comparator 2 enable
/*             This bit switches ON/OFF the comparator2.
             0: Comparator 2 disabled
             1: Comparator 2 enabled	*/

#define COMP1LOCK	(1 << 15)	// Comparator 1 lock
/*             This bit is write-once. It is set by software. It can only be cleared by a system reset.
             It allows to have all control bits of comparator 1 as read-only.
             0: COMP_CSR[15:0] bits are read-write.
             1: COMP_CSR[15:0] bits are read-only.	*/

#define COMP1OUT	(1 << 14)	// Comparator 1 output
/*             This read-only bit is a copy of comparator 1 output state.
             0: Output is low (non-inverting input below inverting input).
             1: Output is high (non-inverting input above inverting input).	*/

#define COMP1HYST	(1 << 12)	// Comparator 1 hysteresis
/*             These bits are controlling the hysteresis level.
             00: No hysteresis
             01: Low hysteresis
             10: Medium hysteresis
             11: High hysteresis
             Please refer to the electrical characteristics for the hysteresis values. */

#define COMP1POL	(1 << 11)	// Comparator 1 output polarity
/*             This bit is used to invert the comparator 1 output.
             0: output is not inverted
             1: output is inverted	*/

#define COMP1OUTSEL		(1 << 8)	// Comparator 1 output selection
/*            These bits selects the destination of the comparator 1 output.
            000: no selection
            001: Timer 15 break input
            010: Timer 3 Input capture 1
            011: Timer 3 OCrefclear input
            100: Timer 2 input capture 4
            101: Timer 2 OCrefclear input
            110: Timer 5 input capture 4
            111: Timer 5 OCrefclear input		*/

/*     Bit 7 Reserved, must be kept at reset value.	*/

#define COMP1INSEL		(1 << 4)	// Comparator 1 inverting input selection
/*            These bits select the source connected to the inverting input of the comparator 1.
            000: 1/4 of Vrefint
            001: 1/2 of Vrefint
            010: 3/4 of Vrefint
            011: Vrefint
            100: DAC1_OUT1output (and PA4)
            101: Input from PA5 (and DAC1_OUT2 output)
            110: Input from PA0
            111: Input from PA6 (and DAC2_OUT1 output)	*/

#define COMP1MODE		(1 << 2)	// Comparator 1 mode
/*            These bits control the operating mode of the comparator1 and allows to adjust the
            speed/consumption.
            00: Ultra-low power
            01: Low power
            10: Medium speed
            11: High speed	*/

#define COMP1_INP_DAC		(1 << 1)	// Comparator 1 enable
/*            This bit closes a switch between comparator 1 non-inverting input on PA0 and PA4
            (DAC) I/O.
            0: Switch open
            1: Switch closed
           Note: This switch is solely intended to redirect signals onto high impedance input, such
                 as COMP1 non-inverting input (highly resistive switch).	*/
#define COMP1EN			(1 << 0)	// Comparator 1 enable
/*            This bit switches COMP1 ON/OFF.
            0: Comparator 1 disabled
            1: Comparator 1 enabled	*/






#endif

