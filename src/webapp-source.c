/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Robert Carr
 */

#include "webapp-source.h"

#include <stdio.h>

#include <glib/gi18n.h>
#include <gio/gio.h>

#ifdef HAVE_BAMF
#include <libbamf/libbamf.h>
#include <libbamf/bamf-application.h>
#include <libbamf/bamf-view.h>
#include <libbamf/bamf-tab.h>
#endif

#include "settings.h"
#include "dbusmenu-collector.h"
#include "source.h"

/**
 * SECTION:hudwebappsource
 * @title:HudWebappSource
 * @short_description: a #HudSource to search through the menus of
 *   webapps registered with the Unity Webapps System.
 *
 * #HudWebappsSource searches through the menu items of webapps
 **/

/**
 * HudWebappSource:
 *
 * This is an opaque structure type.
 **/

struct _HudWebappSource
{
  GObject parent_instance;
  
  GList *applications;
  
#ifdef HAVE_BAMF
  BamfMatcher *matcher;
#endif
};

typedef GObjectClass HudWebappSourceClass;

typedef struct _HudWebappApplicationSource {
  AbstractApplication *application;

  HudSource *source;
  HudSource *collector;
} HudWebappApplicationSource;

static void hud_webapp_source_iface_init (HudSourceInterface *iface);
G_DEFINE_TYPE_WITH_CODE (HudWebappSource, hud_webapp_source, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, hud_webapp_source_iface_init))



#ifdef HAVE_BAMF
static void
hud_webapp_source_collector_changed (HudSource *source,
				     gpointer   user_data)
{
  hud_source_changed ((HudSource *)user_data);
}
#endif

#ifdef HAVE_BAMF
static void
hud_webapp_source_application_child_moved (AbstractApplication *application,
					     BamfView *child,
					     gpointer user_data)
{
  HudSource *source = HUD_SOURCE (user_data);
  
  hud_source_changed (source);
}
#endif

static gboolean
hud_webapp_source_should_search_app (AbstractApplication *application,
				     guint32 active_xid)
{
  gboolean should;
  
  should = FALSE;

#ifdef HAVE_BAMF
  GList *children, *walk;
  children = bamf_view_get_children (BAMF_VIEW (application));

  for (walk = children; walk != NULL; walk = walk->next)
    {
      BamfTab *child;
      
      if (BAMF_IS_TAB (walk->data) == FALSE)
	continue;
      
      child = BAMF_TAB (walk->data);
      
      if ((active_xid == bamf_tab_get_xid (child)) &&
	  bamf_tab_get_is_foreground_tab (child))
	{
	  should = TRUE;
	  break;
	}
    }
  
  g_list_free (children);
#endif
  return should;
}

static void
hud_webapp_source_search (HudSource    *hud_source,
                          HudTokenList *token_list,
                          void        (*append_func) (HudResult * result, gpointer user_data),
                          gpointer      user_data)
{
  HudWebappSource *source;
  GList *walk;
  
  source = HUD_WEBAPP_SOURCE (hud_source);
  
  for (walk = source->applications; walk != NULL; walk = walk->next)
    {
      HudWebappApplicationSource *application_source;
      guint32 active_xid = 0;
      
      application_source = (HudWebappApplicationSource *)walk->data;

#ifdef HAVE_BAMF
      active_xid = bamf_window_get_xid(bamf_matcher_get_active_window(source->matcher));
#endif

      if (hud_webapp_source_should_search_app (application_source->application, active_xid))
	{
	  hud_source_search (application_source->collector, token_list, append_func, user_data);
	}

    }
}

static void
hud_webapp_source_list_applications (HudSource    *hud_source,
                                     HudTokenList *token_list,
                                     void        (*append_func) (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data),
                                     gpointer      user_data)
{
  HudWebappSource *source;
  GList *walk;
  source = HUD_WEBAPP_SOURCE (hud_source);
  
  for (walk = source->applications; walk != NULL; walk = walk->next)
    {
      HudWebappApplicationSource *application_source;
      guint32 active_xid = 0;
      
      application_source = (HudWebappApplicationSource *)walk->data;

#ifdef HAVE_BAMF
      active_xid = bamf_window_get_xid(bamf_matcher_get_active_window(source->matcher));
#endif

      if (active_xid != 0 && hud_webapp_source_should_search_app (application_source->application, active_xid))
        {
          hud_source_list_applications (application_source->collector, token_list, append_func, user_data);
        }
    }
}

