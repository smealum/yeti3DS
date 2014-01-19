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
**
** YY  YY EEEEEE TTTTTT IIIIII 33333  DDDDD
** YY  YY EE       TT     II       33 DD  DD
**  YYYY  EEEE     TT     II     333  DD  DD
**   YY   EE       TT     II       33 DD  DD
**   YY   EEEEEE   TT   IIIIII 33333  DDDDD
*/

/******************************************************************************/

#ifndef __YETI3D_H__
#define __YETI3D_H__

/******************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif

/******************************************************************************/

#define YETI_STR_TITLE      "Yeti3D Engine"
#define YETI_STR_VERSION    "1.0.0.1"
#define YETI_STR_COPYRIGHT  "Copyright (C) 2003 Derek J. Evans. All Rights Reserved"
#define YETI_STR_URL        "http://www.theteahouse.com.au/gba/"
#define YETI_STR_CAPTION    (YETI_STR_TITLE " " YETI_STR_VERSION " - " YETI_STR_COPYRIGHT)


#define YETI_TEXTURE_SKY 9
#define YETI_TEXTURE_WINDOW 10
#define YETI_TEXTURE_LAVA 4
#define YETI_TEXTURE_WATER 31

/******************************************************************************/

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#define ABS(A) ((A)<0?-(A):(A))
#define MIN(A,B) ((A)<(B)?(A):(B))
#define MAX(A,B) ((A)>(B)?(A):(B))

#define CLAMP(A,B,C) MIN(MAX(A, B), C)

#define CLEARMEM(A) yeti_memset(A, 0, sizeof(*(A)))

/******************************************************************************/

#define YETI_CONSTANT_LIGHT i2f(2)
#define YETI_TEXTURE_MAX 32
#define YETI_ENTITY_MAX 100
#define YETI_POLYGON_MAX 16
#define YETI_VPOOL_MAX 512
#define YETI_BULLET_MAX 16
#define YETI_RAY_MAX 40
#define YETI_RAY_QUALITY 1
#define YETI_CELL_MAX 200
#define YETI_MAP_WIDTH 64
#define YETI_MAP_HEIGHT 64
#define YETI_LIGHT_MIN i2f(1)
#define YETI_LIGHT_MAX i2f(63)
#define YETI_RECIPROCAL_MIN 0
#define YETI_RECIPROCAL_MAX 16384
#define YETI_SINTABLE_MAX 2048
#define YETI_TRIANGLE_BUCKET_MAX 512

/******************************************************************************/

#define FIXONE  0x100
#define FIXHALF 0x80
#define FIXCEIL 0xFF

#define f2i(A) ((A)>>8)
#define i2f(A) ((A)<<8)
#define r2i(A) ((A)>>16)
#define i2fdiv2(A) ((A)<<7)
#define f2fl(A) ((A)/256.0)
#define fl2f(A) ((A)*256.0)
#define fixtrunc(A) i2f(f2i(A))
#define fixlerp(A,B,C) ((A)+fixmul((B)-(A),(C)))
#define fixup(A) f2i((A)+FIXCEIL)
#define fixinv(A) fixdiv(FIXUNIT,(A))

#define intdiv(A,B) ((A)/(B))
#define muldiv(A,B,C) intdiv((A)*(B),(C))
#define fixmul(A,B) f2i((A)*(B))
#define fixdiv(A,B) intdiv(i2f(A),(B))
#define fixsin(A) sintable[(A)&2047]
#define fixcos(A) fixsin((A)+512)

/******************************************************************************/

#include "viewports.h"

#ifdef YETI_LOW_QUALITY_WALLS
#define YETI_VCACHE_MAX 4
#else
#define YETI_VCACHE_MAX 8
#endif

//#define YETI_RGB555


#ifdef YETI_RGB555
#define RGB_SET(R,G,B) (((B)<<10)|((G)<<5)|(R))
#define RGB_BLEND_MASK 0x7BDE
#define RGB_BLEND(A,B) ((((A) & RGB_BLEND_MASK) + ((B) & RGB_BLEND_MASK)) >> 1)
#define RGB_RED(A)   (((A)>> 0)&31)
#define RGB_GREEN(A) (((A)>> 5)&31)
#define RGB_BLUE(A)  (((A)>>10)&31)

