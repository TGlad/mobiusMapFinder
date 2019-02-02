#pragma once
#include "basics.h"
#include "vector4.h"
#include "ScreenColour.h"
extern bool g_timeSymmetric;

class Evolver
{
public:
  Evolver(int width, int id);
  void load(char* fileName, int type);
  void draw();
  void reset();
  class Image* bitmap;
  void randomiseMasks(const Evolver& master, float percentVariation);
public: // private:
  void recurseDraw(Image* bitmap, const Vector3 &point, const Vector3 &offset, int depth, int X, int Y);
  void recurse(Image* bitmap, int &count, const Vector3 &point, const Vector3 &offset, int depth, const Vector3 &point0, int X, int Y, bool draw);
  void recurseFast(Image* bitmap, int &count, const Vector3 &point, const Vector3 &offset, int depth, const Vector3 &point0);
  int frame;
  void setMidC(double x, double y);
  void setC(double x, double y);

  // genome here
  Vector3 bends[2];
  Vector3 flips[2];
  Vector3 shifts[2];
  float scale;

  int ID;
  Vector3 distort(const Vector3 &point, int i);
  void generateFullMap();
  void generateJuliaSet();
  void generateBuddhaSet();
  double windowRadius;
  double cx, cy;
};

