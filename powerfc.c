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
  \file raspexi/powerfc.c
  \brief Raspexi Viewer Power FC related functions
  \author Suriyan Laohaprapanon
 */

#include <glib.h>
#include <gmodule.h>
#include <debugging.h>
#include <serialio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include <glib/gstdio.h>

#include <powerfc.h>

extern gconstpointer *global_data;

// FD3S map
static gchar *map[] = {"RPM", "Intakepress", "PressureV",
                       "ThrottleV", "Primaryinp", "Fuelc", "Leadingign", "Trailingign",
                       "Fueltemp", "Moilp", "Boosttp", "Boostwg", "Watertemp", "Intaketemp", "Knock", "BatteryV",
                       "Speed", "Iscvduty", "O2volt", "na1", "Secinjpulse",
					   "na2", 
                       "AUX1", "AUX2", "AUX3", "AUX4", "AUX5", "AUX6", "AUX7", "AUX8",
					   "Analog1", "Analog2", "Analog3", "Analog4",
					   "Power"};

// Nissan and Subaru map
static gchar *map2[] = { "RPM", "EngLoad", "MAF1V",
                         "MAF2V", "Primaryinp", "Fuelc", "Leadingign", "Trailingign",
                         "BoostPres", "BoostDuty", "Watertemp", "Intaketemp", "Knock", "BatteryV",
                         "Speed", "MAFactivity", "O2volt", "O2volt_2", "ThrottleV",
						 "na1", "", "",
						 "AUX1", "AUX2", "AUX3", "AUX4", "AUX5", "AUX6", "AUX7", "AUX8",
						 "Analog1", "Analog2", "Analog3", "Analog4",
						 "Power" };

// Toyota map
static gchar *map3[] = { "RPM", "Intakepress", "PressureV",
                         "ThrottleV", "Primaryinp", "Fuelc", "Leadingign", "Trailingign",
                         "BoostPres", "BoostDuty", "Watertemp", "Intaketemp", "Knock", "BatteryV",
						 "Speed", "Iscvduty", "O2volt", "SuctionAirTemp", "ThrottleV_2",
						 "na1", "", "",
						 "AUX1", "AUX2", "AUX3", "AUX4", "AUX5", "AUX6", "AUX7", "AUX8",
						 "Analog1", "Analog2", "Analog3", "Analog4",
						 "Power" };

static gdouble rtv[MAP_ELEMENTS]; // Plus one is for the last unavailable item (e.g. na2, na1)

static gint previousTime_uSec = 0.0;

/*!
	\brief Wrapper function that does a nonblocking select()/read loop .
	\param fd is the serial port filedescriptor
	\param buf is the pointer to where to stick the data
	\param count is how many bytes to read
	\param len is the pointer to length read
	\returns TRUE on success, FALSE otherwise
	*/
G_MODULE_EXPORT gboolean read_wrapper(gint fd, guint8 * buf, size_t count, gint *len)
{
	gint res = 0;
	fd_set rd;
	struct timeval timeout;
	Serial_Params *serial_params = NULL;
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	ENTER();

	FD_ZERO(&rd);
	FD_SET(fd,&rd);

	*len = 0;
	timeout.tv_sec = 0;
	timeout.tv_usec = DATA_GET(global_data, "read_timeout") == NULL ? 500000:(GINT)DATA_GET(global_data, "read_timeout")*1000;
	/* Network mode requires select to see if data is ready, otherwise
	 * connection will block.  Serial is configured with timeout if no
	 * data is avail,  hence we simulate that with select.. Setting this
	 * timeout around 500ms seems to give us ok function to new zealand,
	 * but may require tweaking for slow wireless links.
	 */
	if (serial_params->net_mode)
	{
		res = select(fd+1,&rd,NULL,NULL,&timeout);
		if (res < 0) /* Error, socket close, abort */
		{
			EXIT();
			return FALSE;
		}
		if (res > 0) /* Data Arrived! */
			*len = recv(fd,(void *)buf,count,0);
		EXIT();
		return TRUE;
	}
	else
		res = read(fd,buf,count);

	if (res < 0)
	{
		EXIT();
		return FALSE;
	}
	else
		*len = res;
	EXIT();
	return TRUE;
}


