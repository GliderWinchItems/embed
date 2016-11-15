

#ifndef LIBOPENCM3_SDADC_H
#define LIBOPENCM3_SDADC_H

#define SDADC1		SDADC1_BASE
#define SDADC2		SDADC2_BASE
#define SDADC3		SDADC3_BASE

/* Master and slave ADCs common registers (ADC12 or ADC34) */


/*----------- SDADC registers -------------------------------------- */

/* Control Register (ADCx_CR1, x=1..3) CR1 */
#define SDADC_CR1(sdadc_base)	MMIO32((sdadc_base) + 0x00)
#define SDADC1_CR1		SDADC_CR1(SDADC1_BASE)
#define SDADC2_CR1		SDADC_CR1(SDADC2_BASE)
#define SDADC3_CR1		SDADC_CR1(SDADC3_BASE)

/* Control Register (ADCx_CR2, x=1..3) CR2 */
#define SDADC_CR2(sdadc_base)	MMIO32((sdadc_base) + 0x04)
#define SDADC1_CR2		SDADC_CR2(SDADC1_BASE)
#define SDADC2_CR2		SDADC_CR2(SDADC2_BASE)
#define SDADC3_CR2		SDADC_CR2(SDADC3_BASE)

/* SDADC interrupt and status register (SDADCx_ISR, x=1..3) */
#define SDADC_ISR(sdadc_base)	MMIO32((sdadc_base) + 0x08)
#define SDADC1_ISR		SDADC_ISR(SDADC1_BASE)
#define SDADC2_ISR		SDADC_ISR(SDADC2_BASE)
#define SDADC3_ISR		SDADC_ISR(SDADC3_BASE)

/* SDADC interrupt and status clear register (SDADCx_CLRISR, x=1..3) */
#define SDADC_CLRISR(sdadc_base)	MMIO32((sdadc_base) + 0x0C)
#define SDADC1_CLRISR		SDADC_CLRISR(SDADC1_BASE)
#define SDADC2_CLRISR		SDADC_CLRISR(SDADC2_BASE)
#define SDADC3_CLRISR		SDADC_CLRISR(SDADC3_BASE)

/* SDADC injected channel group selection register (SDADC_JCHGR), x=1..3) */
#define SDADC_JCHGR(sdadc_base)	MMIO32((sdadc_base) + 0x014)
#define SDADC1_JCHGR		SDADC_JCHGR(SDADC1_BASE)
#define SDADC2_JCHGR		SDADC_JCHGR(SDADC2_BASE)
#define SDADC3_JCHGR		SDADC_JCHGR(SDADC3_BASE)

/* SDADC configuration 0 register (SDADC_CONF0R), x=1..3) */
#define SDADC_CONF0R(sdadc_base)	MMIO32((sdadc_base) + 0x020)
#define SDADC1_CONF0R		SDADC_CONF0R(SDADC1_BASE)
#define SDADC2_CONF0R		SDADC_CONF0R(SDADC2_BASE)
#define SDADC3_CONF0R		SDADC_CONF0R(SDADC3_BASE)

/* SDADC configuration 1 register (SDADC_CONF1R), x=1..3) */
#define SDADC_CONF1R(sdadc_base)	MMIO32((sdadc_base) + 0x024)
#define SDADC1_CONF1R		SDADC_CONF1R(SDADC1_BASE)
#define SDADC2_CONF1R		SDADC_CONF1R(SDADC2_BASE)
#define SDADC3_CONF1R		SDADC_CONF1R(SDADC3_BASE)

/* SDADC configuration 2 register (SDADC_CONF2R), x=1..3) */
#define SDADC_CONF2R(sdadc_base)	MMIO32((sdadc_base) + 0x028)
#define SDADC1_CONF2R		SDADC_CONF2R(SDADC1_BASE)
#define SDADC2_CONF2R		SDADC_CONF2R(SDADC2_BASE)
#define SDADC3_CONF2R		SDADC_CONF2R(SDADC3_BASE)

