#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_i2c.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"

// PWM clock will be SysClock / 16 = 3.125MHz
// PWM_COUNT_LOAD holds the countdown value for the PWM signal
// for 50Hz, 3.125MHz/50Hz = 62,500 then - 1 because this counts down to 0
#define PWM_COUNT_LOAD 62499
// pulse will be high for 3125 PWM clock cycles ~1msec
#define DUTY_CYCLE_5_PER 3125
// pulse will be high for 6250 PWM clock cycles ~2msec
#define DUTY_CYCLE_10_PER 6250
// largest 32-bit int
#define MAX_32_TIMER_SIZE 4294967295

// I2C MUX values (1st Byte of Config)
#define I2C_ADDR 0x48
#define I2C_CURR 0x46
#define I2C_VOLT 0x56
#define I2C_TOR 0x66
#define I2C_THU 0x76

// I2C 2nd Byte of Config
#define I2C_CONFIG 0xC3

// Global variable declaration
uint8_t bytes_4[4];					// 4 byte buffer for 32bit ints
uint8_t bytes_2[2];					// 2 byte buffer for 16bit ints (12-bit ADC)
volatile uint32_t timerLoad;		// Used to load values into Timer
volatile uint32_t freqRPM;			// Used to store and calculate current freq/RPM values
volatile uint8_t nextSection;		// boolean that tells test program to go to next section
volatile uint8_t nextDataBundle;	// boolean that tells test program to gather/send next data bundle


uint16_t ReadI2CADC(uint8_t selectByte){
	uint16_t currRead;
	uint8_t dataIn[2];

	// Address byte of the ADS1015
	I2CMasterSlaveAddrSet(I2C0_BASE, I2C_ADDR, false);
	// Points to the Config Register
	I2CMasterDataPut(I2C0_BASE, 1);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_BASE)){}
	// Read Write two bytes to the Config Register
	I2CMasterDataPut(I2C0_BASE, selectByte);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C0_BASE)){}
	I2CMasterDataPut(I2C0_BASE, I2C_CONFIG);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_BASE)){}

	// Point to Conversion Register
	I2CMasterSlaveAddrSet(I2C0_BASE, I2C_ADDR, false);
	// Points to the Conversion Register
	I2CMasterDataPut(I2C0_BASE, 0);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusy(I2C0_BASE)){}

	I2CMasterSlaveAddrSet(I2C0_BASE, I2C_ADDR, true);
	// Read 1st byte
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C0_BASE)){}
	dataIn[0] = I2CMasterDataGet(I2C0_BASE);
	// Read 2nd byte
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	while(I2CMasterBusy(I2C0_BASE)){}
	dataIn[1] = I2CMasterDataGet(I2C0_BASE);

	currRead = 69;

	return currRead;
}

void TestProgram1(void){	// Proof of concept
	uint16_t data;

	//	Clear UART FIFO
	while(UARTCharsAvail(UART0_BASE)){
		UARTCharPutNonBlocking(UART0_BASE,UARTCharGetNonBlocking(UART0_BASE));
		}

	//	Have terminal ask to press any key to start
	UARTCharPut(UART0_BASE,'s');

	while(!UARTCharsAvail(UART0_BASE)){	//	Waits before starting test
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
		SysCtlDelay(SysCtlClockGet() / (1000 * 3));		// ~ 1msec
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
		SysCtlDelay(SysCtlClockGet() / (10 * 3));
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
		SysCtlDelay(SysCtlClockGet() / (1000 * 3));		// ~ 1msec
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
		SysCtlDelay(SysCtlClockGet()/12);
	}
	//	A key has been pressed on the terminal. Clear UART FIFO
	while(UARTCharsAvail(UART0_BASE)){
		UARTCharPutNonBlocking(UART0_BASE,UARTCharGetNonBlocking(UART0_BASE));
		}

	//--------Test Begin--------
	while(1){
		data = ReadI2CADC(I2C_CURR);
		SysCtlDelay(SysCtlClockGet()/300);
	}

}

void TestProgram2(void){	// Premade programs: Square, Ramp, Sinusoid

}

void TestProgram3(void){	// User chooses start time, run time, and end time

}

