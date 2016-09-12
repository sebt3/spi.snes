#include "theme.h"
#include "rgroup.h"
#include "ui.h"
#include "engine.h"

/***********************************************************************************
 *				WidgetScene
 ***********************************************************************************/
static Eina_Bool menu_widgetScene_AccelManager(menu_Scene *s, SDL_Event *e) {
	Eina_List*		l;
	menu_Accelerator*	a;
	
	EINA_LIST_FOREACH(s->wl->accels, l, a) {
		if (a->key == e->key.keysym.sym && a->callback) {
			a->callback(a->w);
			return EINA_TRUE;
		}
	}

	return EINA_FALSE;
}

void		menu_widgetScene_EventManager(menu_Scene *s, SDL_Event *e) {
	Eina_List*	l;
	menu_widget*	w;
	Eina_Bool	found = EINA_FALSE;
	if (s->wl->eventManager && s->wl->eventManager(NULL, e))
		return;

	switch(e->type) {
	case SDL_QUIT:
		menu_quit = EINA_TRUE;
	break;
	case SDL_KEYDOWN:
		if (s->wl && s->wl->selected) {
			w = eina_list_data_get(s->wl->selected);
			if (w->eventManager)
				w->eventManager(w, e);
		}
	break;
	case SDL_KEYUP:
		if (s->wl && s->wl->selected) {
			w = eina_list_data_get(s->wl->selected);

			if (w && w->eventManager && w->eventManager(w, e))
				break;
			else if (menu_widgetScene_AccelManager(s, e))
				break;
			else if (s->wl->cursor) {
				// Cursor management
				switch(e->key.keysym.sym) {
				case SDLK_DOWN:
				case SDLK_RIGHT:
					EINA_LIST_FOREACH(s->wl->widgets, l, w) {
						if (w == eina_list_data_get(s->wl->selected)) { found = EINA_TRUE; continue; }
						else if (!found) continue;
						if (w != NULL && w->eventManager!=NULL && w->getBackground != NULL) {
							s->wl->selected = l;
							menu_widgetCursor_setCurrent(s->wl->cursor, w);
							break;
						}
					}
				break;
				case SDLK_UP:
				case SDLK_LEFT:
					if (s->wl->cursor == NULL) break;
					EINA_LIST_REVERSE_FOREACH(s->wl->widgets, l, w) {
						if (w == eina_list_data_get(s->wl->selected)) { found = EINA_TRUE; continue; }
						else if (!found) continue;
						if (w != NULL && w->eventManager!=NULL && w->getBackground != NULL) {
							s->wl->selected = l;
							menu_widgetCursor_setCurrent(s->wl->cursor, w);
							break;
						}
					}
				break;
				default:
				break;
				}
			}
		}
		switch(e->key.keysym.sym) {
		case SDLK_q:
		case SDLK_ESCAPE:
			if (s->wl && s->wl->quit)
				s->wl->quit(s);
			else
				menu_quit = EINA_TRUE;
			return;
		default:
		break;
		}
	break;
	//case SDL_MOUSEMOTION: // no mouseOver at least for now
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		if (s->wl == NULL) break;
		EINA_LIST_FOREACH(s->wl->widgets, l, w) {
			if (w->eventManager != NULL && w->eventManager(w, e)) {
				s->wl->selected = l;
				break;
			}
		}
		menu_quit = EINA_FALSE;
	break;
	default:
	break;
	}
}
void	menu_widgetScene_setSelected(menu_widgetList *p_wl, const menu_widget *p_w) {
	Eina_List*	l;
	menu_widget*	w;
	EINA_LIST_FOREACH(p_wl->widgets, l, w) {
		if (w == p_w) {
			p_wl->selected = l;
			break;
		}
	}
}

static void		menu_widgetScene_default_quit(menu_Scene* s) {
	menu_setUIScenePrevious();
}

menu_Scene*		menu_widgetScene_new(char *p_name, Eina_Bool useCur) {
	int i;
	menu_color colr		= {{0.2f, 0.2f, 0.2f, 1.0f}};
	menu_color colbg	= {{1.0f, 1.0f, 1.0f, 0.8f}};
	//menu_color colr	= {{1.0f, 1.0f, 1.0f, 1.0f}};
	menu_Scene* 	ret	= menu_Scene_new(p_name);
	menu_widgetList *wl	= calloc(1, sizeof(menu_widgetList));
	ret->wl			= wl;
	wl->scene		= ret;
	wl->widgets		= NULL;
	wl->selected		= NULL;
	wl->quit		= menu_widgetScene_default_quit;
	wl->name		= eina_stringshare_add(p_name);
	wl->bg			= menu_RGroupSprite_new(0, EINA_FALSE, EINA_TRUE);
	wl->sprites		= menu_RGroupSprite_new(0, EINA_FALSE, EINA_TRUE);
	wl->text1		= menu_RGroupSprite_new(0, EINA_FALSE, EINA_TRUE);
	wl->text2		= menu_RGroupSprite_new(0, EINA_FALSE, EINA_TRUE);
	wl->text3		= menu_RGroupSprite_new(0, EINA_FALSE, EINA_TRUE);
	ret->onEvent		= menu_widgetScene_EventManager;
	menu_Scene_add_rgroup(ret, wl->bg);
	menu_Scene_add_rgroup(ret, wl->sprites);
	menu_RGroup_set_font(wl->text1, menu_Theme_find_Font(g, "text1"));menu_RGroup_uniq_col_set(wl->text1, colr);menu_Scene_add_rgroup(ret, wl->text1);
	menu_RGroup_set_font(wl->text2, menu_Theme_find_Font(g, "text2"));menu_RGroup_uniq_col_set(wl->text2, colr);menu_Scene_add_rgroup(ret, wl->text2);
	menu_RGroup_set_font(wl->text3, menu_Theme_find_Font(g, "text3"));menu_RGroup_uniq_col_set(wl->text3, colr);menu_Scene_add_rgroup(ret, wl->text3);
	menu_widgetMesh_new(wl, "bg");
	menu_RGroup_uniq_col_set(wl->bg, colbg);
	for(i=1;i<5;i++) {
		char 	txt[6];
		sprintf(txt, "text%d", i);
		menu_widgetText_new(wl, txt, 1);
	}
	if (useCur) {
		menu_widgetCursor_new(wl);
	} else
		wl->cursor = NULL;

	return ret;
}

