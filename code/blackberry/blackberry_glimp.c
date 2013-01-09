/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "../renderer/tr_local.h"
#include "../client/client.h"
#include "../q3_ui/ui_local.h"

#ifdef __QNX__
#define FALSE 0
#define TRUE 1
#include <EGL/egl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/keycodes.h>
#include <screen/screen.h>
#include <input/screen_helpers.h>
#include <gestures/set.h>
#include <gestures/types.h>
#include <gestures/set.h>
#include <gestures/swipe.h>
#include <gestures/pinch.h>
#include <gestures/tap.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <bps/screen.h>
#include <bps/navigator.h>
#include <bps/sensor.h>
#include <bps/orientation.h>
#include <bps/virtualkeyboard.h>

screen_context_t screen_ctx;
screen_event_t screen_ev;
screen_window_t screen_win;

int bz[2];
#endif

#define PI 3.14159f
#define NUM_VERT 100
#define RESOLUTION_WIDTH 1024
#define RESOLUTION_HEIGHT 768

extern backEndState_t	backEnd;
extern void RB_SetGL2D (void);

static cvar_t *in_mouse;
static cvar_t *in_dgamouse;
cvar_t *in_subframe;
cvar_t *in_nograb;

static qboolean mouse_avail;
static qboolean mouse_active = qfalse;

static EGLDisplay eglDisplay = EGL_NO_DISPLAY;
static EGLContext eglContext = EGL_NO_CONTEXT;
static EGLSurface eglSurface = EGL_NO_SURFACE;

cvar_t  *r_allowSoftwareGL;
cvar_t  *r_previousglDriver;

int min = 0;
int max = 0;
static int movX, movY, movBoundX, movBoundY, movRad, movBoundRad, r1square, r2square;
static int jmpBoundX, jmpBoundY, jmpBoundRad;
static int fireBoundX, fireBoundY, fireBoundRad;
static int weaponNextX, weaponNextY, weaponNextRad;;

static GLfloat* vert;
int setup_vert = 0;

struct {
	EGLint surface_type;
	EGLint red_size;
	EGLint green_size;
	EGLint blue_size;
	EGLint alpha_size;
	EGLint samples;
	EGLint config_id;
} egl_conf_attr = {
	.surface_type = EGL_WINDOW_BIT,
	.red_size = EGL_DONT_CARE,
	.green_size = EGL_DONT_CARE,
	.blue_size = EGL_DONT_CARE,
	.alpha_size = EGL_DONT_CARE,
	.samples = EGL_DONT_CARE,
	.config_id = EGL_DONT_CARE,
};

    int prevCase = 0;
    int prevX = 0;
    int tempX = 0;
    int prevY = 0;
    int tempY = 0;
    int oldDist = 0;
    int velocity = 0;
    int dir = 0;

static const char *Q_stristr( const char *s, const char *find)
{
	register char c, sc;
	register size_t len;

	if ((c = *find++) != 0)
	{
		if (c >= 'a' && c <= 'z')
		{
			c -= ('a' - 'A');
		}
		len = strlen(find);
		do
		{
			do
			{
				if ((sc = *s++) == 0)
					return NULL;
				if (sc >= 'a' && sc <= 'z')
				{
					sc -= ('a' - 'A');
				}
			} while (sc != c);
		} while (Q_stricmpn(s, find, len) != 0);
		s--;
	}
	return s;
}

void PrintEglError( GLuint errorCode )
{
    switch( errorCode ) {
    case EGL_SUCCESS:
        fprintf( stderr, "EGL_SUCCESS");
        break;
    case EGL_NOT_INITIALIZED:
        fprintf( stderr, ">>EGL_NOT_INITIALIZED");
        break;
    case EGL_BAD_ACCESS:
        fprintf( stderr, ">>EGL_BAD_ACCESS");
        break;
    case EGL_BAD_ALLOC:
        fprintf( stderr, ">>EGL_BAD_ALLOC");
        break;
    case EGL_BAD_ATTRIBUTE:
        fprintf( stderr, ">>EGL_BAD_ATTRIBUTE");
        break;
    case EGL_BAD_CONTEXT:
        fprintf( stderr, ">>EGL_BAD_CONTEXT");
        break;
    case EGL_BAD_CONFIG:
        fprintf( stderr, ">>EGL_BAD_CONFIG");
        break;
    case EGL_BAD_CURRENT_SURFACE:
        fprintf( stderr, ">>EGL_BAD_CURRENT_SURFACE");
        break;
    case EGL_BAD_DISPLAY:
        fprintf( stderr, ">>EGL_BAD_DISPLAY");
        break;
    case EGL_BAD_SURFACE:
        fprintf( stderr, ">>EGL_BAD_SURFACE");
        break;
    case EGL_BAD_MATCH:
        fprintf( stderr, ">>EGL_BAD_MATCH");
        break;
    case EGL_BAD_PARAMETER:
        fprintf( stderr, ">>EGL_BAD_PARAMETER");
        break;
    case EGL_BAD_NATIVE_PIXMAP:
        fprintf( stderr, ">>EGL_BAD_NATIVE_PIXMAP");
        break;
    case EGL_BAD_NATIVE_WINDOW:
        fprintf( stderr, ">>EGL_BAD_NATIVE_WIN i=i+2DOW");
        break;
    case EGL_CONTEXT_LOST:
        fprintf( stderr, ">>EGL_CONTEXT_LOST");
        break;
    default:
        fprintf( stderr, ">>Unknown error");
        break;
    }
}