/*!
	\brief updates the values of all gauges on all dashboards in use
	\param data is unused
	\returns TRUE unless app is closing down
*/
G_MODULE_EXPORT gboolean powerfc_process_auxiliary(gpointer data)
{
	gboolean res = 0;
	gboolean bad_read = FALSE;
	gint len = 0;
	gint total_read = 0;
	gint total_wanted;
	gint zerocount = 0;
	guchar buf[4096];
	guchar *ptr = buf;
	//gdouble mul[] = FC_AUX_INFO_MUL;
	//gdouble add[] = FC_AUX_INFO_ADD;

	Serial_Params *serial_params = NULL;;
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	guchar request[] = FC_REQ_AUX_INFO;
	write(serial_params->fd, request, sizeof(request));

	/* This will be adjusted depending upon the model of logging device */
	total_wanted = 11;

	while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
	{
		if (total_read < 2) {
			res = read_wrapper(serial_params->fd,
					ptr+total_read,
					total_wanted-total_read,&len);
			total_read += len;
		} else {
			total_wanted = buf[1] + 1;
			res = read_wrapper(serial_params->fd,
					ptr+total_read,
					total_wanted-total_read,&len);
			total_read += len;
		}
		/* Increment bad read counter.... */
		if (!res) /* I/O Error Device disappearance or other */
		{
			bad_read = TRUE;
			break;
		}
		if (len == 0) /* Short read!*/
			zerocount++;
		if ((len == 0) && (zerocount > 3))  /* Too many Short reads! */
		{
			bad_read = TRUE;
			break;
		}
	}
	if (bad_read) {
		printf("ERROR: serial read\n");
		return FALSE;
	}
	else {
		fc_aux_info_t *info;
		info = (fc_aux_info_t *)&buf[2];
		rtv[22] = info->AUX1;
		rtv[23] = info->AUX2;
		rtv[24] = info->AUX3;
		rtv[25] = info->AUX4;
		rtv[26] = info->AUX5;
		rtv[27] = info->AUX6;
		rtv[28] = info->AUX7;
		rtv[29] = info->AUX8;


		//Upon receiving the Auxilary analog values perform the calculations as implemented in the config file
		gdouble val1 = 0.0, val2 = 0.0, val3 = 0.0, val4 = 0.0;

		if ((const gchar *)DATA_GET(global_data, "analog_eq1") != NULL)
		{
			int n = sscanf((const gchar *)DATA_GET(global_data, "analog_eq1"), "%lf%*[^0-9]%lf%*[^0-9]%lf%*[^0-9]%lf", &val1, &val2, &val3, &val4);
			rtv[30] = val1 * (rtv[(int)val2 + 21] - (n == 3 ? 0 : rtv[(int)val3 + 21])) + (n == 3 ? val3 : val4);
		}
		if ((const gchar *)DATA_GET(global_data, "analog_eq2") != NULL)
		{
			int n = sscanf((const gchar *)DATA_GET(global_data, "analog_eq2"), "%lf%*[^0-9]%lf%*[^0-9]%lf%*[^0-9]%lf", &val1, &val2, &val3, &val4);
			rtv[31] = val1 * (rtv[(int)val2 + 21] - (n == 3 ? 0 : rtv[(int)val3 + 21])) + (n == 3 ? val3 : val4);
		}
		if ((const gchar *)DATA_GET(global_data, "analog_eq3") != NULL)
		{
			int n = sscanf((const gchar *)DATA_GET(global_data, "analog_eq3"), "%lf%*[^0-9]%lf%*[^0-9]%lf%*[^0-9]%lf", &val1, &val2, &val3, &val4);
			rtv[32] = val1 * (rtv[(int)val2 + 21] - (n == 3 ? 0 : rtv[(int)val3 + 21])) + (n == 3 ? val3 : val4);
		}
		if ((const gchar *)DATA_GET(global_data, "analog_eq4") != NULL)
		{
			int n = sscanf((const gchar *)DATA_GET(global_data, "analog_eq4"), "%lf%*[^0-9]%lf%*[^0-9]%lf%*[^0-9]%lf", &val1, &val2, &val3, &val4);
			rtv[33] = val1 * (rtv[(int)val2 + 21] - (n == 3 ? 0 : rtv[(int)val3 + 21])) + (n == 3 ? val3 : val4);
		}

	}
	return TRUE;
}
G_MODULE_EXPORT gboolean powerfc_process_advanced(gpointer data)
{
	gboolean res = 0;
	gboolean bad_read = FALSE;
	gint len = 0;
	gint total_read = 0;
	gint total_wanted;
	gint zerocount = 0;
	guchar buf[4096];
	guchar *ptr = buf;

	gint model = 0;
	gdouble mul[] = FC_ADV_INFO_MUL; //These are the largest arrays so no need for g_malloc
	gdouble add[] = FC_ADV_INFO_ADD;

	if ((const gchar *)DATA_GET(global_data, "model") == NULL)
	{
		printf("ERROR: No model (vehicle) specified in .cfg file.\n");
		return FALSE;
	}
	else if (g_strcmp0((const gchar *)DATA_GET(global_data, "model"), "Mazda") == 0)
	{
		model = 1;
		gdouble mul_temp[] = FC_ADV_INFO_MUL;
		gdouble add_temp[] = FC_ADV_INFO_ADD;
		memcpy(&mul, &mul_temp, sizeof(mul_temp));
		memcpy(&add, &add_temp, sizeof(add_temp));
	}
	else if ((g_strcmp0((const gchar *)DATA_GET(global_data, "model"), "Nissan") == 0) ||
		(g_strcmp0((const gchar *)DATA_GET(global_data, "model"), "Subaru") == 0))
	{
		model = 2;
		gdouble mul_temp[] = FC_ADV_INFO_MUL_2;
		gdouble add_temp[] = FC_ADV_INFO_ADD_2;
		memcpy(&mul, &mul_temp, sizeof(mul_temp));
		memcpy(&add, &add_temp, sizeof(add_temp));
	}
	else if (g_strcmp0((const gchar *)DATA_GET(global_data, "model"), "Toyota") == 0)
	{
		model = 3;
		gdouble mul_temp[] = FC_ADV_INFO_MUL_3;
		gdouble add_temp[] = FC_ADV_INFO_ADD_3;
		memcpy(&mul, &mul_temp, sizeof(mul_temp));
		memcpy(&add, &add_temp, sizeof(add_temp));
	}

	Serial_Params *serial_params = NULL;;
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");

	guchar request[] = FC_REQ_ADV_INFO;
	write(serial_params->fd, request, sizeof(request));

	total_wanted = FC_ADV_INFO_LEN + 1;

	while ((total_read < total_wanted ) && ((total_wanted-total_read) > 0))
	{
		res = read_wrapper(serial_params->fd,
				ptr+total_read,
				total_wanted-total_read,&len);
		total_read += len;

		/* Increment bad read counter.... */
		if (!res) /* I/O Error Device disappearance or other */
		{
			bad_read = TRUE;
			break;
		}
		if (len == 0) /* Short read!*/
			zerocount++;
		if ((len == 0) && (zerocount > 3))  /* Too many Short reads! */
		{
			bad_read = TRUE;
			break;
		}
	}
	if (bad_read) {
		printf("ERROR: serial read\n");
		return FALSE;
	}
	else {
		if (model == 1)
		{
			fc_adv_info_t *info;
			info = (fc_adv_info_t *)&buf[2];
			struct timeval curTime;
			gettimeofday(&curTime, NULL);
			gint currentTime_uSec = curTime.tv_usec;
			gint Mass = (gint)DATA_GET(global_data, "vehicle_mass");

			//Calculate the extra info components first as they require 'previous' values
			//Power = Mass x Acceleration x Velocity = Mass x (CurrentVelocity - PreviousVelocity) / (CurrentTime - PreviousTime) x CurrentVelocity
			rtv[34] = Mass * ((mul[16] * info->Speed + add[16] - rtv[16]) / 3.6) / (currentTime_uSec - previousTime_uSec) * ((mul[16] * info->Speed + add[16]) / 3.6);

			rtv[0]  = mul[0]  * info->RPM + add[0];
			rtv[1]  = mul[1]  * info->Intakepress + add[1];
			rtv[2]  = mul[2]  * info->PressureV + add[2];
			rtv[3]  = mul[3]  * info->ThrottleV + add[3];
			rtv[4]  = mul[4]  * info->Primaryinp + add[4];
			rtv[5]  = mul[5]  * info->Fuelc + add[5];
			rtv[6]  = mul[6]  * info->Leadingign + add[6];
			rtv[7]  = mul[7]  * info->Trailingign + add[7];
			rtv[8]  = mul[8]  * info->Fueltemp + add[8];
			rtv[9]  = mul[9]  * info->Moilp + add[9];
			rtv[10] = mul[10] * info->Boosttp + add[10];
			rtv[11] = mul[11] * info->Boostwg + add[11];
			rtv[12] = mul[12] * info->Watertemp + add[12];
			rtv[13] = mul[13] * info->Intaketemp + add[13];
			rtv[14] = mul[14] * info->Knock + add[14];
			rtv[15] = mul[15] * info->BatteryV + add[15];
			rtv[16] = mul[16] * info->Speed + add[16];
			rtv[17] = mul[17] * info->Iscvduty + add[17];
			rtv[18] = mul[18] * info->O2volt + add[18];
			rtv[19] = mul[19] * info->na1 + add[19];
			rtv[20] = mul[20] * info->Secinjpulse + add[20];
			rtv[21] = mul[21] * info->na2 + add[21];
		}
		else if (model == 2)
		{
			fc_adv_info_t_2 *info;
			info = (fc_adv_info_t_2 *)&buf[2];
			struct timeval curTime;
			gettimeofday(&curTime, NULL);
			gint currentTime_uSec = curTime.tv_usec;
			gint Mass = (gint)DATA_GET(global_data, "vehicle_mass");

			//Calculate the extra info components first as they require 'previous' values
			//Power = Mass x Acceleration x Velocity = Mass x (CurrentVelocity - PreviousVelocity) / (CurrentTime - PreviousTime) x CurrentVelocity
			rtv[34] = Mass * ((mul[14] * info->Speed + add[14] - rtv[16]) / 3.6) / (currentTime_uSec - previousTime_uSec) * ((mul[14] * info->Speed + add[14]) / 3.6);

			rtv[0] = mul[0] * info->RPM + add[0];
			rtv[1] = mul[1] * info->EngLoad + add[1];
			rtv[2] = mul[2] * info->MAF1V + add[2];
			rtv[3] = mul[3] * info->MAF2V + add[3];
			rtv[4] = mul[4] * info->Primaryinp + add[4];
			rtv[5] = mul[5] * info->Fuelc + add[5];
			rtv[6] = mul[6] * info->Leadingign + add[6];
			rtv[7] = mul[7] * info->Trailingign + add[7];
			rtv[8] = mul[8] * info->BoostPres + add[8];
			if (rtv[8] >= 0x8000)
				rtv[8] = (rtv[8] - 0x8000) * 0.01;
			else
				rtv[8] = (1.0 / 2560 + 0.001) * rtv[8];
			rtv[9] = mul[9] * info->BoostDuty + add[9];
			rtv[10] = mul[10] * info->Watertemp + add[10];
			rtv[11] = mul[11] * info->Intaketemp + add[11];
			rtv[12] = mul[12] * info->Knock + add[12];
			rtv[13] = mul[13] * info->BatteryV + add[13];
			rtv[14] = mul[14] * info->Speed + add[14];
			rtv[15] = mul[15] * info->MAFactivity + add[15];
			rtv[16] = mul[16] * info->O2volt + add[16];
			rtv[17] = mul[17] * info->O2volt_2 + add[17];
			rtv[18] = mul[18] * info->ThrottleV + add[18];
			rtv[19] = mul[19] * info->na1 + add[19];
			rtv[20] = 0;
			rtv[21] = 0;
		}
		else if (model == 3)
		{
			fc_adv_info_t_3 *info;
			info = (fc_adv_info_t_3 *)&buf[2];
			struct timeval curTime;
			gettimeofday(&curTime, NULL);
			gint currentTime_uSec = curTime.tv_usec;
			gint Mass = (gint)DATA_GET(global_data, "vehicle_mass");

			//Calculate the extra info components first as they require 'previous' values
			//Power = Mass x Acceleration x Velocity = Mass x (CurrentVelocity - PreviousVelocity) / (CurrentTime - PreviousTime) x CurrentVelocity
			rtv[34] = Mass * ((mul[14] * info->Speed + add[14] - rtv[16]) / 3.6) / (currentTime_uSec - previousTime_uSec) * ((mul[14] * info->Speed + add[14]) / 3.6);

			rtv[0] = mul[0] * info->RPM + add[0];
			rtv[1] = mul[1] * info->Intakepress + add[1];
			rtv[2] = mul[2] * info->PressureV + add[2];
			rtv[3] = mul[3] * info->ThrottleV + add[3];
			rtv[4] = mul[4] * info->Primaryinp + add[4];
			rtv[5] = mul[5] * info->Fuelc + add[5];
			rtv[6] = mul[6] * info->Leadingign + add[6];
			rtv[7] = mul[7] * info->Trailingign + add[7];
			rtv[8] = mul[8] * info->BoostPres + add[8];
			if (rtv[8] >= 0x8000)
				rtv[8] = (rtv[8] - 0x8000) * 0.01;
			else
				rtv[8] = (1.0 / 2560 + 0.001) * rtv[8];
			rtv[9] = mul[9] * info->BoostDuty + add[9];
			rtv[10] = mul[10] * info->Watertemp + add[10];
			rtv[11] = mul[11] * info->Intaketemp + add[11];
			rtv[12] = mul[12] * info->Knock + add[12];
			rtv[13] = mul[13] * info->BatteryV + add[13];
			rtv[14] = mul[14] * info->Speed + add[14];
			rtv[15] = mul[15] * info->Iscvduty + add[15];
			rtv[16] = mul[16] * info->O2volt + add[16];
			rtv[17] = mul[17] * info->SuctionAirTemp + add[17];
			rtv[18] = mul[18] * info->ThrottleV_2 + add[18];
			rtv[19] = mul[19] * info->na1 + add[19];
			rtv[20] = 0;
			rtv[21] = 0;
		}

		FILE *csvfile;

		csvfile = (FILE *)DATA_GET(global_data,"csvfile");
		if (csvfile != NULL) {
			struct timeval curTime;
			gettimeofday(&curTime, NULL);
			
			previousTime_uSec = curTime.tv_usec;

			int milli = previousTime_uSec / 1000;

			char buffer [80];
			strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));

			char currentTime[84] = "";
			sprintf(currentTime, "%s:%03d", buffer, milli);
			fprintf(csvfile, "%s,%5.0f,%2.4f,%5.0f,%5.0f,%3.4f,%3.4f,%3.0f,%3.0f,%3.0f,%3.4f,%3.4f,%3.4f,%3.0f,%3.0f,%3.0f,%2.4f,%5.0f,%4.4f,%2.4f,%1.4f,%1.4f,%1.4f,%1.4f,%1.4f,%1.4f,%1.4f,%1.4f,%1.4f,%f,%f,%f,%f\n",
					currentTime,
					rtv[0], rtv[1], rtv[2], rtv[3], rtv[4], rtv[5], rtv[6], rtv[7], rtv[8], rtv[9], //Power FC advanced info
					rtv[10], rtv[11], rtv[12], rtv[13], rtv[14], rtv[15], rtv[16], rtv[17], rtv[18], rtv[20], //Power FC advanced info
					rtv[22], rtv[23], rtv[24], rtv[25], rtv[26], rtv[27], rtv[28], rtv[29], //Power FC auxilary info
					rtv[30], rtv[31], rtv[32], rtv[33],  // Analog equation results
					rtv[34]); // Extra info
			fflush(csvfile);
		}
	}
	return TRUE;
}

