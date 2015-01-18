/*
 * LCDBPV2_touch.c
 *
 *  Created on: 27/12/2013
 *      Author: Dani
 */



#include "LCDBPV2_touch.h"


#define I2C_M_BASE  I2C2_MASTER_BASE //I2C1_MASTER_BASE

//Function that handles Timer0 Interrupt.
//
//It is called every ulPeriod period after being activated by touch_IntHandler() (LCDBPV2_touch.c) when stylus press the screen.
//Timer0 Interrupt is disabled when stylus leaves the screen.
//// TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod); sets period to the new ulPeriod. (default ulPeriod=SysCtlClockGet()/800)
//void Timer0IntHandler(void);
//void LCDBPV2_touch_ConfigI2C();
int ReceiveI2C(int op);
//
static unsigned long WaitI2CDone( unsigned int long ulBase);
//send a command to touch driver and receives and acknowledge (x,y,z coordenates)
int LCDBPV2_touch_receiveI2C(int op);
//stores X,Y,Z1,Z2 and Rtouch data.
int tX,tY,tZ1,tZ2,tRtouch;
//stores uncalibrated data
int tYc, tXc;
//ask touch data and calibrates this. Stores X on tX, Y on tY, Z1 on tZ1, Z2 on tZ2 and Rtouch on tRtouch.
void gettouch();
//process events to be executed every touch
void touchprocess();
//flag that stores if touch driver enter into interrupt for the first time after falling edge or not.
int fldown;
//flag that avoid to recalibrate the same data repeatedly
int flcc;

//stores the last state of the touch driver: touchstate=1-> unpressed, touchstate=0-> pressed.
int touchstate;

//*****************************************************************************
//
// A pointer to the function to receive messages from the touch screen driver
// when events occur on the touch screen (debounced presses, movement while
// pressed, and debounced releases).
//
//*****************************************************************************
//static int32_t (*g_pfnTSHandler)(uint32_t ulMessage, int32_t lX, int32_t lY);

//function recalculates coordenates (raw data) with calibrated data.
void LCDBPV2_calcParam();
//flag to indicate I2C error (timeout). Timer 1 needs to be enabled. [DISABLED]
int I2Cwd;
//Stores previous X and Y data
int xbk,ybk;

//create a new type of variable that stores functions.
//typedef void (*functiontype2)();

//stores events (functions) that can be called when touch
functiontype2 touchevents[100];
//send function pointer to be stored into an event to be called when touch.
void addevent(functiontype2 event1);
//count the number of events (functions) to be executed when touch. This variable increases when add a new event.
int tevents;

//set locked or unlocked mode.
//*When lock=0, events stored in touchevents[] array would be executed only one time when touch is pressed.
//*When lock=1, events stored in touchevents[] array would be executed in a loop every timer0 interrupt when touch is pressed.
//int lock;

//*****************************************************************************
//
// A pointer to the current touchscreen calibration parameter set.
//
//*****************************************************************************
const long *g_plParmSet;
long ret[7];
volatile unsigned char g_cCalibrationMode;

unsigned short g_psCalLCD[3][2];	// 3x x/y LCD
unsigned short g_psCalRAW[3][2];	// 3x x/y RAW

long g_plCalibrationMatrix[7];	// 2x3 matrix and divider calculated
///////////////////