void BLACKBERRY_initDriver()
{
    int         err;
    int         egl_ret;
    EGLint      majorVersion;
    EGLint      minorVersion;

    // Get EGL display
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    err = eglGetError();
    if ( eglDisplay == EGL_NO_DISPLAY || err != EGL_SUCCESS){
        Com_Printf("eglGetDisplay");
        PrintEglError( err );
        return;
    }

    // Initialize EGL
    egl_ret = eglInitialize(eglDisplay, &majorVersion, &minorVersion);
    err = eglGetError();
    if( egl_ret != EGL_TRUE || err != EGL_SUCCESS ) {
        Com_Printf("eglInitialize");
        PrintEglError( err );
        return;
    }
}

EGLConfig choose_config(EGLDisplay eglDisplay, const char* str)
{
	EGLConfig egl_conf = (EGLConfig)0;
	EGLConfig *egl_configs;
	EGLint egl_num_configs;
	EGLint val;
	EGLBoolean rc;
	const char *tok;
	EGLint i;

	if (str != NULL) {
		tok = str;
		while (*tok == ' ' || *tok == ',') {
			tok++;
		}

		while (*tok != '\0') {
			if (strncmp(tok, "rgba8888", strlen("rgba8888")) == 0) {
				egl_conf_attr.red_size = 8;
				egl_conf_attr.green_size = 8;
				egl_conf_attr.blue_size = 8;
				egl_conf_attr.alpha_size = 8;
				tok += strlen("rgba8888");
			} else if (strncmp(tok, "rgba5551", strlen("rgba5551")) == 0) {
				egl_conf_attr.red_size = 5;
				egl_conf_attr.green_size = 5;
				egl_conf_attr.blue_size = 5;
				egl_conf_attr.alpha_size = 1;
				tok += strlen("rgba5551");
			} else if (strncmp(tok, "rgba4444", strlen("rgba4444")) == 0) {
				egl_conf_attr.red_size = 4;
				egl_conf_attr.green_size = 4;
				egl_conf_attr.blue_size = 4;
				egl_conf_attr.alpha_size = 4;
				tok += strlen("rgba4444");
			} else if (strncmp(tok, "rgb565", strlen("rgb565")) == 0) {
				egl_conf_attr.red_size = 5;
				egl_conf_attr.green_size = 6;
				egl_conf_attr.blue_size = 5;
				egl_conf_attr.alpha_size = 0;
				tok += strlen("rgb565");
			} else if (isdigit(*tok)) {
				val = atoi(tok);
				while (isdigit(*(++tok)));
				if (*tok == 'x') {
					egl_conf_attr.samples = val;
					tok++;
				} else {
					egl_conf_attr.config_id = val;
				}
			} else {
				fprintf(stderr, "invalid configuration specifier: ");
				while (*tok != ' ' && *tok != ',' && *tok != '\0') {
					fputc(*tok++, stderr);
				}
				fputc('\n', stderr);
			}

			while (*tok == ' ' || *tok == ',') {
				tok++;
			}
		}
	}

	rc = eglGetConfigs(eglDisplay, NULL, 0, &egl_num_configs);
	if (rc != EGL_TRUE) {
		Com_Printf("eglGetConfigs");
		return egl_conf;
	}
	if (egl_num_configs == 0) {
		Com_Printf("eglGetConfigs: could not find a configuration\n");
		return egl_conf;
	}

	egl_configs = malloc(egl_num_configs * sizeof(*egl_configs));
	if (egl_configs == NULL) {
		Com_Printf("could not allocate memory for %d EGL configs\n", egl_num_configs);
		return egl_conf;
	}

	rc = eglGetConfigs(eglDisplay, egl_configs,
		egl_num_configs, &egl_num_configs);
	if (rc != EGL_TRUE) {
		Com_Printf("eglGetConfigs");
		free(egl_configs);
		return egl_conf;
	}

	for (i = 0; i < egl_num_configs; i++) {
		if (egl_conf_attr.config_id != EGL_DONT_CARE) {
			eglGetConfigAttrib(eglDisplay, egl_configs[i], EGL_CONFIG_ID, &val);
			if (val == egl_conf_attr.config_id) {
				egl_conf = egl_configs[i];
				break;
			} else {
				continue;
			}
		}

		eglGetConfigAttrib(eglDisplay, egl_configs[i], EGL_SURFACE_TYPE, &val);
		if ((val & egl_conf_attr.surface_type) != egl_conf_attr.surface_type) {
			continue;
		}

		eglGetConfigAttrib(eglDisplay, egl_configs[i], EGL_RENDERABLE_TYPE, &val);
		if (!(val & EGL_OPENGL_ES_BIT)) {
			continue;
		}

		eglGetConfigAttrib(eglDisplay, egl_configs[i], EGL_DEPTH_SIZE, &val);
		if (val == 0) {
			continue;
		}

		if (egl_conf_attr.red_size != EGL_DONT_CARE) {
			eglGetConfigAttrib(eglDisplay, egl_configs[i], EGL_RED_SIZE, &val);
			if (val != egl_conf_attr.red_size) {
				continue;
			}
		}
		if (egl_conf_attr.green_size != EGL_DONT_CARE) {
			eglGetConfigAttrib(eglDisplay, egl_configs[i], EGL_GREEN_SIZE, &val);
			if (val != egl_conf_attr.green_size) {
				continue;
			}
		}
		if (egl_conf_attr.blue_size != EGL_DONT_CARE) {
			eglGetConfigAttrib(eglDisplay, egl_configs[i], EGL_BLUE_SIZE, &val);
			if (val != egl_conf_attr.blue_size) {
				continue;
			}
		}
		if (egl_conf_attr.alpha_size != EGL_DONT_CARE) {
			eglGetConfigAttrib(eglDisplay, egl_configs[i], EGL_ALPHA_SIZE, &val);
			if (val != egl_conf_attr.alpha_size) {
				continue;
			}
		}
		if (egl_conf_attr.samples != EGL_DONT_CARE) {
			eglGetConfigAttrib(eglDisplay, egl_configs[i], EGL_SAMPLES, &val);
			if (val != egl_conf_attr.samples) {
				continue;
			}
		}

		egl_conf = egl_configs[i];
		break;
	}

	free(egl_configs);

	if (egl_conf == (EGLConfig)0) {
		Com_Printf("eglChooseConfig: could not find a matching configuration\n");
	}

	return egl_conf;
}

