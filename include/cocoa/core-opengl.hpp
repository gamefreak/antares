// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_COCOA_CORE_OPENGL_HPP_
#define ANTARES_COCOA_CORE_OPENGL_HPP_

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>

namespace antares {
namespace cgl {

#ifdef check
#undef check
#endif
void check(CGLError error);

class PixelFormat {
  public:
    PixelFormat(const CGLPixelFormatAttribute* attrs);

    CGLPixelFormatObj c_obj() const;
    GLint npix() const;

  private:
    CGLPixelFormatObj _pixel_format;
    GLint _npix;

    DISALLOW_COPY_AND_ASSIGN(PixelFormat);
};

class Context {
  public:
    Context(CGLPixelFormatObj pix, CGLContextObj share);

    ~Context();
    CGLContextObj c_obj();

  private:
    CGLContextObj _context;

    DISALLOW_COPY_AND_ASSIGN(Context);
};

}  // namespace cgl
}  // namespace antares

#endif  // ANTARES_COCOA_CORE_OPENGL_HPP_
