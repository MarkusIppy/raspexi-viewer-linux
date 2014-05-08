/*
 * Copyright (C) 2014 <COPYRIGHT>
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
  \author <AUTHORS>
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

static gchar *map[] = {"RPM", "Intakepress", "PressureV", "ThrottleV",
						 "Primaryinp", "Fuelc", "Leadingign", "Trailingign",
						 "Fueltemp", "Moilp", "Boosttp", "Boostwg", "Watertemp",
						 "Intaketemp", "Knock", "BatteryV", "Speed", "Iscvduty",
						 "O2volt", "na1", "Secinjpulse", "na2"};
						 
static gdouble rtv[sizeof(map)];

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
G_MODULE_EXPORT gboolean powerfc_process_serial(gpointer data)
{
	gboolean res = 0;
	gboolean bad_read = FALSE;
	gint len = 0;
	gint total_read = 0;
	gint total_wanted;
	gint zerocount = 0;
	guchar buf[4096];
	guchar *ptr = buf;
	gdouble mul[] = FC_ADV_INFO_MUL;
	gdouble add[] = FC_ADV_INFO_ADD;

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
		fc_adv_info_t *info;
		info = (fc_adv_info_t *)&buf[2];
		rtv[0] = mul[0] * info->RPM + add[0];
		rtv[1] = mul[1] * info->Intakepress + add[1];
		rtv[2] = mul[2] * info->PressureV + add[2];
		rtv[3] = mul[3] * info->ThrottleV + add[3];
		rtv[4] = mul[4] * info->Primaryinp + add[4];
		rtv[5] = mul[5] * info->Fuelc + add[5];
		rtv[6] = mul[6] * info->Leadingign + add[6];
		rtv[7] = mul[7] * info->Trailingign + add[7];
		rtv[8] = mul[8] * info->Fueltemp + add[8];
		rtv[9] = mul[9] * info->Moilp + add[9];
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

		FILE *csvfile;

		csvfile = (FILE *)DATA_GET(global_data,"csvfile");
		if (csvfile != NULL) {
			struct timeval curTime;
			gettimeofday(&curTime, NULL);
			int milli = curTime.tv_usec / 1000;

			char buffer [80];
			strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));

			char currentTime[84] = "";
			sprintf(currentTime, "%s:%03d", buffer, milli);

			fprintf(csvfile, "%s,%5.0f,%2.4f,%5.0f,%5.0f,%3.4f,%3.4f,%3.0f,%3.0f,%3.0f,%3.4f,%3.4f,%3.4f,%3.0f,%3.0f,%3.0f,%2.4f,%5.0f,%4.4f,%2.4f,%3.4f\n",
					currentTime, rtv[0], rtv[1], rtv[2], rtv[3], rtv[4], rtv[5], rtv[6], rtv[7], rtv[8], rtv[9], rtv[10], rtv[11], rtv[12], rtv[13], rtv[14],
					rtv[15], rtv[16], rtv[17], rtv[18], rtv[20]);
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
	gint i;

	for (i = 0; i < sizeof(map); i++) {
		if (g_ascii_strcasecmp(source, map[i]) == 0) {
			break;
		}
	}
	if (i == sizeof(map)) {
		return 0.0;
	}
	return rtv[i];
}

G_MODULE_EXPORT FILE *powerfc_open_csvfile(gchar *filename)
{
	FILE *csvfile = NULL;

	if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
		csvfile = g_fopen(filename, "ab+");
	}
	else {
		csvfile = g_fopen(filename, "wb");
			fprintf(csvfile, "time,Engine Speed(rpm),Absolute Intake Pressure(Kg/cm^2),"
				             "Pressure Sensor Voltage(mV),Throttle Voltage(mV),"
				             "Primery Injector Pulse Width(mSec),Fuel Correction,"
				             "Leading Ignition Angle(deg),Trailing Ignition Angle(deg),"
				             "Fuel Templature(deg.C),Metaling Oil PumpDuty(%%),"
				             "\"Boost Duty(Tp, %%)\",\"Boost Duty(Wg, %%)\","
				             "Water Templature(deg.C),Intake Air Templature(deg.C),"
				             "Knocking Level,Battery Voltage(V),Vehicle Speed(Km/h),"
				             "ISCV duty(%%),O2 Sensor Voltage(mV),"
				             "Secondary Injector Pulse Width(mSec)\n");
	}
	return csvfile;
}