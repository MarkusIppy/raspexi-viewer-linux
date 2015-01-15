/*
 * Copyright (C) 2014 Markus Ippy
 *
 * Digital Gauges for Apexi Power FC for RX7 on Raspberry Pi 
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*! 
  \file raspexi/powerfc.h
  \brief Raspexi Viewer Power FC related functions
  \author Suriyan Laohaprapanon
 */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __POWERFC_H__
#define __POWERFC_H__

#include <glib.h>

/*
 * Source: http://www.rx7.org/public/PowerFC-DataLogIt.htm
 * ------------------
 * "Advanced" Information (Cooked Averages?)
 * ------------------
 * 0xF0
 * ------------------
 * short word : rpm (just as a short word, not worked over)
 * short word : pim (don't know... FCEdit just displays it as a short word)
 * short word : pim voltage * 1000.0f (divide by 1000.0f to get the voltage)
 * short word : tps voltage * 1000.0f (divide by 1000.0f to get the voltage)
 * short word : InjFP ms (short float)
 * short word : INJ (inj what? dunno... FCEdit shows it as a short word...)
 * byte : IGL (byte degree)
 * byte : IGT (byte degree)
 * byte : Fuel Temperature (as byte temperature)
 * byte : oil (FCEdit displays as just a byte...)
 * byte : Precontrol % (FCEdit calls it a %, but looks like it's just displaying the byte)
 * byte : Wastegate % (FCEdit calls it a %, but looks like it's just displaying the byte)
 * byte : water temperature (as byte temperature)
 * byte : air temperature (as byte temperature)
 * byte : knock
 * byte : battery voltage * 10.0f (divide by 10.0f to get voltage)
 * 
 * NOTE: From here, FCEdit is unsure of what's what... Consequently, I'm unsure of what's what.
 * 
 * short word : kph (don't know the units yet)
 * short word : unknown
 * byte : O2
 * byte : unknown
 * short word : InjFS (ms)
 */

/*
 * -Engine Speed (rpm)								-RPM
 * -Absolute Intake Pressure (Kg/cm2)				-Intakepress	
 * -Pressure Sensor Voltage (mv)            		-PressureV		
 * -Throttle Sensor Voltage (mv)                 	-ThrottleV
 * -Primary Injector Pulse Width (mSec) 			-Primaryinp
 * -Fuel correction 								-Fuelc
 * -Leading Ignition Angle (deg) 					-Leadingign
 * -Trailing Ignition Angle (deg)					-Trailingign
 * -Fuel Temperature (deg.C)						-Fueltemp
 * -Metering Oil Pump Duty (%)						-Moilp
 * -Boost Duty (TP.%)								-Boosttp
 * -Boost Duty (Wg,%)								-Boostwg
 * -Water Temperature (deg. C)						-Watertemp
 * -Intake Air Temperature (deg C)					-Intaketemp
 * -Knocking Level 									-Knock
 * -Battery Voltage (V)								-BatteryV
 * -Vehicle Speed (Km/h)							-Speed
 * -ISCV duty (%)									-Iscvduty
 * -O2 Sensor Voltage (mv) 							-O2volt
 * -Secondary Injector Pulse Width (mSec) 	  		-Secinjpulse
 */
 
/*
*Structure for Advanceed information Group FD3S : 
*/

typedef struct {
	unsigned short RPM;
	unsigned short Intakepress;
	unsigned short PressureV;
	unsigned short ThrottleV;
	unsigned short Primaryinp;
	unsigned short Fuelc;
	unsigned char Leadingign;
	unsigned char Trailingign;
	unsigned char Fueltemp;
	unsigned char Moilp;
	unsigned char Boosttp;
	unsigned char Boostwg;
	unsigned char Watertemp;
	unsigned char Intaketemp;
	unsigned char Knock;
	unsigned char BatteryV;
	unsigned short Speed;
	unsigned short Iscvduty;
	unsigned char O2volt;
	unsigned char na1;
	unsigned short Secinjpulse;
	unsigned char na2;
} fc_adv_info_t;

/*
*Structure for Auxiliary information : 
*/

typedef struct {
	unsigned char AUX1;
	unsigned char AUX2;
	unsigned char AUX3;
	unsigned char AUX4;
	unsigned char AUX5;
	unsigned char AUX6;
	unsigned char AUX7;
	unsigned char AUX8;
} fc_aux_info_t;

/*
*Advanced Information : 
*/

#define FC_ADV_INFO_LEN		(0x20)
#define FC_REQ_ADV_INFO		{0xF0, 0x02, 0x0D}



/*
* Auxiliary Information
*/

#define FC_REQ_AUX_INFO		{0x00, 0x02, 0xFD}


/*
* preserving original calculations for testing 
* #define FC_ADV_INFO_MUL		{1, 0.0001, 1, 1, 1.0/256, 1.0/256, 1, 1, 1, 212.0/256, 0.4, 0.4, 1, 1, 1, 0.1, 1, 0.1, 0.02, 1, 1.0/256, 1}
* #define FC_ADV_INFO_ADD		{0, 0, 0, 0, 0, 0, -25, -25, -80, 0, 0, 0, -80, -80, 0, 0, 0, 0, 0, 0, 0, 0}
*/

/*
*Calculation to display KG/cm2 for Boost 
*
*/
#define FC_ADV_INFO_MUL		{1, 0.0001, 1, 1, 1.0/256, 1.0/256, 1, 1, 1, 212.0/256, 0.4, 0.4, 1, 1, 1, 0.1, 1, 0.1, 0.02, 1, 1.0/256, 1}
#define FC_ADV_INFO_ADD		{0, -1.0332, 0, 0, 0, 0, -25, -25, -80, 0, 0, 0, -80, -80, 0, 0, 0, 0, 0, 0, 0, 0}
#define FC_AUX_INFO_MUL		{5.0/255, 5.0/255, 5.0/255, 5.0/255, 5.0/255, 5.0/255, 5.0/255, 5.0/255};
#define FC_AUX_INFO_ADD		{0, 0, 0, 0, 0, 0, 0, 0};

gdouble powerfc_get_current_value(gchar *);

gboolean powerfc_process_advanced(gpointer);
gboolean powerfc_process_auxiliary(gpointer);

FILE *powerfc_open_csvfile(gchar *);
#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
