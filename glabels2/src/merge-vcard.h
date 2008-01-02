/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_vcard.h:  vcard merge backend module header file
 *
 *  Copyright (C) 2002  Jim Evins <evins@snaught.com>.
 *  and
 *  Copyright (C) 2005  Austin Henry <ahenry@users.sourceforge.net>
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
#ifndef __MERGE_VCARD_H__
#define __MERGE_VCARD_H__

#include "merge.h"

G_BEGIN_DECLS

/* The following object arguments are available:
 *
 * name               type             description
 * ---------------------------------------------------------------------------
 *
 */

#define GL_TYPE_MERGE_VCARD              (gl_merge_vcard_get_type ())
#define GL_MERGE_VCARD(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MERGE_VCARD, glMergeVCard))
#define GL_MERGE_VCARD_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE_VCARD, glMergeVCardClass))
#define GL_IS_MERGE_VCARD(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MERGE_VCARD))
#define GL_IS_MERGE_VCARD_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE_VCARD))
#define GL_MERGE_VCARD_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_MERGE_VCARD, glMergeVCardClass))


typedef struct _glMergeVCard          glMergeVCard;
typedef struct _glMergeVCardClass     glMergeVCardClass;

typedef struct _glMergeVCardPrivate   glMergeVCardPrivate;


struct _glMergeVCard {
	glMerge              object;

	glMergeVCardPrivate *priv;
};

struct _glMergeVCardClass {
	glMergeClass         parent_class;
};


GType             gl_merge_vcard_get_type            (void) G_GNUC_CONST;

G_END_DECLS


#endif /* __MERGE_VCARD_H__ */
