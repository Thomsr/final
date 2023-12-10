// #include "stdafx.h"
#include <stdlib.h>
#include "GL/glut.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#include <vector>
#include <iostream>
#include <mutex>

#define fParticles 50
#define maxFireworks 7
#define maxBuildings 50

float g_posX = 0.0, g_posY = 300.0, g_posZ = 250.0;
int g_width = 500, g_height = 500;
float g_orientation = 0.0; // y axis

SDL_mutex *mutex;

enum fMode
{
  STANDARD,
  GRADIENT,
  EXPLODE2,
  T,
};

struct explodeInfo
{
  float width;
  float x, y, z;
  float v_x, v_y, v_z;
  float a_x, a_y, a_z;
  float r, g, b, a;
};

struct fireworkInfo
{
  float max_y, v_y;
  float time;
  float x, y, z;
  int active;
  int light;
  fMode mode;
  explodeInfo particles[fParticles * fParticles];
};

struct buildingInfo
{
  float width, height, x, z;
};

struct Vector2
{
  float x, y;
};

Vector2 Tfirework[1000];

fireworkInfo fireworks[maxFireworks];
buildingInfo buildings[maxBuildings];

float lerp(float a, float b, float t)
{
  return a + t * (b - a);
}

int soundThreadExplode(void *data)
{
  (void)data;
  Mix_Chunk *sound = Mix_LoadWAV("firework.wav");
  if (sound == nullptr)
  {
    std::cerr << "Error: Unable to load sound file - "
              << Mix_GetError() << std::endl;
  }
  Mix_PlayChannel(-1, sound, 0);
  if (Mix_Playing(-1) == -1)
  {
    std::cerr << "Error: Unable to load sound file - "
              << Mix_GetError() << std::endl;
  }
  return 0;
}

int soundThreadFire(void *data)
{
  (void)data;
  Mix_Chunk *sound = Mix_LoadWAV("fireworkfire.wav");
  if (sound == nullptr)
  {
    std::cerr << "Error: Unable to load sound file - "
              << Mix_GetError() << std::endl;
  }
  Mix_PlayChannel(-1, sound, 0);
  if (Mix_Playing(-1) == -1)
  {
    std::cerr << "Error: Unable to load sound file - "
              << Mix_GetError() << std::endl;
  }
  return 0;
}

void playExplodeSound()
{
  SDL_LockMutex(mutex);
  SDL_Thread *thread = SDL_CreateThread(soundThreadExplode, "SoundThreadExplode", NULL);
  if (thread == NULL)
  {
    std::cerr << "Error: Unable to create sound thread: " << SDL_GetError() << std::endl;
  }
  SDL_UnlockMutex(mutex);
}

void playFireSound()
{
  SDL_LockMutex(mutex);
  SDL_Thread *thread = SDL_CreateThread(soundThreadFire, "SoundThreadFire", NULL);
  if (thread == NULL)
  {
    std::cerr << "Error: Unable to create sound thread: " << SDL_GetError() << std::endl;
  }
  SDL_UnlockMutex(mutex);
}

void activateLight(float red, float green, float blue, fireworkInfo *firework)
{
  GLfloat lightColor[] = {red, green, blue, 0.0f};
  GLfloat lightPos[] = {firework->x, firework->y, firework->z, 1.0};
  glLightfv(GL_LIGHT0 + firework->light, GL_SPECULAR, lightColor);
  glLightfv(GL_LIGHT0 + firework->light, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT0 + firework->light, GL_POSITION, lightPos);

  glLightf(GL_LIGHT0 + firework->light, GL_CONSTANT_ATTENUATION, 1.0);
  glEnable(GL_LIGHT0 + firework->light);
}

void explode(fireworkInfo *firework)
{
  float r = 10.0;
  int a = 10;
  int index = 0;

  float red = (rand() / float(RAND_MAX));
  float green = (rand() / float(RAND_MAX));
  float blue = (rand() / float(RAND_MAX));

  for (double theta = 0; theta < 2 * M_PI; theta += 2 * M_PI / fParticles)
  {
    for (double phi = 0; phi < M_PI; phi += M_PI / fParticles)
    {
      explodeInfo *particle = &firework->particles[index];
      particle->width = 1;
      particle->x = r * sin(phi) * cos(theta) + firework->x;
      particle->y = r * cos(phi) + firework->y;
      particle->z = r * sin(phi) * sin(theta) + firework->z;
      particle->v_x = (r + a) * sin(phi) * cos(theta) + firework->x - particle->x;
      particle->v_y = (r + a) * cos(phi) + firework->y - particle->y + a * 2;
      particle->v_z = (r + a) * sin(phi) * sin(theta) + firework->z - particle->z;
      particle->a_x = 0;
      particle->a_y = 0;
      particle->a_z = 0;

      if (firework->mode == GRADIENT)
      {
        float value = lerp(0.0, 1.0, 1 - (particle->y - (r * cos(0) + firework->y)) / float((r * cos(M_PI) + firework->y) - (r * cos(0) + firework->y)));
        particle->r = value;
        particle->g = 1 - value;
        particle->b = 2 - value;
      }
      else
      {
        particle->r = red;
        particle->g = green;
        particle->b = blue;
      }
      particle->a = 1;
      index++;
    }
  }

  if (firework->mode == GRADIENT)
    activateLight(.5, .5, 1, firework);
  else
    activateLight(red, green, blue, firework);

  glutGet(GLUT_ELAPSED_TIME);
}

