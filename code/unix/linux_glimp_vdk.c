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
/*
** GLW_IMP.C
**
** This file contains ALL Linux specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
** GLimp_SetGamma
**
*/
/* We USE EGL on X */
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include <dlfcn.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../renderer/tr_local.h"
#include "../client/client.h"
#include "linux_local.h" // bk001130

#include "unix_glw.h"

// we use egl
#include <EGL/egl.h>
#ifndef USE_VDK
#error Non-VDK version not supported
#endif

#include <gc_vdk.h>

#define	WINDOW_CLASS_NAME	"Quake III: Arena"

typedef enum
{
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;

glwstate_t glw_state;

/*
static Display *dpy = NULL;
static int scrnum;
static Window win = 0;
static GLXContext ctx = NULL;
*/
static vdkPrivate vdk;

static NativeDisplayType display = NULL;
static NativeWindowType  window  = NULL;

static EGLDisplay eglDisplay = NULL;
static EGLContext eglContext = NULL;
static EGLSurface eglSurface = NULL;


static qboolean mouse_avail;
static qboolean mouse_active = qfalse;
static int mwx = 0, mwy = 0;
static int mx = 0, my = 0;

// Time mouse was reset, we ignore the first 50ms of the mouse to allow settling of events
static int mouseResetTime = 0;
#define MOUSE_RESET_DELAY 50

static cvar_t *in_mouse;
static cvar_t *in_dgamouse; // user pref for dga mouse
cvar_t *in_subframe;
cvar_t *in_nograb; // this is strictly for developers

cvar_t  *r_allowSoftwareGL;   // don't abort out if the pixelformat claims software
cvar_t  *r_previousglDriver;

static int win_x, win_y;

static int mouse_accel_numerator;
static int mouse_accel_denominator;
static int mouse_threshold;    

/*
* Find the first occurrence of find in s.
*/
// bk001130 - from cvs1.17 (mkv), const
// bk001130 - made first argument const
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

/*****************************************************************************
** KEYBOARD
** NOTE TTimo the keyboard handling is done with KeySyms
**   that means relying on the keyboard mapping provided by X
**   in-game it would probably be better to use KeyCode (i.e. hardware key codes)
**   you would still need the KeySyms in some cases, such as for the console and all entry textboxes
**     (cause there's nothing worse than a qwerty mapping on a french keyboard)
**
** you can turn on some debugging and verbose of the keyboard code with #define KBD_DBG
******************************************************************************/

//#define KBD_DBG

static char *LateKey(int scancode, int *key)
{
	static char buf[4];

	buf[0] = scancode;
	*key = 0;

#ifdef KBD_DBG
	ri.Printf(PRINT_ALL, "LateKey: scancode %d", scancode);
#endif
	
	switch (scancode)
	{
	case VDK_PAD_9:  *key = K_KP_PGUP; break;
	case VDK_PGUP:   *key = K_PGUP; break;

	case VDK_PAD_3: *key = K_KP_PGDN; break;
	case VDK_PGDN:  *key = K_PGDN; break;

	case VDK_PAD_7: *key = K_KP_HOME; break;
	case VDK_HOME:  *key = K_HOME; break;

	case VDK_PAD_1:   *key = K_KP_END; break;
	case VDK_END:   *key = K_END; break;

	case VDK_PAD_4: *key = K_KP_LEFTARROW; break;
	case VDK_LEFT:  *key = K_LEFTARROW; break;

	case VDK_PAD_6: *key = K_KP_RIGHTARROW; break;
	case VDK_RIGHT:  *key = K_RIGHTARROW;    break;

	case VDK_PAD_2:    *key = K_KP_DOWNARROW; break;
	case VDK_DOWN:  *key = K_DOWNARROW; break;

	case VDK_PAD_8:    *key = K_KP_UPARROW; break;
	case VDK_UP:    *key = K_UPARROW;   break;

	case VDK_ESCAPE: *key = K_ESCAPE;    break;

	case VDK_PAD_ENTER: *key = K_KP_ENTER;  break;
	case VDK_ENTER: *key = K_ENTER;    break;

	case VDK_TAB:    *key = K_TAB;      break;

	case VDK_F1:    *key = K_F1;       break;

	case VDK_F2:    *key = K_F2;       break;

	case VDK_F3:    *key = K_F3;       break;

	case VDK_F4:    *key = K_F4;       break;

	case VDK_F5:    *key = K_F5;       break;

	case VDK_F6:    *key = K_F6;       break;

	case VDK_F7:    *key = K_F7;       break;

	case VDK_F8:    *key = K_F8;       break;

	case VDK_F9:    *key = K_F9;       break;

	case VDK_F10:    *key = K_F10;      break;

	case VDK_F11:    *key = K_F11;      break;

	case VDK_F12:    *key = K_F12;      break;

		// bk001206 - from Ryan's Fakk2 
		//case VDK_BackSpace: *key = 8; break; // ctrl-h
	case VDK_BACKSPACE: *key = K_BACKSPACE; break; // ctrl-h

	case VDK_PAD_PERIOD: *key = K_KP_DEL; break;

	case VDK_DELETE: *key = K_DEL; break; 
	case VDK_BREAK:  *key = K_PAUSE;    break;

	case VDK_LSHIFT:
	case VDK_RSHIFT:  *key = K_SHIFT;   break;

	case VDK_LCTRL: 
	case VDK_RCTRL:  *key = K_CTRL;  break;

	case VDK_LALT:  
	case VDK_RALT: *key = K_ALT;     break;

	case VDK_PAD_5: *key = K_KP_5;  break;

	case VDK_INSERT:   *key = K_INS; break;
	case VDK_PAD_0: *key = K_KP_INS; break;

	case VDK_PAD_ASTERISK: *key = '*'; break;
	case VDK_PAD_PLUS:  *key = K_KP_PLUS; break;
	case VDK_PAD_HYPHEN: *key = K_KP_MINUS; break;
	case VDK_PAD_SLASH: *key = K_KP_SLASH; break;

		// bk001130 - from cvs1.17 (mkv)
	case VDK_1: *key = '1'; break;
	case VDK_2: *key = '2'; break;
	case VDK_3: *key = '3'; break;
	case VDK_4: *key = '4'; break;
	case VDK_5: *key = '5'; break;
	case VDK_6: *key = '6'; break;
	case VDK_7: *key = '7'; break;
	case VDK_8: *key = '8'; break;
	case VDK_9: *key = '9'; break;
	case VDK_0: *key = '0'; break;
	
	// weird french keyboards ..
	// NOTE: console toggle is hardcoded in cl_keys.c, can't be unbound
	//   cleaner would be .. using hardware key codes instead of the key syms
	//   could also add a new K_KP_CONSOLE
	case VDK_SINGLEQUOTE: *key = '~'; break;
		
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=472
	case VDK_SPACE: *key = K_SPACE; break;

	default:
		if (scancode == 0)
		{
			if (com_developer->value)
			{
				ri.Printf(PRINT_ALL, "Warning: scancode: 0\n");
			}
			return NULL;
		}
		else
		{
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

int Sys_Milliseconds();
static void HandleEvents(void)
{
	int b;
	int key;
	vdkEvent event;
	qboolean dowarp = qfalse;
	char *p;
	int dx, dy;
	int t = 0; // default to 0 in case we don't set
	
	if (!display)
		return;

	while (vdkGetEvent(window, &event))
	{
		switch (event.type)
		{
		case VDK_KEYBOARD:
			if (event.data.keyboard.pressed)
			{
				t = Sys_Milliseconds();
				p = LateKey(event.data.keyboard.scancode, &key);
				if (key)
				{
					Sys_QueEvent( t, SE_KEY, key, qtrue, 0, NULL );
				}
				if (p)
				{
					while (*p)
					{
						Sys_QueEvent( t, SE_CHAR, *p++, 0, 0, NULL );
					}
				}
				break;
			}
			else
			{
				t = Sys_Milliseconds();
				// bk001206 - handle key repeat w/o XAutRepatOn/Off
				//            also: not done if console/menu is active.
				// From Ryan's Fakk2.
				// see game/q_shared.h, KEYCATCH_* . 0 == in 3d game.
				// TODO: need add code to avoid auto key press
				// zongzong.yan
			 // if (cls.keyCatchers == 0)
			 // {   // FIXME: KEYCATCH_NONE
			 //   if (repeated_press(&event) == qtrue)
			 //     continue;
			 // } // if
				LateKey(event.data.keyboard.scancode, &key);

				Sys_QueEvent( t, SE_KEY, key, qfalse, 0, NULL );
				break;
			}
		case VDK_POINTER:
			t = Sys_Milliseconds();
			if (mouse_active)
			{
				if (in_dgamouse->value)
				{
					if (abs(event.data.pointer.x) > 1)
						mx += event.data.pointer.x * 2;
					else
						mx += event.data.pointer.x;
					if (abs(event.data.pointer.y) > 1)
						my += event.data.pointer.y * 2;
					else
						my += event.data.pointer.y;
					if (t - mouseResetTime > MOUSE_RESET_DELAY)
					{
						Sys_QueEvent(t, SE_MOUSE, mx, my, 0, NULL);
					}
					mx = my = 0;
				} else
				{
					// If it's a center motion, we've just returned from our warp
					if (event.data.pointer.x == glConfig.vidWidth/2 &&
						  event.data.pointer.y == glConfig.vidHeight/2)
					{
						mwx = glConfig.vidWidth / 2;
						mwy = glConfig.vidHeight / 2;
						mx = my = 0;
						break;
					}

					dx = ((int)event.data.pointer.x - mwx);
					dy = ((int)event.data.pointer.y - mwy);
					if (abs(dx) > 1)
						mx += dx * 2;
					else
						mx += dx;
					if (abs(dy) > 1)
						my += dy * 2;
					else
						my += dy;

					mwx = event.data.pointer.x;
					mwy = event.data.pointer.y;
					dowarp = qtrue;

					if (t - mouseResetTime > MOUSE_RESET_DELAY)
					{
						Sys_QueEvent(t, SE_MOUSE, mx, my, 0, NULL);
						mx = my = 0;
					}
				}
			}
			break;

		case VDK_BUTTON:
			t = Sys_Milliseconds();
			{
				// NOTE TTimo there seems to be a weird mapping for K_MOUSE1 K_MOUSE2 K_MOUSE3 ..
				static int left;
				static int right;
				static int middle;

				if (left ^ event.data.button.left)
				{
					left = event.data.button.left;
					Sys_QueEvent(t, SE_KEY, K_MOUSE1 + 0, left != 0, 0, NULL);
				}

				if (right ^ event.data.button.right)
				{
					right = event.data.button.right;
					Sys_QueEvent(t, SE_KEY, K_MOUSE1 + 1, right != 0, 0, NULL);
				}

				if (middle ^ event.data.button.middle)
				{
					middle = event.data.button.middle;
					Sys_QueEvent(t, SE_KEY, K_MOUSE1 + 2, middle != 0, 0, NULL);
				}
			}
			break;

		default:
			break;
		}
	}

	if (dowarp)
	{
#if 0
		XWarpPointer(dpy,None,win,0,0,0,0, 
						     (glConfig.vidWidth/2),(glConfig.vidHeight/2));
#endif
	}
}

// NOTE TTimo for the tty console input, we didn't rely on those .. 
//   it's not very surprising actually cause they are not used otherwise
void KBD_Init(void)
{
}

void KBD_Close(void)
{
}

void IN_ActivateMouse( void ) 
{
	if (!mouse_avail || !display || !window)
		return;

	if (!mouse_active)
	{
// TODO: comment this first
// zongzong.yan
#if 0
		if (!in_nograb->value)
			install_grabs();
		else if (in_dgamouse->value) // force dga mouse to -1 if using nograb
#endif
			ri.Cvar_Set("in_dgamouse", "0");
		mouse_active = qtrue;
	}
}

void IN_DeactivateMouse( void ) 
{
	if (!mouse_avail || !display || !window)
		return;

	if (mouse_active)
	{
// TODO: comment this first
// zongzong.yan
#if 0
		if (!in_nograb->value)
			uninstall_grabs();
		else if (in_dgamouse->value) // force dga mouse to 0 if using nograb
#endif
		ri.Cvar_Set("in_dgamouse", "0");
		mouse_active = qfalse;
	}
}
/*****************************************************************************/

/*
** GLimp_SetGamma
**
** This routine should only be called if glConfig.deviceSupportsGamma is TRUE
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
{
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GLimp_Shutdown( void )
{
	if (!eglContext || !display)
		return;
	IN_DeactivateMouse();
	// bk001206 - replaced with H2/Fakk2 solution
	// XAutoRepeatOn(dpy);
	// autorepeaton = qfalse; // bk001130 - from cvs1.17 (mkv)
	if (display)
	{
		if (eglContext)
	{
			eglMakeCurrent(eglDisplay, NULL, NULL, NULL);
			eglDestroyContext(eglDisplay, eglContext);
			eglDestroySurface(eglDisplay, eglSurface);
			eglTerminate(eglDisplay);
	}
		if (window)
		{
			vdkDestroyWindow(window);
			vdkDestroyDisplay(display);
			vdkExit(vdk);
		}
	}

	display = NULL;
	window  = NULL;
	eglContext = NULL;
	eglSurface = NULL;

	memset( &glConfig, 0, sizeof( glConfig ) );
	memset( &glState, 0, sizeof( glState ) );
}

/*
** GLimp_LogComment
*/
void GLimp_LogComment( char *comment ) 
{
	if ( glw_state.log_fp )
	{
		fprintf( glw_state.log_fp, "%s", comment );
	}
}

/*
** GLW_StartDriverAndSetMode
*/
// bk001204 - prototype needed
static int GLW_SetMode( const char *drivername, int mode, qboolean fullscreen );
static qboolean GLW_StartDriverAndSetMode( const char *drivername, 
						                               int mode, 
						                               qboolean fullscreen )
{
	rserr_t err;

	if (fullscreen && in_nograb->value)
	{
		ri.Printf( PRINT_ALL, "Fullscreen not allowed with in_nograb 1\n");
			ri.Cvar_Set( "r_fullscreen", "0" );
			r_fullscreen->modified = qfalse;
			fullscreen = qfalse;		
	}

	err = GLW_SetMode( drivername, mode, fullscreen );

	switch ( err )
	{
	case RSERR_INVALID_FULLSCREEN:
		ri.Printf( PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n" );
		return qfalse;
	case RSERR_INVALID_MODE:
		ri.Printf( PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode );
		return qfalse;
	default:
		break;
	}
	return qtrue;
}

/*
** GLW_SetMode
*/
int GLW_SetMode( const char *drivername, int mode, qboolean fullscreen )
{
	EGLConfig configs[10];
	EGLConfig config;
	EGLint matchingConfigs; 
	EGLint attrib[] = {
		EGL_RED_SIZE,    5,
		EGL_GREEN_SIZE,  6,
		EGL_BLUE_SIZE,   5,
		EGL_DEPTH_SIZE,  16,
		EGL_STENCIL_SIZE, 16,
		EGL_NONE
	};

	int colorbits, depthbits, stencilbits;
	int tcolorbits, tdepthbits, tstencilbits;
	int actualWidth, actualHeight;
	int i;

	ri.Printf( PRINT_ALL, "Initializing OpenGL display\n");

	ri.Printf (PRINT_ALL, "...setting mode %d:", mode );

	if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode ) )
	{
		ri.Printf( PRINT_ALL, " invalid mode\n" );
		return RSERR_INVALID_MODE;
	}
	ri.Printf( PRINT_ALL, " %d %d\n", glConfig.vidWidth, glConfig.vidHeight);

	actualWidth = glConfig.vidWidth;
	actualHeight = glConfig.vidHeight;

	if (!r_colorbits->value)
		colorbits = 24;
	else
		colorbits = r_colorbits->value;

	if ( !Q_stricmp( r_glDriver->string, _3DFX_DRIVER_NAME ) )
		colorbits = 16;

	if (!r_depthbits->value)
		depthbits = 16;
	else
		depthbits = r_depthbits->value;
	stencilbits = r_stencilbits->value;

	for (i = 0; i < 16; i++)
	{
		// 0 - default
		// 1 - minus colorbits
		// 2 - minus depthbits
		// 3 - minus stencil
		if ((i % 4) == 0 && i)
		{
			// one pass, reduce
			switch (i / 4)
			{
			case 2 :
				if (colorbits == 24)
					colorbits = 16;
				break;
			case 1 :
				if (depthbits == 24)
					depthbits = 16;
				else if (depthbits == 16)
					depthbits = 8;
			case 3 :
				if (stencilbits == 24)
					stencilbits = 16;
				else if (stencilbits == 16)
					stencilbits = 8;
			}
		}

		tcolorbits = colorbits;
		tdepthbits = depthbits;
		tstencilbits = stencilbits;

		if ((i % 4) == 3)
		{ // reduce colorbits
			if (tcolorbits == 24)
				tcolorbits = 16;
		}

		if ((i % 4) == 2)
		{ // reduce depthbits
			if (tdepthbits == 24)
				tdepthbits = 16;
			else if (tdepthbits == 16)
				tdepthbits = 8;
		}

		if ((i % 4) == 1)
		{ // reduce stencilbits
			if (tstencilbits == 24)
				tstencilbits = 16;
			else if (tstencilbits == 16)
				tstencilbits = 8;
			else
				tstencilbits = 0;
		}

		if (tcolorbits == 24)
		{
			attrib[1] = 8;
			attrib[3] = 8;
			attrib[5] = 8;
		} else
		{
			// must be 16 bit
			attrib[1] = 5;
			attrib[3] = 6;
			attrib[5] = 5;
		}

		attrib[7] = tdepthbits; // default to 24 depth
		attrib[9] = tstencilbits;

		glConfig.colorBits = tcolorbits;
		glConfig.depthBits = tdepthbits;
		glConfig.stencilBits = tstencilbits;
		break;
	}

	vdk = vdkInitialize();
	display = vdkGetDisplay(vdk);
	window = vdkCreateWindow(display, 0, 0, glConfig.vidWidth, glConfig.vidHeight);

	eglDisplay = eglGetDisplay(display);
	eglInitialize(eglDisplay, NULL, NULL);

	if (eglChooseConfig(eglDisplay, attrib, &(configs[0]), 10, &matchingConfigs) == EGL_FALSE)
	{
		return qfalse;
	}
	if (matchingConfigs < 1)
	{
		return qfalse;
	}

	config = configs[0];

	eglSurface = eglCreateWindowSurface(eglDisplay, config, window, NULL);
	if (eglSurface == NULL)
	{
		return qfalse;
	}

	eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, NULL);
	if (eglContext == NULL)
	{
		return qfalse;
	}

	if (eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) != EGL_TRUE)
	{
		return qfalse;
	}

	vdkShowWindow(window);

	return RSERR_OK;
}

/*
** GLW_InitExtensions
*/
static void GLW_InitExtensions( void )
{
	if ( !r_allowExtensions->integer )
	{
		ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	glConfig.textureCompression = TC_NONE;
	glConfig.textureEnvAddAvailable = qfalse;
}

static void GLW_InitGamma()
{
}

static qboolean GLW_LoadOpenGL( const char *name )
{
	qboolean fullscreen;
	ri.Printf( PRINT_ALL, "...loading %s: ", name );

	// disable the 3Dfx splash screen and set gamma
	// we do this all the time, but it shouldn't hurt anything
	// on non-3Dfx stuff
	putenv("FX_GLIDE_NO_SPLASH=0");

	// Mesa VooDoo hacks
	putenv("MESA_GLX_FX=fullscreen\n");

	// load the QGL layer
	//  if ( QGL_Init( name ) )
	{
		fullscreen = r_fullscreen->integer;

		// create the window and set up the context
		if ( !GLW_StartDriverAndSetMode( name, r_mode->integer, fullscreen ) )
		{
			if (r_mode->integer != 3)
			{
				if ( !GLW_StartDriverAndSetMode( name, 3, fullscreen ) )
				{
					goto fail;
				}
			} else
				goto fail;
		}

		return qtrue;
	}
	fail:

	return qfalse;
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  
*/
void GLimp_Init( void )
{
	qboolean success = qfalse;
	char  buf[1024];
	cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );

	r_allowSoftwareGL = ri.Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );

	r_previousglDriver = ri.Cvar_Get( "r_previousglDriver", "", CVAR_ROM );

	InitSig();

	// Hack here so that if the UI 
	if ( *r_previousglDriver->string )
	{
		// The UI changed it on us, hack it back
		// This means the renderer can't be changed on the fly
		ri.Cvar_Set( "r_glDriver", r_previousglDriver->string );
	}
	
	//
	// load and initialize the specific OpenGL driver
	//
	if ( !GLW_LoadOpenGL( r_glDriver->string ) )
	{
		return;
	}

	// Save it in case the UI stomps it
	ri.Cvar_Set( "r_previousglDriver", r_glDriver->string );

	// This values force the UI to disable driver selection
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;

	// get our config strings
	const char* str = qglGetString(GL_VENDOR);
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

	//
	// this is where hardware specific workarounds that should be
	// detected/initialized every startup should go.
	//
	if ( Q_stristr( buf, "banshee" ) || Q_stristr( buf, "Voodoo_Graphics" ) )
	{
		glConfig.hardwareType = GLHW_3DFX_2D3D;
	} else if ( Q_stristr( buf, "rage pro" ) || Q_stristr( buf, "RagePro" ) )
	{
		glConfig.hardwareType = GLHW_RAGEPRO;
	} else if ( Q_stristr( buf, "permedia2" ) )
	{
		glConfig.hardwareType = GLHW_PERMEDIA2;
	} else if ( Q_stristr( buf, "riva 128" ) )
	{
		glConfig.hardwareType = GLHW_RIVA128;
	} else if ( Q_stristr( buf, "riva tnt " ) )
	{
	}

	ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );

	// initialize extensions
	GLW_InitExtensions();
	GLW_InitGamma();

	InitSig(); // not clear why this is at begin & end of function

	return;
}


