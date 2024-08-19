#include "rendertext.h"
#include <iostream>
#include <math.h>
#include <QPainter>

#define MAX_CH 7


const char *vertexShaderText =
    "#version 330 core\n"
    "layout (location = 0) in vec4 vertex;\n"
    "uniform mat4 ModelViewProjectionMatrix;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    " {\n"
    "   gl_Position = ModelViewProjectionMatrix * vec4(vertex.xy, 0.0, 1.0);\n"
    "   TexCoord = vertex.zw;\n"
    " }\n";


const char *fragmentShaderText =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 Color;\n"
    "uniform sampler2D text;\n"
    "vec3 textColor = vec3(0.0, 0.0, 0.0);\n"
    "void main()\n"
    " {\n"
    "   vec4 sampled = vec4(0.0, 0.0, 0.0, texture(text, TexCoord).r);\n"
    "   Color = vec4(textColor, 1.0) * sampled;\n"
    " }\n";


RenderText::RenderText()
{
  textHeight = 0;
  textMaxWidth = 0;
}


RenderText::~RenderText()
{

}


/*!
 * \brief RenderText::initTextRender
 *
 * Initialize OpenGL functions, create shader program
 * and generate text textures. Should be called in
 * \fn initializeGL() once.
 */
void RenderText::initTextRender()
{
  initializeOpenGLFunctions();

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  text_prog = createShader(vertexShaderText, fragmentShaderText);
  glUseProgram(text_prog);

  genTextures();

  glGenBuffers(1, &text_VBO);
  glGenVertexArrays(1, &text_VAO);

  glBindVertexArray(text_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, text_VBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_DOUBLE, GL_FALSE, 4 * sizeof(GLdouble), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}


/*!
 * \brief RenderText::renderText
 *
 * Attach compiled shader program, bind texture with rendered glyphs,
 * bind vertex buffer object where we pack screen and
 * texture coordinates. Should be called in \fn paintGL() or
 * any other paint event.
 */
void RenderText::renderText()
{

  glUseProgram(text_prog);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(text_VAO);
  glEnableClientState(GL_VERTEX_ARRAY);
  glBindTexture(GL_TEXTURE_2D, Texture);
  glBindBuffer(GL_ARRAY_BUFFER, text_VBO);
  for(uint i = 0; i < textBoxes.size(); i++)
    {
      glBufferData(GL_ARRAY_BUFFER, textBoxes[i].pos.size() * sizeof(GLdouble),
                   textBoxes[i].pos.data(), GL_DYNAMIC_DRAW);
      glDrawArrays(GL_TRIANGLES, 0, textBoxes[i].pos.size()/4);
    }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
  glDisableClientState(GL_VERTEX_ARRAY);

}


/*!
 * \brief RenderText::setProjMatrix
 * \param pr projection matrix values
 *
 * Set projection matrix.
 * After that, \fn reserveAxisSpace()
 * should be called to reserve enough space in parent
 * widget, so we don't bother values that
 * should be printed.
 */
void RenderText::setProjMatrix(Proj &pr)
{
  proj.left = pr.left;
  proj.bottom = pr.bottom;
  proj.right = pr.right;
  proj.top = pr.top;
}


/*!
 * \brief RenderText::getProjMatrix
 * \return projection matrix
 */
Proj RenderText::getProjMatrix()
{
  return proj;
}


/*!
 * \brief RenderText::updateShaderMatrix
 */
void RenderText::updateShaderMatrix()
{
  // Create projection matrix to load to shaders
  float proj_matrix[4][4] = {
    { 2/(proj.right - proj.left),            0,               0,        0 },
    {              0,             2/(proj.top - proj.bottom), 0,        0 },
    {              0,                        0,              -2/(-1-1), 0 },
    { -(proj.right + proj.left)/(proj.right - proj.left),
      -(proj.top + proj.bottom)/(proj.top - proj.bottom),
      -(-1+1)/(-1-1), 1   }
  };
  // Update projection matrix in shaders
  glUseProgram(text_prog);
  glUniformMatrix4fv(glGetUniformLocation(text_prog, "ModelViewProjectionMatrix"), 1, GL_FALSE, &proj_matrix[0][0]);
}


/*!
 * \brief RenderText::hintToPixel
 * \param num number to hint
 * \param pixelsize size of pixel that we need to hint \param num to.
 * \return hinted \param num
 *
 * We need to make sure that some varriables are put exactly
 * on edges of pixels. It should be done due to subpixel
 * interpolation of text, so the rendered textures won't
 * look fuzzy.
 */
double RenderText::hintToPixel(double num, double pixelsize)
{
  double md;
  double frac = modf((num / pixelsize), &md);   // hint num to be in integer size in pixels
  if (1.0 - frac < 0.01)                        // handle almost integer size in pixels
    ++md;
  if (1.0 - frac > 1.0)                         // handle if value is negative
    --md;
  return (md * pixelsize);
}


/*!
 * \brief RenderText::reserveAxisSpace
 * \param width
 * \param height
 *
 * Spesific function for me. Used to print text
 * in the left and bottom part of the screen and
 * don't paint over some objects on OpenGL scene
 */
void RenderText::reserveSpace(int width, int height)
{
  // example value to test reserving enough space for text
  int pix_x_offs = textMaxWidth + characterWidth;  // text width * number of characters + offset
  int pix_y_offs = textHeight + 2;       // text height + offset

  double del_w = pix_x_offs * pixelWidth / static_cast<double>(width);                  // how much pixel size will change after reserving space for text
  double del_h = pix_y_offs * pixelHeight / static_cast<double>(height);

  proj.left -= (pixelWidth + del_w) * pix_x_offs;                         // reserve enough space for text texture
  proj.bottom -= (pixelHeight + del_h) * pix_y_offs;
  proj.right += pixelWidth * 10;
  proj.top += pixelHeight * (textHeight / 2);

  pixelWidth = (proj.right - proj.left) / static_cast<double>(width);    // recalculate pixel sizes
  pixelHeight = (proj.top - proj.bottom) / static_cast<double>(height);

  proj.left = hintToPixel(proj.left, pixelWidth);                      // due to sublixel text interpolation, we need to make projection matrix
  proj.right = hintToPixel(proj.right, pixelWidth);                    // to be equal to integer number in pixels
  proj.bottom = hintToPixel(proj.bottom, pixelHeight);
  proj.top = hintToPixel(proj.top, pixelHeight);
}


/*!
 * \brief Plot::createShader
 * \param vertexShader
 * \param fragmentShader
 * \return compiled shader program
 *
 * Compile passed OpenGL shaders to OpenGL program.
 * Use compiled program to update uniforms and load needed
 * vertex array objects (if OpenGl version supports it) with
 * value to pass to shaders. To do so, you need to use
 * \fn glUseProgram(...) with returned variable and \fn glBindVertexArray()
 * to change buffer without the need to change values, or \fn glUniform()
 * with the name of uniform to update it's values
 */
int RenderText::createShader(const char* vertexShader, const char* fragmentShader)
{
  GLint state = 0;
  GLint loglen = 0;

  GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vertexShader, NULL);
  glCompileShader(vertex);
  glGetShaderiv(vertex, GL_COMPILE_STATUS, &state);
  if(state == GL_FALSE)
    {
      glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &loglen);
      GLchar *infolog = new GLchar[loglen];
      glGetShaderInfoLog(vertex, loglen, NULL, infolog);
      printf("vertex shader failed, %s\n", infolog);
      delete[] infolog;
    }

  GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fragmentShader, NULL);
  glCompileShader(fragment);
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &state);
  if(state == GL_FALSE)
    {
      glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &loglen);
      GLchar *infolog = new GLchar[loglen];
      glGetShaderInfoLog(fragment, loglen, NULL, infolog);
      printf("fragment shader failed, %s\n", infolog);
      delete[] infolog;
    }

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &state);
  if(state == GL_FALSE)
    {
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &loglen);
      GLchar *infolog = new GLchar[loglen];
      glGetProgramInfoLog(program, loglen, NULL, infolog);
      printf("program shader link failed, %s\n", infolog);
      delete[] infolog;
    }

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  return program;
}


