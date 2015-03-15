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
  \file raspexi/main.c
  \brief Raspexi Viewer main application code
  \author Suriyan Laohaprapanon
 */

#include <dashboard.h>
#include <gauge.h>
#include <stdio.h>
#include <init.h>
#include <serialio.h>
#include <config.h>
#include <configfile.h>
#include <glib.h>
#include <glib/gstdio.h>

#include <widgetmgmt.h>
#include <powerfc.h>
#include <helpers.h>

/* Function prototype */
gboolean update_dash_wrapper(gpointer );

gconstpointer *global_data = NULL;

int main (int argc, char **argv)
{
	Serial_Params *serial_params = NULL;
	GtkWidget *window = NULL;
	GtkWidget *gauge = NULL;
	GtkWidget *dash = NULL;
	GMutex *dash_mutex = NULL;
	GMutex *serio_mutex = NULL;
	gchar *tmpbuf = NULL;
	gint tmpi = 0;
	gint timeout = 0;

#if GLIB_MINOR_VERSION < 32
	g_thread_init(NULL);
#endif
	gdk_threads_init();
	gtk_init (&argc, &argv);

	global_data = g_new0(gconstpointer, 1);

	dash_mutex = g_new0(GMutex, 1);
	g_mutex_init(dash_mutex);
	DATA_SET(global_data,"dash_mutex",dash_mutex);

	serio_mutex = g_new0(GMutex, 1);
	g_mutex_init(serio_mutex);
	DATA_SET(global_data,"serio_mutex",serio_mutex);

	serial_params = (Serial_Params *)g_malloc0(sizeof(Serial_Params));
	DATA_SET(global_data,"serial_params",serial_params);

	ConfigFile *cfgfile;
	cfgfile = cfg_open_file("raspexi.cfg");
	if (cfgfile) {

		// Serial config info
		if (cfg_read_string(cfgfile, "default", "port", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "port", g_strdup(tmpbuf), g_free);
			printf("port = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
		if (cfg_read_string(cfgfile, "default", "baud", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "ecu_baud_str", g_strdup(tmpbuf), g_free);
			printf("baud = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
		if (cfg_read_int(cfgfile, "default", "interval", &tmpi))
		{
			DATA_SET(global_data, "interval", GINT_TO_POINTER(tmpi));
			printf("interval = %d\n", tmpi);
		}

		// Vehicle config info
		if (cfg_read_string(cfgfile, "default", "model", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "model", g_strdup(tmpbuf), g_free);
			printf("model = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}

		// Dashboard config info
		if (cfg_read_string(cfgfile, "default", "dash1", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "dash1", g_strdup(tmpbuf), g_free);
			printf("dash1 = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
		if (cfg_read_string(cfgfile, "default", "dash2", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "dash2", g_strdup(tmpbuf), g_free);
			printf("dash2 = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
		if (cfg_read_string(cfgfile, "default", "dash3", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "dash3", g_strdup(tmpbuf), g_free);
			printf("dash3 = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
		if (cfg_read_string(cfgfile, "default", "dash4", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "dash4", g_strdup(tmpbuf), g_free);
			printf("dash4 = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}

		//Analog/Auxilary config info
		if (cfg_read_string(cfgfile, "default", "analog_eq1", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "analog_eq1", g_strdup(tmpbuf), g_free);
			printf("analog_eq1 = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
		else { DATA_SET_FULL(global_data, "analog_eq1", g_strdup("0 1 0 0"), g_free); }

		if (cfg_read_string(cfgfile, "default", "analog_eq2", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "analog_eq2", g_strdup(tmpbuf), g_free);
			printf("analog_eq2 = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
		else { DATA_SET_FULL(global_data, "analog_eq2", g_strdup("0 1 0 0"), g_free); }

		if (cfg_read_string(cfgfile, "default", "analog_eq3", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "analog_eq3", g_strdup(tmpbuf), g_free);
			printf("analog_eq3 = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
		else { DATA_SET_FULL(global_data, "analog_eq3", g_strdup("0 1 0 0"), g_free); }

		if (cfg_read_string(cfgfile, "default", "analog_eq4", &tmpbuf))
		{
			DATA_SET_FULL(global_data, "analog_eq4", g_strdup(tmpbuf), g_free);
			printf("analog_eq4 = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
		else { DATA_SET_FULL(global_data, "analog_eq4", g_strdup("0 1 0 0"), g_free); }

		//Log file config info
		if (cfg_read_string(cfgfile, "default", "csvfile", &tmpbuf))
		{
			FILE *csvfile = NULL;
			csvfile = powerfc_open_csvfile(tmpbuf);
			if (csvfile == NULL)
				return -1;
			DATA_SET(global_data, "csvfile", csvfile);
			printf("csvfile = %s\n", tmpbuf);
			cleanup(tmpbuf);
		}
	}
	else{
		printf("Cannot open configuration file.\n");
		return -1;
	}

	open_serial((gchar *)DATA_GET(global_data,"port"), FALSE);

	setup_serial_params();

	/* Initialize global vars */
	init();

	gchar *filename = g_strconcat("Dashboards/", (const gchar *)DATA_GET(global_data,"dash1"), NULL);
	printf("Loading dash... (%s)\n", filename);
	dash = load_dashboard(filename, 1);
	register_widget(filename, dash);
	toggle_dash_fullscreen(dash,NULL);
	DATA_SET_FULL(global_data, "active_dash", g_strdup(filename), g_free);

	filename = g_strconcat("Dashboards/", (const gchar *)DATA_GET(global_data,"dash2"), NULL);
	printf("Loading dash... (%s)\n", filename);
	dash = load_dashboard(filename, 2);
	register_widget(filename, dash);

	filename = g_strconcat("Dashboards/", (const gchar *)DATA_GET(global_data,"dash3"), NULL);
	printf("Loading dash... (%s)\n", filename);
	dash = load_dashboard(filename, 3);
	register_widget(filename, dash);

	filename = g_strconcat("Dashboards/", (const gchar *)DATA_GET(global_data,"dash4"), NULL);
	printf("Loading dash... (%s)\n", filename);
	dash = load_dashboard(filename, 4);
	register_widget(filename, dash);

	gtk_widget_add_events(GTK_WIDGET(dash),GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK);

	timeout = g_timeout_add((GINT)DATA_GET(global_data,"interval"),(GSourceFunc)update_dash_wrapper,(gpointer)gauge);

	gdk_threads_enter();
	gtk_main ();
	gdk_threads_leave();
	close_serial();
	return 0;
}

gboolean update_dash_wrapper(gpointer data)
{
	g_idle_add(update_dashboards, data);
	return TRUE;
}
