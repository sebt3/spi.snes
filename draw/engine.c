#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_syswm.h>
#include <Eina.h>
#include <Eet.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "engine.h"
#include "theme.h"
#include "filters.h"
#include <X11/Xlib.h>

#ifdef HAVE_GLES
# include <bc_cat.h>
# include <sys/ioctl.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <GLES2/gl2.h>
# include <EGL/egl.h>
# include "eglport.h"
# define glClearDepth glClearDepthf
# define glOrtho glOrthof
#else
# include <GL/gl.h>
# include <GL/glx.h>
#endif

float			menu_Projection[16];

static menu_Scene*	gameScene;
static menu_Scene*	uiScene;
static menu_Scene*	uiScene2;
static Eina_List*	allScenes;
static Eina_Array*	uiStack;
static int		timer;
static int		animeType;

static SDL_Surface*	sdl_screen;
Viewport		menu_viewport;
static menu_Font*	dejavu;
static menu_RGroup*	fps;
static Eina_Bool	showFPS;
Eina_Bool menu_havePBO;
Eina_Bool menu_quit;
SDL_mutex* rendering_mutex;

#ifndef HAVE_GLES
getProcAddressProc       glGetProcAddress;
glGenBuffersProc         glGenBuffers;
glBindBufferProc         glBindBuffer;
glBufferDataProc         glBufferData;
glBufferSubDataProc      glBufferSubData;
glMapBufferProc          glMapBuffer;
glUnmapBufferProc        glUnmapBuffer;
glDeleteBuffersProc      glDeleteBuffers;
glCreateProgramProc      glCreateProgram;
glCreateShaderProc       glCreateShader;
glCompileShaderProc      glCompileShader;
glDeleteShaderProc       glDeleteShader;
glDeleteProgramProc      glDeleteProgram;
glAttachShaderProc       glAttachShader;
glDetachShaderProc       glDetachShader;
glLinkProgramProc        glLinkProgram;
glUseProgramProc         glUseProgram;
glShaderSourceProc       glShaderSource;
glGetUniformLocationProc glGetUniformLocation;
glUniform2fvProc         glUniform2fv;
glGetShaderivProc	 glGetShaderiv;
glGetShaderInfoLogProc	 glGetShaderInfoLog;
glUniformMatrix4fvProc	 glUniformMatrix4fv;
glGetAttribLocationProc	 glGetAttribLocation;
glUniform4fProc		 glUniform4f;
glUniform4fvProc	 glUniform4fv;
glEnableVertexAttribArrayProc glEnableVertexAttribArray;
glVertexAttribPointerProc glVertexAttribPointer;
//glActiveTextureProc	 glActiveTexture;

static const char *glGenBuffersNames[] = { "glGenBuffers",
                                           "glGenBuffersARB",
                                           "glGenBuffersEXT",
                                           NULL };
static const char *glDeleteBuffersNames[] = { "glDeleteBuffers",
                                              "glDeleteBuffersARB",
                                              "glDeleteBuffersEXT",
                                              NULL };
static const char *glBindBufferNames[] = { "glBindBuffer",
                                           "glBindBufferARB",
                                           "glBindBufferEXT",
                                           NULL };
static const char *glBufferDataNames[] = { "glBufferData",
                                           "glBufferDataARB",
                                           "glBufferDataEXT",
                                           NULL };
static const char *glBufferSubDataNames[] = { "glBufferSubData",
                                              "glBufferSubDataARB",
                                              "glBufferSubDataEXT",
                                              NULL };
static const char *glMapBufferNames[] = { "glMapBuffer",
                                          "glMapBufferARB",
                                          "glMapBufferEXT",
                                          NULL };
static const char *glUnmapBufferNames[] = { "glUnmapBuffer",
                                            "glUnmapBufferARB",
                                            "glUnmapBufferEXT",
                                            NULL };
#else
PFNGLTEXBINDSTREAMIMGPROC glTexBindStreamIMG = NULL;
PFNGLGETTEXSTREAMDEVICEATTRIBUTEIVIMGPROC glGetTexAttrIMG = NULL;
PFNGLGETTEXSTREAMDEVICENAMEIMGPROC glGetTexDeviceIMG = NULL;
#endif
static menu_RGroup  *loadingRg;
static menu_RGroup  *loadingTextRg;



void		menu_Screen_Init(void);
void		menu_Screen_Shutdown(void);


#define menu_TICK2MS 20
#define ANIMLEN 30
inline Uint32	menu_GetTicks(void) {
	return SDL_GetTicks()/menu_TICK2MS;
}



void	menu_init(void) {
	menu_filter_init();
	menu_Theme_Descriptor_Init();
	menu_Screen_Init();
	menu_Shader_init();

	uiStack		= eina_array_new(10);
	timer		= 0;
	allScenes	= NULL;
	gameScene	= NULL;
	uiScene2	= NULL;
	uiScene		= NULL;
	loadingRg	= NULL;
	loadingTextRg	= NULL;
	g = NULL;
}

