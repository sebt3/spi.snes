#include "mesh.h"
#include "theme.h"


static unsigned short squareIndex[6] = { 0, 1, 2, 2, 3, 0 };

menu_Text*	menu_Text_new(const char *m_name, char *p_font, float* p_pos, char *p_text) {
	menu_Text* ret		= calloc(1, sizeof(menu_Text));
	ret->name		= eina_stringshare_add(m_name);
	ret->font		= eina_stringshare_add(p_font);
	ret->text		= (char*)eina_stringshare_add(p_text);
	menu_vector_cpy(ret->pos, 0, p_pos);
	return ret;
}

void		menu_Text_free(menu_Text* s) {
	menu_String_Free(s->name);
	menu_String_Free(s->font);
	menu_String_Free(s->text);
}

menu_Mesh*	menu_Mesh_new(const char *m_name, const uint32_t d, const uint32_t f, const GLenum m) {
	menu_Mesh* ret		= calloc(1, sizeof(menu_Mesh));
	ret->name		= eina_stringshare_add(m_name);
	ret->mode		= m;
	ret->dots_max		= d*3;
	ret->faces_max		= f;
	ret->dots		= NULL;
	ret->cols		= NULL;
	ret->tex		= NULL;
	ret->norm		= NULL;
	ret->faces		= NULL;
	ret->t			= NULL;
	ret->dots_count		= 0;
	ret->cols_count		= 0;
	ret->tex_count		= 0;
	ret->norm_count		= 0;
	ret->alpha		= EINA_FALSE;
	ret->use_sub		= EINA_FALSE;
	ret->is_tile		= EINA_FALSE;
	ret->loaded		= EINA_FALSE;
	ret->tilemap_w		= 0;
	ret->tilemap_h		= 0;
	menu_color_set(ret->col, 1.0, 1.0, 1.0, 1.0);
	// lighting info :
	menu_color_set(ret->ambiant,	1.0, 1.0, 1.0, 1.0);
	menu_color_set(ret->difuse,	1.0, 1.0, 1.0, 1.0);
	menu_color_set(ret->specular,	1.0, 1.0, 1.0, 1.0);
	menu_color_set(ret->transfilter,1.0, 1.0, 1.0, 1.0);
	return ret;
}

void		menu_Mesh_free(menu_Mesh* s) {
	menu_String_Free(s->name);
	if (s->dots != NULL)
		free(s->dots);
	if (!s->loaded) {
		if (s->cols != NULL)
			free(s->cols);
		if (s->tex != NULL)
			free(s->tex);
		if (s->norm != NULL)
			free(s->norm);
		if (s->faces != NULL)
			free(s->faces);
	}
	// Dont free the texture here as they are shared
	free(s);
}

void		menu_Mesh_finalize(menu_Mesh* s) {
	// hide last useless break.
	if(s->faces_count>3 && s->use_sub)
		s->faces_count-=3;
}

void		menu_Mesh_Texture_set(menu_Mesh* s, menu_Texture* t) {
	if(t==NULL)	return;
	s->t		= t;
	s->texture_name	= eina_stringshare_add(t->name);
	if (t->format  == menu_TEXTURE_FORMAT_4444 || t->format == menu_TEXTURE_FORMAT_ALPHA)
		s->alpha= EINA_TRUE;
}

void		menu_Mesh_uniq_col_set(menu_Mesh* s, const menu_color c) {
	menu_color_copy(s->col, c);
}

void		menu_Mesh_dot_set(menu_Mesh* s, const int id, const menu_vector p) {
	if (s->dots == NULL)
		s->dots = calloc(s->dots_max, sizeof(float));
	menu_vector_cpy(s->dots, id, p);
}

void		menu_Mesh_col_set(menu_Mesh* s, const int id, const menu_color c) {
	if (s->cols == NULL)
		s->cols = calloc(s->dots_max/3*4, sizeof(float));
	menu_color_cpy(s->cols, id, c);
	if (c.t[3]<1.0)
		s->alpha	= EINA_TRUE;
}

void		menu_Mesh_tex_set(menu_Mesh* s, const int id, const menu_texC t) {
	if (s->tex == NULL)
		s->tex = calloc(s->dots_max/3*2, sizeof(float));
	menu_texC_set(s->tex, id, t[0], t[1]);
}

void		menu_Mesh_norm_set(menu_Mesh* s, const int id, const menu_vector p) {
	if (s->norm == NULL)
		s->norm = calloc(s->dots_max, sizeof(float));
	menu_vector_set(s->norm, id, p[0], p[1], p[2]);
}

