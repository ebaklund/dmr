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
//     OPL interface.
//

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

#include "opl.h"
#include "opl_sdl.h"

static int init_stage_reg_writes = 1;

unsigned int opl_sample_rate = 22050;

//
// Init/shutdown code.
//

// Initialize the specified driver and detect an OPL chip.  Returns
// true if an OPL is detected.

static opl_init_result_t InitDriver(unsigned int port_base)
{
    // Initialize the driver.

    if (!OPL_SDL_Init(port_base))
    {
        return OPL_INIT_NONE;
    }

    // The driver was initialized okay, so we now have somewhere
    // to write to.  It doesn't mean there's an OPL chip there,
    // though.  Perform the detection sequence to make sure.
    // (it's done twice, like how Doom does it).

    init_stage_reg_writes = 1;

    opl_init_result_t result1 = OPL_Detect();
    opl_init_result_t result2 = OPL_Detect();

    if (result1 == OPL_INIT_NONE || result2 == OPL_INIT_NONE)
    {
        printf("OPL_Init: No OPL detected.\n");
        OPL_SDL_Shutdown();
        return OPL_INIT_NONE;
    }

    init_stage_reg_writes = 0;

    return result2;
}

// Find a driver automatically by trying each in the list.

static opl_init_result_t AutoSelectDriver(unsigned int port_base)
{
    return InitDriver(port_base);
}

// Initialize the OPL library. Return value indicates type of OPL chip
// detected, if any.

opl_init_result_t OPL_Init(unsigned int port_base)
{
    return InitDriver(port_base);
}

// Shut down the OPL library.

void OPL_Shutdown(void)
{
    OPL_SDL_Shutdown();
}

// Set the sample rate used for software OPL emulation.

int OPL_GetSampleRate()
{
    return opl_sample_rate;
}

void OPL_SetSampleRate(unsigned int rate)
{
    opl_sample_rate = rate;
}

void OPL_WritePort(opl_port_t port, unsigned int value)
{
    OPL_SDL_PortWrite(port, value);
}

unsigned int OPL_ReadPort(opl_port_t port)
{
    return OPL_SDL_PortRead(port);
}

unsigned int OPL_ReadStatus(void)
{
    return OPL_ReadPort(OPL_REGISTER_PORT);
}

// Write an OPL register value

void OPL_WriteRegister(int reg, int value)
{
    int i;

    if (reg & 0x100)
    {
        OPL_WritePort(OPL_REGISTER_PORT_OPL3, reg);
    }
    else
    {
        OPL_WritePort(OPL_REGISTER_PORT, reg);
    }

    // For timing, read the register port six times after writing the
    // register number to cause the appropriate delay

    for (i=0; i<6; ++i)
    {
        // An oddity of the Doom OPL code: at startup initialization,
        // the spacing here is performed by reading from the register
        // port; after initialization, the data port is read, instead.

        if (init_stage_reg_writes)
        {
            OPL_ReadPort(OPL_REGISTER_PORT);
        }
        else
        {
            OPL_ReadPort(OPL_DATA_PORT);
        }
    }

    OPL_WritePort(OPL_DATA_PORT, value);

    // Read the register port 24 times after writing the value to
    // cause the appropriate delay

    for (i=0; i<24; ++i)
    {
        OPL_ReadStatus();
    }
}

// Detect the presence of an OPL chip

opl_init_result_t OPL_Detect(void)
{
    int result1, result2;
    int i;

    // Reset both timers:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x60);

    // Enable interrupts:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x80);

    // Read status
    result1 = OPL_ReadStatus();

    // Set timer:
    OPL_WriteRegister(OPL_REG_TIMER1, 0xff);

    // Start timer 1:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x21);

    // Wait for 80 microseconds
    // This is how Doom does it:

    for (i=0; i<200; ++i)
    {
        OPL_ReadStatus();
    }

    OPL_Delay(1 * OPL_MS);

    // Read status
    result2 = OPL_ReadStatus();

    // Reset both timers:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x60);

    // Enable interrupts:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x80);

    if ((result1 & 0xe0) == 0x00 && (result2 & 0xe0) == 0xc0)
    {
        result1 = OPL_ReadPort(OPL_REGISTER_PORT);
        result2 = OPL_ReadPort(OPL_REGISTER_PORT_OPL3);
        if (result1 == 0x00)
        {
            return OPL_INIT_OPL3;
        }
        else
        {
            return OPL_INIT_OPL2;
        }
    }
    else
    {
        return OPL_INIT_NONE;
    }
}