void	menu_Scenes_free();

void	menu_finish(void) {
	menu_Scenes_free();
	if (g!=NULL)menu_Texture_Theme_shutdown(g);
	menu_Screen_Shutdown();
	menu_Theme_Descriptor_Shutdown();
	menu_filter_deinit();
	exit(0);
}

Eina_Bool	menu_Event_Handle(SDL_Event *e) {
	switch(e->type) {
	case SDL_QUIT:
		menu_quit = EINA_TRUE;
	break;
	case SDL_KEYUP:
		switch(e->key.keysym.sym) {
		case SDLK_q:
		case SDLK_ESCAPE:
			menu_quit = EINA_TRUE;
		break;
		default:
		break;
		}
	break;
	default:
	break;
	}
	return menu_quit;
}

//int dbgt=-1;
void	menu_Update_Ticks(void) {
	// pool Input Event
	SDL_Event event;
	while ( SDL_PollEvent(&event) && !menu_quit) {
		if (timer>0) {
//			if (dbgt!=0) {dbgt=0;printf("Now using timed event loop\n");}
			menu_Event_Handle(&event);
		} else if (uiScene !=NULL && uiScene->onEvent != NULL) {
//			if (dbgt!=1) {dbgt=1;printf("Now using UI event loop\n");}
			uiScene->onEvent(uiScene, &event);
		} else if (gameScene != NULL && gameScene->onEvent != NULL) {
//			if (dbgt!=2) {dbgt=2;printf("Now using game event loop\n");}
			gameScene->onEvent(gameScene, &event);
		} else {
//			if (dbgt!=3) {dbgt=3;printf("Now default timed event loop\n");}
			menu_Event_Handle(&event);
		}
	}
	if (gameScene!=NULL)
 		menu_Scene_runTick(gameScene);
	if (timer>0) {
		if(timer==1) {
			uiScene  = uiScene2;
			uiScene2 = NULL;
		}
		timer--;
		return;
	}
	if (uiScene!=NULL)
 		menu_Scene_runTick(uiScene);
	if (uiScene!=NULL && uiScene->joystickR != NULL)
		uiScene->joystickR();
	
	return;
}

void	menu_main_Loading(const char *p_text, float p_pct) {
	menu_color grpc = {{1.0f, 1.0f, 1.0f, 1.0f}};
	menu_vector text_pos;
	menu_vector p, q;
	menu_Segment *seg;
	menu_vector_set(text_pos, 0, 330.0f, 200.0f, 0.05f);
	if (loadingRg == NULL) {
		loadingRg = menu_RGroupSprite_new(0, EINA_FALSE, EINA_FALSE);
		menu_RGroup_uniq_col_set(loadingRg, grpc);
	}

	if (loadingTextRg == NULL)
		loadingTextRg = menu_RGroupSprite_new(0, EINA_FALSE, EINA_TRUE);
	if (loadingTextRg != NULL && loadingTextRg->f == NULL && menu_Theme_find_Font(g, "fps"))
		menu_RGroup_set_font(loadingTextRg, menu_Theme_find_Font(g, "fps"));

	menu_Screen_setInterface();
	if (loadingRg) {
		if (eina_list_count(loadingRg->segs) < 2) {
			menu_vector_set(p, 0, 205.0f, 165.0f, 0.05f);
			menu_vector_set(q, 0, 595.0f, 185.0f, 0.05f);
			menu_RGroup_Rect_add(loadingRg, p, q);
			menu_vector_set(p, 0, 200.0f, 160.0f, 0.05f);
			menu_vector_set(q, 0, 600.0f, 162.0f, 0.05f);
			menu_RGroup_Rect_add(loadingRg, p, q);
			menu_vector_set(q, 0, 202.0f, 190.0f, 0.05f);
			menu_RGroup_Rect_add(loadingRg, p, q);
			menu_vector_set(p, 0, 200.0f, 188.0f, 0.05f);
			menu_vector_set(q, 0, 600.0f, 190.0f, 0.05f);
			menu_RGroup_Rect_add(loadingRg, p, q);
			menu_vector_set(p, 0, 598.0f, 160.0f, 0.05f);
			menu_RGroup_Rect_add(loadingRg, p, q);
		}
		seg	= eina_list_data_get(loadingRg->segs);
		seg->dots[6]	= seg->dots[3]	= 205.0f + p_pct/100*390;
		menu_RGroup_draw(loadingRg);
	}
	if (loadingTextRg && loadingTextRg->f != NULL) {
		menu_RGroup_empty(loadingTextRg);
		menu_RGroup_Text_add_at(loadingTextRg, p_text, text_pos);
		menu_RGroup_draw(loadingTextRg);
	}

	menu_Screen_Flip();
	if (p_pct>=100.0) {
		// free the rg if existing
		if (loadingRg != NULL) {
			menu_RGroup_free(loadingRg);
			loadingRg	= NULL;
		}
		if (loadingTextRg != NULL) {
			menu_RGroup_free(loadingTextRg);
			loadingTextRg	= NULL;
		}
	}
}
static Uint32 next_game_tick;
static SDL_Thread *thread = NULL;
static Eina_Bool useThreads = EINA_FALSE;
int	_menu_main_renderThread(void *ptr) {
#ifdef USE_RENDER_THREAD

#if defined(PANDORA)
	EGL_MakeCurrent();
#elif defined(__linux__)
    SDL_SysWMinfo wm_info;
    SDL_VERSION( &wm_info.version );
    if ( SDL_GetWMInfo( &wm_info ) && useThreads ) {
        Display *display = wm_info.info.x11.gfxdisplay;
        Window   window  = wm_info.info.x11.window;
	glXMakeCurrent( display, window, ptr );
        XSync( display, EINA_FALSE );
    }
#endif
	do {
#endif

	if (gameScene!=NULL) {
		menu_Screen_setViewport();
		menu_Scene_draw(gameScene);
	}
#ifndef USE_SHADER
	if (timer>0) {
		menu_Screen_setInterface();
		switch(animeType) {
		case 0:
			glTranslate(-(800/ANIMLEN)*(ANIMLEN-timer),0,0);
			break;
		case 1:
			glTranslate(0, -(480/ANIMLEN)*(ANIMLEN-timer),0);
			break;
		case 2:
			glTranslate((800/ANIMLEN)*(ANIMLEN-timer),0,0);
			break;
		case 3:
			glTranslate(0, (480/ANIMLEN)*(ANIMLEN-timer),0);
			break;
		}
		menu_Scene_draw(uiScene);
		menu_Screen_setInterface();
		switch(animeType) {
		case 0:
			glTranslate((800/ANIMLEN)*timer,0,0);
			break;
		case 1:
			glTranslate(0, (480/ANIMLEN)*timer,0);
			break;
		case 2:
			glTranslate(-(800/ANIMLEN)*timer,0,0);
			break;
		case 3:
			glTranslate(0, -(480/ANIMLEN)*timer,0);
			break;
		}
		menu_Scene_draw(uiScene2);
	} else
#endif
	if (uiScene!=NULL) {
		menu_Screen_setInterface();
		menu_Scene_draw(uiScene);
	}

	menu_Screen_Flip();

#ifdef USE_RENDER_THREAD
	} while(!menu_quit && thread != NULL);
	
# if defined(PANDORA)
	EGL_MakeUnCurrent();
# endif
#else
	SDL_Delay( (next_game_tick-menu_GetTicks())*menu_TICK2MS-5);
#endif
	return 0;
}

