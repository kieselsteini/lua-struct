/*
 * lua-struct  - a lua module to read/write binary data
 *
 * Copyright (c) 2012 Sebastian Steinhauer <s.steinhauer@yahoo.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define LUASTRUCT_VERSION   "lua-struct 0.1.0"

#define BO_LE   1234  /* little endian */
#define BO_BE   4321  /* big endian */

/* detect machine byteorder... */
#if  defined(__i386__) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || \
  (defined(__alpha__) || defined(__alpha)) || \
  defined(__arm__) || defined(ARM) || \
  (defined(__mips__) && defined(__MIPSEL__)) || \
  defined(__SYMBIAN32__) || \
  defined(__x86_64__) || \
  defined(__LITTLE_ENDIAN__)
#define BYTEORDER BO_LE
#define BYTEORDER_STRING    "le"
#else
#define BYTEORDER BO_BE
#define BYTEORDER_STRING    "be"
#endif

static unsigned short _swap16(unsigned short i)
{
  return (i << 8) | (i >> 8);
}

static unsigned int _swap32(unsigned int i)
{
  return (i << 24)| ((i << 8) & 0x00FF0000) |
    ((i >> 8) & 0x0000FF00) | (i >> 24);
}

static unsigned short swap16(unsigned short x, int b)
{
#if BYTEORDER == BO_LE
  if (b == BO_LE) return x;
  else return _swap16(x);
#else
  if (b == BO_LE) return _swap16(x);
  else return _swap16(x);
#endif
}

static unsigned int swap32(unsigned int x, int b)
{
#if BYTEORDER == BO_LE
  if (b == BO_LE) return x;
  else return _swap32(x);
#else
  if (b == BO_LE) return _swap32(x);
  else return x;
#endif
}


static const char* avail(lua_State *L, size_t needed,
    const char **data, size_t *size)
{
  const char *ptr = *data;
  if (*size < needed)
    luaL_error(L, LUA_QL("%d") " more byte(s) needed", needed);
  *data += needed;
  *size -= needed;
  return ptr;
}

/* pack binary data */
static int pack(lua_State *L)
{
  int arg, byteorder = BYTEORDER;
  luaL_Buffer buf;
  const char *fmtstr = luaL_checkstring(L, 1);
  luaL_buffinit(L, &buf);
  for (arg = 2; *fmtstr; arg++, fmtstr++) {
    switch (*fmtstr) {
      case '<': /* little endian */
        byteorder = BO_LE;
        arg--;  /* don't count this as argument */
        break;
      case '>': /* big endian */
        byteorder = BO_BE;
        arg--;  /* don't count this as argument */
        break;
      case '@': /* native order */
        byteorder = BYTEORDER;
        arg--;  /* don't count this as argument */
        break;
      case 'x': /* padding byte */
        {
          const char x = 0;
          luaL_addlstring(&buf, &x, 1);
          arg--;  /* don't count this as argument */
          break;
        }
      case 'b': /* signed byte */
        {
          char byte = (char) luaL_checkinteger(L, arg);
          luaL_addlstring(&buf, (const char*) &byte, 1);
          break;
        }
      case 'B': /* unsigned byte */
        {
          unsigned char byte = (unsigned char) luaL_checkinteger(L, arg);
          luaL_addlstring(&buf, (const char*) &byte, 1);
          break;
        }
      case 'f': /* float */
        {
          float f = (float) luaL_checknumber(L, arg);
          luaL_addlstring(&buf, (const char*) &f, sizeof(float));
          break;
        }
      case 'd': /* double */
        {
          double d = (double) luaL_checknumber(L, arg);
          luaL_addlstring(&buf, (const char*) &d, sizeof(double));
          break;
        }
      case '?': /* boolean */
        {
          char b = (char) lua_toboolean(L, arg);
          luaL_addlstring(&buf, (const char*) &b, 1);
          break;
        }
      case 's': /* string */
        {
          int bytes = 0;
          size_t slen;
          const char *str = luaL_checklstring(L, arg, &slen);
          for (fmtstr++;; fmtstr++) {
            if (*fmtstr >= '0' && *fmtstr <= '9')
              bytes = (bytes * 10) + (*fmtstr - '0');
            else {
              fmtstr--; /* go back */
              break;
            }
          }
          if (bytes > 0) {
            if (bytes <= slen)
              luaL_addlstring(&buf, str, bytes);
            else {
              const char b = 0;
              luaL_addlstring(&buf, str, slen);
              for (bytes -= slen; bytes > 0; bytes--) {
                luaL_addlstring(&buf, &b, 1);
              }
            }
          }
          break;
        }
      case 'h': /* short */
        {
          short s = (short)
            swap16((unsigned short) luaL_checkinteger(L, arg), byteorder);
          luaL_addlstring(&buf, (const char*) &s, sizeof(short));
          break;
        }
      case 'H': /* unsigned short */
        {
          unsigned short s = (unsigned short)
            swap16((unsigned short) luaL_checkinteger(L, arg), byteorder);
          luaL_addlstring(&buf, (const char*) &s, sizeof(short));
          break;
        }
      case 'i': /* int */
        {
          int i = (int)
            swap32((unsigned int) luaL_checkinteger(L, arg), byteorder);
          luaL_addlstring(&buf, (const char*) &i, sizeof(int));
          break;
        }
      case 'I': /* unsigned int */
        {
          unsigned int i = (unsigned int)
            swap32((unsigned int) luaL_checkinteger(L, arg), byteorder);
          luaL_addlstring(&buf, (const char*) &i, sizeof(int));
          break;
        }
      default:
        luaL_error(L, "unknown format character " LUA_QL("%c"), *fmtstr);
    }
  }
  luaL_pushresult(&buf);
  return 1;
}

