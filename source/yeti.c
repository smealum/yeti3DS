/*
Copyright (C) 2003 - Derek John Evans 

This file is part of Yeti3D Portable Engine

Yeti3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 2003 - Derek J. Evans <derek@theteahouse.com.au>
Prepared for public release: 10/24/2003 - Derek J. Evans <derek@theteahouse.com.au>
*/

/*
** Name: Yeti3D
** Desc: Portable GameBoy Advanced 3D Engine
** Auth: Derek J. Evans <derek@theteahouse.com.au>
**
** Copyright (C) 2003 Derek J. Evans. All Rights Reserved.
$**
** YY  YY EEEEEE TTTTTT IIIIII 33333  DDDDD
** YY  YY EE       TT     II       33 DD  DD
**  YYYY  EEEE     TT     II     333  DD  DD
**   YY   EE       TT     II       33 DD  DD
**   YY   EEEEEE   TT   IIIIII 33333  DDDDD
*/

#include "yeti.h"

/******************************************************************************/

/*
** Name: palette_overbright
** Desc: Converts a standard palette to a overbright palette. Use this on
**       platforms that bypass the LUA. 
*/
void palette_overbright(palette_t dst, palette_t src, int brightness)
{
  int i;
  
  for (i = 0; i < 256; i++)
  {
    int r = src[i][0] * brightness / FIXONE;
    int g = src[i][1] * brightness / FIXONE;
    int b = src[i][2] * brightness / FIXONE;

    dst[i][0] = CLAMP(r, 0, 255);
    dst[i][1] = CLAMP(g, 0, 255);
    dst[i][2] = CLAMP(b, 0, 255);    
  }
}

/*
** Name: rgb_convert
** Desc: Converts a rgb color to another format.
*/
int rgb_convert(int color, int rmask, int gmask, int bmask)
{
  return
    ((RGB_RED  (color) * rmask / 31) & rmask) |
    ((RGB_GREEN(color) * gmask / 31) & gmask) |
    ((RGB_BLUE (color) * bmask / 31) & bmask) ;
}

/*
** Name: viewport_to_video
** Desc: The Yeti3D uses 5:5:5 colour, so if your platform uses a different
**       format, then one solution is to convert the pixels when blitting to
**       the video buffer.
*/
#ifdef YETI_VIDEO_LUT
void viewport_to_video(
  rgb555_t* video, int pitch,
  viewport_t* vp,
  int rmask, int gmask, int bmask)
{
  int i, y;

  if (!vp->video_lut_filled)
  {
    vp->video_lut_filled = TRUE;

    for (i = RGB_MAX; i--;)
    {
      vp->video_lut[i] = rgb_convert(i, rmask, gmask, bmask);
    }
  }
  for (y = 0; y < YETI_VIEWPORT_HEIGHT; y++)
  {
    rgb555_t* fb = video;
    rgb555_t* tb= vp->back->pixels[y];
    i = YETI_VIEWPORT_WIDTH;
#define AFFINE(I) {fb[I] = vp->video_lut[*tb++];}
    AFFINE_LOOP
#undef AFFINE
    video = (rgb555_t*)((int)video + pitch);
  }
}
#endif

/******************************************************************************/

/*
** Name: cell_init
** Desc: Initialize a yeti cell to its defaults.
*/
void cell_init(cell_t* cell, int issolid)
{
  CLEARMEM(cell);
  
  if (issolid)
  {
    cell->top = i2f(0);
    cell->bot = i2f(0);
  }
  else
  {
    cell->top = i2f(4);
    cell->bot = i2f(0);
  }
#ifdef __YETI_EDITOR__
  cell->tos = cell->top;
  cell->bos = cell->bot;
#endif
  cell->lit = YETI_LIGHT_MAX;
  cell->ttx = 7;
  cell->wtx = 0;
  cell->btx = 28;
}

/*
** Name: yeti_entity
** Desc: Creates a new entity and returns a entity pointer.
*/
entity_t* yeti_entity(yeti_t* yeti, int x, int y, int z, entity_behaviour_t behaviour)
{
  entity_t* e = &yeti->entities[yeti->nentities++];

  CLEARMEM(e);
  
  e->x = x;
  e->y = y;
  e->z = z;
  e->ontick = behaviour;
  e->radius = i2fdiv2(1);
  e->life = 100;
  e->yeti = yeti;

  return e;
}

