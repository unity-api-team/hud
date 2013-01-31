#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "param.h"

struct _HudClientParamPrivate {
	int dummy;
};

#define HUD_CLIENT_PARAM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_CLIENT_PARAM_TYPE, HudClientParamPrivate))

static void hud_client_param_class_init (HudClientParamClass *klass);
static void hud_client_param_init       (HudClientParam *self);
static void hud_client_param_dispose    (GObject *object);
static void hud_client_param_finalize   (GObject *object);

G_DEFINE_TYPE (HudClientParam, hud_client_param, G_TYPE_OBJECT);

static void
hud_client_param_class_init (HudClientParamClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudClientParamPrivate));

	object_class->dispose = hud_client_param_dispose;
	object_class->finalize = hud_client_param_finalize;

	return;
}

static void
hud_client_param_init (HudClientParam *self)
{
	return;
}

static void
hud_client_param_dispose (GObject *object)
{

	G_OBJECT_CLASS (hud_client_param_parent_class)->dispose (object);
	return;
}

static void
hud_client_param_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_client_param_parent_class)->finalize (object);
	return;
}
