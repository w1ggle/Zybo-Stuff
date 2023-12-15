/*
 * interrupt_counter_tut_2B.c
 *
 *  Created on: 	Unknown
 *      Author: 	Ross Elliot
 *     Version:		1.1
 */

/********************************************************************************************

* VERSION HISTORY
********************************************************************************************
* 	v1.1 - 01/05/2015
* 		Updated for Zybo ~ DN
*
*	v1.0 - Unknown
*		First version created.
*******************************************************************************************/

#include "xparameters.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"

#include <stdio.h>
#include "PmodOLED.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xparameters.h"

#include "PmodMAXSONAR.h"

#define PMOD_MAXSONAR_BASEADDR XPAR_PMODMAXSONAR_1_AXI_LITE_GPIO_BASEADDR

#ifdef __MICROBLAZE__
#define CLK_FREQ XPAR_CPU_M_AXI_DP_FREQ_HZ
#else
#define CLK_FREQ 100000000 // FCLK0 frequency not found in xparameters.h
#endif

/* ------------------------------------------------------------ */
/*                  Global Variables                            */
/* ------------------------------------------------------------ */

PmodOLED myDevice;
PmodMAXSONAR mySONAR;

// Parameter definitions
#define INTC_DEVICE_ID 		XPAR_PS7_SCUGIC_0_DEVICE_ID
#define BTNS_DEVICE_ID		XPAR_AXI_GPIO_0_DEVICE_ID
#define LEDS_DEVICE_ID		XPAR_AXI_GPIO_1_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR

#define BTN_INT 			XGPIO_IR_CH1_MASK

XGpio LEDInst, BTNInst;
XScuGic INTCInst;
static int led_data;
//static int btn_value;

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------
static void BTN_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr);

int DemoInitialize();
void DemoRun();
void DemoCleanup();
void EnableCaches();
void DisableCaches();

// To change between PmodOLED and OnBoardOLED is to change Orientation
const u8 orientation = 0x0; // Set up for Normal PmodOLED(false) vs normal
                            // Onboard OLED(true)
const u8 invert = 0x0; // true = whitebackground/black letters
                       // false = black background /white letters

//----------------------------------------------------
// INTERRUPT HANDLER FUNCTIONS
// - called by the timer, button interrupt, performs
// - LED flashing
//----------------------------------------------------

char *units;
//char c = inbyte();
//if (c == 'f')

//else
//	   units = "inches";

int counter = 1;

void BTN_Intr_Handler(void *InstancePtr)
{
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BTNInst, BTN_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) !=
			BTN_INT) {
			return;
		}
	//btn_value = XGpio_DiscreteRead(&BTNInst, 1);
	// Increment counter based on button value


	if (XGpio_DiscreteRead(&BTNInst, 1) > 0){
		if (counter == 1) {
			units = "feet";

			counter = 0;
		}
		else if (counter == 0) {
			units = "inches";
			counter = 1;
		}
	}



    (void)XGpio_InterruptClear(&BTNInst, BTN_INT);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNInst, BTN_INT);
}

//----------------------------------------------------
// MAIN FUNCTION
//----------------------------------------------------
int main (void)
{





  DemoInitialize();
  DemoRun();
  DemoCleanup();

  return 0;
}

int DemoInitialize() {
   EnableCaches();
   OLED_Begin(&myDevice, XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR,
         XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR, orientation, invert);
   MAXSONAR_begin(&mySONAR, PMOD_MAXSONAR_BASEADDR, CLK_FREQ);

   int status;
   //----------------------------------------------------
   // INITIALIZE THE PERIPHERALS & SET DIRECTIONS OF GPIO
   //----------------------------------------------------
   // Initialise LEDs
   status = XGpio_Initialize(&LEDInst, LEDS_DEVICE_ID);
   if(status != XST_SUCCESS) return XST_FAILURE;
   // Initialise Push Buttons
   status = XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
   if(status != XST_SUCCESS) return XST_FAILURE;
   // Set LEDs direction to outputs
   XGpio_SetDataDirection(&LEDInst, 1, 0x00);
   // Set all buttons direction to inputs
   XGpio_SetDataDirection(&BTNInst, 1, 0xFF);

   // Initialize interrupt controller
   status = IntcInitFunction(INTC_DEVICE_ID, &BTNInst);
   if(status != XST_SUCCESS) return XST_FAILURE;

   return status;
}

/* ------------------------------------------------------------ */
/*** DemoRun()
**
**   Parameters:
**      none
**
**   Return Value:
**      none
**
**   Errors:
**      If the demo is shut down without properly exiting, does not reinitialize
**      properly.
**
**   Description:
**      Displays Demo message and each available Fill Pattern.
**      Pauses between runs to check if user wants to continue, if not, exits.
**      To be safe, exit through prompt before closing demo.
**      Requires UART connection to terminal program on PC.
*/


int in[] = {25, 50, 75, 100};
int ft[] = {5, 10, 15, 20};

int led_threshholds[];
void DemoRun() {

   //make a unit selections
    OLED_SetCharUpdate(&myDevice, 0);
    OLED_ClearBuffer(&myDevice);
    OLED_SetCursor(&myDevice, 0, 0); //top left corner
	OLED_PutString(&myDevice, "Default = inches\npress button = switch units");
    OLED_Update(&myDevice); //update
    usleep(10000000); //delay?
   units = "inches";


   //outputting to display
   char *measure;
   u32 dist;
   while (1) {
      OLED_SetCharUpdate(&myDevice, 0);
      OLED_ClearBuffer(&myDevice);
      OLED_SetCursor(&myDevice, 0, 0); //top left corner

      dist = MAXSONAR_getDistance(&mySONAR);

      if (strcmp(units, "feet") == 0){
    	  dist = dist / 12;
      }




      if(dist < 4){
    	  led_data = 0;
      }
      else if(dist < 64){
    	  led_data = 1;
      } else if (dist < 128){
    	  led_data = 3;
      }else if (dist < 196){
    	  led_data = 7;
      }else //256
    	  led_data = 15;


      XGpio_DiscreteWrite(&LEDInst, 1, led_data);

      sprintf(measure, "%lu ", dist); //convert int to string
      strcat(measure, units);  //cat strings

      OLED_PutString(&myDevice, measure); // output distance and unit
      OLED_Update(&myDevice); //update
      usleep(100000); //delay?

   }

}

void DemoCleanup() {
   OLED_End(&myDevice);
   DisableCaches();
}

void EnableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheEnable();
#endif
#endif
}

void DisableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheDisable();
#endif
#endif
}


//----------------------------------------------------
// INITIAL SETUP FUNCTIONS
//----------------------------------------------------

int InterruptSystemSetup(XScuGic *XScuGicInstancePtr)
{
	// Enable interrupt
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
	XGpio_InterruptGlobalEnable(&BTNInst);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			 	 	 	 	 	 XScuGicInstancePtr);
	Xil_ExceptionEnable();


	return XST_SUCCESS;

}

int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialisation
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;
	
	// Connect GPIO interrupt to handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_GPIO_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)BTN_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Enable GPIO interrupts interrupt
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// Enable GPIO and timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);
	
	return XST_SUCCESS;
}


