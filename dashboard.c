/*
 * Copyright (C) 2014
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
  \brief Raspexi Viewer application code
  \author Guru Cafe
 */

#include <args.h>
#include <xmlbase.h>
#include <dashboard.h>
#include <defines.h>
#include <debugging.h>
#include <getfiles.h>
#include <gdk/gdkkeysyms.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <string.h>
#include <widgetmgmt.h>

#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#include <powerfc.h>
#include <helpers.h>

GdkColor black = { 0, 0, 0, 0};

extern gconstpointer *global_data;

/* Function prototype */
gboolean leave(GtkWidget *widget, gpointer data);
void toggle_visible(gint i);

/*!
  \brief load_dashboard() loads the specified dashboard configuration file
  and initializes the dash.
  \param filename is the pointer to the file we should load
  \param index is the dash ID (1 or 2)
  \returns pointer to a new dashboard container widget
  */
G_MODULE_EXPORT GtkWidget * load_dashboard(const gchar *filename, gint index)
{
	GtkWidget *window = NULL;
	GtkWidget *dash = NULL;
	GtkWidget *ebox = NULL;
	gchar *key = NULL;
	gchar *prefix = NULL;
	gint width = 0;
	gint height = 0;
	gint x = 0;
	gint y = 0;
	gfloat * ratio = NULL;
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	
	ENTER();

	if (filename == NULL)
		return NULL;

	LIBXML_TEST_VERSION

	/*parse the file and get the DOM */
	doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL)
	{
		printf(_("Error: could not parse dashboard XML file %s"),filename);
		return NULL;
	}

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),_("Raspexi Viewer ("GIT_COMMIT")"));
	gtk_window_set_decorated(GTK_WINDOW(window),FALSE);

	g_signal_connect (G_OBJECT (window), "configure_event",
			G_CALLBACK (dash_configure_event), NULL);
	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (dummy), NULL);
	g_signal_connect (G_OBJECT (window), "destroy_event",
			G_CALLBACK (dummy), NULL);
	ebox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(window),ebox);

	gtk_widget_add_events(GTK_WIDGET(ebox),
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			/*
			GDK_POINTER_MOTION_HINT_MASK |
			GDK_POINTER_MOTION_MASK |
			*/
			GDK_KEY_PRESS_MASK |
			GDK_LEAVE_NOTIFY_MASK
			);

//	g_signal_connect (G_OBJECT (ebox), "motion_notify_event",
//			G_CALLBACK (dash_motion_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "leave-notify-event",
			G_CALLBACK (enter_leave_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "button_release_event",
			G_CALLBACK (dash_button_event), NULL);
	g_signal_connect(G_OBJECT (ebox), "button_press_event",
			G_CALLBACK (dash_button_event), NULL);
	g_signal_connect (G_OBJECT (ebox), "popup-menu",
			G_CALLBACK (dash_popup_menu_handler), NULL);
	g_signal_connect (G_OBJECT (window), "key_press_event",
			G_CALLBACK (dash_key_event), NULL);

	dash = gtk_fixed_new();
	gtk_widget_set_has_window(dash,TRUE);
	gtk_widget_modify_bg(GTK_WIDGET(dash),GTK_STATE_NORMAL,&black);
	OBJ_SET(window,"dash",dash);
	OBJ_SET(ebox,"dash",dash);
	gtk_container_add(GTK_CONTAINER(ebox),dash);

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);
	load_elements(dash,root_element);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	link_dash_datasources(dash,GINT_TO_POINTER(index));

	/* Store global info about this dash */
	prefix = g_strdup_printf("dash_%i",index);
	key = g_strdup_printf("%s_name",prefix);
	DATA_SET_FULL(global_data,key, g_strdup(filename),g_free);
	g_free(key);
	/* retrieve coord info from global store */
	key = g_strdup_printf("%s_x_origin",prefix);
	x = (GINT)DATA_GET(global_data,key);
	g_free(key);
	key = g_strdup_printf("%s_y_origin",prefix);
	y = (GINT)DATA_GET(global_data,key);
	g_free(key);
	key = g_strdup_printf("%s_size_ratio",prefix);
	ratio = (gfloat *)DATA_GET(global_data,key);
	g_free(key);
	g_free(prefix);
	OBJ_SET(ebox,"index", GINT_TO_POINTER(index));

	width = (GINT)OBJ_GET(dash,"orig_width");
	height = (GINT)OBJ_GET(dash,"orig_height");
	/*printf("move/resize to %i,%i, %ix%i\n",x,y,width,height); */
	gtk_window_move(GTK_WINDOW(window), x,y);
	if (ratio)
		gtk_window_set_default_size(GTK_WINDOW(window), (GINT)(width*(*ratio)),(GINT)(height*(*ratio)));
	else
		gtk_window_set_default_size(GTK_WINDOW(window), width,height);
	if (index == 1) {
		gtk_widget_show_all(window);
	}
	dash_shape_combine(dash,TRUE);
	return window;
}


/*!
  \brief dashboard configure event.  Handles the dashboard setup and render
  of the spots for each gauge and initiates the shape combine
  \param widget is the pointer to the dash drawing area
  \param event is the pointer to the GdkEventConfigure event structure
  \returns FALSE so other signals run
  */
