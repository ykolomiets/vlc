/*****************************************************************************
 * audio.c: Generic lua interface functions
 *****************************************************************************
 * Copyright (C) 2007-2008 the VideoLAN team
 * $Id$
 *
 * Authors: Antoine Cellerier <dionoea at videolan tod org>
 *          Pierre d'Herbemont <pdherbemont # videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#ifndef  _GNU_SOURCE
#   define  _GNU_SOURCE
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_meta.h>
#include <vlc_playlist.h>

#include "../vlc.h"
#include "../libs.h"

/*****************************************************************************
 * Aout control
 *****************************************************************************/

static int vlclua_audio_get_device( lua_State *L )
{
    playlist_t *p_playlist = vlclua_get_playlist_internal( L );
    audio_output_t *p_aout = playlist_GetAout( p_playlist );
    if( p_aout == NULL )
        return 0;

    char **ids, **names;
    int n = aout_DevicesList( p_aout, &ids, &names );
    if( n < 0 )
      return 0;

    char *dev = aout_DeviceGet( p_aout );
    if ( dev == NULL )
        lua_pushnil( L );
    else
    {
        lua_newtable( L );
        lua_pushstring( L, dev );
        lua_setfield( L, -2, "id" );
        for( int i = 0; i < n; i++ )
        {
            if( !strcmp(dev, ids[i]) )
            {
                lua_pushstring( L, names[i] );
                lua_setfield( L, -2, "name" );
                break;
            }
        }

        free( dev );
        for( int i = 0; i < n; i++ )
        {
            free( ids[i] );
            free( names[i] );
        }
        free( ids );
        free( names );
    }

    return 1;
}

static int vlclua_audio_get_devices_list( lua_State *L )
{
    playlist_t *p_playlist = vlclua_get_playlist_internal( L );
    audio_output_t *p_aout = playlist_GetAout( p_playlist );
    if( p_aout == NULL )
        return 0;

    char **ids, **names;
    int n = aout_DevicesList( p_aout, &ids, &names );
    if( n < 0 )
      return 0;

    char *dev = aout_DeviceGet( p_aout );
    const char* devstr = (dev != NULL) ? dev : "";

    lua_newtable( L );
    for ( int i = 0; i < n; i++ )
    {
        lua_newtable( L );
        lua_pushstring( L, ids[i] );
        lua_setfield(L, -2, "id" );
        lua_pushstring( L, names[i] );
        lua_setfield(L, -2, "name" );
        if ( !strcmp(devstr, ids[i]) )
        {
            lua_pushboolean( L, true );
            lua_setfield( L, -2, "active" );
        }
        lua_rawseti( L, -2, i+1 );
    }

    free( dev );
    for( int i = 0; i < n; i++ )
    {
        free( ids[i] );
        free( names[i] );
    }
    free( ids );
    free( names );

    return 1;
}

static int vlclua_audio_set_device( lua_State *L )
{
    const char *devid = luaL_checkstring( L, 1 );

    playlist_t *p_playlist = vlclua_get_playlist_internal( L );
    audio_output_t *p_aout = playlist_GetAout( p_playlist );
    if( p_aout == NULL )
        return 0;

    return aout_DeviceSet( p_aout, devid );
}

static int vlclua_audio_cycle_device( lua_State *L )
{
    playlist_t *p_playlist = vlclua_get_playlist_internal( L );
    audio_output_t *p_aout = playlist_GetAout( p_playlist );
    if( p_aout == NULL )
        return 0;

    char **ids, **names;
    int n = aout_DevicesList( p_aout, &ids, &names );
    if( n < 0)
      return 0;

    char *dev = aout_DeviceGet( p_aout );
    const char *devstr = (dev != NULL) ? dev : "";

    int next_device_idx = 0;
    for( int i = 0; i < n; i++ )
    {
        if( !strcmp(devstr, ids[i]) )
            next_device_idx = (i + 1) % n;
    }
    free( dev );

    int ret = aout_DeviceSet( p_aout, ids[next_device_idx] );

    for( int i = 0; i < n; i++ )
    {
        free( ids[i] );
        free( names[i] );
    }
    free( ids );
    free( names );

    return ret;
}

/*****************************************************************************
 *
 *****************************************************************************/
static const luaL_Reg vlclua_audio_reg[] = {
    { "get_device", vlclua_audio_get_device },
    { "set_device", vlclua_audio_set_device },
    { "get_devices_list", vlclua_audio_get_devices_list },
    { "cycle_device", vlclua_audio_cycle_device },
    { NULL, NULL }
};

void luaopen_audio( lua_State *L )
{
    lua_newtable( L );
    luaL_register( L, NULL, vlclua_audio_reg );
    lua_setfield( L, -2, "audio" );
}