void		menu_widgetScene_redraw(menu_widgetList *p_wl) {
	Eina_List*	l;
	menu_widget*	w;
	menu_RGroup_empty(p_wl->bg);
	menu_RGroup_empty(p_wl->sprites);
	menu_RGroup_empty(p_wl->text1);
	menu_RGroup_empty(p_wl->text2);
	menu_RGroup_empty(p_wl->text3);
	EINA_LIST_FOREACH(p_wl->widgets, l, w) {
		if (w->redraw)
			w->redraw(w);
	}
	menu_widgetCursor_setCurrent(p_wl->cursor, eina_list_data_get(p_wl->selected));
}

void		menu_widgetScene_addAccel(menu_widgetList *p_wl, SDLKey p_key, menu_widget_CB p_callback, menu_widget *p_w) {
	menu_Accelerator *accel	= calloc(1, sizeof(menu_Accelerator));
	accel->key		= p_key;
	accel->callback		= p_callback;
	accel->w		= p_w;
	p_wl->accels		= eina_list_append(p_wl->accels, accel);
}

void		menu_widgetScene_add(menu_widgetList *p_wl, const menu_widget *toAdd) {
	Eina_List*	l;
	menu_widget*	w;
	if(toAdd!=NULL) {
		p_wl->widgets = eina_list_append(p_wl->widgets, toAdd);
		if (p_wl->selected == NULL) {
			EINA_LIST_FOREACH(p_wl->widgets, l, w) {
				if (w->getBackground) {
					p_wl->selected = l;
					menu_widgetCursor_setCurrent(p_wl->cursor, w);
					break;
				}
			}
		}
	}
}

/***********************************************************************************
 *				Widgets
 ***********************************************************************************/
menu_widget*	menu_widget_new(menu_widgetList *p_wl, char *p_name) {
	menu_widget *ret	= calloc(1, sizeof(menu_widget));
	ret->name		= eina_stringshare_add(p_name);
	ret->wl			= p_wl;
	ret->properties		= NULL;
	ret->eventManager	= NULL;
	ret->redraw		= NULL;
	ret->freeProperties	= NULL;
	ret->getBackground	= NULL;
	menu_widgetScene_add(p_wl, ret);
	return ret;
}

static char 	frm_txt[1024];
char*		menu_widget_get_frm(menu_widget* p_w, char *p_opt) {
	if (p_opt==NULL)
		sprintf(frm_txt, "%s_%s.frm", p_w->wl->name, p_w->name);
	else
		sprintf(frm_txt, "%s_%s_%s.frm", p_w->wl->name, p_w->name, p_opt);
	return frm_txt;
}

void		menu_widget_free(menu_widget *w) {
	menu_String_Free(w->name);
	if (w->properties != NULL) {
		if (w->freeProperties != NULL)
			w->freeProperties(w->properties);
		else	free(w->properties);
	}
	free(w);
}

static void	menu_widgetMesh_redraw(menu_widget *w) {
	menu_widgetMesh *p	= w->properties;
	p->sprite		= menu_RGroup_Mesh_add(w->wl->bg, p->mesh);
}

menu_widget*	menu_widgetMesh_new(menu_widgetList *p_wl, char *p_name) {
	char 	txt[1024];
	sprintf(txt, "%s_%s.frm", p_wl->name, p_name);
	menu_Mesh	*m = menu_Theme_find_Mesh(g, txt);
	if (m==NULL)	return NULL;
	else {
		menu_widget *ret	= menu_widget_new(p_wl, p_name);
		menu_widgetMesh *p	= calloc(1, sizeof(menu_widgetMesh));
		p->mesh			= m;
		ret->properties		= p;
		ret->redraw		= menu_widgetMesh_redraw;
		ret->redraw(ret);

		return ret;
	}
}

static void	menu_widgetText_redraw(menu_widget *w) {
	menu_widgetText *p	= w->properties;
	menu_RGroup_Text_add(p->rg, p->text);
}

menu_widget*	menu_widgetText_new(menu_widgetList *p_wl, char *p_name, int16_t type) {
	char 	txt[1024];
	sprintf(txt, "%s_%s.frm", p_wl->name, p_name);
	menu_Text	*m = menu_Theme_find_Text(g, txt);
	if (m==NULL)	return NULL;
	else {
		menu_widget *ret	= menu_widget_new(p_wl, p_name);
		menu_widgetText *p	= calloc(1, sizeof(menu_widgetText));
		p->text			= m;
		switch(type) {
			case 3:	p->rg	= p_wl->text3;break;
			case 2:	p->rg	= p_wl->text2;break;
			default:p->rg	= p_wl->text1;break;
		}
		ret->properties		= p;
		ret->redraw		= menu_widgetText_redraw;
		ret->redraw(ret);

		return ret;
	}
}

/***********************************************************************************
 *				WidgetCursor
 ***********************************************************************************/
