/*
 * GnomeSword Bible Study Tool
 * dictlex_dialog.c - dialog for displaying a dictlex module
 *
 * Copyright (C) 2000,2001,2002 GnomeSword Developer Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <gtkhtml/gtkhtml.h>

#include "gui/dictlex_dialog.h"

#include "main/dictlex.h"
#include "main/gs_gnomesword.h"
#include "main/gs_html.h"
#include "main/settings.h"
#include "main/lists.h"

/******************************************************************************
 * globals
 */
gboolean isrunningSD = FALSE;	/* is the view dictionary dialog runing */
GtkWidget *frameShowDict;

/******************************************************************************
 * static - global to this file only
 */
static GtkWidget *clKeys;
static GtkWidget *textSDmodule;
static GtkWidget *cbSDMods;
static GtkWidget *ceSDMods;
static GtkWidget *hpaned2;
static GtkWidget *dlgViewDict;

/******************************************************************************
 * Name
 *  on_btnLoadKeys_clicked 
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   void on_btnLoadKeys_clicked(GtkButton * button, 
 *					gpointer user_data)
 *
 * Description
 *    
 *
 * Return value
 *   void
 */

static void on_btnLoadKeys_clicked(GtkButton * button, 
					gpointer user_data)
{
	gchar *key;
	
	gtk_paned_set_position(GTK_PANED(hpaned2), 120);
	gtk_clist_clear(GTK_CLIST(clKeys));
	gtk_clist_freeze(GTK_CLIST(clKeys));
	
	key = get_first_key_viewdict();
	gtk_clist_append(GTK_CLIST(clKeys), &key);
	g_free(key);
	
	while((key = get_next_key_viewdict()) != NULL) {
		gtk_clist_append(GTK_CLIST(clKeys), &key);
		g_free(key);
	}
	
	gtk_clist_thaw(GTK_CLIST(clKeys));
}

/******************************************************************************
 * Name
 *   on_btnClose_clicked
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   void on_btnClose_clicked(GtkButton * button, gpointer user_data)
 *
 * Description
 *    
 *
 * Return value
 *   void
 */

