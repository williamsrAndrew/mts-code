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
#include "driverlib/timer.h"

#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"

// PWM clock will be SysClock / 16 = 3.125MHz
// PWM_COUNT_LOAD holds the countdown value for the PWM signal
// for 50Hz, 3.125MHz/50Hz = 62,500 then - 1 because this counts down to 0
#define PWM_COUNT_LOAD 62499
// pulse will be high for 3125 PWM clock cycles ~1msec
#define DUTY_CYCLE_5_PER 3125
// pulse will be high for 6250 PWM clock cycles ~2msec
#define DUTY_CYCLE_10_PER 6250

#define WTIMER_BIT_LOAD 400000000000000000

// I2C MUX values (1st Byte of Config)
#define I2C_ADDR 0x48
#define I2C_CURR 0x42
#define I2C_VOLT 0x52
#define I2C_TOR 0x62
#define I2C_THU 0x72

// I2C 2nd Byte of Config
#define I2C_CONFIG 0xE3

// Global variable declaration
uint8_t bytes_4[4];					// 4 byte buffer for 32bit ints
uint8_t bytes_2[2];					// 2 byte buffer for 16bit ints (12-bit ADC)
volatile uint32_t timerLoad;		// Used to load values into Timer
volatile uint32_t timeMilli;		// Keeps track of the elapsed time in milliseconds
volatile uint8_t nextSection;		// boolean that tells test program to go to next section
volatile uint8_t nextDataBundle;	// boolean that tells test program to gather/send next data bundle

void UARTFlush(void){
    while(UARTCharsAvail(UART0_BASE)){
		UARTCharGetNonBlocking(UART0_BASE);
    	}
}

uint16_t ReadI2CADC(uint8_t selectByte){
	uint16_t I2Cdata;
	uint16_t temp;
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

	// Delay ~2.25 millisecond to allow ADC to catch up and take sample from chosen channel
	ROM_SysCtlDelay(SysCtlClockGet()/1500);

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

	I2Cdata = (uint16_t)dataIn[0];
	I2Cdata = I2Cdata << 8;
	temp = (uint16_t)dataIn[1];
	I2Cdata = I2Cdata + temp;

	return I2Cdata;
}

void GatherData(){
	int i;
	uint16_t I2Cdata = 0;
	uint8_t speed = 0;

	TimerEnable(TIMER1_BASE, TIMER_A);

	timeMilli = (uint32_t)(((WTIMER_BIT_LOAD-TimerValueGet64(WTIMER0_BASE))*1000)/SysCtlClockGet());

	// Time in milliseconds
	bytes_4[0] = (timeMilli >> 24) & 0xFFFF;
	bytes_4[1] = (timeMilli >> 16) & 0xFFFF;
	bytes_4[2] = (timeMilli >> 8) & 0xFFFF;
	bytes_4[3] = timeMilli & 0xFFFF;
	for(i=0;i<4;i++){
	    UARTCharPutNonBlocking(UART0_BASE,bytes_4[i]);
	}
	// Input speed
	speed = (uint8_t)(((PWMPulseWidthGet(PWM1_BASE, PWM_GEN_0)*100)/3125)-100);
	UARTCharPutNonBlocking(UART0_BASE,speed);
	// Frequency
	bytes_4[0] = (SysCtlClockGet() >> 24) & 0xFFFF;
	bytes_4[1] = (SysCtlClockGet() >> 16) & 0xFFFF;
	bytes_4[2] = (SysCtlClockGet() >> 8) & 0xFFFF;
	bytes_4[3] = SysCtlClockGet() & 0xFFFF;
	for(i=0;i<4;i++){
	    UARTCharPutNonBlocking(UART0_BASE,bytes_4[i]);
	}
	// data
	I2Cdata = ReadI2CADC(I2C_VOLT);
	bytes_2[0] = (I2Cdata >> 8) & 0xFF;
	bytes_2[1] = I2Cdata & 0xFF;
	for(i=0;i<2;i++){
		UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
	}
	// data
	I2Cdata = ReadI2CADC(I2C_CURR);
	bytes_2[0] = (I2Cdata >> 8) & 0xFF;
	bytes_2[1] = I2Cdata & 0xFF;
	for(i=0;i<2;i++){
		UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
	}
	// data
	I2Cdata = ReadI2CADC(I2C_THU);
	bytes_2[0] = (I2Cdata >> 8) & 0xFF;
	bytes_2[1] = I2Cdata & 0xFF;
	for(i=0;i<2;i++){
		UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
	}
	// data
	I2Cdata = ReadI2CADC(I2C_TOR);
	bytes_2[0] = (I2Cdata >> 8) & 0xFF;
	bytes_2[1] = I2Cdata & 0xFF;
	for(i=0;i<2;i++){
		UARTCharPutNonBlocking(UART0_BASE,bytes_2[i]);
	}
	while(!nextDataBundle){
	}
	nextDataBundle = false;
}

