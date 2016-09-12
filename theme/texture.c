#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <Eina.h>
#include <Eet.h>
#include <stdlib.h>

#include "texture.h"
#include "theme.h"

static __inline__ int p2(int o) {	// build next power of 2 number
	int v = 1;
	while (v < o)
		v <<= 1;
	return v;
}

menu_Texture*    menu_Texture_new(const char *p_name, const unsigned int w, const unsigned int h) {
	menu_Texture* ret = calloc(1, sizeof(menu_Texture));

	ret->name	= eina_stringshare_add(p_name);
	ret->w		= p2(w);
	ret->h		= p2(h);
	ret->orig_w	= w;
	ret->orig_h	= h;
	ret->data	= calloc(ret->w*ret->h, sizeof(uint16_t));
	ret->format	= menu_TEXTURE_FORMAT_4444;
	ret->spritesheet= EINA_FALSE;
	ret->tilesheet	= EINA_FALSE;
	ret->scale	= EINA_FALSE;
	ret->tile_w	= 0;
	ret->tile_h	= 0;
	ret->pbo	= 0;
	ret->sprites	= NULL;
	ret->stream1	= NULL;
	ret->stream2	= NULL;
	ret->isUpdated	= EINA_FALSE;
	ret->isStream	= EINA_FALSE;
	ret->use_PBO	= EINA_FALSE;
	ret->sourceData	= NULL;
	ret->filter	= NULL;

	memset(ret->data, 0, w*h*sizeof(uint16_t));

	return ret;
}

void		menu_Texture_free(menu_Texture *t) {
	menu_sprite_coord* sc;
	if (!t)		return;
	if (t->data)	free(t->data);
	EINA_LIST_FREE(t->sprites, sc) {
		menu_String_Free(sc->name);
		free(sc);
	}
	menu_String_Free(t->name);
	free(t);
}
void		menu_Texture_add_sprite(menu_Texture* t, const char *s_name, menu_texC tl, menu_texC br) {
	menu_sprite_coord* sc = calloc(1, sizeof(menu_sprite_coord));
	sc->name	= eina_stringshare_add(s_name);
	sc->sw		= (int)br[0]-(int)tl[0];
	sc->sh		= (int)br[1]-(int)tl[1];
	menu_texC_set(sc->tex, 0, (tl[0]+0.1)/t->w, (tl[1]+0.1)/t->h);
	menu_texC_set(sc->tex, 1, (br[0]-0.1)/t->w, sc->tex[1]);
	menu_texC_set(sc->tex, 2, sc->tex[2],       (br[1]-0.1)/t->h);
	menu_texC_set(sc->tex, 3, (tl[0]+0.1)/t->w, sc->tex[5]);
	t->sprites	= eina_list_append(t->sprites, sc);
}

menu_sprite_coord* menu_Texture_get_sprite(menu_Texture* t, const char *s_name) {
	Eina_List*	l;
	menu_sprite_coord* sc;

	EINA_LIST_FOREACH(t->sprites, l, sc) {
		if (!strcmp(sc->name, s_name))
			return sc;
	}
	return NULL;
}

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

menu_Font*	menu_Font_new(const char *ttfFile, const unsigned int size, const char *name) {
	unsigned char ttf_buffer[1<<20];

	menu_Font* ret	= calloc(1, sizeof(menu_Font));
	FILE* f		= fopen(ttfFile, "rb");

	fread(ttf_buffer, 1, 1<<20, f);
	fclose(f);

	ret->name	= name;

	ret->tex_name	= calloc(strlen(name)+5,sizeof(char));
	sprintf(ret->tex_name, "fnt-%s", name);
	ret->texture	= menu_Texture_new(ret->tex_name, 512, 512);
	if (ret->texture != NULL) {
		ret->texture->format	= menu_TEXTURE_FORMAT_ALPHA;
		free(ret->texture->data);
		ret->texture->data = calloc(512*512, sizeof(unsigned char));
		stbtt_BakeFontBitmap(ttf_buffer,0, (float)size,   (unsigned char *)ret->texture->data, 512,512, 32,96, ret->cdata); // no guarantee this fits!
	}

	ret->tex_name2	= calloc(strlen(name)+7,sizeof(char));
	sprintf(ret->tex_name2, "fnt-%s-2", name);
	ret->texture2	= menu_Texture_new(ret->tex_name2, 512, 512);
	if (ret->texture2 != NULL) {
		ret->texture2->format	= menu_TEXTURE_FORMAT_ALPHA;
		free(ret->texture2->data);
		ret->texture2->data = calloc(512*512, sizeof(unsigned char));
		stbtt_BakeFontBitmap(ttf_buffer,0, (float)size*2, (unsigned char *)ret->texture2->data,512,512, 32,96, ret->cdata2); // no guarantee this fits!
	}
	return ret;
}

void		menu_Font_free(menu_Font* f) {
	free(f);
}

menu_Texture*	menu_Texture_load_from(const char *p_name, const char *p_filename) {
	SDL_Surface *dest	= NULL;
	SDL_Surface *sur	= IMG_Load(p_filename);
	menu_Texture *ret	= NULL;
	uint16_t *a;
	int i;
	if (!sur) {
		fprintf(stderr, "Could not load image '%s'.\n", p_filename);
		return NULL;
	}
	int w = p2(sur->w);
	int h = p2(sur->h);
	ret = menu_Texture_new(p_name, w, sur->h);
	if (!ret) {
		fprintf(stderr, "Could not create texture for '%s'.\n", p_filename);
		SDL_FreeSurface(sur);
		return NULL;
	}
	ret->orig_w	= sur->w;
	ret->h		= h;
	ret->orig_h	= sur->h;
	if (sur->format->Amask>0) {
		// use 4444 texture format
		dest = SDL_CreateRGBSurface(0, w, sur->h, 16, 0xf000, 0x0f00, 0x00f0, 0x000f);
		a = (uint16_t *)dest->pixels;
		for (i=0;i<w*sur->h;i++)
			a[i]=0x555f;
		ret->format = menu_TEXTURE_FORMAT_4444;
	} else {
		// use 565 texture format
		dest = SDL_CreateRGBSurface(0, w, sur->h, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000);
		ret->format = menu_TEXTURE_FORMAT_656;
	}
	SDL_BlitSurface(sur, 0, dest, 0);
	if (ret->format == menu_TEXTURE_FORMAT_4444) {
		a = (uint16_t *)dest->pixels;
		for (i=0;i<w*sur->h;i++)
			if (a[i]==0x555f)
 				a[i]=0;
	}

	SDL_FreeSurface(sur);
	memcpy(ret->data, dest->pixels, w*sur->h*sizeof(uint16_t));
	SDL_FreeSurface(dest);
	return ret;
}

Eina_Bool	menu_Texture_save_to(const char *p_name, const char *p_filename) {
	return EINA_FALSE;
}

