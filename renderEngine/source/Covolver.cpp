#include "Covolver.h"
#include "Image.h"

Covolver::Covolver(int depth)
{
  this->depth = depth;

  bitmaps = new Image*[depth+1];
  bitmapDuals = new Image*[depth+1];
  for (int size = 2, i=1; i<=depth; size *= 2, i++)
  {
    bitmaps[i] = new Image(size+2, size+2); // store each pixel
    bitmaps[i]->clear();
    bitmapDuals[i] = new Image(size+2, size+2); // store each pixel
    bitmapDuals[i]->clear();
  }
  type = 1;
  reset();
}

void Covolver::reset()
{
  frame = 0;

  // type 3
  for (int i = 0; i<1<<9; i++)
    siblingMasks[i] = random()>0;
  for (int i = 0; i<1<<4; i++)
    parentMasks[i] = random()>0;
  for (int i = 0; i<1<<4; i++)
    childMasks[i] = random()>0;

  for (int i = 0; i<32; i++)
    totalMasks[i] = random() > 0 ? 1 : 0;

  // type 6
  for (int i = 0; i<6; i++)
    for (int j = 0; j<3; j++)
      for (int k = 0; k<3; k++)
      {
        parentsAdd[i][j][k] = random() > 0 ? 1 : 0;
        parentsRemove[i][j][k] = random() > 0 ? 1 : 0;
      }

  // type 7
  for (int i = 0; i<1<<7; i++)
    octagonalMasks[i] = random()>0;

  randomise();
}

void Covolver::load(char* fileName, int type)
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


void Covolver::randomiseMasks(const Covolver& master, float percentVariation)
{
  type = master.type;
  float threshold = 1.0f - 2.0f*0.01f*percentVariation;
  for (int i = 0; i<1<<9; i++)
  {
    siblingMasks[i] = master.siblingMasks[i];
    if (random() > threshold)
      siblingMasks[i] = !siblingMasks[i];
  }
  for (int i = 0; i<1<<4; i++)
  {
    parentMasks[i] = master.parentMasks[i];
    if (random() > threshold)
      parentMasks[i] = !parentMasks[i];
  }
  for (int i = 0; i<1<<4; i++)
  {
    childMasks[i] = master.childMasks[i];
    if (random() > threshold)
      childMasks[i] = !childMasks[i];
  }
  frame = 0;

  for (int i = 0; i<32; i++)
  {
    totalMasks[i] = master.totalMasks[i];
    if (random() > threshold)
      totalMasks[i] = 1-totalMasks[i];
  }
  for (int i = 0; i<6; i++)
    for (int j = 0; j<3; j++)
      for (int k = 0; k<3; k++)
      {
        parentsAdd[i][j][k] = master.parentsAdd[i][j][k];
        parentsRemove[i][j][k] = master.parentsRemove[i][j][k];
        if (random() > threshold)
          parentsAdd[i][j][k] = !parentsAdd[i][j][k];
        if (random() > threshold)
          parentsRemove[i][j][k] = !parentsRemove[i][j][k];
      }

  for (int i = 0; i<1<<7; i++)
  {
    octagonalMasks[i] = master.octagonalMasks[i];
    if (random() > threshold)
      octagonalMasks[i] = !octagonalMasks[i];
  }
}

void Covolver::randomise()
{
  // initialise the bitmaps to some random image:
  // This is a recursive process, generating the data procedurally as we go deeper in detail level.
  for (int level = 2; level<=depth; level++)
  {
    int size = 1<<level;
    for (int i = 0; i<size; i++)
      for (int j = 0; j<size; j++)
        bitmaps[level]->setPixel(i, j, random() > (type == 7 ? 0.0f : 0.5f) ? 128 : 0);
//        bitmapDuals[level]->setPixel(i, j, random() > 0.5f ? 128 : 0); // only need to do this is we have a dual system not just dependent on parents
  }
}

