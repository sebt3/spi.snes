#ifndef __menu_UI_H__
#define __menu_UI_H__
#include <SDL/SDL.h>
#include "rgroup.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Core system

typedef struct menu_Anime_	menu_Anime;
typedef struct menu_Scene_	menu_Scene;
typedef struct menu_widgetList_	menu_widgetList;
typedef struct menu_widget_	menu_widget;

typedef void		(*menu_Joy_CB)(void);
typedef void		(*menu_Anime_CB)(menu_Anime* a);
typedef void		(*menu_Event_CB)(menu_Scene *s, SDL_Event *e);
typedef void		(*menu_Scene_proc)(menu_Scene* s);
typedef void		(*menu_widget_CB)(menu_widget* w);
typedef menu_Segment*	(*menu_widget_getBG)(menu_widget* w);
typedef Eina_Bool	(*menu_widgetEvent_CB)(menu_widget* w, SDL_Event *e);

typedef struct {
	SDLKey		key;
	menu_widget_CB	callback;
	menu_widget	*w;
} menu_Accelerator;

struct menu_Anime_ {
	menu_Segment*	seg;
	int		tickLeft;
	menu_Anime_CB	onEnd;
	menu_Anime_CB	custom;
	menu_vector	move;
	menu_vector	rot;
	float		scale;
	void*		obj;
	int		objid;
};

struct  menu_Scene_ {
	const char*	name;
	Eina_List*	rgroups;
	Eina_List*	animes;
	menu_Event_CB	onEvent;
	menu_Joy_CB	joystickR;
	Eina_Bool	isEmu;
	menu_widgetList* wl;
	menu_Scene_proc	prepare;
	menu_Scene_proc	dispose;
};

struct menu_widgetList_ {
	const char	*name;
	Eina_List	*widgets;
	Eina_List	*accels;
	Eina_List	*selected;
	menu_Scene	*scene;
	menu_RGroup	*sprites, *bg, *text1, *text2, *text3;
	menu_widget	*cursor;
	menu_Scene_proc	quit;
	menu_widgetEvent_CB eventManager;
};

struct menu_widget_ {
	const char	*name;
	menu_widgetList	*wl;
	menu_vector	min;
	menu_vector	max;
	void		*properties;
	menu_widgetEvent_CB eventManager;
	menu_widget_CB	redraw;
	menu_widget_CB	freeProperties;
	menu_widget_getBG getBackground;
};

// Widgets definitions

typedef struct {
	menu_Segment	*sprite;
	menu_Mesh	*mesh;
} menu_widgetMesh;

typedef struct {
	menu_Text	*text;
	menu_RGroup	*rg;
} menu_widgetText;

typedef struct {
	menu_Segment	*sprite;
	menu_Segment	*s_ico;
	menu_Mesh	*m_ico;
	menu_Mesh	*mesh;
	menu_Text	*text;
	menu_widget_CB	onClick;
} menu_widgetButton;

typedef struct {
	const char	*target;
	menu_Segment	*s_bg;
	menu_Segment	*s_arrow;
	menu_Mesh	*m_bg;
	menu_Mesh	*m_arrow;
	menu_Text	*text;
	menu_widget_CB	onClick;
} menu_widgetArrow;

typedef struct {
	char		**values;
	uint16_t	maxVal;
	uint16_t	selected;
	menu_Segment	*s_bg;
	menu_Segment	*s_back;
	menu_Segment	*s_next;
	menu_Mesh	*m_bg;
	menu_Mesh	*m_back;
	menu_Mesh	*m_next;
	menu_Text	*option;
	menu_Text	*desc;
	menu_widget_CB	onChange;
} menu_widgetSelect;

typedef struct {
	uint16_t	value;
	menu_Segment	*s_bg;
	menu_Segment	*s_top;
	menu_Mesh	*m_bg;
	menu_Mesh	*m_top;
	menu_Text	*t_pct;
} menu_widgetProgress;

typedef struct {
	menu_Segment	*s_bg;
	menu_Segment	*s_left;
	menu_Segment	*s_right;
	menu_Mesh	*m_bg;
	menu_Mesh	*m_left;
	menu_Mesh	*m_right;
} menu_widgetCursor;