/* SDADC channel configuration register 1 (SDADC_CONFCHR1), x=1..3) */
#define SDADC_CONFCHR1(sdadc_base)	MMIO32((sdadc_base) + 0x040)
#define SDADC1_CONFCHR1		SDADC_CONFCHR1(SDADC1_BASE)
#define SDADC2_CONFCHR1		SDADC_CONFCHR1(SDADC2_BASE)
#define SDADC3_CONFCHR1		SDADC_CONFCHR1(SDADC3_BASE)

/* SDADC channel configuration register 2 (SDADC_CONFCHR2), x=1..3) */
#define SDADC_CONFCHR2(sdadc_base)	MMIO32((sdadc_base) + 0x044)
#define SDADC1_CONFCHR2		SDADC_CONFCHR2(SDADC1_BASE)
#define SDADC2_CONFCHR2		SDADC_CONFCHR2(SDADC2_BASE)
#define SDADC3_CONFCHR2		SDADC_CONFCHR2(SDADC3_BASE)

/* SDADC data register for injected group (SDADC_JDATAR), x=1..3) */
#define SDADC_JDATAR(sdadc_base)	MMIO32((sdadc_base) + 0x060)
#define SDADC1_JDATAR		SDADC_JDATAR(SDADC1_BASE)
#define SDADC2_JDATAR		SDADC_JDATAR(SDADC2_BASE)
#define SDADC3_JDATAR		SDADC_JDATAR(SDADC3_BASE)

/* SDADC data register for the regular channel (SDADC_RDATAR), x=1..3) */
#define SDADC_RDATAR(sdadc_base)	MMIO32((sdadc_base) + 0x064)
#define SDADC1_RDATAR		SDADC_RDATAR(SDADC1_BASE)
#define SDADC2_RDATAR		SDADC_RDATAR(SDADC2_BASE)
#define SDADC3_RDATAR		SDADC_RDATAR(SDADC3_BASE)

/* SDADC1 and SDADC2 injected data register (SDADC_JDATA12R), x=1..3) */
#define SDADC_JDATA12R(sdadc_base)	MMIO32((sdadc_base) + 0x70)
#define SDADC1_JDATA12R		SDADC_JDATA12R(SDADC1_BASE)
#define SDADC2_JDATA12R		SDADC_JDATA12R(SDADC2_BASE)
#define SDADC3_JDATA12R		SDADC_JDATA12R(SDADC3_BASE)

/* SDADC1 and SDADC2 regular data register (SDADC_RDATA12R), x=1..3) */
#define SDADC_RDATA12R(sdadc_base)	MMIO32((sdadc_base) + 0x74)
#define SDADC1_RDATA12R		SDADC_RDATA12R(SDADC1_BASE)
#define SDADC2_RDATA12R		SDADC_RDATA12R(SDADC2_BASE)
#define SDADC3_RDATA12R		SDADC_RDATA12R(SDADC3_BASE)

/* SDADC1 and SDADC3 injected data register (SDADC_JDATA13R), x=1..3) */
#define SDADC_JDATA13R(sdadc_base)	MMIO32((sdadc_base) + 0x78)
#define SDADC1_JDATA13R		SDADC_JDATA13R(SDADC1_BASE)
#define SDADC2_JDATA13R		SDADC_JDATA13R(SDADC2_BASE)
#define SDADC3_JDATA13R		SDADC_JDATA13R(SDADC3_BASE)

/* SDADC1 and SDADC3 regular data register (SDADC_RDATA13R), x=1..3) */
#define SDADC_RDATA13R(sdadc_base)	MMIO32((sdadc_base) + 0x7C)
#define SDADC1_RDATA13R		SDADC_RDATA13R(SDADC1_BASE)
#define SDADC2_RDATA13R		SDADC_RDATA13R(SDADC2_BASE)
#define SDADC3_RDATA13R		SDADC_RDATA13R(SDADC3_BASE)



#endif