G_MODULE_EXPORT gboolean dash_configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
	gint orig_width = 0;
	gint orig_height = 0;
	gint cur_width = 0;
	gint cur_height = 0;
	gfloat x_ratio = 0.0;
	gfloat y_ratio = 0.0;
	gboolean w_constricted = FALSE;
	GtkWidget *gauge = NULL;
	GList *children = NULL;
	guint i = 0;
	gint child_w = 0;
	gint child_h = 0;
	gint child_x = 0;
	gint child_y = 0;
	gfloat ratio = 0.0;
	GtkWidget * dash  = NULL;
	
	ENTER();

	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
		return FALSE;

	if (OBJ_GET(dash,"moving"))
		return FALSE;

	orig_width = (GINT) OBJ_GET(dash,"orig_width");
	orig_height = (GINT) OBJ_GET(dash,"orig_height");
	cur_width = event->width;
	cur_height = event->height;

	x_ratio = (float)cur_width/(float)orig_width;
	y_ratio = (float)cur_height/(float)orig_height;
	ratio = x_ratio > y_ratio ? y_ratio:x_ratio;
	w_constricted = x_ratio > y_ratio ? FALSE:TRUE;

	g_signal_handlers_block_by_func(G_OBJECT(widget),(gpointer)dash_configure_event,NULL);
	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		child_x = (GINT)OBJ_GET(gauge,"orig_x_offset");
		child_y = (GINT)OBJ_GET(gauge,"orig_y_offset");
		child_w = (GINT)OBJ_GET(gauge,"orig_width");
		child_h = (GINT)OBJ_GET(gauge,"orig_height");
		if (w_constricted)
			gtk_fixed_move(GTK_FIXED(dash),gauge,ratio*child_x,ratio*child_y+(((y_ratio-x_ratio)*orig_height)/2));
		else
			gtk_fixed_move(GTK_FIXED(dash),gauge,ratio*child_x-(((y_ratio-x_ratio)*orig_width)/2),ratio*child_y);
		gtk_widget_set_size_request(gauge,child_w*ratio,child_h*ratio);
	}
	dash_shape_combine(dash,FALSE);

	g_signal_handlers_unblock_by_func(G_OBJECT(widget),(gpointer)dash_configure_event,NULL);
	return FALSE;
}


/*!
  \brief XML processing function to load the elements for each dashboard
  \param dash is the pointer to dashboard window
  \param a_node is the pointer to XML node
  */
G_MODULE_EXPORT void load_elements(GtkWidget *dash, xmlNode *a_node)
{
	xmlNode *cur_node = NULL;
	
	ENTER();

	/* Iterate though all nodes... */
	for (cur_node = a_node;cur_node;cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"dash_geometry") == 0)
				load_geometry(dash,cur_node);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"gauge") == 0)
				load_gauge(dash,cur_node);
		}
		load_elements(dash,cur_node->children);
	}
}


/*!
  \brief XML processing function to load the geometry data for the dashboard
  \param dash is the pointer to dashboard widget
  \param node is the pointer to XML node
  */
G_MODULE_EXPORT void load_geometry(GtkWidget *dash, xmlNode *node)
{
	GdkGeometry hints;
	xmlNode *cur_node = NULL;
	gint width = 0;
	gint height = 0;
	
	ENTER();
	if (!node->children)
	{
		printf(_("ERROR, load_geometry, xml node is empty!!\n"));
		return;
	}
	cur_node = node->children;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"width") == 0)
				generic_xml_gint_import(cur_node,&width);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"height") == 0)
				generic_xml_gint_import(cur_node,&height);
		}
		cur_node = cur_node->next;

	}
	OBJ_SET(dash,"orig_width", GINT_TO_POINTER(width));
	OBJ_SET(dash,"orig_height", GINT_TO_POINTER(height));
	
	hints.min_width = 100;
	hints.min_height = 100;

	gtk_window_set_geometry_hints(GTK_WINDOW(gtk_widget_get_toplevel(dash)),NULL,&hints,GDK_HINT_MIN_SIZE);

}


/*!
  \brief XML processing function to load the gauge data for the dashboard
  \param dash is the pointer to dashboard widget
  \param node is the pointer to XML node
  */