void		menu_widgetCursor_setCurrent(menu_widget *w, menu_widget *current) {
	if (w == NULL ||current == NULL) return;
	menu_vector	pos;
	menu_Segment	*w_seg	= current->getBackground(current);
	menu_widgetCursor *p	= w->properties;
	menu_vector_set(pos, 0, 
			w_seg->min[0] - p->s_left->max[0] - 5.0,
			w_seg->min[1] - p->s_left->min[1] - ((p->s_left->max[1]-p->s_left->min[1])-(w_seg->max[1]-w_seg->min[1]))/2,
			0.0f);
	menu_Segment_move_by(p->s_left, pos);
	menu_vector_set(pos, 0, 
			w_seg->max[0] - p->s_right->min[0] + 5.0,
			w_seg->min[1] - p->s_right->min[1] - ((p->s_left->max[1]-p->s_left->min[1])-(w_seg->max[1]-w_seg->min[1]))/2,
			0.0f);
	menu_Segment_move_by(p->s_right, pos);
	menu_vector_set(pos, 0, 
			w_seg->min[0] - p->s_bg->min[0],
			w_seg->min[1] - p->s_bg->min[1], 0.0f);	
	menu_Segment_move_by(p->s_bg, pos);

	// TODO: fix me this wont always be only a quad
	p->s_bg->dots[6] = p->s_bg->dots[3]	= w_seg->max[0];
	p->s_bg->dots[7] = p->s_bg->dots[10]	= w_seg->max[1];
}

static void	menu_widgetCursor_redraw(menu_widget *w) {
	menu_widgetCursor *p	= w->properties;
	p->s_bg			= menu_RGroup_Mesh_add(w->wl->sprites, p->m_bg);
	p->s_left		= menu_RGroup_Mesh_add(w->wl->sprites, p->m_left);
	p->s_right		= menu_RGroup_Mesh_add(w->wl->sprites, p->m_right);
}

menu_widget*	menu_widgetCursor_new(menu_widgetList *p_wl) {
	menu_widget *ret	= menu_widget_new(p_wl, "cur");
	menu_widgetCursor *p	= calloc(1, sizeof(menu_widgetCursor));
	ret->properties		= p;
	ret->redraw		= menu_widgetCursor_redraw;
	p->m_bg			= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bg"));
	p->m_left		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "lt"));
	p->m_right		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "rt"));
	ret->redraw(ret);
	p_wl->cursor		= ret;
	return ret;
}


/***********************************************************************************
 *				WidgetArrow
 ***********************************************************************************/
static void		menu_widgetArrow_redraw(menu_widget *w) {
	menu_widgetArrow *p	= w->properties;
	p->s_bg		= menu_RGroup_Mesh_add(w->wl->sprites, p->m_bg);
	p->s_arrow	= menu_RGroup_Mesh_add(w->wl->sprites, p->m_arrow);

	menu_RGroup_Text_add(w->wl->text2, p->text);
}

static menu_Segment*	menu_widgetArrow_getbg(menu_widget *w) {
	menu_widgetArrow *p	= w->properties;
	return p->s_bg;
}

static Eina_Bool	menu_widgetArrow_EventManager(menu_widget *w, SDL_Event *e) { // return true if consumed
	menu_widgetArrow *b	= w->properties;
	switch(e->type) {
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
		case SDLK_RETURN:
		case SDLK_RIGHT:
#ifdef PANDORA
		case SDLK_HOME:
		case SDLK_END:
		case SDLK_PAGEDOWN:
		case SDLK_PAGEUP:
#endif
			if (b->onClick != NULL) {
				b->onClick(w);
				return EINA_TRUE;
			} else if (b->target) {
				menu_setUIScene(menu_findScene(b->target));
				return EINA_TRUE;
			}
			
		break;
		default:
		break;
		}
	break;
	case SDL_MOUSEBUTTONUP:
		if (	(e->button.x<=(int)b->s_bg->max[0] &&e->button.x>=(int)b->s_bg->min[0]) &&  
			(e->button.y<=(int)b->s_bg->max[1] &&e->button.y>=(int)b->s_bg->min[1])) {
			if (b->onClick)
				b->onClick(w);
			else if (b->target)
				menu_setUIScene(menu_findScene(b->target));
			menu_widgetScene_setSelected(w->wl, w);
			return EINA_TRUE;
		}
	break;
	}
	return EINA_FALSE;
}

menu_widget*	menu_widgetArrow_new(	menu_widgetList *p_wl, char *p_name, menu_widget_CB onClick, const char *target) {
	menu_widget *ret	= menu_widget_new(p_wl, p_name);
	menu_widgetArrow *p	= calloc(1, sizeof(menu_widgetArrow));
	p->target		= target;
	p->onClick		= onClick;
	ret->properties		= p;
	ret->redraw		= menu_widgetArrow_redraw;
	ret->eventManager	= menu_widgetArrow_EventManager;
	ret->getBackground	= menu_widgetArrow_getbg;
	p->m_bg			= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bg"));
	p->m_arrow		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "arr"));
	p->text			= menu_Theme_find_Text(g, menu_widget_get_frm(ret, "text"));
	ret->redraw(ret);
	return ret;
}


/***********************************************************************************
 *				WidgetButton
 ***********************************************************************************/
static void		menu_widgetButton_redraw(menu_widget *w) {
	menu_widgetButton *p	= w->properties;
	p->sprite		= menu_RGroup_Mesh_add(w->wl->sprites, p->mesh);
	if (p->m_ico)
		p->s_ico	= menu_RGroup_Mesh_add(w->wl->sprites, p->m_ico);

	menu_RGroup_Text_add(w->wl->text3, p->text);
}

static menu_Segment*	menu_widgetButton_getbg(menu_widget *w) {
	menu_widgetButton *p	= w->properties;
	return p->sprite;
}