void TestProgram1(void){	// single-cycle test
	int i = 0;
	uint32_t ct = 0;		// Used to keep track whether or not to increment speed
	// The following values must be numbers from 0-60
	uint8_t startTime;		// Stores start time in seconds
	uint8_t midTime;		// Stores mid time in seconds
	uint8_t endTime;		// Stores end time in seconds
	uint8_t maxSpeed;		// Stores max speed in %
	uint32_t incr[2];		// Fixed point increment of PWMCount that must be added or subtracted (*1000000)
	uint32_t maxPWMcnt;		// Stores maximum PWM count value (sets pulse width and thus motor speed)
	timeMilli = 0;
	nextSection = false;
	uint64_t debug;

	ROM_SysCtlDelay(SysCtlClockGet()/300);

	// Get user input from UART comm and convert to 8-bit unsigned int
	startTime = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
	midTime = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
	endTime = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
	maxSpeed = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);

	// Calculates increment of speed increase and decrease
	// Note, these values are fixed point and must be divided by 1000000
	if(startTime!=0){
		incr[0] = ((uint32_t)maxSpeed*1000000)/((uint32_t)startTime*50);
		incr[0] = (3125*incr[0])/100;
	}
	if(endTime!=0){
		incr[1] = ((uint32_t)maxSpeed*1000000)/((uint32_t)endTime)/50;
		incr[1] = (3125*incr[1])/100;
	}
	// Calculate max PWM count number from maxSpeed
	maxPWMcnt = (3125*((uint32_t)maxSpeed + 100))/100;

	// Set Section timer to 5 seconds (first section is 5 seconds of dead-time)
	timerLoad = (SysCtlClockGet() * 5) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	// Set Test Timer to high value
	TimerLoadSet64(WTIMER0_BASE, WTIMER_BIT_LOAD);

	// Red LED indicates dead-time Section)
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1);

	// Duty Cycle of 5% is 0% speed
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);

	//	Enable timers
	TimerEnable(TIMER0_BASE, TIMER_A);
	TimerEnable(WTIMER0_BASE, TIMER_A);

	//--------Test Begin--------

	//	~~~~~ Section # 1 ~~~~~

	while(!nextSection){
		GatherData();
	}

	//	~~~~~ Section # 2 ~~~~~

	nextSection = false;
	// Blue LED indicates Active Test Section)
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);

	if(startTime!=0){
		timerLoad = (SysCtlClockGet()*startTime)-1;
		TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
		TimerEnable(TIMER0_BASE, TIMER_A);
		while(!nextSection){
			if(i%2==0){
				ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 3125+(ct*incr[0])/1000000);
				ct++;
			}
			GatherData();
			i++;
			debug = PWMPulseWidthGet(PWM1_BASE, PWM_OUT_0);
		}

	}
	ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, maxPWMcnt);

	//	~~~~~ Section # 3 ~~~~~

	nextSection = false;

	if(midTime!=0){
		timerLoad = (SysCtlClockGet()*midTime)-1;
		TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
		TimerEnable(TIMER0_BASE, TIMER_A);
		while(!nextSection){
			GatherData();
		}
	}
	i = 0;
	ct = 0;

	//	~~~~~ Section # 4 ~~~~~

	nextSection = false;

	if(endTime!=0){
		timerLoad = (SysCtlClockGet()*endTime)-1;
		TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
		TimerEnable(TIMER0_BASE, TIMER_A);
		while(!nextSection){
			if(i%2==0){
				ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, maxPWMcnt - (ct*incr[1])/1000000);
				ct++;
			}
			GatherData();
			i++;
		}
	}
	ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);

	//	~~~~~ Section # 5 ~~~~~

	nextSection = false;
	timerLoad = (SysCtlClockGet() * 5) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	TimerEnable(TIMER0_BASE, TIMER_A);

	// Green LED indicates ending Test Section
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);

	while(!nextSection){
		GatherData();
	}

	// Signal end of test
	for(i=0;i<4;i++){
		UARTCharPutNonBlocking(UART0_BASE,'~');
	}
	TimerDisable(WTIMER0_BASE, TIMER_A);
	nextSection = false;
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, false);
	ROM_PWMGenDisable(PWM1_BASE, PWM_GEN_0);
	UARTFlush();
}