/*!
	\brief updates the values of all gauges on all dashboards in use
	\param data is unused
	\returns TRUE unless app is closing down
*/
G_MODULE_EXPORT gdouble powerfc_get_current_value(gchar *source)
{
	gint i = 0;
	gint model = 1;
	gchar *model_str = (const gchar *)DATA_GET(global_data, "model");

	if (g_strcmp0(model_str, "Mazda") == 0)
		model = 1;
	else if ((g_strcmp0(model_str, "Nissan") == 0) ||
			(g_strcmp0(model_str, "Subaru") == 0))
		model = 2;
	else if (g_strcmp0(model_str, "Toyota") == 0)
		model = 3;
	
	for (i = 0; i < MAP_ELEMENTS; i++)
	{
		if (model == 1){
			if (g_ascii_strcasecmp(source, map[i]) == 0) break; 
		}
		else if (model == 2){
			if (g_ascii_strcasecmp(source, map2[i]) == 0) break; 
		}
		else if (model == 3){
			if (g_ascii_strcasecmp(source, map3[i]) == 0) break; 
		}
	}

	if (i == MAP_ELEMENTS)
	{
		printf("'%s' is not supported for '%s'\n", source, model_str);
		return 0.0;
	}

	return rtv[i];
}

G_MODULE_EXPORT FILE *powerfc_open_csvfile(gchar *filename)
{
	gint model = 1;
	if (g_strcmp0((const gchar *)DATA_GET(global_data, "model"), "Mazda") == 0)
		model = 1;
	else if ((g_strcmp0((const gchar *)DATA_GET(global_data, "model"), "Nissan") == 0) || 
		    (g_strcmp0((const gchar *)DATA_GET(global_data, "model"), "Subaru") == 0))
		model = 2;
	else if (g_strcmp0((const gchar *)DATA_GET(global_data, "model"), "Toyota") == 0)
		model = 3;

	FILE *csvfile = NULL;

	if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
		csvfile = g_fopen(filename, "ab+");

		if (csvfile == NULL)
			printf("Error opening log file '%s'\n", filename);
	}
	else {
		csvfile = g_fopen(filename, "wb");

		if (csvfile == NULL)
			printf("Error opening log file '%s'\n", filename);
		else{
			if (model == 1)
				fprintf(csvfile, CSV_HEADER_1
				CSV_HEADER_AUX
				CSV_HEADER_ANALOG
				CSV_HEADER_EXTRA);
			else if (model == 2)
				fprintf(csvfile, CSV_HEADER_2
				CSV_HEADER_AUX
				CSV_HEADER_ANALOG
				CSV_HEADER_EXTRA);
			else if (model == 3)
				fprintf(csvfile, CSV_HEADER_3
				CSV_HEADER_AUX
				CSV_HEADER_ANALOG
				CSV_HEADER_EXTRA);
			fflush(csvfile);
		}
	}

	return csvfile;
}