int choose_format(EGLDisplay eglDisplay, EGLConfig egl_conf)
{
	EGLint buffer_bit_depth, alpha_bit_depth;

	eglGetConfigAttrib(eglDisplay, egl_conf, EGL_BUFFER_SIZE, &buffer_bit_depth);
	eglGetConfigAttrib(eglDisplay, egl_conf, EGL_ALPHA_SIZE, &alpha_bit_depth);
	switch (buffer_bit_depth) {
	case 32: {
		return SCREEN_FORMAT_RGBA8888;
	}
	case 24: {
		return SCREEN_FORMAT_RGB888;
	}
	case 16: {
		switch (alpha_bit_depth) {
		case 4: {
			return SCREEN_FORMAT_RGBA4444;
		}
		case 1: {
			return SCREEN_FORMAT_RGBA5551;
		}
		default: {
			return SCREEN_FORMAT_RGB565;
		}
		}
	}
	default: {
		return 0;
	}
	}
}

static void InitControls(void)
{
	movBoundX = (glConfig.vidWidth * 12) / 100;
	movBoundY = (glConfig.vidHeight * 70) / 100;
	movBoundRad = glConfig.vidWidth * 0.08f;

	fireBoundX = (glConfig.vidWidth * 78) / 100;
	fireBoundY = (glConfig.vidHeight * 69) / 100;
	fireBoundRad = glConfig.vidWidth * 0.088f;

	jmpBoundX = (glConfig.vidWidth * 91) / 100;
	jmpBoundY = (glConfig.vidHeight * 62) / 100;
	jmpBoundRad = glConfig.vidWidth * 0.065f;

	weaponNextX = (glConfig.vidWidth * 88) / 100;
	weaponNextY = (glConfig.vidHeight * 76) / 100;
	weaponNextRad = glConfig.vidWidth * 0.030f;

	r1square = movBoundRad * movBoundRad;
	r2square = fireBoundRad * fireBoundRad;
}

