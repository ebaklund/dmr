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
//	WAD I/O functions.
//

#include <stdio.h>

#include "config.h"
#include "doomtype.h"
#include "m_argv.h"
#include "w_file.h"
#include "m_misc.h"
#include "z_zone.h"

extern wad_file_class_t stdc_wad_file;

wad_file_t *W_OpenFile(const char *path)
{
    wad_file_t*result;
    FILE *fstream;

    fstream = fopen(path, "rb");

    if (fstream == NULL)
    {
        return NULL;
    }

    // Create a new stdc_wad_file_t to hold the file handle.

    result = Z_Malloc(sizeof(wad_file_t), PU_STATIC, 0);
    result->file_class = &stdc_wad_file;
    result->mapped = NULL;
    result->length = M_FileLength(fstream);
    char* x = M_StringDuplicate(path);
    result->path = x;
    result->fstream = fstream;

    return result;
}

void W_CloseFile(wad_file_t *wad)
{
    wad->file_class->CloseFile(wad);
}

size_t W_Read(wad_file_t *wad, unsigned int offset,
              void *buffer, size_t buffer_len)
{
    return wad->file_class->Read(wad, offset, buffer, buffer_len);
}

