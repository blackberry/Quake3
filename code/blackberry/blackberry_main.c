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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include <unistd.h>

#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

sysEvent_t  eventQue[MAX_QUED_EVENTS];
byte        sys_packetReceived[MAX_MSGLEN];
int         eventHead = 0;
int         eventTail = 0;

int			sys_curtime;

static qboolean ttycon_on = qfalse;


void Sys_BeginStreamedFile( fileHandle_t f, int readAhead )
{
}

void Sys_EndStreamedFile( fileHandle_t f )
{
}

int Sys_StreamedRead( void *buffer, int size, int count, fileHandle_t f )
{
  return FS_Read( buffer, size * count, f );
}

void Sys_StreamSeek( fileHandle_t f, int offset, int origin )
{
  FS_Seek( f, offset, origin );
}

void Sys_mkdir ( const char *path )
{
    Com_Printf("Using Sys_mkdir which has no programmed behaviour");
}

void Sys_Error( const char *error, ...)
{
	va_list	argptr;
	printf ("Sys_Error: ");	
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

	exit (1);
}

void Sys_Quit (void)
{
	exit (0);
}

void Sys_UnloadGame (void)
{
}

void* Sys_GetGameAPI (void *parms)
{
	return NULL;
}

char* Sys_GetClipboardData( void )
{
	return NULL;
}

char* Sys_FindFirst (char *path, unsigned musthave, unsigned canthave)
{
	return NULL;
}

char* Sys_FindNext (unsigned musthave, unsigned canthave)
{
	return NULL;
}

void Sys_FindClose ( void )
{
}

void Sys_In_Restart_f( void )
{
    IN_Shutdown();
    IN_Init();
}

void Sys_Init ( void )
{
    Cmd_AddCommand ("in_restart", Sys_In_Restart_f);
    Cvar_Set( "arch", "rim" );
    Cvar_Set( "username", Sys_GetCurrentUser() );
    IN_Init();
}

void Sys_EarlyOutput( char *string )
{
	printf( "%s", string );
}

void Sys_Print( const char *msg )
{
}

char* Sys_ConsoleInput(void)
{
    return NULL;
}

void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr )
{
  sysEvent_t  *ev;

  ev = &eventQue[ eventHead & MASK_QUED_EVENTS ];
  if ( eventHead - eventTail >= MAX_QUED_EVENTS )
  {
    Com_Printf("Sys_QueEvent: overflow\n");
    // we are discarding an event, but don't leak memory
    if ( ev->evPtr )
    {
        Z_Free( ev->evPtr );
    }
    eventTail++;
  }
  eventHead++;

  if ( time == 0 )
  {
      time = Sys_Milliseconds();
  }

  ev->evTime = time;
  ev->evType = type;
  ev->evValue = value;
  ev->evValue2 = value2;
  ev->evPtrLength = ptrLength;
  ev->evPtr = ptr;
}

void Sys_ConsoleInputShutdown()
{
  if (ttycon_on)
  {
    Com_Printf("Shutdown tty console\n");
  }
}

void Sys_Exit( int ex )
{
    Sys_ConsoleInputShutdown();

    // Give me a backtrace on error exits.
    assert( ex == 0 );
    exit(ex);

}

sysEvent_t Sys_GetEvent( void )
{
    sysEvent_t  ev;
    char    *s;
    qmsg_t   netmsg;
    netadr_t  adr;

    // return if we have data
    if ( eventHead > eventTail )
    {
        eventTail++;
        return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
    }

    // Pump the message loop in vga this calls KBD_Update, under X, it calls GetEvent
    Sys_SendKeyEvents ();

    // check for console commands
    s = Sys_ConsoleInput();
    if ( s )
    {
        char  *b;
        int   len;

        len = strlen( s ) + 1;
        b = Z_Malloc( len );
        strcpy( b, s );
        Sys_QueEvent( 0, SE_CONSOLE, 0, 0, len, b );
    }

    // check for other input devices
    IN_Frame();

    // check for network packets
    MSG_Init( &netmsg, sys_packetReceived, sizeof( sys_packetReceived ) );
    if ( Sys_GetPacket ( &adr, &netmsg ) )
    {
        netadr_t    *buf;
        int       len;

        // copy out to a seperate buffer for qeueing
        len = sizeof( netadr_t ) + netmsg.cursize;
        buf = Z_Malloc( len );
        *buf = adr;
        memcpy( buf+1, netmsg.data, netmsg.cursize );
        Sys_QueEvent( 0, SE_PACKET, 0, 0, len, buf );
    }

    // return if we have data
    if ( eventHead > eventTail )
    {
        eventTail++;
        return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
    }

    // create an empty event to return
    memset( &ev, 0, sizeof( ev ) );
    ev.evTime = Sys_Milliseconds();

    return ev;
}

extern int UI_vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  );
extern void UI_dllEntry( int (QDECL *syscallptr)( int arg,... ) );
extern int CG_vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  );
extern void CG_dllEntry( int (QDECL *syscallptr)( int arg,... ) );
extern int G_vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  );
extern void G_dllEntry( int (QDECL *syscallptr)( int arg,... ) );

void* Sys_LoadDll( const char *name, char *fqpath , int (**entryPoint)(int, ...), int (*systemcalls)(int, ...) )
{
    void *libHandle;
    int  tmp=0;
    libHandle = &tmp;

    if(strcmp(name, "ui") == 0)
    {
        *entryPoint = &UI_vmMain;
        UI_dllEntry( systemcalls );
        Q_strncpyz ( fqpath , "daisy_q3ui.lib" , MAX_QPATH );
    }
    else if(strcmp(name, "cgame") == 0)
    {
        *entryPoint = &CG_vmMain;
        CG_dllEntry( systemcalls );
        Q_strncpyz ( fqpath , "daisy_cgame.lib" , MAX_QPATH );
    }
    else if(strcmp(name, "qagame") == 0)
    {
        *entryPoint = &G_vmMain;
        G_dllEntry( systemcalls );
        Q_strncpyz ( fqpath , "daisy_game.lib" , MAX_QPATH );
    }
    else
    {
        Com_Printf("Error, Sys_LoadDll is loading unknown library: '%s'", name);
        exit( 0 );
    }

    // Return arbitary value in order to indicate dll has been 'loaded'
    // The only time this return value is actually used is when called with Sys_UnloadDll
    // which simply ignores libHandle
    return libHandle;
}

void Sys_UnloadDll( void *dllHandle )
{
}

qboolean Sys_LowPhysicalMemory()
{
    return qfalse;
}

void Sys_BeginProfiling( void )
{
}

#ifdef __QNX__
int main(int argc, char **argv)
{
    char cmdline[] = "+set sv_pure 0 +set vm_ui 0 +set vm_game 0 +set vm_cgame 0 +set fs_basepath ./app/native";
	cvar_t* cv = NULL;
	char cmd_rundemo[100];
    Sys_SetDefaultCDPath("./app/native");

    // Clear the queues
    memset( &eventQue[0], 0, MAX_QUED_EVENTS*sizeof(sysEvent_t) );
    memset( &sys_packetReceived[0], 0, MAX_MSGLEN*sizeof(byte) );

    // Initialize game
    Com_Init(cmdline);
    NET_Init();

	while (1)
	{
		Com_Frame();
	}
}
#endif