int BLACKBERRY_InitGL(void)
{
    bps_initialize();
    navigator_request_events(0);

#ifdef __X86__
    int usage = SCREEN_USAGE_OPENGL_ES1;
#else
    // Physical device copy directly into physical display
    int usage = SCREEN_USAGE_DISPLAY|SCREEN_USAGE_OPENGL_ES1;
#endif
	int transp = SCREEN_TRANSPARENCY_NONE;
	EGLint interval = 1;
	int size[2] = { -1, -1 };
	int pos[2] = { 0, 0 };
	int nbuffers = 2;
	int format;
	EGLConfig config;
    EGLint err;

    config = choose_config(eglDisplay, "rgba8888");
    if (config == (EGLConfig)0) {
        Com_Printf( "Demo Thread Init: failed to find config!" );
        return FALSE;
	}

    // Create EGL rendering context
    eglContext = eglCreateContext( eglDisplay, config, EGL_NO_CONTEXT, NULL );
    err = eglGetError( );
    if ( eglContext == EGL_NO_CONTEXT ) {
        Com_Printf( "Demo Thread Init: can't create gles2 context!" );
        PrintEglError( err );
		return FALSE;
    }

    err = screen_create_context(&screen_ctx, 0);
	if (err) {
		Com_Printf("screen_create_context");
		return FALSE;
	}

    err = screen_create_window(&screen_win, screen_ctx);
	if (err) {
		Com_Printf("screen_create_window");
		return FALSE;
	}

	format = choose_format(eglDisplay, config);
	err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &format);
	if (err) {
		Com_Printf("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)");
		return FALSE;
	}

	err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
	if (err) {
		Com_Printf("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE)");
		return FALSE;
	}

	size[0]=RESOLUTION_WIDTH;
	size[1]=RESOLUTION_HEIGHT;

	err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size);
	if (err) {
		Com_Printf("screen_set_window_property_iv(screen_set_window_property_iv)");
		return FALSE;
	}

	if (size[0] > 0 && size[1] > 0) {
		err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SIZE, size);
		if (err) {
			Com_Printf("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE)");
			return FALSE;
		}
	} else {
		err = screen_get_window_property_iv(screen_win, SCREEN_PROPERTY_SIZE, size);
		if (err) {
			Com_Printf("screen_get_window_property_iv(SCREEN_PROPERTY_SIZE)");
			return FALSE;
		}
	}

	glConfig.vidWidth = size[0];
	glConfig.vidHeight = size[1];

	InitControls();

	bz[0] = size[0];
	bz[1] = size[1];

	if (pos[0] != 0 || pos[1] != 0) {
		err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_POSITION, pos);
		if (err) {
			Com_Printf("screen_set_window_property_iv(SCREEN_PROPERTY_POSITION)");
			return FALSE;
		}
	}

	err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_TRANSPARENCY, &transp);
	if (err) {
		Com_Printf("screen_set_window_property_iv(SCREEN_PROPERTY_TRANSPARENCY)");
		return FALSE;
	}

	err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SWAP_INTERVAL, &interval);
	if (err) {
		Com_Printf("screen_set_window_property_iv(SCREEN_PROPERTY_SWAP_INTERVAL)");
		return FALSE;
	}

	err = screen_create_window_buffers(screen_win, nbuffers);
	if (err) {
		Com_Printf("screen_create_window_buffers");
		return FALSE;
	}

	size[0] = RESOLUTION_WIDTH;
	size[1] = RESOLUTION_HEIGHT;
	err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SIZE, size);
	if (err) {
		Com_Printf("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE)");
		return FALSE;
	}

	err = screen_create_event(&screen_ev);
	if (err) {
		Com_Printf("screen_create_event");
		return FALSE;
	}
	screen_request_events(screen_ctx);

	eglSurface = eglCreateWindowSurface(eglDisplay, config, screen_win, NULL);
	if (eglSurface == EGL_NO_SURFACE) {
        Com_Printf( "Demo Thread Init: can't create surface!" );
        PrintEglError( err );
        return FALSE;
	}

	err = eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if (err != EGL_TRUE) {
        Com_Printf( "Demo Thread Init: can't make current!" );
		return FALSE;
	}

    return TRUE;
}

static void drawControls()
{
	float ctrlAlpha = 0.20f;

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vert);

	// Outer move joggle
	glColor4f(0.8f, 0.8f, 0.8f, ctrlAlpha);
	glPushMatrix();
	glTranslatef(movBoundX, movBoundY, 0);
	glScalef(movBoundRad, movBoundRad, 1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERT + 2);
	glPopMatrix();

	if (movX != -1 && movY != -1) {
		// Inner move joggle
		glPushMatrix();
		glTranslatef(movX + movBoundX, movY + movBoundY, 0);
		glScalef(movBoundRad/3, movBoundRad/3, 1);
		glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERT + 2);
		glPopMatrix();
	}

	// Fire button
	glColor4f(0.8f, 0.0f, 0.0f, ctrlAlpha);
	glPushMatrix();
	glTranslatef(fireBoundX, fireBoundY, 0);
	glScalef(fireBoundRad, fireBoundRad, 1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERT + 2);
	glPopMatrix();

	// Jump button
	glColor4f(0.8f, 0.8f, 0.0f, ctrlAlpha);
	glPushMatrix();
	glTranslatef(jmpBoundX, jmpBoundY, 0);
	glScalef(jmpBoundRad, jmpBoundRad, 1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERT + 2);
	glPopMatrix();

	// Weapon next button
	glColor4f(0.2f, 0.6f, 0.2f, ctrlAlpha);
	glPushMatrix();
	glTranslatef(weaponNextX, weaponNextY, 0);
	glScalef(weaponNextRad, weaponNextRad, 1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERT + 2);
	glPopMatrix();
}