void Covolver::read(FILE* fp)
{
  switch(type)
  {
  case(1):
    for (int i = 0; i<16; i++)
      fread(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(2):
    for (int i = 0; i<17; i++)
      fread(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(3):
    for (int i = 0; i<1<<9; i++)
      fread(&siblingMasks[i], sizeof(bool), 1, fp);
    for (int i = 0; i<1<<4; i++)
      fread(&parentMasks[i], sizeof(bool), 1, fp);
    for (int i = 0; i<1<<4; i++)
      fread(&childMasks[i], sizeof(bool), 1, fp);
    break;
  case(4):
    for (int i = 0; i<33; i++)
      fread(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(5):
    for (int i = 0; i<164; i++)
      fread(&siblingMasks[i], sizeof(int), 1, fp);
    break;
  case(6):
    for (int i = 0; i<6; i++)
      for (int j = 0; j<3; j++)
        for (int k = 0; k<3; k++)
        {
          fread(&parentsAdd[i][j][k], sizeof(bool), 1, fp);
          fread(&parentsRemove[i][j][k], sizeof(bool), 1, fp);
        }
    break;
  case(7):
    for (int i = 0; i<1<<6; i++)
      fread(&octagonalMasks[i], sizeof(bool), 1, fp);
    break;
  case(8):
    for (int i = 0; i<1<<7; i++)
      fread(&octagonalMasks[i], sizeof(bool), 1, fp);
    break;
  default:
    return;
  }
  frame = 0;
}

void Covolver::write(FILE* fp)
{
  switch(type)
  {
  case(1):
    for (int i = 0; i<16; i++)
      fwrite(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(2):
    for (int i = 0; i<17; i++)
      fwrite(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(3):
    for (int i = 0; i<1<<9; i++)
      fwrite(&siblingMasks[i], sizeof(bool), 1, fp);
    for (int i = 0; i<1<<4; i++)
      fwrite(&parentMasks[i], sizeof(bool), 1, fp);
    for (int i = 0; i<1<<4; i++)
      fwrite(&childMasks[i], sizeof(bool), 1, fp);
    break;
  case(4):
    for (int i = 0; i<33; i++)
      fwrite(&totalMasks[i], sizeof(int), 1, fp);
    break;
  case(5):
    for (int i = 0; i<164; i++)
      fwrite(&siblingMasks[i], sizeof(int), 1, fp);
    break;
  case(6):
    for (int i = 0; i<6; i++)
      for (int j = 0; j<3; j++)
        for (int k = 0; k<3; k++)
        {
          fwrite(&parentsAdd[i][j][k], sizeof(bool), 1, fp);
          fwrite(&parentsRemove[i][j][k], sizeof(bool), 1, fp);
        }
    break;
  case(7):
    for (int i = 0; i<1<<6; i++)
      fwrite(&octagonalMasks[i], sizeof(bool), 1, fp);
    break;
  case(8):
    for (int i = 0; i<1<<7; i++)
      fwrite(&octagonalMasks[i], sizeof(bool), 1, fp);
    break;
  default:
    break;
  }
}


void Covolver::set(const Covolver& Covolver)
{
  type = Covolver.type;
  for (int i = 0; i<1<<9; i++)
    siblingMasks[i] = Covolver.siblingMasks[i];
  for (int i = 0; i<1<<4; i++)
    parentMasks[i] = Covolver.parentMasks[i];
  for (int i = 0; i<1<<4; i++)
    childMasks[i] = Covolver.childMasks[i];

  frame = 0;
  for (int i = 0; i<32; i++)
    totalMasks[i] = Covolver.totalMasks[i];

  for (int i = 0; i<6; i++)
    for (int j = 0; j<3; j++)
      for (int k = 0; k<3; k++)
      {
        parentsAdd[i][j][k] = Covolver.parentsAdd[i][j][k];
        parentsRemove[i][j][k] = Covolver.parentsRemove[i][j][k];
      }

  for (int i = 0; i<1<<7; i++)
    octagonalMasks[i] = Covolver.octagonalMasks[i];
}

void Covolver::draw()
{
  bitmaps[depth]->generateTexture();
  bitmaps[depth]->draw();
}

bool Covolver::getNewValue(int level, int X, int Y)
{
  int dirX = X%2 ? 1 : -1;
  int dirY = Y%2 ? 1 : -1;

  int xx = X-dirX;
  int pattern1 = 0;
  int pattern2 = 0;
  for (int x = 0; x<3; x++)
  {
    int yy = Y-dirY;
    for (int y = 0; y<3; y++)
    {
      if (bitmaps[level]->isSet(xx, yy))
      {
        pattern1 += 1<<(x+3*y);
        pattern2 += 1<<(y+3*x);
      }
      yy += dirY;
    }
    xx += dirX;
  }
  if (!siblingMasks[min(pattern1, pattern2)])
    return false;

  xx = X/2;
  pattern1 = 0;
  pattern2 = 0;
  if (level > 0)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y/2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level-1]->isSet(xx, yy))
        {
          pattern1 += 1<<(x+2*y);
          pattern2 += 1<<(y+2*x);
        }
        yy += dirY;
      }
      xx += dirX;
    }
  }
  if (parentMasks[min(pattern1, pattern2)]) // the min ensures we remain symettric
    return true;

  pattern1 = 0;
  pattern2 = 0;
  xx = X*2 + 1 - X%2;
  if (level+1 <= depth)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y*2 + 1 - Y%2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level+1]->isSet(xx, yy))
        {
          pattern1 += 1<<(x+2*y);
          pattern2 += 1<<(y+2*x);
        }
        yy += dirY;
      }
      xx += dirX;
    }
  }
  if (childMasks[min(pattern1, pattern2)])
    return true;
  return false;
}

