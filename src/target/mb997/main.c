//-----------------------------------------------------------------------------
/*

   MB997C Board

 */
//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"
#include "debounce.h"
#include "utils.h"
#include "io.h"
#include "zforth.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------
// IO configuration

static const struct gpio_info gpios[] = {
	// leds
	{IO_LED_RED, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_LED_BLUE, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_LED_GREEN, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_LED_AMBER, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	// push buttons
	{IO_PUSH_BUTTON, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	// serial port (usart2 function)
	{IO_UART_TX, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_HI, GPIO_PUPD_NONE, GPIO_AF7, 0},
	{IO_UART_RX, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_HI, GPIO_PUPD_NONE, GPIO_AF7, 0},
	// display
#if defined(SPI_DRIVER_HW)
	{IO_LCD_SDO, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF5, 0},
	{IO_LCD_SCK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF5, 0},
	{IO_LCD_SDI, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF5, 0},
#elif defined(SPI_DRIVER_BITBANG)
	{IO_LCD_SDO, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_LCD_SCK, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_LCD_SDI, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 0},
#else
#error "what kind of SPI driver are we building?"
#endif
	{IO_LCD_DATA_CMD, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 1},
	{IO_LCD_RESET, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 1},
	{IO_LCD_CS, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 1},
	{IO_LCD_LED, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	// audio
	{IO_AUDIO_RESET, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_AUDIO_I2C_SCL, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_AUDIO_I2C_SDA, GPIO_MODER_IN, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{IO_AUDIO_I2S_MCK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{IO_AUDIO_I2S_SCK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{IO_AUDIO_I2S_SD, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
	{IO_AUDIO_I2S_WS, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF6, 0},
};

//-----------------------------------------------------------------------------

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t * file, uint32_t line) {
	while (1);
}
#endif

void Error_Handler(void) {
	while (1);
}

//-----------------------------------------------------------------------------

void NMI_Handler(void) {
}
void HardFault_Handler(void) {
	while (1);
}
void MemManage_Handler(void) {
	while (1);
}
void BusFault_Handler(void) {
	while (1);
}
void UsageFault_Handler(void) {
	while (1);
}
void SVC_Handler(void) {
}
void DebugMon_Handler(void) {
}
void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
	uint32_t ticks = HAL_GetTick();
	// blink the green led every 512 ms
	if ((ticks & 511) == 0) {
		gpio_toggle(IO_LED_GREEN);
	}
	// sample debounced inputs every 16 ms
	if ((ticks & 15) == 0) {
		debounce_isr();
	}
	HAL_IncTick();
}

//-----------------------------------------------------------------------------

static void SystemClock_Config(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	// Enable Power Control clock
	__PWR_CLK_ENABLE();

	// The voltage scaling allows optimizing the power consumption when the device is
	// clocked below the maximum system frequency, to update the voltage scaling value
	// regarding system frequency refer to product datasheet.
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	// Enable HSE Oscillator and activate PLL with HSE as source
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	// Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

//-----------------------------------------------------------------------------
// key debouncing (called from the system tick isr)

#define PUSH_BUTTON_BIT 0

// handle a key down
void debounce_on_handler(uint32_t bits) {
	if (bits & (1 << PUSH_BUTTON_BIT)) {
		//event_wr(EVENT_TYPE_KEY_DN | 0U, NULL);
	}
}

// handle a key up
void debounce_off_handler(uint32_t bits) {
	if (bits & (1 << PUSH_BUTTON_BIT)) {
		//event_wr(EVENT_TYPE_KEY_UP | 0U, NULL);
	}
}

// map the gpio inputs to be debounced into the 32 bit debounce state
uint32_t debounce_input(void) {
	return gpio_rd(IO_PUSH_BUTTON) << PUSH_BUTTON_BIT;
}

//-----------------------------------------------------------------------------
// console port (on USART2)

struct usart_cfg serial_cfg = {
	.base = USART2_BASE,
	.baud = 115200,
	.data = 8,
	.parity = 0,
	.stop = 1,
};

struct usart_drv serial_drv;

void USART2_IRQHandler(void) {
	usart_isr(&serial_drv);
}

//-----------------------------------------------------------------------------

static const char *abort_msg[] = {
	"ok", // ZF_OK,
	"internal error", // ZF_ABORT_INTERNAL_ERROR,
	"outside memory", // ZF_ABORT_OUTSIDE_MEM,
	"dstack underrun", // ZF_ABORT_DSTACK_UNDERRUN,
	"dstack overrun", // ZF_ABORT_DSTACK_OVERRUN,
	"rstack underrun", // ZF_ABORT_RSTACK_UNDERRUN,
	"rstack overrun", // ZF_ABORT_RSTACK_OVERRUN,
	"not a word", // ZF_ABORT_NOT_A_WORD,
	"compile-only word", // ZF_ABORT_COMPILE_ONLY_WORD,
	"invalid size", // ZF_ABORT_INVALID_SIZE,
	"division by zero", // ZF_ABORT_DIVISION_BY_ZERO
};

#define ABORT_MSG_MAX (sizeof(abort_msg) / sizeof(const char *))

static zf_result do_eval(const char *buf) {
	zf_result rv = zf_eval(buf);
	if (rv != ZF_OK) {
		if (rv < ABORT_MSG_MAX) {
			fprintf(stderr, "%s\n", abort_msg[rv]);
		} else {
			fprintf(stderr, "unknown error: %d\n", rv);
		}
	}
	return rv;
}

zf_cell zf_host_parse_num(const char *buf) {
	zf_cell v;
	int r = sscanf(buf, ZF_CELL_FMT, &v);
	if (r == 0) {
		zf_abort(ZF_ABORT_NOT_A_WORD);
	}
	return v;
}

zf_input_state zf_host_sys(zf_syscall_id id, const char *input) {
	(void)input;

	switch ((int)id) {
	case ZF_SYSCALL_EMIT:
		putchar((char)zf_pop());
		fflush(stdout);
		break;
	case ZF_SYSCALL_PRINT:
		printf(ZF_CELL_FMT " ", zf_pop());
		break;
	default: {
		printf("unhandled syscall %d\n", id);
		break;
	}
	}
	return ZF_INPUT_INTERPRET;
}

int SEGGER_RTT_vprintf(unsigned BufferIndex, const char *sFormat, va_list * pParamList);

void zf_host_trace(const char *fmt, va_list va) {
	SEGGER_RTT_vprintf(0, fmt, &va);
	SEGGER_RTT_WriteString(0, "\n");
}

//-----------------------------------------------------------------------------

int main(void) {
	int rc;

	HAL_Init();
	SystemClock_Config();

	rc = log_init();
	if (rc != 0) {
		goto exit;
	}

	rc = gpio_init(gpios, sizeof(gpios) / sizeof(struct gpio_info));
	if (rc != 0) {
		DBG("gpio_init failed %d", rc);
		goto exit;
	}

	rc = debounce_init();
	if (rc != 0) {
		DBG("debounce_init failed %d", rc);
		goto exit;
	}

	rc = usart_init(&serial_drv, &serial_cfg);
	if (rc != 0) {
		DBG("usart_init failed %d", rc);
		goto exit;
	}
	// setup the interrupts for the serial port
	HAL_NVIC_SetPriority(USART2_IRQn, 10, 0);
	NVIC_EnableIRQ(USART2_IRQn);

	DBG("init good");

	// initialize zforth
	zf_init(1);
	zf_bootstrap();
	zf_eval(": . 1 sys ;");

	puts("\nzForth for Cortex-M\n");

	for (;;) {
		char buf[256];
		if (fgets(buf, sizeof(buf), stdin)) {
			do_eval(buf);
		} else {
			break;
		}
	}

exit:
	while (1);
	return 0;
}

//-----------------------------------------------------------------------------