static Eina_Bool	menu_widgetButton_EventManager(menu_widget *w, SDL_Event *e) { // return true if consumed
	menu_widgetButton *b	= w->properties;
	switch(e->type) {
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
		case SDLK_RETURN:
#ifdef PANDORA
		case SDLK_HOME:
		case SDLK_END:
		case SDLK_PAGEDOWN:
		case SDLK_PAGEUP:
#endif
			if (b->onClick != NULL) {
				b->onClick(w);
				return EINA_TRUE;
			}
		break;
		default:
		break;
		}
	break;
	case SDL_MOUSEBUTTONUP:
		if (	(e->button.x<=(int)b->sprite->max[0] &&e->button.x>=(int)b->sprite->min[0]) &&  
			(e->button.y<=(int)b->sprite->max[1] &&e->button.y>=(int)b->sprite->min[1]) &&
			b->onClick != NULL) {
			b->onClick(w);
			menu_widgetScene_setSelected(w->wl, w);
			menu_widgetScene_redraw(w->wl);
			return EINA_TRUE;
		}
	break;
	}
	return EINA_FALSE;
}

menu_widget*	menu_widgetButton_new(menu_widgetList *p_wl, char *p_name, menu_widget_CB onClick) {
	menu_widget *ret	= menu_widget_new(p_wl, p_name);
	menu_widgetButton *p	= calloc(1, sizeof(menu_widgetButton));
	p->onClick		= onClick;
	ret->properties		= p;
	ret->redraw		= menu_widgetButton_redraw;
	ret->eventManager	= menu_widgetButton_EventManager;
	ret->getBackground	= menu_widgetButton_getbg;
	p->mesh			= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bt"));
	p->m_ico		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "ico"));
	p->text			= menu_Theme_find_Text(g, menu_widget_get_frm(ret, "text"));
	ret->redraw(ret);
	return ret;
}

/***********************************************************************************
 *				WidgetSelectList
 ***********************************************************************************/
static menu_Segment*	menu_widgetSelectList_getbg(menu_widget *w) {
	menu_widgetSelectList *p= w->properties;
	return p->s_bg;
}

static void	menu_widgetSelectList_internRedraw(menu_widget *w) {
	int i = 0;
	float h = 0;
	menu_vector pos;
	menu_vector mv = {0.0,0.0,0.0};
	menu_Segment* s;
	menu_widgetSelectList *p= w->properties;
	menu_RGroup_empty(p->rg_sprite);
	menu_RGroup_empty(p->rg_text);
	menu_vector_cpy(pos, 0, p->t_txt->pos);
	for (i=0;i+p->currentStart<p->itemCount&&i<150;i++) {
		s = menu_RGroup_Mesh_add(p->rg_sprite, p->m_opt);
		h = s->max[1] - s->min[1];
		mv[1] = h*i;
		menu_Segment_move_by(s, mv);
		menu_RGroup_Text_add_at(p->rg_text, p->items[i+p->currentStart].name, pos);
		pos[1]+=h;
		
	}
	p->h = h;
	// place the cursor
	p->s_cur = menu_RGroup_Mesh_add(p->rg_sprite, p->m_alt);
	mv[1] = h*(p->current-p->currentStart);
	menu_Segment_move_by(p->s_cur, mv);
}
static void	menu_widgetSelectList_updateScroll(menu_widget *w) {
	menu_widgetSelectList *p= w->properties;
	uint16_t len = (p->s_bg->max[1]-p->s_bg->min[1])/p->itemCount;
	uint16_t pos = ((p->s_bg->max[1]-p->s_bg->min[1])*p->current)/p->itemCount + p->s_bg->min[1];
	if (len < 10) len  = 10;

	// TODO: fix me this wont always be only a quad
	p->s_sol->dots[4]	= p->s_sol->dots[1]  = pos;
	p->s_sol->dots[7]	= p->s_sol->dots[10] = pos + len;
}

void		menu_widgetSelectList_setItems(menu_widget *w, menu_widgetSelectItem* p_items, uint16_t p_count) {
	menu_vector mv = {0.0,0.0,0.0};
	menu_widgetSelectList *p= w->properties;
	p->items		= p_items;
	p->itemCount		= p_count;
	if (p->current>p->itemCount) {
		p->current	= 0;
		p->currentStart	= 0;
	} else if (p->itemCount<=75 && p->currentStart+75>p->itemCount) {
		p->currentStart=0;
	} else if (p->currentStart+75>p->itemCount) {
		p->currentStart=p->itemCount-75;
	}
	if (p->current<p->currentStart) {
		if (p->currentStart<75)
			p->currentStart=0;
		else
			p->currentStart-=50;
	}else if (p->current>p->currentStart+75) {
		if (p->currentStart>p->itemCount-75)
			p->currentStart=p->itemCount-75;
		else
			p->currentStart+=50;
	}
	menu_RGroup_empty(p->rg_sprite);
	menu_RGroup_empty(p->rg_text);
	if (!p_items)
		return;
	menu_widgetSelectList_internRedraw(w);
	if (p->s_cur->min[1] < p->s_bg->min[1]) {
		mv[1] = p->s_bg->min[1] - p->s_cur->min[1];
		menu_RGroup_move_by(p->rg_sprite, mv);
		menu_RGroup_move_by(p->rg_text, mv);
	}else if (p->s_cur->max[1] > p->s_bg->max[1]) {
		mv[1] = p->s_bg->max[1] - p->s_cur->max[1];
		menu_RGroup_move_by(p->rg_sprite, mv);
		menu_RGroup_move_by(p->rg_text, mv);
	}
	menu_widgetSelectList_updateScroll(w);
}