G_MODULE_EXPORT void load_gauge(GtkWidget *dash, xmlNode *node)
{
	xmlNode *cur_node = NULL;
	GtkWidget *gauge = NULL;
	GList *children = NULL;
	gchar * filename = NULL;
	gint width = 0;
	gint height = 0;
	gint x_offset = 0;
	gint y_offset = 0;
	gchar *xml_name = NULL;
	gchar *datasource = NULL;
	gchar *pathstub = NULL;
	
	ENTER();

	if (!node->children)
	{
		printf(_("ERROR, load_gauge, xml node is empty!!\n"));
		return;
	}
	cur_node = node->children;
	while (cur_node->next) { if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"width") == 0)
				generic_xml_gint_import(cur_node,&width);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"height") == 0)
				generic_xml_gint_import(cur_node,&height);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"x_offset") == 0)
				generic_xml_gint_import(cur_node,&x_offset);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"y_offset") == 0)
				generic_xml_gint_import(cur_node,&y_offset);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"gauge_xml_name") == 0)
				generic_xml_gchar_import(cur_node,&xml_name);
			if (g_ascii_strcasecmp((gchar *)cur_node->name,"datasource") == 0)
				generic_xml_gchar_import(cur_node,&datasource);
		}
		cur_node = cur_node->next;

	}
	if (xml_name && datasource)
	{
		gauge = mtx_gauge_face_new();
		gtk_fixed_put(GTK_FIXED(dash),gauge,x_offset,y_offset);
		children = OBJ_GET(dash,"children");
		children = g_list_prepend(children,gauge);
		OBJ_SET(dash,"children",children);
		xml_name = g_strdelimit(xml_name,"\\",'/');
		pathstub = g_build_filename(GAUGES_DATA_DIR,xml_name,NULL);
		filename = get_file((const gchar *)DATA_GET(global_data,"project_name"),pathstub,NULL);
		g_free(pathstub);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		gtk_widget_set_size_request(gauge,width,height);
		g_free(filename);
		OBJ_SET_FULL(gauge,"datasource",g_strdup(datasource),g_free);
		OBJ_SET(gauge,"orig_width",GINT_TO_POINTER(width));
		OBJ_SET(gauge,"orig_height",GINT_TO_POINTER(height));
		OBJ_SET(gauge,"orig_x_offset",GINT_TO_POINTER(x_offset));
		OBJ_SET(gauge,"orig_y_offset",GINT_TO_POINTER(y_offset));
		g_free(xml_name);
		g_free(datasource);
	}

}


/*!
  \brief Links the dashboard datasources defined in the XML to actual 
  datasources within megatunix itself (match is via name)
  \param dash is the pointer to dashboard widget
  \param data is unused
  */
G_MODULE_EXPORT void link_dash_datasources(GtkWidget *dash,gpointer data)
{
	Dash_Gauge *d_gauge = NULL;
	GList *children = NULL;
	GtkWidget *cwidget = NULL;
	gint len = 0;
	gint i = 0;
	GData * rtv_obj = NULL;
	gchar * source = NULL;
	GHashTable *dash_hash = NULL;
	
	ENTER();

	if(!GTK_IS_FIXED(dash))
		return;
	
	dash_hash = (GHashTable *)DATA_GET(global_data,"dash_hash");
	if (!dash_hash)
	{
		dash_hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
		DATA_SET_FULL(global_data,"dash_hash",dash_hash,(GDestroyNotify)g_hash_table_destroy);
	}

	children = (GList *)OBJ_GET(dash,"children");
	len = g_list_length(children);

	for (i=0;i<len;i++)
	{
		cwidget = (GtkWidget *)g_list_nth_data(children,i);
		source = (gchar *)OBJ_GET(cwidget,"datasource");
		printf("data source = %s\n", source);
		if (!source)
			continue;
		d_gauge = g_new0(Dash_Gauge, 1);
		d_gauge->source = g_strdup(source);
		d_gauge->gauge = cwidget;
		d_gauge->dash = dash;
		g_hash_table_insert(dash_hash,g_strdup_printf("dash_%i_gauge_%i",(GINT)data,i),(gpointer)d_gauge);
	}
}


/*!
  \brief Random number between min/max value

  \param min_num minimum value
  \param max_num maximum value
  */
int random_number(int min_num, int max_num)
{
	int result=0,low_num=0,hi_num=0;
	if(min_num<max_num)
	{
		low_num=min_num;
		hi_num=max_num+1; // this is done to include max_num in output.
	}else{
		low_num=max_num+1;// this is done to include max_num in output.
		hi_num=min_num;
	}
	srand(time(NULL));
	result = (rand()%(hi_num-low_num))+low_num;
	return result;
}

/*!
  \brief Updates a dashboard gauge with a new value
  \param key is the gauge name
  \param value is the pointer to Dash_Gauge structure
  \param user_data is unused
  */
G_MODULE_EXPORT void update_dash_gauge(gpointer key, gpointer value, gpointer user_data)
{
	Dash_Gauge *d_gauge = (Dash_Gauge *)value;
	GArray *history;
	gfloat current = 0.0;
	gfloat previous = 0.0;
	GtkWidget *gauge = NULL;
	
	ENTER();

	static gfloat lower = 0.0;
	static gfloat upper = 0.0;
	gint i;

	gauge = d_gauge->gauge;
	gchar *datasource = (gchar *)OBJ_GET(gauge,"datasource");

	//current = rtv[i];
	current = powerfc_get_current_value(datasource);
	//current = random_number((int)lower, (int)upper);

	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), LBOUND, &lower);
	mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge), UBOUND, &upper);
	if (current < lower) current = lower;
	if (current > upper) current = upper;
	printf("%s ==> %f\n", datasource, current);

	mtx_gauge_face_get_value(MTX_GAUGE_FACE (gauge), &previous);

	if ((current != previous) || 
			(DATA_GET(global_data,"forced_update")))
	{
		/*printf("updating gauge %s\n",(gchar *)key);*/
		/*printf("updating gauge %s\n",mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(gauge)));*/
		mtx_gauge_face_set_value(MTX_GAUGE_FACE(gauge),current);
	}

}


