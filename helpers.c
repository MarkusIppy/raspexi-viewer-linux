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
  \file raspexi/helpers.c
  \brief Raspexi Viewer helpers and stub functions
  \author <AUTHORS>
 */

#include <args.h>
#include <dataio.h>
#include <debugging.h>
#include <serialio.h>
#include <enums.h>
#include <errno.h>

/* Externs */
extern gconstpointer *global_data;

/* Function prototype */
void dealloc_list(gpointer, gpointer, gpointer);

/*!
  \brief thread_update_logbar() is a function to be called from within threads
  to update a logbar (textview). It's not safe to update a widget from a
  threaded context in win32, hence this fucntion is created to pass the
  information to the main thread via an GAsyncQueue to a dispatcher that will
  take care of the message. Since the functions that call this ALWAYS send
  dynamically allocated test in the msg field we DEALLOCATE it HERE...
  \param view_name is the textual name fothe textview to update (required)
  \param tag_name is the textual name ofthe tag to be applied to the text
  sent.  This can be NULL is no tag is desired
  \param msg is the message to be sent (required)
  \param count is the Flag to display a counter
  \param clear is the Flag to clear the display before writing the text
 */
G_MODULE_EXPORT void  thread_update_logbar(
						const gchar * view_name,
						const gchar * tag_name,
						gchar * msg,
						gboolean count,
						gboolean clear)
{
	/* FIXME: Dummy function require by src/serialio.hc */
	return;
}
/*!
  \brief thread_update_widget() is a function to be called from within threads
  to update a widget (spinner/entry/label). It's not safe to update a
  widget from a threaded context in win32, hence this fucntion is created
  to pass the information to the main thread via an GAsyncQueue to a
  dispatcher that will take care of the message.
  \param widget_name is the textual name of the widget to update
  \param type is the type of widget to update
  \param msg is the message to be sent (required)
 */
G_MODULE_EXPORT void  thread_update_widget(
						const gchar * widget_name,
						WidgetType type,
						gchar * msg)
{
	/* FIXME: Dummy function require by src/serialio.hc */
	return;
}

/*!
  \brief dbg_func() writes debuggging output to the console/log based on if the
  passed debug level is marked in the current debugging mask.
  \param level iss theDbg_Class enumeration defining the debug level
  \param str is the message to print out
 */

G_MODULE_EXPORT void dbg_func(Dbg_Class level, const gchar * file, const gchar * func, gint line, const gchar *format, ...)
{
	/* FIXME: dummy fuction from 'src/debugging.c' */
	return;
}

/*!
  \brief loads a tab when its clicked upon
  \param datamap is the datamap that goes with this tab
  \returns TRUE on success, FALSE otherwise
 */
G_MODULE_EXPORT gboolean handle_dependant_tab_load(gchar * datamap)
{
	/* FIXME: dummy function from 'src/tabloader.c' */
	return FALSE;
}

/* FIXME: grab from 'src/init.c' */

/*!
  \brief deallocates the hashtable of lists
  \param data is the pointer to hashtable to be cleaned up
 */
G_MODULE_EXPORT void dealloc_lists_hash(gpointer data)
{
	ENTER();

	g_hash_table_foreach((GHashTable *)data,(GHFunc)dealloc_list,NULL);
	g_hash_table_destroy((GHashTable *)data);
	EXIT();
	return;
}


/*! 
  \brief Deallocates a linked list
  \param key is unused
  \param value is the pointer to list to be freed
  \param user_data is unused
  */
G_MODULE_EXPORT void dealloc_list(gpointer key, gpointer value, gpointer user_data)
{
	ENTER();

	g_list_free((GList *)value);
	EXIT();
	return;
}


/*!
  \brief generic cleanup function
  \param data is the pointer to arbritrary data to be freed
 */
G_MODULE_EXPORT void cleanup(void *data)
{
	ENTER();

	if (data)
		g_free(data);
	data = NULL;
	EXIT();
	return;
}

/* FIXME: from 'src/init.c' */

/*!
 * init(void)
 * \brief Sets sane values to global variables for a clean startup of 
 * MegaTunix
 */
G_MODULE_EXPORT void init(void)
{
	/* defaults */
	GHashTable *widget_2_tab_hash = NULL;
	if (!widget_2_tab_hash)
	{
			widget_2_tab_hash = g_hash_table_new_full(g_str_hash,g_str_equal,cleanup,cleanup);
			DATA_SET_FULL(global_data,"widget_2_tab_hash",widget_2_tab_hash,g_hash_table_destroy);
	}
}


/*!
	\brief deallocs a dataset
	\param key_id is the keyID to be deallocated
	\param data is unused
	\param user_data is unused
	*/
void dataset_dealloc(GQuark key_id,gpointer data, gpointer user_data)
{
	ENTER();

	printf("removing data for %s\n",g_quark_to_string(key_id));
	g_dataset_remove_data(data,g_quark_to_string(key_id));
	EXIT();
	return;
}


/*!
 * mem_dealloc(void)
 * \brief Deallocates memory allocated with mem_alloc
 * \see mem_alloc
 */
G_MODULE_EXPORT void mem_dealloc(void)
{
	gint i = 0;
	Serial_Params *serial_params = NULL;
	GMutex *serio_mutex = NULL;
	GMutex *mutex = NULL;
	CmdLineArgs *args = NULL;
#ifdef DEBUG
	gchar *tmpbuf = NULL;
#endif

	ENTER();

	args = (CmdLineArgs *)DATA_GET(global_data,"args");
	serial_params = (Serial_Params *)DATA_GET(global_data,"serial_params");
	serio_mutex = (GMutex *)DATA_GET(global_data,"serio_mutex");

	g_mutex_lock(serio_mutex);
	cleanup(serial_params->port_name);
	cleanup(serial_params);
	g_mutex_unlock(serio_mutex);

	mutex = (GMutex *)DATA_GET(global_data,"serio_mutex");
	if (mutex)
	{
		g_mutex_clear(mutex);
		DATA_SET(global_data,"serio_mutex", NULL);
	}

	mutex = (GMutex *)DATA_GET(global_data,"dash_mutex");
	if (mutex)
	{
		g_mutex_clear(mutex);
		g_free(mutex);
		DATA_SET(global_data,"dash_mutex", NULL);
	}

	/* Free all global data and structures */
	printf("Deallocing GLOBAL DATA\n");
	g_dataset_foreach(global_data,dataset_dealloc,NULL);
	g_dataset_destroy(global_data);
	cleanup(global_data);
	EXIT();
	return;
}
