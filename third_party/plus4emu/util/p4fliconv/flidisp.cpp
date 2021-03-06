
// p4fliconv: high resolution interlaced FLI converter utility
// Copyright (C) 2007-2017 Istvan Varga <istvanv@users.sourceforge.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// The Plus/4 program files generated by this utility are not covered by the
// GNU General Public License, and can be used, modified, and distributed
// without any restrictions.

#include "p4fliconv.hpp"
#include "flidisp.hpp"

#include <GL/gl.h>
#include <GL/glext.h>

Plus4FLIConvGUI_Display::Plus4FLIConvGUI_Display(Plus4FLIConvGUI& gui_,
                                                 int xx, int yy, int ww, int hh,
                                                 const char *lbl,
                                                 bool isDoubleBuffered)
  : Plus4Emu::OpenGLDisplay(xx, yy, ww, hh, lbl, isDoubleBuffered),
    gui(gui_)
{
}

Plus4FLIConvGUI_Display::~Plus4FLIConvGUI_Display()
{
}

bool Plus4FLIConvGUI_Display::checkEvents()
{
  if (gui.previewEnabled && !gui.busyFlag) {
    double  t = gui.emulationTimer.getRealTime();
    gui.emulationTimer.reset();
    t = (t > 0.0 ? (t < 0.1 ? t : 0.1) : 0.0);
    int     nCycles = int(t * 17734475.0 + 0.5);
    gui.ted->run(nCycles);
    return Plus4Emu::OpenGLDisplay::checkEvents();
  }
  if (!textureID) {
    // initialize texture on first call
    Plus4Emu::OpenGLDisplay::checkEvents();
  }
  if (gui.emulationTimer.getRealTime() >= 1.0) {
    gui.emulationTimer.reset();
    return true;
  }
  return false;
}

void Plus4FLIConvGUI_Display::draw()
{
  if (gui.previewEnabled && !gui.busyFlag) {
    Plus4Emu::OpenGLDisplay::draw();
    return;
  }
  if (!textureID) {
    // initialize texture on first call
    Plus4Emu::OpenGLDisplay::draw();
  }
  glViewport(0, 0, GLsizei(this->w()), GLsizei(this->h()));
  glPushMatrix();
  glOrtho(0.0, 1.0, 1.0, 0.0, 0.0, 1.0);
  GLuint  textureID_ = GLuint(textureID);
  GLint   savedTextureID = 0;
  glEnable(GL_TEXTURE_2D);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &savedTextureID);
  glBindTexture(GL_TEXTURE_2D, textureID_);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glPixelTransferf(GL_RED_SCALE, GLfloat(1));
  glPixelTransferf(GL_RED_BIAS, GLfloat(0));
  glPixelTransferf(GL_GREEN_SCALE, GLfloat(1));
  glPixelTransferf(GL_GREEN_BIAS, GLfloat(0));
  glPixelTransferf(GL_BLUE_SCALE, GLfloat(1));
  glPixelTransferf(GL_BLUE_BIAS, GLfloat(0));
  glPixelTransferf(GL_ALPHA_SCALE, GLfloat(1));
  glPixelTransferf(GL_ALPHA_BIAS, GLfloat(0));
  glDisable(GL_BLEND);
  for (size_t yc = 0; yc < 576; yc += 288) {
    // load texture
    for (size_t yoffs = 0; yoffs < 288; yoffs++) {
      size_t  txtYoffs = yoffs & 7;
      for (size_t xc = 0; xc < 768; xc++) {
        unsigned char *p =
            &(gui.imageFileData[(((yc + yoffs) * 768) + xc) * 3]);
        uint32_t  tmp =
            uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16)
            | 0xFF000000U;
        textureBuffer32[(txtYoffs * 768) + xc] = tmp;
      }
      if (txtYoffs == 7) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, GLint(yoffs - 7), 768, 8,
                        GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, textureSpace);
      }
    }
    // update display
    GLfloat ycf0 = GLfloat(double(int(yc)) / 576.0);
    GLfloat ycf1 = GLfloat(double(int(yc + 288)) / 576.0);
    glBegin(GL_QUADS);
    glTexCoord2f(GLfloat(0.0), GLfloat(0.001 / 512.0));
    glVertex2f(GLfloat(0.0), ycf0);
    glTexCoord2f(GLfloat(768.0 / 1024.0), GLfloat(0.001 / 512.0));
    glVertex2f(GLfloat(1.0), ycf0);
    glTexCoord2f(GLfloat(768.0 / 1024.0), GLfloat(287.999 / 512.0));
    glVertex2f(GLfloat(1.0), ycf1);
    glTexCoord2f(GLfloat(0.0), GLfloat(287.999 / 512.0));
    glVertex2f(GLfloat(0.0), ycf1);
    glEnd();
  }
  // clean up
  glBindTexture(GL_TEXTURE_2D, GLuint(savedTextureID));
  glPopMatrix();
  glFlush();
}