#endif

#ifdef YETI_RGB565
#define RGB_SET(R,G,B) (((B)<<11)|((G)<<5)|(R))
#define RGB_BLEND_MASK 0x7BDE
#define RGB_BLEND(A,B) ((((A) & RGB_BLEND_MASK) + ((B) & RGB_BLEND_MASK)) >> 1)
#define RGB_RED(A)   (((A)>> 0)&31)
#define RGB_GREEN(A) (((A)>> 5)&31)
#define RGB_BLUE(A)  (((A)>>11)&31)

#endif

#ifdef YETI_RGB444
#define RGB_SET(R,G,B) ((((R)>>1)<<8)|(((G)>>1)<<4)|((B)>>1))
#define RGB_BLEND_MASK 0x0EEE
#define RGB_BLEND(A,B) ((((A) & RGB_BLEND_MASK) + ((B) & RGB_BLEND_MASK)) >> 1)
#define RGB_RED(A)   ((((A)>>8)<<1)&31)
#define RGB_GREEN(A) ((((A)>>4)<<1)&31)
#define RGB_BLUE(A)  ((((A)>>0)<<1)&31)
#endif

#define RGB_MAX 65536

#define RGB_WHITE RGB_SET(31, 31, 31)
/******************************************************************************/

#define YETI_TEXGEN_WALL_LR  0
#define YETI_TEXGEN_WALL_FB  1
#define YETI_TEXGEN_FLAT     2
#define YETI_TEXGEN_BASIC    3

/******************************************************************************/

#ifndef TYPES_H
  typedef unsigned char   u8;
  typedef unsigned short  u16;
  typedef unsigned long   u32;

  typedef signed char     s8;
  typedef signed short    s16;
  typedef signed long     s32;

  typedef unsigned char byte;
#endif

static inline void putPixel(u8* fb, u16 v, u16 I)
{
  fb[I*3*YETI_VIEWPORT_HEIGHT]=RGB_BLUE(v)<<3;
  fb[I*3*YETI_VIEWPORT_HEIGHT+1]=RGB_GREEN(v)<<3;
  fb[I*3*YETI_VIEWPORT_HEIGHT+2]=RGB_RED(v)<<3;
}
/******************************************************************************/

/*#define AFFINE_CASE(I) case (I): AFFINE(0); fb++;

#define AFFINE_LOOP  \
for (; i >= 16; i -= 16, fb += 16) { \
AFFINE( 0) AFFINE( 1) AFFINE( 2) AFFINE( 3) AFFINE( 4) AFFINE( 5) AFFINE( 6) AFFINE( 7) \
AFFINE( 8) AFFINE( 9) AFFINE(10) AFFINE(11) AFFINE(12) AFFINE(13) AFFINE(14) AFFINE(15) \
} switch (i) { \
AFFINE_CASE(16) AFFINE_CASE(15) AFFINE_CASE(14) AFFINE_CASE(13) \
AFFINE_CASE(12) AFFINE_CASE(11) AFFINE_CASE(10) AFFINE_CASE( 9) \
AFFINE_CASE( 8) AFFINE_CASE( 7) AFFINE_CASE( 6) AFFINE_CASE( 5) \
AFFINE_CASE( 4) AFFINE_CASE( 3) AFFINE_CASE( 2) AFFINE_CASE( 1) \
}*/
#define AFFINE_CASE(I) case (I): AFFINE(0); fb2+=3*YETI_VIEWPORT_HEIGHT;

