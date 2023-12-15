/******************************************************************************/
/*                                                                            */
/* main.c -- Demo project for the PmodOLED IP                                 */
/*                                                                            */
/******************************************************************************/
/* Author: Arthur Brown                                                       */
/* Copyright 2016, Digilent Inc.                                              */
/******************************************************************************/
/* File Description:                                                          */
/*                                                                            */
/* This demo_project initializes and uses the PmodOLED to display strings     */
/* and show different available fill patterns.                                */
/*                                                                            */
/* In order to properly quit the demo, a serial terminal application should   */
/* be connected to your board over UART at the appropriate Baud rate, as      */
/* specified below.                                                           */
/*                                                                            */
/******************************************************************************/
/* Revision History:                                                          */
/*                                                                            */
/*    06/20/2016(ArtVVB):   Created                                           */
/*    12/15/2016(jPeyron):  Edited for better use for OnboardOLED in, as well */
/*                          as inverting the white and black                  */
/*    08/25/2017(ArtVVB):   Added proper cache management functions           */
/*    02/17/2018(atangzwj): Validated for Vivado 2017.4                       */
/*                                                                            */
/******************************************************************************/
/* Baud Rates:                                                                */
/*                                                                            */
/*    Microblaze: 9600 or what was specified in UARTlite core                 */
/*    Zynq: 115200                                                            */
/*                                                                            */
/******************************************************************************/

/* ------------------------------------------------------------ */
/*                  Include File Definitions                    */
/* ------------------------------------------------------------ */

#include <stdio.h>
#include "PmodOLED.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xparameters.h"

#include "PmodMAXSONAR.h"

/************ Macro Definitions ************/

#define PMOD_MAXSONAR_BASEADDR XPAR_PMODMAXSONAR_0_AXI_LITE_GPIO_BASEADDR

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

/* ------------------------------------------------------------ */
/*                  Forward Declarations                        */
/* ------------------------------------------------------------ */

void DemoInitialize();
void DemoRun();
void DemoCleanup();
void EnableCaches();
void DisableCaches();

// To change between PmodOLED and OnBoardOLED is to change Orientation
const u8 orientation = 0x0; // Set up for Normal PmodOLED(false) vs normal
                            // Onboard OLED(true)
const u8 invert = 0x0; // true = whitebackground/black letters
                       // false = black background /white letters

/* ------------------------------------------------------------ */
/*                  Function Definitions                        */
/* ------------------------------------------------------------ */

int main() {
   DemoInitialize();
   DemoRun();
   DemoCleanup();

   return 0;
}

void DemoInitialize() {
   EnableCaches();
   OLED_Begin(&myDevice, XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR,
         XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR, orientation, invert);
   MAXSONAR_begin(&mySONAR, PMOD_MAXSONAR_BASEADDR, CLK_FREQ);
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
void DemoRun() {

   //make a unit selections
   xil_printf("f for feet or anything else for inches:\n\r");
   char c = inbyte();
   char *units;
   if (c == 'f')
	   units = "feet";
   else
	   units = "inches";

   //outputting to display
   char measure;
   u32 dist;
   while (1) {
      OLED_SetCharUpdate(&myDevice, 0);
      OLED_ClearBuffer(&myDevice);
      OLED_SetCursor(&myDevice, 0, 0); //top left corner

      dist = 3;//dist = MAXSONAR_getDistance(&mySONAR);
      sprintf(measure, "%d ", dist); //convert int to string
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

