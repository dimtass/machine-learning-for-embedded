/**
 * This Example is using the mod_led.h in noarch_c_lib
 * to toggle the LED. The difference with the simple
 * example is that now the LED supports also patterns.
 */

#include <stdio.h>
#include "stm32f10x.h"
#include "debug_trace.h"
#include "dev_uart.h"
#include "mod_led.h"
#include "timer_sched.h"
#include "neural_network.h"

#define LED_TIMER_MS 500

/* Those weight were calculated by training the model */
double weights[] = {
	9.67299303,
	-0.2078435,
	-4.62963669};

/* Those are the inputs to test and benchmark */
double inputs[8][3] = {
		{0, 0, 0},
		{0, 0, 1},
		{0, 1, 0},
		{0, 1, 1},
		{1, 0, 0},
		{1, 0, 1},
		{1, 1, 0},
		{1, 1, 1},
};

/* Benchmark timer object */
struct obj_timer_t * benchmark_timer;

volatile uint32_t glb_tmr_1ms;

#define LED_PORT GPIOC
#define LED_PIN GPIO_Pin_13

#define DEBUG_PORT GPIOB
#define DEBUG_PIN GPIO_Pin_12

uint32_t trace_levels;

/* Create the list head for the timer */
static LIST_HEAD(obj_timer_list);

// Declare uart
DECLARE_UART_DEV(dbg_uart, USART1, 115200, 256, 10, 1);

extern uint32_t overclock_stm32f103(void);
void dbg_uart_parser(uint8_t *buffer, size_t bufferlen, uint8_t sender);

static inline void main_loop(void)
{
	/* 1 ms timer */
	if (glb_tmr_1ms) {
		glb_tmr_1ms = 0;
		mod_timer_polling(&obj_timer_list);
	}
}

void led_on(void *data)
{
	LED_PORT->ODR |= LED_PIN;
}

void led_off(void *data)
{
	LED_PORT->ODR &= ~LED_PIN;
}

void led_init(void *data)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	LED_PORT->ODR |= LED_PIN;
}

void debug_pin_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = DEBUG_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(DEBUG_PORT, &GPIO_InitStructure);

	LED_PORT->ODR &= ~DEBUG_PIN;
}

void test_neural_network()
{

	for (int i=0; i<8; i++) {
		double result = nn_predict(inputs[i], weights, 3);
		TRACE(("result: %f\n", result));
	}
	TRACE(("\n"));
}

void benchmark_neural_network()
{
	DEBUG_PORT->ODR |= DEBUG_PIN;
	nn_predict(inputs[0], weights, 3);
	DEBUG_PORT->ODR &= ~DEBUG_PIN;
	TRACE(("DONE...\n"));
}

int main(void)
{
	SystemCoreClock = overclock_stm32f103();
	if (SysTick_Config(SystemCoreClock / 1000)) {
		/* Capture error */
		while (1);
	}
	
	trace_levels_set(
			0
			| TRACE_LEVEL_DEFAULT
			,1);

	// setup uart port
	dev_uart_add(&dbg_uart);
	// set callback for uart rx
 	dbg_uart.fp_dev_uart_cb = &dbg_uart_parser;
 	mod_timer_add((void*) &dbg_uart, 5, (void*) &dev_uart_update, &obj_timer_list);

	/* Declare LED module and initialize it */
	DECLARE_MODULE_LED(led_module, 8, 250);
	mod_led_init(&led_module);
	mod_timer_add((void*) &led_module, led_module.tick_ms, (void*) &mod_led_update, &obj_timer_list);

	/* Declare LED */
	DECLARE_DEV_LED(def_led, &led_module, 1, NULL, &led_init, &led_on, &led_off);
	dev_led_add(&def_led);
	dev_led_set_pattern(&def_led, 0b11001100);

	/* configure debug pin for debug */
	debug_pin_init();

	TRACE(("Program started\n"));

	// test_neural_network();

	/* main loop */
	while (1) {
		main_loop();
	}
}

void dbg_uart_parser(uint8_t *buffer, size_t bufferlen, uint8_t sender)
{
	buffer[bufferlen] = 0;
	if (!strncmp((char*) buffer, "TEST", 4)) {
		TRACE(("Running nn test:\n\n"));
		test_neural_network();
		TRACE(("\nFinished.\n----------\n"));
	}
	else if (!strncmp((char*) buffer, "START", 4)) {
		TRACE(("Starting benchmark mode...\n\n"));
		benchmark_timer = mod_timer_add(NULL, 2000, (void*) &benchmark_neural_network, &obj_timer_list);
	}
	else if (!strncmp((char*) buffer, "STOP", 4)) {
		TRACE(("Stoping benchmark mode...\n\n"));
		mod_timer_del(benchmark_timer, &obj_timer_list);
	}
}