void		menu_widgetSelectList_move(menu_widget *w, int count) {
	menu_vector mv = {0.0,0.0,0.0};
	int  moved = 0;
	menu_widgetSelectList *p= w->properties;
	mv[1] = p->h*count;
	if ((p->current<=-count && count<=-2) || (p->current<=0 && count<=0) || (p->current>=p->itemCount-count && count>=0)) return;
	menu_Segment_move_by(p->s_cur, mv);
	p->current += count;
	if (p->current<p->currentStart) {
		if (p->currentStart<75)
			p->currentStart=0;
		else
			p->currentStart-=50;
		menu_widgetSelectList_internRedraw(w);
		moved=1;
	}else if (p->current>p->currentStart+75) {
		if (p->currentStart>p->itemCount-75)
			p->currentStart=p->itemCount-75;
		else
			p->currentStart+=50;
		menu_widgetSelectList_internRedraw(w);
		moved=2;
	}
	if (p->onChange) p->onChange(w);
	if (p->s_cur->min[1] < p->s_bg->min[1] || moved==1) {
		mv[1] = p->s_bg->min[1] - p->s_cur->min[1];
		menu_RGroup_move_by(p->rg_sprite, mv);
		menu_RGroup_move_by(p->rg_text, mv);
	}else if (p->s_cur->max[1] > p->s_bg->max[1] || moved==2) {
		mv[1] = p->s_bg->max[1] - p->s_cur->max[1];
		menu_RGroup_move_by(p->rg_sprite, mv);
		menu_RGroup_move_by(p->rg_text, mv);
	}
	menu_widgetSelectList_updateScroll(w);
}

static void menu_widgetSelectList_moveAnime(menu_Anime* a) {
	menu_widget *w = (menu_widget *)a->obj;
	menu_widgetSelectList *p= w->properties;
	if (a->tickLeft==1) {
		a->tickLeft= 9;
		menu_widgetSelectList_move(w, p->mov);
	}
	if (p && p->mov== 0) a->tickLeft=1;
}

static Eina_Bool	menu_widgetSelectList_EventManager(menu_widget *w, SDL_Event *e) { // return true if consumed
	menu_widgetSelectList *b	= w->properties;
	menu_Anime* a;
	int diff;
	switch(e->type) {
	case SDL_KEYDOWN:
		switch(e->key.keysym.sym) {
#ifdef PANDORA
		case SDLK_RSHIFT: // L
#else
		case SDLK_PAGEUP:
#endif
		case SDLK_UP:
			a = menu_Scene_add_anime(w->wl->scene, b->s_cur);
			a->custom = menu_widgetSelectList_moveAnime;
			a->obj = (void*)w;
			b->mov=(e->key.keysym.sym==SDLK_UP)?-1:-10;
			return EINA_TRUE;
		break;
#ifdef PANDORA
		case SDLK_RCTRL:  // R
#else
		case SDLK_PAGEDOWN:
#endif
		case SDLK_DOWN:
			a = menu_Scene_add_anime(w->wl->scene, b->s_cur);
			a->custom = menu_widgetSelectList_moveAnime;
			a->obj = (void*)w;
			b->mov=(e->key.keysym.sym==SDLK_DOWN)?1:10;
			return EINA_TRUE;
		break;
		default:
		break;
		}
	break;
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
#ifdef PANDORA
		case SDLK_RSHIFT: // L
		case SDLK_RCTRL:  // R
#else
		case SDLK_PAGEDOWN:
		case SDLK_PAGEUP:
#endif
		case SDLK_DOWN:
		case SDLK_UP:
			b->mov=0;
			return EINA_TRUE;
		break;
		case SDLK_RETURN:
#ifdef PANDORA
		case SDLK_HOME:
		case SDLK_END:
		case SDLK_PAGEDOWN:
		case SDLK_PAGEUP:
#endif
			if (b->onClick != NULL) {
				b->onClick(w);
				return EINA_TRUE;
			}
		break;
		default:
		break;
		}
	break;
	case SDL_MOUSEBUTTONUP:
		if (	(e->button.x<=(int)b->s_bg->max[0] &&e->button.x>=(int)b->s_bg->min[0]) &&  
			(e->button.y<=(int)b->s_bg->max[1] &&e->button.y>=(int)b->s_bg->min[1])) {
			diff = (e->button.y - b->s_cur->min[1])/b->h;
			if (e->button.y < b->s_cur->min[1]) diff--;
			if (diff !=0)
				menu_widgetSelectList_move(w, diff);
			else if (b->onClick)
				b->onClick(w);
			return EINA_TRUE;
		}
	break;
	}
	return EINA_FALSE;
}

static void		menu_widgetSelectList_redraw(menu_widget *w) {
	menu_widgetSelectList *p= w->properties;
	p->s_bg			= menu_RGroup_Mesh_add(w->wl->sprites, p->m_bg);
	p->s_sol		= menu_RGroup_Mesh_add(w->wl->sprites, p->m_sol);
	menu_widgetSelectList_updateScroll(w);
}