void TestProgram2(void){	// multi-cycle test
	int i = 0;
	uint32_t ct = 0;			// Used to keep track whether or not to increment speed
	// The following values must be numbers from 0-60
	uint8_t startTime;		// Stores start time in seconds
	uint8_t midTime;		// Stores mid time in seconds
	uint8_t endTime;		// Stores end time in seconds
	uint8_t maxSpeed;		// Stores max speed in %
	uint8_t numCycles;		// Number of on off sequences
	uint8_t deadTime;		// Wait time between cycles in seconds
	uint32_t incr[2];		// Fixed point increment of PWMCount that must be added or subtracted (*1000000)
	uint32_t maxPWMcnt;		// Stores maximum PWM count value (sets pulse width and thus motor speed)
	timeMilli = 0;

	ROM_SysCtlDelay(SysCtlClockGet()/750);

	// Get user input from UART comm and convert to 8-bit unsigned int
	startTime = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
	midTime = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
	ROM_SysCtlDelay(SysCtlClockGet()/300);
	endTime = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
	maxSpeed = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
	numCycles = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
	deadTime = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);

	// Calculates increment of speed increase and decrease
	// Note, these values are fixed point and must be divided by 1000000
	if(startTime!=0){
		incr[0] = ((uint32_t)maxSpeed*1000000)/((uint32_t)startTime*50);
		incr[0] = (3125*incr[0])/100;
	}
	if(endTime!=0){
		incr[1] = ((uint32_t)maxSpeed*1000000)/((uint32_t)endTime)/50;
		incr[1] = (3125*incr[1])/100;
	}
	// Calculate max PWM count number from maxSpeed
	maxPWMcnt = (3125*((uint32_t)maxSpeed + 100))/100;

	// Set Section timer to 5 seconds (first section is 5 seconds of dead-time)
	timerLoad = (SysCtlClockGet() * 5) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	// Set Test Timer to high value
	TimerLoadSet64(WTIMER0_BASE, WTIMER_BIT_LOAD);

	// Red LED indicates dead-time Section)
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1);

	// Duty Cycle of 5% is 0% speed
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);

	//	Enable timers
	TimerEnable(TIMER0_BASE, TIMER_A);
	TimerEnable(WTIMER0_BASE, TIMER_A);

	//--------Test Begin--------

	//	~~~~~ Section # 1 ~~~~~
	//	5 second test start delay

	while(!nextSection){
		GatherData();
	}

	for(i=0;i<numCycles;i++){

		//	~~~~~ Section # 2 ~~~~~
		//	Motor Rise Time

		nextSection = false;
		// Blue LED indicates Active Test Section)
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);

		if(startTime!=0){
			timerLoad = (SysCtlClockGet()*startTime)-1;
			TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
			TimerEnable(TIMER0_BASE, TIMER_A);
			while(!nextSection){
				if(i%2==0){
					ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 3125+(ct*incr[0])/1000000);
					ct++;
				}
				GatherData();
				i++;
			}
		}
		ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, maxPWMcnt);

		//	~~~~~ Section # 3 ~~~~~
		//	Motor Hold Time

		nextSection = false;

		if(midTime!=0){
			timerLoad = (SysCtlClockGet()*midTime)-1;
			TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
			TimerEnable(TIMER0_BASE, TIMER_A);
			while(!nextSection){
				GatherData();
			}
		}
		i = 0;
		ct = 0;

		//	~~~~~ Section # 4 ~~~~~
		//	Motor Decline Time

		nextSection = false;

		if(endTime!=0){
			timerLoad = (SysCtlClockGet()*endTime)-1;
			TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
			TimerEnable(TIMER0_BASE, TIMER_A);
			while(!nextSection){
				if(i%2==0){
					ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, maxPWMcnt - (ct*incr[1])/1000000);
					ct++;
				}
				GatherData();
				i++;
			}
		}
		ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);

		//	~~~~~ Section # 5 ~~~~~
		//	Motor Off Time

		nextSection = false;
		timerLoad = (SysCtlClockGet() * deadTime) - 1;
		TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
		TimerEnable(TIMER0_BASE, TIMER_A);

		// Green LED indicates ending Test Section
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);

		while(!nextSection){
			GatherData();
		}

	}
	// Signal end of test
	for(i=0;i<4;i++){
		UARTCharPutNonBlocking(UART0_BASE,'~');
	}
	TimerDisable(WTIMER0_BASE, TIMER_A);
	nextSection = false;
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, false);
	ROM_PWMGenDisable(PWM1_BASE, PWM_GEN_0);
	UARTFlush();
}