Uint32 fpsmc=0;
void	EMU_pause(void);
void	menu_main_Loop(void) {
	next_game_tick = menu_GetTicks();
	Eina_Bool doFrameSkip;
	menu_quit = EINA_FALSE;
	int i =0;
	menu_main_Loading("Done", 100.0);

#ifdef USE_RENDER_THREAD
	useThreads = EINA_TRUE;
	// create the mutex
	rendering_mutex = SDL_CreateMutex();

# if defined(PANDORA)
	EGL_MakeUnCurrent();
# endif
	
	// start the redenring thread
	thread = SDL_CreateThread(_menu_main_renderThread, 
# if defined(__linux__) && !defined(PANDORA)
					 glXGetCurrentContext()
# else
					 NULL
# endif
	);//*/
	if (thread==NULL) {
		printf("No rendering thread created\n");
		useThreads = EINA_FALSE;
	}
#endif

	while( menu_quit == EINA_FALSE ) {
		// Frame skipping (up to 5)
		doFrameSkip = EINA_TRUE;
		do {
			i++;
			menu_Update_Ticks();
			next_game_tick ++;
			if (uiScene!=NULL && uiScene->isEmu)
				doFrameSkip = EINA_FALSE;
		} while (doFrameSkip && menu_GetTicks() > next_game_tick && !menu_quit && i<5);
		if (thread==NULL)
			_menu_main_renderThread(NULL);

		// frame limiter
		if (!menu_quit && doFrameSkip && next_game_tick>menu_GetTicks()) {
			int dlay = (next_game_tick-menu_GetTicks())*menu_TICK2MS-5;
			if (dlay >dlay%1000) next_game_tick = menu_GetTicks();
 			else SDL_Delay(dlay);
		}
	}
	EMU_pause();
#if USE_RENDER_THREAD
	menu_quit = EINA_TRUE;
	SDL_WaitThread(thread, NULL);
	//SDL_KillThread(thread);
	SDL_DestroyMutex(rendering_mutex);
# if defined(PANDORA)
	EGL_MakeCurrent();
# endif
#endif
}

menu_Scene*	menu_findScene(const char *name) {
	Eina_List*	l;
	menu_Scene*	t;

	if (!allScenes) return NULL;
	EINA_LIST_FOREACH(allScenes, l, t) {
		if (!strcmp(t->name, name))
			return t;
	}
	printf("Scene not found : %s\n", name);
	return NULL;
}

