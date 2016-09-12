#ifndef __menu_FILTERS_H__
#define __menu_FILTERS_H__

#include <inttypes.h>

#ifndef __menu_TEXTURE_H__
typedef struct menu_Filter_ menu_Filter;
#endif

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(PANDORA)
#define FILTER_MAX_ID 9
#else
#define FILTER_MAX_ID 8
#endif
typedef void (*menu_filter_proc)(const uint16_t *src, uint16_t *dst, unsigned int width, unsigned int srcstride, unsigned int dststride, unsigned int height);
struct menu_Filter_{
	const char*		name;
	uint16_t		scale;
	menu_filter_proc	proc;
};

extern void		menu_filter_init(void);
extern void		menu_filter_deinit(void);
extern menu_Filter*	menu_filter_get(uint16_t idx);

#if defined(__cplusplus)
}
#endif
#endif