#define AFFINE_LOOP  \
for (; i >= 16; i -= 16, fb2 += 48*YETI_VIEWPORT_HEIGHT) { \
AFFINE( 0) AFFINE( 1) AFFINE( 2) AFFINE( 3) AFFINE( 4) AFFINE( 5) AFFINE( 6) AFFINE( 7) \
AFFINE( 8) AFFINE( 9) AFFINE(10) AFFINE(11) AFFINE(12) AFFINE(13) AFFINE(14) AFFINE(15) \
} switch (i) { \
AFFINE_CASE(16) AFFINE_CASE(15) AFFINE_CASE(14) AFFINE_CASE(13) \
AFFINE_CASE(12) AFFINE_CASE(11) AFFINE_CASE(10) AFFINE_CASE( 9) \
AFFINE_CASE( 8) AFFINE_CASE( 7) AFFINE_CASE( 6) AFFINE_CASE( 5) \
AFFINE_CASE( 4) AFFINE_CASE( 3) AFFINE_CASE( 2) AFFINE_CASE( 1) \
}

/******************************************************************************/

typedef struct
{
  int x, y;
} vec2_t;

typedef struct
{
  int x, y, z;
} vec3_t;

typedef int matrx_t[3][3];
typedef vec3_t Quad_t[4];

/******************************************************************************/

typedef u16 rgb555_t;
typedef rgb555_t video_lut_t[RGB_MAX];

typedef struct
{
  rgb555_t pixels[YETI_VIEWPORT_HEIGHT][YETI_VIEWPORT_WIDTH];
} framebuffer_t;

typedef struct
{
  framebuffer_t* front;
  framebuffer_t* back;
#ifdef YETI_VIDEO_LUT
  video_lut_t video_lut;
  int video_lut_filled;
#endif
} viewport_t;

typedef struct
{
  u8 pixels[8][8];
} fontchar_t;

typedef u8 texture_t[64][64];
typedef u16 skin_t[256][256];
typedef rgb555_t lut_t[256];
typedef lut_t lua_t[64];
typedef lua_t lighting_t[4];
typedef u8 color_t[3];
typedef color_t palette_t[256];

typedef rgb555_t* sprite_t;

#define sprite_width(A) (A)[0]
#define sprite_height(A) (A)[1]
#define sprite_pixel(A,X,Y) (A)[(Y)*sprite_width(A)+(X)+2]

/******************************************************************************/

// Type definition of a polygon vertex
typedef struct  
{ 
  s32 x;  // 24:8 Fixed Point X Position.
  s32 y;  // 24:8 Fixed Point Y Position.
  s32 z;  // 24:8 Fixed Point Z Position.
  s32 l;  // 24:8 Fixed point lighting. Ranges from 0 to 63.
  s32 u;  // 24:8 texture 'u' cooridinate.
  s32 v;  // 24:8 texture 'v' cooridinate.
  s32 d;  // Distance to plane value. Also used for vertex scanline storage.
  s32 i;  // Vertex ID. Used to lookup a vertex in a cell vcache.
  s32 sx; // X projected into screen space.
  s32 sy; // Y projected into screen space.
} vertex_t;

typedef vertex_t polygon_t[YETI_POLYGON_MAX];
typedef vertex_t* polyclip_t[YETI_POLYGON_MAX];

/******************************************************************************/

// The entity visual is used to store data used for rendering a entity. By
// default this is a 2D sprite, but you can override entity_t.ondraw and
// render anything from meshs to particle effects.

#define ENTITY_SWI_NO_COLLISION_RESPONSE 1

typedef struct
{
  void* data;
  u8 width;
  u8 height;
  u8 mode;
} entity_visual_t;

typedef struct entity_t
{
  struct entity_t* next;
  
  void* tag;
  void* yeti;
  
  entity_visual_t visual;

  u8 swi;
  s8 life;
  u16 radius;
  s32 x, xx;  // X position and velocity.
  s32 y, yy;  // Y position and velocity.
  s32 z, zz;  // Z position and velocity.
  s32 p, pp;  // Pitch
  s32 r, rr;  // Roll
  s32 t, tt;  // Turn
  s32 rx;
  s32 ry;
  s32 rz;
  void (*ontick)(struct entity_t* e);
  void (*onhit)(struct entity_t* e1, struct entity_t* e2);
  void (*ondraw)(struct entity_t* e);
} entity_t;

typedef void (*entity_behaviour_t)(entity_t* e);

/******************************************************************************/

