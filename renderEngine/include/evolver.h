#pragma once
#include "basics.h"
#include "vector4.h"
#include "ScreenColour.h"
extern bool g_timeSymmetric;

class Evolver
{
public:
  Evolver(int depth);
  void load(char* fileName, int type);
  void draw();
  void update();
  void reset();
  void set(const Evolver& evolver);
  void write(FILE* fp);
  class Image* bitmap;
  void randomiseMasks(const Evolver& master, float percentVariation);
public: // private:
  void read(FILE* fp);
  void recurse(int &count, const Vector3 &point, const Vector3 &offset, int depth, float size);
  int frame;
  int type;
  int depth;
  float mags[2]; // precalculated

  // genome here
  Vector3 bends[2];
  Vector3 flips[2];
  Vector3 shifts[2];
  float scale;
};

