/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
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

*/
/*
** QGL.H
*/

#ifndef __QGL_H__
#define __QGL_H__

#ifdef _WIN32
#  include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"

#ifdef __unix__
//#include <GL/fxmesa.h>
#include <GL/glx.h>
#endif

#ifndef APIENTRY
#  define APIENTRY
#endif

typedef struct qgl_s // jitglstruct
{
	void (APIENTRY *Accum)(GLenum op, GLfloat value);
	void (APIENTRY *AlphaFunc)(GLenum func, GLclampf ref);
	GLboolean (APIENTRY *AreTexturesResident)(GLsizei n, const GLuint *textures, GLboolean *residences);
	void (APIENTRY *ArrayElement)(GLint i);
	void (APIENTRY *Begin)(GLenum mode);
	void (APIENTRY *BindTexture)(GLenum target, GLuint texture);
	void (APIENTRY *Bitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
	void (APIENTRY *BlendFunc)(GLenum sfactor, GLenum dfactor);
	void (APIENTRY *CallList)(GLuint list);
	void (APIENTRY *CallLists)(GLsizei n, GLenum type, const GLvoid *lists);
	void (APIENTRY *Clear)(GLbitfield mask);
	void (APIENTRY *ClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	void (APIENTRY *ClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	void (APIENTRY *ClearDepth)(GLclampd depth);
	void (APIENTRY *ClearIndex)(GLfloat c);
	void (APIENTRY *ClearStencil)(GLint s);
	void (APIENTRY *ClipPlane)(GLenum plane, const GLdouble *equation);
	void (APIENTRY *Color3b)(GLbyte red, GLbyte green, GLbyte blue);
	void (APIENTRY *Color3bv)(const GLbyte *v);
	void (APIENTRY *Color3d)(GLdouble red, GLdouble green, GLdouble blue);
	void (APIENTRY *Color3dv)(const GLdouble *v);
	void (APIENTRY *Color3f)(GLfloat red, GLfloat green, GLfloat blue);
	void (APIENTRY *Color3fv)(const GLfloat *v);
	void (APIENTRY *Color3i)(GLint red, GLint green, GLint blue);
	void (APIENTRY *Color3iv)(const GLint *v);
	void (APIENTRY *Color3s)(GLshort red, GLshort green, GLshort blue);
	void (APIENTRY *Color3sv)(const GLshort *v);
	void (APIENTRY *Color3ub)(GLubyte red, GLubyte green, GLubyte blue);
	void (APIENTRY *Color3ubv)(const GLubyte *v);
	void (APIENTRY *Color3ui)(GLuint red, GLuint green, GLuint blue);
	void (APIENTRY *Color3uiv)(const GLuint *v);
	void (APIENTRY *Color3us)(GLushort red, GLushort green, GLushort blue);
	void (APIENTRY *Color3usv)(const GLushort *v);
	void (APIENTRY *Color4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
	void (APIENTRY *Color4bv)(const GLbyte *v);
	void (APIENTRY *Color4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
	void (APIENTRY *Color4dv)(const GLdouble *v);
	void (APIENTRY *Color4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	void (APIENTRY *Color4fv)(const GLfloat *v);
	void (APIENTRY *Color4i)(GLint red, GLint green, GLint blue, GLint alpha);
	void (APIENTRY *Color4iv)(const GLint *v);
	void (APIENTRY *Color4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
	void (APIENTRY *Color4sv)(const GLshort *v);
	void (APIENTRY *Color4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
	void (APIENTRY *Color4ubv)(const GLubyte *v);
	void (APIENTRY *Color4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
	void (APIENTRY *Color4uiv)(const GLuint *v);
	void (APIENTRY *Color4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
	void (APIENTRY *Color4usv)(const GLushort *v);
	void (APIENTRY *ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	void (APIENTRY *ColorMaterial)(GLenum face, GLenum mode);
	void (APIENTRY *ColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *CopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
	void (APIENTRY *CopyTexImage1D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
	void (APIENTRY *CopyTexImage2D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	void (APIENTRY *CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	void (APIENTRY *CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	void (APIENTRY *CullFace)(GLenum mode);
	void (APIENTRY *DeleteLists)(GLuint list, GLsizei range);
	void (APIENTRY *DeleteTextures)(GLsizei n, const GLuint *textures);
	void (APIENTRY *DepthFunc)(GLenum func);
	void (APIENTRY *DepthMask)(GLboolean flag);
	void (APIENTRY *DepthRange)(GLclampd zNear, GLclampd zFar);
	void (APIENTRY *Disable)(GLenum cap);
	void (APIENTRY *DisableClientState)(GLenum array);
	void (APIENTRY *DrawArrays)(GLenum mode, GLint first, GLsizei count);
	void (APIENTRY *DrawBuffer)(GLenum mode);
	void (APIENTRY *DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
	void (APIENTRY *DrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *EdgeFlag)(GLboolean flag);
	void (APIENTRY *EdgeFlagPointer)(GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *EdgeFlagv)(const GLboolean *flag);
	void (APIENTRY *Enable)(GLenum cap);
	void (APIENTRY *EnableClientState)(GLenum array);
	void (APIENTRY *End)(void);
	void (APIENTRY *EndList)(void);
	void (APIENTRY *EvalCoord1d)(GLdouble u);
	void (APIENTRY *EvalCoord1dv)(const GLdouble *u);
	void (APIENTRY *EvalCoord1f)(GLfloat u);
	void (APIENTRY *EvalCoord1fv)(const GLfloat *u);
	void (APIENTRY *EvalCoord2d)(GLdouble u, GLdouble v);
	void (APIENTRY *EvalCoord2dv)(const GLdouble *u);
	void (APIENTRY *EvalCoord2f)(GLfloat u, GLfloat v);
	void (APIENTRY *EvalCoord2fv)(const GLfloat *u);
	void (APIENTRY *EvalMesh1)(GLenum mode, GLint i1, GLint i2);
	void (APIENTRY *EvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
	void (APIENTRY *EvalPoint1)(GLint i);
	void (APIENTRY *EvalPoint2)(GLint i, GLint j);
	void (APIENTRY *FeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer);
	void (APIENTRY *Finish)(void);
	void (APIENTRY *Flush)(void);
	void (APIENTRY *Fogf)(GLenum pname, GLfloat param);
	void (APIENTRY *Fogfv)(GLenum pname, const GLfloat *params);
	void (APIENTRY *Fogi)(GLenum pname, GLint param);
	void (APIENTRY *Fogiv)(GLenum pname, const GLint *params);
	void (APIENTRY *FrontFace)(GLenum mode);
	void (APIENTRY *Frustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	GLuint (APIENTRY *GenLists)(GLsizei range);
	void (APIENTRY *GenTextures)(GLsizei n, GLuint *textures);
	void (APIENTRY *GetBooleanv)(GLenum pname, GLboolean *params);
	void (APIENTRY *GetClipPlane)(GLenum plane, GLdouble *equation);
	void (APIENTRY *GetDoublev)(GLenum pname, GLdouble *params);
	GLenum (APIENTRY *GetError)(void);
	void (APIENTRY *GetFloatv)(GLenum pname, GLfloat *params);
	void (APIENTRY *GetIntegerv)(GLenum pname, GLint *params);
	void (APIENTRY *GetLightfv)(GLenum light, GLenum pname, GLfloat *params);
	void (APIENTRY *GetLightiv)(GLenum light, GLenum pname, GLint *params);
	void (APIENTRY *GetMapdv)(GLenum target, GLenum query, GLdouble *v);
	void (APIENTRY *GetMapfv)(GLenum target, GLenum query, GLfloat *v);
	void (APIENTRY *GetMapiv)(GLenum target, GLenum query, GLint *v);
	void (APIENTRY *GetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
	void (APIENTRY *GetMaterialiv)(GLenum face, GLenum pname, GLint *params);
	void (APIENTRY *GetPixelMapfv)(GLenum map, GLfloat *values);
	void (APIENTRY *GetPixelMapuiv)(GLenum map, GLuint *values);
	void (APIENTRY *GetPixelMapusv)(GLenum map, GLushort *values);
	void (APIENTRY *GetPointerv)(GLenum pname, GLvoid* *params);
	void (APIENTRY *GetPolygonStipple)(GLubyte *mask);
	const GLubyte * (APIENTRY *GetString)(GLenum name);
	void (APIENTRY *GetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params);
	void (APIENTRY *GetTexEnviv)(GLenum target, GLenum pname, GLint *params);
	void (APIENTRY *GetTexGendv)(GLenum coord, GLenum pname, GLdouble *params);
	void (APIENTRY *GetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params);
	void (APIENTRY *GetTexGeniv)(GLenum coord, GLenum pname, GLint *params);
	void (APIENTRY *GetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
	void (APIENTRY *GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
	void (APIENTRY *GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
	void (APIENTRY *GetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
	void (APIENTRY *GetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
	void (APIENTRY *Hint)(GLenum target, GLenum mode);
	void (APIENTRY *IndexMask)(GLuint mask);
	void (APIENTRY *IndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *Indexd)(GLdouble c);
	void (APIENTRY *Indexdv)(const GLdouble *c);
	void (APIENTRY *Indexf)(GLfloat c);
	void (APIENTRY *Indexfv)(const GLfloat *c);
	void (APIENTRY *Indexi)(GLint c);
	void (APIENTRY *Indexiv)(const GLint *c);
	void (APIENTRY *Indexs)(GLshort c);
	void (APIENTRY *Indexsv)(const GLshort *c);
	void (APIENTRY *Indexub)(GLubyte c);
	void (APIENTRY *Indexubv)(const GLubyte *c);
	void (APIENTRY *InitNames)(void);
	void (APIENTRY *InterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer);
	GLboolean (APIENTRY *IsEnabled)(GLenum cap);
	GLboolean (APIENTRY *IsList)(GLuint list);
	GLboolean (APIENTRY *IsTexture)(GLuint texture);
	void (APIENTRY *LightModelf)(GLenum pname, GLfloat param);
	void (APIENTRY *LightModelfv)(GLenum pname, const GLfloat *params);
	void (APIENTRY *LightModeli)(GLenum pname, GLint param);
	void (APIENTRY *LightModeliv)(GLenum pname, const GLint *params);
	void (APIENTRY *Lightf)(GLenum light, GLenum pname, GLfloat param);
	void (APIENTRY *Lightfv)(GLenum light, GLenum pname, const GLfloat *params);
	void (APIENTRY *Lighti)(GLenum light, GLenum pname, GLint param);
	void (APIENTRY *Lightiv)(GLenum light, GLenum pname, const GLint *params);
	void (APIENTRY *LineStipple)(GLint factor, GLushort pattern);
	void (APIENTRY *LineWidth)(GLfloat width);
	void (APIENTRY *ListBase)(GLuint base);
	void (APIENTRY *LoadIdentity)(void);
	void (APIENTRY *LoadMatrixd)(const GLdouble *m);
	void (APIENTRY *LoadMatrixf)(const GLfloat *m);
	void (APIENTRY *LoadName)(GLuint name);
	void (APIENTRY *LogicOp)(GLenum opcode);
	void (APIENTRY *Map1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
	void (APIENTRY *Map1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
	void (APIENTRY *Map2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
	void (APIENTRY *Map2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
	void (APIENTRY *MapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
	void (APIENTRY *MapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
	void (APIENTRY *MapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
	void (APIENTRY *MapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
	void (APIENTRY *Materialf)(GLenum face, GLenum pname, GLfloat param);
	void (APIENTRY *Materialfv)(GLenum face, GLenum pname, const GLfloat *params);
	void (APIENTRY *Materiali)(GLenum face, GLenum pname, GLint param);
	void (APIENTRY *Materialiv)(GLenum face, GLenum pname, const GLint *params);
	void (APIENTRY *MatrixMode)(GLenum mode);
	void (APIENTRY *MultMatrixd)(const GLdouble *m);
	void (APIENTRY *MultMatrixf)(const GLfloat *m);
	void (APIENTRY *NewList)(GLuint list, GLenum mode);
	void (APIENTRY *Normal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
	void (APIENTRY *Normal3bv)(const GLbyte *v);
	void (APIENTRY *Normal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
	void (APIENTRY *Normal3dv)(const GLdouble *v);
	void (APIENTRY *Normal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
	void (APIENTRY *Normal3fv)(const GLfloat *v);
	void (APIENTRY *Normal3i)(GLint nx, GLint ny, GLint nz);
	void (APIENTRY *Normal3iv)(const GLint *v);
	void (APIENTRY *Normal3s)(GLshort nx, GLshort ny, GLshort nz);
	void (APIENTRY *Normal3sv)(const GLshort *v);
	void (APIENTRY *NormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *Ortho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	void (APIENTRY *PassThrough)(GLfloat token);
	void (APIENTRY *PixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat *values);
	void (APIENTRY *PixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint *values);
	void (APIENTRY *PixelMapusv)(GLenum map, GLsizei mapsize, const GLushort *values);
	void (APIENTRY *PixelStoref)(GLenum pname, GLfloat param);
	void (APIENTRY *PixelStorei)(GLenum pname, GLint param);
	void (APIENTRY *PixelTransferf)(GLenum pname, GLfloat param);
	void (APIENTRY *PixelTransferi)(GLenum pname, GLint param);
	void (APIENTRY *PixelZoom)(GLfloat xfactor, GLfloat yfactor);
	void (APIENTRY *PointSize)(GLfloat size);
	void (APIENTRY *PolygonMode)(GLenum face, GLenum mode);
	void (APIENTRY *PolygonOffset)(GLfloat factor, GLfloat units);
	void (APIENTRY *PolygonStipple)(const GLubyte *mask);
	void (APIENTRY *PopAttrib)(void);
	void (APIENTRY *PopClientAttrib)(void);
	void (APIENTRY *PopMatrix)(void);
	void (APIENTRY *PopName)(void);
	void (APIENTRY *PrioritizeTextures)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
	void (APIENTRY *PushAttrib)(GLbitfield mask);
	void (APIENTRY *PushClientAttrib)(GLbitfield mask);
	void (APIENTRY *PushMatrix)(void);
	void (APIENTRY *PushName)(GLuint name);
	void (APIENTRY *RasterPos2d)(GLdouble x, GLdouble y);
	void (APIENTRY *RasterPos2dv)(const GLdouble *v);
	void (APIENTRY *RasterPos2f)(GLfloat x, GLfloat y);
	void (APIENTRY *RasterPos2fv)(const GLfloat *v);
	void (APIENTRY *RasterPos2i)(GLint x, GLint y);
	void (APIENTRY *RasterPos2iv)(const GLint *v);
	void (APIENTRY *RasterPos2s)(GLshort x, GLshort y);
	void (APIENTRY *RasterPos2sv)(const GLshort *v);
	void (APIENTRY *RasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *RasterPos3dv)(const GLdouble *v);
	void (APIENTRY *RasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *RasterPos3fv)(const GLfloat *v);
	void (APIENTRY *RasterPos3i)(GLint x, GLint y, GLint z);
	void (APIENTRY *RasterPos3iv)(const GLint *v);
	void (APIENTRY *RasterPos3s)(GLshort x, GLshort y, GLshort z);
	void (APIENTRY *RasterPos3sv)(const GLshort *v);
	void (APIENTRY *RasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	void (APIENTRY *RasterPos4dv)(const GLdouble *v);
	void (APIENTRY *RasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void (APIENTRY *RasterPos4fv)(const GLfloat *v);
	void (APIENTRY *RasterPos4i)(GLint x, GLint y, GLint z, GLint w);
	void (APIENTRY *RasterPos4iv)(const GLint *v);
	void (APIENTRY *RasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
	void (APIENTRY *RasterPos4sv)(const GLshort *v);
	void (APIENTRY *ReadBuffer)(GLenum mode);
	void (APIENTRY *ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
	void (APIENTRY *Rectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
	void (APIENTRY *Rectdv)(const GLdouble *v1, const GLdouble *v2);
	void (APIENTRY *Rectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	void (APIENTRY *Rectfv)(const GLfloat *v1, const GLfloat *v2);
	void (APIENTRY *Recti)(GLint x1, GLint y1, GLint x2, GLint y2);
	void (APIENTRY *Rectiv)(const GLint *v1, const GLint *v2);
	void (APIENTRY *Rects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
	void (APIENTRY *Rectsv)(const GLshort *v1, const GLshort *v2);
	GLint (APIENTRY *RenderMode)(GLenum mode);
	void (APIENTRY *Rotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *Rotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *Scaled)(GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *Scalef)(GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *Scissor)(GLint x, GLint y, GLsizei width, GLsizei height);
	void (APIENTRY *SelectBuffer)(GLsizei size, GLuint *buffer);
	void (APIENTRY *ShadeModel)(GLenum mode);
	void (APIENTRY *StencilFunc)(GLenum func, GLint ref, GLuint mask);
	void (APIENTRY *StencilMask)(GLuint mask);
	void (APIENTRY *StencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
	void (APIENTRY *TexCoord1d)(GLdouble s);
	void (APIENTRY *TexCoord1dv)(const GLdouble *v);
	void (APIENTRY *TexCoord1f)(GLfloat s);
	void (APIENTRY *TexCoord1fv)(const GLfloat *v);
	void (APIENTRY *TexCoord1i)(GLint s);
	void (APIENTRY *TexCoord1iv)(const GLint *v);
	void (APIENTRY *TexCoord1s)(GLshort s);
	void (APIENTRY *TexCoord1sv)(const GLshort *v);
	void (APIENTRY *TexCoord2d)(GLdouble s, GLdouble t);
	void (APIENTRY *TexCoord2dv)(const GLdouble *v);
	void (APIENTRY *TexCoord2f)(GLfloat s, GLfloat t);
	void (APIENTRY *TexCoord2fv)(const GLfloat *v);
	void (APIENTRY *TexCoord2i)(GLint s, GLint t);
	void (APIENTRY *TexCoord2iv)(const GLint *v);
	void (APIENTRY *TexCoord2s)(GLshort s, GLshort t);
	void (APIENTRY *TexCoord2sv)(const GLshort *v);
	void (APIENTRY *TexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
	void (APIENTRY *TexCoord3dv)(const GLdouble *v);
	void (APIENTRY *TexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
	void (APIENTRY *TexCoord3fv)(const GLfloat *v);
	void (APIENTRY *TexCoord3i)(GLint s, GLint t, GLint r);
	void (APIENTRY *TexCoord3iv)(const GLint *v);
	void (APIENTRY *TexCoord3s)(GLshort s, GLshort t, GLshort r);
	void (APIENTRY *TexCoord3sv)(const GLshort *v);
	void (APIENTRY *TexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	void (APIENTRY *TexCoord4dv)(const GLdouble *v);
	void (APIENTRY *TexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	void (APIENTRY *TexCoord4fv)(const GLfloat *v);
	void (APIENTRY *TexCoord4i)(GLint s, GLint t, GLint r, GLint q);
	void (APIENTRY *TexCoord4iv)(const GLint *v);
	void (APIENTRY *TexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
	void (APIENTRY *TexCoord4sv)(const GLshort *v);
	void (APIENTRY *TexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *TexEnvf)(GLenum target, GLenum pname, GLfloat param);
	void (APIENTRY *TexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
	void (APIENTRY *TexEnvi)(GLenum target, GLenum pname, GLint param);
	void (APIENTRY *TexEnviv)(GLenum target, GLenum pname, const GLint *params);
	void (APIENTRY *TexGend)(GLenum coord, GLenum pname, GLdouble param);
	void (APIENTRY *TexGendv)(GLenum coord, GLenum pname, const GLdouble *params);
	void (APIENTRY *TexGenf)(GLenum coord, GLenum pname, GLfloat param);
	void (APIENTRY *TexGenfv)(GLenum coord, GLenum pname, const GLfloat *params);
	void (APIENTRY *TexGeni)(GLenum coord, GLenum pname, GLint param);
	void (APIENTRY *TexGeniv)(GLenum coord, GLenum pname, const GLint *params);
	void (APIENTRY *TexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *TexParameterf)(GLenum target, GLenum pname, GLfloat param);
	void (APIENTRY *TexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
	void (APIENTRY *TexParameteri)(GLenum target, GLenum pname, GLint param);
	void (APIENTRY *TexParameteriv)(GLenum target, GLenum pname, const GLint *params);
	void (APIENTRY *TexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *Translated)(GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *Translatef)(GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *Vertex2d)(GLdouble x, GLdouble y);
	void (APIENTRY *Vertex2dv)(const GLdouble *v);
	void (APIENTRY *Vertex2f)(GLfloat x, GLfloat y);
	void (APIENTRY *Vertex2fv)(const GLfloat *v);
	void (APIENTRY *Vertex2i)(GLint x, GLint y);
	void (APIENTRY *Vertex2iv)(const GLint *v);
	void (APIENTRY *Vertex2s)(GLshort x, GLshort y);
	void (APIENTRY *Vertex2sv)(const GLshort *v);
	void (APIENTRY *Vertex3d)(GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *Vertex3dv)(const GLdouble *v);
	void (APIENTRY *Vertex3f)(GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *Vertex3fv)(const GLfloat *v);
	void (APIENTRY *Vertex3i)(GLint x, GLint y, GLint z);
	void (APIENTRY *Vertex3iv)(const GLint *v);
	void (APIENTRY *Vertex3s)(GLshort x, GLshort y, GLshort z);
	void (APIENTRY *Vertex3sv)(const GLshort *v);
	void (APIENTRY *Vertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	void (APIENTRY *Vertex4dv)(const GLdouble *v);
	void (APIENTRY *Vertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void (APIENTRY *Vertex4fv)(const GLfloat *v);
	void (APIENTRY *Vertex4i)(GLint x, GLint y, GLint z, GLint w);
	void (APIENTRY *Vertex4iv)(const GLint *v);
	void (APIENTRY *Vertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
	void (APIENTRY *Vertex4sv)(const GLshort *v);
	void (APIENTRY *VertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *Viewport)(GLint x, GLint y, GLsizei width, GLsizei height);
	void (APIENTRY *LockArraysEXT)( int, int );
	void (APIENTRY *UnlockArraysEXT)( void );
	void (APIENTRY *PointParameterfEXT)( GLenum param, GLfloat value );
	void (APIENTRY *PointParameterfvEXT)( GLenum param, const GLfloat *value );
	void (APIENTRY *ColorTableEXT)( GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid * );
	void (APIENTRY *SelectTextureSGIS)( GLenum );
	void (APIENTRY *MultiTexCoord2fARB)( GLenum, GLfloat, GLfloat );
	void (APIENTRY *ActiveTextureARB)( GLenum );
	void (APIENTRY *ClientActiveTextureARB)( GLenum );
	void (APIENTRY *MultiTexCoord3fARB)( GLenum, GLfloat, GLfloat, GLfloat );
	void (APIENTRY *MultiTexCoord3fvARB)( GLenum, GLfloat * );


	//****************************************************************************
	// nVidia extensions
	//****************************************************************************
	PFNGLCOMBINERPARAMETERFVNVPROC					CombinerParameterfvNV;
	PFNGLCOMBINERPARAMETERFNVPROC					CombinerParameterfNV;
	PFNGLCOMBINERPARAMETERIVNVPROC					CombinerParameterivNV;
	PFNGLCOMBINERPARAMETERINVPROC					CombinerParameteriNV;
	PFNGLCOMBINERINPUTNVPROC						CombinerInputNV;
	PFNGLCOMBINEROUTPUTNVPROC						CombinerOutputNV;
	PFNGLFINALCOMBINERINPUTNVPROC					FinalCombinerInputNV;
	PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC			GetCombinerInputParameterfvNV;
	PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC			GetCombinerInputParameterivNV;
	PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC			GetCombinerOutputParameterfvNV;
	PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC			GetCombinerOutputParameterivNV;
	PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC		GetFinalCombinerInputParameterfvNV;
	PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC		GetFinalCombinerInputParameterivNV;
	//****************************************************************************

	// === jitwater -- fragment program extensions
	PFNGLGENPROGRAMSARBPROC             GenProgramsARB;
	PFNGLDELETEPROGRAMSARBPROC          DeleteProgramsARB;
	PFNGLBINDPROGRAMARBPROC             BindProgramARB;
	PFNGLPROGRAMSTRINGARBPROC           ProgramStringARB;
	PFNGLPROGRAMENVPARAMETER4FARBPROC   ProgramEnvParameter4fARB;
	PFNGLPROGRAMLOCALPARAMETER4FARBPROC ProgramLocalParameter4fARB;
	// jitwater ===

	// === jitwater - FBO extensions
	PFNGLISRENDERBUFFEREXTPROC                               IsRenderbufferEXT;
	PFNGLBINDRENDERBUFFEREXTPROC                             BindRenderbufferEXT;
	PFNGLDELETERENDERBUFFERSEXTPROC                          DeleteRenderbuffersEXT;
	PFNGLGENRENDERBUFFERSEXTPROC                             GenRenderbuffersEXT;
	PFNGLRENDERBUFFERSTORAGEEXTPROC                          RenderbufferStorageEXT;
	PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC                   GetRenderbufferParameterivEXT;
	PFNGLISFRAMEBUFFEREXTPROC                                IsFramebufferEXT;
	PFNGLBINDFRAMEBUFFEREXTPROC                              BindFramebufferEXT;
	PFNGLDELETEFRAMEBUFFERSEXTPROC                           DeleteFramebuffersEXT;
	PFNGLGENFRAMEBUFFERSEXTPROC                              GenFramebuffersEXT;
	PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC                       CheckFramebufferStatusEXT;
	PFNGLFRAMEBUFFERTEXTURE1DEXTPROC                         FramebufferTexture1DEXT;
	PFNGLFRAMEBUFFERTEXTURE2DEXTPROC                         FramebufferTexture2DEXT;
	PFNGLFRAMEBUFFERTEXTURE3DEXTPROC                         FramebufferTexture3DEXT;
	PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC                      FramebufferRenderbufferEXT;
	PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC          GetFramebufferAttachmentParameterivEXT;
	PFNGLGENERATEMIPMAPEXTPROC                               GenerateMipmapEXT;
	// jitwater ===
	//****************************************************************************
	

#ifdef WIN32
	int   (WINAPI *wChoosePixelFormat)(HDC, CONST PIXELFORMATDESCRIPTOR *);
	int   (WINAPI *wDescribePixelFormat) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
	int   (WINAPI *wGetPixelFormat)(HDC);
	BOOL  (WINAPI *wSetPixelFormat)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
	BOOL  (WINAPI *wSwapBuffers)(HDC);
	BOOL  (WINAPI *wCopyContext)(HGLRC, HGLRC, UINT);
	HGLRC (WINAPI *wCreateContext)(HDC);
	HGLRC (WINAPI *wCreateLayerContext)(HDC, int);
	BOOL  (WINAPI *wDeleteContext)(HGLRC);
	HGLRC (WINAPI *wGetCurrentContext)(VOID);
	HDC   (WINAPI *wGetCurrentDC)(VOID);
	PROC  (WINAPI *wGetProcAddress)(LPCSTR);
	BOOL  (WINAPI *wMakeCurrent)(HDC, HGLRC);
	BOOL  (WINAPI *wShareLists)(HGLRC, HGLRC);
	BOOL  (WINAPI *wUseFontBitmaps)(HDC, DWORD, DWORD, DWORD);
	BOOL  (WINAPI *wUseFontOutlines)(HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT);
	BOOL  (WINAPI *wDescribeLayerPlane)(HDC, int, int, UINT, LPLAYERPLANEDESCRIPTOR);
	int   (WINAPI *wSetLayerPaletteEntries)(HDC, int, int, int, CONST COLORREF *);
	int   (WINAPI *wGetLayerPaletteEntries)(HDC, int, int, int, COLORREF *);
	BOOL  (WINAPI *wRealizeLayerPalette)(HDC, int, BOOL);
	BOOL  (WINAPI *wSwapLayerBuffers)(HDC, UINT);
	BOOL  (WINAPI *wSwapIntervalEXT)(int interval);
	BOOL  (WINAPI *wGetDeviceGammaRampEXT)(unsigned char *, unsigned char *, unsigned char *);
	BOOL  (WINAPI *wSetDeviceGammaRampEXT)(const unsigned char *, const unsigned char *, const unsigned char *);
#endif
} qgl_t;

extern qgl_t qgl;


#ifdef __unix__

// local function in dll
extern void *wGetProcAddress(char *symbol);

// 3dfxSetPaletteEXT shunt
void Fake_glColorTableEXT( GLenum target, GLenum internalformat,
                             GLsizei width, GLenum format, GLenum type,
                             const GLvoid *table );

#endif // unix

/*
** extension constants
*/
#define GL_POINT_SIZE_MIN_EXT				0x8126
#define GL_POINT_SIZE_MAX_EXT				0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT	0x8128
#define GL_DISTANCE_ATTENUATION_EXT			0x8129

#ifdef __sgi
#define GL_SHARED_TEXTURE_PALETTE_EXT		GL_TEXTURE_COLOR_TABLE_SGI
#else
#define GL_SHARED_TEXTURE_PALETTE_EXT		0x81FB
#endif

#define GL_TEXTURE0_SGIS					0x835E
#define GL_TEXTURE1_SGIS					0x835F
#define GL_TEXTURE0_ARB						0x84C0
#define GL_TEXTURE1_ARB						0x84C1

extern int QGL_TEXTURE0, QGL_TEXTURE1, QGL_TEXTURE2;

#define  GL_VERTEX_PROGRAM_ARB				0x8620 // jitwater
#define  GL_FRAGMENT_PROGRAM_ARB			0x8804 // jitwater
#define  GL_PROGRAM_FORMAT_ASCII_ARB		0x8875 // jitwater


#endif