typedef struct {
	menu_Segment	*s_bg;
	menu_Segment	*s_img;
	menu_Mesh	*m_bg;
	menu_RGroup	*rg;
	menu_widget_CB	onClick;
} menu_widgetImage;

typedef struct {
	menu_Segment	*s_bg;
	menu_Mesh	*m_bg;
	char		*text;
} menu_widgetTextBox;

typedef struct {
	char 		*name;
	void		*value;
} menu_widgetSelectItem;

typedef struct {
	menu_Segment	*s_bg;
	menu_Segment	*s_cur;
	menu_Segment	*s_sol;
	menu_Mesh	*m_bg;
	menu_Mesh	*m_sol;
	menu_Mesh	*m_alt;
	menu_Mesh	*m_opt;
	menu_Text	*t_txt;
	menu_RGroup	*rg_text;
	menu_RGroup	*rg_sprite;
	menu_widget_CB	onClick;
	menu_widget_CB	onChange;
	menu_widgetSelectItem *items;
	uint16_t	itemCount;
	uint16_t	current;
	uint16_t	currentStart;
	float		h;
	int		mov;
} menu_widgetSelectList;

typedef struct {
	menu_Segment	*s_bg;
	menu_Segment	*s_cell;
	menu_Mesh	*m_bg;
	menu_Mesh	*m_cell;
	menu_Text	*t_desc;
	menu_Text	*t_opt;
	char		text[20];
} menu_widgetTextInput;


// Scenes
menu_Scene*	menu_Scene_new(const char *p_name);
void		menu_Scene_free(menu_Scene* s);
void		menu_Scene_empty(menu_Scene* s);
void		menu_Scene_draw(menu_Scene* s);
void		menu_Scene_runTick(menu_Scene* s);
void		menu_Scene_add_rgroup(menu_Scene* s, menu_RGroup *m);
menu_Anime*	menu_Scene_add_anime(menu_Scene* s, menu_Segment* seg);

// Widgets
menu_Scene*	menu_widgetScene_new(char *p_name, Eina_Bool useCur);
void		menu_widgetScene_redraw(menu_widgetList *p_wl);
void		menu_widgetScene_addAccel(menu_widgetList *p_wl, SDLKey p_key, menu_widget_CB p_callback, menu_widget *p_w);
void		menu_widgetScene_setSelected(menu_widgetList *p_wl, const menu_widget *p_w);
void		menu_widget_free(menu_widget *w);

menu_widget*	menu_widgetCursor_new(	menu_widgetList *p_wl);
menu_widget*	menu_widgetMesh_new(	menu_widgetList *p_wl, char *p_name);
menu_widget*	menu_widgetText_new(	menu_widgetList *p_wl, char *p_name, int16_t type);
menu_widget*	menu_widgetArrow_new(	menu_widgetList *p_wl, char *p_name, menu_widget_CB onClick, const char *target);
menu_widget*	menu_widgetButton_new(	menu_widgetList *p_wl, char *p_name, menu_widget_CB onClick);
menu_widget*	menu_widgetImage_new(	menu_widgetList *p_wl, char *p_name, menu_Texture *p_t, menu_widget_CB onClick);
menu_widget*	menu_widgetTextBox_new(	menu_widgetList *p_wl, char *p_name);
menu_widget*	menu_widgetSelect_new(	menu_widgetList *p_wl, char *p_name, const char **values, const int sel, menu_widget_CB onChange);
menu_widget*	menu_widgetProgress_new(menu_widgetList *p_wl, char *p_name, uint16_t value);
menu_widget*	menu_widgetSelectList_new(menu_widgetList *p_wl, char *p_name, menu_widget_CB onClick, menu_widget_CB onChange);
menu_widget*	menu_widgetTextInput_new(menu_widgetList *p_wl, char *p_name);

void		menu_widgetSelect_setValues(menu_widget *w, const char **values);
void		menu_widgetCursor_setCurrent(menu_widget *w, menu_widget *current);
void		menu_widgetProgress_setPct(menu_widget *w, uint16_t pct);
void		menu_widgetTextBox_setText(menu_widget *w, const char *p_text);
void		menu_widgetSelectList_setItems(menu_widget *w, menu_widgetSelectItem* p_items, uint16_t p_count);
#if defined(__cplusplus)
}
#endif
#endif