bool Covolver::checkAddRemove(int level, int X, int Y, bool addRemove[6][3][3], int i, bool flip)
{
  int dirX = X%2 ? 1 : -1;
  int dirY = Y%2 ? 1 : -1;
  int xx = X/2 - dirX;
  for (int x = 0; x<3; x++, xx += dirX)
  {
    int yy = Y/2 - dirY;
    for (int y = 0; y<3; y++, yy += dirY)
    {
      if (x==1 && y==1)
        continue;
//       if ((addRemove == parentsRemove && x==2 && y==2) || (addRemove == parentsAdd && x==0 && y==0))
//         continue;
      bool mask = flip ? addRemove[i][y][x] : addRemove[i][x][y];
      if (bitmaps[level-1]->isSet(xx, yy) != mask)
        return false;
    }
  }
  return true;
}

bool Covolver::getNewValueParentsOnly(int level, int X, int Y)
{
  ASSERT(level > 0);
  if (bitmaps[level-1]->isSet(X/2, Y/2)) // then prepare to remove
  {
    for (int i = 0; i<6; i++)
      if (checkAddRemove(level, X, Y, parentsRemove, i, false) || checkAddRemove(level, X, Y, parentsRemove, i, true))
        return false;
    return true;
  }
  else
  {
    for (int i = 0; i<6; i++)
      if (checkAddRemove(level, X, Y, parentsAdd, i, false) || checkAddRemove(level, X, Y, parentsAdd, i, true))
        return true;
    return false;
  }
}