void explode2(fireworkInfo *firework)
{
  float v_y = 30;

  float red = (rand() / float(RAND_MAX));
  float green = (rand() / float(RAND_MAX));
  float blue = (rand() / float(RAND_MAX));

  for (int i = 0; i < fParticles; i++)
  {
    explodeInfo *particle = &firework->particles[i];
    particle->width = 2;
    particle->x = firework->x;
    particle->y = firework->y + (rand() / float(RAND_MAX)) * 10;
    particle->z = firework->z;

    particle->v_x = (rand() / float(RAND_MAX)) * 20 - 10;
    particle->v_y = v_y;
    particle->v_z = (rand() / float(RAND_MAX)) * 20 - 10;

    particle->a_x = 0;
    particle->a_y = 0;
    particle->a_z = 0;

    particle->r = red;
    particle->g = green;
    particle->b = blue;
    particle->a = 1;
  }

  activateLight(red, green, blue, firework);

  glutGet(GLUT_ELAPSED_TIME);
}

void explodeT(fireworkInfo *firework)
{
  float red = (rand() / float(RAND_MAX));
  float green = (rand() / float(RAND_MAX));
  float blue = (rand() / float(RAND_MAX));

  for (int i = 0; i < 1000; i++)
  {
    explodeInfo *particle = &firework->particles[i];
    particle->width = 2;
    particle->x = firework->x;
    particle->y = firework->y;
    particle->z = firework->z;

    particle->v_x = Tfirework[i].x / 5;
    particle->v_y = Tfirework[i].y / 5 + 30;
    particle->v_z = 0;

    particle->a_x = 0;
    particle->a_y = 0;
    particle->a_z = 0;

    particle->r = red;
    particle->g = green;
    particle->b = blue;
    particle->a = 1;
  }

  activateLight(red, green, blue, firework);
}

void fireFirework(fMode mode)
{
  for (int i = 0; i < maxFireworks; i++)
  {
    if (fireworks[i].active == 0)
    {
      fireworks[i].max_y = (rand() / float(RAND_MAX)) * 400 + 200;
      fireworks[i].time = 0;
      fireworks[i].v_y = 200;
      fireworks[i].y = 0;
      fireworks[i].x = (rand() / float(RAND_MAX)) * 200 - 100;
      fireworks[i].z = (rand() / float(RAND_MAX)) * 50 - 25;
      fireworks[i].active = 1;
      fireworks[i].mode = mode;
      playFireSound();
      return;
    }
  }
}

void drawOneParticle()
{
  glBegin(GL_TRIANGLE_STRIP);
  // triangle 1
  glVertex3f(-0.5, 0.0, 0.5); // A
  glVertex3f(0.0, 0.0, -0.5); // B
  glVertex3f(0.0, 1.0, 0.0);  // top
  // triangle 2
  glVertex3f(0.5, 0.0, 0.5); // C
  // triangle 3
  glVertex3f(-0.5, 0.0, 0.5); // A again
  // triangle 4 (bottom)
  glVertex3f(0.0, 0.0, -0.5); // B again

  glEnd();
}

void drawCylinder()
{
  float numSegments = 30.0;
  float height = 3;
  float radius = .5;
  glBegin(GL_QUAD_STRIP);

  for (int i = 0; i <= numSegments; ++i)
  {
    float theta = (2.0f * M_PI * i) / numSegments;
    glVertex3f(radius * cos(theta), -height / 2.0f, radius * sin(theta));
    glVertex3f(radius * cos(theta), height / 2.0f, radius * sin(theta));
  }

  glEnd();
}