void		menu_addScene(menu_Scene* sc) {
	if(sc!=NULL) allScenes = eina_list_append(allScenes, sc);
}

void		menu_Scenes_free() {
	menu_Scene*	t;
	EINA_LIST_FREE(allScenes, t) {
		menu_Scene_free(t);
	}

}

void		menu_setScene(menu_Scene* game, menu_Scene* ui) {
	if (gameScene && gameScene->dispose)
		gameScene->dispose(gameScene);
	if (uiScene && uiScene->dispose)
		uiScene->dispose(uiScene);
	gameScene = game;
	uiScene = ui;
	if (gameScene && gameScene->prepare)
		gameScene->prepare(gameScene);
	if (uiScene && uiScene->prepare)
		uiScene->prepare(uiScene);
}

void		menu_setUISceneAnime(menu_Scene* ui, int type) {
	timer		= ANIMLEN;
	animeType	= type;
	uiScene2	= ui;
}

void		menu_setUIScene(menu_Scene* ui) {
	if (uiScene)
		eina_array_push(uiStack, (void*)uiScene);
	if (uiScene && uiScene->dispose)
		uiScene->dispose(uiScene);
	uiScene		= ui;
	if (uiScene && uiScene->prepare)
		uiScene->prepare(uiScene);
}

void		menu_setUIScenePrevious() {
	menu_Scene* ui	= (menu_Scene*) eina_array_pop(uiStack);
	if (uiScene && uiScene->dispose)
		uiScene->dispose(uiScene);
	if (ui == NULL)
		menu_quit = EINA_TRUE;
	uiScene		= ui;
	if (uiScene && uiScene->prepare)
		uiScene->prepare(uiScene);
}

void		menu_UISceneEmptyStack() {
	eina_array_clean(uiStack);
}

void		menu_setGameScene(menu_Scene* game) {
	if (gameScene && gameScene->dispose)
		gameScene->dispose(gameScene);
	gameScene = game;
	if (gameScene && gameScene->prepare)
		gameScene->prepare(gameScene);
}



#define __glPi 3.14159265358979323846

#ifndef USE_SHADER
static void _menu_MakeIdentityf(GLfloat m[16]) {
	memset(m,0,16);
	m[0] = m[5] = m[10] = m[15] = 1;
}

static void _menu_normalize(float v[3]) {
    float r;

    r = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    if (r == 0.0) return;

    v[0] /= r;
    v[1] /= r;
    v[2] /= r;
}

static void _menu_cross(float v1[3], float v2[3], float result[3]) {
	result[0] = v1[1]*v2[2] - v1[2]*v2[1];
	result[1] = v1[2]*v2[0] - v1[0]*v2[2];
	result[2] = v1[0]*v2[1] - v1[1]*v2[0];
}
#endif


void menu_Perspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
#ifndef USE_SHADER
    GLfloat m[4][4];
    float sine, cotangent, deltaZ;
    float radians = fovy / 2 * __glPi / 180;

    deltaZ = zFar - zNear;
    sine = sinf(radians);
    if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
	return;
    }
    cotangent = cosf(radians) / sine;

    _menu_MakeIdentityf(&m[0][0]);
    m[0][0] = cotangent / aspect;
    m[1][1] = cotangent;
    m[2][2] = -(zFar + zNear) / deltaZ;
    m[2][3] = -1;
    m[3][2] = -2 * zNear * zFar / deltaZ;
    m[3][3] = 0;
    GL_CHECK(glMultMatrixf(&m[0][0]));
#endif
}

void menu_LookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
		GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy,
		GLfloat upz) {
#ifndef USE_SHADER
	float forward[3], side[3], up[3];
	GLfloat m[4][4];

	forward[0] = centerx - eyex;
	forward[1] = centery - eyey;
	forward[2] = centerz - eyez;

	up[0] = upx;
	up[1] = upy;
	up[2] = upz;

	_menu_normalize(forward);

	/* Side = forward x up */
	_menu_cross(forward, up, side);
	_menu_normalize(side);

	/* Recompute up as: up = side x forward */
	_menu_cross(side, forward, up);

	_menu_MakeIdentityf(&m[0][0]);
	m[0][0] = side[0];
	m[1][0] = side[1];
	m[2][0] = side[2];

	m[0][1] = up[0];
	m[1][1] = up[1];
	m[2][1] = up[2];

	m[0][2] = -forward[0];
	m[1][2] = -forward[1];
	m[2][2] = -forward[2];

	glMultMatrixf(&m[0][0]);
	GL_CHECK(glTranslatef(-eyex, -eyey, -eyez));
#endif
}

