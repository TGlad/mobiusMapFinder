#include "Evolver.h"
#include "Image.h"
// #define EXTREME_VARIATIONS
const int specSize = 2000;
static int spectrum[specSize];
#define SIMPLE_BEND

Evolver::Evolver(int depth)
{
  for (int i = 0; i<specSize; i++)
  {
#if 1
    float f = (logf((float)(i+8)) - logf(8.0f)) / logf((float)(specSize+7));
    float f2 = f;

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
    float scale = 255.0f * (1.0f - (1.0f-f2)*(1.0f-f2));
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
const float threshold = 2.0f;
void Evolver::recurse(int &count, const Vector3 &point, const Vector3 &offset, int depth, float size)
{
  for (int i = 0; i<2; i++)
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
    pos = (0.0f*shifts[i] + pos - (2.0 * flips[i] * pos.dot(flips[i]))) * scale + offset;
    size *= scale;
    if (pos.magnitudeSquared() > 9.0)
      continue;
    if (depth==0) //size > 15000.0f || depth == -20)
      count++;
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
    for (int Y = frame % size; Y<(frame % size)+4; Y++)
    {
      float y = 4.0f*((float)Y - ((float)(size-1) / 2.0)) / (float)(size-1);
      for (int X = 0; X<size; X++)
      {
        float x = 4.0f*((float)X - ((float)(size-1) / 2.0)) / (float)(size-1);

        // so iterate fractal for this position
        const int maxIterations = depth;
        int count = 0;
        recurse(count, Vector3(0, 0, 0), Vector3(x, y, 0), depth+8, 1.0f);

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