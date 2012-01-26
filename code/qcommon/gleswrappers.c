#ifdef USE_OPENGL_ES_1_1
#include"gleswrappers.h"


#define GW_BUFFERSIZE (16384)

////////////////////////////////////////////////////////////////////////////////////////////////


GLenum g_curmode=-1;
int g_texfixedmode=-1;
int g_vtxfixedmode=-1;
GLfixed g_btfvals[256];
union GLNUMBER g_vtxbuf[GW_BUFFERSIZE];
union GLNUMBER g_texbuf[GW_BUFFERSIZE];
//GLNUMBER g_texbuf2[GW_BUFFERSIZE];
unsigned short g_indices[GW_BUFFERSIZE];
int g_vtxsize=0;
int g_texsize=0;
int g_vtxlen=0;
int g_texlen=0;
int g_vtxmod3=0;
int g_texmod3=0;
int g_idxsize=0;

//#ifdef TARGET_AXIMX50V
//void (*pClipPlanexIMG)( GLenum p, const GLfixed *eqn )=(TYPEOF_pClipPlanexIMG)NULL;
//#endif

int g_glbegincount=0;


////////////////////////////////////////////////////////////////////////////////////////////////

void GL_APIENTRY glBegin(GLenum mode)
{
	g_glbegincount++;
//	ASSERT(g_glbegincount!=99);

	g_curmode=mode;
	g_texfixedmode=-1;
	g_vtxfixedmode=-1;
	g_vtxsize=0;
	g_texsize=0;
	g_vtxlen=0;
	g_texlen=0;
	g_vtxmod3=0;
	g_texmod3=0;
	g_idxsize=0;

}

