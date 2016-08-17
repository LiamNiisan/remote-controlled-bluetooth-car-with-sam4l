//Code to control a car by bluetooth and UART protocol 
//Par Badr Jaidi

/* To help make the code, I used the libraries and drivers provided by ATMEL in ATMEL studio
This code is made to work with a sam4l microchip*/


#include <asf.h>
#include <string.h>
#include <stdio.h>
#include "usart.h"
#include <inttypes.h>
#include <conf_board.h>
#include <conf_clock.h>

volatile int sonne, hpLevel = 0;
volatile int freq = 500;// variable to control the frequency emited by the speaker
volatile int CountMusic = 0;





/// @cond
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

#define STRING_EOL    "\r"
#define STRING_HEADER "--TC capture waveform Example --\r\n" \
		"-- "BOARD_NAME " --\r\n" \
		"-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL

//! [tc_capture_selection]
#define TC_CAPTURE_TIMER_SELECTION TC_CMR_TCCLKS_TIMER_CLOCK3
//! [tc_capture_selection]

struct waveconfig_t {
	/** Internal clock signals selection. */
	uint32_t ul_intclock;
	/** Waveform frequency (in Hz). */
	uint16_t us_frequency;
	/** Duty cycle in percent (positive).*/
	uint16_t us_dutycycle;
};

/** TC waveform configurations */
static const struct waveconfig_t gc_waveconfig[] = {
	{TC_CMR_TCCLKS_TIMER_CLOCK4, 178, 30},
	{TC_CMR_TCCLKS_TIMER_CLOCK3, 375, 50},
	{TC_CMR_TCCLKS_TIMER_CLOCK3, 800, 75},
	{TC_CMR_TCCLKS_TIMER_CLOCK2, 1000, 80},
	{TC_CMR_TCCLKS_TIMER_CLOCK2, 4000, 55}
};

#if (SAM4L)
/* The first one is meaningless */
static const uint32_t divisors[5] = { 0, 2, 8, 32, 128};
#else
/* The last one is meaningless */
static const uint32_t divisors[5] = { 2, 8, 32, 128, 0};
#endif

/** Current wave configuration*/
static uint8_t gs_uc_configuration = 0;

/** Number of available wave configurations */
const uint8_t gc_uc_nbconfig = sizeof(gc_waveconfig)
		/ sizeof(struct waveconfig_t);

/** Capture status*/
static uint32_t gs_ul_captured_pulses;
static uint32_t gs_ul_captured_ra;
static uint32_t gs_ul_captured_rb;

/**
 * \brief Display the user menu on the UART.
 */

/**
 * \brief Configure TC TC_CHANNEL_WAVEFORM in waveform operating mode.
 */
static void tc_waveform_initialize(void)
{
	
    
	/* Configure the PMC to enable the TC module. */
	sysclk_enable_peripheral_clock(ID_TC_WAVEFORM);
#if SAMG55
	/* Enable PCK output */
	pmc_disable_pck(PMC_PCK_3);
	pmc_switch_pck_to_mck(PMC_PCK_3, PMC_PCK_PRES_CLK_1);
	pmc_enable_pck(PMC_PCK_3);
#endif
	/* Init TC to waveform mode. */
	tc_init(TC, TC_CHANNEL_WAVEFORM,
			/* Waveform Clock Selection */
			gc_waveconfig[gs_uc_configuration].ul_intclock
			| TC_CMR_EEVT_XC0_OUTPUT
            | TC_CMR_WAVE /* Waveform mode is enabled */
			| TC_CMR_ACPA_SET /* RA Compare Effect: set */
			| TC_CMR_ACPC_CLEAR /* RC Compare Effect: clear */
			| TC_CMR_BCPB_SET /* RA Compare Effect: set */
			| TC_CMR_BCPC_CLEAR /* RC Compare Effect: clear */
			| TC_CMR_CPCTRG /* UP mode with automatic trigger on RC Compare */
	);



	/* Enable TC TC_CHANNEL_WAVEFORM. */
	tc_start(TC, TC_CHANNEL_WAVEFORM);

}

//! [tc_capture_init]

/**
 * \brief Interrupt handler for the TC TC_CHANNEL_CAPTURE
 */