/*!
  \brief gives the dashboard that floating look without a bounding window/box
  \param dash is the pointer to dashboard widget
  \param hide_resizers is the flag to display or hide the dashboard resizers
  */
G_MODULE_EXPORT void dash_shape_combine(GtkWidget *dash, gboolean hide_resizers)
{
	GtkWidget *cwidget = NULL;
	cairo_t *cr = NULL;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	gint xc = 0;
	gint yc = 0;
	gint radius = 0;
	guint i = 0;
	GList *children = NULL;
	GdkBitmap *bitmap = NULL;
	GtkRequisition req;
	GtkAllocation alloc;
	gint width = 0;
	gint height = 0;
	GMutex *dash_mutex = (GMutex *)DATA_GET(global_data,"dash_mutex");
	GdkWindow *window = NULL;
	
	ENTER();

	if(!GTK_IS_WIDGET(dash))
		return;
	if(!GTK_IS_WINDOW(gtk_widget_get_toplevel(dash)))
		return;
	g_mutex_lock(dash_mutex);

	window = gtk_widget_get_window(dash);
	gtk_window_get_size(GTK_WINDOW(gtk_widget_get_toplevel(dash)),&width,&height);
	bitmap = gdk_pixmap_new(NULL,width,height,1);
	cr = gdk_cairo_create(bitmap);
	cairo_set_operator(cr,CAIRO_OPERATOR_DEST_OUT);
	cairo_paint(cr);
	cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgb(cr, 1.0,1.0,1.0);
	if (hide_resizers == FALSE)
	{
		cairo_rectangle(cr,0,0,16,3);
		cairo_rectangle(cr,0,0,3,16);
		cairo_rectangle(cr,width-16,0,16,3);
		cairo_rectangle(cr,width-3,0,3,16);
		cairo_rectangle(cr,width-16,height-3,16,3);
		cairo_rectangle(cr,width-3,height-16,3,16);
		cairo_rectangle(cr,0,height-3,16,3);
		cairo_rectangle(cr,0,height-16,3,16);
		cairo_fill(cr);
	}

	if ((GBOOLEAN)DATA_GET(global_data,"dash_fullscreen"))
		cairo_rectangle(cr,0,0,width,height);

	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		cwidget = (GtkWidget *)g_list_nth_data(children,i);
		gtk_widget_get_allocation(cwidget,&alloc);
		x = alloc.x;
		y = alloc.y;
		gtk_widget_size_request(cwidget,&req);
		w = req.width;
		h = req.height;
		radius = MIN(w,h)/2;
		xc = x+w/2;
		yc = y+h/2;

		cairo_arc(cr,xc,yc,radius,0,2 * M_PI);
		cairo_fill(cr);
		cairo_stroke(cr);
	}
	cairo_destroy(cr);
	if (GTK_IS_WINDOW(gtk_widget_get_toplevel(dash)))
	{
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
		{
			gtk_widget_input_shape_combine_mask(gtk_widget_get_toplevel(dash),bitmap,0,0);
		}
#endif
		gtk_widget_shape_combine_mask(gtk_widget_get_toplevel(dash),bitmap,0,0);
	}
	else
	{
#if GTK_MINOR_VERSION >= 10
		if (gtk_minor_version >= 10)
		{
			gdk_window_input_shape_combine_mask(window,bitmap,0,0);
		}
#endif
		gdk_window_shape_combine_mask(window,bitmap,0,0);
	}
	g_object_unref(bitmap);
	g_mutex_unlock(dash_mutex);
	return;
}


/*!
  \brief Dashboard motion event handler (not used yet)
  \param widget is unused
  \param event is a pointer to GdkEventMotion structure
  \param data is unused
  \return FALSE so other handlers run
  */
/*
G_MODULE_EXPORT gboolean dash_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	int x,y;
	GdkModifierType state;
	ENTER();
	if (event->is_hint)
	{
		gdk_window_get_pointer(event->window, &x, &y, &state);
		printf("motion hint, at %i,%i\n",x,y);
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
		printf("motion at %i,%i\n",x,y);
	}
	printf("dash motion event!\n");
	EXIT();
	return FALSE;
}
*/


/*!
  \brief Dashboard keyboard event handler that handles dashboard hotkeys
  \param widget is th pointer to dashboard widget
  \param event is th pointer to a GdkEventKey structure
  \param data is unused
  \return FALSE so other handlers run
  */
G_MODULE_EXPORT gboolean dash_key_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	gboolean retval = FALSE;
	ENTER();
	printf("Dash Key press: %d\n", event->keyval);
	if (event->type == GDK_KEY_RELEASE)
	{
		EXIT();
		return FALSE;
	}

	switch (event->keyval)
	{
		case GDK_q:
		case GDK_Q:
			leave(widget, NULL);
			retval = TRUE;
			break;
		case GDK_1:
			toggle_visible(1);
			break;
		case GDK_2:
			toggle_visible(2);
			break;
		case GDK_3:
			toggle_visible(3);
			break;
		case GDK_4:
			toggle_visible(4);
			break;
		case GDK_f:
		case GDK_F:
			toggle_dash_fullscreen(widget,NULL);
			retval = TRUE;
		case GDK_T:
		case GDK_t:
			dash_toggle_attribute(widget,TATTLETALE);
			retval = TRUE;
		case GDK_A:
		case GDK_a:
			dash_toggle_attribute(widget,ANTIALIAS);
			retval = TRUE;
			break;
	}
	EXIT();
	return retval;
}