//Init of the touch driver. User must call this function to enable touch.
void LCDBPV2_touch_Init()
{
	//**configure touch

	//set address
	I2CMasterSlaveAddrSet( I2C2_MASTER_BASE, 0x48, false);
	//send command byte
	//****SETUP COMMAND: 1011+00+0(MAV filter)+1(1=90k, 0=50k IRQ PULLUP RESISTOR)
	I2CMasterDataPut( I2C2_MASTER_BASE, 0xB0);
	I2CMasterControl( I2C2_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);
	WaitI2CDone( I2C2_MASTER_BASE);

	//Activate Y+, X-drivers: 1010+00(Power down between cycles. PENIRQ enabled)+0(12bit mode)+0(dont care)
	//start
	//send address byte I2C_SLAVE_ADDRESS
	//I2CMasterSlaveAddrSet( I2C2_MASTER_BASE, 0x48, false);
	////send command byte
	//I2CMasterDataPut( I2C2_MASTER_BASE, 0xA0);
	//I2CMasterControl( I2C2_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);
	//WaitI2CDone( I2C2_MASTER_BASE);
	//stop


	//dummy write to set low power mode
	//**power down between cycles (low power mode) 	00:c0
	//**A/D on, IRQ disabled 						01:c4
	//**A/D off, IRQ enabled 						10:c8
	//**A/D on, IRQ disabled 						11:cc
	tY=LCDBPV2_touch_receiveI2C(0xc0);
	//initializes events counter.
	tevents=0;
	//set unlocked mode
	lock=0;
	//denotes that incoming touch is the first touch after a falling edge.
	fldown=0;
	//set "data needs to be calibrated"
	flcal=0;
	//set I2C error flag to 0
	I2Cwd=0;

    touchstate=1;
}

//
static unsigned long WaitI2CDone( unsigned int long ulBase) //max I2C comunication: 4ms. Si en 4ms no ha salido del while 1, sacarlo
{
	// Wait until done transmitting
	//TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	while( I2CMasterBusy(ulBase)&&(I2Cwd==0));
	//TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	// Return I2C error code
	return I2CMasterErr( ulBase);
}

//send a command to touch driver and receives and acknowledge (x,y,z coordenates)
int LCDBPV2_touch_receiveI2C(int op)
{
	int datah;
	int datal;

	//start
	//send address byte I2C_SLAVE_ADDRESS
	I2CMasterSlaveAddrSet( I2C_M_BASE, 0x48, false);
	//send command byte
	I2CMasterDataPut( I2C_M_BASE, op);
	I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_SINGLE_SEND);
	WaitI2CDone( I2C_M_BASE);
	//stop

	if(I2Cwd==0)
	{
		//i2c read
		//send address byte I2C_SLAVE_ADDRESS
		I2CMasterSlaveAddrSet( I2C_M_BASE, 0x48, true);   // false = write, true = read
		//read data byte 1
		I2CMasterControl( I2C_M_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
		//TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
		while(I2CMasterBusy(I2C_M_BASE)&&(I2Cwd==0));
		{
			datah = I2CMasterDataGet(I2C_M_BASE);
		}
	}
	else datah=0;

	if(I2Cwd==0)
	{
		I2CMasterControl(I2C_M_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH); //I2C_MASTER_CMD_BURST_RECEIVE_CONT
		//TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
		while(I2CMasterBusy(I2C_M_BASE)&&(I2Cwd==0));
		{
			datal = I2CMasterDataGet(I2C_M_BASE);
		}
	}
	else datal=0;

	return ((datal/16)+(datah*16));
}

//Handles touch interrupt
void touch_IntHandler(void) //button x=330, y=250  x=53, y=257
{
	GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_0);

	SysCtlDelay(20000); //200000

	if(!GPIOPinRead(GPIO_PORTE_BASE,GPIO_PIN_0)) //temp & GPIO_PIN_0 &(fl)    GPIOPinIntStatus(GPIO_PORTE_BASE, false)&(GPIO_PIN_0) !GPIOPinRead(GPIO_PORTE_BASE,GPIO_PIN_0)
	{
		//LCDBPV2_touch_Init();
		touchstate=0; //0=down
		fldown=0;
		TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	}
	else if(GPIOPinRead(GPIO_PORTE_BASE,GPIO_PIN_0)) //&flg temp & GPIO_PIN_0 &(fl)
	{
		touchstate=1; //1=high
		TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	}

	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_0);
	//delay to avoid rebounds
	SysCtlDelay(100);
	GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_0);
}

//process events to be executed every touch
void touchprocess()
{

	switch(touchstate)
	{
		case 0:
			if ((fldown==0)||(lock==1))
			{
				gettouch();
				if((tX!=xbk)||(tY!=ybk))
				{

					if((tRtouch<100)) //&&g_pfnTSHandler
					{
						xbk=tX;
						ybk=tY;

						WidgetPointerMessage(WIDGET_MSG_PTR_DOWN, tX, tY);
						int jj;
						for(jj=0;jj<tevents;jj++)
						{
							touchevents[jj]();
						}
					}
					else
					{

					}
					fldown=1;
				}
				flcc=0;

			}
			break;
		case 1:
			//if(g_pfnTSHandler)
			//{
			WidgetPointerMessage(WIDGET_MSG_PTR_UP , xbk, ybk);
			//}
			fldown=0;
			break;
		case 2:
			SysCtlDelay(10);
			break;
		default:
			SysCtlDelay(10);
	}
}