void DebugProgram(void){
	int i;
	uint32_t timeMilli = 0;

	// 	Set timer to 5 seconds
	timerLoad = (SysCtlClockGet() * 5) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	timerLoad = (SysCtlClockGet()/100) - 1;
	TimerLoadSet(TIMER1_BASE, TIMER_A, timerLoad);
	TimerLoadSet(TIMER2_BASE, TIMER_A, MAX_32_TIMER_SIZE);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1);

    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);
    //	Enable PWM Signal
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);

	//	Enable timers
	TimerEnable(TIMER0_BASE, TIMER_A);
	TimerEnable(TIMER2_BASE, TIMER_A);
    // Enable GPIO interrupts on Port E pin 3
    IntEnable(INT_GPIOE);
    GPIOIntEnable(GPIO_PORTE_BASE, GPIO_INT_PIN_3);

	//--------Test Begin--------

	/*	~~~~~ Section # 1 ~~~~~
	 *
	 *	Lasts for 5 seconds. Runs motor at 0% speed.
	 *
	 */

	while(!nextSection){
			TimerEnable(TIMER1_BASE, TIMER_A);

			// Time in milliseconds
			bytes_4[0] = (timeMilli >> 24) & 0xFFFF;
			bytes_4[1] = (timeMilli >> 16) & 0xFFFF;
			bytes_4[2] = (timeMilli >> 8) & 0xFFFF;
			bytes_4[3] = timeMilli & 0xFFFF;
			for(i=0;i<4;i++){
			    UARTCharPutNonBlocking(UART0_BASE,bytes_4[i]);
			}
			// Input speed
			UARTCharPutNonBlocking(UART0_BASE,100);
			// Frequency
			bytes_4[0] = (freqRPM >> 24) & 0xFFFF;
			bytes_4[1] = (freqRPM >> 16) & 0xFFFF;
			bytes_4[2] = (freqRPM >> 8) & 0xFFFF;
			bytes_4[3] = freqRPM & 0xFFFF;
			for(i=0;i<4;i++){
			    UARTCharPutNonBlocking(UART0_BASE,bytes_4[i]);
			}
			// data
			bytes_2[0] = (6969 >> 8) & 0xFF;
			bytes_2[1] = 6969 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
			// data
			bytes_2[0] = (1000 >> 8) & 0xFF;
			bytes_2[1] = 1000 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
			// data
			bytes_2[0] = (9999 >> 8) & 0xFF;
			bytes_2[1] = 9999 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
			// data
			bytes_2[0] = (1234 >> 8) & 0xFF;
			bytes_2[1] = 1234 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
	    	while(!nextDataBundle){
	    	}
	    	nextDataBundle = false;
	    	timeMilli+=10;
		}

	/*	~~~~~ Section # 2 ~~~~~
	 *
	 *	Lasts for 15 seconds. Runs motor at 100% speed.
	 *
	 */

	//	Set PWM Signal to 2 ms (100% speed)
	ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 4500);

	// 	Set timer to 15 seconds
	timerLoad = (SysCtlClockGet() * 1) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	TimerEnable(TIMER0_BASE, TIMER_A);

	//	Disable continue to next section
	nextSection = false;

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);

	while(!nextSection){
			TimerEnable(TIMER1_BASE, TIMER_A);

			// Time in milliseconds
			bytes_4[0] = (timeMilli >> 24) & 0xFFFF;
			bytes_4[1] = (timeMilli >> 16) & 0xFFFF;
			bytes_4[2] = (timeMilli >> 8) & 0xFFFF;
			bytes_4[3] = timeMilli & 0xFFFF;
			for(i=0;i<4;i++){
			    UARTCharPutNonBlocking(UART0_BASE,bytes_4[i]);
			}
			// Input speed
			UARTCharPutNonBlocking(UART0_BASE,100);
			// Frequency
			bytes_4[0] = (freqRPM >> 24) & 0xFFFF;
			bytes_4[1] = (freqRPM >> 16) & 0xFFFF;
			bytes_4[2] = (freqRPM >> 8) & 0xFFFF;
			bytes_4[3] = freqRPM & 0xFFFF;
			for(i=0;i<4;i++){
			    UARTCharPutNonBlocking(UART0_BASE,bytes_4[i]);
			}
			// data
			bytes_2[0] = (6969 >> 8) & 0xFF;
			bytes_2[1] = 6969 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
			// data
			bytes_2[0] = (1000 >> 8) & 0xFF;
			bytes_2[1] = 1000 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
			// data
			bytes_2[0] = (9999 >> 8) & 0xFF;
			bytes_2[1] = 9999 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
			// data
			bytes_2[0] = (1234 >> 8) & 0xFF;
			bytes_2[1] = 1234 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
	    	while(!nextDataBundle){
	    	}
	    	nextDataBundle = false;
	    	timeMilli+=10;
		}

	/*	~~~~~ Section # 3 ~~~~~
	 *
	 *	Lasts for 10 seconds. Runs motor at 0% speed.
	 *
	 */

	//	Set PWM Signal back to 1 ms (0% speed)
	ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);

	// 	Set timer to 5 seconds
	timerLoad = (SysCtlClockGet() * 1) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	TimerEnable(TIMER0_BASE, TIMER_A);

	//	Disable continue to next section
	nextSection = false;

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);

	while(!nextSection){
			TimerEnable(TIMER1_BASE, TIMER_A);

			// Time in milliseconds
			bytes_4[0] = (timeMilli >> 24) & 0xFFFF;
			bytes_4[1] = (timeMilli >> 16) & 0xFFFF;
			bytes_4[2] = (timeMilli >> 8) & 0xFFFF;
			bytes_4[3] = timeMilli & 0xFFFF;
			for(i=0;i<4;i++){
			    UARTCharPutNonBlocking(UART0_BASE,bytes_4[i]);
			}
			// Input speed
			UARTCharPutNonBlocking(UART0_BASE,100);
			// Frequency
			bytes_4[0] = (freqRPM >> 24) & 0xFFFF;
			bytes_4[1] = (freqRPM >> 16) & 0xFFFF;
			bytes_4[2] = (freqRPM >> 8) & 0xFFFF;
			bytes_4[3] = freqRPM & 0xFFFF;
			for(i=0;i<4;i++){
			    UARTCharPutNonBlocking(UART0_BASE,bytes_4[i]);
			}
			// data
			bytes_2[0] = (6969 >> 8) & 0xFF;
			bytes_2[1] = 6969 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
			// data
			bytes_2[0] = (1000 >> 8) & 0xFF;
			bytes_2[1] = 1000 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
			// data
			bytes_2[0] = (9999 >> 8) & 0xFF;
			bytes_2[1] = 9999 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
			// data
			bytes_2[0] = (1234 >> 8) & 0xFF;
			bytes_2[1] = 1234 & 0xFF;
			for(i=0;i<2;i++){
				UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
			}
	    	while(!nextDataBundle){
	    	}
	    	nextDataBundle = false;
	    	timeMilli+=10;
		}

	nextSection = false;

	/*	~~~~~ Section # 4 ~~~~~
	 *
	 *	Terminate Test Program 1
	 *
	 */

	//	Turn off
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, false);
	ROM_PWMGenDisable(PWM1_BASE, PWM_GEN_0);


	// Signal end of test
	for(i=0;i<4;i++){
		UARTCharPutNonBlocking(UART0_BASE,'~');
	}
	freqRPM = 0;
	nextSection = false;
	IntDisable(INT_GPIOE);
	GPIOIntDisable(GPIO_PORTE_BASE, GPIO_INT_PIN_3);
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, false);
	ROM_PWMGenDisable(PWM1_BASE, PWM_GEN_0);
}