/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
	// don't flip if drawing to front buffer
	if ( stricmp( r_drawBuffer->string, "GL_FRONT" ) != 0 )
	{
		eglSwapBuffers(eglDisplay, eglSurface);
	}
}

void GLimp_RenderThreadWrapper( void *stub ) {}
qboolean GLimp_SpawnRenderThread( void (*function)( void ) ) {
	ri.Printf( PRINT_WARNING, "ERROR: SMP support was disabled at compile time\n");
	return qfalse;
}
void *GLimp_RendererSleep( void ) {
	return NULL;
}
void GLimp_FrontEndSleep( void ) {}
void GLimp_WakeRenderer( void *data ) {}

/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/

void IN_Init(void) {
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

void IN_Shutdown(void)
{
	mouse_avail = qfalse;
}

void IN_Frame (void) {

	if ( cls.keyCatchers & KEYCATCH_CONSOLE )
	{
		// temporarily deactivate if not in the game and
		// running on the desktop
		// voodoo always counts as full screen
		if (Cvar_VariableValue ("r_fullscreen") == 0
				&& strcmp( Cvar_VariableString("r_glDriver"), _3DFX_DRIVER_NAME ) )
		{
			IN_DeactivateMouse ();
			return;
		}
	}

	IN_ActivateMouse();
}

void IN_Activate(void)
{
}

void Sys_SendKeyEvents (void) {
	// XEvent event; // bk001204 - unused

	if (!display)
		return;
	HandleEvents();
}

