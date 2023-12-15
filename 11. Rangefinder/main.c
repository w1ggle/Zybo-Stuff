//need .elf boot.bin and system.bif for SD card portabililty

#include "xparameters.h"

#include "xgpio.h"

#include "xscugic.h"

#include "xil_exception.h"

#include "xil_printf.h"
#include <stdio.h>
#include "PmodOLED.h"
#include "sleep.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "PmodMAXSONAR.h"


//global vars

#define PMOD_MAXSONAR_BASEADDR XPAR_PMODMAXSONAR_1_AXI_LITE_GPIO_BASEADDR

#define CLK_FREQ 100000000 // FCLK0 frequency not found in xparameters.h

#define INTC_DEVICE_ID 		XPAR_PS7_SCUGIC_0_DEVICE_ID

#define BTNS_DEVICE_ID		XPAR_AXI_GPIO_0_DEVICE_ID

#define LEDS_DEVICE_ID		XPAR_AXI_GPIO_1_DEVICE_ID

#define INTC_GPIO_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR

#define BTN_INT 			XGPIO_IR_CH1_MASK



PmodOLED myOLED;

PmodMAXSONAR mySONAR;

XGpio LEDInst, BTNInst;

XScuGic INTCInst;


char *units = "inches";
int *led_threshholds;

int state = 1;



static void BTN_Intr_Handler(void *baseaddr_p);

static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);

static int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr);
int DemoInitialize();
void DemoRun();



int main (void)

{



  int status;



  status = DemoInitialize();
  DemoRun();



  return status;

}


int DemoInitialize() {




   // To change between PmodOLED and OnBoardOLED is to change Orientation

   const u8 orientation = 0x0; // Set up for Normal PmodOLED(false) vs normal

                               // Onboard OLED(true)

   const u8 invert = 0x0; // true = whitebackground/black letters

                          // false = black background /white letters


   OLED_Begin(&myOLED, XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR, XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR, orientation, invert);
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

void DemoRun() {

	OLED_PutString(&myOLED, "Default = inches\npress button = switch units"); //startup screen
    usleep(5000000); //give user 5 seconds to read it


   u32 dist;

   u8 lastData = 0;

   u8 led_data;

   char *measured;


   while (1) {
      OLED_SetCharUpdate(&myOLED, 0); //pause screen

      OLED_ClearBuffer(&myOLED);	//clear it
      OLED_SetCursor(&myOLED, 0, 0); //move cursor to top left corner

      dist = MAXSONAR_getDistance(&mySONAR) * 2; //maxsonar gets data (inches) //multiply by 2 as per profs suggestion, get 0-125 but expecting 0-255 (*2 is close enough scaler)



      if (strcmp(units, "feet") == 0){ //if user selected feet, divide by 12
    	  dist = dist / 12;
      }



      dist = (dist + lastData) / 2; //averaging out previous number and new number

      lastData = dist; //store old number for averaging next iteration

      if(dist < led_threshholds[0]){ //set leds, if close 1 led, far is 4 leds
    	  led_data = 0;
      }else if(dist < led_threshholds[1]){
    	  led_data = 1;
      }else if (dist < led_threshholds[2]){
    	  led_data = 3;
      }else if (dist < led_threshholds[3]){
    	  led_data = 7;
      }else //to max or 255 in or 21 ft
    	  led_data = 15;
      XGpio_DiscreteWrite(&LEDInst, 1, led_data);

      sprintf(measured, "%lu ", dist); //convert int from sonar to string
      strcat(measured, units);  //cat strings

      OLED_PutString(&myOLED, measured); // output string (distance and unit)
      OLED_Update(&myOLED); //update screen
      //usleep(100000); // delay (if not program crashes bc it polls too fast lol)

   }
}




void BTN_Intr_Handler(void *InstancePtr)

{

	// Disable GPIO interrupts

	XGpio_InterruptDisable(&BTNInst, BTN_INT);

	// Ignore additional button presses

	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) != BTN_INT) {

			return;

		}



	int in[4] = {4, 64, 128, 196};

	int ft[4] = {1, 5, 10, 15};



	if (XGpio_DiscreteRead(&BTNInst, 1) > 0){

		if (state == 1) {

			units = "feet";

			led_threshholds = ft;

			state = 0;

		}

		else if (state == 0) {

			units = "inches";

			led_threshholds = in;

			state = 1;

		}

	}



    (void)XGpio_InterruptClear(&BTNInst, BTN_INT);

    // Enable GPIO interrupts

    XGpio_InterruptEnable(&BTNInst, BTN_INT);

}



int InterruptSystemSetup(XScuGic *XScuGicInstancePtr)

{

	// Enable interrupt

	XGpio_InterruptEnable(&BTNInst, BTN_INT);

	XGpio_InterruptGlobalEnable(&BTNInst);



	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, XScuGicInstancePtr);

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

	status = XScuGic_Connect(&INTCInst, INTC_GPIO_INTERRUPT_ID, (Xil_ExceptionHandler)BTN_Intr_Handler, (void *)GpioInstancePtr);

	if(status != XST_SUCCESS) return XST_FAILURE;



	// Enable GPIO interrupts interrupt

	XGpio_InterruptEnable(GpioInstancePtr, 1);

	XGpio_InterruptGlobalEnable(GpioInstancePtr);



	// Enable GPIO and timer interrupts in the controller

	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);

	

	return XST_SUCCESS;

}



