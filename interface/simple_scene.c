#include "scenes.h"
#include "db.h"
static menu_db		*userDB;
static menu_Scene	*gameSc,   *uiSc;
static menu_widget	*tbox, *sct, *sel;
static menu_widgetSelectItem p_items[4096];

static void sct_clicked(menu_widget* w) {
	menu_widgetSelectList *p= w->properties;
	if (p->itemCount<=0) return;
	menu_File *f = (menu_File *)p->items[p->current].value;
	EMU_LoadRom((char*)f->path);
	if (EMU_getCRC()) {
		f->startCount++;
		menu_setUIScene(menu_findScene("EMU_UI"));
	}
}

static void sct_changed(menu_widget* w) {
	char txt[1024];
	menu_Texture*	ts;
	menu_widgetSelectList *p= w->properties;
	menu_widgetImage *i = sct->properties;
	if (p->itemCount<=0) return;
	menu_Texture*	td = i->rg->t;
	menu_File *f = (menu_File *)p->items[p->current].value;
	menu_Game *a = menu_db_find_Game_byCrc(userDB, f->gameId);
	if (a) {
		sprintf(txt, "%8x/screen0.png", a->gameId);
		ts = menu_db_find_Texture(userDB, txt);
		menu_widgetTextBox_setText(tbox, a->overView);
	} else {
		ts = NULL;
		menu_widgetTextBox_setText(tbox, f->path);
	}
	if (!ts)
		ts = menu_Theme_find_Texture(g, "no_screen.png");
	if (ts && ts->data) {
		memcpy(td->data, ts->data, td->w*td->orig_h*sizeof(uint16_t));
		td->isUpdated = EINA_TRUE;
	}
}

static uint16_t EMU_filter_sortType = 0;
static int	EMU_filter_sortFnct(const void *a, const void *b) {
	const menu_widgetSelectItem *da = (const menu_widgetSelectItem *) a;
	const menu_widgetSelectItem *db = (const menu_widgetSelectItem *) b;
	menu_File		*fa	= (menu_File*)da->value;
	menu_File		*fb	= (menu_File*)db->value;
	menu_Game		*ga	= menu_db_find_Game_byCrc(userDB, fa->gameId);
	menu_Game		*gb	= menu_db_find_Game_byCrc(userDB, fb->gameId);
	int			cmp	= strcmp(da->name, db->name);
	if (cmp==0)		cmp	= strcmp(fa->path, fb->path);
	switch (EMU_filter_sortType) {
	default:
	case 0: // known first
		if (ga!=NULL && gb == NULL) return -1;
		if (ga==NULL && gb != NULL) return 1;
		return cmp;
	case 1: // reverse
		if (ga!=NULL && gb == NULL) return 1;
		if (ga==NULL && gb != NULL) return -1;
		return -cmp;
	case 2: // Most used
		if(fa->startCount>fb->startCount) return -1;
		if(fa->startCount<fb->startCount) return 1;
		if (ga!=NULL && gb == NULL) return -1;
		if (ga==NULL && gb != NULL) return 1;
		return cmp;
	case 3: // Least used
		if(fa->startCount>fb->startCount) return 1;
		if(fa->startCount<fb->startCount) return -1;
		if (ga!=NULL && gb == NULL) return 1;
		if (ga==NULL && gb != NULL) return -1;
		return -cmp;
	case 4: // alpha
		return cmp;
	}
	return 0;
}


int		EMU_filter_roms(menu_widgetSelectItem *target, char *genre_filter, char *name_filter, uint16_t sortby) {
	int i=0, j, k;
	Eina_List*	l;
	menu_Game*	a;
	menu_File*	f;
	menu_Game_Genre*ge;
	char*		pnt;
	EINA_LIST_FOREACH(userDB->files, l, f) {
		a = menu_db_find_Game_byCrc(userDB, f->gameId);
		if (!a&&(genre_filter||(name_filter&&name_filter[0])))
			continue;
		else if (a) {
			if (genre_filter) {
				k=0;
				for (j=0;j<5 && a->genres[j];j++) {
					ge = menu_db_find_Game_Genre(userDB, a->genres[j]);
					if (ge && strstr(ge->name, genre_filter))
						k=1;
				}
				if (k==0)continue;
			}
			if (name_filter && name_filter[0]) {
				if (!strcasestr((char*)a->title, name_filter))
					continue;
			}
			target[i].name  = (char*)a->title;
		
		} else if (strstr((char*)f->name, "__")) {
			if ((pnt = strrchr(f->path, '/')))
				target[i].name  = pnt+1;
			else
				target[i].name  = (char*)f->path;
		} else
			target[i].name  = (char*)f->name;
		target[i].value = (void*)f;
		i++;if (i>4095) break;
	}
	EMU_filter_sortType	= sortby;
	qsort(target, i, sizeof(menu_widgetSelectItem), EMU_filter_sortFnct);
	return i;
}

extern char *menu_cfgSelect_genre_filter;
extern char *menu_cfgSelect_fltr_filter;
extern uint16_t menu_cfgSelect_sort_type;

void		menu_Scenes_simple_prepare(menu_Scene *s) {	
	EMU_pause();
	menu_UISceneEmptyStack();
	menu_widgetSelectList_setItems(sel, p_items, EMU_filter_roms(p_items, menu_cfgSelect_genre_filter, menu_cfgSelect_fltr_filter, menu_cfgSelect_sort_type));
	menu_widgetScene_setSelected(uiSc->wl, sel);
	sct_changed(sel);
}

static void menu_Scenes_simple_AccelCB(menu_widget* w) {
	menu_setUIScene(menu_findScene("cfgs"));
}

void		menu_Scenes_simple_init(menu_db *db) {
	userDB			= db;

	// BG setup
	gameSc			= menu_Scene_new("simpleBG");
	menu_RGroup* rg_bg	= menu_RGroupSprite_new(1, EINA_FALSE, EINA_TRUE);
	menu_Scene_add_rgroup(gameSc, rg_bg);
	menu_RGroup_Texture_set(rg_bg, menu_Theme_find_Texture(g, "bg.png"));
	menu_RGroup_Image_add(rg_bg, 0,0, 800, 480);


	// UI setup
	uiSc			= menu_widgetScene_new("simple", EINA_FALSE);
	uiSc->prepare		= menu_Scenes_simple_prepare;
	sel 			= menu_widgetSelectList_new(uiSc->wl, "sel", sct_clicked, sct_changed);
	sct			= menu_widgetImage_new(uiSc->wl, "sct", menu_Texture_newUpdatable("sct", 256, 224), NULL);
	tbox			= menu_widgetTextBox_new(uiSc->wl, "tbx");
	menu_widgetScene_addAccel(uiSc->wl, SDLK_m, menu_Scenes_simple_AccelCB, NULL);
}