/*!
 * \brief RenderText::getCharFromFloat
 * \param number vector where char numbers will be stored
 * \param input float value to divide by char numbers
 *
 * Simple function to slice float number into char characters
 */
void RenderText::getCharFromFloat(std::vector<char> *number, double input)
{
  double frac_part = 0;
  double int_part = 0;
  int offs_begin = 0;
  int offs_end = 0;

  number->reserve(MAX_CH);

  frac_part = modf((input), &int_part);

  if(int_part < 0)
    {
      number->push_back('-');
      offs_begin = 1;
      int_part *= -1;
      frac_part *= -1;
    }

  if(int_part == 0)
    {
      number->push_back('0');
    }


  double add_int_part = 0;
  char to_vector;
  while (int_part > 0)
    {
      add_int_part = modf((int_part / 10), &int_part);
      add_int_part *= 10;
      to_vector = static_cast<char>(char('0') + (add_int_part + 0.00001));
      number->push_back(to_vector);
    }

  if(frac_part > 0)
    {
      number->push_back(',');
      ++offs_end;
    }
  else
    {
      std::reverse((number->begin() + offs_begin), number->end());
      return;
    }

  frac_part += 0.00000001;
  double add_frac_part = 0;
  double a = 0.0000001;
  while ( frac_part > 0)
    {
      frac_part = modf((frac_part * 10), &add_frac_part);
      to_vector = static_cast<char>(char('0')  + (add_frac_part));
      number->push_back(to_vector);
      ++offs_end;
      a *= 10;
      if((frac_part < a) || (offs_end > MAX_CH - 2))
        {
          break;
        }
    }

  std::reverse((number->begin() + offs_begin), (number->end() - offs_end));
}