void drawParticles()
{
  for (auto f : fireworks)
  {
    for (int index = 0; index < fParticles * fParticles; index++)
    {
      glPushMatrix();
      glTranslatef(f.particles[index].x, f.particles[index].y, f.particles[index].z);
      glScalef(f.particles[index].width, f.particles[index].width, f.particles[index].width);
      glColor4f(f.particles[index].r, f.particles[index].g, f.particles[index].b, f.particles[index].a);
      drawOneParticle();
      glPopMatrix();
    }
  }
}

void drawFirework()
{
  for (int i = 0; i < maxFireworks; i++)
  {
    glPushMatrix();
    glTranslatef(fireworks[i].x, fireworks[i].y, fireworks[i].z);
    if (fireworks[i].active == 1)
    {
      glScalef(2, 5, 2);
    }
    else
    {
      glScalef(0, 0, 0);
    }
    glColor4f(1, 1, 1, fireworks[i].active == 1);
    drawCylinder();
    glPopMatrix();
  }
}

void drawBuilding(int height)
{
  glColor3f(0.5, 0.5, 0.5);
  // Back
  glNormal3f(0, 0, -1);
  glBegin(GL_QUADS);
  glVertex3f(-50.0, 0, -50.0);
  glVertex3f(50.0, 0, -50.0);
  glVertex3f(50.0, height, -50.0);
  glVertex3f(-50.0, height, -50.0);

  // Front
  glNormal3f(0, 0, 1);
  glVertex3f(-50.0, 0, 50.0);
  glVertex3f(50.0, 0, 50.0);
  glVertex3f(50.0, height, 50.0);
  glVertex3f(-50.0, height, 50.0);

  // Right
  glNormal3f(-1, 0, 0);
  glVertex3f(-50.0, 0, -50.0);
  glVertex3f(-50.0, height, -50.0);
  glVertex3f(-50.0, height, 50.0);
  glVertex3f(-50.0, 0, 50.0);

  // Left
  glNormal3f(1, 0, 0);
  glVertex3f(50.0, 0, -50.0);
  glVertex3f(50.0, height, -50.0);
  glVertex3f(50.0, height, 50.0);
  glVertex3f(50.0, 0, 50.0);

  // Top
  glNormal3f(0, 1, 0);
  glVertex3f(50.0, height, -50.0);
  glVertex3f(-50.0, height, -50.0);
  glVertex3f(-50.0, height, 50.0);
  glVertex3f(50.0, height, 50.0);
  glEnd();
}

void drawBuildings()
{
  for (int i = 0; i < maxBuildings; i++)
  {
    glPushMatrix();
    glTranslatef(buildings[i].x, 0, buildings[i].z);
    glScalef(buildings[i].width, 1, buildings[i].width);
    drawBuilding(buildings[i].height);
    glPopMatrix();
  }
}

void keyboard(unsigned char key, int x, int y)
{
  (void)x, (void)y;
  switch (key)
  {
  case 'a': // up
    g_posY = g_posY + 1.0;
    break;
  case 'z': // down
    g_posY = g_posY - 1.0;
    break;
  case 'j': // left
    g_orientation = g_orientation - 15.0;
    break;
  case 'l': // right
    g_orientation = g_orientation + 15.0;
    break;
  case 'i': // forwards
    g_posX = g_posX + sin(g_orientation / 180.0 * M_PI);
    g_posZ = g_posZ - cos(g_orientation / 180.0 * M_PI);
    break;
  case 'k': // forwards
    g_posX = g_posX - sin(g_orientation / 180.0 * M_PI);
    g_posZ = g_posZ + cos(g_orientation / 180.0 * M_PI);
    break;
  case 'f':
    fireFirework(STANDARD);
    break;
  case 'g':
    fireFirework(GRADIENT);
    break;
  case 'd':
    fireFirework(EXPLODE2);
    break;
  case 't':
    fireFirework(T);
    break;
  case 'q': // exit
    exit(0);
    break;
  }
  glutPostRedisplay();
}

void update()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glRotatef(g_orientation, 0.0, 1.0, 0.0); // rotate in y axis
  glTranslatef(-g_posX, -g_posY, -g_posZ);

  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);

  glColor3f(.65, .65, .65);
  glNormal3f(0, 1, 0);
  glBegin(GL_QUADS);
  glVertex3f(5000, 0, -5000);
  glVertex3f(-5000, 0, -5000);
  glVertex3f(-5000, 0, 5000);
  glVertex3f(5000, 0, 5000);
  glEnd();

  drawParticles();
  drawFirework();
  drawBuildings();

  glFlush();
}

