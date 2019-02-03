#include <stdlib.h>
#include <glut.h>
#include <stdio.h>
#include "Core.h"
#include "view.h"
#include <time.h>

static Core core; // Just a singleton
static int width = 768;
static int height = 768;
extern int g_fullView;

void init(void)
{    
  srand((unsigned int)time(NULL));
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  core.init(width, height);
}

void display(void)
{
  core.render();
  glutSwapBuffers();
}

void update(void)
{
  core.update(1.0f/60.0f); // avoids outliers
  glutPostRedisplay();
//  glutIdleFunc(NULL); // means you click for each update
}
void reshape(int w, int h)
{
  width = w;
  height = h;
}

void mouse(int button, int state, int x, int y) 
{
   switch (button) 
   {
      case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
          if (x >= 0 && x<384 && y>=0 && y<384) // top left
          {
            core.view->setC((double)x / 384.0, 1.0 - (double)y / 384.0);
          }
          else if (x >= 384 && y < 768 && y >= 0 && y<384) // top right
          {
            double xx = (double)(x - 384) / 384.0;
            double yy = 1.0 - (double)y / 384.0;
            core.view->setMidC(xx, yy);
          }
         break;
      default:
         break;
   }
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);   
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
   glutInitWindowSize(width, height); 
   glutInitWindowPosition(100, 100);
   glutCreateWindow(argv[0]);
   init();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutMouseFunc(mouse);
   glutIdleFunc(update); // starts the updating
   glutMainLoop();
   return 0;
}