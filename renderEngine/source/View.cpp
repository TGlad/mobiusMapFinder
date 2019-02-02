#include "View.h"
#include "Screen.h"
#include <conio.h>
#include <string.h>

View::View(int width, int height)
{
  this->width = width;
  this->height = height;

  for (int i = 0; i < 4; i++)
    evolvers[i] = new Evolver(384, i);
}

void View::setMidC(double x, double y)
{
  evolvers[1]->setMidC(x, y);
}
void View::setC(double x, double y)
{
  evolvers[2]->setC(x, y);
  evolvers[3]->setC(x, y);
}


void View::recordToScreen(Screen* screen)
{
  glColor4f(0.3f, 0.5f, 0.7f, 1.0f);   
  glClear(GL_COLOR_BUFFER_BIT);

  // Ortho mode ---------------------------------------------------
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble) screen->width, 0.0, (GLdouble) screen->height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glPixelZoom(1.0f, 1.0f); // so we don't have to rearrange in the setPixel call
  
  glRasterPos2i(0, 384);
  evolvers[0]->draw();
  glRasterPos2i(384, 384);
  evolvers[1]->draw();
  glRasterPos2i(0, 0);
  evolvers[2]->draw();
  glRasterPos2i(384, 0);
  evolvers[3]->draw();
}

void View::load()
{
  char ext[5];
//  sprintf_s(ext, ".km%d", g_type);
  printf("Type name of %s file to load, without extension: ", ext);
  char key[80];
  int c = 0;
  do
  {
    key[c] = _getch();
    if (key[c] == 13)
    {
      key[c] = 0; // null terminate
      printf("\n");
    }
    else
      printf("%c", key[c]);
  } while (key[c++] != 0);
  strcat_s(key, ext);
//  evolvers[0]->load(key, g_type);
 // for (int i = 1; i<numEvolvers; i++)
 //   evolvers[i]->randomiseMasks(*evolvers[0], (float)i*2);
  printf("File loaded\n");
}


void View::save()
{
  char ext[5];
 // sprintf_s(ext, ".km%d", g_type);
  printf("Type name of %s file to save, without extension: ", ext);
  char key[80];
  int c = 0;
  do
  {
    key[c] = _getch();
    if (key[c] == 13)
    {
      key[c] = 0; // null terminate
      printf("\n");
    }
    else
      printf("%c", key[c]);
  } while (key[c++] != 0);
  FILE* fp;
  strcat_s(key, ext);
  if (fopen_s(&fp, key, "wb"))
  {
    printf("Cannot open file for writing: %s\n", key);
    return;
  }
 // evolvers[0]->write(fp);
  fclose(fp);
  printf("File saved\n");
}

void View::update()
{
  if (_kbhit())
  {
    switch(_getch())
    {
    case('l'):
      load();
      break;
    case('s'):
      save();
      break;
    case('f'):
      break;
    default:
      break;
    }
  }
}