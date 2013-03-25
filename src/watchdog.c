#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "watchdog.h"

typedef struct _HudWatchdogPrivate HudWatchdogPrivate;
struct _HudWatchdogPrivate {
	gulong timer;
};

#define HUD_WATCHDOG_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_WATCHDOG_TYPE, HudWatchdogPrivate))

static void hud_watchdog_class_init (HudWatchdogClass *klass);
static void hud_watchdog_init       (HudWatchdog *self);
static void hud_watchdog_dispose    (GObject *object);
static void hud_watchdog_finalize   (GObject *object);

G_DEFINE_TYPE (HudWatchdog, hud_watchdog, G_TYPE_OBJECT);

static void
hud_watchdog_class_init (HudWatchdogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudWatchdogPrivate));

	object_class->dispose = hud_watchdog_dispose;
	object_class->finalize = hud_watchdog_finalize;

	return;
}

static void
hud_watchdog_init (HudWatchdog *self)
{

	return;
}

static void
hud_watchdog_dispose (GObject *object)
{

	G_OBJECT_CLASS (hud_watchdog_parent_class)->dispose (object);
	return;
}

static void
hud_watchdog_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_watchdog_parent_class)->finalize (object);
	return;
}
