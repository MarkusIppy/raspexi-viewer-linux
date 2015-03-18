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

// Model 13B-REW 
#define CSV_HEADER_1  "Time,"													   \
		"Engine Speed (rpm),"								/*-RPM				*/ \
		"Absolute Intake Pressure (Kg/cm2),"				/*-Intakepress		*/ \
		"Pressure Sensor Voltage (mv),"            			/*-PressureV		*/ \
		"Throttle Sensor Voltage (mv),"                 	/*-ThrottleV		*/ \
		"Primary Injector Pulse Width (mSec)," 				/*-Primaryinp		*/ \
		"Fuel correction," 									/*-Fuelc			*/ \
		"Leading Ignition Angle (deg)," 					/*-Leadingign		*/ \
		"Trailing Ignition Angle (deg),"					/*-Trailingign		*/ \
		"Fuel Temperature (deg.C),"							/*-Fueltemp			*/ \
		"Metering Oil Pump Duty (%),"						/*-Moilp			*/ \
		"Boost Duty (TP %),"								/*-Boosttp			*/ \
		"Boost Duty (Wg %),"								/*-Boostwg			*/ \
		"Water Temperature (deg. C),"						/*-Watertemp		*/ \
		"Intake Air Temperature (deg C),"					/*-Intaketemp		*/ \
		"Knocking Level," 									/*-Knock			*/ \
		"Battery Voltage (V),"								/*-BatteryV			*/ \
		"Vehicle Speed (Km/h),"								/*-Speed			*/ \
		"ISCV duty (%),"									/*-Iscvduty			*/ \
		"O2 Sensor Voltage (V)," 							/*-O2volt			*/ \
		"Secondary Injector Pulse Width (mSec)," 	  		/*-Secinjpulse		*/	
 
 
// For most NISSAN and SUBARU engines
#define CSV_HEADER_2  "Time,"													   \
		"Engine Speed (rpm),"								/*-RPM				*/ \
		"Engine Load (N),"									/*-EngLoad			*/ \
		"Mass Flow Sensor #1 (mv),"            				/*-MAF1V			*/ \
		"Mass Flow Sensor #2 (mv),"							/*-MAF2V			*/ \
		"Primary Injector Pulse Width (mSec)," 				/*-Primaryinp		*/ \
		"Fuel correction," 									/*-Fuelc			*/ \
		"Ignition Timing (deg),"		 					/*-Leadingign		*/ \
		"Ignition Dwell Angle (deg),"						/*-Trailingign		*/ \
		"Boost Pressure (PSI),"								/*-BoostPres		*/ \
		"Boost Duty (%),"									/*-BoostDuty		*/ \
		"Water Temperature (deg. C),"						/*-Watertemp		*/ \
		"Intake Air Temperature (deg C),"					/*-Intaketemp		*/ \
		"Knocking Level,"									/*-Knock			*/ \
		"Battery Voltage (V),"								/*-BatteryV			*/ \
		"Vehicle Speed (Km/h),"								/*-Speed			*/ \
		"Mass Air Flow sensor activity ratio (%),"			/*-MAFactivity		*/ \
		"O2 Sensor #1 Voltage (V),"							/*-O2volt			*/ \
		"O2 Sensor #2 Voltage (V),"							/*-O2volt_2			*/ \
		"Throttle Sensor Voltage (mv),,"  					/*-ThrottleV		*/


// For most TOYOTA engines
#define CSV_HEADER_3  "Time,"													   \
		"Engine Speed (rpm),"								/*-RPM				*/ \
		"Absolute Intake Pressure (Kg/cm2),"				/*-Intakepress		*/ \
		"Pressure Sensor Voltage (mv),"            			/*-PressureV		*/ \
		"Throttle Sensor #1 Voltage (mv),"					/*-ThrottleV		*/ \
		"Primary Injector Pulse Width (mSec)," 				/*-Primaryinp		*/ \
		"Fuel correction," 									/*-Fuelc			*/ \
		"Leading Ignition Angle (deg)," 					/*-Leadingign		*/ \
		"Trailing Ignition Angle (deg),"					/*-Trailingign		*/ \
		"Boost Pressure (PSI),"								/*-BoostPres		*/ \
		"Boost Duty (%),"									/*-BoostDuty		*/ \
		"Water Temperature (deg. C),"						/*-Watertemp		*/ \
		"Intake Air Temperature (deg C),"					/*-Intaketemp		*/ \
		"Knocking Level,"									/*-Knock			*/ \
		"Battery Voltage (V),"								/*-BatteryV			*/ \
		"Vehicle Speed (Km/h),"								/*-Speed			*/ \
		"ISCV duty (%),"									/*-Iscvduty			*/ \
		"O2 Sensor Voltage (V),"							/*-O2volt			*/ \
		"Suction In Air Temperature (mV),"					/*-SuctionAirTemp	*/ \
		"Throttle Sensor #2 Voltage(mv),,"  				/*-ThrottleV_2		*/

#define CSV_HEADER_AUX 	"AUX1,"	\
						"AUX2,"	\
						"AUX3,"	\
						"AUX4,"	\
						"AUX5,"	\
						"AUX6,"	\
						"AUX7,"	\
						"AUX8,"

#define CSV_HEADER_ANALOG 	"Analog1,"	\
							"Analog2,"	\
							"Analog3,"	\
							"Analog4,"

/*
*Structure for Advanced information Group FD3S : 
*/