/* unpack binary data */
static int unpack(lua_State *L)
{
  size_t size;
  int arg, byteorder = BYTEORDER;
  const char *data;
  const char *fmtstr = luaL_checkstring(L, 1);
  data = luaL_checklstring(L, 2, &size);
  for (arg = 0; *fmtstr; fmtstr++, arg++) {
    switch (*fmtstr) {
      case '<': /* little endian */
        byteorder = BO_LE;
        arg--; /* don't count this as value */
        break;
      case '>': /* big endian */
        byteorder = BO_BE;
        arg--; /* don't count this as value */
        break;
      case '@': /* native byteorder */
        byteorder = BYTEORDER;
        arg--; /* don't count this as value */
        break;
      case 'x': /* padding byte */
        {
          avail(L, 1, &data, &size);  /* eat 1 byte */
          arg--; /* don't count this as value */
          break;
        }
      case 'b': /* signed byte */
        {
          lua_pushinteger(L, *((char*) avail(L, 1, &data, &size)));
          break;
        }
      case 'B': /* unsigned byte */
        {
          lua_pushinteger(L, *((unsigned char*)
                avail(L, 1, &data, &size)));
          break;
        }
      case 'f': /* float */
        {
          lua_pushnumber(L, *((float*)
                avail(L, sizeof(float), &data, &size)));
          break;
        }
      case 'd': /* double */
        {
          lua_pushnumber(L, *((double*)
                avail(L, sizeof(double), &data, &size)));
          break;
        }
      case '?': /* boolean */
        {
          lua_pushboolean(L, *((char*) avail(L, 1, &data, &size)));
          break;
        }
      case 's': /* string */
        {
          int bytes = 0;
          for (fmtstr++;; fmtstr++) {
            if (*fmtstr >= '0' && *fmtstr <= '9')
              bytes = (bytes * 10) + (*fmtstr - '0');
            else {
              fmtstr--; /* go back */
              break;
            }
          }
          if (bytes > 0) {
            lua_pushlstring(L, ((const char*)
                  avail(L, bytes, &data, &size)), bytes);
          }
          break;
        }
      case 'h': /* short */
        {
          lua_pushinteger(L, (short) swap16(*((unsigned short*)
                  avail(L, sizeof(short), &data, &size)), byteorder));
          break;
        }
      case 'H': /* unsigned short */
        {
          lua_pushinteger(L, (unsigned short) swap16(*((unsigned short*)
                  avail(L, sizeof(short), &data, &size)), byteorder));
          break;
        }
      case 'i': /* int */
        {
          lua_pushinteger(L, (int) swap32(*((unsigned int*)
                  avail(L, sizeof(int), &data, &size)), byteorder));
          break;
        }
      case 'I': /* unsigned int */
        {
          lua_pushinteger(L, (unsigned int) swap32(*((unsigned int*)
                  avail(L, sizeof(int), &data, &size)), byteorder));
          break;
        }
      default:
        luaL_error(L, "unknown format character " LUA_QL("%c"), *fmtstr);
    }
  }
  return arg;
}

/* calculate the size of a format string */
static int size(lua_State *L)
{
  size_t bytes = 0;
  const char *fmtstr = luaL_checkstring(L, 1);
  for (; *fmtstr; fmtstr++) {
    switch (*fmtstr) {
      case '@':
      case '<':
      case '>':
        break;
      case 'b':
      case 'B':
        bytes++;
        break;
      case 'h':
      case 'H':
        bytes += sizeof(short);
        break;
      case 'i':
      case 'I':
        bytes += sizeof(int);
        break;
      case 'f':
        bytes += sizeof(float);
        break;
      case 'd':
        bytes += sizeof(double);
        break;
      case 's':
        {
          int b = 0;
          for (fmtstr++;; fmtstr++) {
            if (*fmtstr >= '0' && *fmtstr <= '9')
              b = (b * 10) + (*fmtstr - '0');
            else {
              fmtstr--; /* go back */
              break;
            }
          }
          bytes += b;
          break;
        }
      default:
        luaL_error(L, "unknown format character " LUA_QL("%c"), *fmtstr);
    }
  }
  lua_pushinteger(L, bytes);
  return 1;
}

static luaL_Reg struct_functions[] = {
  {"pack", pack},
  {"unpack", unpack},
  {"size", size},
  {NULL, NULL}
};

/* set some variables for our package */
static void registervars(lua_State *L)
{
  lua_pushliteral(L, LUASTRUCT_VERSION);
  lua_setfield(L, -2, "_VERSION");
  lua_pushliteral(L, BYTEORDER_STRING);
  lua_setfield(L, -2, "byteorder");
  lua_pushliteral(L, "@<>bBhHiIfds");
  lua_setfield(L, -2, "formatchars");
}

#if LUA_VERSION_NUM >= 502
/* build lua 5.2 compatible module */
LUAMOD_API int luaopen_struct(lua_State *L)
{
  luaL_newlib(L, struct_functions);
  registervars(L);
  return 1;
}
#else
/* build an old lua 5.1 module */
extern int luaopen_struct(lua_State *L)
{
  luaL_openlib(L, "struct", struct_functions, 0);
  registervars(L);
  return 1;
}
#endif
/* vim: set ts=2 sts=2 sw=2 et: */
