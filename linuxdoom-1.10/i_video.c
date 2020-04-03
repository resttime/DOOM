// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for SDL2, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

#include <SDL2/SDL.h>

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *surface;
SDL_Texture *texture;
SDL_Event sdl_event;

int r_width;
int r_height;

boolean grabMouse;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;

//
//  Translates the SDL_Keycode to doom keycode
//

int xlatekey(void)
{

    int rc;

    switch(rc = sdl_event.key.keysym.sym) {
      case SDLK_LEFT:	rc = KEY_LEFTARROW;	break;
      case SDLK_RIGHT:	rc = KEY_RIGHTARROW;	break;
      case SDLK_DOWN:	rc = KEY_DOWNARROW;	break;
      case SDLK_UP:	rc = KEY_UPARROW;	break;
      case SDLK_ESCAPE:	rc = KEY_ESCAPE;	break;
      case SDLK_RETURN:	rc = KEY_ENTER;		break;
      case SDLK_TAB:	rc = KEY_TAB;		break;
      case SDLK_F1:	rc = KEY_F1;		break;
      case SDLK_F2:	rc = KEY_F2;		break;
      case SDLK_F3:	rc = KEY_F3;		break;
      case SDLK_F4:	rc = KEY_F4;		break;
      case SDLK_F5:	rc = KEY_F5;		break;
      case SDLK_F6:	rc = KEY_F6;		break;
      case SDLK_F7:	rc = KEY_F7;		break;
      case SDLK_F8:	rc = KEY_F8;		break;
      case SDLK_F9:	rc = KEY_F9;		break;
      case SDLK_F10:	rc = KEY_F10;		break;
      case SDLK_F11:	rc = KEY_F11;		break;
      case SDLK_F12:	rc = KEY_F12;		break;
	
      case SDLK_BACKSPACE:
      case SDLK_DELETE:	rc = KEY_BACKSPACE;	break;

      case SDLK_PAUSE:	rc = KEY_PAUSE;		break;

      case SDLK_KP_EQUALS:
      case SDLK_EQUALS:	rc = KEY_EQUALS;	break;

      case SDLK_KP_MINUS:
      case SDLK_MINUS:	rc = KEY_MINUS;		break;

      case SDLK_LSHIFT:
      case SDLK_RSHIFT:
	rc = KEY_RSHIFT;
	break;
	
      case SDLK_LCTRL:
      case SDLK_RCTRL:
	rc = KEY_RCTRL;
	break;
	
      case SDLK_LALT:
      case SDLK_LGUI:
      case SDLK_RALT:
      case SDLK_RGUI:
	rc = KEY_RALT;
	break;
	
      default:
	if (rc >= SDLK_SPACE && rc <= SDLK_BACKQUOTE)
	    rc = rc - SDLK_SPACE + ' ';
	if (rc >= 'A' && rc <= 'Z')
	    rc = rc - 'A' + 'a';
	break;
    }

    return rc;

}

void I_ShutdownGraphics(void) {
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

void I_GetEvent(void)
{

    event_t event;

    // put event-grabbing stuff in here
    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
            case SDL_KEYDOWN:
                event.type = ev_keydown;
                event.data1 = xlatekey();
                D_PostEvent(&event);
                // fprintf(stderr, "k");
                break;
            case SDL_KEYUP:
                event.type = ev_keyup;
                event.data1 = xlatekey();
                D_PostEvent(&event);
                // fprintf(stderr, "ku");
                break;
            case SDL_MOUSEBUTTONDOWN:
                event.type = ev_mouse;
                event.data1 =
                    SDL_GetMouseState(NULL, NULL)
                    | (sdl_event.button.button == SDL_BUTTON_LEFT)
                    | (sdl_event.button.button == SDL_BUTTON_MIDDLE ? 2 : 0)
                    | (sdl_event.button.button == SDL_BUTTON_RIGHT ? 4 : 0);
                event.data2 = event.data3 = 0;
                D_PostEvent(&event);
                // fprintf(stderr, "b");
                break;
            case SDL_MOUSEBUTTONUP:
                event.type = ev_mouse;
                event.data1 =
                    SDL_GetMouseState(NULL, NULL);
                // suggest parentheses around arithmetic in operand of |
                event.data1 =
                    event.data1
                    ^ (sdl_event.button.button == SDL_BUTTON_LEFT ? 1 : 0)
                    ^ (sdl_event.button.button == SDL_BUTTON_MIDDLE ? 2 : 0)
                    ^ (sdl_event.button.button == SDL_BUTTON_RIGHT ? 4 : 0);
                event.data2 = event.data3 = 0;
                D_PostEvent(&event);
                // fprintf(stderr, "bu");
                break;
            case SDL_MOUSEMOTION:
                event.type = ev_mouse;
                event.data1 = SDL_GetMouseState(NULL, NULL);
                event.data2 = sdl_event.motion.xrel << 2;
                event.data3 = -sdl_event.motion.yrel << 2;
                if (event.data2 || event.data3) {
                    D_PostEvent(&event);
                    // fprintf(stderr, "m");
                }
                break;
            default:
                // fprintf(stderr, "?");
                break;
        }
    }
}