void		menu_Screen_Ortho(float left, float right, float bottom, float top, float near, float far) {
#if !defined(USE_SHADER)
	glOrtho(left,right,bottom,top,near,far);
#else
	menu_Projection[1]  = menu_Projection[2] = menu_Projection[3]  = 0.0;
	menu_Projection[4]  = menu_Projection[6] = menu_Projection[7]  = 0.0;
	menu_Projection[8]  = menu_Projection[9] = menu_Projection[11] = 0.0;
	menu_Projection[0]  = 2 / (right - left);
	menu_Projection[5]  = 2 / (top - bottom);
	menu_Projection[10] = -2 / (far - near);
	menu_Projection[12] = -(right + left) / (right - left);
	menu_Projection[13] = -(top + bottom) / (top - bottom);
	menu_Projection[14] = -(far + near) / (far - near);
	menu_Projection[15] = 1.0;
#endif
}


#ifndef HAVE_GLES
gl_proc		get_aliased_extension (const char **name) {
	int i;
	gl_proc ext_proc = NULL;

	for (i = 0; name[i] && !ext_proc; i++)
		ext_proc = glGetProcAddress ((GLubyte *) name[i]);

	return ext_proc;
}
#else
int bc_cat[10];
int tex_free[10];
const GLubyte * bcdev[10];
int bcdev_w, bcdev_h, bcdev_n;
int bcdev_fmt;
unsigned long buf_paddr[10];    // physical address
char *buf_vaddr[10];            // virtual adress
char *buf_vaddr2[10];            // virtual adress 2nd buffer
int		open_bccat(int i) {
	if (bc_cat[i]>-1)
		return bc_cat[i];
	char buff[]="/dev/bccat0";
	buff[strlen(buff)-1]='0'+i;
	bc_cat[i] = open(buff, O_RDWR|O_NDELAY);
	return bc_cat[i];
}
void		close_bccat(int i) {
	if (bc_cat[i]==-1)
		return;
	close(bc_cat[i]);
	bc_cat[i]=-1;
	return;
}
int		menu_alloc_buff(int buff, int width, int height) {
	if ((buff<0) || (buff>9))
		return 0;
	if (!tex_free[buff])
		return 0;
	if (open_bccat(buff)<0)
		return 0;
	BCIO_package ioctl_var;
	bc_buf_params_t buf_param;
	buf_param.count = 1;	// only 1 buffer?
	buf_param.width = width;
	buf_param.height = height;
	buf_param.fourcc = BC_PIX_FMT_RGB565;	// only RGB565 here (other choices are only some YUV formats)
	buf_param.type = BC_MEMORY_MMAP;
	if (ioctl(bc_cat[buff], BCIOREQ_BUFFERS, &buf_param) != 0) {
		printf("LIBGL: BCIOREQ_BUFFERS failed\n");
		return 0;
	}
	if (ioctl(bc_cat[buff], BCIOGET_BUFFERCOUNT, &ioctl_var) != 0) {
		printf("LIBGL: BCIOREQ_BUFFERCOUNT failed\n");
		return 0;
	}
	if (ioctl_var.output == 0) {
		printf("LIBGL: Streaming, no texture buffer available\n");
		return 0;
	}
	const unsigned char *bcdev = glGetTexDeviceIMG(buff);
	if (!bcdev) {
		printf("LIBGL: problem with getting the GL_IMG_texture_stream device\n");
		return 0;
	} else {
		bcdev_w = width;
		bcdev_h = height;
		bcdev_n = 1;
		glGetTexAttrIMG(buff, GL_TEXTURE_STREAM_DEVICE_NUM_BUFFERS_IMG, &bcdev_n);
		glGetTexAttrIMG(buff, GL_TEXTURE_STREAM_DEVICE_WIDTH_IMG, &bcdev_w);
		glGetTexAttrIMG(buff, GL_TEXTURE_STREAM_DEVICE_HEIGHT_IMG, &bcdev_h);
		glGetTexAttrIMG(buff, GL_TEXTURE_STREAM_DEVICE_FORMAT_IMG, &bcdev_fmt);
		printf("LIBGL: Streaming device = %s num: %d, width: %d, height: %d, format: 0x%x\n",
			bcdev, bcdev_n, bcdev_w, bcdev_h, bcdev_fmt);
		if (bcdev_w!=width) {
			printf("LIBGL: Streaming not activate, buffer width != asked width\n");
			return 0;
		}
	}

	ioctl_var.input = 0;
	if (ioctl(bc_cat[buff], BCIOGET_BUFFERPHYADDR, &ioctl_var) != 0) {
		printf("LIBGL: BCIOGET_BUFFERADDR failed\n");
		return 0;
	} else {
		buf_paddr[buff] = ioctl_var.output;
		buf_vaddr[buff] = (char *)mmap(NULL, width*height*2,
						  PROT_READ | PROT_WRITE, MAP_SHARED,
						  bc_cat[buff], buf_paddr[buff]);

		if (buf_vaddr[buff] == MAP_FAILED) {
			printf("LIBGL: mmap failed\n");
			return 0;
		}
	}

#if 0
	ioctl_var.input = 1;
	if (ioctl(bc_cat[buff], BCIOGET_BUFFERPHYADDR, &ioctl_var) != 0) {
		printf("LIBGL: BCIOGET_BUFFERADDR failed\n");
		return 0;
	} else {
		buf_paddr[buff] = ioctl_var.output;
		buf_vaddr2[buff] = (char *)mmap(NULL, width*height*2,
						  PROT_READ | PROT_WRITE, MAP_SHARED,
						  bc_cat[buff], buf_paddr[buff]);

		if (buf_vaddr2[buff] == MAP_FAILED) {
			printf("LIBGL: mmap failed\n");
			return 0;
		}
	}
#endif	
	// All done!
	tex_free[buff] = 0;
	return 1;
}

