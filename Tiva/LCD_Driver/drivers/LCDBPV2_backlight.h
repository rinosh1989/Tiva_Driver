/*
 * LCDBPV2_backlight.h
 *
 *  Created on: 27/12/2013
 *      Author: Dani
 */

#ifndef LCDBPV2_BACKLIGHT_H_
#define LCDBPV2_BACKLIGHT_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define I2C2_MASTER_BASE I2C2_BASE

unsigned long WaitI2CDone( unsigned int long ulBase);
void SetupI2C();
void ConfigBK_I2C();
void Config();
void LCDBPV2_backlight_init();

#ifdef __cplusplus
}
#endif
#endif /* LCDBPV2_BACKLIGHT_H_ */