void SwapBuffers( void )
{
	int err, i;
	int t;
	int size[2];
	int pos[2];

	t = Sys_Milliseconds();

	if (!setup_vert)
	{
		vert = (GLfloat*)malloc(2*(NUM_VERT+2) * sizeof(GLfloat));
		vert[0] = 0;
		vert[1] = 0;
		for (i = 0; i < NUM_VERT+1; i+=2)
		{
			vert[i+2] = (GLfloat)(sinf(2 * PI / NUM_VERT * i)) * 600 / 768;
			vert[i+3] = (GLfloat)(cosf(2 * PI / NUM_VERT * i));
		}
		setup_vert = 1;
	}

	if (cls.state == CA_ACTIVE)
	{
		if ( !backEnd.projection2D )
			RB_SetGL2D();

		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		if (!uis.activemenu)
			drawControls();

		glEnable(GL_TEXTURE_2D);
	}
	eglSwapBuffers(eglDisplay, eglSurface);
	err = eglGetError( );
	if (err != EGL_SUCCESS ) {
		Com_Printf( "Error, eglSwapBuffers failed" );
		PrintEglError( err );
		return;
	}
}

/*
** BLACKBERRY_InitGraphics
*/
int BLACKBERRY_InitGraphics( int mode )
{
	int ret;

    ret = BLACKBERRY_InitGL();
    if(FALSE == ret) {
        Com_Printf("Error, failed to initialize gl");
        return FALSE;
    }

	return TRUE;
}

static qboolean LoadOpenGL( const char *name )
{
	qboolean fullscreen;
	ri.Printf( PRINT_ALL, "...loading %s: ", name );
	// load GL layer
	fullscreen = r_fullscreen->integer;
    ri.Cvar_Set( "r_fullscreen", "1" );
	r_fullscreen->modified = qtrue;

	// create the window and set up the context
	if ( !BLACKBERRY_InitGraphics(r_mode->integer) )
	{
        ri.Printf( PRINT_ALL, "FAILURE: InitGL failed in this mode\n" );
        return qfalse;
	}
	return qtrue;
}

void GLimp_Init( void )
{
	char  buf[1024];
	cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );
	r_allowSoftwareGL = ri.Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );
	r_previousglDriver = ri.Cvar_Get( "r_previousglDriver", "", CVAR_ROM );

	// Hack here so that if the UI 
	if ( *r_previousglDriver->string )
	{
		// The UI changed it on us, hack it back, this means the renderer can't be changed on the fly
		ri.Cvar_Set( "r_glDriver", r_previousglDriver->string );
	}
	
	// Load and initialize the specific OpenGL driver
	if ( !LoadOpenGL( r_glDriver->string ) ) {
		return;
	}

	// Save it in case the UI stomps it
	ri.Cvar_Set( "r_previousglDriver", r_glDriver->string );

	// This values force the UI to disable driver selection
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;

	// get our config strings
	Q_strncpyz( glConfig.vendor_string, qglGetString (GL_VENDOR), sizeof( glConfig.vendor_string ) );
	Q_strncpyz( glConfig.renderer_string, qglGetString (GL_RENDERER), sizeof( glConfig.renderer_string ) );
	if (*glConfig.renderer_string && glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] == '\n')
		glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] = 0;
	Q_strncpyz( glConfig.version_string, qglGetString (GL_VERSION), sizeof( glConfig.version_string ) );
	Q_strncpyz( glConfig.extensions_string, qglGetString (GL_EXTENSIONS), sizeof( glConfig.extensions_string ) );

	//
	// chipset specific configuration
	//
	strcpy( buf, glConfig.renderer_string );
	strlwr( buf );

	//
	// NOTE: if changing cvars, do it within this block.  This allows them
	// to be overridden when testing driver fixes, etc. but only sets
	// them to their default state when the hardware is first installed/run.
	//
	if ( Q_stricmp( lastValidRenderer->string, glConfig.renderer_string ) )
	{
		glConfig.hardwareType = GLHW_GENERIC;

		ri.Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST" );

		// VOODOO GRAPHICS w/ 2MB
		if ( Q_stristr( buf, "voodoo graphics/1 tmu/2 mb" ) )
		{
			ri.Cvar_Set( "r_picmip", "2" );
			ri.Cvar_Get( "r_picmip", "1", CVAR_ARCHIVE | CVAR_LATCH );
		} else
		{
			ri.Cvar_Set( "r_picmip", "1" );

			if ( Q_stristr( buf, "rage 128" ) || Q_stristr( buf, "rage128" ) )
			{
				ri.Cvar_Set( "r_finish", "0" );
			}
			// Savage3D and Savage4 should always have trilinear enabled
			else if ( Q_stristr( buf, "savage3d" ) || Q_stristr( buf, "s3 savage4" ) )
			{
				ri.Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
			}
		}
	}

	ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );

	return;
}