menu_widget*	menu_widgetSelectList_new(menu_widgetList *p_wl, char *p_name, menu_widget_CB onClick, menu_widget_CB onChange) {
	menu_color colr		= {{0.2f, 0.2f, 0.2f, 1.0f}};
	menu_widget *ret	= menu_widget_new(p_wl, p_name);
	menu_widgetSelectList *p= calloc(1, sizeof(menu_widgetSelectList));
	ret->properties		= p;
	ret->eventManager	= menu_widgetSelectList_EventManager;
	ret->redraw		= menu_widgetSelectList_redraw;
	ret->getBackground	= menu_widgetSelectList_getbg;
	p->onClick		= onClick;
	p->onChange		= onChange;
	p->m_bg			= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bg"));
	p->m_sol		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "sol"));
	p->m_opt		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "opt"));
	p->m_alt		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "alt"));
	p->t_txt		= menu_Theme_find_Text(g, menu_widget_get_frm(ret, "txt"));
	menu_widgetSelectList_redraw(ret);

	p->rg_sprite		= menu_RGroupSprite_new(0, EINA_FALSE, EINA_TRUE);
	p->rg_text		= menu_RGroupSprite_new(0, EINA_FALSE, EINA_TRUE);
	
	menu_Scene_add_rgroup(p_wl->scene, p->rg_sprite);
	menu_Scene_add_rgroup(p_wl->scene, p->rg_text);
	menu_RGroup_set_font(p->rg_text, menu_Theme_find_Font(g, "text2"));
	menu_RGroup_uniq_col_set(p->rg_text, colr);

	menu_RGroup_setScissor(p->rg_sprite, (int32_t)p->s_bg->min[0], (int32_t)p->s_bg->max[1], (int32_t)(p->s_bg->max[0]-p->s_bg->min[0]), (int32_t)(p->s_bg->max[1]-p->s_bg->min[1]));
	menu_RGroup_setScissor(p->rg_text,   (int32_t)p->s_bg->min[0], (int32_t)p->s_bg->max[1], (int32_t)(p->s_bg->max[0]-p->s_bg->min[0]), (int32_t)(p->s_bg->max[1]-p->s_bg->min[1]));

	return ret;
}

/***********************************************************************************
 *				WidgetImage
 ***********************************************************************************/
static menu_Segment*	menu_widgetImage_getbg(menu_widget *w) {
	menu_widgetImage *p	= w->properties;
	return p->s_bg;
}

static Eina_Bool	menu_widgetImage_EventManager(menu_widget *w, SDL_Event *e) { // return true if consumed
	menu_widgetImage *b	= w->properties;
	switch(e->type) {
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
		case SDLK_RETURN:
#ifdef PANDORA
		case SDLK_HOME:
		case SDLK_END:
		case SDLK_PAGEDOWN:
		case SDLK_PAGEUP:
#endif
			if (b->onClick != NULL) {
				b->onClick(w);
				return EINA_TRUE;
			}
		break;
		default:
		break;
		}
	break;
	case SDL_MOUSEBUTTONUP:
		if (	(e->button.x<=(int)b->s_bg->max[0] &&e->button.x>=(int)b->s_bg->min[0]) &&  
			(e->button.y<=(int)b->s_bg->max[1] &&e->button.y>=(int)b->s_bg->min[1]) &&
			b->onClick != NULL) {
			b->onClick(w);
			menu_widgetScene_setSelected(w->wl, w);
			menu_widgetScene_redraw(w->wl);
			return EINA_TRUE;
		}
	break;
	}
	return EINA_FALSE;
}

static void		menu_widgetImage_redraw(menu_widget *w) {
	menu_widgetImage *p	= w->properties;
	p->s_bg			= menu_RGroup_Mesh_add(w->wl->sprites, p->m_bg);
}

menu_widget*	menu_widgetImage_new(menu_widgetList *p_wl, char *p_name, menu_Texture *p_t, menu_widget_CB onClick) {
	menu_widget *ret	= menu_widget_new(p_wl, p_name);
	menu_widgetImage *p	= calloc(1, sizeof(menu_widgetImage));
	ret->properties		= p;
	ret->eventManager	= menu_widgetImage_EventManager;
	ret->redraw		= menu_widgetImage_redraw;
	ret->getBackground	= menu_widgetImage_getbg;
	p->onClick		= onClick;
	p->m_bg			= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bg"));
	p->rg			= menu_RGroupSprite_new(1, EINA_FALSE, EINA_TRUE);
	menu_Scene_add_rgroup(p_wl->scene, p->rg);
	menu_RGroup_Texture_set(p->rg, p_t);
	menu_widgetImage_redraw(ret);
	p->s_img		= menu_RGroup_Image_add(p->rg, (int)p->s_bg->min[0]+10, (int)p->s_bg->min[1]+10, (int)p->s_bg->max[0]-10, (int)p->s_bg->max[1]-10);
	return ret;
}


/***********************************************************************************
 *				WidgetTextBox
 ***********************************************************************************/
void		menu_widgetTextBox_setText(menu_widget *w, const char *p_text) {
	menu_widgetTextBox *p	= w->properties;
	p->text			= (char*)p_text;
	menu_widgetScene_redraw(w->wl);
}
static void		menu_widgetTextBox_redraw(menu_widget *w) {
	menu_widgetTextBox *p	= w->properties;
	p->s_bg			= menu_RGroup_Mesh_add(w->wl->sprites, p->m_bg);
	menu_RGroup_Text_add_bounded(w->wl->text3, p->text, p->s_bg->min, p->s_bg->max);
}

menu_widget*	menu_widgetTextBox_new(	menu_widgetList *p_wl, char *p_name) {
	menu_widget *ret	= menu_widget_new(p_wl, p_name);
	menu_widgetTextBox *p	= calloc(1, sizeof(menu_widgetTextBox));
	ret->properties		= p;
	ret->redraw		= menu_widgetTextBox_redraw;
	p->m_bg			= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bg"));

	menu_widgetTextBox_redraw(ret);
	return ret;
}

/***********************************************************************************
 *				WidgetTextInput
 ***********************************************************************************/

static void		menu_widgetTextInput_redraw(menu_widget *w) {
	menu_widgetTextInput *p	= w->properties;
	p->s_bg			= menu_RGroup_Mesh_add(w->wl->sprites, p->m_bg);
	p->s_cell		= menu_RGroup_Mesh_add(w->wl->sprites, p->m_cell);
	menu_RGroup_Text_add(w->wl->text2, p->t_desc);
	menu_RGroup_Text_add_at(w->wl->text3, p->text, p->t_opt->pos);
}