void Timer0IntHandler(void){
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	nextSection = true;
}

void Timer1AIntHandler(void){
	// Clear the timer interrupt
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	nextDataBundle = true;
}

void RisEdgeIntHandler(void){
	// Clear the Rising Edge interrupt
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_INT_PIN_3);
	freqRPM = TimerValueGet(TIMER2_BASE, TIMER_A);
	TimerLoadSet(TIMER2_BASE, TIMER_A, MAX_32_TIMER_SIZE);
	if(freqRPM == 0){
		freqRPM = 1;
	}
	freqRPM = MAX_32_TIMER_SIZE - freqRPM;
	freqRPM = SysCtlClockGet()/freqRPM;
	freqRPM = (10*freqRPM);		// rpm = (120*freq)/poles   ... p = 12 for now
}

void Config(void){
	/*
	 * Note: ROM_* is a way to call a function for the TIVA's onboard ROM instead
	 * of relying on cluttering its flash drive memory with too many functions
	 */

	// Sets clock at 50MHz... 400MHz PLL /2 = 200 then div SYSDIV_4 = 50MHz
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	// PWM clock set to SysClock / 16 = 3.125MHz
	ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_16);
	
	// Enable peripherals...
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);	// start mid end sections
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);	// sample bundle control (1 per 10 ms)
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);	// RPM value control
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);	// Communication with PC
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);		// PWM control to ESC
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);		// Serial comm with 4-channel ADC
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);	// where UART is
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);	// where I2C is
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);	// where PWM is
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);	// used in RPM interrupt detection
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);	// where LED's are

    ROM_GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    ROM_GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    ROM_GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    ROM_GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
    ROM_I2CMasterEnable(I2C0_BASE);
    ROM_I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(),false);

    // Configure Port F pins to blink blue, green, and red light
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    // Configure UART pins, clock, and set BAUD RATE to 115200
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    // Configure Port E pin 3 to be a rising edge interrupt input
    // This will be used to detect when the RPM sensor rises high
    ROM_GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3);
    ROM_GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_RISING_EDGE);

    // Configure PWM pins and PWM mode
    ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
    ROM_GPIOPinConfigure(GPIO_PD0_M1PWM0);
    // Sets PWM generator as a count down to 0 from a specified value every PWM clock cycle
    ROM_PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
    // Loads the count number as PWM_COUNT_LOAD
    ROM_PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, PWM_COUNT_LOAD);

    // Configure Timer which will control the timing of test legs
	ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
	// Configure Timer which will be used to time data transmission
	ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);
	// Configure Timer which will be used to record RPM information
	ROM_TimerConfigure(TIMER2_BASE, TIMER_CFG_ONE_SHOT);

	// Configure Timer Interrupts
	//	Timer handling test sections (start, mid, end)
	ROM_IntEnable(INT_TIMER0A);
	ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	//	Timer handling data group timing (100 sps...)
	ROM_IntEnable(INT_TIMER1A);
	ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}

