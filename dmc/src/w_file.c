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

#include "w_file.h"
#include "m_misc.h"
#include "z_zone.h"

extern wad_file_class_t stdc_wad_file;

wad_file_t *W_OpenFile(const char *path)
{
    FILE* fstream = fopen(path, "rb");

    if (fstream == NULL)
    {
        return NULL;
    }

    wad_file_t* result = Z_Malloc(sizeof(wad_file_t), PU_STATIC, 0);
    result->file_class = NULL;
    result->mapped = NULL;
    result->length = M_FileLength(fstream);
    char* x = M_StringDuplicate(path);
    result->path = x;
    result->fstream = fstream;

    return result;
}

void W_CloseFile(wad_file_t *wad)
{
    fclose(wad->fstream);
    Z_Free(wad);
}

size_t W_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t buffer_len)
{
    fseek(wad->fstream, offset, SEEK_SET);
    size_t result = fread(buffer, 1, buffer_len, wad->fstream);

    return result;
}

