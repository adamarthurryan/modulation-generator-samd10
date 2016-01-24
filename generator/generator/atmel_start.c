/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "atmel_start.h"
#include <utils.h>
#include <hal_init.h>
#include <hpl_irq.h>
#include <hpl_pm1_v201_base.h>
#include <hpl_gclk1_v210_base.h>
#include <peripheral_gclk_config.h>
#if CONF_DMAC_MAX_USED_DESC > 0
#endif

extern struct _irq_descriptor *_irq_table[PERIPH_COUNT_IRQn];
extern void Default_Handler(void);

struct timer_descriptor TIMER_0;

static struct timer_task TIMER_0_task1, TIMER_0_task2;

struct usart_sync_descriptor USART_0;

struct pwm_descriptor PWM_0;

struct dac_sync_descriptor DAC_0;

struct flash_descriptor FLASH_0;

void USART_0_PORT_init(void)
{
	gpio_set_pin_function(PA10, PINMUX_PA10C_SERCOM0_PAD2);

	gpio_set_pin_function(PA11, PINMUX_PA11C_SERCOM0_PAD3);
}

void USART_0_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
	_gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
	_gclk_enable_channel(SERCOM0_GCLK_ID_SLOW, CONF_GCLK_SERCOM0_SLOW_SRC);
}

void USART_0_init(void)
{
	USART_0_CLOCK_init();
	usart_sync_init(&USART_0, SERCOM0);
	USART_0_PORT_init();
}

/**
 * Example of using USART_0 to write "Hello World" using the IO abstraction.
 */
void USART_0_example(void)
{
	struct io_descriptor *io;
	usart_sync_get_io_descriptor(&USART_0, &io);
	usart_sync_enable(&USART_0);

	io_write(io, (uint8_t *)"Hello World!", 12);
}

/**
 * \brief Timer initialization function
 *
 * Enables Timer peripheral, clocks and initializes Timer driver
 */
static void TIMER_0_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, TC2);
	_gclk_enable_channel(TC2_GCLK_ID, CONF_GCLK_TC2_SRC);
	timer_init(&TIMER_0, TC2, _tc_get_timer());
}

void PWM_0_PORT_init(void)
{
	// Set pin direction to output
	gpio_set_pin_direction(PA14, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(PA14,
	        // <y> Initial level
	        // <id> pad_initial_level
	        // <false"> Low
	        // <true"> High
			false);

	gpio_set_pin_function(PA14, PINMUX_PA14F_TCC0_WO0);

	// Set pin direction to output
	gpio_set_pin_direction(PA15, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(PA15,
	        // <y> Initial level
	        // <id> pad_initial_level
	        // <false"> Low
	        // <true"> High
			false);

	gpio_set_pin_function(PA15, PINMUX_PA15F_TCC0_WO1);

	// Set pin direction to output
	gpio_set_pin_direction(PA09, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(PA09,
	        // <y> Initial level
	        // <id> pad_initial_level
	        // <false"> Low
	        // <true"> High
			false);

	gpio_set_pin_function(PA09, PINMUX_PA09E_TCC0_WO3);
}

void PWM_0_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, TCC0);
	_gclk_enable_channel(TCC0_GCLK_ID, CONF_GCLK_TCC0_SRC);
}

void PWM_0_init(void)
{
	PWM_0_CLOCK_init();
	PWM_0_PORT_init();
	pwm_init(&PWM_0, TCC0, _tcc_get_pwm());
}

/**
 * Example of using PWM_0.
 */
void PWM_0_example(void)
{
	pwm_set_parameters(&PWM_0, 10000, 5000);
	pwm_enable(&PWM_0);
}

void EXTERNAL_IRQ_0_init(void)
{
	_gclk_enable_channel(EIC_GCLK_ID, CONF_GCLK_EIC_SRC);

	ext_irq_init();

	// Set pin direction to input
	gpio_set_pin_direction(PA25, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(PA25,
	        // <y> Pull configuration
	        // <id> pad_pull_config
	        // <GPIO_PULL_OFF"> Off
	        // <GPIO_PULL_UP"> Pull-up
	        // <GPIO_PULL_DOWN"> Pull-down
			GPIO_PULL_OFF);

	gpio_set_pin_function(PA25, PINMUX_PA25A_EIC_EXTINT5);
}

void DAC_0_PORT_init(void)
{
	// Disable digital pin circuitry
	gpio_set_pin_direction(PA02, GPIO_DIRECTION_OFF);
	gpio_set_pin_function(PA02, PINMUX_PA02B_DAC_VOUT);

	// Disable digital pin circuitry
	gpio_set_pin_direction(PA03, GPIO_DIRECTION_OFF);
	gpio_set_pin_function(PA03, PINMUX_PA03B_DAC_VREFP);
}

void DAC_0_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, DAC);
	_gclk_enable_channel(DAC_GCLK_ID, CONF_GCLK_DAC_SRC);
}

