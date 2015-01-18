#include <stdint.h>
#include <stdbool.h>

#include "stdio.h"

#include <time.h>
//#include "inc/tm4c123gh6pm.h"
// For Tivaware 2.1.0.12573
#include "inc/tm4c123gh6pge.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
//#include "inc/hw_ints.h"

#include "utils/ustdlib.h"

#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/udma.h"

#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/pushbutton.h"

#include "drivers/LCDBP320x240x16_SSD1289.h"
#include "drivers/LCDBPV2_backlight.h"
#include "drivers/LCDBPV2_touch.h"

//widget example
extern tCanvasWidget g_sBackground;
extern tPushButtonWidget g_sPushBtn;

// *****************************************************************************
//
// The DMA control structure table.
//
// *****************************************************************************
#ifdef ewarm
#pragma data_alignment=1024
tDMAControlTable sDMAControlTable[64];
#elif defined(ccs)
//#pragma DATA_ALIGN(sDMAControlTable, 1024)
tDMAControlTable sDMAControlTable[64];
#else
tDMAControlTable sDMAControlTable[64] __attribute__ ((aligned(1024)));
#endif

void makeCal();
void OnButtonPress(tWidget *pWidget);

Canvas(g_sHeading, &g_sBackground, 0, &g_sPushBtn,
		&g_sLCDBP320x240x16_SSD1289, 0, 0, 320, 23,
		(CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT),
		ClrBlack, ClrWhite, ClrRed, g_psFontCm20, "LED Control", 0, 0);

Canvas(g_sBackground, WIDGET_ROOT, 0, &g_sHeading,
		&g_sLCDBP320x240x16_SSD1289, 0, 23, 320, (240 - 23),
		CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sPushBtn, &g_sHeading, 0, 0,
		&g_sLCDBP320x240x16_SSD1289, 60, 60, 200, 40,
		(PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
				PB_STYLE_FILL), ClrGray, ClrWhite, ClrRed, ClrRed,
				g_psFontCmss22b, "Toggle red LED", 0, 0, 0, 0, OnButtonPress);

bool g_RedLedOn = false;
void OnButtonPress(tWidget *pWidget)
{
    g_RedLedOn = !g_RedLedOn;

    if(g_RedLedOn)
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02);
    }
    else
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00);
    }
}
//!widget example

extern const uint8_t g_pui8Image[];
tContext sContext;
tRectangle sRect;

void ClrScreen(int color);

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//tDMAControlTable sDMAControlTable[64];

//*****************************************************************************
//
// Main 'C' Language entry point.  Toggle an LED using TivaWare.
// See http://www.ti.com/tm4c123g-launchpad/project0 for more information and
// tutorial videos.
//
//*****************************************************************************
int
main(void)
{
	FPUEnable();
	FPULazyStackingEnable();
    //
    // Setup the system clock to run at 50 Mhz from PLL with crystal reference
    //
    SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    /// Configure and enable uDMA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    SysCtlDelay(10);
    uDMAControlBaseSet(&sDMAControlTable[0]);
    uDMAEnable();

    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    /*
    //Enable Pin A2 (buzzer)

    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);
    //Enable Pin F4 (vibrator)

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);
    */

    // ******Enable I2C


    GPIOPinTypeI2CSCL(GPIO_PORTE_BASE, GPIO_PIN_4); //  special I2CSCL treatment for M4F devices
    GPIOPinTypeI2C(GPIO_PORTE_BASE, GPIO_PIN_5);
    GPIOPinConfigure(GPIO_PE4_I2C2SCL);
    GPIOPinConfigure(GPIO_PE5_I2C2SDA);
    SysCtlPeripheralEnable( SYSCTL_PERIPH_I2C2);
    I2CMasterInitExpClk( I2C2_MASTER_BASE, SysCtlClockGet(), false);
    SysCtlDelay(100);

    // enable led pins
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

    //enable LCDBPV2
    LCDBPV2_backlight_init();
    LCDBP320x240x16_SSD1289Init();
    LCDBPV2_touch_Init();

    SysCtlDelay(SysCtlClockGet()/8);

    // *******Enable Interrupts on E0
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0);
    //pullup GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_BOTH_EDGES);//GPIO_FALLING_EDGE GPIO_LOW_LEVEL GPIO_DISCRETE_INT GPIO_RISING_EDGE GPIO_BOTH_EDGES
    //IntPrioritySet(INT_GPIO, 0x00);

    unsigned long ulPeriod;
    ulPeriod=(SysCtlClockGet()/2500 ); //0.04*ulClockMS 1000000 800=0.25ms 1500=0'42ms 3000=91uS 2500=0'152ms
    //setup timer0 (secuential touch read interrupt) avoid rebounds
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); //TIMER_CFG_PERIODIC TIMER_CFG_32_BIT_PER
    TimerLoadSet(TIMER0_BASE, TIMER_A,ulPeriod);
    SysCtlDelay(SysCtlClockGet()/4); //needed to avoid faultISR
    IntEnable(INT_TIMER0A);

    IntMasterEnable();

    //enable interrupts on GPIOE0 (touch interrupts)
    IntEnable(INT_GPIOE);
    GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_0);
    GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_0);

    //Enable Timer 0
    TimerEnable(TIMER0_BASE, TIMER_A);
    //TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

 	SysCtlDelay(SysCtlClockGet()/4);

 	GrContextInit(&sContext, &g_sLCDBP320x240x16_SSD1289);

 	//draw "LCDBPV2" text
 	ClrScreen(ClrBlack);
    GrContextFontSet(&sContext, &g_sFontCm30b);
    GrContextForegroundSet(&sContext, ClrRed);
	GrStringDrawCentered(&sContext, "LCDBPV2", -1,GrContextDpyWidthGet(&sContext) / 2, 80, 0);
    SysCtlDelay(SysCtlClockGet());

    //draw image
    GrImageDraw(&sContext, g_pui8Image, 0, 0);
    GrFlush(&sContext);
    SysCtlDelay(SysCtlClockGet()/4);

    //draw banner
    ClrScreen(ClrBlack);
    sRect.i16XMin = 1;
    sRect.i16YMin = 1;
    sRect.i16XMax = 318;
    sRect.i16YMax = 238;
    GrContextForegroundSet(&sContext, ClrRed);
    GrContextFontSet(&sContext, &g_sFontCmss30b);
    GrStringDraw(&sContext, "Texas", -1, 110, 2, 0);
    GrStringDraw(&sContext, "Instruments", -1, 80, 32, 0);
    GrStringDraw(&sContext, "Graphics", -1, 100, 62, 0);
    GrStringDraw(&sContext, "Lab", -1, 135, 92, 0);
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sRect);
    GrFlush(&sContext);
    SysCtlDelay(SysCtlClockGet()/2);

    //white screen & led
    ClrScreen(ClrWhite);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED|BLUE_LED|GREEN_LED);
    SysCtlDelay(SysCtlClockGet()/2);
    //red screen & led
    ClrScreen(ClrRed);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);
    SysCtlDelay(SysCtlClockGet()/2);
    //green screen & led
    ClrScreen(ClrGreen);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    SysCtlDelay(SysCtlClockGet()/2);
    //blue screen & led
    ClrScreen(ClrBlue);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
    SysCtlDelay(SysCtlClockGet()/2);
    //black screen & led off
    ClrScreen(ClrBlack);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, 0);

    /*
     * To Calibrate screen you need to use makeCal() function to get
     * "A", "B", "C", "D", "E", "F" and "Div" parameters for LCDBPV2_loadparm() function.
     * Once this is done you only need to call LCDBPV2_loadparm();
     */

    //run touch calibration and get parameters for LCDBPV2_loadparm()
    //makeCal();
    //load stored calibration data
    LCDBPV2_loadparm();


    //widget example
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sBackground);
    WidgetPaint(WIDGET_ROOT);

    while(1)
    {
    	WidgetMessageQueueProcess();
    }
}