//
// I_StartTic
//
void I_StartTic (void)
{
    if (!window) return;

	I_GetEvent();
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{

    static int	lasttic;
    int		tics;
    int		i;
    // UNUSED static unsigned char *bigscreen=0;

    // draws little dots on the bottom of the screen
    if (devparm)
    {

	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    
    }

    // scales the screen size before blitting it
    if (multiply == 2)
    {
	unsigned int *olineptrs[2];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int twoopixels;
	unsigned int twomoreopixels;
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0 ; i<2 ; i++)
	    olineptrs[i] = (unsigned int *) &((char*)surface->pixels)[i*r_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		twoopixels =	(fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xffff00)
		    |	((fouripixels>>16) & 0xff);
		twomoreopixels =	((fouripixels<<16) & 0xff000000)
		    |	((fouripixels<<8) & 0xffff00)
		    |	(fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
#else
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
#endif
	    } while (x-=4);
	    olineptrs[0] += r_width/4;
	    olineptrs[1] += r_width/4;
	}

    }
    else if (multiply == 3)
    {
	unsigned int *olineptrs[3];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int fouropixels[3];
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0 ; i<3 ; i++)
	    olineptrs[i] = (unsigned int *) &((char*)surface->pixels)[i*r_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		fouropixels[0] = (fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xff0000)
		    |	((fouripixels>>16) & 0xffff);
		fouropixels[1] = ((fouripixels<<8) & 0xff000000)
		    |	(fouripixels & 0xffff00)
		    |	((fouripixels>>8) & 0xff);
		fouropixels[2] = ((fouripixels<<16) & 0xffff0000)
		    |	((fouripixels<<8) & 0xff00)
		    |	(fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
#else
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
#endif
	    } while (x-=4);
	    olineptrs[0] += 2*r_width/4;
	    olineptrs[1] += 2*r_width/4;
	    olineptrs[2] += 2*r_width/4;
	}

    }
    else if (multiply == 4)
    {
	// Broken. Gotta fix this some day.
	void Expand4(unsigned *, double *);
  	Expand4 ((unsigned *)(screens[0]), (double *) (surface->pixels));
    }

    // create texture and load into renderer
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // display renderer and destroy texture
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//
// Palette stuff.
//
static SDL_Color colors[256];

void UploadNewPalette(byte *palette)
{
    // set the colormap entries
    for (register int i=0 ; i<256 ; i++) {
        register int c;
        c = gammatable[usegamma][*palette++];
        colors[i].r = (c<<8) + c;
        c = gammatable[usegamma][*palette++];
        colors[i].g = (c<<8) + c;
        c = gammatable[usegamma][*palette++];
        colors[i].b = (c<<8) + c;
    }

    //  store the colors to the current colormap
    SDL_SetPaletteColors(surface->format->palette, colors, 0, 256);
}

//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
    UploadNewPalette(palette);
}

