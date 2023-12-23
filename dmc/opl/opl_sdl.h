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
// DESCRIPTION:
//     OPL internal interface.
//

#ifndef OPL_INTERNAL_H
#define OPL_INTERNAL_H

int OPL_SDL_Init(unsigned int port_base);
void OPL_SDL_Shutdown();
unsigned int OPL_SDL_PortRead(opl_port_t port);
void OPL_SDL_PortWrite(opl_port_t port, unsigned int value);
void OPL_SDL_SetCallback(uint64_t us, opl_callback_t callback, void *data);
void OPL_SDL_ClearCallbacks(void);
void OPL_SDL_Lock(void);
void OPL_SDL_Unlock(void);
void OPL_SDL_SetPaused(int paused);
void OPL_SDL_AdjustCallbacks(float factor);

#endif /* #ifndef OPL_INTERNAL_H */