#define CELL_SWI_LIGHT    1
#define CELL_SWI_SOLID    2
#define CELL_SWI_TERRAIN  4
#define CELL_SWI_CORNER   8
#define CELL_SWI_LUA_0    0
#define CELL_SWI_LUA_1    16
#define CELL_SWI_LUA_2    32
#define CELL_SWI_LUA_3    48
#define CELL_SWI_LUA_MASK 48
#define CELL_SWI_CAVE     64
#define CELL_SWI_RESERVED 128

typedef struct
{
  u8 swi;   // Cell switchs (flags).
  u8 ent;   // Entity Type. Used for setting start locations for entities.  
  s16 top;  // 8:8 fixed point cell top value.
  s16 mid;  // 8:8 fixed point cell mid value. (Currently Unused/Reserved)
  s16 bot;  // 8:8 fixed point cell bot value.
  u16 lit;  // Cell light setting.
  u8 wtx;   // Cell wall texture id.
  u8 ttx;   // Cell top texture id.
  u8 mtx;   // Cell mid texture id. (Currently Unused/Reserved)
  u8 btx;   // Cell bot texture id.
  u8 res1;  // Reserved.
  u8 res2;  // Reserved.
} rom_cell_t;

typedef struct
{
  u8 name[32];
  u8 auth[32];
  u8 desc[64];
  rom_cell_t cells[YETI_MAP_HEIGHT][YETI_MAP_WIDTH];
} rom_map_t;

typedef struct 
{
  u16 swi;  // Cell flags.
  s16 top;  // 8:8 fixed point cell top value.
  s16 mid;  // 8:8 fixed point cell mid value. (Currently unused)  
  s16 bot;  // 8:8 fixed point cell bot value.
#ifdef __YETI_EDITOR__
  s16 tos;  // Top save.
  s16 bos;  // Bot save.
#endif
  u16 lit;  // Cell light setting.
  u8  wtx;  // Cell wall texture id.
  u8  ttx;  // Cell top texture id.
  u8  btx;  // Cell bot texture id.
  u8  mtx;  // Cell mid texture id.
  u8  ent;

  u32 mark;
  entity_t* entities;

  int vcache_size;  
  vertex_t* vcache[YETI_VCACHE_MAX];
} cell_t;

#define CELL_IS_SOLID(A) ((A)->top <= (A)->bot)
#define CELL_MARK(A) ((A)->mark = yeti->mark)
#define CELL_IS_MARKED(A) ((A)->mark == yeti->mark)
#define CELL_UNMARK(A) ((A)->mark = (yeti->mark - 1))
#define CELL_UNMARK_ALL() (yeti->mark++)

/******************************************************************************/

// Keyboard structure. The keys are based on the GBA keyboard set. Previous
// frame settings are stored so you can test for keydown and keyup events
// by checking a keys previous setting.
// eg: selectdown = kb->select & !kb->_select; 

typedef struct
{
  u8 a, _a;
  u8 b, _b;
  u8 select, _select;
  u8 left, _left;
  u8 right, _right;
  u8 up, _up;
  u8 down, _down;
  u8 r, _r;
  u8 l, _l;
} keyboard_t;

/******************************************************************************/

typedef struct
{
  int gravity;    // Gravity force.
  int jumping;    // Jumping force.
  int crawling;   // Crawling height.
  int walking;    // Walking height.
  int ceiling;    // Crouch distance.
} game_t;

/******************************************************************************/

typedef struct
{
  u8 xsize, ysize;
  s16 xpan, ypan;
} surface_t;

typedef struct
{
  game_t game;

  u32 tick;
  u32 mark;
  u32 fps;
  entity_t* camera;

  viewport_t viewport;
  keyboard_t keyboard;

  int is2d;
  matrx_t m;

  int nentities;
  entity_t entities[YETI_ENTITY_MAX];

  int nvpool;
  vertex_t vpool[YETI_VPOOL_MAX];

  int nbullets;
  entity_t* bullets[YETI_BULLET_MAX];

  surface_t surfaces[YETI_TEXTURE_MAX];

  sprite_t overlay;

  vec2_t ray_pos[YETI_RAY_MAX];
  vec2_t ray_vel[YETI_RAY_MAX];
  vec2_t vis[YETI_CELL_MAX];
  cell_t cells[YETI_MAP_HEIGHT][YETI_MAP_WIDTH];

  texture_t* textures;
  color_t* palette;
  lut_t* lighting[4];
} yeti_t;

