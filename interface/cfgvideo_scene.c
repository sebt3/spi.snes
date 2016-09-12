#include "scenes.h"
#include "filters.h"
#include "db.h"
static menu_db		*userDB;
static menu_widget	*s_cpu, *s_gpu, *s_proj;
static menu_Game_Config *cfg;
static const char *cpu_filters[FILTER_MAX_ID+1];
static const char *gpu_filters[SHADER_MAX_ID+1];
static const char *displays[] = {
	"2x",
	"maximized",
	"fullscreen",
	NULL
};

static void menu_cfgVideo_clicked_ok(menu_widget* w) {
	menu_widgetSelect *proj = (menu_widgetSelect*)s_proj->properties;
	menu_widgetSelect *gpu  = (menu_widgetSelect*)s_gpu->properties;
	menu_widgetSelect *cpu  = (menu_widgetSelect*)s_cpu->properties;
	cfg->proj		= 2-proj->selected;
	cfg->cpuFilter		= cpu->selected;
	cfg->gpuFilter		= gpu->selected;
	EMU_setVideoCfg(EINA_FALSE, cfg->proj, cfg->cpuFilter, cfg->gpuFilter);
	w->wl->quit(w->wl->scene);
}

static void menu_cfgVideo_clicked(menu_widget* w) {
	w->wl->quit(w->wl->scene);
}


void		menu_Scenes_cfgVideo_init(menu_db *db) {
	int i;
	menu_Scene* uiSc= menu_widgetScene_new("cfgv", EINA_TRUE);
	userDB		= db;
	cfg		= menu_db_get_Game_Config(userDB, 0);

	cpu_filters[FILTER_MAX_ID] = NULL;
	for (i=0;i<FILTER_MAX_ID;i++) {
		cpu_filters[i] = menu_filter_get(i)->name;
	}
	gpu_filters[SHADER_MAX_ID] = NULL;
	for (i=0;i<SHADER_MAX_ID;i++) {
		gpu_filters[i] = menu_Shader_get_filter(i)->name;
	}

	s_cpu = menu_widgetSelect_new(uiSc->wl, "cpu",	cpu_filters,	cfg->cpuFilter, NULL);
	s_gpu = menu_widgetSelect_new(uiSc->wl, "gpu",	gpu_filters,	cfg->gpuFilter, NULL);
	s_proj= menu_widgetSelect_new(uiSc->wl, "disp",	displays,	2-cfg->proj, NULL);
	menu_widgetButton_new(uiSc->wl, "ok",	menu_cfgVideo_clicked_ok);
	menu_widgetButton_new(uiSc->wl, "cl",	menu_cfgVideo_clicked);
#ifdef PANDORA
	menu_widgetScene_addAccel(uiSc->wl, SDLK_HOME, menu_cfgVideo_clicked_ok, s_cpu);
	menu_widgetScene_addAccel(uiSc->wl, SDLK_PAGEDOWN, menu_cfgVideo_clicked, s_cpu);
#else
	menu_widgetScene_addAccel(uiSc->wl, SDLK_a, menu_cfgVideo_clicked_ok, s_cpu);
	menu_widgetScene_addAccel(uiSc->wl, SDLK_x, menu_cfgVideo_clicked, s_cpu);
#endif

}