void TestProgram3(void){	// Premade "Normal Operation Test"
	int i;
	timeMilli = 0;

	// 	Set timer to 5 seconds
	timerLoad = (SysCtlClockGet() * 5) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	// Set Test Timer to high value
	TimerLoadSet64(WTIMER0_BASE, WTIMER_BIT_LOAD);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1);

    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);
    //	Enable PWM Signal
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);

	//	Enable timer
	TimerEnable(TIMER0_BASE, TIMER_A);
	TimerEnable(WTIMER0_BASE, TIMER_A);

	//--------Test Begin--------

	/*	~~~~~ Section # 1 ~~~~~
	 *
	 *	Lasts for 5 seconds. Runs motor at 0% speed.
	 *
	 */

	while(!nextSection){
		GatherData();
	}

	/*	~~~~~ Section # 2 ~~~~~
	 *
	 *	Lasts for 100 seconds. Runs motor from 0 - 90% speed.
	 *
	 */
	timerLoad = (SysCtlClockGet()*5)-1;
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);

	for(i=0;i<10;i++){
		ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 3125+(i*313));
		ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
		ROM_TimerEnable(TIMER0_BASE, TIMER_A);
		nextSection = false;

		while(!nextSection){
			GatherData();
			}
	}

	/*	~~~~~ Section # 3 ~~~~~
	 *
	 *	Lasts for 10 seconds. Runs motor down to 0% speed.
	 *
	 */

	// 	Set timer to 5 seconds
	timerLoad = (SysCtlClockGet() * 1) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	TimerEnable(TIMER0_BASE, TIMER_A);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);

	for(i=0;i<9;i++){
		ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 5937-(i*313));
		ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
		ROM_TimerEnable(TIMER0_BASE, TIMER_A);
		nextSection = false;

		while(!nextSection){
			GatherData();
			}
	}

	ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);
	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	ROM_TimerEnable(TIMER0_BASE, TIMER_A);
	nextSection = false;

	while(!nextSection){
		GatherData();
		}

	nextSection = false;

	// Signal end of test
	for(i=0;i<4;i++){
		UARTCharPutNonBlocking(UART0_BASE,'~');
	}
	TimerDisable(WTIMER0_BASE, TIMER_A);
	nextSection = false;
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, false);
	ROM_PWMGenDisable(PWM1_BASE, PWM_GEN_0);
	UARTFlush();
}