//! [tc_capture_irq_handler_start]


volatile uint32_t ul_ms_ticks = 0;

static void mdelay(uint32_t ul_dly_ticks)
{
	uint32_t ul_cur_ticks;

	ul_cur_ticks = ul_ms_ticks;
	while ((ul_ms_ticks - ul_cur_ticks) < ul_dly_ticks) {
	}
}

void SysTick_Handler(void)
{ 
	CountMusic--;
	if(sonne) //To make a frequency come out of the speaker, this handler makes the output of the speaker alternate between 1 and 0  
	{   
		
		if (hpLevel)
		{
			ioport_set_pin_level(PIN_PC26, true);
			hpLevel = 0;
		}
		else
		{
			ioport_set_pin_level(PIN_PC26, false);
			hpLevel = 1;
		}
	}
}


/**
 *  Configure serial console.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console. */
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

#define USART_SERIAL                 USART1
#define USART_SERIAL_ID              ID_USART1  //USART1 for sam4l
#define USART_SERIAL_BAUDRATE        9600
#define USART_SERIAL_CHAR_LENGTH     US_MR_CHRL_8_BIT
#define USART_SERIAL_PARITY          US_MR_PAR_NO
#define USART_SERIAL_STOP_BIT        false //no stop bit 


/* Configure console. */

/**
 * \brief Display the user menu on the terminal.
 */

/**
 * \brief main function : do init and loop
 */
