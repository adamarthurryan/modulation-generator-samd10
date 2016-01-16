/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef ATMEL_START_H_INCLUDED
#define ATMEL_START_H_INCLUDED

#include "atmel_start_pins.h"

#ifdef __cplusplus
		extern "C" {
#endif

#include <hal_usart_sync.h>
#include <hal_timer.h>
#include <hpl_tc1_v112_base.h>

#include <hal_pwm.h>
#include <hpl_tcc_v101_base.h>
#include <hal_ext_irq.h>

#include <hal_dac_sync.h>

	extern struct usart_sync_descriptor USART_0;
	extern struct timer_descriptor TIMER_0;

	extern struct pwm_descriptor PWM_0;

	extern struct dac_sync_descriptor DAC_0;

	void USART_0_PORT_init(void);
	void USART_0_CLOCK_init(void);
	void USART_0_init(void);
	void USART_0_example(void);

	void PWM_0_PORT_init(void);
	void PWM_0_CLOCK_init(void);
	void PWM_0_init(void);
	void PWM_0_example(void);

	void DAC_0_PORT_init(void);
	void DAC_0_CLOCK_init(void);
	void DAC_0_init(void);
	void DAC_0_example(void);

#define CONF_DMAC_MAX_USED_DESC ( /*SERCOM0*/ 0 + /*TC2*/ 0 + /*TCC0*/ 0 + \
	        /*EIC*/ 0 + /*DAC*/ 0 + /*GCLK*/ 0 + /*SYSCTRL*/ 0 + /*PM*/ 0 )

#define CONF_DMAC_MAX_USED_CH ( /*SERCOM0*/ 0 + /*TC2*/ 0 + /*TCC0*/ 0 + \
	        /*EIC*/ 0 + /*DAC*/ 0 + /*GCLK*/ 0 + /*SYSCTRL*/ 0 + /*PM*/ 0 )

	/**
	 * \brief Perform system initialization, initialize pins and clocks for
	 * peripherals
	 */
	void system_init(void);

#ifdef __cplusplus
		}
#endif
#endif // ATMEL_START_H_INCLUDED