int main(void) {
	uint8_t i;
	uint8_t ui8LED = 2;
	char optionInput;
	nextSection = false;
	nextDataBundle = false;
	timerLoad = 1;
	freqRPM = 0;

	Config();

	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	TimerLoadSet(TIMER1_BASE, TIMER_A, timerLoad);
	TimerLoadSet(TIMER2_BASE, TIMER_A, timerLoad);

	// Wait until comm is established with test terminal. Loops twice because there is an
    // initial char sent to the UART RX FIFO when you plug in the USB.
    for(i=0;i<2;i++){
		while(!UARTCharsAvail(UART0_BASE)){
			// Blinks blue 2 times every half second waiting for comm with terminal
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
			SysCtlDelay(SysCtlClockGet() / (1000 * 3));		// ~ 1msec
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
			SysCtlDelay(SysCtlClockGet() / (10 * 3));
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
			SysCtlDelay(SysCtlClockGet() / (1000 * 3));		// ~ 1msec
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
			SysCtlDelay(SysCtlClockGet()/12);
		}
	    // handshake (send back bit)
		UARTCharPutNonBlocking(UART0_BASE,UARTCharGetNonBlocking(UART0_BASE));
    }

    // comm received.
    // clear UART FIFO
    while(UARTCharsAvail(UART0_BASE)){
		UARTCharPutNonBlocking(UART0_BASE,UARTCharGetNonBlocking(UART0_BASE));
    	}

    // Enable all Interrupts
    IntMasterEnable();

    // Loop forever and wait for UART activity
    while (1){
    	while(UARTCharsAvail(UART0_BASE)){
    		optionInput = UARTCharGetNonBlocking(UART0_BASE);

    		switch(optionInput){
    			case '1':
    			{
    				TestProgram1();
    				break;
    			}
    			case '2':
    			{
    				TestProgram2();
    				break;
    			}
    			case '3':
    			{
    				TestProgram3();
    				break;
    			}
    			case '4':
    			{
    				DebugProgram();
    				break;
    			}
    			default:
    				//	Tell testing terminal that the selection was invalid
    		}
    	}
		// Turn on the LED
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
		// Delay for a bit
		SysCtlDelay(SysCtlClockGet()/12);
		// Cycle through Red, Green and Blue LEDs
		if (ui8LED == 8) {ui8LED = 2;} else {ui8LED = ui8LED*2;}
    }
}