typedef struct {
	unsigned short RPM;				//0
	unsigned short Intakepress;		//1
	unsigned short PressureV;		//2
	unsigned short ThrottleV;		//3
	unsigned short Primaryinp;		//4
	unsigned short Fuelc;			//5
	unsigned char Leadingign;		//6
	unsigned char Trailingign;		//7
	unsigned char Fueltemp;			//8
	unsigned char Moilp;			//9
	unsigned char Boosttp;			//10
	unsigned char Boostwg;			//11
	unsigned char Watertemp;		//12
	unsigned char Intaketemp;		//13
	unsigned char Knock;			//14
	unsigned char BatteryV;			//15
	unsigned short Speed;			//16
	unsigned short Iscvduty;		//17
	unsigned char O2volt;			//18
	unsigned char na1;				//19
	unsigned short Secinjpulse;		//20
	unsigned char na2;				//21
} fc_adv_info_t;

/*
*Structure for Advanced information Group NISSAN and SUBARU :
*/

typedef struct {
	unsigned short RPM;				//0
	unsigned short EngLoad;			//1
	unsigned short MAF1V;			//2
	unsigned short MAF2V;			//3
	unsigned short Primaryinp;		//4
	unsigned short Fuelc;			//5
	unsigned char Leadingign;		//6
	unsigned char Trailingign;		//7
	unsigned short BoostPres;		//8
	unsigned short BoostDuty;		//9
	unsigned char Watertemp;		//10
	unsigned char Intaketemp;		//11
	unsigned char Knock;			//12
	unsigned char BatteryV;			//13
	unsigned short Speed;			//14
	unsigned short MAFactivity;		//15
	unsigned char O2volt;			//16
	unsigned char O2volt_2;			//17
	unsigned short ThrottleV;		//18
	unsigned char na1;				//19
} fc_adv_info_t_2;

/*
*Structure for Advanced information Group TOYOTA :
*/

typedef struct {
	unsigned short RPM;				//0
	unsigned short Intakepress;		//1
	unsigned short PressureV;		//2
	unsigned short ThrottleV;		//3
	unsigned short Primaryinp;		//4
	unsigned short Fuelc;			//5
	unsigned char Leadingign;		//6
	unsigned char Trailingign;		//7
	unsigned short BoostPres;		//8
	unsigned short BoostDuty;		//9
	unsigned char Watertemp;		//10
	unsigned char Intaketemp;		//11
	unsigned char Knock;			//12
	unsigned char BatteryV;			//13
	unsigned short Speed;			//14
	unsigned short Iscvduty;		//15
	unsigned char O2volt;			//16
	unsigned char SuctionAirTemp;	//17
	unsigned short ThrottleV_2;		//18
	unsigned char na1;				//19
} fc_adv_info_t_3;

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

typedef struct {
	unsigned char Analog1;
	unsigned char Analog2;
	unsigned char Analog3;
	unsigned char Analog4;
} analog_info_t;


/*
* Maximum number of elements in the above structs
*/
#define FC_ADV_INFO_MAX_ELEMENTS	21
#define FC_AUX_INFO_MAX_ELEMENTS	8
#define ANALOG_INFO_MAX_ELEMENTS	4
#define MAP_ELEMENTS				FC_ADV_INFO_MAX_ELEMENTS + 1 + FC_AUX_INFO_MAX_ELEMENTS + ANALOG_INFO_MAX_ELEMENTS

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
// FD3S
#define FC_ADV_INFO_MUL		{1, 0.0001, 1, 1, 1.0/256, 1.0/256,  1,  1,  1, 212.0/256, 0.4, 0.4,  1,  1, 1, 0.1, 1, 0.1, 0.02, 1, 1.0/256, 1}
#define FC_ADV_INFO_ADD		{0,-1.0332, 0, 0,       0,       0,-25,-25,-80,         0,   0,   0,-80,-80, 0,   0, 0,   0,    0, 0,       0, 0}
//Nissan and Subaru
#define FC_ADV_INFO_MUL_2	{1, 1, 1, 1, 1.0/256, 1.0/256, 1, 1, 1, 0.005,  1,  1, 1, 0.1, 1, 0.1, 0.02, 0.02, 1, 1}
#define FC_ADV_INFO_ADD_2	{0, 0, 0, 0,       0,       0, 0, 0, 0,     0,-80,-80, 0,   0, 0,   0,    0,    0, 0, 0}
//Toyota
#define FC_ADV_INFO_MUL_3	{1, 0.0001, 1, 1, 1.0/256, 1.0/256, 1, 1, 1, 0.005,  1,  1, 1, 0.1, 1, 0.1, 0.02,  1, 1, 1}
#define FC_ADV_INFO_ADD_3	{0,-1.0332, 0, 0,       0,       0, 0, 0, 0,     0,-80,-80, 0,   0, 0,   0,    0,-80, 0, 0}

#define FC_AUX_INFO_MUL		{5.0/255, 5.0/255, 5.0/255, 5.0/255, 5.0/255, 5.0/255, 5.0/255, 5.0/255};
#define FC_AUX_INFO_ADD		{      0,       0,       0,       0,       0,       0,       0,       0};

gdouble powerfc_get_current_value(gchar *);

gboolean powerfc_process_advanced(gpointer);
gboolean powerfc_process_auxiliary(gpointer);

FILE *powerfc_open_csvfile(gchar *);
#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
