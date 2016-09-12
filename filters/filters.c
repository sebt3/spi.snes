#include "filters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(PANDORA)
#include "neon_normal1x.h"
#include "neon_eagle2x.h"
#include "neon_scale2x.h"
#include "neon_scale3x.h"
#define filter_normal1x neon_normal1x_16_16
#define filter_eagle2x  neon_eagle2x_16_16
#define filter_scale2x  neon_scale2x_16_16
#define filter_scale3x  neon_scale3x_16_16
#else
#define filter_eagle2x  filter_SuperEagle
void filter_normal1x(const uint16_t *src, uint16_t *dst, unsigned int width, unsigned int srcstride, unsigned int dststride, unsigned int height);
void filter_scale2x(const uint16_t *src, uint16_t *dst, unsigned int width, unsigned int srcstride, unsigned int dststride, unsigned int height);
#endif

void filter_SuperEagle (const uint16_t *p_srcPtr, uint16_t *p_dstPtr, unsigned int width, unsigned int srcRowBytes, unsigned int dstRowBytes, unsigned int height);
void filter_2xSaI (const uint16_t *p_srcPtr, uint16_t *p_dstPtr, unsigned int width, unsigned int srcRowBytes, unsigned int dstRowBytes, unsigned int height);
void filter_Super2xSaI (const uint16_t *p_srcPtr, uint16_t *p_dstPtr, unsigned int width, unsigned int srcRowBytes, unsigned int dstRowBytes, unsigned int height);

void menu_filter_hq2x_init (void);
void menu_filter_hq2x_deinit (void);
void filter_hq2x (const uint16_t *p_srcPtr, uint16_t *p_dstPtr, unsigned int width, unsigned int srcRowBytes, unsigned int dstRowBytes, unsigned int height);
void filter_hq3x (const uint16_t *p_srcPtr, uint16_t *p_dstPtr, unsigned int width, unsigned int srcRowBytes, unsigned int dstRowBytes, unsigned int height);
void filter_hq4x (const uint16_t *p_srcPtr, uint16_t *p_dstPtr, unsigned int width, unsigned int srcRowBytes, unsigned int dstRowBytes, unsigned int height);


/***********************************************************************************
 *				External view
 ***********************************************************************************/

static menu_Filter *filters[FILTER_MAX_ID];
inline static menu_Filter* filter_new(const char *p_name, uint16_t p_scale, menu_filter_proc p_proc) {
	menu_Filter* ret	= calloc(1, sizeof(menu_Filter));
	ret->name		= p_name;
	ret->scale		= p_scale;
	ret->proc		= p_proc;
	return ret;
}
void		menu_filter_init(void) {
	filters[0] = filter_new("normal",  1, filter_normal1x);
	filters[1] = filter_new("scale2x", 2, filter_scale2x);
	filters[2] = filter_new("Eagle", 2, filter_eagle2x);
	filters[3] = filter_new("2xSaI", 2, filter_2xSaI);
	filters[4] = filter_new("Super2xSaI", 2, filter_Super2xSaI);
	filters[5] = filter_new("SuperEagle", 2, filter_SuperEagle);
	filters[6] = filter_new("hq2x", 2, filter_hq2x);
	filters[7] = filter_new("hq3x", 3, filter_hq3x);
#if defined(PANDORA)
	filters[8] = filter_new("scale3x", 3, filter_scale3x);
#endif
	menu_filter_hq2x_init();
}

void		menu_filter_deinit(void) {
	int i;
	for (i=0;i<FILTER_MAX_ID;i++)
		free(filters[i]);
	menu_filter_hq2x_deinit ();
}

menu_Filter*	menu_filter_get(uint16_t idx) {
	if (idx>=FILTER_MAX_ID) return NULL;
	return filters[idx];
}

/***********************************************************************************
 *				normal1x
 ***********************************************************************************/

#ifndef PANDORA
void		filter_normal1x(const uint16_t *src, uint16_t *dst, unsigned int width, unsigned int srcstride, unsigned int dststride, unsigned int height) {
	int j;
	uint16_t* tp = dst;
	uint16_t* sp = (uint16_t *)src;
	for (j = 0; j < height; ++j) {
		memcpy(tp, sp, width<<1);
		tp += dststride>>1;
		sp += srcstride>>1;
	}
}
#endif

/***********************************************************************************
 *				scale2x
 ***********************************************************************************/

#ifndef PANDORA
#define B sp[i - b]
#define D sp[i - (i>0?1:0)]
#define F sp[i + (i<width?1:0)]
#define H sp[i + h0]
#define E  sp[i]
#define E0 tp[i*2] 
#define E1 tp[i*2 + 1]
#define E2 tp[i*2 + tpitch]
#define E3 tp[i*2 + 1 + tpitch]

void		filter_scale2x(const uint16_t *src, uint16_t *dst, unsigned int width, unsigned int srcstride, unsigned int dststride, unsigned int height) {
	register int i, j;
	int b, h0;

	int tpitch = dststride>>1;
	int spitch = srcstride>>1;
	uint16_t* tp = dst;
	uint16_t* sp = (uint16_t *)src;
 
	for (j = 0; j < height; ++j) {
		b = j>0?spitch:0;
		h0 = j<height?spitch:0;
		for (i = 0; i < width; ++i) {
			if (B != H && D != F) {
				E0 = D == B ? D : E;
				E1 = B == F ? F : E;
				E2 = D == H ? D : E;
				E3 = H == F ? F : E;
			} else {
				E0 = E;
				E1 = E;
				E2 = E;
				E3 = E;
			}
		}
		tp += 2*tpitch;
		sp += spitch;
	}
}

#undef B
#undef D
#undef F
#undef H
#undef E
#undef E0
#undef E1
#undef E2
#undef E3
#endif
