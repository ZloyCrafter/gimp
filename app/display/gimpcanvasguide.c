/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpcanvasguide.c
 * Copyright (C) 2010 Michael Natterer <mitch@gimp.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libgimpbase/gimpbase.h"
#include "libgimpmath/gimpmath.h"

#include "display-types.h"

#include "gimpcanvasguide.h"
#include "gimpdisplayshell.h"
#include "gimpdisplayshell-transform.h"


enum
{
  PROP_0,
  PROP_ORIENTATION,
  PROP_POSITION
};


typedef struct _GimpCanvasGuidePrivate GimpCanvasGuidePrivate;

struct _GimpCanvasGuidePrivate
{
  GimpOrientationType orientation;
  gint                position;
};

#define GET_PRIVATE(guide) \
        G_TYPE_INSTANCE_GET_PRIVATE (guide, \
                                     GIMP_TYPE_CANVAS_GUIDE, \
                                     GimpCanvasGuidePrivate)


/*  local function prototypes  */

static void        gimp_canvas_guide_set_property (GObject          *object,
                                                  guint             property_id,
                                                  const GValue     *value,
                                                  GParamSpec       *pspec);
static void        gimp_canvas_guide_get_property (GObject          *object,
                                                  guint             property_id,
                                                  GValue           *value,
                                                  GParamSpec       *pspec);
static void        gimp_canvas_guide_draw         (GimpCanvasItem   *item,
                                                  GimpDisplayShell *shell,
                                                  cairo_t          *cr);
static GdkRegion * gimp_canvas_guide_get_extents  (GimpCanvasItem   *item,
                                                  GimpDisplayShell *shell);


G_DEFINE_TYPE (GimpCanvasGuide, gimp_canvas_guide, GIMP_TYPE_CANVAS_ITEM)

#define parent_class gimp_canvas_guide_parent_class


static void
gimp_canvas_guide_class_init (GimpCanvasGuideClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  GimpCanvasItemClass *item_class   = GIMP_CANVAS_ITEM_CLASS (klass);

  object_class->set_property = gimp_canvas_guide_set_property;
  object_class->get_property = gimp_canvas_guide_get_property;

  item_class->draw           = gimp_canvas_guide_draw;
  item_class->get_extents    = gimp_canvas_guide_get_extents;

  g_object_class_install_property (object_class, PROP_ORIENTATION,
                                   g_param_spec_enum ("orientation", NULL, NULL,
                                                      GIMP_TYPE_ORIENTATION_TYPE,
                                                      GIMP_ORIENTATION_HORIZONTAL,
                                                      GIMP_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_POSITION,
                                   g_param_spec_int ("position", NULL, NULL,
                                                     -GIMP_MAX_IMAGE_SIZE,
                                                     GIMP_MAX_IMAGE_SIZE, 0,
                                                     GIMP_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (GimpCanvasGuidePrivate));
}

static void
gimp_canvas_guide_init (GimpCanvasGuide *guide)
{
}

static void
gimp_canvas_guide_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  GimpCanvasGuidePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_ORIENTATION:
      private->orientation = g_value_get_enum (value);
      break;
    case PROP_POSITION:
      private->position = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_canvas_guide_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  GimpCanvasGuidePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, private->orientation);
      break;
    case PROP_POSITION:
      g_value_set_int (value, private->position);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_canvas_guide_transform (GimpCanvasItem   *item,
                             GimpDisplayShell *shell,
                             gdouble          *x1,
                             gdouble          *y1,
                             gdouble          *x2,
                             gdouble          *y2)
{
  GimpCanvasGuidePrivate *private = GET_PRIVATE (item);
  GtkAllocation           allocation;
  gint                    x, y;

  gtk_widget_get_allocation (shell->canvas, &allocation);

  *x1 = 0;
  *y1 = 0;
  *x2 = allocation.width;
  *y2 = allocation.height;

  switch (private->orientation)
    {
    case GIMP_ORIENTATION_HORIZONTAL:
      gimp_display_shell_transform_xy (shell, 0, private->position, &x, &y);
      *y1 = *y2 = y + 0.5;
      break;

    case GIMP_ORIENTATION_VERTICAL:
      gimp_display_shell_transform_xy (shell, private->position, 0, &x, &y);
      *x1 = *x2 = x + 0.5;
      break;

    case GIMP_ORIENTATION_UNKNOWN:
      return;
    }
}

static void
gimp_canvas_guide_draw (GimpCanvasItem   *item,
                       GimpDisplayShell *shell,
                       cairo_t          *cr)
{
  gdouble x1, y1;
  gdouble x2, y2;

  gimp_canvas_guide_transform (item, shell, &x1, &y1, &x2, &y2);

  cairo_move_to (cr, x1, y1);
  cairo_line_to (cr, x2, y2);

  _gimp_canvas_item_stroke (item, shell, cr);
}

static GdkRegion *
gimp_canvas_guide_get_extents (GimpCanvasItem   *item,
                               GimpDisplayShell *shell)
{
  GdkRectangle rectangle;
  gdouble      x1, y1;
  gdouble      x2, y2;

  gimp_canvas_guide_transform (item, shell, &x1, &y1, &x2, &y2);

  rectangle.x      = MIN (x1, x2) - 1.5;
  rectangle.y      = MIN (y1, y2) - 1.5;
  rectangle.width  = ABS (x2 - x1) + 3.0;
  rectangle.height = ABS (y2 - y1) + 3.0;

  return gdk_region_rectangle (&rectangle);
}

GimpCanvasItem *
gimp_canvas_guide_new (GimpOrientationType  orientation,
                       gint                 position)
{
  return g_object_new (GIMP_TYPE_CANVAS_GUIDE,
                       "orientation", orientation,
                       "position",    position,
                       NULL);
}