// Initialize registers on startup

void OPL_InitRegisters(int opl3)
{
    int r;

    // Initialize level registers

    for (r=OPL_REGS_LEVEL; r <= OPL_REGS_LEVEL + OPL_NUM_OPERATORS; ++r)
    {
        OPL_WriteRegister(r, 0x3f);
    }

    // Initialize other registers
    // These two loops write to registers that actually don't exist,
    // but this is what Doom does ...
    // Similarly, the <= is also intenational.

    for (r=OPL_REGS_ATTACK; r <= OPL_REGS_WAVEFORM + OPL_NUM_OPERATORS; ++r)
    {
        OPL_WriteRegister(r, 0x00);
    }

    // More registers ...

    for (r=1; r < OPL_REGS_LEVEL; ++r)
    {
        OPL_WriteRegister(r, 0x00);
    }

    // Re-initialize the low registers:

    // Reset both timers and enable interrupts:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL,      0x60);
    OPL_WriteRegister(OPL_REG_TIMER_CTRL,      0x80);

    // "Allow FM chips to control the waveform of each operator":
    OPL_WriteRegister(OPL_REG_WAVEFORM_ENABLE, 0x20);

    if (opl3)
    {
        OPL_WriteRegister(OPL_REG_NEW, 0x01);

        // Initialize level registers

        for (r=OPL_REGS_LEVEL; r <= OPL_REGS_LEVEL + OPL_NUM_OPERATORS; ++r)
        {
            OPL_WriteRegister(r | 0x100, 0x3f);
        }

        // Initialize other registers
        // These two loops write to registers that actually don't exist,
        // but this is what Doom does ...
        // Similarly, the <= is also intenational.

        for (r=OPL_REGS_ATTACK; r <= OPL_REGS_WAVEFORM + OPL_NUM_OPERATORS; ++r)
        {
            OPL_WriteRegister(r | 0x100, 0x00);
        }

        // More registers ...

        for (r=1; r < OPL_REGS_LEVEL; ++r)
        {
            OPL_WriteRegister(r | 0x100, 0x00);
        }
    }

    // Keyboard split point on (?)
    OPL_WriteRegister(OPL_REG_FM_MODE,         0x40);

    if (opl3)
    {
        OPL_WriteRegister(OPL_REG_NEW, 0x01);
    }
}

//
// Timer functions.
//

void OPL_SetCallback(uint64_t us, opl_callback_t callback, void *data)
{
    OPL_SDL_SetCallback(us, callback, data);
}

void OPL_ClearCallbacks(void)
{
    OPL_SDL_ClearCallbacks();
}

void OPL_Lock(void)
{
    OPL_SDL_Lock();
}

void OPL_Unlock(void)
{
    OPL_SDL_Unlock();
}

typedef struct
{
    int finished;

    SDL_mutex *mutex;
    SDL_cond *cond;
} delay_data_t;

static void DelayCallback(void *_delay_data)
{
    delay_data_t *delay_data = _delay_data;

    SDL_LockMutex(delay_data->mutex);
    delay_data->finished = 1;

    SDL_CondSignal(delay_data->cond);

    SDL_UnlockMutex(delay_data->mutex);
}

void OPL_Delay(uint64_t us)
{
    delay_data_t delay_data;

    // Create a callback that will signal this thread after the
    // specified time.

    delay_data.finished = 0;
    delay_data.mutex = SDL_CreateMutex();
    delay_data.cond = SDL_CreateCond();

    OPL_SetCallback(us, DelayCallback, &delay_data);

    // Wait until the callback is invoked.

    SDL_LockMutex(delay_data.mutex);

    while (!delay_data.finished)
    {
        SDL_CondWait(delay_data.cond, delay_data.mutex);
    }

    SDL_UnlockMutex(delay_data.mutex);

    // Clean up.

    SDL_DestroyMutex(delay_data.mutex);
    SDL_DestroyCond(delay_data.cond);
}

void OPL_SetPaused(int paused)
{
    OPL_SDL_SetPaused(paused);
}

void OPL_AdjustCallbacks(float value)
{
    OPL_SDL_AdjustCallbacks(value);
}