bool Covolver::getNewValueParentsOctagonal2(Image* image, int X, int Y, bool flip, bool extended, int level)
{
  int pattern1 = 0;
  int pattern2 = 0;
  int pattern3 = 0;
  int pattern4 = 0;
  for (int i = 0; i<3; i++)
  {
    for (int j = 0; j<2; j++)
    {
      int I;
      int J;
      if (!flip)
      {
        I = X + i;
        J = Y + j;
      }
      else
      {
        I = X + j;
        J = Y + i;
      }
      if (image->isSet(I, J))
      {
        pattern1 += 1<<(i + 3*j);
        pattern2 += 1<<((2-i) + 3*j);
        pattern3 += 1<<(i + 3*(1-j));
        pattern4 += 1<<((2-i) + 3*(1-j));
      }
    }
  }
  if (extended)
  {
    int i = 1;
    for (int j = -1; j<=2; j+=3)
    {
      int I;
      int J;
      if (!flip)
      {
        I = X + i;
        J = Y + j;
      }
      else
      {
        I = X + j;
        J = Y + i;
      }
      I = clamped(I, 0, 1<<level);
      J = clamped(J, 0, 1<<level);
      if (image->isSet(I, J))
      {
        pattern1 += j==-1 ? 64 : 128;
        pattern2 += j==-1 ? 64 : 128;
        pattern3 += j==-1 ? 128 : 64;
        pattern4 += j==-1 ? 128 : 64;
      }
    }
    int minPattern = min(pattern1, min(pattern2, min(pattern3, pattern4)));
    if (minPattern <= 127)
      return octagonalMasks[minPattern];
    else 
      return !octagonalMasks[255 - minPattern];
  }
  return octagonalMasks[min(pattern1, min(pattern2, min(pattern3, pattern4)))];
}

bool Covolver::getNewValue2(int level, int X, int Y)
{
  int dirX = X%2 ? 1 : -1;
  int dirY = Y%2 ? 1 : -1;

  int xx = X-dirX;
  // First, num neighbours
  int pattern1 = 0;
  int pattern2 = 0;
  int numNeighbours = 0;
  bool centreSet = false;
  for (int x = 0; x<3; x++)
  {
    int yy = Y-dirY;
    for (int y = 0; y<3; y++)
    {
      if (bitmaps[level]->isSet(xx, yy))
      {
        if (x==1 && y==1)
          centreSet = true;
        else
          numNeighbours++;
      }
      yy += dirY;
    }
    xx += dirX;
  }

  xx = X/2;
  bool hasParent = level > 0 ? bitmaps[level-1]->isSet(X/2, Y/2) : 0;

  int numChildren = 0;
  xx = X*2 + 1 - X%2;
  if (level+1 <= depth)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y*2 + 1 - Y%2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level+1]->isSet(xx, yy))
          numChildren++;
        yy += dirY;
      }
      xx += dirX;
    }
  }
  return siblingMasks[((numNeighbours + numChildren*8)*2 + (int)hasParent) * 2 + (int)centreSet];
}

bool Covolver::getNewValueSimple(int level, int X, int Y)
{
  int dirX = X%2 ? 1 : -1;
  int dirY = Y%2 ? 1 : -1;

  int xx = X-dirX;
  // First, num neighbours
  int pattern1 = 0;
  int pattern2 = 0;
  int numNeighbours = 0;
  bool centreSet = false;
  for (int x = 0; x<3; x++)
  {
    int yy = Y-dirY;
    for (int y = 0; y<3; y++)
    {
      if (bitmaps[level]->isSet(xx, yy))
      {
        if (type != 2 && x==1 && y==1)
          centreSet = true;
        else
          numNeighbours++;
      }
      yy += dirY;
    }
    xx += dirX;
  }

  xx = X/2;
  if (level > 0)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y/2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level-1]->isSet(xx, yy))
          numNeighbours++;
        yy += dirY;
      }
      xx += dirX;
    }
  }

  xx = X*2 + 1 - X%2;
  if (level+1 <= depth)
  {
    for (int x = 0; x<2; x++)
    {
      int yy = Y*2 + 1 - Y%2;
      for (int y = 0; y<2; y++)
      {
        if (bitmaps[level+1]->isSet(xx, yy))
          numNeighbours++;
        yy += dirY;
      }
      xx += dirX;
    }
  }
  if (type == 4)
    return totalMasks[centreSet ? numNeighbours : numNeighbours + 16]!=0;
  else
    return totalMasks[numNeighbours]!=0;
}