void I_InitGraphics(void)
{
    // Flag whether this function has been run already since we only
    // want to initialize graphics once
    static boolean firsttime = true;
    if (!firsttime) {
        return;
    }
    firsttime = false;

    // Intialize the SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Set the interrupt signal to quit
    signal(SIGINT, (void (*)(int)) I_Quit);

    // Check for command line argument for resolution multiplier
    if (M_CheckParm("-2")) {
        multiply = 2;
    }
    if (M_CheckParm("-3")) {
        multiply = 3;
    }
    if (M_CheckParm("-4")) {
        multiply = 4;
    }

    // check if the user wanted to grab the mouse (quite unnice)
    grabMouse = !!M_CheckParm("-grabmouse");

    // check for command-line display name
    int			pnum;
    char*		displayname;
    if ( (pnum=M_CheckParm("-disp")) ) { // suggest parentheses around assignment
        displayname = myargv[pnum+1];
    } else {
        displayname = 0;
    }

    // check for command-line geometry
    int			x=0;
    int			y=0;
    if ( (pnum=M_CheckParm("-geom")) ) { // suggest parentheses around assignment
        int			n;
        // warning: char format, different type arg
        char		xsign=' ';
        char		ysign=' ';
        // warning: char format, different type arg 3,5
        n = sscanf(myargv[pnum+1], "%c%d%c%d", &xsign, &x, &ysign, &y);
    
        if (n==2) {
            x = y = 0;
        }
        else if (n==6) {
            if (xsign == '-') x = -x;
            if (ysign == '-') y = -y;
        } else {
            I_Error("bad -geom parameter");
        }
    }

    // create the main window
    r_width = SCREENWIDTH * multiply;
    r_height = SCREENHEIGHT * multiply;
    window = SDL_CreateWindow(
        displayname,                       // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        r_width,                           // width, in pixels
        r_height,                          // height, in pixels
        SDL_WINDOW_SHOWN                   // flags - see below
        );
    if (window == NULL) {
        I_Error("Could not create SDL2 window");
    }

    // grab, hide, and lock pointer position for motion events
    if (grabMouse) {
        SDL_SetWindowGrab(window, SDL_TRUE);
        SDL_SetRelativeMouseMode(SDL_TRUE);
    };

    // create renderer to render context of the window
    renderer = SDL_CreateRenderer(window, -1, 0);

    // create surface for image data
    surface = SDL_CreateRGBSurface(0, r_width, r_height, 8, 0, 0, 0, 0);

    if (multiply == 1) {
        screens[0] = (unsigned char *) (surface->pixels);
    } else {
        screens[0] = (unsigned char *) malloc (SCREENWIDTH * SCREENHEIGHT);
    }
}


unsigned	exptable[256];

void InitExpand (void)
{
    int		i;
	
    for (i=0 ; i<256 ; i++)
	exptable[i] = i | (i<<8) | (i<<16) | (i<<24);
}

double		exptable2[256*256];

void InitExpand2 (void)
{
    int		i;
    int		j;
    // UNUSED unsigned	iexp, jexp;
    double*	exp;
    union
    {
	double 		d;
	unsigned	u[2];
    } pixel;
	
    printf ("building exptable2...\n");
    exp = exptable2;
    for (i=0 ; i<256 ; i++)
    {
	pixel.u[0] = i | (i<<8) | (i<<16) | (i<<24);
	for (j=0 ; j<256 ; j++)
	{
	    pixel.u[1] = j | (j<<8) | (j<<16) | (j<<24);
	    *exp++ = pixel.d;
	}
    }
    printf ("done.\n");
}

int	inited;

void
Expand4
( unsigned*	lineptr,
  double*	xline )
{
    double	dpixel;
    unsigned	x;
    unsigned 	y;
    unsigned	fourpixels;
    unsigned	step;
    double*	exp;
	
    exp = exptable2;
    if (!inited)
    {
	inited = 1;
	InitExpand2 ();
    }
		
		
    step = 3*SCREENWIDTH/2;
	
    y = SCREENHEIGHT-1;
    do
    {
	x = SCREENWIDTH;

	do
	{
	    fourpixels = lineptr[0];
			
	    dpixel = *(double *)( (uintptr_t)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[0] = dpixel;
	    xline[160] = dpixel;
	    xline[320] = dpixel;
	    xline[480] = dpixel;
			
	    dpixel = *(double *)( (uintptr_t)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[1] = dpixel;
	    xline[161] = dpixel;
	    xline[321] = dpixel;
	    xline[481] = dpixel;

	    fourpixels = lineptr[1];
			
	    dpixel = *(double *)( (uintptr_t)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[2] = dpixel;
	    xline[162] = dpixel;
	    xline[322] = dpixel;
	    xline[482] = dpixel;
			
	    dpixel = *(double *)( (uintptr_t)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[3] = dpixel;
	    xline[163] = dpixel;
	    xline[323] = dpixel;
	    xline[483] = dpixel;

	    fourpixels = lineptr[2];
			
	    dpixel = *(double *)( (uintptr_t)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[4] = dpixel;
	    xline[164] = dpixel;
	    xline[324] = dpixel;
	    xline[484] = dpixel;
			
	    dpixel = *(double *)( (uintptr_t)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[5] = dpixel;
	    xline[165] = dpixel;
	    xline[325] = dpixel;
	    xline[485] = dpixel;

	    fourpixels = lineptr[3];
			
	    dpixel = *(double *)( (uintptr_t)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[6] = dpixel;
	    xline[166] = dpixel;
	    xline[326] = dpixel;
	    xline[486] = dpixel;
			
	    dpixel = *(double *)( (uintptr_t)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[7] = dpixel;
	    xline[167] = dpixel;
	    xline[327] = dpixel;
	    xline[487] = dpixel;

	    lineptr+=4;
	    xline+=8;
	} while (x-=16);
	xline += step;
    } while (y--);
}


