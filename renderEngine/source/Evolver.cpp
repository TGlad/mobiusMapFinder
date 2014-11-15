#include "Evolver.h"
#include "Image.h"
// #define EXTREME_VARIATIONS
const int specSize = 1000;
static int spectrum[specSize];
#define SIMPLE_BEND

Evolver::Evolver(int depth)
{
  for (int i = 0; i<specSize; i++)
  {
#if 1
    float f = (logf((float)(i+8)) - logf(8.0f)) / logf((float)(specSize+7));
    float f2 = 1.0f - (1.0f-f)*(1.0f-f);

    float red = 0.5f*(cosf(f * pi) + 1.0f);
    float green = 0.375f*(1.0f - cosf(f * 2.0f * pi));
    float blue = 0.5f*(cosf((1.0f - f) * pi) + 1.0f);
#else
    int repeat = 100;
    float f = (float)(i%repeat) / (float)repeat;
    float f2 = min(1.0f, (float)i / (float)repeat);

    float red = 0.5f*(cosf(f * 2.0*pi) + 1.0f);
    float green = 0.25f*(3.0f - cosf(f * 4.0f * pi));
    float blue = 0.5f*(cosf((1.0f - f) * 2.0f*pi) + 1.0f);

#endif
    float scale = 255.0f * f2;
    spectrum[i] = (int)(red*scale);
    spectrum[i] += (int)(green*scale) << 8;
    spectrum[i] += (int)(blue*scale) << 16;
  }
  int size = 1<<depth;
  bitmap = new Image(size, size);
  type = 1;
  this->depth = depth;
  scale = 2.0f;
  reset();
}

void Evolver::reset()
{
  frame = 0;
  randomiseMasks(*this, 50.0f);

  // type 3
//   for (int i = 0; i<1<<9; i++)
//     siblingMasks[i] = random()>0;
}

void Evolver::load(char* fileName, int type)
{
  this->type = type;
  FILE* fp;
  if (fopen_s(&fp, fileName, "rb"))
  {
    printf("Cannot find file: %s\n", fileName);
    return;
  }
  read(fp);
  fclose(fp);
}

void Evolver::randomiseMasks(const Evolver& master, float percentVariation)
{
  type = master.type;
  if (percentVariation == 50.0f)
  {
    for (int i = 0; i<2; i++)
    {
      bends[i] = Vector3(random(-5.0f, 5.0f), random(-5.0f, 5.0f), 0);
#if !defined(SIMPLE_BEND)
      bends[i].normalise();
      bends[i] *= random(2.0f, 3.0f);
#endif
      shifts[i] = Vector3(random(-0.5f, 0.5f), random(-0.5f, 0.5f), 0);
      flips[i] = Vector3(random(-1.0f, 1.0f), random(-1.0f, 1.0f), 0);
      flips[i].normalise();
    }
  }
  else
  {
    for (int i = 0; i<2; i++)
    {
      bends[i] += Vector3(random(-0.1f, 0.1f), random(-0.1f, 0.1f), 0);
#if !defined(SIMPLE_BEND)
      bends[i].normalise();
      bends[i] *= random(2.0f, 3.0f);
#endif
      shifts[i] += Vector3(random(-0.1f, 0.1f), random(-0.1f, 0.1f), 0);
      flips[i] += Vector3(random(-0.1f, 0.1f), random(-0.1f, 0.1f), 0);
      flips[i].normalise();
    }
  }
//   flips[0].set(-sqrt(0.5f),sqrt(0.5f),0);
//   flips[1].set(flips[0].y, flips[0].x, 0);
//   bends[0] = Vector3(1,0,0);
//   bends[1] = Vector3(0,1,0);
   shifts[0].setToZero();
   shifts[1].setToZero();
  frame = 0;
}

void Evolver::read(FILE* fp)
{
  switch(type)
  {
  case(1):
    for (int i = 0; i<2; i++)
    {
      fread(&bends[i], sizeof(Vector3), 1, fp);
      fread(&flips[i], sizeof(Vector3), 1, fp);
      fread(&shifts[i], sizeof(Vector3), 1, fp);
    }
    break;
  default:
    return;
  }
  frame = 0;
}

void Evolver::write(FILE* fp)
{
  switch(type)
  {
  case(1):
    for (int i = 0; i<2; i++)
    {
      fwrite(&bends[i], sizeof(Vector3), 1, fp);
      fwrite(&flips[i], sizeof(Vector3), 1, fp);
      fwrite(&shifts[i], sizeof(Vector3), 1, fp);
    }
    break;
  default:
    break;
  }
}


