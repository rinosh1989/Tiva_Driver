/*
 * LCDBPV2_touch.h
 *
 *  Created on: 27/12/2013
 *      Author: Dani
 */

#ifndef LCDBPV2_TOUCH_H_
#define LCDBPV2_TOUCH_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
//#include "inc/hw_adc.h"
//#include "inc/hw_gpio.h"
//#include "inc/hw_ints.h"
//#include "inc/hw_memmap.h"
//#include "inc/hw_timer.h"
//#include "inc/hw_types.h"
//#include "driverlib/adc.h"
#include "driverlib/gpio.h"
//#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
//#include "driverlib/timer.h"
#include "grlib/grlib.h"
#include "grlib/widget.h"
//#include "driverlib/i2c.h"
//#include "driverlib/rom.h"
#include "driverlib/eeprom.h"
//#include <stdint.h>
//#include <stdbool.h>
//#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
//#include "driverlib/sysctl.h"
//
//#include "driverlib/i2c.h"
//#include "inc/hw_adc.h"
//#include "inc/hw_gpio.h"
////#include "inc/hw_ints.h"
//#include "inc/hw_memmap.h"
//#include "inc/hw_timer.h"
//#include "inc/hw_types.h"
//#include "driverlib/adc.h"
//#include "driverlib/gpio.h"
//#include "driverlib/interrupt.h"
//#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
//#include "grlib/grlib.h"
//#include "grlib/widget.h"
#include "driverlib/i2c.h"
//#include "driverlib/rom.h"
//#include "driverlib/eeprom.h"


void LCDBPV2_touch_IntHandler(void);
void touch_IntHandler(void);
void gettouch();
void LCDBPV2_touch_Debouncer(void);
void LCDBPV2_touch_Init()  ;

//void LCDBPV2_touch_CallbackSet(long (*pfnCallback)(unsigned long ulMessage, long lX, long lY));
void LCDBPV2_TouchScreenCallbackSet(int32_t (*pfnCallback)(uint32_t ulMessage, int32_t lX, int32_t lY));

void LCDBPV2_touch_CalibrationPoint(unsigned short sPointX, unsigned short sPointY, unsigned long ulPointIndex);
long* LCDBPV2_touch_Calibrate(void);
int tY,tX,tZ1,tZ2;
int	tRtouch, fl;
typedef void (*functiontype2)();
void addevent(functiontype2 event1);
int LCDBPV2_touch_receiveI2C(int op);
int down;
void touchint(int status );
void touchprocess();
int lock;
//flag that indicates if data needs to be calibrated (0) or not(1)
int flcal;

void TouchScreenCalibrationPoint(uint16_t sPointX, uint16_t sPointY, uint16_t ulPointIndex);
void nothing(uint16_t sPointX, uint16_t sPointY, uint16_t ulPointIndex);

long* LCDBPV2_TouchScreenCalibrate(void);
void LCDBPV2_calcParam();
int I2Cwd;

//extern long LCDBPV2_getParm();
long ret[7];

void LCDBPV2_setparm(long* parm);
void LCDBPV2_calibration2eeprom();
void LCDBPV2_eeprom2calibration();
long* LCDBPV2_loadparm();

#define I2C2_MASTER_BASE I2C2_BASE
#ifdef __cplusplus
}
#endif
#endif /* LCDBPV2_TOUCH_H_ */