int		menu_free_buff(int buff) {
	if ((buff<0) || (buff>9))
		return 0;
	close_bccat(buff);
	tex_free[buff] = 1;
	return 1;
}
#endif

Eina_Bool	menu_Screen_loadExtentions(void) {
	menu_havePBO = EINA_FALSE;
#ifndef HAVE_GLES
	const char *extensions = (const char *) glGetString (GL_EXTENSIONS);
	//printf("GL extensions : %s\n",extensions);
	void *dl_handle = NULL;

	dl_handle = dlopen (NULL, RTLD_LAZY);
	if (dl_handle) {
		dlerror ();

		glGetProcAddress = dlsym (dl_handle, "glXGetProcAddress");

		if (dlerror () != NULL) {
			glGetProcAddress  = dlsym (dl_handle, "glXGetProcAddressARB");

			if (dlerror () != NULL)
				return EINA_FALSE;
		}

		dlclose (dl_handle);
	} else {
		return EINA_FALSE;
	}

	if (!extensions)
		return EINA_FALSE;

	if (strstr (extensions, "fragment_program")) {
		glCreateProgram		= (glCreateProgramProc) glGetProcAddress ((GLubyte *) "glCreateProgram");
		glCreateShader		= (glCreateShaderProc) glGetProcAddress ((GLubyte *) "glCreateShader");
		glCompileShader		= (glCompileShaderProc) glGetProcAddress ((GLubyte *) "glCompileShader");
		glDeleteShader		= (glDeleteShaderProc) glGetProcAddress ((GLubyte *) "glDeleteShader");
		glDeleteProgram		= (glDeleteProgramProc) glGetProcAddress ((GLubyte *) "glDeleteProgram");
		glAttachShader		= (glAttachShaderProc) glGetProcAddress ((GLubyte *) "glAttachShader");
		glDetachShader		= (glDetachShaderProc) glGetProcAddress ((GLubyte *) "glDetachShader");
		glLinkProgram		= (glLinkProgramProc) glGetProcAddress ((GLubyte *) "glLinkProgram");
		glUseProgram		= (glUseProgramProc) glGetProcAddress ((GLubyte *) "glUseProgram");
		glShaderSource		= (glShaderSourceProc) glGetProcAddress ((GLubyte *) "glShaderSource");
		glGetUniformLocation	= (glGetUniformLocationProc) glGetProcAddress ((GLubyte *) "glGetUniformLocation");
		glUniform2fv		= (glUniform2fvProc) glGetProcAddress ((GLubyte *) "glUniform2fv");
		glGetShaderiv		= (glGetShaderivProc) glGetProcAddress ((GLubyte *) "glGetShaderiv");
		glGetShaderInfoLog	= (glGetShaderInfoLogProc) glGetProcAddress ((GLubyte *) "glGetShaderInfoLog");
		glUniformMatrix4fv	= (glUniformMatrix4fvProc) glGetProcAddress ((GLubyte *) "glUniformMatrix4fv");
		glGetAttribLocation	= (glGetAttribLocationProc) glGetProcAddress ((GLubyte *) "glGetAttribLocation");
		glUniform4f		= (glUniform4fProc) glGetProcAddress ((GLubyte *) "glUniform4f");
		glUniform4fv		= (glUniform4fvProc) glGetProcAddress ((GLubyte *) "glUniform4fv");
		glVertexAttribPointer	= (glVertexAttribPointerProc) glGetProcAddress ((GLubyte *) "glVertexAttribPointer");
		glEnableVertexAttribArray= (glEnableVertexAttribArrayProc) glGetProcAddress ((GLubyte *) "glEnableVertexAttribArray");

		if (!glCreateProgram		||
		    !glCreateShader		||
		    !glCompileShader		||
		    !glDeleteShader		||
		    !glDeleteProgram		||
		    !glAttachShader		||
		    !glDetachShader		||
		    !glLinkProgram		||
		    !glUseProgram		||
		    !glShaderSource		||
		    !glGetUniformLocation	||
		    !glUniform2fv		||
		    !glGetShaderiv		||
		    !glGetShaderInfoLog		||
		    !glUniformMatrix4fv		||
		    !glGetAttribLocation	||
		    !glUniform4f		||
		    !glUniform4fv		||
		    !glVertexAttribPointer	||
		    !glEnableVertexAttribArray)
			return EINA_FALSE;
		

	} else printf("no Shader found oups\n");

	if (strstr (extensions, "pixel_buffer_object")) {
		glGenBuffers	= (glGenBuffersProc)	get_aliased_extension (glGenBuffersNames);
		glDeleteBuffers	= (glDeleteBuffersProc)	get_aliased_extension (glDeleteBuffersNames);
		glBindBuffer	= (glBindBufferProc)	get_aliased_extension (glBindBufferNames);
		glBufferData	= (glBufferDataProc)	get_aliased_extension (glBufferDataNames);
		glBufferSubData = (glBufferSubDataProc)	get_aliased_extension (glBufferSubDataNames);
		glMapBuffer	= (glMapBufferProc)	get_aliased_extension (glMapBufferNames);
		glUnmapBuffer	= (glUnmapBufferProc)	get_aliased_extension (glUnmapBufferNames);

		if (!glGenBuffers    ||
		    !glBindBuffer    ||
		    !glBufferData    ||
		    !glBufferSubData ||
		    !glMapBuffer     ||
		    !glUnmapBuffer   ||
		    !glDeleteBuffers) {
			return EINA_TRUE;
		}
		menu_havePBO = EINA_TRUE;
	} else printf("no buffer found found oups\n");
#else
	int i;
	glTexBindStreamIMG =(PFNGLTEXBINDSTREAMIMGPROC)eglGetProcAddress("glTexBindStreamIMG");
	glGetTexAttrIMG = (PFNGLGETTEXSTREAMDEVICEATTRIBUTEIVIMGPROC)eglGetProcAddress("glGetTexStreamDeviceAttributeivIMG");
	glGetTexDeviceIMG = (PFNGLGETTEXSTREAMDEVICENAMEIMGPROC)eglGetProcAddress("glGetTexStreamDeviceNameIMG");

	if (!glTexBindStreamIMG || !glGetTexAttrIMG || !glGetTexDeviceIMG) {
		return EINA_TRUE;
	}
	for (i=0; i<10; i++) {
		bc_cat[i] = -1;
		tex_free[i] = 1;
	}
	menu_havePBO = EINA_TRUE;
#endif
	return EINA_TRUE;
}