void GL_APIENTRY glEnd(void)
{
	if(g_idxsize>0)
	{

		switch(g_curmode)
		{
		case GL_POINTS:
			glDrawElements(GL_POINTS,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		case GL_LINES:
			glDrawElements(GL_LINES,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		case GL_LINE_LOOP:
			glDrawElements(GL_LINE_LOOP,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		case GL_LINE_STRIP:
			glDrawElements(GL_LINE_STRIP,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		case GL_TRIANGLES:
			glDrawElements(GL_TRIANGLES,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		case GL_TRIANGLE_STRIP:
			glDrawElements(GL_TRIANGLE_STRIP,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		case GL_TRIANGLE_FAN:
			glDrawElements(GL_TRIANGLE_FAN,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		case GL_QUADS:
			glDrawElements(GL_TRIANGLES,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		case GL_QUAD_STRIP:
			glDrawElements(GL_TRIANGLE_STRIP,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		case GL_POLYGON:
			glDrawElements(GL_TRIANGLE_FAN,g_idxsize,GL_UNSIGNED_SHORT,g_indices);
			break;
		}	
	}
	else
	{
		int numelems=0;

		if(g_vtxsize>0)
		{
			// ASSERT(g_vtxfixedmode!=-1);
			glVertexPointer(g_vtxsize,(g_vtxfixedmode==0)?GL_FLOAT:GL_FIXED,sizeof(union GLNUMBER),g_vtxbuf);
			if(g_vtxsize>numelems)
			{
				numelems=g_vtxsize;
			}
		}
		if(g_texsize>0)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			// ASSERT(g_texfixedmode!=-1);
			glTexCoordPointer(g_texsize,(g_texfixedmode==0)?GL_FLOAT:GL_FIXED,sizeof(union GLNUMBER),g_texbuf);
			if(g_texsize>numelems)
			{
				numelems=g_texsize;
			}
		}
		else
		{
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		switch(g_curmode)
		{
		case GL_POINTS:
			glDrawArrays(GL_POINTS,0,numelems);
			break;
		case GL_LINES:
			glDrawArrays(GL_LINES,0,numelems);
			break;
		case GL_LINE_LOOP:
			glDrawArrays(GL_LINE_LOOP,0,numelems);
			break;
		case GL_LINE_STRIP:
			glDrawArrays(GL_LINE_STRIP,0,numelems);
			break;
		case GL_TRIANGLES:
			glDrawArrays(GL_TRIANGLES,0,numelems);
			break;
		case GL_TRIANGLE_STRIP:
			glDrawArrays(GL_TRIANGLE_STRIP,0,numelems);
			break;
		case GL_TRIANGLE_FAN:
			glDrawArrays(GL_TRIANGLE_FAN,0,numelems);
			break;
		case GL_QUADS:
			glDrawArrays(GL_TRIANGLES,0,numelems);
			break;
		case GL_QUAD_STRIP:
			glDrawArrays(GL_TRIANGLE_STRIP,0,numelems);
			break;
		case GL_POLYGON:
			glDrawArrays(GL_TRIANGLE_FAN,0,numelems);
			break;
		}
	}

	g_curmode=-1;
}


void glTexCoord2x (GLfixed s, GLfixed t)
{
	if(g_texsize==0)
	{
		g_texsize=2;
	}
	else if(g_texsize!=2)
	{
		// ASSERT(0);
		return;
	}
	
	if(g_curmode==GL_QUADS)
	{
		if(g_texmod3==2)
		{
			GLfixed s0, s1, t0, t1;
			pushtexfixed(s);
			pushtexfixed(t);

			s0=g_texbuf[g_texlen-g_texsize*3].x;
			t0=g_texbuf[g_texlen-g_texsize*3+1].x;
			s1=g_texbuf[g_texlen-g_texsize*1].x;
			t1=g_texbuf[g_texlen-g_texsize*1+1].x;
			
			pushtexfixed(s0);
			pushtexfixed(t0);
			pushtexfixed(s1);
			pushtexfixed(t1);
			pushtexfixed(s);
			pushtexfixed(t);

		}
		else
		{
			pushtexfixed(s);
			pushtexfixed(t);
		}
	}
	else
	{
		pushtexfixed(s);
		pushtexfixed(t);
	}

	g_texmod3++;
	if(g_texmod3==3)
	{
		g_texmod3=0;
	}
}


void GL_APIENTRY glTexCoord2f (GLfloat s, GLfloat t)
{
	if(g_texsize==0)
	{
		g_texsize=2;
	}
	else if(g_texsize!=2)
	{
		// ASSERT(0);
		return;
	}
	
	if(g_curmode==GL_QUADS)
	{
		if(g_texmod3==2)
		{
			GLfloat s0, s1, t0, t1;

			pushtexfloat(s);
			pushtexfloat(t);

			s0 =g_texbuf[g_texlen-g_texsize*3].f;
			t0=g_texbuf[g_texlen-g_texsize*3+1].f;
			s1=g_texbuf[g_texlen-g_texsize*1].f;
			t1=g_texbuf[g_texlen-g_texsize*1+1].f;
			
			pushtexfloat(s0);
			pushtexfloat(t0);
			pushtexfloat(s1);
			pushtexfloat(t1);
			pushtexfloat(s);
			pushtexfloat(t);

		}
		else
		{
			pushtexfloat(s);
			pushtexfloat(t);
		}
	}
	else
	{
		pushtexfloat(s);
		pushtexfloat(t);
	}

	g_texmod3++;
	if(g_texmod3==3)
	{
		g_texmod3=0;
	}
}

void GL_APIENTRY glVertex2x (GLfixed x, GLfixed y)
{
	if(g_vtxsize==0)
	{
		g_vtxsize=2;
	}
	else if(g_vtxsize!=2)
	{
		// ASSERT(0);
		return;
	}
	
	if(g_curmode==GL_QUADS)
	{
		if(g_vtxmod3==2)
		{
			GLfixed x0, x1, y0, y1;

			pushvtxfixed(x);
			pushvtxfixed(y);

			x0=g_vtxbuf[g_vtxlen-g_vtxsize*3].x;
			y0=g_vtxbuf[g_vtxlen-g_vtxsize*3+1].x;
			x1=g_vtxbuf[g_vtxlen-g_vtxsize*1].x;
			y1=g_vtxbuf[g_vtxlen-g_vtxsize*1+1].x;
			
			pushvtxfixed(x0);
			pushvtxfixed(y0);
			pushvtxfixed(x1);
			pushvtxfixed(y1);
			pushvtxfixed(x);
			pushvtxfixed(y);

		}
		else
		{
			pushvtxfixed(x);
			pushvtxfixed(y);
		}
	}
	else
	{
		pushvtxfixed(x);
		pushvtxfixed(y);
	}

	g_vtxmod3++;
	if(g_vtxmod3==3)
	{
		g_vtxmod3=0;
	}
}

void GL_APIENTRY glVertex2f (GLfloat x, GLfloat y)
{
	if(g_vtxsize==0)
	{
		g_vtxsize=2;
	}
	else if(g_vtxsize!=2)
	{
		// ASSERT(0);
		return;
	}
	
	if(g_curmode==GL_QUADS)
	{
		if(g_vtxmod3==2)
		{
			GLfloat x0, x1, y0, y1;

			pushvtxfloat(x);
			pushvtxfloat(y);

			x0=g_vtxbuf[g_vtxlen-g_vtxsize*3].f;
			y0=g_vtxbuf[g_vtxlen-g_vtxsize*3+1].f;
			x1=g_vtxbuf[g_vtxlen-g_vtxsize*1].f;
			y1=g_vtxbuf[g_vtxlen-g_vtxsize*1+1].f;
			
			pushvtxfloat(x0);
			pushvtxfloat(y0);
			pushvtxfloat(x1);
			pushvtxfloat(y1);
			pushvtxfloat(x);
			pushvtxfloat(y);

		}
		else
		{
			pushvtxfloat(x);
			pushvtxfloat(y);
		}
	}
	else
	{
		pushvtxfloat(x);
		pushvtxfloat(y);
	}

	g_vtxmod3++;
	if(g_vtxmod3==3)
	{
		g_vtxmod3=0;
	}
}



void GL_APIENTRY glVertex3i (GLfixed x, GLfixed y, GLfixed z)
{
	if(g_vtxsize==0)
	{
		g_vtxsize=3;
	}
	else if(g_vtxsize!=3)
	{
		// ASSERT(0);
		return;
	}
	
	if(g_curmode==GL_QUADS)
	{
		if(g_vtxmod3==2)
		{
			GLfixed x0, x1, y0, y1, z0, z1;

			pushvtxfixed(x);
			pushvtxfixed(y);
			pushvtxfixed(z);

			x0=g_vtxbuf[g_vtxlen-g_vtxsize*3].x;
			y0=g_vtxbuf[g_vtxlen-g_vtxsize*3+1].x;
			z0=g_vtxbuf[g_vtxlen-g_vtxsize*3+2].x;
			x1=g_vtxbuf[g_vtxlen-g_vtxsize*1].x;
			y1=g_vtxbuf[g_vtxlen-g_vtxsize*1+1].x;
			z1=g_vtxbuf[g_vtxlen-g_vtxsize*1+2].x;
			
			pushvtxfixed(x0);
			pushvtxfixed(y0);
			pushvtxfixed(z0);
			pushvtxfixed(x1);
			pushvtxfixed(y1);
			pushvtxfixed(z1);
			pushvtxfixed(x);
			pushvtxfixed(y);
			pushvtxfixed(z);
		}
		else
		{
			pushvtxfixed(x);
			pushvtxfixed(y);
			pushvtxfixed(z);
		}
	}
	else
	{
		pushvtxfixed(x);
		pushvtxfixed(y);
		pushvtxfixed(z);
	}

	g_vtxmod3++;
	if(g_vtxmod3==3)
	{
		g_vtxmod3=0;
	}
}

void GL_APIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z)
{
	if(g_vtxsize==0)
	{
		g_vtxsize=3;
	}
	else if(g_vtxsize!=3)
	{
		// ASSERT(0);
		return;
	}
	
	if(g_curmode==GL_QUADS)
	{
		if(g_vtxmod3==2)
		{
			GLfloat x0, x1, y0, y1, z0, z1;

			pushvtxfloat(x);
			pushvtxfloat(y);
			pushvtxfloat(z);

			x0=g_vtxbuf[g_vtxlen-g_vtxsize*3].f;
			y0=g_vtxbuf[g_vtxlen-g_vtxsize*3+1].f;
			z0=g_vtxbuf[g_vtxlen-g_vtxsize*3+2].f;
			x1=g_vtxbuf[g_vtxlen-g_vtxsize*1].f;
			y1=g_vtxbuf[g_vtxlen-g_vtxsize*1+1].f;
			z1=g_vtxbuf[g_vtxlen-g_vtxsize*1+2].f;
			
			pushvtxfloat(x0);
			pushvtxfloat(y0);
			pushvtxfloat(z0);
			pushvtxfloat(x1);
			pushvtxfloat(y1);
			pushvtxfloat(z1);
			pushvtxfloat(x);
			pushvtxfloat(y);
			pushvtxfloat(z);
		}
		else
		{
			pushvtxfloat(x);
			pushvtxfloat(y);
			pushvtxfloat(z);
		}
	}
	else
	{
		pushvtxfloat(x);
		pushvtxfloat(y);
		pushvtxfloat(z);
	}

	g_vtxmod3++;
	if(g_vtxmod3==3)
	{
		g_vtxmod3=0;
	}
}
#endif // USE_OPENGL_ES_1_1

