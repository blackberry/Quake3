#ifdef USE_OPENGL_ES_1_1
#pragma once

#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif

#undef FIXED
#define FIXED_IS_FLOAT 1

#ifdef FIXED
#include"../qcommon/fixed.h"
#endif

#if defined(RIM_NDK) || defined(__QNXNTO__)
#include <egl.h>
#include <gl.h>
#else
#include <EGL/egl.h>
#include <GLES/gl.h>
#endif

#define GLFLOAT_1 (1.0f)
#define GLFLOAT_0 (0.0f)
#define GLFIXED_1 (0x00010000)
#define GLFIXED_0 (0x00000000)

#ifdef FIXED_IS_FLOAT
#define GL_FIXED_OR_FLOAT GL_FLOAT
#else
#define GL_FIXED_OR_FLOAT GL_FIXED
#endif

union GLNUMBER
{
	GLfloat f;
	GLfixed x;
};

#define GW_BUFFERSIZE (16384)

//#define GL_POINTS                         0x0000
//#define GL_LINES                          0x0001
//#define GL_LINE_LOOP                      0x0002
//#define GL_LINE_STRIP                     0x0003
//#define GL_TRIANGLES                      0x0004
//#define GL_TRIANGLE_STRIP                 0x0005
//#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_QUAD_STRIP                     0x0008
#define GL_POLYGON                        0x0009

//////////////////////////////////////////////////////////////////////////////

extern GLfixed g_btfvals[256];

extern GLenum g_curmode;
extern int g_texfixedmode;
extern int g_vtxfixedmode;
extern union GLNUMBER g_vtxbuf[GW_BUFFERSIZE];
extern union GLNUMBER g_texbuf[GW_BUFFERSIZE];
//extern GLNUMBER g_texbuf2[GW_BUFFERSIZE];
extern unsigned short g_indices[GW_BUFFERSIZE];
extern int g_vtxsize;
extern int g_texsize;
extern int g_vtxlen;
extern int g_texlen;
extern int g_vtxmod3;
extern int g_texmod3;
extern int g_idxsize;

extern int g_glbegincount;

//////////////////////////////////////////////////////////////////////////////

void GL_APIENTRY glBegin(GLenum mode);
void GL_APIENTRY glEnd(void);

void GL_APIENTRY glTexCoord2f (GLfloat s, GLfloat t);
void GL_APIENTRY glVertex2f (GLfloat x, GLfloat y);
void GL_APIENTRY glVertex3i (GLfixed x, GLfixed y, GLfixed z);
void GL_APIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z);

static __inline void GL_APIENTRY pushtexfixed(GLfixed x)
{
//	ASSERT(g_texfixedmode==-1 || g_texfixedmode==1);
	g_texfixedmode=1;

	g_texbuf[g_texlen++].x=x;
//	ASSERT(g_texlen<=GW_BUFFERSIZE);
}

static __inline void GL_APIENTRY pushvtxfixed(GLfixed x)
{
//	ASSERT(g_vtxfixedmode==-1 || g_vtxfixedmode==1);
	g_vtxfixedmode=1;

	g_vtxbuf[g_vtxlen++].x=x;
//	ASSERT(g_vtxlen<=GW_BUFFERSIZE);
}

static __inline void GL_APIENTRY pushtexfloat(GLfloat f)
{
	// ASSERT(g_texfixedmode==-1 || g_texfixedmode==0);
	g_texfixedmode=0;

	g_texbuf[g_texlen++].f=f;
	// ASSERT(g_texlen<=GW_BUFFERSIZE);
}

static __inline void GL_APIENTRY pushvtxfloat(GLfloat f)
{
	// ASSERT(g_vtxfixedmode==-1 || g_vtxfixedmode==0);
	g_vtxfixedmode=0;

	g_vtxbuf[g_vtxlen++].f=f;
	// ASSERT(g_vtxlen<=GW_BUFFERSIZE);
}


static __inline void GL_APIENTRY pushindex(unsigned short idx)
{
	g_indices[g_idxsize++]=idx;
	// ASSERT(g_idxsize<=GW_BUFFERSIZE);
}

static __inline void GL_APIENTRY glTexCoord2fv (const GLfloat *v)
{
	glTexCoord2f(v[0],v[1]);
}

static __inline void GL_APIENTRY glVertex3fv (const GLfloat *v)
{
	glVertex3f(v[0],v[1],v[2]);
}

static __inline void GL_APIENTRY glColor4ubv( const GLubyte *v)
{
	glColor4x(g_btfvals[(unsigned int)v[0]],g_btfvals[(unsigned int)v[1]],g_btfvals[(unsigned int)v[2]],g_btfvals[(unsigned int)v[3]]);
}

static __inline void GL_APIENTRY glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
	glColor4f(r,g,b,GLFLOAT_1);
}

static __inline void GL_APIENTRY qglColor3fv(const GLfloat *v)
{
	glColor4f(v[0], v[1], v[2], GLFLOAT_1);
}

static __inline void GL_APIENTRY glArrayElement(GLint idx)
{
	pushindex(idx);
}
#endif // USE_OPENGL_ES_1_1