void Evolver::set(const Evolver& evolver)
{
  type = evolver.type;
//   for (int i = 0; i<1<<9; i++)
//     siblingMasks[i] = evolver.siblingMasks[i];
}

void Evolver::draw()
{
  bitmap->generateTexture();
  bitmap->draw();
}
static int start = 0;
static int end = 2;
static Vector3 finalPoint;
const int n = 2;
static Vector3 escape1[n][n];
static Vector3 escape2[n][n];
static float threshold = 4.0f;
static Vector3 thresh1;
static Vector3 thresh2;
void Evolver::recurse(int &count, const Vector3 &point, const Vector3 &offset, int depth, float size)
{
  for (int i = start; i<end; i++)
  {
    Vector3 pos;
#if defined(SIMPLE_BEND)
    pos = point - bends[i];
    float times = bends[i].magnitudeSquared() / pos.magnitudeSquared();
    pos *= times;
    size *= times;
    pos += bends[i];
#else
    // keep the radius 2 circle where it is
    pos = point - bends[i];
    pos *= 1.0f / pos.magnitudeSquared();
    float near = 1.0f/(mags[i] + 2.0f);
    float far = 1.0f/(mags[i] - 2.0f);
    float avg = (near+far)*0.5f;
    float scale = 4.0 / (far - near);
    pos += bends[i] * avg / mags[i];
    pos *= scale;
#endif
    pos = (shifts[i] + pos - (2.0 * flips[i] * pos.dot(flips[i]))) * scale + offset;
    size *= scale;
    float dist2 = (pos - thresh1).magnitudeSquared() + (pos - thresh2).magnitudeSquared();
    if (dist2 < threshold)
      continue;
    if (depth==0) //size > 15000.0f || depth == -20)
    {
      count++;
      finalPoint = pos;
    }
    else
      recurse(count, pos, offset, depth - 1, size);
  }
}
void Evolver::update()
{
  // This performs the covolution
  if (type == 1)
  {
    mags[0] = bends[0].magnitude();
    mags[1] = bends[1].magnitude();
    int size = 1<<depth; 
    threshold = 0.0f;
    thresh1.setToZero();
    thresh2.setToZero();
    for (int Y = 0; Y<n; Y++)
    {
      float y = -1.0f + 2.0f*((float)Y/(float)(n-1));
      for (int X = 0; X<n; X++)
      {
        float x = -1.0f + 2.0f*((float)X/(float)(n-1));
        int count = 0;
        start = 0; end = 1;
        recurse(count, Vector3(0, 0, 0), Vector3(x, y, 0), depth+8, 1.0f);
        escape1[Y][X] = finalPoint;
        start = 1; end = 2;
        recurse(count, Vector3(0, 0, 0), Vector3(x, y, 0), depth+8, 1.0f);
        escape2[Y][X] = finalPoint;
      }
    }

    start = 0; end = 2;
    threshold = 4.0f;
    for (int Y = frame % size; Y<(frame % size)+4; Y++)
    {
      float y = 2.0f*((float)Y - ((float)(size-1) / 2.0)) / (float)(size-1);
      float yy = (float)Y / (float)(size-1);
      for (int X = 0; X<size; X++)
      {
        float x = 2.0f*((float)X - ((float)(size-1) / 2.0)) / (float)(size-1);
        float xx = (float)X / (float)(size-1);
        Vector3 mid1 = escape1[0][0]*(1.0f-xx) + escape1[0][1]*xx;
        Vector3 mid2 = escape1[1][0]*(1.0f-xx) + escape1[1][1]*xx;
        thresh1 = mid1*(1.0f-yy) + mid2*yy;
        Vector3 mid21 = escape2[0][0]*(1.0f-xx) + escape2[0][1]*xx;
        Vector3 mid22 = escape2[1][0]*(1.0f-xx) + escape2[1][1]*xx;
        thresh2 = mid21*(1.0f-yy) + mid22*yy;
        threshold = (thresh1 - thresh2).magnitudeSquared() * 1.0f;

        // so iterate fractal for this position
        const int maxIterations = depth;
        int count = 0;
        recurse(count, Vector3(0, 0, 0), Vector3(x, y, 0), depth+3, 1.0f);

        // now we know if is inside or not, so colour the pixel
        if (count == 0)
          bitmap->setPixel(X, Y, 0);
        else
          bitmap->setPixel(X, Y, spectrum[min(count, specSize-1)]);
      }
    }
  }
  frame+=4;
}