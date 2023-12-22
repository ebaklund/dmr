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
//   Functions and definitions relating to the game type and operational
//   mode.
//

#ifndef __D_MODE__
#define __D_MODE__

#include "doomtype.h"

// The "mission" controls what game we are playing.

typedef enum
{
  doom,            // Doom 1
  none
} GameMission_t;

// The "mode" allows more accurate specification of the game mode we are
// in: eg. shareware vs. registered.  So doom1.wad and doom.wad are the
// same mission, but a different mode.

typedef enum
{
  shareware,       // Doom shareware
  indetermined     // Unknown.
} GameMode_t;

// What version are we emulating?

typedef enum
{
    exe_doom_1_2,    // Doom 1.2: shareware and registered
    exe_doom_1_666,  // Doom 1.666: for shareware, registered
    exe_doom_1_7,    // Doom 1.7/1.7a: "
    exe_doom_1_8,    // Doom 1.8: "
    exe_doom_1_9,    // Doom 1.9: "
} GameVersion_t;

// What IWAD variant are we using?

typedef enum
{
    vanilla,    // Vanilla Doom
    freedoom,   // FreeDoom: Phase 1 + 2
    freedm,     // FreeDM
    bfgedition, // Doom Classic (Doom 3: BFG Edition)
} GameVariant_t;

// Skill level.

typedef enum
{
    sk_noitems = -1,        // the "-skill 0" hack
    sk_baby = 0,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare
} skill_t;

boolean D_ValidGameMode(GameMission_t mission, GameMode_t mode);

const char *D_GameMissionString(GameMission_t mission);

#endif /* #ifndef __D_MODE__ */