static menu_Segment*	menu_widgetTextInput_getbg(menu_widget *w) {
	menu_widgetTextInput *p	= w->properties;
	return p->s_bg;
}

static Eina_Bool	menu_widgetTextInput_EventManager(menu_widget *w, SDL_Event *e) {
	menu_widgetTextInput *p	= w->properties;
	int l = strlen(p->text);
	switch(e->type) {
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
		case SDLK_BACKSPACE:
		case SDLK_DELETE:
			if (l==0) break;
			p->text[l-1] = 0;
			menu_widgetScene_redraw(w->wl);
			return EINA_TRUE;
		break;
		default:
			if ( e->key.keysym.sym >= SDLK_a && e->key.keysym.sym <= SDLK_z && l < 15) {
				p->text[l] = e->key.keysym.sym-SDLK_a+'a';
				menu_widgetScene_redraw(w->wl);
				return EINA_TRUE;
			}
		break;
		}
	break;
	default:
	break;
	}
	return EINA_FALSE;
}

menu_widget*	menu_widgetTextInput_new(menu_widgetList *p_wl, char *p_name) {
	menu_widget *ret	= menu_widget_new(p_wl, p_name);
	menu_widgetTextInput *p	= calloc(1, sizeof(menu_widgetTextInput));
	ret->properties		= p;
	ret->redraw		= menu_widgetTextInput_redraw;
	ret->eventManager	= menu_widgetTextInput_EventManager;
	ret->getBackground	= menu_widgetTextInput_getbg;
	p->m_bg			= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bg"));
	p->m_cell		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "cell"));
	p->t_desc		= menu_Theme_find_Text(g, menu_widget_get_frm(ret, "desc"));
	p->t_opt		= menu_Theme_find_Text(g, menu_widget_get_frm(ret, "opt"));

	menu_widgetTextInput_redraw(ret);
	return ret;
}

/***********************************************************************************
 *				WidgetSelect
 ***********************************************************************************/
static void		menu_widgetSelect_redraw(menu_widget *w) {
	menu_widgetSelect *p	= w->properties;
	p->s_bg			= menu_RGroup_Mesh_add(w->wl->sprites, p->m_bg);
	p->s_back		= menu_RGroup_Mesh_add(w->wl->sprites, p->m_back);
	p->s_next		= menu_RGroup_Mesh_add(w->wl->sprites, p->m_next);
	menu_RGroup_Text_add(w->wl->text2, p->desc);
	menu_RGroup_Text_add_at(w->wl->text3, p->values[p->selected], p->option->pos);
}

static menu_Segment*	menu_widgetSelect_getbg(menu_widget *w) {
	menu_widgetSelect *p	= w->properties;
	return p->s_bg;
}

static Eina_Bool	menu_widgetSelect_EventManager(menu_widget *w, SDL_Event *e) { // return true if consumed
	menu_widgetSelect *s	= w->properties;
	switch(e->type) {
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
		case SDLK_LEFT:
#ifdef PANDORA
		case SDLK_RSHIFT:
#endif
			if (s->selected > 0) {
				s->selected--;
				if (s->onChange != NULL)
					s->onChange(w);
				menu_widgetScene_redraw(w->wl);
				return EINA_TRUE;
			}
		break;
		case SDLK_RIGHT:
#ifdef PANDORA
		case SDLK_RCTRL:
#endif
			if (s->selected < s->maxVal) {
				s->selected++;
				if (s->onChange != NULL)
					s->onChange(w);
				menu_widgetScene_redraw(w->wl);
				return EINA_TRUE;
			}
		break;
		default:
		break;
		}
	break;
	case SDL_MOUSEBUTTONUP:
		if (	(e->button.x<=(int)s->s_back->max[0] &&e->button.x>=(int)s->s_back->min[0]) &&  
			(e->button.y<=(int)s->s_back->max[1] &&e->button.y>=(int)s->s_back->min[1]) &&
			s->selected > 0) {
			s->selected--;
			if (s->onChange != NULL)
				s->onChange(w);
			menu_widgetScene_setSelected(w->wl, w);
			menu_widgetScene_redraw(w->wl);
			return EINA_TRUE;
		}
		if (	(e->button.x<=(int)s->s_next->max[0] &&e->button.x>=(int)s->s_next->min[0]) &&  
			(e->button.y<=(int)s->s_next->max[1] &&e->button.y>=(int)s->s_next->min[1]) &&
			s->selected < s->maxVal) {
			s->selected++;
			if (s->onChange != NULL)
				s->onChange(w);
			menu_widgetScene_setSelected(w->wl, w);
			menu_widgetScene_redraw(w->wl);
			return EINA_TRUE;
		}
	break;
	}
	return EINA_FALSE;
}

void		menu_widgetSelect_setValues(menu_widget *w, const char **values) {
	int	i;
	menu_widgetSelect *s	= w->properties;
	s->values		= (char**)values;
	for(i=0;values[i];i++);
	s->maxVal		= i-1;
	w->redraw(w);
}

menu_widget*	menu_widgetSelect_new(menu_widgetList *p_wl, char *p_name, const char **values, const int sel, menu_widget_CB onChange) {
	menu_widget *ret	= menu_widget_new(p_wl, p_name);
	menu_widgetSelect *p	= calloc(1, sizeof(menu_widgetSelect));
	ret->properties		= p;
	ret->redraw		= menu_widgetSelect_redraw;
	ret->eventManager	= menu_widgetSelect_EventManager;
	ret->getBackground	= menu_widgetSelect_getbg;
	p->onChange		= onChange;
	p->selected		= sel;
	p->m_bg			= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bg"));
	p->m_back		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bk"));
	p->m_next		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "nt"));
	p->option		= menu_Theme_find_Text(g, menu_widget_get_frm(ret, "opt"));
	p->desc			= menu_Theme_find_Text(g, menu_widget_get_frm(ret, "desc"));
	menu_widgetSelect_setValues(ret, values);
	return ret;
}

