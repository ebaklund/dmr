//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Main loop stuff.
//

#ifndef __D_LOOP__
#define __D_LOOP__

#include <d_ticcmd.h>

// Callback function invoked while waiting for the netgame to start.
// The callback is invoked when new players are ready. The callback
// should return true, or return false to abort startup.

typedef boolean (*netgame_startup_callback_t)(int ready_players,
                                              int num_players);

typedef struct
{
    // Read events from the event queue, and process them.

    void (*ProcessEvents)();

    // Given the current input state, fill in the fields of the specified
    // ticcmd_t structure with data for a new tic.

    void (*BuildTiccmd)(ticcmd_t *cmd, int maketic);

    // Advance the game forward one tic, using the specified player input.

    void (*RunTic)(ticcmd_t *cmds, boolean *ingame);

    // Run the menu (runs independently of the game).

    void (*RunMenu)();
} loop_interface_t;

//? how many ticks to run?
void TryRunTics (void);

// Called at start of game loop to initialize timers
void D_StartGameLoop(void);

extern boolean singletics;
extern int gametic, ticdup;
extern int oldleveltime; // [crispy] check if leveltime keeps tickin'

// Check if it is permitted to record a demo with a non-vanilla feature.
boolean D_NonVanillaRecord(boolean conditional, const char *feature);

// Check if it is permitted to play back a demo with a non-vanilla feature.
boolean D_NonVanillaPlayback(boolean conditional, int lumpnum,
                             const char *feature);

#endif