/*!
  \brief toggle dash board visible by ID 1-4
  */
void toggle_visible(gint i)
{
	const gchar *active;
	gchar *dashname = NULL;
	GtkWidget *tmpwidget = NULL;

	switch (i) {
		case 1:
			dashname = g_strconcat("Dashboards/", (const gchar *)DATA_GET(global_data,"dash1"), NULL);
		break;
		case 2:
			dashname = g_strconcat("Dashboards/", (const gchar *)DATA_GET(global_data,"dash2"), NULL);
		break;
		case 3:
			dashname = g_strconcat("Dashboards/", (const gchar *)DATA_GET(global_data,"dash3"), NULL);
		break;
		case 4:
			dashname = g_strconcat("Dashboards/", (const gchar *)DATA_GET(global_data,"dash4"), NULL);
		break;
	}
	tmpwidget = lookup_widget(dashname);

	ENTER();
	if (!GTK_IS_WIDGET(tmpwidget))
	{
		g_free(dashname);
		EXIT();
		return;
	}

	active = (const gchar *)DATA_GET(global_data,"active_dash");
	GtkWidget *activewidget = lookup_widget(active);
	gtk_widget_hide(activewidget);

	gtk_widget_show_all(tmpwidget);
	DATA_SET_FULL(global_data,"active_dash", dashname, g_free);
	if ((GBOOLEAN)DATA_GET(global_data,"dash_fullscreen")) {
		if ((GBOOLEAN)OBJ_GET(tmpwidget,"dash_on_top"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(tmpwidget)),NULL);
		gtk_window_fullscreen(GTK_WINDOW(gtk_widget_get_toplevel(tmpwidget)));
	}
	EXIT();
	return;
}


/*!
  \brief Turns on/off gauge attributes of a dash (tattletales, etc )
  \param widget is the pointer to dashboard widget 
  \param attr is the Enumeration for type of attribute
  */
G_MODULE_EXPORT void dash_toggle_attribute(GtkWidget *widget,MtxGenAttr attr)
{
	GList *children = NULL;
	guint i = 0;
	gboolean state = FALSE;
	GtkWidget * dash  = NULL;
	GtkWidget * gauge  = NULL;
	gchar * text_attr = NULL;

	ENTER();

	text_attr = g_strdup_printf("%i",attr);
	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
	{
		printf(_("dashboard widget is null cannot set attribute(s)!\n"));
		EXIT();
		return;
	}
	children = (GList *)OBJ_GET(dash,"children");
	if ((GBOOLEAN)OBJ_GET(dash,text_attr))
		state = FALSE;
	else
		state = TRUE;
	OBJ_SET(dash,text_attr,GINT_TO_POINTER(state));
	g_free(text_attr);
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		mtx_gauge_face_set_attribute(MTX_GAUGE_FACE(gauge),attr,(gfloat)state);
	}
	EXIT();
	return;
}


/*!
  \brief Queries the status of gauge attirbutes for a dashboard
  \param widget is the pointer to dashboard widget 
  \param attr is the Enumeration for type of attribute
  */
G_MODULE_EXPORT gboolean dash_lookup_attribute(GtkWidget *widget, MtxGenAttr attr)
{
	gchar * text_attr = NULL;
	GtkWidget * dash  = NULL;
	GList *children = NULL;
	GtkWidget * gauge  = NULL;
	gfloat tmpf = 0.0;
	guint i = 0;
	gint t_count = 0;
	gint f_count = 0;

	ENTER();

	text_attr = g_strdup_printf("%i",attr);
	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	g_free(text_attr);
	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		mtx_gauge_face_get_attribute(MTX_GAUGE_FACE(gauge),attr,&tmpf);
		if ((GBOOLEAN)tmpf)
			t_count++;
		else
			f_count++;
	}
	EXIT();
	if (t_count > f_count)
		return TRUE;
	else
		return FALSE;
}


/*!
  \brief Pops up the menu for the dashboard when right clicked upon the dash
  \param widget is the pointer to dashboard window
  \returns TRUE to block any other handlers from running
  */
G_MODULE_EXPORT gboolean dash_popup_menu_handler(GtkWidget *widget)
{
	ENTER();
	dash_context_popup(widget, NULL);
	EXIT();
	return TRUE;
}


/*!
  \brief Pops up the menu for the dashboard when right clicked upon the dash
  \param widget is the pointer to dashboard window
  \param event is the pointer to a GdkEventButton structure
  */
