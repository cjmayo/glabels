/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  media_select.c:  media selection widget module
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <config.h>

#include "media_select.h"
#include "template.h"
#include "mini_preview.h"
#include "prefs.h"
#include "util.h"

#include "debug.h"

#define MINI_PREVIEW_WIDTH  160
#define MINI_PREVIEW_HEIGHT 200

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glMediaSelectSignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint media_select_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_media_select_class_init (glMediaSelectClass * class);
static void gl_media_select_init (glMediaSelect * media_select);
static void gl_media_select_destroy (GtkObject * object);

static void gl_media_select_construct (glMediaSelect * media_select);

static void page_size_entry_changed_cb (GtkEntry * entry, gpointer user_data);
static void template_entry_changed_cb (GtkEntry * entry, gpointer user_data);

static void details_update (glMediaSelect * media_select, gchar * name);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_media_select_get_type (void)
{
	static guint media_select_type = 0;

	if (!media_select_type) {
		GtkTypeInfo media_select_info = {
			"glMediaSelect",
			sizeof (glMediaSelect),
			sizeof (glMediaSelectClass),
			(GtkClassInitFunc) gl_media_select_class_init,
			(GtkObjectInitFunc) gl_media_select_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		media_select_type = gtk_type_unique (gtk_vbox_get_type (),
						     &media_select_info);
	}

	return media_select_type;
}

static void
gl_media_select_class_init (glMediaSelectClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_media_select_destroy;

	media_select_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glMediaSelectClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, media_select_signals,
				      LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_media_select_init (glMediaSelect * media_select)
{
	media_select->page_size_entry = NULL;
	media_select->template_entry = NULL;

	media_select->mini_preview = NULL;

	media_select->desc_label = NULL;
	media_select->sheet_size_label = NULL;
	media_select->number_label = NULL;
	media_select->label_size_label = NULL;
}

static void
gl_media_select_destroy (GtkObject * object)
{
	glMediaSelect *media_select;
	glMediaSelectClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MEDIA_SELECT (object));

	media_select = GL_MEDIA_SELECT (object);
	class = GL_MEDIA_SELECT_CLASS (GTK_OBJECT (media_select)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_media_select_new (void)
{
	glMediaSelect *media_select;

	media_select = gtk_type_new (gl_media_select_get_type ());

	gl_media_select_construct (media_select);

	return GTK_WIDGET (media_select);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_media_select_construct (glMediaSelect * media_select)
{
	GtkWidget *whbox, *wvbox, *wcombo, *wvbox1, *whbox1;
	gchar *name;
	GList *template_names, *page_sizes = NULL;
	const gchar *page_size;

	page_size = gl_prefs_get_page_size ();

	wvbox = GTK_WIDGET (media_select);

	whbox = gtk_hbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (wvbox), whbox, TRUE, TRUE, GNOME_PAD);

	/* Page size selection control */
	wcombo = gtk_combo_new ();
	page_sizes = g_list_append (page_sizes, "US-Letter");
	page_sizes = g_list_append (page_sizes, "A4");
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), page_sizes);
	g_list_free (page_sizes);
	media_select->page_size_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (media_select->page_size_entry),
				FALSE);
	gtk_widget_set_usize (media_select->page_size_entry, 100, 0);
	gtk_entry_set_text (GTK_ENTRY (media_select->page_size_entry),
			    page_size);
	gtk_box_pack_start (GTK_BOX (whbox), wcombo, FALSE, FALSE, GNOME_PAD);

	/* Actual selection control */
	template_names = gl_template_get_name_list (page_size);
	media_select->template_combo = gtk_combo_new ();
	gtk_combo_set_popdown_strings (GTK_COMBO (media_select->template_combo),
				       template_names);
	gl_template_free_name_list (&template_names);
	media_select->template_entry =
	    GTK_COMBO (media_select->template_combo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (media_select->template_entry),
				FALSE);
	gtk_widget_set_usize (media_select->template_entry, 400, 0);
	gtk_box_pack_start (GTK_BOX (whbox), media_select->template_combo,
			    FALSE, FALSE, GNOME_PAD);

	whbox = gtk_hbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (wvbox), whbox, TRUE, TRUE, GNOME_PAD);

	/* mini_preview canvas */
	media_select->mini_preview = gl_mini_preview_new ( MINI_PREVIEW_HEIGHT,
							   MINI_PREVIEW_WIDTH);
	gtk_box_pack_start (GTK_BOX (whbox), media_select->mini_preview,
			    FALSE, FALSE, GNOME_PAD);

	/* Label column */
	wvbox1 = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (whbox), wvbox1, FALSE, FALSE, 0);

	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1),
			    gtk_label_new (_("Description:")),
			    FALSE, FALSE, GNOME_PAD);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1),
			    gtk_label_new (_("Page size:")),
			    FALSE, FALSE, GNOME_PAD);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1),
			    gtk_label_new (_("Label size:")),
			    FALSE, FALSE, GNOME_PAD);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1),
			    gtk_label_new (_("Layout:")),
			    FALSE, FALSE, GNOME_PAD);

	/* detail widgets column */
	wvbox1 = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (whbox), wvbox1, FALSE, FALSE, 0);

	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	media_select->desc_label = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (whbox1), media_select->desc_label,
			    FALSE, FALSE, GNOME_PAD);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	media_select->sheet_size_label = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (whbox1), media_select->sheet_size_label,
			    FALSE, FALSE, GNOME_PAD);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	media_select->label_size_label = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (whbox1), media_select->label_size_label,
			    FALSE, FALSE, GNOME_PAD);
	whbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (wvbox1), whbox1, FALSE, FALSE, 0);
	media_select->number_label = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (whbox1), media_select->number_label,
			    FALSE, FALSE, GNOME_PAD);

	/* Update mini_preview and details from default template */
	name =
	    gtk_editable_get_chars (GTK_EDITABLE (media_select->template_entry),
				    0, -1);
	gl_mini_preview_set_label (GL_MINI_PREVIEW (media_select->mini_preview),
				   name);
	details_update (media_select, name);
	g_free (name);

	/* Connect signals to controls */
	gtk_signal_connect (GTK_OBJECT (media_select->page_size_entry),
			    "changed",
			    GTK_SIGNAL_FUNC (page_size_entry_changed_cb),
			    media_select);
	gtk_signal_connect (GTK_OBJECT (media_select->template_entry),
			    "changed",
			    GTK_SIGNAL_FUNC (template_entry_changed_cb),
			    media_select);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