static void on_btnClose_clicked(GtkButton * button, gpointer user_data)
{
	gtk_widget_hide(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}

/******************************************************************************
 * Name
 *   on_btnVDSync_clicked
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   void on_btnVDSync_clicked(GtkButton * button, gpointer user_data)
 *
 * Description
 *    
 *
 * Return value
 *   void
 */

static void on_btnVDSync_clicked(GtkButton * button, gpointer user_data)
{
	search_text_changed_viewdict(settings.dictkey);
}

/******************************************************************************
 * Name
 *  on_dlgViewDict_destroy 
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   void on_dlgViewDict_destroy(GtkObject * object,
 *				   gpointer user_data)
 *
 * Description
 *    
 *
 * Return value
 *   void
 */

static void on_dlgViewDict_destroy(GtkObject * object,
				   gpointer user_data)
{
	isrunningSD = FALSE;
	shutdown_viewdict();
}

/******************************************************************************
 * Name
 *   on_ceSDMods_changed
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   void on_ceSDMods_changed(GtkEditable * editable, 
 *						gpointer user_data)
 *
 * Description
 *    
 *
 * Return value
 *   void
 */

static void on_ceSDMods_changed(GtkEditable * editable, 
						gpointer user_data)
{
	gchar *buf, *module_name, title[256];
	static gboolean firsttime = TRUE;
	
	module_name = gtk_entry_get_text(GTK_ENTRY(editable));
	if (!firsttime) {		
		gtk_paned_set_position(GTK_PANED(hpaned2), 0);
		load_module_viewdict(module_name);
		/* set frame label to current Module name  */
		gtk_frame_set_label(GTK_FRAME(frameShowDict),
				    module_name);
		buf =
		    gtk_entry_get_text(GTK_ENTRY
				       (GTK_WIDGET(user_data)));
		search_text_changed_viewdict(buf);
	}
	sprintf(title, "GnomeSword - %s", get_module_description
		(module_name));
	gtk_window_set_title(GTK_WINDOW(dlgViewDict), title);
	firsttime = FALSE;
}

/******************************************************************************
 * Name
 *   on_entrySDLookup_changed
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   void on_entrySDLookup_changed(GtkEditable * editable,
 *						gpointer user_data)
 *
 * Description
 *    
 *
 * Return value
 *   void
 */

static void on_entrySDLookup_changed(GtkEditable * editable,
						gpointer user_data)
{
	gchar *buf;
	buf = gtk_entry_get_text(GTK_ENTRY(editable));
	search_text_changed_viewdict(buf);
}


/******************************************************************************
 * Name
 *   on_entrySDLookup_key_release_event
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   gboolean on_entrySDLookup_key_release_event(GtkWidget * widget,
				GdkEventKey * event, gpointer user_data)
 *
 * Description
 *    
 *
 * Return value
 *   gboolean
 */

static gboolean on_entrySDLookup_key_release_event(GtkWidget * widget,
				GdkEventKey * event, gpointer user_data)
{

	return FALSE;
}

/******************************************************************************
 * Name
 *   on_clKeys_select_row
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   void on_clKeys_select_row(GtkCList *clist, gint row, gint column,
 *				GdkEvent * event, gpointer user_data)
 *
 * Description
 *    
 *
 * Return value
 *   void
 */

static void on_clKeys_select_row(GtkCList *clist, gint row, gint column,
				GdkEvent * event, gpointer user_data)
{
	gchar *buf;
	gtk_clist_get_text(clist, row, 0, &buf);
	goto_key_viewdict(buf);
}


/******************************************************************************
 * Name
 *   initSD
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   void initSD(gchar * modName)
 *
 * Description
 *    load first module
 *
 * Return value
 *   void
 */

void initSD(gchar * modName)
{
	load_module_viewdict(modName);
}

/******************************************************************************
 * Name
 *   gui_create_dictlex_dialog
 *
 * Synopsis
 *   #include "dictlex_dialog.h"
 *
 *   GtkWidget *gui_create_dictlex_dialog(GtkWidget * app)
 *
 * Description
 *    create the interface
 *
 * Return value
 *   GtkWidget *
 */

GtkWidget *gui_create_dictlex_dialog(GtkWidget * app)
{
	GtkWidget *dialog_vbox13;
	GtkWidget *vbox31;
	GtkWidget *toolbar27;
	GtkWidget *tmp_toolbar_icon;
	GtkWidget *btnVDSync;
	GtkWidget *label126;
	GtkWidget *entrySDLookup;
	GtkWidget *btnLoadKeys;
	GtkWidget *hbox26;
	GtkWidget *scrolledwindow37;
	GtkWidget *label125;
	GtkWidget *frame20;
	GtkWidget *scrolledwindow36;
	GtkWidget *dialog_action_area13;
	GtkWidget *btnClose;
	GList *dictList;
	
	dlgViewDict =
	    gnome_dialog_new(_("GnomeSwrod - View Dictionary"), NULL);
	gtk_object_set_data(GTK_OBJECT(dlgViewDict), "dlgViewDict",
			    dlgViewDict);
	gtk_window_set_default_size(GTK_WINDOW(dlgViewDict), 465, 275);
	gtk_window_set_policy(GTK_WINDOW(dlgViewDict), TRUE, TRUE,
			      FALSE);
	dialog_vbox13 = GNOME_DIALOG(dlgViewDict)->vbox;
	gtk_object_set_data(GTK_OBJECT(dlgViewDict), "dialog_vbox13",
			    dialog_vbox13);
	gtk_widget_show(dialog_vbox13);
	vbox31 = gtk_vbox_new(FALSE, 0);
	gtk_widget_ref(vbox31);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "vbox31",
				 vbox31,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(vbox31);
	gtk_box_pack_start(GTK_BOX(dialog_vbox13), vbox31, TRUE, TRUE,
			   0);
	toolbar27 =
	    gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL,
			    GTK_TOOLBAR_ICONS);
	gtk_widget_ref(toolbar27);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "toolbar27",
				 toolbar27,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(toolbar27);
	gtk_box_pack_start(GTK_BOX(vbox31), toolbar27, FALSE, FALSE, 0);
	gtk_toolbar_set_button_relief(GTK_TOOLBAR(toolbar27),
				      GTK_RELIEF_NONE);
	tmp_toolbar_icon =
	    gnome_stock_pixmap_widget(dlgViewDict,
				      GNOME_STOCK_PIXMAP_REFRESH);
	btnVDSync =
	    gtk_toolbar_append_element(GTK_TOOLBAR(toolbar27),
				       GTK_TOOLBAR_CHILD_BUTTON, NULL,
				       _("Sync"),
				       _("Lookup key from main window"),
				       NULL, tmp_toolbar_icon, NULL,
				       NULL);
	gtk_widget_ref(btnVDSync);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "btnVDSync",
				 btnVDSync,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(btnVDSync);
	cbSDMods = gtk_combo_new();
	gtk_widget_ref(cbSDMods);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "cbSDMods",
				 cbSDMods,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(cbSDMods);
	gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar27), cbSDMods,
				  NULL, NULL);
	gtk_widget_set_usize(cbSDMods, 201, -2);
	ceSDMods = GTK_COMBO(cbSDMods)->entry;
	gtk_widget_ref(ceSDMods);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "ceSDMods",
				 ceSDMods,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(ceSDMods);
	gtk_widget_set_usize(ceSDMods, 158, -2);
	label126 = gtk_label_new(_("Lookup: "));
	gtk_widget_ref(label126);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "label126",
				 label126,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(label126);
	gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar27), label126,
				  NULL, NULL);
	gtk_widget_set_usize(label126, 53, -2);
	entrySDLookup = gtk_entry_new();
	gtk_widget_ref(entrySDLookup);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict),
				 "entrySDLookup", entrySDLookup,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(entrySDLookup);
	gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar27), entrySDLookup,
				  NULL, NULL);
	gtk_widget_set_usize(entrySDLookup, 119, -2);
	tmp_toolbar_icon =
	    gnome_stock_pixmap_widget(dlgViewDict,
				      GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST);
	btnLoadKeys =
	    gtk_toolbar_append_element(GTK_TOOLBAR(toolbar27),
				       GTK_TOOLBAR_CHILD_BUTTON, NULL,
				       _("button6"),
				       _
				       ("Load all keys for this module"),
				       NULL, tmp_toolbar_icon, NULL,
				       NULL);
	gtk_widget_ref(btnLoadKeys);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "btnLoadKeys",
				 btnLoadKeys,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(btnLoadKeys);
	frameShowDict = gtk_frame_new("");
	gtk_widget_ref(frameShowDict);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict),
				 "frameShowDict", frameShowDict,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(frameShowDict);
	gtk_box_pack_start(GTK_BOX(vbox31), frameShowDict, TRUE, TRUE,
			   0);
	hbox26 = gtk_hbox_new(FALSE, 0);
	gtk_widget_ref(hbox26);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "hbox26",
				 hbox26,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(hbox26);
	gtk_container_add(GTK_CONTAINER(frameShowDict), hbox26);
	hpaned2 = gtk_hpaned_new();
	gtk_widget_ref(hpaned2);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "hpaned2",
				 hpaned2,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(hpaned2);
	gtk_box_pack_start(GTK_BOX(hbox26), hpaned2, TRUE, TRUE, 0);
	gtk_paned_set_handle_size(GTK_PANED(hpaned2), 6);
	gtk_paned_set_position(GTK_PANED(hpaned2), 0);
	scrolledwindow37 = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_ref(scrolledwindow37);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict),
				 "scrolledwindow37", scrolledwindow37,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(scrolledwindow37);
	gtk_paned_pack1(GTK_PANED(hpaned2), scrolledwindow37, FALSE,
			TRUE);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW
				       (scrolledwindow37),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	clKeys = gtk_clist_new(1);
	gtk_widget_ref(clKeys);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "clKeys",
				 clKeys,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(clKeys);
	gtk_container_add(GTK_CONTAINER(scrolledwindow37), clKeys);
	gtk_clist_set_column_width(GTK_CLIST(clKeys), 0, 80);
	gtk_clist_column_titles_hide(GTK_CLIST(clKeys));
	label125 = gtk_label_new(_("label125"));
	gtk_widget_ref(label125);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "label125",
				 label125,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(label125);
	gtk_clist_set_column_widget(GTK_CLIST(clKeys), 0, label125);
	frame20 = gtk_frame_new(NULL);
	gtk_widget_ref(frame20);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "frame20",
				 frame20,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(frame20);
	gtk_paned_pack2(GTK_PANED(hpaned2), frame20, TRUE, TRUE);
	gtk_frame_set_shadow_type(GTK_FRAME(frame20), GTK_SHADOW_IN);
	scrolledwindow36 = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_ref(scrolledwindow36);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict),
				 "scrolledwindow36", scrolledwindow36,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(scrolledwindow36);
	gtk_container_add(GTK_CONTAINER(frame20), scrolledwindow36);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW
				       (scrolledwindow36),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);
	textSDmodule = gtk_html_new();
	gtk_widget_ref(textSDmodule);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict),
				 "textSDmodule", textSDmodule,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(textSDmodule);
	gtk_container_add(GTK_CONTAINER(scrolledwindow36),
			  textSDmodule);
	dialog_action_area13 = GNOME_DIALOG(dlgViewDict)->action_area;
	gtk_object_set_data(GTK_OBJECT(dlgViewDict),
			    "dialog_action_area13",
			    dialog_action_area13);
	gtk_widget_show(dialog_action_area13);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(dialog_action_area13),
				  GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(dialog_action_area13),
				   8);
	gnome_dialog_append_button(GNOME_DIALOG(dlgViewDict),
				   GNOME_STOCK_BUTTON_CLOSE);
	btnClose =
	    g_list_last(GNOME_DIALOG(dlgViewDict)->buttons)->data;
	gtk_widget_ref(btnClose);
	gtk_object_set_data_full(GTK_OBJECT(dlgViewDict), "btnClose",
				 btnClose,
				 (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show(btnClose);
	GTK_WIDGET_SET_FLAGS(btnClose, GTK_CAN_DEFAULT);
	gtk_signal_connect(GTK_OBJECT(dlgViewDict), "destroy",
			   GTK_SIGNAL_FUNC(on_dlgViewDict_destroy),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(btnVDSync), "clicked",
			   GTK_SIGNAL_FUNC(on_btnVDSync_clicked), NULL);
	gtk_signal_connect(GTK_OBJECT(ceSDMods), "changed",
			   GTK_SIGNAL_FUNC(on_ceSDMods_changed),
			   (GtkWidget *) entrySDLookup);
	gtk_signal_connect(GTK_OBJECT(entrySDLookup), "changed",
			   GTK_SIGNAL_FUNC(on_entrySDLookup_changed),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(entrySDLookup),
			   "key_release_event",
			   GTK_SIGNAL_FUNC
			   (on_entrySDLookup_key_release_event), NULL);
	gtk_signal_connect(GTK_OBJECT(btnLoadKeys), "clicked",
			   GTK_SIGNAL_FUNC(on_btnLoadKeys_clicked),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(clKeys), "select_row",
			   GTK_SIGNAL_FUNC(on_clKeys_select_row), NULL);
	gtk_signal_connect(GTK_OBJECT(btnClose), "clicked",
			   GTK_SIGNAL_FUNC(on_btnClose_clicked), NULL);
	gtk_signal_connect(GTK_OBJECT(textSDmodule), "link_clicked",
			   GTK_SIGNAL_FUNC(on_link_clicked), NULL);
	gtk_signal_connect(GTK_OBJECT(textSDmodule), "on_url",
			   GTK_SIGNAL_FUNC(on_url), (gpointer) app);
	dictList = NULL;
	
	setup_viewdict(textSDmodule);
	
	dictList = get_list(DICT_LIST);
	gtk_combo_set_popdown_strings(GTK_COMBO(cbSDMods), dictList);	
	g_list_free(dictList);
	
	gtk_entry_set_text(GTK_ENTRY(ceSDMods),
			   settings.DictWindowModule);
	gtk_entry_set_text(GTK_ENTRY(entrySDLookup), settings.dictkey);	
	search_text_changed_viewdict(settings.dictkey);
	isrunningSD = TRUE;
	return dlgViewDict;
}
