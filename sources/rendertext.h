#ifndef RENDERTEXT_H
#define RENDERTEXT_H

#include <QOpenGLFunctions_3_3_Core>

#include <vector>
#include <unordered_map>



struct Proj
{
  float left;
  float right;
  float bottom;
  float top;
};


class RenderText : protected QOpenGLFunctions_3_3_Core
{

  enum Arrange
  {
    horizontal,
    vertical
  };


  struct CHPrintInfo
  {
    uint Bearing;
    uint Advance;
    GLdouble texX;
    GLdouble texY;
  };

  struct Text
  {
    double num;
    std::vector<char> print;
    std::vector<CHPrintInfo>printInfo;
    std::vector<GLdouble> pos;
    Arrange ar;
    double width;
    double height;
  };

  struct TexAtlas   // in work
  {
    GLint rows;
    GLint cols;
    GLdouble rowFactor;
    GLdouble colFactor;
  };


  struct Character
  {
    GLuint sizex;
    GLuint sizey;
    GLint BearingX;
    GLint BearingY;
    GLint Advance;
    GLdouble texX;
    GLdouble texY;
  };

public:
  explicit RenderText();
  ~RenderText();

  void initTextRender();
  void reserveSpace(int width, int height);

  int createShader(const char* vertex, const char* fragment);
  void genTextures();

  void createCharacter(GLbyte ch);
  void createQCharacters(QString &q_str);


  void updateShaderMatrix();
  void updateTextPositions();

  void getCharFromFloat(std::vector<char> *number, double input);
  void renderTextEasy(double number, double xi, double yi, Arrange ar);
  void renderText();

  double hintToPixel(double num, double pixelsize);

  inline void setPixelHeight(double height);
  inline void setPixelWidth(double width);
  void setProjMatrix(Proj &pr);

  inline double getPixelHeight();
  inline double getPixelWidth();
  Proj getProjMatrix();

  void setText(std::vector<double> &y, std::vector<double> &x);


private:
  Proj proj;               ///< Holds projection matrix values


  std::unordered_map<char, Character> Characters;  ///<
  std::vector<Text> textBoxes;

  TexAtlas texAtlas;      ///< Holds information about texture atlas
  GLuint Texture;         ///< Texture atlas where all characters glyps is painted

  GLuint text_prog;       ///< Shader program that used to render characters glyphs
  GLuint text_VBO;        ///< Vertex buffer object that holds screen and texture coordinates on where to render glyphs
  GLuint text_VAO;        ///< Vertex array object used to load values to compiled shader program

  double pixelWidth;
  double pixelHeight;
  int textMaxWidth;
  int textHeight;
  int characterWidth;
  int characterHeight;

};


inline void RenderText::setPixelHeight(double height)
{
  pixelHeight = height;
}


inline void RenderText::setPixelWidth(double width)
{
  pixelWidth = width;
}


inline double RenderText::getPixelHeight()
{
  return pixelHeight;
}


inline double RenderText::getPixelWidth()
{
  return pixelWidth;
}


#endif // RENDERTEXT_H