void		menu_Mesh_finish_sub(menu_Mesh* s, const uint16_t d, const uint16_t f, const unsigned short* idx) {
	int i;
	//if(s->faces_count>s->faces_max) printf("FAILURE:%d>%d\n",s->faces_count,s->faces_max);
	if (idx==NULL) return;
	// copy index and offset by the sub position
	if (s->faces == NULL)
		s->faces = calloc(s->faces_max, sizeof(unsigned short));
	for(i=0;i<f;i++)
		s->faces[i+s->faces_count] = idx[i] + s->dots_count/3;
	// add an invalid tri in the index to create a break
	s->faces[f+s->faces_count+0] = idx[f-1] + s->dots_count/3;
	s->faces[f+s->faces_count+1] = idx[f-1] + s->dots_count/3;
	s->faces[f+s->faces_count+2] = idx[f-1] + s->dots_count/3;
	// improve previous break
	if(s->faces_count>3)
		s->faces[s->faces_count-1] = s->faces[s->faces_count];

	s->dots_count += d*3;
	s->faces_count += f+3;
	if (s->cols != NULL)
		s->cols_count += d*4;
	if (s->tex != NULL)
		s->tex_count += d*2;
	s->use_sub = EINA_TRUE;
}

void		menu_Mesh_dot_sub(menu_Mesh* s, const int id, const menu_vector p) {
	if (s->dots == NULL)
		s->dots = calloc(s->dots_max, sizeof(float));
	menu_vector_cpy(s->dots, (id+s->dots_count/3), p);
}

void		menu_Mesh_col_sub(menu_Mesh* s, const int id, const menu_color c) {
	if (s->cols == NULL)
		s->cols = calloc(s->dots_max/3*4, sizeof(float));
	menu_color_cpy(s->cols, (id+s->dots_count/3), c);
	if (c.t[3]<1.0)
		s->alpha	= EINA_TRUE;
}

void		menu_Mesh_tex_sub(menu_Mesh* s, const int id, const menu_texC t) {
	if (s->tex == NULL)
		s->tex = calloc(s->dots_max/3*2, sizeof(float));
	menu_texC_cpy(s->tex, (id+s->dots_count/3), t);
}

void		menu_Mesh_norm_sub(menu_Mesh* s, const int id, const menu_vector p) {
	if (s->norm == NULL)
		s->norm = calloc(s->dots_max, sizeof(float));
	menu_vector_cpy(s->norm, (3*id+s->dots_count/3), p);
}

void		_menu_Mesh_add_rect_vtx(menu_Mesh* s, menu_vector vtl, menu_vector vbr) {
	menu_vector p0;
	int scale = 1;
	if (s->t->scale) scale = 2;

	p0[1] = vtl[1]*scale;	p0[2] = vtl[2];
	p0[0] = vtl[0]*scale;	menu_Mesh_dot_sub(s, 0, p0);
	p0[0] = vbr[0]*scale;	menu_Mesh_dot_sub(s, 1, p0);
	p0[1] = vbr[1]*scale;	menu_Mesh_dot_sub(s, 2, p0);
	p0[0] = vtl[0]*scale;	menu_Mesh_dot_sub(s, 3, p0);
}

void		menu_Mesh_add_rect(menu_Mesh* s, menu_vector vtl, menu_vector vbr) {
	_menu_Mesh_add_rect_vtx(s,vtl,vbr);
	menu_Mesh_finish_sub(s, 4, 6, squareIndex);
}

void		menu_Mesh_add_sprite(menu_Mesh* m, menu_vector vtl, menu_vector vbr, menu_sprite_coord* sc) {
	if (m->tex == NULL)
		m->tex = calloc(m->dots_max/3*2, sizeof(float));
	_menu_Mesh_add_rect_vtx(m,vtl,vbr);
	menu_texC_cpys(m->tex, (m->dots_count/3), sc->tex, 4);
	menu_Mesh_finish_sub(m, 4, 6, squareIndex);
}

