#include "Evolver.h"
#include "Image.h"
#include <iostream>
#include <fstream>
using namespace std;
const int specSize = 1000;
static int spectrum[specSize];
#define SIMPLE_BEND
static int choices[16];

Evolver::Evolver(int width, int id)
{
  ID = id;
  windowRadius = 0.2;
  for (int i = 0; i<specSize; i++)
  {
    float f = (logf((float)(i+8)) - logf(8.0f)) / logf((float)(specSize+7));
    float f2 = 1.0f - (1.0f-f)*(1.0f-f);

    float red = 0.5f*(cosf(f * pi) + 1.0f);
    float green = 0.375f*(1.0f - cosf(f * 2.0f * pi));
    float blue = 0.5f*(cosf((1.0f - f) * pi) + 1.0f);

    float scale = 255.0f * f2;
    spectrum[i] = (int)(red*scale);
    spectrum[i] += (int)(green*scale) << 8;
    spectrum[i] += (int)(blue*scale) << 16;
  }
  bitmap = new Image(width, width);
  bitmap->clear(0);
  scale = 2.0f;
  reset();
}

void Evolver::reset()
{
  frame = 0;
  randomiseMasks(*this, 50.0f);
  if (ID == 0)
  {
    generateFullMap();
  }
  else if (ID == 1)
  {

  }
}

void Evolver::load(char* fileName, int type)
{
  FILE* fp;
  if (fopen_s(&fp, fileName, "rb"))
  {
    printf("Cannot find file: %s\n", fileName);
    return;
  }
 // read(fp);
  fclose(fp);
}

void Evolver::randomiseMasks(const Evolver& master, float percentVariation)
{
  flips[0].set(-sqrt(0.5f),sqrt(0.5f),0);
  flips[1].set(flips[0].y, flips[0].x, 0);
  bends[0] = Vector3(1,0,0);
  bends[1] = Vector3(0,1,0);

  shifts[0].setToZero();
  shifts[1].setToZero();
  frame = 0;
}

void Evolver::draw()
{
  bitmap->generateTexture();
  bitmap->draw();
}

static float threshold = 4.0f;
static Vector3 smid;
static Vector3 sdir;
static Vector3 avRed;
static double numRed;
static Vector3 avGreen;
static double numGreen;

Vector3 Evolver::distort(const Vector3 &point, int i)
{
  Vector3 pos = point - bends[i];
  pos *= bends[i].magnitudeSquared() / pos.magnitudeSquared();
  pos += bends[i];
  pos = (shifts[i] + pos - (2.0 * flips[i] * pos.dot(flips[i]))) * scale;
  return pos;
}
void Evolver::recurseDraw(Image* bitmap, const Vector3 &point, const Vector3 &offset, int depth, int X2, int Y2)
{
//  int i = (point - mid).dot(dir) > 0.0 ? 1 : 0;
  int i = choices[16 - depth];
  int size = bitmap->width;
  if (i == 0)
  {
    avRed += point;
    numRed++;
  }
  else
  {
    avGreen += point;
    numGreen++;
  }

  int X = round(((double)size - 1.0)*(point.x + 1.0) / 1.6);
  int Y = round(((double)size - 1.0)*(point.y + 1.0) / 1.6);
  if (X >= 0 && X < size && Y >= 0 && Y < size)
    bitmap->incrementPixel(X, Y, 0, i == 0 ? 1 : 0, i == 1 ? 1 : 0);
  Vector3 pos = distort(point, i) + offset;
  if (depth > 0)
    recurseDraw(bitmap, pos, offset, depth - 1, X, Y);
}

void Evolver::recurse(Image* bitmap, int &count, const Vector3 &point, const Vector3 &offset, int depth, const Vector3 &point0, int X, int Y, bool draw)
{
  for (int i = 0; i<2; i++)
  {
    Vector3 pos = distort(point, i) + offset;
    float dist2 = pos.magnitudeSquared();
    if (dist2 > threshold)
      continue;
    choices[16 - depth] = i;
    if (depth == 0)
    {
      if (draw)
        recurseDraw(bitmap, point0, offset, 16, X, Y);
      count++;
    }
    else
      recurse(bitmap, count, pos, offset, depth - 1, point0, X, Y, draw);
  }
}

void Evolver::recurseFast(Image* bitmap, int &count, const Vector3 &point, const Vector3 &offset, int depth, const Vector3 &point0)
{
  double size = bitmap->width - 1.0;
  Vector3 p(size * (point.x + 1.0) / 1.6, size * (point.y + 1.0) / 1.6, 0);
  int i = (p - smid).dot(sdir) > 0.0 ? 1 : 0;

  Vector3 pos = distort(point, i) + offset;
  float dist2 = pos.magnitudeSquared();
  if (dist2 > threshold)
    return;// continue;
  if (depth == 0)
  {
    count++;
  }
  else
    recurseFast(bitmap, count, pos, offset, depth - 1, point0);
}

struct Pair
{
  Vector3 pos;
  Vector3 dir;
};
static bool done = false;
static const int N = 4;
static Pair pairs[N][N];


void Evolver::generateFullMap()
{
  int size = bitmap->width;
  bitmap->clear();
  for (int Y = 0; Y<size; Y++)
  {
    float y = -1.0f + 1.6f*(float)Y / (float)(size - 1);
    for (int X = 0; X<size; X++)
    {
      float x = -1.0f + 1.6f*(float)X / (float)(size - 1);
      int count = 0;
      recurse(bitmap, count, Vector3(x, y, 0), Vector3(x, y, 0), 16, Vector3(x, y, 0), X, Y, false); 
      if (count == 0)
        bitmap->setPixel(X, Y, 0);
      else
        bitmap->setPixel(X, Y, spectrum[min(count, specSize - 1)]);
    }
  }
}