G_MODULE_EXPORT void dash_context_popup(GtkWidget *widget, GdkEventButton *event)
{
	static GtkWidget *menu = NULL;
	GtkWidget *item = NULL;
	GtkWidget *d_item = NULL;
	GtkWidget *n_item = NULL;
	gint button = 0;
	gint event_time = 0;
	GtkWidget *dash = gtk_bin_get_child(GTK_BIN(widget));

	ENTER();

	menu = gtk_menu_new();

	/* Create Menu here */
	d_item = gtk_radio_menu_item_new_with_label(NULL,"Daytime Mode");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(d_item),get_dash_daytime_mode(gtk_widget_get_toplevel(widget)));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),d_item);
	n_item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(d_item),"Nitetime Mode");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(n_item),!get_dash_daytime_mode(gtk_widget_get_toplevel(widget)));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),n_item);
	g_signal_connect(G_OBJECT(d_item),"toggled",
			G_CALLBACK(set_dash_time_mode),(gpointer)gtk_widget_get_toplevel(widget));

	item = gtk_check_menu_item_new_with_label("Show Tattletales");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),dash_lookup_attribute(gtk_widget_get_toplevel(widget),TATTLETALE));
	g_signal_connect(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_dash_tattletales),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_menu_item_new_with_label("Reset Tattletales");
	gtk_widget_set_sensitive(item,dash_lookup_attribute(gtk_widget_get_toplevel(widget),TATTLETALE));
	g_signal_connect(G_OBJECT(item),"activate",
			G_CALLBACK(reset_dash_tattletales),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_check_menu_item_new_with_label("Antialiasing");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),dash_lookup_attribute(gtk_widget_get_toplevel(widget),ANTIALIAS));
	g_signal_connect(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_dash_antialias),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_check_menu_item_new_with_label("Fullscreen");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),(GBOOLEAN)DATA_GET(global_data,"dash_fullscreen"));
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_dash_fullscreen),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_check_menu_item_new_with_label("Stay on Top");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),(GBOOLEAN)OBJ_GET(dash,"dash_on_top"));
	g_signal_connect_swapped(G_OBJECT(item),"toggled",
			G_CALLBACK(toggle_dash_on_top),(gpointer)widget);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	item = gtk_menu_item_new_with_label("Quit Raspexi Viewer");
	g_signal_connect(G_OBJECT(item),"activate",
			G_CALLBACK(leave),NULL);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

	if (event)
	{
		button = event->button;
		event_time = event->time;
	}
	else
	{
		button = 0;
		event_time = gtk_get_current_event_time ();
	}

	gtk_menu_attach_to_widget (GTK_MENU (menu), widget, NULL);
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 
			button, event_time);
	gtk_widget_show_all(menu);
	EXIT();
	return;
}


/*!
  \brief Enables or disables the dashboard tattletales...
  \param menuitem is the pointer to context menuitem
  \param data is the pointer to dashboard widget
  \returns TRUE
  */
G_MODULE_EXPORT gboolean toggle_dash_tattletales(GtkWidget *menuitem, gpointer data)
{
	GtkWidget *widget = (GtkWidget *)data;
	ENTER();
	dash_toggle_attribute(gtk_widget_get_toplevel(widget),TATTLETALE);
	EXIT();
	return TRUE;
}


/*!
  \brief Conext menu handler that Sets the dashboard to daytime or nitetime mode
  \param menuitem is the pointer to context menuitem
  \param data is the pointer to dashboard widget
  \returns TRUE
  */
G_MODULE_EXPORT gboolean set_dash_time_mode(GtkWidget *menuitem, gpointer data)
{
	gboolean value;
	GtkWidget *widget = (GtkWidget *)data;
	ENTER();
	g_object_get(menuitem,"active",&value, NULL);
	set_dash_daytime_mode(widget,value);
	EXIT();
	return TRUE;
}


/*!
  \brief Gets the dashboard to daytime or nitetime mode
  \param widget is the pointer to dashboard widget
  \returns TRUE if daytime, otherwise FALSE
  */
G_MODULE_EXPORT gboolean get_dash_daytime_mode(GtkWidget *widget)
{
	GtkWidget * dash  = NULL;
	GList *children = NULL;
	GtkWidget * gauge  = NULL;
	guint i = 0;
	gint t_count = 0;
	gint f_count = 0;

	ENTER();

	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		if(mtx_gauge_face_get_daytime_mode(MTX_GAUGE_FACE(gauge)))
			t_count++;
		else
			f_count++;
	}
	EXIT();
	if (t_count > f_count)
		return TRUE;
	else
		return FALSE;
}


/*!
  \brief Sets the dashboard to daytime or nitetime mode
  \param widget is the pointer to dashboard widget
  \param state is the flag whether we are daytime or nitetime
  */
G_MODULE_EXPORT void set_dash_daytime_mode(GtkWidget *widget, gboolean state)
{
	GtkWidget * dash  = NULL;
	GList *children = NULL;
	GtkWidget * gauge  = NULL;
	guint i = 0;

	ENTER();

	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	children = (GList *)OBJ_GET(dash, "children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		mtx_gauge_face_set_daytime_mode(MTX_GAUGE_FACE(gauge),state);
	}
	EXIT();
	return;
}


/*!
  \brief resets the dashboard tattletales for all gauges 
  \param menuitem is the pointer to the dash context menuitem
  \param data is unused
  \returns TRUE if successfull, FALSE otherwise
  */
