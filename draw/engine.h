#ifndef __menu_ENGINE_H__
#define __menu_ENGINE_H__
#include <SDL/SDL.h>
#include "texture.h"
#include "rgroup.h"
#include "ui.h"
#if defined(DEBUG)
#ifdef HAVE_GLES
#define GL_CHECK(x) \
        x; \
        { \
                GLenum glError = glGetError(); \
                switch(glError) { \
                        case GL_INVALID_OPERATION:      fprintf(stderr, "glGetError(INVALID_OPERATION) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                        case GL_INVALID_ENUM:           fprintf(stderr, "glGetError(INVALID_ENUM) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                        case GL_INVALID_VALUE:          fprintf(stderr, "glGetError(INVALID_VALUE) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                        case GL_OUT_OF_MEMORY:          fprintf(stderr, "glGetError(OUT_OF_MEMORY) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                        case 0: break; \
			default:                        fprintf(stderr, "glGetError(DEFAULT) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                } \
        }
#else
#define GL_CHECK(x) \
        x; \
        { \
                GLenum glError = glGetError(); \
                switch(glError) { \
                        case GL_INVALID_OPERATION:      fprintf(stderr, "glGetError(INVALID_OPERATION) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                        case GL_INVALID_ENUM:           fprintf(stderr, "glGetError(INVALID_ENUM) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                        case GL_INVALID_VALUE:          fprintf(stderr, "glGetError(INVALID_VALUE) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                        case GL_OUT_OF_MEMORY:          fprintf(stderr, "glGetError(OUT_OF_MEMORY) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                        case GL_INVALID_FRAMEBUFFER_OPERATION:  fprintf(stderr, "glGetError(INVALID_FRAMEBUFFER) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                        case 0: break; \
			default:                        fprintf(stderr, "glGetError(DEFAULT) = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__);      break; \
                } \
        }
#endif
#else
#define GL_CHECK(x) x;
#endif

#define glTranslate glTranslatef
#define glColor glColor4f

#if defined(__cplusplus)
extern "C" {
#endif


/* extentions code */
#ifndef HAVE_GLES
typedef void      (*gl_proc)                  (	void);
typedef gl_proc   (*getProcAddressProc)       (	const GLubyte *name);
typedef void      (*glGenBuffersProc)         (	GLsizei n, GLuint *buffers);
typedef void      (*glDeleteBuffersProc)      (	GLsizei n, const GLuint *buffers);
typedef void      (*glBindBufferProc)         (	GLenum target, GLuint buffer);
typedef void      (*glBufferDataProc)         (	GLenum target,
						GLsizeiptr size,
						const GLvoid *data,
						GLenum usage);
typedef void      (*glBufferSubDataProc)      (	GLenum target,
						GLintptr offset,
						GLsizeiptr size,
						const GLvoid *data);
typedef GLvoid    *(*glMapBufferProc)         (	GLenum target, GLenum access);
typedef GLboolean (*glUnmapBufferProc)        (	GLenum target);
typedef GLuint    (*glCreateProgramProc)      (	void);
typedef GLuint    (*glCreateShaderProc)       (	GLenum type);
typedef void      (*glCompileShaderProc)      (	GLuint shader);
typedef void      (*glDeleteShaderProc)       (	GLuint shader);
typedef void      (*glDeleteProgramProc)      (	GLuint program);
typedef void      (*glAttachShaderProc)       (	GLuint program, GLuint shader);
typedef void      (*glDetachShaderProc)       (	GLuint program, GLuint shader);
typedef void      (*glLinkProgramProc)        (	GLuint program);
typedef void      (*glUseProgramProc)         (	GLuint program);
typedef void      (*glShaderSourceProc)       (	GLuint shader,
						GLsizei count,
						const GLchar* *string,
						const GLint *length);
typedef GLint     (*glGetUniformLocationProc) (	GLuint program,
						const GLchar *name);
typedef void      (*glUniform2fvProc)         ( GLint location,
						GLsizei count,
						const GLfloat *value);
typedef void (*glGetShaderivProc)	      (	GLuint shader,
						GLenum pname,
						GLint *params);
typedef void (*glGetShaderInfoLogProc)	      (	GLuint shader,
						GLsizei maxLength,
						GLsizei *length,
						GLchar *infoLog);
typedef void (*glUniformMatrix4fvProc)	      (	GLint location,
						GLsizei count,
						GLboolean transpose,
						const GLfloat *value);
typedef GLint (*glGetAttribLocationProc)      (	GLuint program,
						const GLchar *name);
typedef void (*glUniform4fProc)		      (	GLint location,
						GLfloat v0,
						GLfloat v1,
						GLfloat v2,
						GLfloat v3);
typedef void (*glUniform4fvProc)	      (	GLint location,
						GLsizei count,
						const GLfloat *value);
typedef void (*glEnableVertexAttribArrayProc) (	GLuint index);
typedef void (*glVertexAttribPointerProc)      (	GLuint index,
						GLint size,
						GLenum type,
						GLboolean normalized,
						GLsizei stride,
						const GLvoid * pointer);


extern getProcAddressProc       glGetProcAddress;
extern glGenBuffersProc         glGenBuffers;
extern glBindBufferProc         glBindBuffer;
extern glBufferDataProc         glBufferData;
extern glBufferSubDataProc      glBufferSubData;
extern glMapBufferProc          glMapBuffer;
extern glUnmapBufferProc        glUnmapBuffer;
extern glDeleteBuffersProc      glDeleteBuffers;
extern glCreateProgramProc      glCreateProgram;
extern glCreateShaderProc       glCreateShader;
extern glCompileShaderProc      glCompileShader;
extern glDeleteShaderProc       glDeleteShader;
extern glDeleteProgramProc      glDeleteProgram;
extern glAttachShaderProc       glAttachShader;
extern glDetachShaderProc       glDetachShader;
extern glLinkProgramProc        glLinkProgram;
extern glUseProgramProc         glUseProgram;
extern glShaderSourceProc       glShaderSource;
extern glGetUniformLocationProc glGetUniformLocation;
extern glUniform2fvProc         glUniform2fv;
extern glGetShaderivProc	glGetShaderiv;
extern glGetShaderInfoLogProc	glGetShaderInfoLog;
extern glUniformMatrix4fvProc	glUniformMatrix4fv;
extern glGetAttribLocationProc	glGetAttribLocation;
extern glUniform4fProc		glUniform4f;
extern glUniform4fvProc		glUniform4fv;
extern glVertexAttribPointerProc glVertexAttribPointer;
extern glEnableVertexAttribArrayProc glEnableVertexAttribArray;
#else
# ifndef GL_IMG_texture_stream
#  define GL_TEXTURE_STREAM_IMG                                   0x8C0D     
#  define GL_TEXTURE_NUM_STREAM_DEVICES_IMG                       0x8C0E     
#  define GL_TEXTURE_STREAM_DEVICE_WIDTH_IMG                      0x8C0F
#  define GL_TEXTURE_STREAM_DEVICE_HEIGHT_IMG                     0x8EA0     
#  define GL_TEXTURE_STREAM_DEVICE_FORMAT_IMG                     0x8EA1      
#  define GL_TEXTURE_STREAM_DEVICE_NUM_BUFFERS_IMG                0x8EA2     
# endif

# ifndef GL_IMG_texture_stream2
#  define GL_IMG_texture_stream2 1
#  ifndef GL_APIENTRYP
#   define GL_APIENTRYP GL_APIENTRY*
#  endif
# endif

typedef void (*PFNGLTEXBINDSTREAMIMGPROC) (GLint device, GLint deviceoffset);
typedef const GLubyte *(*PFNGLGETTEXSTREAMDEVICENAMEIMGPROC) (GLenum target);
typedef void (*PFNGLGETTEXSTREAMDEVICEATTRIBUTEIVIMGPROC) (GLenum target, GLenum pname, GLint *params);
extern PFNGLTEXBINDSTREAMIMGPROC glTexBindStreamIMG;
extern PFNGLGETTEXSTREAMDEVICEATTRIBUTEIVIMGPROC glGetTexAttrIMG;
extern PFNGLGETTEXSTREAMDEVICENAMEIMGPROC glGetTexDeviceIMG;

int		menu_alloc_buff(int buff, int width, int height);
int		menu_free_buff(int buff);
extern unsigned long buf_paddr[10];    // physical address
extern char *buf_vaddr[10];            // virtual adress
extern char *buf_vaddr2[10];           // virtual adress, 2nd buffer

#endif

/* engine code */
typedef struct {
	menu_vector	min, max, cur, orig, scrn, br;
	menu_vector	eye, at,  up;
	int		scaleFactor;
	Eina_Bool	isInterface;
	Eina_Bool	is3D;
} Viewport;

extern Viewport		menu_viewport;
extern float		menu_Projection[16];
extern Eina_Bool	menu_havePBO;
extern SDL_mutex*	rendering_mutex;


extern Eina_Bool menu_quit;
void		menu_init(void);
void		menu_finish(void);
void		menu_main_Loading(const char *p_text, float p_pct);
void		menu_main_Loop(void);
inline Uint32	menu_GetTicks(void);

void		menu_setScene(menu_Scene *game, menu_Scene *ui);
void		menu_setUIScene(menu_Scene*ui);
void		menu_setUIScenePrevious();
void		menu_UISceneEmptyStack();
void		menu_setGameScene(menu_Scene* game);
void		menu_setUISceneAnime(menu_Scene*ui, int type);
menu_Scene*	menu_findScene(const char *name);
void		menu_addScene(menu_Scene* sc);
void		menu_Scenes_free();

void		menu_Screen_Flip(void);
void		menu_Screen_setViewport(void);
void		menu_Screen_setInterface(void);
void		menu_Screen_defaultArea_set(void);
void		menu_Screen_Area_set(menu_vector mn, menu_vector mx);
void		menu_Screen_View_move(menu_vector v);
void		menu_Screen_Eye_move(menu_vector v);
void		menu_Screen_LookAt(menu_vector eye, menu_vector at, menu_vector up);

#if defined(__cplusplus)
}
#endif
#endif