/***********************************************************************************
 *				WidgetProgress
 ***********************************************************************************/
static void		menu_widgetProgress_redraw(menu_widget *w) {
	char 	txt[8];
	menu_widgetProgress *p	= w->properties;
	p->s_bg			= menu_RGroup_Mesh_add(w->wl->sprites, p->m_bg);
	p->s_top		= menu_RGroup_Mesh_add(w->wl->sprites, p->m_top);
	sprintf(txt, "%03d%%", p->value);
	menu_RGroup_Text_add_at(w->wl->text3, txt, p->t_pct->pos);
	
	// TODO: fix me this wont always be only a quad
	p->s_top->dots[6] = p->s_top->dots[3]	= p->s_top->min[0] +(p->s_top->max[0]-p->s_top->min[0])*p->value/100;
}

menu_widget*	menu_widgetProgress_new(menu_widgetList *p_wl, char *p_name, uint16_t value) {
	menu_widget *ret	= menu_widget_new(p_wl, p_name);
	menu_widgetProgress *p	= calloc(1, sizeof(menu_widgetProgress));
	p->value		= value;
	ret->properties		= p;
	ret->redraw		= menu_widgetProgress_redraw;
	p->m_bg			= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "bg"));
	p->m_top		= menu_Theme_find_Mesh(g, menu_widget_get_frm(ret, "top"));
	p->t_pct		= menu_Theme_find_Text(g, menu_widget_get_frm(ret, "pct"));
	ret->redraw(ret);
	return ret;
}

void		menu_widgetProgress_setPct(menu_widget *w, uint16_t pct) {
	menu_widgetProgress *p	= w->properties;
	p->value		= pct;
	menu_widgetScene_redraw(w->wl);
}

/***********************************************************************************
 *				Scenes
 ***********************************************************************************/

menu_Anime*	menu_Anime_new(menu_Segment* s) {
	menu_Anime* ret	= calloc(1, sizeof(menu_Anime));
	ret->seg	= s;
	ret->onEnd	= NULL;
	ret->custom	= NULL;
	ret->tickLeft	= 1;
	ret->scale	= 0.0;
	ret->obj	= NULL;
	ret->objid	= 0;
	menu_vector_set(ret->move, 0, 0.0, 0.0, 0.0);
	menu_vector_set(ret->rot,  0, 0.0, 0.0, 0.0);
	return ret;
}

menu_Scene*	menu_Scene_new(const char *p_name) {
	menu_Scene* ret	= calloc(1, sizeof(menu_Scene));
	ret->name	= p_name;
	ret->rgroups	= NULL;
	ret->animes	= NULL;
	ret->joystickR	= NULL;
	ret->onEvent	= NULL;
	ret->wl		= NULL;
	ret->isEmu	= EINA_FALSE;
	menu_addScene(ret);
	return ret;
}

void		menu_Scene_free(menu_Scene* s) {
	menu_RGroup *m;
	menu_Anime   *a;
	EINA_LIST_FREE(s->animes, a) {
		free(a);
	}
	EINA_LIST_FREE(s->rgroups, m) {
		menu_RGroup_free(m);
	}
}

void		menu_Scene_empty(menu_Scene* s) {
	menu_RGroup *m;
	menu_Anime   *a;
	EINA_LIST_FREE(s->animes, a) {
		free(a);
	}
	EINA_LIST_FREE(s->rgroups, m) {
		menu_RGroup_free(m);
	}
	s->rgroups	= NULL;
}


void		menu_Scene_draw(menu_Scene* s) {
	Eina_List*	l1;
	menu_RGroup	*m;

	EINA_LIST_FOREACH(s->rgroups, l1, m) {
		menu_RGroup_draw(m);
	}
}

void		menu_Scene_runTick(menu_Scene* s) {
	Eina_List*	l1;
	Eina_List*	l2 = NULL;
	menu_Anime	*a;
	EINA_LIST_FOREACH(s->animes, l1, a) {
		if (a!=NULL && a->tickLeft>0) {
			if (a->custom != NULL) {
				a->custom(a);
			} else if (a->seg == NULL) {
				menu_Screen_View_move(a->move);
			} else {
				if (a->scale != 0.0)
					menu_Segment_scale_by(a->seg, a->scale);
				if (a->move[0] != 0.0 || a->move[1] != 0.0 || a->move[2] != 0.0)
					menu_Segment_move_by(a->seg, a->move);
				if (a->rot[0] != 0.0 || a->rot[1] != 0.0 || a->rot[2] != 0.0)
					menu_Segment_rotate_by(a->seg, a->rot);
			}
			a->tickLeft--;
			if (a->tickLeft == 0)
				l2 = eina_list_append(l2, a);
		} else if (a!=NULL)
			l2 = eina_list_append(l2, a);
	}
	EINA_LIST_FREE(l2, a) {
		if (a->onEnd != NULL)
			a->onEnd(a);
		if (a->tickLeft<=0) {
			s->animes  = eina_list_remove(s->animes, a);
			free(a);
		}
	}
}

menu_Anime*	menu_Scene_add_anime(menu_Scene* s, menu_Segment* seg) {
	menu_Anime* ret = menu_Anime_new(seg);
	s->animes = eina_list_append(s->animes, ret);
	return ret;
}

void		menu_Scene_add_rgroup(menu_Scene* s, menu_RGroup *m) {
	s->rgroups = eina_list_append(s->rgroups, m);
}