G_MODULE_EXPORT gboolean reset_dash_tattletales(GtkWidget *menuitem, gpointer data)
{
	GList *children = NULL;
	guint i = 0;
	GtkWidget * widget  = NULL;
	GtkWidget * dash  = NULL;
	GtkWidget * gauge  = NULL;

	ENTER();

	widget = gtk_widget_get_toplevel(GTK_WIDGET(data));
	dash = (GtkWidget *)OBJ_GET(widget,"dash");
	if (!GTK_IS_WIDGET(dash))
	{
		printf(_("dashboard widget is null cannot reset tattletale!\n"));
		EXIT();
		return FALSE;
	}
	children = (GList *)OBJ_GET(dash,"children");
	for (i=0;i<g_list_length(children);i++)
	{
		gauge = (GtkWidget *)g_list_nth_data(children,i);
		mtx_gauge_face_clear_peak(MTX_GAUGE_FACE(gauge));
	}
	EXIT();
	return TRUE;
}


/*!
  \brief Toggles the dashboard antialiasing
  \param menuitem is the dash context menu item
  \param data is the pointer to dashboard widget
  \returns TRUE
  */
G_MODULE_EXPORT gboolean toggle_dash_antialias(GtkWidget *menuitem, gpointer data)
{
	GtkWidget *widget = (GtkWidget *)data;
	ENTER();
	dash_toggle_attribute(gtk_widget_get_toplevel(widget),ANTIALIAS);
	EXIT();
	return TRUE;
}



/*!
  \brief Dash mouse button event handler to handler move/resize
  \param widget is the pointer to dashboard widget
  \param event is the pointer to EventButton structure.
  \param data is unused
  \returns TRUE, if it handles something, otherwise FALSE
  */
G_MODULE_EXPORT gboolean dash_button_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkAllocation allocation;

	ENTER();

	gtk_widget_get_allocation(widget,&allocation);
	GtkWidget *dash = gtk_bin_get_child(GTK_BIN(widget));
	if (!OBJ_GET(dash,"resizers_visible"))
	{
		dash_shape_combine(dash,FALSE);
		OBJ_SET(dash,"resizers_visible",GINT_TO_POINTER(TRUE));
	}
	if ((event->type == GDK_BUTTON_RELEASE) && (event->button == 1))
	{
		OBJ_SET(dash,"moving",GINT_TO_POINTER(FALSE));
		OBJ_SET(dash,"resizing",GINT_TO_POINTER(FALSE));
		EXIT();
		return TRUE;
	}

	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
	{
		dash_context_popup(widget,event);
		EXIT();
		return TRUE;
	}
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 1))
	{
		gint edge = -1;
		/*printf("dash button event\n"); */
		if (event->x > (allocation.width-16))
		{
			/* Upper portion */
			if (event->y < 16)
				edge = GDK_WINDOW_EDGE_NORTH_EAST;
			/* Lower portion */
			else if (event->y > (allocation.height-16))
				edge = GDK_WINDOW_EDGE_SOUTH_EAST;
			else
				edge = -1;
		}
		/* Left Side of window */
		else if (event->x < 16)
		{
			/* If it's in the middle portion */
			/* Upper portion */
			if (event->y < 16)
				edge = GDK_WINDOW_EDGE_NORTH_WEST;
			/* Lower portion */
			else if (event->y > (allocation.height-16))
				edge = GDK_WINDOW_EDGE_SOUTH_WEST;
			else
				edge = -1;
		}
		else
			edge = -1;

		if ((edge == -1 ) && (GTK_IS_WINDOW(gtk_widget_get_parent(widget))))
		{
			/*printf("MOVE drag\n"); */
			OBJ_SET(dash,"moving",GINT_TO_POINTER(TRUE));
			gtk_window_begin_move_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
					event->button,
					event->x_root,
					event->y_root,
					event->time);
			EXIT();
			return TRUE;
		}
		else if (GTK_IS_WINDOW(gtk_widget_get_parent(widget)))
		{
			/*printf("RESIZE drag\n"); */
			OBJ_SET(dash,"resizing",GINT_TO_POINTER(TRUE));
			gtk_window_begin_resize_drag (GTK_WINDOW(gtk_widget_get_toplevel(widget)),
					(GdkWindowEdge)edge,
					event->button,
					event->x_root,
					event->y_root,
					event->time);
		}
	}
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 2))
	{
		toggle_dash_fullscreen(widget,NULL);
		EXIT();
		return TRUE;
	}
	EXIT();
	return FALSE;
}


/*! 
  \brief dummy function that jsut returns true to block signal...
  \param widget is unused
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean dummy(GtkWidget *widget,gpointer data)
{
	ENTER();
	EXIT();
	return TRUE;
}


/*!
  \brief wrapper to call the real function via g_idle_add
  \param data is the pointer to dashboard widget
  \returns FALSE
  */
G_MODULE_EXPORT gboolean hide_dash_resizers_wrapper(gpointer data)
{
	ENTER();
	g_idle_add(hide_dash_resizers,data);
	EXIT();
	return FALSE;
}

/*!
  \brief Hides dashboard resizers
  \param data is the pointer to dashboard widget
  \returns FALSE
  */