#define YETI_VERTEX(YETI) (&(YETI)->vpool[(YETI)->nvpool++ & 511])
#define YETI_BULLET(YETI) ((YETI)->bullets[(YETI)->nbullets++ & 15])

/******************************************************************************/

extern void entity_motion(entity_t* e);

/******************************************************************************/

extern void CODE_IN_IWRAM yeti_draw(yeti_t* yeti);
extern void CODE_IN_IWRAM draw_texture(yeti_t* yeti, polyclip_t src, int n, int tid);
extern void CODE_IN_IWRAM draw_poly(vertex_t* p, int n, YETI_ROM texture_t texture, framebuffer_t* dst, lua_t lua);
extern int CODE_IN_IWRAM polygon_clip(yeti_t* yeti, vertex_t** dst, vertex_t** src, int n);
extern void CODE_IN_IWRAM draw_clipped_poly(yeti_t* yeti, polyclip_t src, int n, int tid);

void CODE_IN_IWRAM draw_sprite(
  yeti_t* yeti, vertex_t a, vertex_t b,
  YETI_ROM sprite_t sprite,
  int cl, int ct, int cr, int cb, int mode);
  
extern void CODE_IN_IWRAM draw_sprite_di(
  yeti_t* yeti,
  int x1, int y1, int x2, int y2, int z,
  YETI_ROM sprite_t sprite, int mode);

extern void CODE_IN_IWRAM stretch_sprite_di(
  yeti_t* yeti,
  int x1, int y1, int x2, int y2, int z,
  YETI_ROM sprite_t sprite,
  int u1, int v1, int u2, int v2, int mode);

/******************************************************************************/

extern void palette_overbright(palette_t dst, palette_t src, int brightness);
extern void viewport_to_video(
  rgb555_t* video, int pitch,
  viewport_t* vp,
  int rmask, int gmask, int bmask);
  
extern void yeti_light(yeti_t* yeti, int lightx, int lighty);
extern void yeti_clear_entities(yeti_t* yeti);
extern void yeti_load_map(yeti_t* yeti, rom_map_t* map);
extern void yeti_save_map(yeti_t* yeti, rom_map_t* map);
extern void yeti_init_map(yeti_t* yeti);
extern void yeti_init(yeti_t* yeti,
  framebuffer_t* front,
  framebuffer_t* back,
  texture_t* textures,
  palette_t palette,
  lua_t lua);
extern void yeti_tick(yeti_t* yeti);
extern void yeti_putc(int x, int c);
extern void yeti_puts(int x, char* s);
extern entity_t* yeti_entity(yeti_t* yeti, int x, int y, int z, entity_behaviour_t behaviour);
extern void yeti_default_lighting(yeti_t* yeti);

void CODE_IN_IWRAM matrix_rotate_world(matrx_t m, int alp, int bet, int gam);
void CODE_IN_IWRAM matrix_rotate_object(matrx_t m, int alp, int bet, int gam);
void CODE_IN_IWRAM vertex_project(vertex_t* v);

/******************************************************************************/

void* yeti_memcpy(void* d, void* s, unsigned n);
void* yeti_memset(void *s, int c, unsigned n);

/******************************************************************************/

extern YETI_ROM rom_map_t e1m1;

/******************************************************************************/

extern YETI_ROM texture_t textures[YETI_TEXTURE_MAX];
extern YETI_ROM palette_t palette;
extern YETI_ROM lua_t lua;
extern YETI_ROM int reciprocal[YETI_RECIPROCAL_MAX];
extern YETI_ROM int sintable[YETI_SINTABLE_MAX];

/******************************************************************************/

#include "sprites.h"

#ifdef __cplusplus
} // extern "C"
#endif

#endif


