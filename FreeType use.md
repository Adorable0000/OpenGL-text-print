### FreeType usage example

<https://freetype.org/>

## CMake build

Copy font file to build directory

```CMake
get_filename_component(PARENT_PATH
                       "${CMAKE_CURRENT_LIST_DIR}"
                       ABSOLUTE)
file(COPY "${PARENT_PATH}/sources/NotoSans-CondensedLight.ttf" DESTINATION ${CMAKE_BINARY_DIR})
```

To use precompiled FreeType dll files on Windows

```CMake
if(WIN32)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
                set(FREETYPE_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/freetype-2.13.2/include")
                set(FREETYPE_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/freetype-2.13.2/win64/freetype.lib")
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		set(FREETYPE_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/freetype-2.13.2/include")
		set(FREETYPE_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/freetype-2.13.2/win32/freetype.lib")
	endif()
endif()
```

Or build from source files

```CMake
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/freetype-2.13.2")
find_library(Freetype freetype PATH_SUFFIXES freetype)
```

On Linux 
```
sudo apt install libfreetype6
```

```CMake
if(UNIX)
    find_package(Freetype REQUIRED)
endif()
```

Find OpenGL lib

```CMake
find_package(OpenGL REQUIRED)
```

After all of that

```CMake
target_include_directories(${PROJECT_NAME} PUBLIC
    ${FREETYPE_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${FREETYPE_LIBRARIES}
    ${OPENGL_LIBRARIES}
)
```

Or if you build FreeType from source

```CMake
target_link_libraries(${PROJECT_NAME} PUBLIC
   freetype)
```

## Example

Some example source code to use FreeType. We use FreeType to render glyphs into textures
and swap between them when painting some text to the screen. To improve this, you can
use texture atlas. Remember that we use screen coordinates, so we should multiply textures
coordinates by orthographic matrix. It can be done with shaders. This part replaces the
functions in rendertext.cpp that named the same way. ftInit() function should be called once.
```cpp
#include "freetype/freetype.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BITMAP_H
#include FT_MODULE_H

#define FT_HINTING_ADOBE     1
#define TT_INTERPRETER_VERSION_40  40

/*!
 * \brief Plot::ftInit FreeType initialization
 * \param font_name Name of font to load
 *
 * FreeType initialization function. Load font .ttf file
 * from the source directory. Remember to use
 * FT_Done_Face() and FT_Done_FreeType() after
 * rendering glyphs. Rendered glyph can be found
 * by calling FT_Face::glyph.
 */
void Plot::ftInit(const char *font_name)
{
  if(FT_Init_FreeType(&ft))
    {
      printf("Error, can't load freetype\n");
      return;
    }

  FT_UInt hinting_engine = FT_HINTING_ADOBE;
  FT_Property_Set( ft, "truetype",
                   "hinting-engine", &hinting_engine );

  FT_UInt interpreter_version = TT_INTERPRETER_VERSION_40;
  FT_Property_Set( ft, "truetype",
                   "interpreter-version",
                   &interpreter_version );

  if(FT_New_Face( ft, font_name, 0, &face ))
    {
      printf("Error, can't load font\n");
      return;
    }

  if(FT_Set_Pixel_Sizes(face, 0, 13))
    {
      printf("Error, can't set size");
      return;
    }

}


/*!
 * \brief Plot::createCharacter
 * \param ch ASCII code of letter or number to create texture
 * Load rendered glyphs from FreeType, create textures and pack them into
 * vector for further use, see \brief Plot::renderText
 */
void Plot::createCharacter(GLbyte ch)
{
  FT_UInt g = FT_Get_Char_Index(face, ch);
  FT_Load_Glyph(face, g, FT_LOAD_DEFAULT);
  FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
        );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  Character character = {
    GLchar(ch),
    texture,
    face->glyph->bitmap.width,
    face->glyph->bitmap.rows,
    face->glyph->bitmap_left,
    face->glyph->bitmap_top,
    static_cast<GLint>(face->glyph->advance.x)
  };
  Characters.push_back(character);
  glBindTexture(GL_TEXTURE_2D, 0);
}

// textures for 0-9 , - e
void Plot::genTexture()
{
  for(GLbyte c = 48; c < 58; c++)
    {
      createCharacter(c);
    }
  createCharacter(44);
  createCharacter(45);
  createCharacter(101);

  FT_Done_Face(face);
  FT_Done_FreeType(ft);
}


/*!
 * \brief Plot::renderText
 * \param program compiled shader program for text render,
 * see \brief Plot::createShader
 * \param text string to render
 * \param xi x position of text to render in ortho coordinates
 * \param yi y position of text to render in ortho coordinates
 * \param scale scaling size of textures
 *
 * Renders text from input string \param text. Find numbers or
 * letters from vector Characters. 
 */
void Plot::renderText(GLuint program, std::string &text, double xi, double yi)
{
  std::string::const_iterator c;
  std::vector<Character> print_characters;
  GLfloat tex_offX = 0;
  GLfloat tex_width = 0;

  for (c = text.begin(); c != text.end(); c++)
    {
      for(uint i = 0; i < Characters.size(); i++)
        {
          if(*c == Characters.at(i).name)
            {
              //ch = Characters.at(i);
              print_characters.push_back(Characters.at(i));
              break;
            }
        }
    }

  if(print_characters.size() == 0) return;

  // we need to make sure that the edge of a texture
  // will always be in the edge of a pixel
  // so the rendered text won't look fuzzy
  double md;
  modf((xi / pixelWidth), &md);
  double x = md * pixelWidth;

  modf((yi / pixelHeight), &md);
  double y = md * pixelHeight;

  glUseProgram(program);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(text_VAO);
  glEnableClientState(GL_VERTEX_ARRAY);

  for(uint i = 0; i < print_characters.size(); i++)
    {
      GLdouble yposH = y + print_characters[i].sizey * pixelHeight;
      GLdouble xpos = x + print_characters[i].BearingX * pixelWidth;    // render glyph considering it's horizontal shift

      GLint y_off = 0;
      if( (print_characters[i].BearingY > print_characters[i].sizey) &&
         (print_characters[i].name == '-') )
        {
          // lift '-' glyph so it will be in the right position
          // for some reason vertical shift of '-' glyph is more then
          // it's height, same problem with '=' and some other operands
          y_off = -(print_characters[i].sizey + print_characters[i].BearingY);
        }
      else
        {
          // otherwise count vertical shift, it can be useful
          // to render 'g', 'p' or any other glyph, in most situations
          // it will be equal to 0
          y_off = (print_characters[i].sizey - print_characters[i].BearingY);
        }

      GLdouble ypos = y - (y_off * pixelHeight);      // render glyph considering it's vertical shift

      GLdouble w = print_characters[i].sizex * pixelWidth;  // width of space for glyph to render

      // *---*
      // | \ |  render glyph using triangles
      // *---*
      GLdouble vertices[6][4] = {
        {xpos,     yposH, 0.0, 0.0},
        {xpos,     y,     0.0, 1.0},
        {xpos + w, y,     1.0, 1.0},

        {xpos,     yposH, 0.0, 0.0},
        {xpos + w, y,     1.0, 1.0},
        {xpos + w, yposH, 1.0, 0.0}
      };

      glBindTexture(GL_TEXTURE_2D, print_characters[i].TextureID);
      glBindBuffer(GL_ARRAY_BUFFER, text_VBO);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glDrawArrays(GL_TRIANGLES, 0, 6);
      
      x += (print_characters[i].Advance >> 6) * pixelWidth;   // position of next glyph to render
    }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisableClientState(GL_VERTEX_ARRAY);
}
```