void IN_DeactivateMouse( void ) 
{
    if (!mouse_avail || !eglDisplay)
		return;

	if (mouse_active)
	{
		ri.Cvar_Set("in_dgamouse", "0");
		mouse_active = qfalse;
	}
}

void GLimp_Shutdown( void )
{
	if (!eglContext || !eglDisplay)
	    return;
	
    IN_DeactivateMouse();

    screen_destroy_event(screen_ev);

    eglMakeCurrent(eglDisplay, NULL, NULL, NULL);
	eglDestroySurface(eglDisplay, eglSurface);
	screen_destroy_window(screen_win);
	screen_destroy_context(screen_ctx);
	eglDestroyContext(eglDisplay, eglContext);
	eglTerminate(eglDisplay);
	eglReleaseThread();

    screen_stop_events(screen_ctx);
    bps_shutdown();

	eglDisplay = NULL;
	eglContext = NULL;
	eglSurface = NULL;

	memset( &glConfig, 0, sizeof( glConfig ) );
	memset( &glState, 0, sizeof( glState ) );
}

void GLimp_EnableLogging( qboolean enable )
{
}

void GLimp_LogComment( char *comment )
{
}

void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
{
}

qboolean GLimp_SpawnRenderThread( void (*function)( void ) )
{
	ri.Printf( PRINT_WARNING, "ERROR: SMP support was disabled at compile time\n");
	return qfalse;
}
void GLimp_FrontEndSleep( void )
{
}

void *GLimp_RendererSleep( void )
{
    return NULL;
}

void GLimp_WakeRenderer( void *data )
{
}

qboolean QGL_Init( const char *dllname )
{

	return qtrue;
}

void QGL_Shutdown( void )
{
}

void IN_Init( void )
{
	Com_Printf ("\n------- Input Initialization -------\n");
	// mouse variables
	in_mouse = Cvar_Get ("in_mouse", "1", CVAR_ARCHIVE);
  	in_dgamouse = Cvar_Get ("in_dgamouse", "0", CVAR_ARCHIVE);

	// turn on-off sub-frame timing of X events
	in_subframe = Cvar_Get ("in_subframe", "1", CVAR_ARCHIVE);
	// developer feature, allows to break without loosing mouse pointer
	in_nograb = Cvar_Get ("in_nograb", "0", 0);
	if (in_mouse->value)
		mouse_avail = qtrue;
	else
		mouse_avail = qfalse;
	Com_Printf ("------------------------------------\n");
}

int jvmfd = -1;

void IN_ActivateMouse( void ) 
{
    if (!mouse_avail || !eglDisplay)
        return;

	if (!mouse_active)
	{
		ri.Cvar_Set("in_dgamouse", "0");
		mouse_active = qtrue;
	}
}

void IN_Frame (void)
{
	if ( cls.keyCatchers & KEYCATCH_CONSOLE )
	{
		// temporarily deactivate if not in the game and running on the desktop voodoo always counts as full screen
		if (Cvar_VariableValue ("r_fullscreen") == 0 && strcmp( Cvar_VariableString("r_glDriver"), _3DFX_DRIVER_NAME ) )
		{
			IN_DeactivateMouse ();
			return;
		}
	}

	if( ri.Cvar_Set )
		IN_ActivateMouse();
}

void IN_Shutdown(void)
{
	mouse_avail = qfalse;
}


static char* LateKey(int scancode, int *key)
{
	static char buf[4];

	buf[0] = scancode;
	*key = 0;

#ifdef KBD_DBG
	ri.Printf(PRINT_ALL, "LateKey: scancode %d", scancode);
#endif
	
	switch (scancode)
	{
	    default:
		    if (scancode == 0)
		    {
			    if (com_developer->value)
			    {
				    ri.Printf(PRINT_ALL, "Warning: scancode: 0\n");
			    }
			    return NULL;
		    } else {
			    // XK_* tests failed, but XLookupString got a buffer, so let's try it
			    *key = *(unsigned char *)buf;
			    if (*key >= 'A' && *key <= 'Z')
				    *key = *key - 'A' + 'a';
			    // if ctrl is pressed, the keys are not between 'A' and 'Z', for instance ctrl-z == 26 ^Z ^C etc.
			    // see https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=19
			    else if (*key >= 1 && *key <= 26)
		 	      *key = *key + 'a' - 1;
		    }
		    break;
	}

	return buf;
}


static int mouseResetTime = 0;
#define MOUSE_RESET_DELAY 50
#define CAM_THROTTLE_THRESHHOLD 30