//ask touch data and calibrates this. Stores X on tX, Y on tY, Z1 on tZ1, Z2 on tZ2 and Rtouch on tRtouch.
void gettouch()
{
	//		MODE		D7-D4 	D3-D2	D1	D0
	//C8	Measure X  (1100)	10		0	Dont care (0)
	//D8	Measure Y  (1101)	10		0	Dont care (0)
	//F9	Measure Z1 (1110)	10		0	Dont care (0)
	//E8	Measure Z2 (1111)	10		0	Dont care (0)


	do
	{
		//TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
		//TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
		SysCtlDelay(100);
		I2Cwd=0;
		tY=LCDBPV2_touch_receiveI2C(0xC4); //c0 c8 c1
		SysCtlDelay(10);
		if(I2Cwd==0)
		{
			tX=LCDBPV2_touch_receiveI2C(0xD4);
			SysCtlDelay(10);
			//TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
		}
		if(I2Cwd==0)
		{
			tZ1=LCDBPV2_touch_receiveI2C(0xf4); //f4
			SysCtlDelay(10);
			//TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
		}
		if(I2Cwd==0)
		{
			tZ2=LCDBPV2_touch_receiveI2C(0xe4); //e8
			SysCtlDelay(10);
			tXc=tX;
			tYc=tY;
			//TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
		}

		if(I2Cwd==0)
		{
			I2CMasterSlaveAddrSet( I2C2_MASTER_BASE, 0x48, false);
			//send command byte
			I2CMasterDataPut( I2C2_MASTER_BASE, 0xe8); //E0 e8
			I2CMasterControl( I2C2_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);
			WaitI2CDone( I2C2_MASTER_BASE);
			//TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
		}
		//TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
		//TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	}while (I2Cwd==1);
	I2Cwd=0;

	//****calc tRtouch
	tRtouch=((tX*(tZ1-tZ2))/(4096*tZ2));

	//if flcal==1 raw data is requested by calibrate function. If 0 raw data is calibrated.
	if(flcal==0)
	{
		LCDBPV2_calcParam();
		if(tY<0) tY=0;
		if(tY>240) tY=240;
		if(tX<0) tX=0;
		if(tX>320) tX=320;
	}

}

void addevent(functiontype2 event1)
{
	touchevents[tevents]=event1;
	tevents++;
}

//*****************************************************************************
//
//! Waits for calibration point to be pressed by user.
//!
//! This function sets the driver into calibration mode until the next pen
//! up event happens. The reading is stored for the provided point index.
//! After calling this function for point 0, 1 and 2, call TouchScreenCalibrate
//! to calculate new calibration matrix.
//!
//! \param sPointX	X screen coordinate of calibration point
//! \param sPointY	Y screen coordinate of calibration point
//! \param ulPointIndex Number of calibration point 0-2
//!
//! \return None.
//
//*****************************************************************************
void TouchScreenCalibrationPoint(uint16_t sPointX, uint16_t sPointY, uint16_t ulPointIndex)
{

	//set only one read each touch interrupt
	lock=0;

	//set calibration state on. In this state raw data is not processed with any calibration data.
	flcal=1;
	//whait until touchstate==1

	do
	{
		SysCtlDelay(200);
	}while(touchstate!=0);

	//(touch Interrupt has occurred and gpio pin is in low state. RAW data has been read (tX and tY). Has not taken place a rising edge yet.)

	//whait until touchstate==2.
	do
	{
		SysCtlDelay(200);
	}while(touchstate!=1);
	//(touch Interrupt has occurred and gpio pin is in high state. Has occurred a rising edge.)


	if(ulPointIndex <= 3)
	{
		// store LCD coordinates and raw touch data for later processing
		g_psCalLCD[ulPointIndex][0] = sPointX;
		g_psCalLCD[ulPointIndex][1] = sPointY;
		g_psCalRAW[ulPointIndex][0] = tXc;//g_sCalibrateX
		g_psCalRAW[ulPointIndex][1] = tYc;//g_sCalibrateY
	}
	flcal=0;
	//return;
}

//*****************************************************************************
//
//! Calculates calibration matrix from calibration points
//!
//! This function calculates the calibration matrix from information collected
//! from preceding calls to TouchScreenCalibrationPoint.
//! If calibration was successful, the touch driver will use the new
//! calibration matrix.
//!
//! \return Returns pointer to calibration matrix if successful, 0 otherwise.
//
//*****************************************************************************
long* LCDBPV2_TouchScreenCalibrate(void)
{
	// Calculate calibration matrix using collected calibration data

	// Calculate divider value
	g_plCalibrationMatrix[6] = ((g_psCalRAW[0][0] - g_psCalRAW[2][0]) * (g_psCalRAW[1][1] - g_psCalRAW[2][1])) - ((g_psCalRAW[1][0] - g_psCalRAW[2][0]) * (g_psCalRAW[0][1] - g_psCalRAW[2][1]));
	ret[6]=g_plCalibrationMatrix[6];

	if(g_plCalibrationMatrix[6] == 0)
	{
		// fatal error, divider can't be 0
		return(0);
	}

	// Calculate A
	g_plCalibrationMatrix[0] =((g_psCalLCD[0][0] - g_psCalLCD[2][0]) * (g_psCalRAW[1][1] - g_psCalRAW[2][1])) -((g_psCalLCD[1][0] - g_psCalLCD[2][0]) * (g_psCalRAW[0][1] - g_psCalRAW[2][1]));
	ret[0]=g_plCalibrationMatrix[0];

	// Calculate B
	g_plCalibrationMatrix[1] =((g_psCalRAW[0][0] - g_psCalRAW[2][0]) * (g_psCalLCD[1][0] - g_psCalLCD[2][0])) -((g_psCalLCD[0][0] - g_psCalLCD[2][0]) * (g_psCalRAW[1][0] - g_psCalRAW[2][0]));
	ret[1]=g_plCalibrationMatrix[1];

	// Calculate C
	g_plCalibrationMatrix[2] =(g_psCalRAW[2][0] * g_psCalLCD[1][0] - g_psCalRAW[1][0] * g_psCalLCD[2][0]) * g_psCalRAW[0][1] + (g_psCalRAW[0][0] * g_psCalLCD[2][0] -
			g_psCalRAW[2][0] * g_psCalLCD[0][0]) * g_psCalRAW[1][1] +(g_psCalRAW[1][0] * g_psCalLCD[0][0] - g_psCalRAW[0][0] * g_psCalLCD[1][0]) * g_psCalRAW[2][1];
	ret[2]=g_plCalibrationMatrix[2];

	// Calculate D
	g_plCalibrationMatrix[3] =((g_psCalLCD[0][1] - g_psCalLCD[2][1]) * (g_psCalRAW[1][1] - g_psCalRAW[2][1])) -((g_psCalLCD[1][1] - g_psCalLCD[2][1]) * (g_psCalRAW[0][1] - g_psCalRAW[2][1]));
	ret[3]=g_plCalibrationMatrix[3];

	// Calculate E
	g_plCalibrationMatrix[4] =((g_psCalRAW[0][0] - g_psCalRAW[2][0]) * (g_psCalLCD[1][1] - g_psCalLCD[2][1])) -((g_psCalLCD[0][1] - g_psCalLCD[2][1]) * (g_psCalRAW[1][0] - g_psCalRAW[2][0]));
	ret[4]=g_plCalibrationMatrix[4];

	// Calculate F
	g_plCalibrationMatrix[5] =(g_psCalRAW[2][0] * g_psCalLCD[1][1] - g_psCalRAW[1][0] * g_psCalLCD[2][1]) * g_psCalRAW[0][1] +(g_psCalRAW[0][0] * g_psCalLCD[2][1] -
			g_psCalRAW[2][0] * g_psCalLCD[0][1]) * g_psCalRAW[1][1] +(g_psCalRAW[1][0] * g_psCalLCD[0][1] - g_psCalRAW[0][0] * g_psCalLCD[1][1]) * g_psCalRAW[2][1];
	ret[5]=g_plCalibrationMatrix[5];

	// Update touch driver to use new matrix
	g_plParmSet = g_plCalibrationMatrix;
	// calibration successful
	return(g_plCalibrationMatrix);
}

//function recalculates coordenates (raw data) with calibrated data.
void LCDBPV2_calcParam()
{
	if(flcc==0)
	{
		long lTemp;
		lTemp = (((tX * g_plParmSet[0]) + (tY * g_plParmSet[1]) + g_plParmSet[2]) /g_plParmSet[6]);
		tY = (((tX * g_plParmSet[3]) + (tY * g_plParmSet[4]) + g_plParmSet[5]) /g_plParmSet[6]);
		tX = lTemp;
		flcc=1;
	}
}
//Function that handles Timer0 Interrupt.
//
//It is called every ulPeriod period after being activated by touch_IntHandler() (LCDBPV2_touch.c) when stylus press the screen.
//Timer0 Interrupt is disabled when stylus leaves the screen.
//// TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod); sets period to the new ulPeriod. (default ulPeriod=SysCtlClockGet()/800)


void LCDBPV2_setparm(long *parm)
{
	g_plParmSet=parm;
}
long tempparm[7];
long* LCDBPV2_loadparm()
{

	//A
	tempparm[0]=380680;
	//B
	tempparm[1]=-728;
	//C
	tempparm[2]=-1484942016;
	//D
	tempparm[3]=-11440;
	//E
	tempparm[4]=286864;
	//F
	tempparm[5]=-1080423872;
	//Div
	tempparm[6]=-4265432;

	g_plParmSet=tempparm;
	return(tempparm);

}
void LCDBPV2_calibration2eeprom()
{
	int32_t a;
	uint32_t c;
	a=ret[0];
	c=(int32_t)a;
	SysCtlDelay(100);
	EEPROMProgram(&c, 0x0, sizeof(ret[0]));
	a=ret[1];
	c=(int32_t)a;
	EEPROMProgram(&c, 0x20, sizeof(ret[1]));
	a=ret[2];
	c=(int32_t)a;
	EEPROMProgram(&c, 0x40, sizeof(ret[2]));
	a=ret[3];
	c=(int32_t)a;
	EEPROMProgram(&c, 0x60, sizeof(ret[3]));
	a=ret[4];
	c=(int32_t)a;
	EEPROMProgram(&c, 0x80, sizeof(ret[4]));
	a=ret[5];
	c=(int32_t)a;
	EEPROMProgram(&c, 0xA0, sizeof(ret[5]));
	a=ret[6];
	c=(int32_t)a;
	EEPROMProgram(&c, 0xC0, sizeof(ret[6]));
}
void LCDBPV2_eeprom2calibration()
{
	uint32_t b[1];
	EEPROMRead(b, 0x0, sizeof(b));
	ret[0]=(int32_t)b[0];
	EEPROMRead(b, 0x20, sizeof(b));
	ret[1]=(int32_t)b[0];
	EEPROMRead(b, 0x40, sizeof(b));
	ret[2]=(int32_t)b[0];
	EEPROMRead(b, 0x60, sizeof(b));
	ret[3]=(int32_t)b[0];
	EEPROMRead(b, 0x80, sizeof(b));
	ret[4]=(int32_t)b[0];
	EEPROMRead(b, 0xA0, sizeof(b));
	ret[5]=(int32_t)b[0];
	EEPROMRead(b, 0xC0, sizeof(b));
	ret[6]=(int32_t)b[0];
	LCDBPV2_setparm(ret);
}
void nothing(uint16_t sPointX, uint16_t sPointY, uint16_t ulPointIndex)
{

}
/*
void LCDBPV2_TouchScreenCallbackSet(int32_t (*pfnCallback)(uint32_t ulMessage, int32_t lX, int32_t lY))
{
    //
    // Save the pointer to the callback function.
    //
    g_pfnTSHandler = pfnCallback;
}
*/
