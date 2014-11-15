﻿#include "View.h"
#include "Screen.h"
#include <conio.h>
#include <string.h>

int g_type = 1;
int g_fullView = 0;
bool g_timeSymmetric = false;
static int numEvolvers = 1;//7;

View::View(int width, int height)
{
  this->width = width;
  this->height = height;

  evolvers[0] = new Evolver(10); // so we can draw it bigger
  evolvers[0]->depth = 8;
  for (int i = 1; i<numEvolvers; i++)
    evolvers[i] = new Evolver(8);

  printf("Relativistic Fractal Search Tool\n");
  printf("Click on your preferred of the seven systems to bring it to the top and generate six new mutated versions below\n");
  printf("Press key 'l' to load, 's' to save, 'f' to toggle full view, with this window in focus.\n");
  printf("Press 't' key to toggle time symmetric mode on/off\n");
  printf("Number keys are mapping families to search within:\n");
  printf("'1' for num neighbours type (default)\n");
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
  if (g_fullView)
  {
    glRasterPos2i(0, 0);
    evolvers[0]->draw();
  }
  else
  {
    glRasterPos2i(256, 512);
    evolvers[0]->draw();

//     int c = 1;
//     for (int j = 1; j>=0; j--)
//     {
//       for (int i = 0; i<3; i++)
//       {
//         glRasterPos2i(256*i, 256*j);
//         evolvers[c++]->draw();
//       }
//     }
  }
}

void View::setMaster(int m)
{
  if (g_type != evolvers[m]->type)
    printf("Now using type %d\n", evolvers[m]->type);
  g_type = evolvers[m]->type;
  if (m!=0)
    evolvers[0]->set(*evolvers[m]);
  evolvers[0]->frame = 0;
  if (!g_fullView)
  {
    for (int i = 0; i<numEvolvers; i++)
      evolvers[i]->randomiseMasks(*evolvers[0], 50.0f);
  }
}

void View::resetFromHead()
{
  if (g_type != evolvers[0]->type)
    printf("Now using type %d\n", evolvers[0]->type);

  g_type = evolvers[0]->type;
  for (int i = 0; i<numEvolvers; i++)
  {
    evolvers[i]->randomiseMasks(*evolvers[0], 50.0f); // to swap data with probability 50% is the same as setting it, it doesn't matter what it was before
    evolvers[i]->frame = 0; 
  }
}


void View::load()
{
  char ext[5];
  sprintf_s(ext, ".km%d", g_type);
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
  evolvers[0]->load(key, g_type);
  for (int i = 1; i<numEvolvers; i++)
    evolvers[i]->randomiseMasks(*evolvers[0], (float)i*2);
  printf("File loaded\n");
}


void View::save()
{
  char ext[5];
  sprintf_s(ext, ".km%d", g_type);
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
  evolvers[0]->write(fp);
  fclose(fp);
  printf("File saved\n");
}

void View::update()
{
  if (g_fullView)
    evolvers[0]->update();
  else
    for (int i = 0; i<numEvolvers; i++)
      evolvers[i]->update();

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
      {
        g_fullView = (g_fullView + 1)%3;
        evolvers[0]->depth = 8 + g_fullView;
        int width = 768;
        if (g_fullView==1)
          width = 512;
        else if (g_fullView == 2)
          width = 1024;
        evolvers[0]->frame = 0;
        if (evolvers[0]->type == 6 || evolvers[0]->type == 7 || evolvers[0]->type == 8)
          evolvers[0]->update();
        // resize the screen here?
        glutReshapeWindow(width, width);
      }
      break;
    
    case('1'):
      evolvers[0]->type = 1;
      resetFromHead();
      break;
    case('2'):
      evolvers[0]->type = 2;
      resetFromHead();
      break;
    case('3'):
      evolvers[0]->type = 3;
      resetFromHead();
      break;
    case('4'):
      evolvers[0]->type = 4;
      resetFromHead();
      break;
    case('5'):
      evolvers[0]->type = 5;
      resetFromHead();
      break;
    case('6'):
      evolvers[0]->type = 6;
      resetFromHead();
      break;
    case('7'):
      evolvers[0]->type = 7;
      resetFromHead();
      break;
    case('8'):
      evolvers[0]->type = 8;
      resetFromHead();
      break;
    case('m'):
      evolvers[0]->randomiseMasks(0, 10.0);
      break;
    case('r'):
      evolvers[0]->randomiseMasks(0, 50.0);
      break;
    default:
      break;
    }
  }
}