void		menu_Mesh_add_sprite_boxed(menu_Mesh* m, menu_vector vtl, menu_vector vbr, menu_sprite_coord* sc, int b) {
	menu_vector v2tl;
	menu_vector v2br;
	menu_sprite_coord sc2;

	// Top-left
	menu_vector_set(v2tl, 0, vtl[0],   vtl[1],   vtl[2]);
	menu_vector_set(v2br, 0, vtl[0]+b, vtl[1]+b, vtl[2]);
	menu_texC_set(sc2.tex, 0, sc->tex[0],			 sc->tex[1]);
	menu_texC_set(sc2.tex, 1, sc->tex[0]+((float)b)/m->t->w, sc->tex[1]);
	menu_texC_set(sc2.tex, 2, sc->tex[0]+((float)b)/m->t->w, sc->tex[1]+((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 3, sc->tex[0],			 sc->tex[1]+((float)b)/m->t->w);
	menu_Mesh_add_sprite(m, v2tl, v2br, &sc2);
	// Top
	menu_vector_set(v2tl, 0, vtl[0]+b, vtl[1],   vtl[2]);
	menu_vector_set(v2br, 0, vbr[0]-b, vtl[1]+b, vtl[2]);
	menu_texC_set(sc2.tex, 0, sc->tex[0]+((float)b)/m->t->w, sc->tex[1]);
	menu_texC_set(sc2.tex, 1, sc->tex[4]-((float)b)/m->t->w, sc->tex[1]);
	menu_texC_set(sc2.tex, 2, sc->tex[4]-((float)b)/m->t->w, sc->tex[1]+((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 3, sc->tex[0]+((float)b)/m->t->w, sc->tex[1]+((float)b)/m->t->w);
	menu_Mesh_add_sprite(m, v2tl, v2br, &sc2);
	// Top-right
	menu_vector_set(v2tl, 0, vbr[0]-b, vtl[1],   vtl[2]);
	menu_vector_set(v2br, 0, vbr[0],   vtl[1]+b, vtl[2]);
	menu_texC_set(sc2.tex, 0, sc->tex[4]-((float)b)/m->t->w, sc->tex[1]);
	menu_texC_set(sc2.tex, 1, sc->tex[4],			 sc->tex[1]);
	menu_texC_set(sc2.tex, 2, sc->tex[4],			 sc->tex[1]+((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 3, sc->tex[4]-((float)b)/m->t->w, sc->tex[1]+((float)b)/m->t->w);
	menu_Mesh_add_sprite(m, v2tl, v2br, &sc2);

	// left
	menu_vector_set(v2tl, 0, vtl[0],   vtl[1]+b, vtl[2]);
	menu_vector_set(v2br, 0, vtl[0]+b, vbr[1]-b, vtl[2]);
	menu_texC_set(sc2.tex, 0, sc->tex[0],			 sc->tex[1]+((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 1, sc->tex[0]+((float)b)/m->t->w, sc->tex[1]+((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 2, sc->tex[0]+((float)b)/m->t->w, sc->tex[5]-((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 3, sc->tex[0],			 sc->tex[5]-((float)b)/m->t->w);
	menu_Mesh_add_sprite(m, v2tl, v2br, &sc2);
	// Center
	menu_vector_set(v2tl, 0, vtl[0]+b, vtl[1]+b, vtl[2]);
	menu_vector_set(v2br, 0, vbr[0]-b, vbr[1]-b, vtl[2]);
	menu_texC_set(sc2.tex, 0, sc->tex[0]+((float)b)/m->t->w, sc->tex[1]+((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 1, sc->tex[4]-((float)b)/m->t->w, sc->tex[1]+((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 2, sc->tex[4]-((float)b)/m->t->w, sc->tex[5]-((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 3, sc->tex[0]+((float)b)/m->t->w, sc->tex[5]-((float)b)/m->t->w);
	menu_Mesh_add_sprite(m, v2tl, v2br, &sc2);
	// right
	menu_vector_set(v2tl, 0, vbr[0]-b, vtl[1]+b,  vtl[2]);
	menu_vector_set(v2br, 0, vbr[0],   vbr[1]-b,  vtl[2]);
	menu_texC_set(sc2.tex, 0, sc->tex[4]-((float)b)/m->t->w, sc->tex[1]+((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 1, sc->tex[4],			 sc->tex[1]+((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 2, sc->tex[4],			 sc->tex[5]-((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 3, sc->tex[4]-((float)b)/m->t->w, sc->tex[5]-((float)b)/m->t->w);
	menu_Mesh_add_sprite(m, v2tl, v2br, &sc2);

	// bottom-left
	menu_vector_set(v2tl, 0, vtl[0],   vbr[1]-b, vtl[2]);
	menu_vector_set(v2br, 0, vtl[0]+b, vbr[1],   vtl[2]);
	menu_texC_set(sc2.tex, 0, sc->tex[0],			 sc->tex[5]-((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 1, sc->tex[0]+((float)b)/m->t->w, sc->tex[5]-((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 2, sc->tex[0]+((float)b)/m->t->w, sc->tex[5]);
	menu_texC_set(sc2.tex, 3, sc->tex[0],			 sc->tex[5]);
	menu_Mesh_add_sprite(m, v2tl, v2br, &sc2);
	// bottom
	menu_vector_set(v2tl, 0, vtl[0]+b, vbr[1]-b, vtl[2]);
	menu_vector_set(v2br, 0, vbr[0]-b, vbr[1],   vtl[2]);
	menu_texC_set(sc2.tex, 0, sc->tex[0]+((float)b)/m->t->w, sc->tex[5]-((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 1, sc->tex[4]-((float)b)/m->t->w, sc->tex[5]-((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 2, sc->tex[4]-((float)b)/m->t->w, sc->tex[5]);
	menu_texC_set(sc2.tex, 3, sc->tex[0]+((float)b)/m->t->w, sc->tex[5]);
	menu_Mesh_add_sprite(m, v2tl, v2br, &sc2);
	// bottom-right
	menu_vector_set(v2tl, 0, vbr[0]-b, vbr[1]-b,  vtl[2]);
	menu_vector_set(v2br, 0, vbr[0],   vbr[1],    vtl[2]);
	menu_texC_set(sc2.tex, 0, sc->tex[4]-((float)b)/m->t->w, sc->tex[5]-((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 1, sc->tex[4],			 sc->tex[5]-((float)b)/m->t->w);
	menu_texC_set(sc2.tex, 2, sc->tex[4],			 sc->tex[5]);
	menu_texC_set(sc2.tex, 3, sc->tex[4]-((float)b)/m->t->w, sc->tex[5]);
	menu_Mesh_add_sprite(m, v2tl, v2br, &sc2);
}

void		menu_Mesh_add_texture_part(menu_Mesh* m, menu_vector vtl, menu_texC ttl, menu_texC tbr) {
	menu_vector vbr = {0.0, 0.0, 0.0};
	menu_texC   t0;
	vbr[0] = vtl[0] + tbr[0] - ttl[0];
	vbr[1] = vtl[1] + tbr[1] - ttl[1];
	vbr[2] = vtl[2];
	_menu_Mesh_add_rect_vtx(m,vtl,vbr);
	t0[0] = (ttl[0]+0.1)/m->t->w;
	t0[1] = (ttl[1]+0.1)/m->t->h;menu_Mesh_tex_sub(m, 0, t0);
	t0[0] = (tbr[0]-0.1)/m->t->w;menu_Mesh_tex_sub(m, 1, t0);
	t0[1] = (tbr[1]-0.1)/m->t->h;menu_Mesh_tex_sub(m, 2, t0);
	t0[0] = (ttl[0]+0.1)/m->t->w;menu_Mesh_tex_sub(m, 3, t0);
	menu_Mesh_finish_sub(m, 4, 6, squareIndex);
}

void		menu_Mesh_add_tile(menu_Mesh* m, menu_vector p, int32_t id) {
	int maxts, ix, iy;
	menu_texC   tl = {0.0, 0.0};
	menu_texC   br = {0.0, 0.0};

	if (! m->t->tilesheet)	return;
	if (id == 0)		return;
	maxts	= m->t->orig_w/m->t->tile_w;
	iy	= (id-1)/maxts;
	ix	= (id-1) - iy*maxts;
	menu_texC_set(tl, 0, (float)(ix*m->t->tile_w),     (float)(iy*m->t->tile_h));
	menu_texC_set(br, 0, (float)((ix+1)*m->t->tile_w), (float)((iy+1)*m->t->tile_h));
	menu_Mesh_add_texture_part(m, p, tl, br);
}

void		menu_Mesh_add_tilemap_tile(menu_Mesh* m, menu_vector o, int32_t x, int32_t y, int32_t id) {
	int maxts, ix, iy;
	menu_vector p  = {0.0, 0.0, 0.0};
	menu_texC   tl = {0.0, 0.0};
	menu_texC   br = {0.0, 0.0};

	if (! m->t->tilesheet) 	return;
	if (id == 0)		return;
	maxts = m->t->orig_w/m->t->tile_w;
	iy = (id-1)/maxts;
	ix = (id-1) - iy*maxts;
	menu_vector_set(p, 0,(float)(x*m->t->tile_w)+o[0], (float)(y*m->t->tile_h)+o[1], o[2]);
	menu_texC_set(tl, 0, (float)(ix*m->t->tile_w),     (float)(iy*m->t->tile_h));
	menu_texC_set(br, 0, (float)((ix+1)*m->t->tile_w), (float)((iy+1)*m->t->tile_h));
	menu_Mesh_add_texture_part(m, p, tl, br);
}