/*!
 * \brief RenderText::setText
 * \param y
 * \param x
 *
 *
 */
void RenderText::setText(std::vector<double> &y, std::vector<double> &x)
{
  textBoxes.clear();
  textBoxes.resize(y.size() + x.size());

  textMaxWidth = 0;
  textHeight = characterHeight;
  int max = 0;

  for(uint i = 0; i < y.size(); i++)
    {
      textBoxes[i].num = y[i];
      getCharFromFloat(&textBoxes[i].print, y[i]);
      textBoxes[i].printInfo.resize(textBoxes[i].print.size());
      textBoxes[i].ar = Arrange::vertical;
      textBoxes[i].pos.resize(textBoxes[i].print.size() * 6 * 4);
      textBoxes[i].width = textBoxes[i].print.size() * characterWidth;

      max = textBoxes[i].width;
      if(max > textMaxWidth)
        {
          textMaxWidth = max;
        }

      for(uint j = 0; j < textBoxes[i].print.size(); j++)
        {
          textBoxes[i].printInfo[j].Advance = Characters.at(textBoxes[i].print[j]).Advance;
          textBoxes[i].printInfo[j].Bearing = Characters.at(textBoxes[i].print[j]).BearingX;
          textBoxes[i].printInfo[j].texX = Characters.at(textBoxes[i].print[j]).texX;
        }
    }

  for(uint i = y.size(); i < y.size() + x.size(); i++)
    {
      textBoxes[i].num = x[i - y.size()];
      getCharFromFloat(&textBoxes[i].print, x[i - y.size()]);

      textBoxes[i].printInfo.resize(textBoxes[i].print.size());
      textBoxes[i].ar = Arrange::horizontal;
      textBoxes[i].pos.resize(textBoxes[i].print.size() * 6 * 4);

      textBoxes[i].width = textBoxes[i].print.size() * characterWidth;

      for(uint j = 0; j < textBoxes[i].print.size(); j++)
        {
          textBoxes[i].printInfo[j].Advance = Characters.at(textBoxes[i].print[j]).Advance;
          textBoxes[i].printInfo[j].Bearing = Characters.at(textBoxes[i].print[j]).BearingX;
          textBoxes[i].printInfo[j].texX = Characters.at(textBoxes[i].print[j]).texX;
        }
    }
}


/*!
 * \brief RenderText::updateTextPositions
 *
 * Update positions of text of screen.
 *
 * Call after resize events and \fn setText() calls.
 * Remember, we use triangles to render textures from texture atlas.
 * Buffer is packed [Screen X | Screen Y | Tex X | Tex Y] x6
 * Screen coords:
 * 0---*    3---5
 * | \ |    | \ |
 * 1---2    *---4
 * Texture coords:
 * 1---2    *---4
 * | / |    | / |
 * 0---*    3---5
 */
