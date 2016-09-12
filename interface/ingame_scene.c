#include "scenes.h"
#include "filters.h"
#include "db.h"
static menu_db		*userDB;
static menu_widget	*s_slot;

static const char *slots[] = {
	"slot 0",
	"slot 1",
	"slot 2",
	"slot 3",
	"slot 4",
	"slot 5",
	"slot 6",
	"slot 7",
	"slot 8",
	"slot 9",
	NULL
};

static void menu_inGame_clicked_load(menu_widget* w) {
	menu_widgetSelect *state  = (menu_widgetSelect*)s_slot->properties;
	EMU_loadState(state->selected);
	w->wl->quit(w->wl->scene);
}

static void menu_inGame_clicked_save(menu_widget* w) {
	menu_widgetSelect *state  = (menu_widgetSelect*)s_slot->properties;
	EMU_saveState(state->selected);
	w->wl->quit(w->wl->scene);
}

static void menu_inGame_clicked_ok(menu_widget* w) {
	w->wl->quit(w->wl->scene);
}

static void menu_inGame_clicked_qt(menu_widget* w) {
	//menu_UISceneEmptyStack();
	w->wl->quit(w->wl->scene);
	w->wl->quit(w->wl->scene);
}

void		menu_Scenes_inGame_init(menu_db *db) {
	menu_Scene* uiSc= menu_widgetScene_new("menu", EINA_TRUE);
	userDB		= db;

	s_slot = menu_widgetSelect_new(uiSc->wl, "slot", slots,	0, NULL);

	menu_widgetArrow_new(uiSc->wl, "save",	menu_inGame_clicked_save, NULL);
	menu_widgetArrow_new(uiSc->wl, "load",	menu_inGame_clicked_load, NULL);
	menu_widgetArrow_new(uiSc->wl, "vid", 	NULL, "cfgv");
	menu_widgetButton_new(uiSc->wl, "ok",	menu_inGame_clicked_ok);
	menu_widgetButton_new(uiSc->wl, "qt",	menu_inGame_clicked_qt);
#ifdef PANDORA
	menu_widgetScene_addAccel(uiSc->wl, SDLK_HOME, menu_inGame_clicked_ok, s_slot);
	menu_widgetScene_addAccel(uiSc->wl, SDLK_PAGEDOWN, menu_inGame_clicked_qt, s_slot);
#else
	menu_widgetScene_addAccel(uiSc->wl, SDLK_a, menu_inGame_clicked_ok, s_slot);
	menu_widgetScene_addAccel(uiSc->wl, SDLK_x, menu_inGame_clicked_qt, s_slot);
#endif

}