/*
** Name: yeti_light
** Desc: Renders a spot light at a given cell location.
*/
void yeti_light(yeti_t* yeti, int lightx, int lighty)
{
  int i, lit, xhit, yhit;

  for (i = 0; i < 2048; i += 8)
  {
    int x = lightx, xx = fixsin(i) >> 1;
    int y = lighty, yy = fixcos(i) >> 1;

    for (lit = 63; lit > 0; )
    {
      x += xx; if ((xhit = CELL_IS_SOLID(&yeti->cells[f2i(y)][f2i(x)])) != 0) xx = -xx;
      y += yy; if ((yhit = CELL_IS_SOLID(&yeti->cells[f2i(y)][f2i(x)])) != 0) yy = -yy;

      yeti->cells[f2i(y + FIXHALF)][f2i(x + FIXHALF)].lit += lit;

      if (xhit || yhit) lit >>= 2;
    }
  }
}

/*
** Name: entity_motion
** Desc: Simple map to entity motion & collision.
*/
void entity_motion(entity_t* e)
{
  yeti_t* yeti = (yeti_t*)e->yeti;

  e->x += e->xx; e->y += e->yy; e->z += e->zz;
  e->r += e->rr; e->t += e->tt; e->p += e->pp;

  if (e->radius)
  {
    int x, y;
    cell_t* cell;

#define IS_COLLISION ((e->z > cell->top) || (e->z < cell->bot))

    cell = &yeti->cells[f2i(e->y)][f2i(x = e->x - e->radius)];

    if (IS_COLLISION)
    {
      e->x += FIXONE - (x & FIXCEIL);
      e->xx = 0;
    }

    cell = &yeti->cells[f2i(e->y)][f2i(x = e->x + e->radius)];

    if (IS_COLLISION)
    {
      e->x -= (x & FIXCEIL);
      e->xx = 0;
    }

    cell = &yeti->cells[f2i(y = e->y - e->radius)][f2i(e->x)];

    if (IS_COLLISION)
    {
      e->y += FIXONE - (y & FIXCEIL);
      e->yy = 0;
    }

    cell = &yeti->cells[f2i(y = e->y + e->radius)][f2i(e->x)];

    if (IS_COLLISION)
    {
      e->y -= (y & FIXCEIL);
      e->yy = 0;
    }
  }
}

/******************************************************************************/

void yeti_clear_entities(yeti_t* yeti)
{
  int i;

  yeti->nentities = 0;

  yeti->camera = yeti_entity(yeti,
    i2fdiv2(YETI_MAP_WIDTH),
    i2fdiv2(YETI_MAP_HEIGHT),
    i2f(0), 0);

  for (i = 0; i < YETI_BULLET_MAX; i++)
  {
    yeti->bullets[i] = yeti_entity(yeti, 0, 0, 0, 0);
  }
}

/*
** Name: yeti_load_cell
** Desc: Loads a cell from a ROM based structure.
*/
void yeti_load_cell(cell_t* dst, rom_cell_t* src)
{
  dst->swi = src->swi;
  dst->ent = src->ent;
  dst->top = src->top;
  dst->mid = src->mid;
  dst->bot = src->bot;
  dst->wtx = src->wtx;
  dst->ttx = src->ttx;
  dst->btx = src->btx;
  dst->lit = src->lit;
}

/*
** Name: yeti_save_cell
** Desc: Saves a cell from a ROM based structure.
*/
void yeti_save_cell(cell_t* src, rom_cell_t* dst)
{
  dst->swi = src->swi;
  dst->ent = src->ent;
  dst->top = src->top;
  dst->mid = src->mid;
  dst->bot = src->bot;
  dst->wtx = src->wtx;
  dst->ttx = src->ttx;
  dst->btx = src->btx;
  dst->lit = src->lit;
}

void yeti_load_map(yeti_t* yeti, rom_map_t* src)
{
  int x, y;

  for (y = 0; y < YETI_MAP_HEIGHT; y++)
  {
    for (x = 0; x < YETI_MAP_WIDTH; x++)
    {
      yeti_load_cell(&yeti->cells[y][x], &src->cells[y][x]);
    }
  }
  if (!yeti->camera) yeti_clear_entities(yeti);
  
  for (y = 0; y < YETI_MAP_HEIGHT; y++)
  {
    for (x = 0; x < YETI_MAP_WIDTH; x++)
    {
      if (yeti->cells[y][x].ent == 1)
      {
        yeti->camera->x = i2f(x) + FIXHALF;
        yeti->camera->y = i2f(y) + FIXHALF;
      }
    }
  }
  yeti->camera->t = 0;
  yeti->camera->r = 0;
  yeti->camera->p = i2f(256);
}

