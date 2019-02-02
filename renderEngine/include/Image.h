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

/*  inline void drawLine(int x0, int y0, int x1, int y1)
  {

  }
  inline void drawLine(Vector3 &mid, Vector3 &dir)
  {
    Vector3 along(dir.y, -dir.x, 0);
    Vector3 start = mid - along*(double)width / 2.0;
    Vector3 end = mid + along*(double)width / 2.0;
    drawLine(start.x, start.y, end.x, end.y);
  }*/
  inline void incrementPixel(int x, int y, int r, int g, int b)
  {
    int *ptr = data + x + y*width;
    int val = *ptr;
    int red = val % 256;
    int green = (val / 256) % 256;
    int blue = val / 65536;
    red = min(255, red + r);
    green = min(255, green + g);
    blue = min(255, blue + b);
    *ptr = red + green * 256 + blue * 65536;
  }
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