void Covolver::update()
{
  // This performs the covolution
  frame++;
  if (type == 6)
  {
    if (frame == 1)
    {
      for (int level = 3; level<=depth; level++)
      {
        int size = 1<<level;
        for (int i = 0; i<size; i++)
        {
          for (int j = 0; j<size; j++)
          {
            int& pixel = bitmaps[level]->pixel(i, j);
            pixel = getNewValueParentsOnly(level, i, j) ? 192 : 0;
          }
        }
      }
    }
    return;
  }
  if (type == 7 || type == 8)
  {
    if (frame == 1)
    {
      bool extended = frame==8;
      for (int level = 3; level<=depth; level++)
      {
        Image* mapFrom = bitmaps[level-1];
        Image* mapTo = bitmapDuals[level];
        int size = 1<<level;
        for (int i = 0; i<size/2; i++)
        {
          for (int j = 0; j<size/2 + 1; j++)
          {
            int toX = i + j;
            int toY = size/2 - 1 + j-i;
            int& pixel = mapTo->pixel(toX, toY);
            pixel = getNewValueParentsOctagonal2(mapFrom, i-1, j-1, false, extended, level-1) ? 192 : 0;
          }
        }
        for (int i = 0; i<size/2 + 1; i++)
        {
          for (int j = 0; j<size/2; j++)
          {
            int toX = i + j;
            int toY = size/2 + j-i;
            int& pixel = mapTo->pixel(toX, toY);
            pixel = getNewValueParentsOctagonal2(mapFrom, i-1, j-1, true, extended, level-1) ? 192 : 0;
          }
        }

        mapFrom = mapTo;
        mapTo = bitmaps[level];
        for (int i = 0; i<size; i++)
        {
          for (int j = 0; j<size; j++)
          {
            int& pixel = mapTo->pixel(i, j);
            int fromX = -1 + (1 + i+j)/2;
            int fromY = -1 + (size + j-i)/2;
            pixel = getNewValueParentsOctagonal2(mapFrom, fromX, fromY, (i+j)%2, extended, level) ? 192 : 0;
          }
        }
      }
    }
    return;
  }
  // I need to get the timing algorithm right here...
  int currentLevel = -1;
  for (int i = 0; i<32 && currentLevel==-1; i++)
    if ((1<<i) & frame)
      currentLevel = i; // should go 0,1,0,2,0,1,0,3,...
  currentLevel = depth - currentLevel; // so most common is the highest detail.
  if (currentLevel == 1)
  {
    currentLevel = depth;
    frame = 1;
  }
  ASSERT(currentLevel <= depth && currentLevel > 1);

  int size = 1<<currentLevel;
  if (type < 3 || type == 4)
  {
    for (int i = 0; i<size; i++)
    {
      for (int j = 0; j<size; j++)
      {
        int& pixel = bitmaps[currentLevel]->pixel(i, j);
        adjustPixel(pixel, getNewValueSimple(currentLevel, i, j));
      }
    }
  }
  else if (type == 3)
  {
    for (int i = 0; i<size; i++)
    {
      for (int j = 0; j<size; j++)
      {
        int& pixel = bitmaps[currentLevel]->pixel(i, j);
        adjustPixel(pixel, getNewValue(currentLevel, i, j));
      }
    }
  }
  else if (type == 5)
  {
    for (int i = 0; i<size; i++)
    {
      for (int j = 0; j<size; j++)
      {
        int& pixel = bitmaps[currentLevel]->pixel(i, j);
        adjustPixel(pixel, getNewValue2(currentLevel, i, j));
      }
    }
  }
        
  for (int i = 0; i<size; i++)
    for (int j = 0; j<size; j++)
      bitmaps[currentLevel]->pixel(i, j) >>= 1;
}