void RenderText::updateTextPositions()
{
  double xpos = 0;
  double ypos = 0;
  double x_hinted = 0;
  double xpos_hinted = 0;
  double ypos_hinted = 0;
  double tex_pos_x = 0;
  double char_width = characterWidth * pixelWidth;
  double char_height = characterHeight * pixelHeight;

  for(uint i = 0; i < textBoxes.size(); i++ )
    {
      if(textBoxes[i].ar == horizontal)
        {
          xpos = textBoxes[i].num - (textBoxes[i].width * pixelWidth / 2);
          ypos = proj.bottom + (pixelHeight * 1);
        }
      if(textBoxes[i].ar == vertical)
        {
          xpos = proj.left + (pixelWidth * 4);
          ypos = textBoxes[i].num - (char_height / 2);
        }

      x_hinted = hintToPixel(xpos, pixelWidth);
      ypos_hinted = hintToPixel(ypos, pixelHeight);
      for (uint j = 0; j < textBoxes[i].pos.size(); j+=24)
        {
          xpos_hinted = x_hinted + ( textBoxes[i].printInfo[j/24].Bearing * pixelWidth );
          tex_pos_x = textBoxes[i].printInfo[j/24].texX;


          textBoxes[i].pos[j] = xpos_hinted;
          textBoxes[i].pos[j + 1] = ypos_hinted + char_height;
          textBoxes[i].pos[j + 2] = tex_pos_x;
          textBoxes[i].pos[j + 3] = 0;

          textBoxes[i].pos[j + 4] = xpos_hinted;
          textBoxes[i].pos[j + 5] = ypos_hinted;
          textBoxes[i].pos[j + 6] = tex_pos_x;
          textBoxes[i].pos[j + 7] = 1.0;

          textBoxes[i].pos[j + 8] = xpos_hinted + char_width;
          textBoxes[i].pos[j + 9] = ypos_hinted;
          textBoxes[i].pos[j + 10] = tex_pos_x + texAtlas.colFactor;
          textBoxes[i].pos[j + 11] = 1.0;

          textBoxes[i].pos[j + 12] = xpos_hinted;
          textBoxes[i].pos[j + 13] = ypos_hinted + char_height;
          textBoxes[i].pos[j + 14] = tex_pos_x;
          textBoxes[i].pos[j + 15] = 0;

          textBoxes[i].pos[j + 16] = xpos_hinted + char_width;
          textBoxes[i].pos[j + 17] = ypos_hinted;
          textBoxes[i].pos[j + 18] = tex_pos_x + texAtlas.colFactor;
          textBoxes[i].pos[j + 19] = 1.0;

          textBoxes[i].pos[j + 20] = xpos_hinted + char_width;
          textBoxes[i].pos[j + 21] = ypos_hinted + char_height;
          textBoxes[i].pos[j + 22] = tex_pos_x + texAtlas.colFactor;
          textBoxes[i].pos[j + 23] = 0;

          x_hinted += textBoxes[i].printInfo[j/24].Advance * pixelWidth;
        }
    }
}


/*!
 * \brief RenderText::genTextures
 *
 * Create QString of characters to load into textures
 */
void RenderText::genTextures()
{
  QString c = "0123456789,-e";
  createQCharacters(c);
}


/*!
 * \brief Plot::createQCharacter
 * \param q_str QString of charactes to load into textures,
 *    see \brief genTextures
 *
 * Example of how to load glyphs into textures atlas using QPainter.
 * Create qrey colored QImage and paint characters into it.
 * Then create textures and load pixels from QImage.
 * Remember that Qt use AGRB format of images, so only
 * every 4 pixel need to be packed into texture.
 */
void RenderText::createQCharacters(QString &q_str)
{
  Characters.reserve(q_str.length());

  QFont qfont;
  qfont.setStyleStrategy(QFont::PreferAntialias);
  qfont.setPointSize(9);
//  qfont.setPixelSize(12);

  QFontMetrics qftmetrics(qfont);

  uint width =  qftmetrics.horizontalAdvance(q_str[0]);
  uint height = qftmetrics.height();

  characterWidth = width;
  characterHeight = height;

  QImage qimg(width * q_str.size(), height, QImage::Format_Grayscale8);

  QPainter qpaint(&qimg);
  qpaint.setFont(qfont);
  qpaint.setBrush(Qt::white);
  qpaint.setPen(Qt::white);
  qpaint.setRenderHint(QPainter::TextAntialiasing, true);
  qpaint.setRenderHint(QPainter::SmoothPixmapTransform, true);
  qpaint.fillRect(0, 0, width * q_str.size(), height, Qt::black);

  texAtlas.cols = q_str.size();
  texAtlas.rows = 1;
  texAtlas.colFactor = 1/static_cast<GLdouble>(texAtlas.cols);
  texAtlas.rowFactor = 1/static_cast<GLdouble>(texAtlas.rows);
  int curr_row = 0;

  for(QString::Iterator c = q_str.begin(); c != q_str.end(); c++)
    {
      char ch = QString(*c).toLocal8Bit().front();
      qpaint.drawText((curr_row * width) - qftmetrics.leftBearing(*c),
                      height - qftmetrics.descent(), (*c));

      Character character = {
        width,
        height,
        qftmetrics.leftBearing(*c),
        qftmetrics.ascent(),
        qftmetrics.horizontalAdvance(*c),
        (curr_row * texAtlas.colFactor),
        0.0,
      };

      Characters.insert(std::pair<char, Character>(ch, character));
      curr_row++;
    }

  glGenTextures(1, &Texture);
  glBindTexture(GL_TEXTURE_2D, Texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        width * q_str.size(),
        height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        qimg.constBits());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
}