static HudSource *
hud_webapp_source_get (HudSource   *hud_source,
                       const gchar *application_id)
{
  HudWebappSource *source;
  GList *walk;
  source = HUD_WEBAPP_SOURCE (hud_source);

  for (walk = source->applications; walk != NULL; walk = walk->next)
    {
      HudWebappApplicationSource *application_source;
      guint32 active_xid = 0;

      application_source = (HudWebappApplicationSource *)walk->data;

#ifdef HAVE_BAMF
      active_xid = bamf_window_get_xid(bamf_matcher_get_active_window(source->matcher));
#endif

      if (active_xid != 0 && hud_webapp_source_should_search_app (application_source->application, active_xid))
        {
          HudSource *result = hud_source_get(application_source->collector, application_id);
          if (result != NULL)
            return result;
        }
    }

  return NULL;
}

HudWebappApplicationSource *
hud_webapp_application_source_new (HudSource *source,
				   AbstractApplication *application)
{
  gchar *name, *path;

  name = path = NULL;

#ifdef HAVE_BAMF
  /*
   * This function is deprecated in upstream libbamf.
   *
   * Let's just ignore the warnings and hope that the whole BAMF
   * get's deprecated by MIR before they actually remove this function
   * altogether. ;)
   */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  bamf_application_get_application_menu (application,
					 &name, &path);
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif
  
  if (name == NULL || *name == '\0')
    return NULL;

#ifdef HAVE_BAMF
  HudWebappApplicationSource *application_source;

  application_source = g_malloc0 (sizeof (HudWebappApplicationSource));
  
  application_source->application = g_object_ref (G_OBJECT (application));
  application_source->source = source;
  
  application_source->collector = (HudSource *)hud_dbusmenu_collector_new_for_endpoint (bamf_view_get_name (BAMF_VIEW (application)),
											bamf_view_get_name (BAMF_VIEW (application)),
											bamf_view_get_icon (BAMF_VIEW (application)),
											0,
											name, path,
											HUD_SOURCE_ITEM_TYPE_BACKGROUND_APP);
  
  g_signal_connect_object (application, "child-moved",
			   G_CALLBACK (hud_webapp_source_application_child_moved),
			   source, 0);
  
  
  
  g_free (name);
  g_free (path);
  
  return application_source;
#else
  return NULL;
#endif
}

#ifdef HAVE_BAMF
static void
hud_webapp_application_source_free (HudWebappApplicationSource *application_source)
{
  g_object_unref (G_OBJECT (application_source->application));
  g_object_unref (G_OBJECT (application_source->collector));
  
  g_free (application_source);
}
#endif

#ifdef HAVE_BAMF
static void
on_active_changed (BamfApplication *application,
		   gboolean active,
		   HudSource *source)
{
  hud_source_changed (source);
}
#endif

#ifdef HAVE_BAMF
static void
hud_webapp_source_remove_application (HudWebappSource *source,
				      AbstractApplication *application)
{
  HudWebappApplicationSource *application_source;
  GList *walk;
  
  application_source = NULL;
  
  for (walk = source->applications; walk != NULL; walk = walk->next)
    {
      HudWebappApplicationSource *t;
      
      t = (HudWebappApplicationSource *)walk->data;
      
      if (t->application == application)
	application_source = t;
    }
  
  if (application_source == NULL)
    return;
  
  g_signal_handlers_disconnect_by_func (application_source->collector,
					G_CALLBACK (hud_webapp_source_collector_changed),
					source);
  g_signal_handlers_disconnect_by_func (application, G_CALLBACK (on_active_changed),
					source);

  source->applications = g_list_remove (source->applications, application_source);
  
  hud_webapp_application_source_free (application_source);
  
  hud_webapp_source_collector_changed ((HudSource *)source, source);
  
}
#endif