/*
** Name: yeti_save_map
** Desc: Saves the current map to a rom based map.
*/
void yeti_save_map(yeti_t* yeti, rom_map_t* map)
{
  int x, y;

  for (y = 0; y < YETI_MAP_HEIGHT; y++)
  {
    for (x = 0; x < YETI_MAP_WIDTH; x++)
    {
      yeti_save_cell(&yeti->cells[y][x], &map->cells[y][x]);
    }
  }
}

/*
** Name: yeti_init_map
** Desc: Clears a yeti map to its default settings.
*/
void yeti_init_map(yeti_t* yeti)
{
  int x, y;
  
  for (y = 0; y < YETI_MAP_HEIGHT; y++)
  {
    for (x = 0; x < YETI_MAP_WIDTH; x++)
    {
      cell_t* cell = &yeti->cells[y][x];
      cell_init(cell,
        x == 0 || y == 0 ||
        (x == YETI_MAP_WIDTH  - 1) ||
        (y == YETI_MAP_HEIGHT - 1));

      if (!((x+8)&15) && !((y+8)&15))
      {
        cell->swi |= CELL_SWI_LIGHT;
      }
    } 
  }
}

/******************************************************************************/

/*
** Name: isqrt
** Desc: Integer square root. Take the square root of an integer.
*/
int isqrt(int value)
{
  int root = 0;

#define STEP(shift) \
    if((0x40000000 >> shift) + root <= value)          \
    {                                                   \
        value -= (0x40000000 >> shift) + root;          \
        root = (root >> 1) | (0x40000000 >> shift);     \
    }                                                   \
    else                                                \
    {                                                   \
        root >>= 1;                                     \
    }

  STEP( 0); STEP( 2); STEP( 4); STEP( 6);
  STEP( 8); STEP(10); STEP(12); STEP(14);
  STEP(16); STEP(18); STEP(20); STEP(22);
  STEP(24); STEP(26); STEP(28); STEP(30);

  // round to the nearest integer, cuts max error in half

  if (root < value) root++;

  return root;
}

/******************************************************************************/

/*
** Name: yeti_init
** Desc: Setup a default map and position the camera.
*/
void yeti_init(
  yeti_t* yeti,
  framebuffer_t* front,
  framebuffer_t* back,
  texture_t* textures,
  palette_t palette,
  lua_t lua)
{
  int i;

  CLEARMEM(yeti);

  for (i = 0; i < YETI_TEXTURE_MAX; i++)
  {
    yeti->surfaces[i].xsize = 6;
    yeti->surfaces[i].ysize = 6;
  }

  yeti->surfaces[YETI_TEXTURE_SKY].xsize = 5;
  yeti->surfaces[YETI_TEXTURE_SKY].ysize = 5;

  yeti->surfaces[0].xsize = 4;
  yeti->surfaces[0].ysize = 4;
  yeti->surfaces[2].xsize = 5;
  yeti->surfaces[2].ysize = 5;

  yeti_init_map(yeti);
  
  yeti->viewport.front = front;
  yeti->viewport.back  = back;

  yeti->textures    = textures;
  yeti->palette     = palette;
  yeti->lighting[0] = lua;
  yeti->lighting[1] = lua;
  yeti->lighting[2] = lua;
  yeti->lighting[3] = lua;
}