void CalibrationProgram(void){
	int i;
	timeMilli = 0;

	// 	Set timer to 5 seconds
	timerLoad = (SysCtlClockGet() * 5) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	// Set Test Timer to high value
	TimerLoadSet64(WTIMER0_BASE, WTIMER_BIT_LOAD);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1);

    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);
    //	Enable PWM Signal
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);

	//	Enable timers
	TimerEnable(TIMER0_BASE, TIMER_A);
	TimerEnable(WTIMER0_BASE, TIMER_A);

	//--------Test Begin--------

	/*	~~~~~ Section # 1 ~~~~~
	 *
	 *	Lasts for 5 seconds. Runs motor at 0% speed.
	 *
	 */

	while(!nextSection){
		GatherData();
	}

	/*	~~~~~ Section # 2 ~~~~~
	 *
	 *	Lasts for 20 seconds. Runs motor at 50% speed.
	 *
	 */

	//	Set PWM Signal to 1.5 ms (50% speed)
	ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 4688);

	// 	Set timer to 10 seconds
	timerLoad = (SysCtlClockGet() * 10) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	TimerEnable(TIMER0_BASE, TIMER_A);

	//	Disable continue to next section
	nextSection = false;

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);

	while(!nextSection){
		GatherData();
	}

	/*	~~~~~ Section # 3 ~~~~~
	 *
	 *	Lasts for 5 seconds. Runs motor at 0% speed.
	 *
	 */

	//	Set PWM Signal to 1 ms 0% speed)
	ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, DUTY_CYCLE_5_PER);

	// 	Set timer to 5 seconds
	timerLoad = (SysCtlClockGet() * 5) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);
	TimerEnable(TIMER0_BASE, TIMER_A);

	//	Disable continue to next section
	nextSection = false;

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);

	while(!nextSection){
		GatherData();
	}

	// Signal end of test
	for(i=0;i<4;i++){
		UARTCharPutNonBlocking(UART0_BASE,'~');
	}
	TimerDisable(WTIMER0_BASE, TIMER_A);
	nextSection = false;
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, false);
	ROM_PWMGenDisable(PWM1_BASE, PWM_GEN_0);
	UARTFlush();
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
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);	// Time Elapsed value control
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);	// Communication with PC
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);		// PWM control to ESC
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);		// Serial comm with 4-channel ADC
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);	// where UART is
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);	// where I2C is
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);	// where PWM is
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
	// Configure Timer which will be used to record elapsed time information
	ROM_TimerConfigure(WTIMER0_BASE, TIMER_CFG_ONE_SHOT);

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

	Config();

	TimerLoadSet(TIMER0_BASE, TIMER_A, timerLoad);

	// Set Data Bundle timer to ~10 ms (sends data bundle every 10ms)
	timerLoad = (SysCtlClockGet()/100) - 100;
	TimerLoadSet(TIMER1_BASE, TIMER_A, timerLoad);

	// Wait until comm is established with test terminal. Loops twice because there is an
    // initial char sent to the UART RX FIFO when you plug in the USB.
    for(i=0;i<2;i++){
		while(!UARTCharsAvail(UART0_BASE)){
			// Blinks blue 2 times every half second waiting for comm with terminal
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
			ROM_SysCtlDelay(SysCtlClockGet() / (1000 * 3));		// ~ 1msec
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
			ROM_SysCtlDelay(SysCtlClockGet() / (10 * 3));
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
			ROM_SysCtlDelay(SysCtlClockGet() / (1000 * 3));		// ~ 1msec
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
			ROM_SysCtlDelay(SysCtlClockGet()/12);
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
    				CalibrationProgram();
    				break;
    			}
    			default:
    				//	Tell testing terminal that the selection was invalid
    		}
    	}
		// Turn on the LED
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
		// Delay for a bit
		ROM_SysCtlDelay(SysCtlClockGet()/12);
		// Cycle through Red, Green and Blue LEDs
		if (ui8LED == 8) {ui8LED = 2;} else {ui8LED = ui8LED*2;}
    }
}