void ClrScreen(int color)
{
   sRect.i16XMin = 0;
   sRect.i16YMin = 0;
   sRect.i16XMax = 319;
   sRect.i16YMax = 239;
   GrContextForegroundSet(&sContext, color);
   GrRectFill(&sContext, &sRect);
   GrFlush(&sContext);
}

void Timer0IntHandler(void)
{
	touchprocess();
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void makeCal()
{
	// Paint touch calibration targets and collect calibration data
	GrContextForegroundSet(&sContext,ClrWhite);
	GrContextBackgroundSet(&sContext,ClrBlack);
	GrContextFontSet(&sContext,&g_sFontCm20);

	//StringDraw("Touch center of circles to calibrate", -1, 0, 0, 1);
	GrCircleDraw(&sContext,32, 24, 10);
	TouchScreenCalibrationPoint(32, 24, 0);

	GrCircleDraw(&sContext,280, 200, 10);
	TouchScreenCalibrationPoint(280, 200, 1);

	GrCircleDraw(&sContext,200, 40, 10);
	TouchScreenCalibrationPoint(200, 40, 2);

	// Calculate and set calibration matrix
	long* plCalibrationMatrix = LCDBPV2_TouchScreenCalibrate();

	GrContextForegroundSet(&sContext,ClrWhite);
	GrContextBackgroundSet(&sContext,ClrBlack);

	char pcStringBuf[20];
	usprintf(pcStringBuf, "A %d", plCalibrationMatrix[0]);
	GrStringDraw(&sContext,pcStringBuf, -1, 0, 20, 1);
	usprintf(pcStringBuf, "B %d", plCalibrationMatrix[1]);
	GrStringDraw(&sContext,pcStringBuf, -1, 0, 40, 1);
	usprintf(pcStringBuf, "C %d", plCalibrationMatrix[2]);
	GrStringDraw(&sContext,pcStringBuf, -1, 0, 60, 1);
	usprintf(pcStringBuf, "D %d", plCalibrationMatrix[3]);
	GrStringDraw(&sContext,pcStringBuf, -1, 0, 80, 1);
	usprintf(pcStringBuf, "E %d", plCalibrationMatrix[4]);
	GrStringDraw(&sContext,pcStringBuf, -1, 0, 100, 1);
	usprintf(pcStringBuf, "F %d", plCalibrationMatrix[5]);
	GrStringDraw(&sContext,pcStringBuf, -1, 0, 120, 1);
	usprintf(pcStringBuf, "Div %d", plCalibrationMatrix[6]);
	GrStringDraw(&sContext,pcStringBuf, -1, 0, 140, 1);
	TouchScreenCalibrationPoint(0,0,0);	// wait for dummy touch
	 ClrScreen(ClrBlack);
}
