#include "Evolver.h"
#include "Image.h"
#include <iostream>
#include <fstream>
using namespace std;
const int specSize = 1000;
static int spectrum[specSize];
#define SIMPLE_BEND
static int choices[16];
static double minX = 0.0;
static double maxX = 0.5;
static double minY = -0.25;
static double maxY = 0.25;
static const int N = 8;

Evolver::Evolver(int width, int id)
{
  ID = id;
  windowRadius = 1.0;
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
  scale = 2.0f; // 2.5 is also quite good
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
  flips[0].set(1.0,0,0);
  flips[1].set(1.0,0.0,0);
  bends[0] = Vector3(1, 1, 0) / sqrt(2.0);
  bends[1] = Vector3(-1, 1, 0) / sqrt(2.0);
//  bends[0] = Vector3(4.0/5.0, 3.0/5.0, 0);
//  bends[1] = Vector3(-4.0/5.0, 3.0/5.0, 0);

  shifts[0] = Vector3(0, -0.15f, 0); //  // -0.15 or -0.2, maybe -0.2
  shifts[1] = Vector3(0, -0.15f, 0); 
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
void Evolver::recurseDraw(Image* bitmap, const Vector3 &point, const Vector3 &offset, int depth, int size)
{
//  int i = (point - mid).dot(dir) > 0.0 ? 1 : 0;
  int i = choices[16 - depth];

  int X = round(((double)size - 1.0)*(point.x + windowRadius) / (2.0*windowRadius));
  int Y = round(((double)size - 1.0)*(point.y + windowRadius) / (2.0*windowRadius));
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
  int I = (double)M * 0.5*(1.0 + point.x / windowRadius);
  int J = (double)M * 0.5*(1.0 + point.y / windowRadius);
  if (i == 0)
    bluegreen.greens[max(0, min(I, M - 1))][max(0, min(J, M - 1))]++;
  else
    bluegreen.blues[max(0, min(I, M - 1))][max(0, min(J, M - 1))]++;
  if (X >= 0 && X < size && Y >= 0 && Y < size)
  {
    bitmap->incrementPixel(X, Y, 0, i == 0 ? 1 : 0, i == 1 ? 1 : 0);
  }
  Vector3 pos = distort(point, i) + offset;
  if (depth > 0)
    recurseDraw(bitmap, pos, offset, depth - 1, size);
}

void Evolver::recurse(Image* bitmap, int &count, const Vector3 &point, const Vector3 &offset, int depth, const Vector3 &point0, int size, bool draw)
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
        recurseDraw(bitmap, point0, offset, 16, size);
      count++;
    }
    else
      recurse(bitmap, count, pos, offset, depth - 1, point0, size, draw);
  }
}

void Evolver::recurseFast(Image* bitmap, int &count, const Vector3 &point, const Vector3 &offset, int depth)
{
  double a = (double)(M - 1)*(windowRadius + point.y) / (2.0*windowRadius);
  double b = (double)M - 1.0 - 1e-10;
  double y = max(0.0, min(a, b));
  int j = (int)y;
  double blendY = y - (double)j;

  a = (double)(M - 1)*(windowRadius + point.x) / (2.0*windowRadius);
  double x = max(0.0, min(a, b));
  int i = (int)x;
  double blendX = x - (double)i;
  double green = bluegreen.greens[i][j] * (1.0 - blendX)*(1.0 - blendY) + bluegreen.greens[i][j + 1] * (1.0 - blendX)*(blendY)+bluegreen.greens[i + 1][j] * (blendX)*(1.0 - blendY) + bluegreen.greens[i + 1][j + 1] * (blendX)*(blendY);
  double blue = bluegreen.blues[i][j] * (1.0 - blendX)*(1.0 - blendY) + bluegreen.blues[i][j + 1] * (1.0 - blendX)*(blendY)+bluegreen.blues[i + 1][j] * (blendX)*(1.0 - blendY) + bluegreen.blues[i + 1][j + 1] * (blendX)*(blendY);

  i = green > blue ? 0 : 1;

  Vector3 pos = distort(point, i) + offset;
  float dist2 = pos.magnitudeSquared();
  if (dist2 > threshold)
    return;// continue;
  if (depth == 0)
  {
    count++;
  }
  else
    recurseFast(bitmap, count, pos, offset, depth - 1);
}