#ifdef HAVE_BAMF
static void
hud_webapp_source_bamf_view_closed (BamfMatcher *matcher,
				    BamfView *view,
				    gpointer user_data)
{
  HudWebappSource *source;
  
  source = (HudWebappSource *)user_data;
  
  if (BAMF_IS_APPLICATION (view) == FALSE)
    return;
  
  hud_webapp_source_remove_application (source, (BamfApplication *)view);
}
#endif

#ifdef HAVE_BAMF 
static void
hud_webapp_source_register_application (HudWebappSource *source,
					AbstractApplication *application)
{
  HudWebappApplicationSource *application_source;
  
  application_source = hud_webapp_application_source_new (HUD_SOURCE (source), application);
  
  if (application_source == NULL)
    return;
  
  g_signal_connect (application_source->collector, "changed", G_CALLBACK (hud_webapp_source_collector_changed), source);
  
  source->applications = g_list_append (source->applications, application_source);
  
  hud_webapp_source_collector_changed ((HudSource *)source, source);

  g_signal_connect (application, "active-changed", G_CALLBACK (on_active_changed), source);
  
}
#endif

static void
hud_webapp_source_finalize (GObject *object)
{

  G_OBJECT_CLASS(hud_webapp_source_parent_class)->finalize(object);
  return;
}

#ifdef HAVE_BAMF
static void
hud_webapp_source_bamf_view_opened (BamfMatcher *matcher,
				    BamfView *view,
				    gpointer user_data)
{
  HudWebappSource *source;
  BamfApplication *application;
  
  source = (HudWebappSource *)user_data;
  
  if (BAMF_IS_APPLICATION (view) == FALSE)
    return;
  
  application = BAMF_APPLICATION (view);
  
  if (g_strcmp0 (bamf_application_get_application_type (application), "webapp") != 0)
    return;
  
  hud_webapp_source_register_application (source, application);
}
#endif

static void
hud_webapp_source_init (HudWebappSource *source)
{
#ifdef HAVE_BAMF
  BamfMatcher *matcher;
  GList *applications, *walk;
#endif
  
  source->applications = NULL;
  
#ifdef HAVE_BAMF
  matcher = bamf_matcher_get_default ();
  
  applications = bamf_matcher_get_applications (matcher);
  
  for (walk = applications; walk != NULL; walk = walk -> next)
    {
      BamfApplication *application;
      
      application = (BamfApplication *)walk->data;
      
      if (g_strcmp0 (bamf_application_get_application_type (application), "webapp") != 0)
	continue;
      
      hud_webapp_source_register_application (source, application);      
    }
  
  g_signal_connect (matcher, "view-opened", G_CALLBACK (hud_webapp_source_bamf_view_opened), source);
  g_signal_connect (matcher, "view-closed", G_CALLBACK (hud_webapp_source_bamf_view_closed), source);
  
  g_list_free (applications);
#endif
}

static void
hud_webapp_source_use (HudSource *hud_source)
{
}

static void
hud_webapp_source_unuse (HudSource *hud_source)
{
}

static void
hud_webapp_source_iface_init (HudSourceInterface *iface)
{
  iface->search = hud_webapp_source_search;
  iface->list_applications = hud_webapp_source_list_applications;
  iface->use = hud_webapp_source_use;
  iface->unuse = hud_webapp_source_unuse;
  iface->get = hud_webapp_source_get;
}

static void
hud_webapp_source_class_init (HudWebappSourceClass *class)
{
  class->finalize = hud_webapp_source_finalize;
}

/**
 * hud_webapp_source_new:
 *
 * Creates a #HudWebappSource.
 *
 * Returns: a new #HudWebappSource
 **/
HudWebappSource *
hud_webapp_source_new (void)
{
  HudWebappSource *webapp_source;
  
  webapp_source = (HudWebappSource *) g_object_new (HUD_TYPE_WEBAPP_SOURCE, NULL);
  
  return webapp_source;
}