void		menu_Screen_Init(void) {
	menu_vector p;

	menu_viewport.scrn[0] = menu_viewport.orig[0] = 800.0;
	menu_viewport.scrn[1] = menu_viewport.orig[1] = 480.0;
	menu_viewport.is3D = EINA_FALSE;
	menu_viewport.scaleFactor = 1;
	menu_vector_set(menu_viewport.eye, 0, 200.0,   0.0, 240.0);
	menu_vector_set(menu_viewport.at,  0, 200.0, 140.0,   0.0);
	menu_vector_set(menu_viewport.up,  0,   0.0,   1.0,   0.0);
	menu_viewport.scrn[2] = menu_viewport.orig[2] = 0.0;
	showFPS		     = EINA_TRUE;
	p[0] = p[1] = p[2]   = 0.0;


	// Init SDL
	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK );
	IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG);
	// Init the GLScreen
#if defined(HAVE_GLES)
	sdl_screen = SDL_SetVideoMode(menu_viewport.scrn[0], menu_viewport.scrn[1], 0, SDL_SWSURFACE | SDL_FULLSCREEN );
	if (EGL_Open( 800, 480 )) {
		fprintf(stderr,"ERROR: Cannot open EGL interface\n");
		SDL_Quit();
		exit(1);
	}

#else
	sdl_screen = SDL_SetVideoMode(menu_viewport.scrn[0], menu_viewport.scrn[1], 0, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL );
#endif
	if (!sdl_screen) {
		fprintf(stderr,"ERROR: Cannot allocate screen: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	if(! menu_Screen_loadExtentions()) {
		fprintf(stderr,"ERROR: Cannot load extentions\n");
		SDL_Quit();
		exit(1);
	}


#if !defined(HAVE_GLES)
	SDL_WM_SetCaption( "Test", NULL );

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,		4);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,		4);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,		4);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,		4);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,		16);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,		16);

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL,	1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,	1);
#endif

	// set the view port
	glDisable(GL_DEPTH_TEST);
#ifndef USE_SHADER
	glDisable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
#endif
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	//Set the viewport
	glViewport(0, 0, menu_viewport.scrn[0], menu_viewport.scrn[1]);
	menu_Screen_defaultArea_set();
#if !defined(HAVE_GLES)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif
#ifndef USE_SHADER
	glEnableClientState(GL_VERTEX_ARRAY);