int main(void)
{
	
	
        uint8_t key;
        uint16_t frequence, dutycycle;	
		int i = 0;
		int note = 0;
		int Xtra = 0;
		
		 uint8_t a= 1234;
		 int bit_rx = 6;
		 int ctlr7 = "7";
		 uint32_t ra, rb ,rc;
	
		/* Initialize the SAM system */
		sysclk_init();
		board_init();
		
		
			 ioport_set_pin_dir(PIN_PA11, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PC00, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PC01, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PC02, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PC03, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PC04, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PC26, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PB14, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PB15, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PC24, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PC25, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PB12, IOPORT_DIR_INPUT);
			 ioport_set_pin_dir(PIN_PB13, IOPORT_DIR_INPUT);
			 ioport_set_pin_dir(PIN_PA22, IOPORT_DIR_INPUT);
			 ioport_set_pin_dir(PIN_PA16A_USART1_TXD, IOPORT_DIR_OUTPUT);
			 ioport_set_pin_dir(PIN_PA15A_USART1_RXD, IOPORT_DIR_INPUT);
			 /* Output example information */

	         if (SysTick_Config(sysclk_get_cpu_hz() / freq)) { //the divider controls the frequency from here
		         while (1) {  /* Capture error */
		         }
	         }
	         


	         
	         
	         const sam_usart_opt_t usart_console_settings = {
		         USART_SERIAL_BAUDRATE,
		         USART_SERIAL_CHAR_LENGTH,
		         USART_SERIAL_PARITY,
		         USART_SERIAL_STOP_BIT,
		         US_MR_CHMODE_NORMAL
	         };
	         
	         static usart_serial_options_t usart_options = {
		         .baudrate = USART_SERIAL_BAUDRATE,
		         .charlength = USART_SERIAL_CHAR_LENGTH,
		         .paritytype = USART_SERIAL_PARITY,
		         .stopbits = USART_SERIAL_STOP_BIT
	         };
	         usart_serial_init(USART_SERIAL, &usart_options);
	         #if SAM4L
	         sysclk_enable_peripheral_clock(USART_SERIAL);
	         #else
	         sysclk_enable_peripheral_clock(USART_SERIAL_ID);
	         #endif
	         
	         usart_enable_tx(USART_SERIAL);
	         usart_enable_rx(USART_SERIAL);
	         
	         
	         ioport_set_pin_level(PIN_PC04, true);
        	/** Configure PIO Pins for TC */
        	ioport_set_pin_mode(PIN_TC_WAVEFORM, PIN_TC_WAVEFORM_MUX);
			ioport_set_pin_mode(PIN_TC_WAVEFORM2, PIN_TC_WAVEFORM_MUX2);
        	/** Disable I/O to enable peripheral mode) */
        	ioport_disable_pin(PIN_TC_WAVEFORM);
			ioport_disable_pin(PIN_TC_WAVEFORM2);
        	/** Configure PIO Pins for TC */
        
        	//! [tc_capture_gpio]                 
        	tc_waveform_initialize();
			
			
				
			rc = 10000; /* By initializing rc here, the result of the PWM will always be ra and rb by comparaison to rc*/ 
			
			ra = 10; //ra controls channel A of PWM
			rb = 10; //rb is same as ra but for channel B
			
            //! [tc_capture_init_irq]
        	/** Configure TC interrupts for TC TC_CHANNEL_CAPTURE only */
        	NVIC_DisableIRQ(TC_IRQn);
        	NVIC_ClearPendingIRQ(TC_IRQn);
        	NVIC_SetPriority(TC_IRQn, 0);
        	NVIC_EnableIRQ(TC_IRQn);
        	//! [tc_capture_init_irq]

	   
while (1) {
		  if (SysTick_Config(sysclk_get_cpu_hz() / freq)) {
			  while (1) {  /* Capture error */
			  }
		  }
		
		
					usart_serial_getchar(USART_SERIAL, &a); 
					/*As long as nothing is received on RX, this function will wait*/
					 
					 /*This function plays music while the car is moving, I'm using a clock handler to not have to distrupt 
					 the main code*/
					 
				    if (Xtra == 1)
				    {   
						sonne = 1;
						if (CountMusic <= 0 )
						{
							
							if (note == 0)
							{
								
								freq = 880;
								CountMusic = 250;
								note = 1;
							}
						   
						   
						    else if (note == 1)
						    {
								
							    freq = 987;
								CountMusic = 250;
								note = 2;
						    }
							
							  else if (note == 2)
						    {
								
							    freq = 783;
								CountMusic = 250;
								note = 3;
						    }
							
							  else if (note == 3)
						    {
								
							    freq = 880;
								CountMusic = 250;
								note = 4;
						    }
							
							  else if (note == 4)
						    {
								
							    freq = 1046;
								CountMusic = 250;
								note = 5;
						    }
							
							  else if (note == 5)
						    {
								
							    freq = 987;
								CountMusic = 100;
								note = 6;
						    }
							
							  else if (note == 6)
						    {
								
							    freq = 783;
								CountMusic = 250;
								note = 7;
						    }
							
							  else if (note == 7)
						    {
								
							    freq = 880;
								CountMusic = 250;
								note = 0;
						    }
							
							
						}
				    }
				   
					bit_rx = (int)a;
		            //usart_serial_putchar(USART_SERIAL,a);
					
					switch (a)
					{
						case 'S': // Stops the car in PWM mode, because ra = rc, so the counter stops
						
						tc_write_rc(TC, TC_CHANNEL_WAVEFORM, rc);
						tc_write_ra(TC, TC_CHANNEL_WAVEFORM, rc);
						tc_write_rb(TC, TC_CHANNEL_WAVEFORM, rc);
						break;
						
						
						case 'F': //forward
						
						tc_write_rc(TC, TC_CHANNEL_WAVEFORM, rc);
						tc_write_rb(TC, TC_CHANNEL_WAVEFORM, rb);
						break;
						
						case 'B': //back
						
						tc_write_rc(TC, TC_CHANNEL_WAVEFORM, rc);
						tc_write_ra(TC, TC_CHANNEL_WAVEFORM, ra);
						break;
						
					    case 'L': //right
					    
					    ioport_set_pin_level(PIN_PC02, 1);
					    for(i = 0; i < 25000; i++);
						ioport_set_pin_level(PIN_PC02, 0);
						if (ioport_get_pin_level(PIN_PB13) == 1);
						{
							ioport_set_pin_level(PIN_PC02, 0);
						}
					    break;
					    
					    
					    case 'R': //left
					    
					    ioport_set_pin_level(PIN_PC03, 1);
						for(i = 0; i < 25000; i++);
						ioport_set_pin_level(PIN_PC03, 0);
						if (ioport_get_pin_level(PIN_PB13) == 1);
						{
							ioport_set_pin_level(PIN_PC03, 0);
						}
					    break;
						
					    case 'W': //front light on
								    
					    ioport_set_pin_level(PIN_PB14, LED_0_ACTIVE);
						ioport_set_pin_level(PIN_PB15, LED_0_ACTIVE);
					 
					    break;
						
						case 'w': //front light off
						 
						ioport_set_pin_level(PIN_PB14, !LED_0_ACTIVE);
						ioport_set_pin_level(PIN_PB15, !LED_0_ACTIVE);
						 
						break;
						
					    case 'U': //rear light on
					    
					    ioport_set_pin_level(PIN_PC24, LED_0_ACTIVE);
					    ioport_set_pin_level(PIN_PC25, LED_0_ACTIVE);
					    
					    break;
					    
					    case 'u': //rear light off
					    
					    ioport_set_pin_level(PIN_PC24, !LED_0_ACTIVE);
					    ioport_set_pin_level(PIN_PC25, !LED_0_ACTIVE);
					    
					    break;
						
						case 'X': //allows the speaker to output sounf
						
						Xtra = 1;
						freq = 3000;
						break;
						
						case 'x': //stops the speaker
						
						Xtra = 0;
						sonne = 0;
						
						break;
						
						
						//speed control, from the lowest 0 to q 
						
						case '0':  
						
						rb = 10000;
						ra = 10000;
						break;
						
						case '1': 
						
						rb = 5000;
						ra = 5000;
						break;
						
						case '2': 
						
						rb = 2500;
						ra = 2500;
						break;
						
						case '3': 
						
						rb = 1250;
						ra = 1250;
						break;
						
						case '4': 
						
						rb = 700;
						ra = 700;
						break;
						
						case '5': 
						
						rb = 350;
						ra = 350;
						break;
						
						case '6': 
						
						rb = 175;
						ra = 175;
						break;
						
						case '7': 
						
						rb = 80;
						ra = 80;
						break;
						
						case '8': 
						
						rb = 50;
						ra = 50;
						break;
						
						case '9': 
						
						rb = 10;
						ra = 10;
						break;
						
						case 'q': 
						
						rb = 1;
						ra = 1;
						break;
						
						case 'V':  //horn on
						sonne = 0;
						freq = 950;
						sonne = 1;
						break;
						
						case 'v':  //horn off
						sonne = 0;
						break;
						
						case 'a' ://board test LED on
						
						ioport_set_pin_level(PIN_PA11, LED_0_ACTIVE);
						
						usart_write_line(USART_SERIAL, "LED ON \n");
						break;
						
						
						case 'b' : //board test LED off
						
						ioport_set_pin_level(PIN_PA11, !LED_0_ACTIVE);
					    
						usart_write_line(USART_SERIAL,"LED OFF \n");
						break;
						
						case ';':  //output sound 659HZ
						sonne = 0;
						freq = 659;
						sonne = 1;
						break;
						
						case '!':  //output sound 698HZ
						sonne = 0;
						freq = 698;
						sonne = 1;
						break;
						
						case '?':  //output sound 783HZ
						sonne = 0;
						freq = 783;
						sonne = 1;
						break;
						
						case '@':  //output sound 523HZ = Do
						sonne = 0;
						freq = 523;
						sonne = 1;
						break;
						
						case '#':  //output sound 587HZ = Re
						sonne = 0;
						freq = 587;
						sonne = 1;
						break;
						
						case '$':  //output sound 659Z = Mi
						sonne = 0;
						freq = 659;
						sonne = 1;
						break;
						
						case '%':  //output sound 698HZ = Fa
						sonne = 0;
						freq = 698;
						sonne = 1;
						break;
						
						case '&':  //output sound 783HZ =  Sol
						sonne = 0;
						freq = 783;
						sonne = 1;   
						break;
						
						case '-':  //output sound 880HZ =  La
						sonne = 0;
						freq = 880;
						sonne = 1;
						break;
						
						case '+':  //output sound 987HZ =  Si
						sonne = 0;
						freq = 987;
						sonne = 1;
						break;
						
						case '(':  //output sound 1046HZ =  Do
						sonne = 0;
						freq = 1046;
						sonne = 1;
						break;	
						
						case ')':  //fait marcher le Claxon
						sonne = 0;
						freq = 932;
						sonne = 1;
						break;					
						
					}
						
						
		}
		
   }

/// @cond
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