void reshape(int width, int height)
{
  g_width = width;
  g_height = height;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, g_width, g_height);
  glLoadIdentity();
  gluPerspective(120.0, (float)(g_width) / g_height, 1.0, 1000.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void removeParticles(fireworkInfo *firework)
{
  for (int i = 0; i < fParticles * fParticles; i++)
  {
    firework->particles[i].width = 0;
  }
}

void explodeTimer(fireworkInfo *firework, float time)
{
  for (int i = 0; i < fParticles * fParticles; i++)
  {
    firework->particles[i].x += firework->particles[i].v_x * time;
    firework->particles[i].y += firework->particles[i].v_y * time;
    firework->particles[i].z += firework->particles[i].v_z * time;

    firework->particles[i].x += firework->particles[i].v_x * time;
    firework->particles[i].y += firework->particles[i].v_y * time;
    firework->particles[i].z += firework->particles[i].v_z * time;

    firework->particles[i].width -= firework->particles[i].width <= 0 ? 0 : 0.2 * time;

    if (firework->particles[i].y != 0)
    {
      firework->particles[i].v_y -= 9.8 * time;
    }
    if (firework->particles[i].y < 0)
    {
      firework->particles[i].y = 0;
      firework->particles[i].a_y = 0;
      firework->particles[i].a_x = 0;
      firework->particles[i].a_z = 0;
      firework->particles[i].v_y = 0;
      firework->particles[i].v_x = 0;
      firework->particles[i].v_z = 0;
    }
  }

  firework->time += time;
  if (firework->time > 5)
    firework->active = 0;
}

void fireworkTimer(fireworkInfo *firework, float time)
{
  {
    if (firework->active == 1)
    {
      firework->y += firework->v_y * time;
      if (firework->y >= firework->max_y)
      {
        firework->active = 2;
        playExplodeSound();
        switch (firework->mode)
        {
        case EXPLODE2:
          explode2(firework);
          break;
        case T:
          explodeT(firework);
          break;
        default:
          explode(firework);
          break;
        }
      }
    }
  }
}

void timer(int value)
{
  (void)value;
  static int lastTime;
  int thisTime;
  float time;
  thisTime = glutGet(GLUT_ELAPSED_TIME);
  time = (thisTime - lastTime) / 500.0;
  lastTime = thisTime;

  for (int i = 0; i < maxFireworks; i++)
  {
    switch (fireworks[i].active)
    {
    case 1:
      fireworkTimer(&fireworks[i], time);
      break;
    case 2:
      explodeTimer(&fireworks[i], time);
      break;
    default:
      removeParticles(&fireworks[i]);
      glDisable(GL_LIGHT0 + fireworks[i].light);
      break;
    }
  }

  glutPostRedisplay();
  glutTimerFunc(50, &timer, 0);
}

void initBuildings()
{
  for (int i = 0; i < maxBuildings; i++)
  {
    buildings[i].x = (rand() / float(RAND_MAX)) * 3000 - 1500;
    buildings[i].z = (rand() / float(RAND_MAX)) * 400 - 500;
    buildings[i].width = (rand() / float(RAND_MAX)) + .5;
    buildings[i].height = (rand() / float(RAND_MAX)) * 300 + 300;
  }
}

void initT()
{
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 50; j++)
    {
      Tfirework[i * 50 + j].x = j - 25;
      Tfirework[i * 50 + j].y = i - 5;
    }
  }

  for (int i = 0; i < 50; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      Tfirework[500 + i * 10 + j].x = j - 5;
      Tfirework[500 + i * 10 + j].y = i - 50;
    }
  }
}

void initFireworks()
{
  for (int i = 0; i < maxFireworks; i++)
  {
    fireworks[i].light = i + 1;
  }
}

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_AUDIO) < 0)
  {
    printf("Error: SDL init\n");
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
  {
    printf("Error: mix openaudio\n");
  }
  if (Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD) != (MIX_INIT_FLAC | MIX_INIT_MOD))
  {
    printf("Error: mix_init\n");
  }

  float bgcolor[4] = {0.0, 0.7, 0.5, 1.0};
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bgcolor);

  initBuildings();
  initT();
  initFireworks();

  srand(time(NULL));
  glutInit(&argc, argv);
  glutInitWindowSize(g_width, g_height);
  glutCreateWindow("Fireworks");
  glMatrixMode(GL_PROJECTION);
  gluPerspective(120.0, 1.0, 1.0, 1000.0);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  float ambientLevel[] = {.25, .25, .25, 1};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLevel);
  GLfloat lightColor0[] = {.5, .5, .5, 0.0f};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
  glEnable(GL_LIGHT0);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glutReshapeFunc(&reshape);
  glutDisplayFunc(&update);
  glutKeyboardFunc(&keyboard);
  glutTimerFunc(50, &timer, 0);
  glutMainLoop();

  Mix_CloseAudio();
  SDL_Quit();
  return 0;
}
