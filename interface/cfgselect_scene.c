#include "scenes.h"
#include "filters.h"
#include "db.h"
static menu_db		*userDB;
static menu_widget	*s_genre, *s_name, *s_srt;
static const char	**genre_filters;
static uint16_t		genre_filters_count;
static const char *sort_type[] = {
	"Known first",
	"Reverse",
	"Most used",
	"Least used",
	"Alphanumeric",
	NULL
};

char *menu_cfgSelect_genre_filter = NULL;
char *menu_cfgSelect_fltr_filter;
uint16_t menu_cfgSelect_sort_type;

static void	menu_cfgSelect_clicked_ok(menu_widget* w) {
	menu_widgetSelect *gnr   = (menu_widgetSelect*)s_genre->properties;
	menu_widgetSelect *srt   = (menu_widgetSelect*)s_srt->properties;
	menu_cfgSelect_sort_type = srt->selected;
	if (gnr->selected==0)
		menu_cfgSelect_genre_filter = NULL;
	else
		menu_cfgSelect_genre_filter = (char*)genre_filters[gnr->selected];
	w->wl->quit(w->wl->scene);
}

static void	menu_cfgSelect_clicked(menu_widget* w) {
	w->wl->quit(w->wl->scene);
}
static void	menu_Scenes_cfgSelect_prepare(menu_Scene *s) {
	const char		**mem;
	int			i = 1;
	Eina_List*		l;
	menu_Game_Genre*	g;
	if (genre_filters_count < eina_list_count(userDB->genres) + 2) {
		genre_filters_count = eina_list_count(userDB->genres) + 2;
		mem = realloc(genre_filters, sizeof(char*)*genre_filters_count);
		if (!mem) {
			printf("ERRROR: Realloc for genre_filters failed.\n");
			return;
		}
		genre_filters = mem;

		genre_filters[0]	= "All";
		EINA_LIST_FOREACH(userDB->genres, l, g) {
			genre_filters[i] = g->name;
			i++;
		}
		genre_filters[i] = NULL;
		menu_widgetSelect_setValues(s_genre, genre_filters);
	}
}

void		menu_Scenes_cfgSelect_init(menu_db *db) {
	menu_Scene* uiSc	= menu_widgetScene_new("cfgs", EINA_TRUE);
	menu_widgetTextInput *p;
	userDB			= db;
	uiSc->prepare		= menu_Scenes_cfgSelect_prepare;
	genre_filters_count	= eina_list_count(db->genres) + 2;
	genre_filters		= calloc(genre_filters_count, sizeof(char*));
	Eina_List*		l;
	menu_Game_Genre*	g;
	int i			= 1;

	genre_filters[0]	= "All";
	EINA_LIST_FOREACH(db->genres, l, g) {
		genre_filters[i] = g->name;
		i++;
	}

	s_name  = menu_widgetTextInput_new(uiSc->wl, "fltr");
	s_genre = menu_widgetSelect_new(uiSc->wl, "genre", genre_filters, 0, NULL);
	s_srt   = menu_widgetSelect_new(uiSc->wl, "sort", sort_type, 0, NULL);
	p = s_name->properties;
	menu_cfgSelect_fltr_filter = p->text;
	menu_widgetButton_new(uiSc->wl, "ok",	menu_cfgSelect_clicked_ok);
	menu_widgetButton_new(uiSc->wl, "cl",	menu_cfgSelect_clicked);
	menu_cfgSelect_sort_type = 0;
#ifdef PANDORA
	menu_widgetScene_addAccel(uiSc->wl, SDLK_HOME, menu_cfgSelect_clicked_ok, s_genre);
	menu_widgetScene_addAccel(uiSc->wl, SDLK_PAGEDOWN, menu_cfgSelect_clicked, s_genre);
#else
	menu_widgetScene_addAccel(uiSc->wl, SDLK_a, menu_cfgSelect_clicked_ok, s_genre);
	menu_widgetScene_addAccel(uiSc->wl, SDLK_x, menu_cfgSelect_clicked, s_genre);
#endif
}
