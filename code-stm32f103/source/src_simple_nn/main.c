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

/* Those are the weights for the NN with the hidden layer */
double weights_1[32][3] = {
		{ 1.5519611 ,  0.65860842, -0.85158313},
		{ 1.06265208,  0.31949065, -0.13750623},
		{-2.30455417,  0.30591339,  0.7240955 },
		{ 0.30877321,  0.13609545,  0.06030529},
		{-1.17039621,  0.37650795,  0.68144084},
		{-0.69348733,  0.60220631,  0.28797794},
		{ 0.40589165, -0.18831817,  0.35705378},
		{-0.05932593,  0.59004868, -0.1701374 },
		{ 1.392689  ,  0.70259631, -0.74218012},
		{ 0.48329391,  0.60207043,  0.38431145},
		{-0.95295493,  0.07404055,  0.3302735 },
		{ 2.10166106,  0.39611358, -1.00823685},
		{-0.08448525, -0.10317019,  0.53841852},
		{ 2.540211  ,  0.05323626, -1.06332893},
		{-1.84539917,  0.48765352,  0.5685712 },
		{-0.01802026,  0.18219485,  0.62404852},
		{ 0.92427558,  0.04736742,  0.12561239},
		{ 1.08965851, -0.0801783 , -0.08860555},
		{-1.82176436, -0.20573879,  0.780304  },
		{ 0.45220284,  0.4997675 , -0.18653686},
		{ 2.14035487, -0.03879152, -0.80268326},
		{ 1.82177898, -0.11746983, -0.36502932},
		{ 0.62309348,  0.30696019, -0.13735078},
		{-0.05199235, -0.05282913,  0.91758557},
		{ 0.29694869,  0.54693956,  0.69834386},
		{ 0.39180283,  0.07674308,  0.54732749},
		{-1.72233784,  0.36036508,  0.28150989},
		{-1.70252288,  0.30217376,  0.69514718},
		{ 0.78278019, -0.07366525, -0.16997775},
		{ 1.28860736,  0.17580282, -0.17742198},
		{-0.47323461,  0.48374213,  0.08422625},
		{ 1.49427898,  0.191315  , -0.6043115 }
};

double weights_2[] = {
		  1.45800076,
		  0.70967606,
		 -2.68516009,
		  0.02268129,
		 -1.49619033,
		 -0.91787325,
		  0.09344477,
		 -0.29547836,
		  1.28547469,
		  0.04046954,
		 -1.29961045,
		  2.02908422,
		 -0.42753493,
		  2.49285692,
		 -2.11993655,
		 -0.59068471,
		  0.62114759,
		  0.76446439,
		 -2.22804807,
		  0.25244996,
		  1.9591224 ,
		  1.53351039,
		  0.37624935,
		 -0.74102723,
		 -0.48484892,
		 -0.2683768 ,
		 -2.020174  ,
		 -1.99787033,
		  0.54688533,
		  0.90035289,
		 -0.67413096,
		  1.31408841,
};

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

static inline void toggle_debug_pin(void)
{
	/* First toggle */
	DEBUG_PORT->ODR |= DEBUG_PIN;
	DEBUG_PORT->ODR &= ~DEBUG_PIN;

	/* Second toggle */
	DEBUG_PORT->ODR |= DEBUG_PIN;
	DEBUG_PORT->ODR &= ~DEBUG_PIN;
}

void benchmark_neural_network()
{
	toggle_debug_pin();
	/* Predict */
	DEBUG_PORT->ODR |= DEBUG_PIN;
  	double output = sigmoid(dot(inputs[0], weights, 3));
	DEBUG_PORT->ODR &= ~DEBUG_PIN;
	TRACE(("DONE: %f\n", output));
}

/**
 *  @brief This NN has a hidden layer with 32 nodes.
 * 	This function will print all the outputs in order
 *  to verify that are valid
 */
void test_neural_network2()
{
	double layer[32];

	for (int k=0; k<8; k++) {
		for (int i=0; i<32; i++) {
			layer[i] = sigmoid(dot(inputs[k], weights_1[i], 3));
		}
		double output = sigmoid(dot(layer, weights_2, 32));
		TRACE(("output[%d]: %f\n", k, output));
	}
}

/**
 * @brief This NN has a hidden layer with 32 nodes.
 * This function will just do a forward run
 */
void benchmark_neural_network2()
{
	double layer[32];

	DEBUG_PORT->ODR |= DEBUG_PIN;
	for (int i=0; i<32; i++) {
		layer[i] = sigmoid(dot(inputs[0], weights_1[i], 3));
	}
	double output = sigmoid(dot(layer, weights_2, 32));
	DEBUG_PORT->ODR &= ~DEBUG_PIN;
	TRACE(("DONE...%f\n", output));
}

int main(void)
{
	/* Uncomment this for 128 MHz */
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

	/* main loop */
	while (1) {
		main_loop();
	}
}

void dbg_uart_parser(uint8_t *buffer, size_t bufferlen, uint8_t sender)
{
	buffer[bufferlen] = 0;
	if (!strncmp((char*) buffer, "TEST=", 5)) {
		/* Get mode */
		uint8_t mode = atoi((const char*) &buffer[5]);

		TRACE(("Running nn test mode: %d\n\n", mode));
		switch(mode) {
			case 1:
				test_neural_network();
				break;
			case 2:
				test_neural_network2();
				break;
			default:
				TRACE(("Available modes: 1-2\n"));
		}
		TRACE(("\nFinished.\n----------\n"));
	}
	else if (!strncmp((char*) buffer, "START=", 6)) {
		/* Get mode */
		uint8_t mode = atoi((const char*) &buffer[6]);

		TRACE(("Starting benchmark mode %d...\n\n", mode));
		switch(mode) {
			case 1:
				benchmark_timer = mod_timer_add(NULL, 2000, (void*) &benchmark_neural_network, &obj_timer_list);
				break;
			case 2:
				benchmark_timer = mod_timer_add(NULL, 2000, (void*) &benchmark_neural_network2, &obj_timer_list);
				break;
			default:
				TRACE(("Available modes: 1-2\n"));
		}
	}
	else if (!strncmp((char*) buffer, "STOP", 4)) {
		TRACE(("Stoping benchmark mode...\n\n"));
		mod_timer_del(benchmark_timer, &obj_timer_list);
	}
}