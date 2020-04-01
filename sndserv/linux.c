// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: linux.c,v 1.3 1997/01/26 07:45:01 b1 Exp $
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
//
// $Log: linux.c,v $
// Revision 1.3  1997/01/26 07:45:01  b1
// 2nd formatting run, fixed a few warnings as well.
//
// Revision 1.2  1997/01/21 19:00:01  b1
// First formatting run:
//  using Emacs cc-mode.el indentation for C++ now.
//
// Revision 1.1  1997/01/19 17:22:45  b1
// Initial check in DOOM sources as of Jan. 10th, 1997
//
//
// DESCRIPTION:
//	UNIX, soundserver for Linux i386.
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: linux.c,v 1.3 1997/01/26 07:45:01 b1 Exp $";


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "soundsrv.h"


void I_InitMusic(void)
{
}

void
I_InitSound
( int	samplerate,
  int	samplesize )
{

    // initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_INIT could not initialize audio\n");
    }

    // initialize SDL_mixer
    if(Mix_OpenAudio(samplerate, AUDIO_U8, 1, MIXBUFFERSIZE) < 0) {
        fprintf(stderr, "SDL_mixer could not initialize\n");
    }
}

void
I_SubmitOutputBuffer
( void*	samples,
  int	samplecount )
{
    // write(audio_fd, samples, samplecount*4);
}

void I_ShutdownSound(void)
{
    Mix_CloseAudio();
}

void I_ShutdownMusic(void)
{
}