void Evolver::generateFullMap()
{
  int size = bitmap->width;
  bitmap->clear();
  for (int Y = 0; Y<size; Y++)
  {
    float y = minY + (maxY - minY)*(float)Y / (float)(size - 1);
    for (int X = 0; X<size; X++)
    {
      float x = minX + (maxX-minX)*(float)X / (float)(size - 1);
      int count = 0;
      recurse(bitmap, count, Vector3(x, y, 0), Vector3(x, y, 0), 16, Vector3(x, y, 0), size, false); 
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

  BlueGreen bluegreens[N][N];
  // precalculate separation data
  for (int I = 0; I < N; I++)
  {
    cx = minX + (maxX - minX)*(double)I / (double)(N - 1);
    for (int J = 0; J < N; J++)
    {
      cy = minY + (maxY - minY)*(double)J / (double)(N - 1);
      cout << "generating set matrix " << I << ", " << J << ", cx: " << cx << ", cy: " << cy << endl;
      generateBuddhaSet();
      bluegreens[I][J] = bluegreen;
    }
  }

  bitmap->clear();
  int size = bitmap->width;
  int numSet = 0;
  for (int Y = 0; Y<size; Y++)
  {
    float y = minY + (maxY-minY)*(float)Y / (float)(size - 1);
    double y2 = (double)(N-1)*(double)Y / (double)size;
    int j = y2;
    double blendY = y2 - (double)j;

    for (int X = 0; X<size; X++)
    {
      float x = minX + (maxX-minX)*(float)X / (float)(size - 1);

      // interpolate the bluegreens....
      double x2 = (double)(N-1)*(double)X / (double)size;
      int i = x2;
      double blendX = x2 - (double)i;
      bluegreen = bluegreens[i][j] * (1.0 - blendX)*(1.0 - blendY) + bluegreens[i][j+1] * (1.0 - blendX)*(blendY) + bluegreens[i+1][j] * (blendX)*(1.0 - blendY) + bluegreens[i+1][j+1] * (blendX)*(blendY);

      // so iterate fractal for this position
      int count = 0;
      recurseFast(bitmap, count, Vector3(x, y, 0), Vector3(x, y, 0), 16);
      if (count > 0)
      {
        bitmap->setPixel(X, Y, 255);
        numSet++;
      }
    }
  }
  cout << "number of pixels in final output: " << numSet << endl;
}


void Evolver::setC(double x, double y)
{
  cx = x*(maxX - minX) + minX;
  cy = y*(maxY-minY) + minY;
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
    double y = windowRadius * (-1.0f + 2.0*(float)Y / (float)(size - 1));
 //   double y = minY + (maxY - minY)*(float)Y / (float)(size - 1);
    for (int X = 0; X<size; X++)
    {
      double x = windowRadius * (-1.0f + 2.0f*(float)X / (float)(size - 1));
 //     double x = minX + (maxX - minX)*(float)X / (float)(size - 1);
      int count = 0;
      recurse(bitmap, count, Vector3(x, y, 0), Vector3(cx, cy, 0), 16, Vector3(x, y, 0), size, false);
      if (count == 0)
        bitmap->setPixel(X, Y, 0);
      else
        bitmap->setPixel(X, Y, spectrum[min(count, specSize - 1)]);
    }
  }
  cout << "finished generating Julia set" << endl;
}
// TODO: I need this to be smaller...
void Evolver::generateBuddhaSet()
{
  cout << "generating Buddha set" << endl;
  int size = bitmap->width / 4;
  bitmap->clear();
  avRed = Vector3(0, 0, 0);
  avGreen = Vector3(0, 0, 0);
  numRed = numGreen = 0.0;
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < M; j++)
    {
      bluegreen.blues[i][j] = 0;
      bluegreen.greens[i][j] = 0;
    }
  }
  for (int Y = 0; Y<size; Y++)
  {
    double y = windowRadius * (-1.0f + 2.0*(float)Y / (float)(size - 1));
 //   double y = minY + (maxY-minY)*(float)Y / (float)(size - 1);
    for (int X = 0; X<size; X++)
    {
      double x = windowRadius * (-1.0f + 2.0f*(float)X / (float)(size - 1));
//      double x = minX + (maxX - minX)*(float)X / (float)(size - 1);
      int count = 0;
      recurse(bitmap, count, Vector3(x, y, 0), Vector3(cx, cy, 0), 16, Vector3(x, y, 0), size, true);
    }
  }
  // disperse
  double dispersalRate = 0.1;
  for (int it = 0; it < M; it++)
  {
    BlueGreen temp = bluegreen;
    for (int i = 0; i < M; i++)
    {
      for (int j = 0; j < M; j++)
      {
        double count = 0;
        double averageBlue = 0;
        double averageGreen = 0;
        for (int x = -1; x < 2; x++)
        {
          for (int y = -1; y < 2; y++)
          {
            int X = i + x;
            int Y = j + y;
            if (X >= 0 && X < M && Y>=0 && Y<M)
            {
              averageBlue += bluegreen.blues[X][Y];
              averageGreen += bluegreen.greens[X][Y];
              count++;
            }
          }
        }
        temp.blues[i][j] += ((averageBlue / count) - temp.blues[i][j])*dispersalRate;
        temp.greens[i][j] += ((averageGreen / count) - temp.greens[i][j])*dispersalRate;
      }
    }
    bluegreen = temp;
  }
  for (int Y = 0; Y < size; Y++)
  {
    double y = max(0.0, min(-0.5 + ((double)Y * (double)M / (double)size), (double)M-1.0 -1e-10));
    int j = (int)y;
    double blendY = y - (double)j;
    for (int X = 0; X < size; X++)
    {
      double x = max(0.0, min(-0.5 + ((double)X * (double)M / (double)size), (double)M-1.0 -1e-10));
      int i = (int)x;
      double blendX = x - (double)i;
      double green = bluegreen.greens[i][j] * (1.0 - blendX)*(1.0 - blendY) + bluegreen.greens[i][j + 1] * (1.0 - blendX)*(blendY)+bluegreen.greens[i + 1][j] * (blendX)*(1.0 - blendY) + bluegreen.greens[i + 1][j + 1] * (blendX)*(blendY);
      double blue = bluegreen.blues[i][j] * (1.0 - blendX)*(1.0 - blendY) + bluegreen.blues[i][j + 1] * (1.0 - blendX)*(blendY)+bluegreen.blues[i + 1][j] * (blendX)*(1.0 - blendY) + bluegreen.blues[i + 1][j + 1] * (blendX)*(blendY);
      
//      bitmap->incrementPixel(X, Y, 0, green / 180.0, blue / 180.0);
      int inc = 40;
      bitmap->incrementPixel(X, Y, 0, green > blue ? inc : 0, blue > green ? inc : 0);
    }
  }

  cout << "finished generating buddha set" << endl;
}