page_size_entry_changed_cb (GtkEntry * entry,
			    gpointer user_data)
{
	glMediaSelect *media_select = GL_MEDIA_SELECT (user_data);
	gchar *page_size;
	GList *template_names;

	/* Update template selections for new page size */
	page_size = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	template_names = gl_template_get_name_list (page_size);
	gtk_combo_set_popdown_strings (GTK_COMBO (media_select->template_combo),
				       template_names);
	gl_template_free_name_list (&template_names);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
template_entry_changed_cb (GtkEntry * entry,
			   gpointer user_data)
{
	glMediaSelect *media_select = GL_MEDIA_SELECT (user_data);
	gchar *name;

	/* Update mini_preview canvas & details with template */
	name = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	gl_mini_preview_set_label (GL_MINI_PREVIEW (media_select->mini_preview),
				   name);
	details_update (media_select, name);
	g_free (name);

	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (user_data), media_select_signals[CHANGED]);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE. update "details" widgets from new template.               */
/*--------------------------------------------------------------------------*/
static void
details_update (glMediaSelect * media_select,
		gchar * name)
{
	glTemplate *template;
	gchar *text;
	glPrefsUnits units;
	const gchar *units_string;
	gdouble units_per_point;

	units = gl_prefs_get_units ();
	units_string = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();

	/* Fetch template */
	template = gl_template_from_name (name);

	gtk_label_set_text (GTK_LABEL (media_select->desc_label),
			    template->description);

	gtk_label_set_text (GTK_LABEL (media_select->sheet_size_label),
			    template->page_size);

	text = g_strdup_printf (_("%d x %d  (%d per sheet)"),
				template->nx, template->ny,
				template->nx * template->ny);

	gtk_label_set_text (GTK_LABEL (media_select->number_label), text);
	g_free (text);

	if ( units == GL_PREFS_UNITS_INCHES ) {
		gchar *xstr, *ystr;

		xstr = gl_util_fraction (template->label_height
					 * units_per_point);
		ystr = gl_util_fraction (template->label_width
					 * units_per_point);
		text = g_strdup_printf (_("%s x %s %s"),
					xstr, ystr, units_string);
		g_free (xstr);
		g_free (ystr);
	} else {
		text = g_strdup_printf (_("%.5g x %.5g %s"),
					template->label_height*units_per_point,
					template->label_width*units_per_point,
					units_string);
	}
	gtk_label_set_text (GTK_LABEL (media_select->label_size_label), text);
	g_free (text);

	gl_template_free( &template );
}

/****************************************************************************/
/* query selected label template name.                                      */
/****************************************************************************/
gchar *
gl_media_select_get_name (glMediaSelect * media_select)
{
	return
	    gtk_editable_get_chars (GTK_EDITABLE (media_select->template_entry),
				    0, -1);
}

/****************************************************************************/
/* set selected label template name.                                        */
/****************************************************************************/
void
gl_media_select_set_name (glMediaSelect * media_select,
			  gchar * name)
{
	gint pos;

	gtk_signal_handler_block_by_func (GTK_OBJECT
					  (media_select->template_entry),
					  GTK_SIGNAL_FUNC
					  (template_entry_changed_cb),
					  media_select);
	gtk_editable_delete_text (GTK_EDITABLE (media_select->template_entry),
				  0, -1);
	gtk_signal_handler_unblock_by_func (GTK_OBJECT
					    (media_select->template_entry),
					    GTK_SIGNAL_FUNC
					    (template_entry_changed_cb),
					    media_select);

	pos = 0;
	gtk_editable_insert_text (GTK_EDITABLE (media_select->template_entry),
				  name, strlen (name), &pos);
}

/****************************************************************************/
/* query selected label template page size.                                 */
/****************************************************************************/
gchar *
gl_media_select_get_page_size (glMediaSelect * media_select)
{
	return
	    gtk_editable_get_chars (GTK_EDITABLE
				    (media_select->page_size_entry), 0, -1);
}

/****************************************************************************/
/* set selected label template page size.                                   */
/****************************************************************************/
void
gl_media_select_set_page_size (glMediaSelect * media_select,
			       gchar * page_size)
{
	gint pos;

	gtk_signal_handler_block_by_func (GTK_OBJECT
					  (media_select->page_size_entry),
					  GTK_SIGNAL_FUNC
					  (page_size_entry_changed_cb),
					  media_select);
	gtk_editable_delete_text (GTK_EDITABLE (media_select->page_size_entry),
				  0, -1);
	gtk_signal_handler_unblock_by_func (GTK_OBJECT
					    (media_select->page_size_entry),
					    GTK_SIGNAL_FUNC
					    (page_size_entry_changed_cb),
					    media_select);

	pos = 0;
	gtk_editable_insert_text (GTK_EDITABLE (media_select->page_size_entry),
				  page_size, strlen (page_size), &pos);
}