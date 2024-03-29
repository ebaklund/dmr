//
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
//
// DESCRIPTION:
//   Functions and definitions relating to the game type and operational
//   mode.
//

#include "d_mode.h"
#include "doomtype.h"

// Valid game mode/mission combinations, with the number of
// episodes/maps for each.

static struct
{
  GameMission_t mission;
  GameMode_t mode;
  int episode;
  int map;
} valid_modes[] = {
    {doom, shareware, 1, 9},
};

// Table of valid versions

static struct
{
  GameMission_t mission;
  GameVersion_t version;
} valid_versions[] = {
    {doom, exe_doom_1_2}, {doom, exe_doom_1_666}, {doom, exe_doom_1_7},
    {doom, exe_doom_1_8}, {doom, exe_doom_1_9},
};

const char* D_GameMissionString(GameMission_t mission)
{
  switch (mission)
  {
    case none:
    default:
      return "none";
    case doom:
      return "doom";
  }
}