void DAC_0_init(void)
{
	DAC_0_CLOCK_init();
	dac_sync_init(&DAC_0, DAC);
	DAC_0_PORT_init();
}

/**
 * Example of using DAC_0 to generate waveform.
 */
void DAC_0_example(void)
{
	uint16_t i = 0;

	dac_sync_enable_channel(&DAC_0, 0);

	for(;; ) {
		dac_sync_write(&DAC_0, 0, &i, 1);
		i = ( i+1 ) % 1024;
	}
}

void FLASH_0_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBB, NVMCTRL);
}

void FLASH_0_init(void)
{
	FLASH_0_CLOCK_init();
	flash_init(&FLASH_0, NVMCTRL);
}

static uint8_t src_data[128];
static uint8_t chk_data[128];

/**
 * Example of using FLASH_0 to read and write buffer.
 */
void FLASH_0_example(void)
{
	uint32_t page_size;
	uint16_t i;

	/* Init source data */
	page_size = flash_get_page_size(&FLASH_0);

	for(i = 0; i < page_size; i++) {
		src_data[i] = i;
	}

	/* Write data to flash */
	flash_write(&FLASH_0, 0x3200, src_data, page_size);

	/* Read data from flash */
	flash_read(&FLASH_0, 0x3200, chk_data, page_size);
}

void SERCOM0_Handler(void)
{
	if (_irq_table[ SERCOM0_IRQn + 0 ]) {
		_irq_table[ SERCOM0_IRQn + 0 ]->handler(
				_irq_table[ SERCOM0_IRQn + 0 ]->parameter);
	} else {
		Default_Handler();
	}
}

void TC2_Handler(void)
{
	if (_irq_table[ TCC0_IRQn + 2 ]) {
		_irq_table[ TCC0_IRQn + 2 ]->handler(
				_irq_table[ TCC0_IRQn + 2 ]->parameter);
	} else {
		Default_Handler();
	}
}

void TCC0_Handler(void)
{
	if (_irq_table[ TCC0_IRQn + 0 ]) {
		_irq_table[ TCC0_IRQn + 0 ]->handler(
				_irq_table[ TCC0_IRQn + 0 ]->parameter);
	} else {
		Default_Handler();
	}
}

void EIC_Handler(void)
{
	if (_irq_table[ EIC_IRQn + 0 ]) {
		_irq_table[ EIC_IRQn + 0 ]->handler(
				_irq_table[ EIC_IRQn + 0 ]->parameter);
	} else {
		Default_Handler();
	}
}

void DAC_Handler(void)
{
	if (_irq_table[ DAC_IRQn + 0 ]) {
		_irq_table[ DAC_IRQn + 0 ]->handler(
				_irq_table[ DAC_IRQn + 0 ]->parameter);
	} else {
		Default_Handler();
	}
}

void SYSCTRL_Handler(void)
{
	if (_irq_table[ SYSCTRL_IRQn + 0 ]) {
		_irq_table[ SYSCTRL_IRQn + 0 ]->handler(
				_irq_table[ SYSCTRL_IRQn + 0 ]->parameter);
	} else {
		Default_Handler();
	}
}

void PM_Handler(void)
{
	if (_irq_table[ PM_IRQn + 0 ]) {
		_irq_table[ PM_IRQn + 0 ]->handler(_irq_table[ PM_IRQn + 0 ]->parameter);
	} else {
		Default_Handler();
	}
}

void NVMCTRL_Handler(void)
{
	if (_irq_table[ NVMCTRL_IRQn + 0 ]) {
		_irq_table[ NVMCTRL_IRQn + 0 ]->handler(
				_irq_table[ NVMCTRL_IRQn + 0 ]->parameter);
	} else {
		Default_Handler();
	}
}

/**
 * Example of using TIMER_0.
 */
static void TIMER_0_task1_cb(const struct timer_task *const timer_task)
{
}

static void TIMER_0_task2_cb(const struct timer_task *const timer_task)
{
}

void TIMER_0_example(void)
{
	TIMER_0_task1.interval = 100;
	TIMER_0_task1.cb = TIMER_0_task1_cb;
	TIMER_0_task1.mode = TIMER_TASK_REPEAT;
	TIMER_0_task2.interval = 200;
	TIMER_0_task2.cb = TIMER_0_task2_cb;
	TIMER_0_task2.mode = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_0, &TIMER_0_task1);
	timer_add_task(&TIMER_0, &TIMER_0_task2);
	timer_start(&TIMER_0);
}

static void button_on_PA25_pressed(void)
{
}

/**
 * Example of using EXTERNAL_IRQ_0
 */
void EXTERNAL_IRQ_0_example(void)
{
	ext_irq_register(PIN_PA25, button_on_PA25_pressed);
}

void system_init(void)
{
	init_mcu();

	USART_0_init();
	TIMER_0_init();

	PWM_0_init();
	EXTERNAL_IRQ_0_init();

	DAC_0_init();

	FLASH_0_init();
}