G_MODULE_EXPORT gboolean hide_dash_resizers(gpointer data)
{
	ENTER();
	if (!data)
	{
		EXIT();
		return FALSE;
	}
	if ((GTK_IS_WIDGET(data)) && (OBJ_GET(data,"resizers_visible")))
		dash_shape_combine((GtkWidget *)data,TRUE);
	OBJ_SET(data,"timer_active",GINT_TO_POINTER(FALSE));
	OBJ_SET(data,"timer_id",GINT_TO_POINTER(0));
	OBJ_SET(data,"resizers_visible",GINT_TO_POINTER(FALSE));
	EXIT();
	return FALSE;
}


/*!
  \brief Enter/Leave Event handler
  \param widget is the pointer to dash window/eventbox
  \param event is the pointer to a GdkEventCrossing structure
  \param data is unused
  \returns FALSE normally
  */
G_MODULE_EXPORT gboolean enter_leave_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	GtkWidget *dash = gtk_bin_get_child(GTK_BIN(widget));

	ENTER();

	if (event->state & GDK_BUTTON1_MASK)
	{
		EXIT();
		return TRUE;
	}
	OBJ_SET(dash,"moving",GINT_TO_POINTER(FALSE));
	OBJ_SET(dash,"resizing",GINT_TO_POINTER(FALSE));
	/* If "leaving" the window, set timeout to hide the resizers */
	if ((!OBJ_GET(dash,"timer_active")) && (OBJ_GET(dash,"resizers_visible")))
	{
		guint id = g_timeout_add(5000,hide_dash_resizers_wrapper,dash);
		OBJ_SET(dash,"timer_active",GINT_TO_POINTER(TRUE));
		OBJ_SET(dash,"timer_id",GINT_TO_POINTER(id));
	}
	EXIT();
	return FALSE;
}


/*!
  \brief Toggles the dashboard fullscreen status
  \param widget is the pointer to dashboard widget
  \param data is unused
  */
G_MODULE_EXPORT void toggle_dash_fullscreen(GtkWidget *widget, gpointer data)
{
	GtkWidget *dash = (GtkWidget *)OBJ_GET(widget,"dash");

	ENTER();

	if ((GBOOLEAN)DATA_GET(global_data,"dash_fullscreen"))
	{
			DATA_SET(global_data,"dash_fullscreen",GINT_TO_POINTER(FALSE));
		gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);
		gtk_window_unfullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
		if ((GBOOLEAN)OBJ_GET(dash,"dash_on_top"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),GTK_WINDOW(lookup_widget("main_window")));
	}
	else
	{
			DATA_SET(global_data,"dash_fullscreen",GINT_TO_POINTER(TRUE));
		if ((GBOOLEAN)OBJ_GET(dash,"dash_on_top"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);
		gtk_window_fullscreen(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
	}
	EXIT();
	return;
}


/*!
  \brief Toggles the dashboard force on top function
  \param widget is the pointer to dashboard widget
  \param data is unused
  */
G_MODULE_EXPORT void toggle_dash_on_top(GtkWidget *widget, gpointer data)
{
	GtkWidget *dash = gtk_bin_get_child(GTK_BIN(widget));

	ENTER();

	if ((GBOOLEAN)OBJ_GET(dash,"dash_on_top"))
	{
			OBJ_SET(dash,"dash_on_top",GINT_TO_POINTER(FALSE));
		gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);
	}
	else
	{
		if (!(GBOOLEAN)DATA_GET(global_data,"dash_fullscreen"))
			gtk_window_set_transient_for(GTK_WINDOW(gtk_widget_get_toplevel(widget)),GTK_WINDOW(lookup_widget("main_window")));
			OBJ_SET(dash,"dash_on_top",GINT_TO_POINTER(TRUE));
	}
	EXIT();
}


/*!
  \brief updates the values of all gauges on all dashboards in use
  \param data is unused
  \returns TRUE unless app is closing down
  */
G_MODULE_EXPORT gboolean update_dashboards(gpointer data)
{
	static GMutex *dash_mutex = NULL;
	ENTER();
	if (DATA_GET(global_data,"leaving"))
	{
		EXIT();
		return FALSE;
	}
	if (!dash_mutex)
		dash_mutex = (GMutex *)DATA_GET(global_data,"dash_mutex");

	g_mutex_lock(dash_mutex);
	if (DATA_GET(global_data,"dash_hash"))
		powerfc_process_serial(NULL);
		g_hash_table_foreach((GHashTable *)DATA_GET(global_data,"dash_hash"),update_dash_gauge,NULL);
	g_mutex_unlock(dash_mutex);
	EXIT();
	return FALSE;
}


/*!
  \brief leave() is the main shutdown function. It shuts down
  whatever runnign handlers are still going, deallocates memory and quits
  \param widget is  unused
  \param data is unused
  \returns TRUE
  */
gboolean leave(GtkWidget *widget, gpointer data)
{
	FILE *csvfile;

	ENTER();
	/* Set global flag */
	DATA_SET(global_data,"leaving",GINT_TO_POINTER(TRUE));

	csvfile = (FILE *)DATA_GET(global_data,"csvfile");
	if (csvfile != NULL) {
		fclose(csvfile);
	}
	gtk_widget_destroy(widget);
	/* Free all buffers */
	mem_dealloc();
	gtk_main_quit();
	exit(0);
	EXIT();
	return TRUE;
}
