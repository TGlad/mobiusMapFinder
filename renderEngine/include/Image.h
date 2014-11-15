#pragma once
#include "basics.h"
#include "ScreenColour.h"

class Image
{
public:
  int width;
  int height;
  int widthShift;
  int *data;
  GLuint textureID;

  inline void setPixel(int x, int y, const int& colour)
  {
    int *ptr = data + x + y*width;
    *ptr = colour;
  }
  inline int& pixel(int x, int y)
  {
    int *ptr = data + x + y*width;
    return *ptr;
  }
  inline bool isSet(int x, int y)
  {
    int *ptr = data + x + y*width;
    return (*ptr & 128) != 0;
  }
  void draw();
  void clear(char shade = 0);
  void generateTexture();

  Image(int width, int height);
  ~Image(void);
};