static void HandleEvents(void)
{
	int t = 0;
	int rc = 0;
	int freeze = 0;
	int prop;
	int size[2];
	int x = 0, y = 0, tid = 0, key = 0, keyflags = 0;
	mtouch_event_t mtouch_event;
	static int retainMovX = 0, retainMovY = 0;
	static int lookX = 0, lookY = 0;
	static int moveTouchId = -1;
	static int lookTouchId = -1;
	int domain = 0, eventType = 0;
	static qboolean touchIds[3] = { false, false, false };
	qboolean dowarp = qfalse;
	int jmping = 0;

	t = Sys_Milliseconds();

	if (jmping)
	{
		Sys_QueEvent( t, SE_KEY, K_SPACE, qfalse, 0, NULL );
	}

    bps_event_t* event = NULL;

    while (true)
    {
        rc = bps_get_event(&event, 1);
        if (rc != BPS_SUCCESS || event == NULL)
            break;

        domain = bps_event_get_domain(event);

        if (domain == screen_get_domain())
        {
            screen_ev = screen_event_get_event(event);
            screen_get_event_property_iv(screen_ev, SCREEN_PROPERTY_TYPE, &eventType);

            switch (eventType)
            {
                case SCREEN_EVENT_CLOSE:
                {
                    Com_Printf("SCREEN CLOSE EVENT!\n");
                }
                break;

                case SCREEN_EVENT_MTOUCH_RELEASE:
                {
                    screen_get_mtouch_event(screen_ev, &mtouch_event, 0);
                    tid = mtouch_event.contact_id;

                    if (touchIds[tid])
                    {
                        if (moveTouchId == tid)
                        {
                            moveTouchId = -1;
                            touchIds[tid] = false;
                            movX = movY = 0;
                            retainMovX = retainMovY = 0;
                        }
                        else if (lookTouchId == tid)
                        {
                            lookTouchId = -1;
                            touchIds[tid] = false;
                            lookX = lookY = 0;
                        }
                    }
                    Sys_QueEvent(t, SE_KEY, K_MOUSE1, qfalse, 0, NULL);
                    Sys_QueEvent( t, SE_KEY, K_SPACE, qfalse, 0, NULL );
                    Sys_QueEvent( t, SE_KEY, 's', qfalse, 0, NULL );
                    Sys_QueEvent( t, SE_KEY, 'w', qfalse, 0, NULL );
                    Sys_QueEvent( t, SE_KEY, 'd', qfalse, 0, NULL );
                    Sys_QueEvent( t, SE_KEY, 'a', qfalse, 0, NULL );
                    Sys_QueEvent( t, SE_KEY, 'n', qfalse, 0, NULL );
                }
                break;

                case SCREEN_EVENT_MTOUCH_MOVE:
                case SCREEN_EVENT_MTOUCH_TOUCH:
                case SCREEN_EVENT_POINTER:
                {
                    screen_get_mtouch_event(screen_ev, &mtouch_event, 0);
                    if (mouse_active && tid < 3)
                    {
                        if (cls.state == CA_ACTIVE && !uis.activemenu)
                        {
                            x = mtouch_event.x;
                            y = mtouch_event.y;
                            tid = mtouch_event.contact_id;

                            if (eventType == SCREEN_EVENT_MTOUCH_TOUCH)
                            {
                                // check for new input type
                                if (moveTouchId == -1 && (((movBoundX - x) * (movBoundX - x) + (movBoundY - y)*(movBoundY - y)) < r1square ))
                                {
                                    // move
                                    moveTouchId = tid;
                                    touchIds[tid] = true;
                                }
                                else
                                {
                                    // look
                                    lookTouchId = tid;
                                    touchIds[tid] = true;
                                    lookX = x;
                                    lookY = y;

                                    // allow looking while firing/jumping
                                    if (((fireBoundX - x) * (fireBoundX - x) + (fireBoundY - y)*(fireBoundY - y)) < r2square)
                                    {
                                        // fire
                                        Sys_QueEvent(t, SE_KEY, K_MOUSE1, qtrue, 0, NULL);
                                    }
                                    else if (((jmpBoundX - x) * (jmpBoundX - x) + (jmpBoundY - y)*(jmpBoundY - y)) < r2square )
                                    {
                                        // jump
                                        Sys_QueEvent( t, SE_KEY, K_SPACE, qtrue, 0, NULL );
                                    }
                                    else if (((weaponNextX - x) * (weaponNextX - x) + (weaponNextY - y)*(weaponNextY - y)) < r2square )
                                    {
                                        // weapon prev
                                        Sys_QueEvent( t, SE_KEY, 'n', qtrue, 0, NULL );
                                    }
                                }
                            }

                            if (moveTouchId == tid)
                            {
                                // process moving
                                x = (x - movBoundX);
                                y = (y - movBoundY);
                                retainMovX = x;
                                retainMovY = y;
                                movX = x * 640 / 786;
                                movY = y;
                                if (x * x + y * y > r1square)
                                {
                                    movX = movY = -1; // hide
                                }
                            }
                            else if (lookTouchId == tid)
                            {
                                // process looking
                                Sys_QueEvent(t, SE_MOUSE, (x - lookX) * 2, (y - lookY) * 2, 0, NULL);
                                lookX = x;
                                lookY = y;
                            }
                        }
                        else if (cls.state == CA_CINEMATIC)
                        {
                            x = mtouch_event.x;
                            y = mtouch_event.y;

                            if (eventType == SCREEN_EVENT_MTOUCH_TOUCH)
                            {
                                Sys_QueEvent(t, SE_KEY, K_MOUSE1, qtrue, 0, NULL);
                            }
                            else if (eventType == SCREEN_EVENT_MTOUCH_RELEASE)
                            {
                                Sys_QueEvent(t, SE_KEY, K_MOUSE1, qfalse, 0, NULL);
                            }
                        }
                        else if ((cls.state == CA_ACTIVE && uis.activemenu) ||  (cls.state != CA_ACTIVE && cls.state != CA_CINEMATIC))
                        {
                            x = (mtouch_event.x * 640) / RESOLUTION_WIDTH;
                            y = (mtouch_event.y * 480 ) / RESOLUTION_HEIGHT;

                            if (x == glConfig.vidWidth/2 &&  y == glConfig.vidHeight/2)
                            {
                                x = y = 0;
                                break;
                            }

                            dowarp = qtrue;

                            x =  x - prevX;
                            y =  y - prevY;

                            Sys_QueEvent(t, SE_MOUSE, x, y, 0, NULL);

                            prevX = (mtouch_event.x * 640) / RESOLUTION_WIDTH;
                            prevY = (mtouch_event.y * 480 ) / RESOLUTION_HEIGHT;

                            if (eventType == SCREEN_EVENT_MTOUCH_TOUCH)
                            {
                                Sys_QueEvent(t, SE_KEY, K_MOUSE1, qtrue, 0, NULL);
                            }
                            else if (eventType == SCREEN_EVENT_MTOUCH_RELEASE)
                            {
                                Sys_QueEvent(t, SE_KEY, K_MOUSE1, qfalse, 0, NULL);
                            }
                        }

                    }
                    if (retainMovX > movBoundRad / 2)
                    {
                        Sys_QueEvent( t, SE_KEY, 'd', qtrue, 0, NULL );
                    }
                    else if (retainMovX < -movBoundRad / 2 )
                    {
                        Sys_QueEvent( t, SE_KEY, 'a', qtrue, 0, NULL );
                    }
                    else
                    {
                        Sys_QueEvent( t, SE_KEY, 'd', qfalse, 0, NULL );
                        Sys_QueEvent( t, SE_KEY, 'a', qfalse, 0, NULL );
                    }
                    if (retainMovY > movBoundRad / 2)
                    {
                        Sys_QueEvent( t, SE_KEY, 's', qtrue, 0, NULL );
                    }
                    else if (retainMovY < -movBoundRad / 2)
                    {
                        Sys_QueEvent( t, SE_KEY, 'w', qtrue, 0, NULL );
                    }
                    else
                    {
                        Sys_QueEvent( t, SE_KEY, 's', qfalse, 0, NULL );
                        Sys_QueEvent( t, SE_KEY, 'w', qfalse, 0, NULL );
                    }
                }
                break;


                case SCREEN_EVENT_KEYBOARD:
                {
                    screen_get_event_property_iv(screen_ev, SCREEN_PROPERTY_KEY_SYM, &key);
                    screen_get_event_property_iv(screen_ev, SCREEN_PROPERTY_KEY_FLAGS, &keyflags);
                    if (keyflags & KEY_DOWN)
                    {
                        if (key >= ' ' && key < '~')
                        {
                            Sys_QueEvent( t, SE_CHAR, key, qfalse, 0, NULL );
                        }
                        else
                        {
                            key = key - 0xf000;
                            switch (key)
                            {
                            case 8: // backspace
                                Sys_QueEvent( t, SE_CHAR, '\b', qfalse, 0, NULL );
                                return;
                            default:
                                break;
                            }
                            Sys_QueEvent( t, SE_KEY, key, qtrue, 0, NULL );
                            Sys_QueEvent( t, SE_KEY, key, qfalse, 0, NULL );
                        }
                    }
                }
                break;
            }
        }
        else if (domain == navigator_get_domain())
        {
            switch (bps_event_get_code(event))
             {
             case NAVIGATOR_SWIPE_DOWN:
                 Sys_QueEvent( Sys_Milliseconds(), SE_KEY, K_ESCAPE, qtrue, 0, NULL );
                 Sys_QueEvent( Sys_Milliseconds(), SE_KEY, K_ESCAPE, qfalse, 0, NULL );
                 break;
             case NAVIGATOR_EXIT:
                 exit(0);
                 break;
             }
        }
	}
}

void Sys_SendKeyEvents (void)
{
	if (!eglDisplay)
	    return;

	HandleEvents();
}

void GLimp_EndFrame( void )
{
	char buffer[500];
	char* p;
	int len;
	const char *msg = NULL;
	int interval;
	int size[2];
	int pos[2];
	int rc;

	if ( strcmp( r_drawBuffer->string, "GL_FRONT" ) != 0 )
	{
		SwapBuffers();
	}
}