/*!
 * \brief RenderText::renderTextEasy
 * \param number  float input value to render
 * \param xi  x position of text
 * \param yi  y position of text
 * \param ar  arrange of text
 *
 * Proof of concept function, not for use.
 * The simplest way to render text. Load \param number into vector<char>
 * and then find related characters params and textures. And then
 * render charactes one by one using triangles
 */
void RenderText::renderTextEasy(double number, double xi, double yi, Arrange ar)
{
  std::vector<Character> print_characters;
  std::vector<char> num_vec;
  print_characters.reserve(MAX_CH);

  GLfloat tex_width = 0;
  getCharFromFloat(&num_vec, number);
  std::unordered_map<char, Character>::iterator it;
  double offy = 0;

  for( uint c = 0; c < num_vec.size(); c++)
    {
      it = Characters.find(num_vec[c]);
      if(it != Characters.end())
        {
          print_characters.push_back(it->second);
          tex_width += it->second.Advance * pixelWidth;
          offy = (it->second.sizey * pixelHeight)/2;
        }
    }

  if(print_characters.size() == 0) return;

  if(ar == horizontal)
    {
      xi -= tex_width / 2;                                // make text texture to be in the center of the position in x_axis
      yi = proj.bottom + (pixelHeight * 1);
    }
  if(ar == vertical)
    {
      xi = proj.left + (pixelWidth * 4);
      yi -= offy;     // make y_axis text texture to be in the center of position
    }

  // we need to make sure that the edge of a texture
  // will always be in the edge of a pixel
  // so the rendered text won't look fuzzy
  double x = hintToPixel(xi, pixelWidth);
  double y = hintToPixel(yi, pixelHeight);

  glUseProgram(text_prog);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(text_VAO);
  glEnableClientState(GL_VERTEX_ARRAY);
  GLdouble yposH = y + (print_characters[0].sizey * pixelHeight);

  glBindTexture(GL_TEXTURE_2D,Texture);

  for(uint i = 0; i < print_characters.size(); i++)
    {
      GLdouble xpos = x + print_characters[i].BearingX * pixelWidth;    // render glyph considering it's horizontal shift
      GLdouble texposx = print_characters[i].texX;
      GLdouble texposy = print_characters[i].texY;

      GLdouble w = print_characters[i].sizex * pixelWidth;  // width of space for glyph to render

      // render glyph using triangles
      // buffer is packed [Screen X, Screen Y, Tex X, Tex Y] x6
      // Screen coords:
      // 0---*    3---5
      // | \ |    | \ |
      // 1---2    *---4
      // Texture coords:
      // 1---2    *---4
      // | / |    | / |
      // 0---*    3---5
      GLdouble vertices[6][4] = {
        {xpos,     yposH, texposx,                      texposy},
        {xpos,     y,     texposx,                      texposy + texAtlas.rowFactor},
        {xpos + w, y,     texposx + texAtlas.colFactor, texposy + texAtlas.rowFactor},

        {xpos,     yposH, texposx,                      texposy},
        {xpos + w, y,     texposx + texAtlas.colFactor, texposy + texAtlas.rowFactor},
        {xpos + w, yposH, texposx + texAtlas.colFactor, texposy}
      };

      glBindBuffer(GL_ARRAY_BUFFER, text_VBO);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glDrawArrays(GL_TRIANGLES, 0, 6);
      x += (print_characters[i].Advance) * pixelWidth;
    }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisableClientState(GL_VERTEX_ARRAY);
}