void Evolver::setMidC(double midCx, double midCy)
{
  if (ID != 1)
    return;

  double window = 0.4;
  int size = bitmap->width;
  for (int I = 0; I < N; I++)
  {
    for (int J = 0; J < N; J++)
    {
      avRed = Vector3(0, 0, 0);
      numRed = 0.0;
      avGreen = Vector3(0, 0, 0);
      numGreen = 0.0;

      for (int Y = 0; Y<size; Y++)
      {
        float y = -1.0f + 1.6f*(float)Y / (float)(size - 1);
        for (int X = 0; X<size; X++)
        {
          float x = -1.0f + 1.6f*(float)X / (float)(size - 1);
          // so iterate fractal for this position
          int count = 0;
          double xx = midCx + window * (-0.5 + (double)I / (double)(N - 1));
          double yy = midCy + window * (-0.5 + (double)J / (double)(N - 1));
          recurse(bitmap, count, Vector3(x, y, 0), Vector3(xx, yy, 0), 16, Vector3(x, y, 0), X, Y, true);
        }
      }
      Vector3 mid = ((avRed / numRed) + (avGreen / numGreen)) / 2.0;
      Vector3 dir = avGreen / numGreen - avRed / numRed;
      dir.normalise();
      pairs[I][J].pos = mid;
      pairs[I][J].dir = dir;
    }
  }

  for (int Y = 0; Y<size; Y++)
  {
    float y = -1.0f + 1.6f*(float)Y / (float)(size - 1);
    for (int X = 0; X<size; X++)
    {
      float x = -1.0f + 1.6f*(float)X / (float)(size - 1);
      int count = 0;
      double x2 = (double)N*((double)X / (double)size);
      double y2 = (double)N*((double)Y / (double)size);

      int I = (int)x2;
      int J = (int)y2;
      double blendI = x2 - I;
      double blendJ = x2 - J;
      smid = pairs[I][J].pos * (1.0 - blendI)*(1.0 - blendJ) + pairs[I][J + 1].pos * (1.0 - blendI)*blendJ + pairs[I + 1][J].pos * blendI*(1.0 - blendJ) + pairs[I + 1][J + 1].pos * blendI*blendJ;
      sdir = pairs[I][J].dir * (1.0 - blendI)*(1.0 - blendJ) + pairs[I][J + 1].dir * (1.0 - blendI)*blendJ + pairs[I + 1][J].dir * blendI*(1.0 - blendJ) + pairs[I + 1][J + 1].dir * blendI*blendJ;

      double xx = midCx - 0.5*window + window * ((double)X / (double)size);
      double yy = midCy - 0.5*window + window * ((double)Y / (double)size);
      recurseFast(bitmap, count, Vector3(x, y, 0), Vector3(xx, yy, 0), 16, Vector3(x, y, 0)); // also -0.4, -0.3

      // now we know if is inside or not, so colour the pixel
      if (count == 0)
        bitmap->setPixel(X, Y, 0);
      else
        bitmap->setPixel(X, Y, 255);

      Vector3 pos(X, Y, 0);
      if ((pos - smid).dot(sdir) > 0.0 && !bitmap->pixel(X, Y))
        bitmap->incrementPixel(X, Y, 0, 0, 60);
    }
  }
}


void Evolver::setC(double x, double y)
{
  cx = x*1.6 - 1.0;
  cy = y*1.6 - 1.0;
  if (ID == 2)
    generateJuliaSet();
  else if (ID == 3)
    generateBuddhaSet();
}

void Evolver::generateJuliaSet()
{
  int size = bitmap->width;
  cout << "generating Julia set. cx: " << cx << ", cy: " << cy << endl;
  bitmap->clear();
  for (int Y = 0; Y<size; Y++)
  {
//    double y = cy + windowRadius * (-1.0f + 2.0*(float)Y / (float)(size - 1));
    double y = -1.0f + 1.6*(float)Y / (float)(size - 1);
    for (int X = 0; X<size; X++)
    {
//      double x = cx + windowRadius * (-1.0f + 2.0f*(float)X / (float)(size - 1));
      double x = -1.0f + 1.6*(float)X / (float)(size - 1);
      int count = 0;
      recurse(bitmap, count, Vector3(x, y, 0), Vector3(cx, cy, 0), 16, Vector3(x, y, 0), X, Y, false);
      if (count == 0)
        bitmap->setPixel(X, Y, 0);
      else
        bitmap->setPixel(X, Y, spectrum[min(count, specSize - 1)]);
    }
  }
  cout << "finished generating Julia set" << endl;
}
void Evolver::generateBuddhaSet()
{
  cout << "generating Buddha set" << endl;
  int size = bitmap->width;
  bitmap->clear();
  for (int Y = 0; Y<size; Y++)
  {
//    double y = cy + windowRadius * (-1.0f + 2.0*(float)Y / (float)(size - 1));
    double y = -1.0f + 1.6*(float)Y / (float)(size - 1);
    for (int X = 0; X<size; X++)
    {
//      double x = cx + windowRadius * (-1.0f + 2.0f*(float)X / (float)(size - 1));
      double x = -1.0f + 1.6*(float)X / (float)(size - 1);
      int count = 0;
      recurse(bitmap, count, Vector3(x, y, 0), Vector3(cx, cy, 0), 16, Vector3(x, y, 0), X, Y, true);
    }
  }
  cout << "finished generating buddha set" << endl;
}