#endif
	gameScene = NULL;
	uiScene = NULL;
	dejavu = NULL;
	fps = menu_RGroupSprite_new(0, EINA_FALSE, EINA_TRUE);

	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void		menu_Screen_setViewport(void) {
	menu_viewport.isInterface = EINA_FALSE;
	if (menu_viewport.cur[0]==menu_viewport.br[0] || menu_viewport.cur[1] == menu_viewport.br[1])
		return;
#if !defined(USE_SHADER)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#endif
	if(menu_viewport.is3D)
		menu_Perspective(70.0,80/48,1.0,1000.0);
	else
		menu_Screen_Ortho(menu_viewport.cur[0], menu_viewport.br[0], menu_viewport.br[1], menu_viewport.cur[1], -1.0, 1.0);
#if !defined(USE_SHADER)
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	if(menu_viewport.is3D == EINA_TRUE)
		menu_LookAt(menu_viewport.eye[0], menu_viewport.eye[1], menu_viewport.eye[2], 
			   menu_viewport.at[0],  menu_viewport.at[1],  menu_viewport.at[2], 
			   menu_viewport.up[0],  menu_viewport.up[1],  menu_viewport.up[2]);
		
#endif
}

void		menu_Screen_setInterface(void) {
	menu_viewport.isInterface = EINA_TRUE;
	if (menu_viewport.cur[0]==menu_viewport.br[0] || menu_viewport.cur[1] == menu_viewport.br[1])
		return;
#if !defined(USE_SHADER)
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#endif
	menu_Screen_Ortho(0.0, menu_viewport.scrn[0]/menu_viewport.scaleFactor, menu_viewport.scrn[1]/menu_viewport.scaleFactor, 0.0, -1.0, 1.0);
}

Uint32 t=0;
Uint32 fpsrc=0;
Uint32 len=0;
Uint32 tmp=0;
void		menu_Screen_Flip(void) {
	if (showFPS) menu_RGroup_draw(fps);
	tmp = SDL_GetTicks();
#if !defined(HAVE_GLES)
	SDL_GL_SwapBuffers();
#else
	EGL_SwapBuffers();
#endif
	len+=SDL_GetTicks()-tmp;
	fpsrc++;
	if (SDL_GetTicks()>=t+1000 && showFPS) {
		if (menu_Theme_find_Font(g, "fps") && fps->f==NULL) {
			menu_RGroup_set_font(fps, menu_Theme_find_Font(g, "fps"));
		}
		char *txt  = calloc(18, sizeof(char));
		menu_vector p;
		p[1] = p[2] = 0.05;
		p[0] = 700.0;
		//p[0] = menu_viewport.scrn[0]/menu_viewport.scaleFactor-80.0*menu_viewport.scaleFactor;
		sprintf(txt, "e=%d, r=%d", fpsmc, fpsrc);
		if (fps->f!=NULL) {
			menu_RGroup_empty(fps);
			menu_RGroup_Text_add_at(fps, txt, p);
		}
		free(txt);

		t=SDL_GetTicks();
		fpsrc=0;
		fpsmc=0;
		len=0;
	}
	//glClearColor(0.0f,0.0f,0.0f,0.0f);
	//glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void		menu_Screen_Shutdown(void) {
#ifndef USE_SHADER
	glDisableClientState(GL_VERTEX_ARRAY);
#endif
#if defined(HAVE_GLES)
	EGL_Close();
#endif
	SDL_Quit();
}

void		menu_Screen_defaultArea_set(void) {
	menu_vector mn, mx;
	mn[0] = mn[1] = mn[2] = mx[2]  = 0.0;
	mx[0] = menu_viewport.scrn[0];
	mx[1] = menu_viewport.scrn[1];
	menu_Screen_Area_set(mn,mx);
}

void		menu_Screen_Area_set(menu_vector mn, menu_vector mx) {
	int i;
	menu_vector x = {0.0,0.0,0.0};
	for(i=0;i<2;i++) {
		menu_viewport.cur[i] = menu_viewport.min[i] = mn[i];menu_viewport.max[i] = mx[i];
		if(menu_viewport.max[i]-menu_viewport.min[i]<menu_viewport.orig[i]) menu_viewport.max[i]=menu_viewport.orig[i]+menu_viewport.min[i];
	}
	menu_Screen_View_move(x);
}

void		menu_Screen_View_move(menu_vector v) {
	int i;

	if(menu_viewport.is3D)
		for(i=0;i<3;i++) {
			menu_viewport.eye[i] += v[i];
			menu_viewport.at[i]  += v[i];
		}
	else
		for(i=0;i<2;i++) {
			menu_viewport.cur[i] += v[i];
			if(menu_viewport.cur[i]<menu_viewport.min[i]) menu_viewport.cur[i]=menu_viewport.min[i];
			if(menu_viewport.cur[i]>menu_viewport.max[i]-menu_viewport.orig[i]) menu_viewport.cur[i]=menu_viewport.max[i]-menu_viewport.orig[i];
			menu_viewport.br[i] = menu_viewport.orig[i] + menu_viewport.cur[i];
		}
}

void		menu_Screen_LookAt(menu_vector eye, menu_vector at, menu_vector up) {
	int i;
	for(i=0;i<3;i++) {
		menu_viewport.eye[i]= eye[i];
		menu_viewport.at[i] = at[i];
		menu_viewport.up[i] = up[i];
	}
}