void yeti_tick(yeti_t* yeti)
{
  int i;

  yeti->tick++;

  for (i = 0; i < yeti->nentities; i++)
  {
    entity_t* e = &yeti->entities[i];
    if (e->ontick) e->ontick(e);
  }

  for (i = 0; i < yeti->nentities; i++)
  {
    entity_t* e1 = &yeti->entities[i];

    entity_motion(e1);

    if (e1->radius)
    {
      int j;
      
      for (j = i + 1; j < yeti->nentities; j++)
      {
        entity_t* e2 = &yeti->entities[j];

        if (e2->radius)
        {
          int size = e1->radius + e2->radius;
          int x = e1->x - e2->x;
          int y = e1->y - e2->y;
          int z = e1->z - e2->z;

          if (
            x < size && x > -size &&
            y < size && y > -size &&
            z < size && z > -size )
          {
            if (e1->onhit) e1->onhit(e1, e2);
            if (e2->onhit) e2->onhit(e2, e1);

            if (!(e1->swi & ENTITY_SWI_NO_COLLISION_RESPONSE))
            {
              if (x > 0) {e1->x += ( size - x) >> 1;}
              if (x < 0) {e1->x += (-size - x) >> 1;}
              if (y > 0) {e1->y += ( size - y) >> 1;}
              if (y < 0) {e1->y += (-size - y) >> 1;}
            }
            if (!(e2->swi & ENTITY_SWI_NO_COLLISION_RESPONSE))
            {
              if (x > 0) {e2->x -= ( size - x) >> 1;}
              if (x < 0) {e2->x -= (-size - x) >> 1;}
              if (y > 0) {e2->y -= ( size - y) >> 1;}
              if (y < 0) {e2->y -= (-size - y) >> 1;}
            }
          }
        }
      }
    }
  }
  yeti->keyboard._a       = yeti->keyboard.a;
  yeti->keyboard._b       = yeti->keyboard.b;
  yeti->keyboard._select  = yeti->keyboard.select;
  yeti->keyboard._left    = yeti->keyboard.left;
  yeti->keyboard._right   = yeti->keyboard.right;
  yeti->keyboard._up      = yeti->keyboard.up;
  yeti->keyboard._down    = yeti->keyboard.down;
  yeti->keyboard._r       = yeti->keyboard.r;
  yeti->keyboard._l       = yeti->keyboard.l;

  yeti->surfaces[YETI_TEXTURE_SKY].xpan = -(yeti->camera->x>>1) + (yeti->tick<<2);
  yeti->surfaces[YETI_TEXTURE_SKY].ypan = -(yeti->camera->y>>1) + (yeti->tick<<2);
  yeti->surfaces[YETI_TEXTURE_WINDOW].xpan = -(yeti->camera->y>>1) + (yeti->tick<<2);

  yeti->surfaces[YETI_TEXTURE_LAVA ].xpan = fixsin(yeti->tick << 4) >> 2;
  yeti->surfaces[YETI_TEXTURE_LAVA ].ypan = fixcos(yeti->tick << 4) >> 2;
  yeti->surfaces[YETI_TEXTURE_WATER].xpan = fixsin(yeti->tick << 4) >> 2;
  yeti->surfaces[YETI_TEXTURE_WATER].ypan = fixcos(yeti->tick << 4) >> 2;
}

/*
** Name: yeti_ambient_lighting
** Desc: Setup a standard ambient lighting. Current lighting is removed.
** Note: This is obsolete since we have better lighting code now.
*/
void yeti_ambient_lighting(yeti_t* yeti, int lit)
{
  int x, y;

  for (y = 0; y < YETI_MAP_HEIGHT; y++)
  {
    for (x = 0; x < YETI_MAP_WIDTH; x++)
    {
      yeti->cells[y][x].lit = lit;
    }
  }
}

/*
** Name: yeti_default_lighting
** Desc: Renders a default lighting setup for maps that dont have lights. This
**       is currently the only lighting system and is designed to give good
**       results for all map designs.
*/
void yeti_default_lighting(yeti_t* yeti)
{
  int x, y;

  yeti_ambient_lighting(yeti, i2f(0));

  for (y = 1; y < YETI_MAP_HEIGHT - 1; y++)
  {
    for (x = 1; x < YETI_MAP_WIDTH - 1; x++)
    {
      cell_t* cell = &yeti->cells[y][x];
      
      if (cell->swi & CELL_SWI_LIGHT)
      {
        yeti_light(yeti, i2f(x) + FIXHALF, i2f(y) + FIXHALF);
      }
    }
  }
}

/******************************************************************************/

void* yeti_memcpy(void* d, void* s, unsigned n)
{
  while (n--) ((u8*)d)[n] = ((u8*)s)[n];
  return d;
}

void* yeti_memset(void *s, int c, unsigned n)
{
  while (n--) ((u8*)s)[n] = c;
  return s;
}

/******************************************************************************/

