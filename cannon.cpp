// #include "stdafx.h"
#include <stdlib.h>
#include "GL/glut.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <vector>
#include <iostream>

#define fParticles 50
#define trailLength 5

float g_posX = 0.0, g_posY = 300.0, g_posZ = 250.0;
float g_orientation = 0.0; // y axis

enum fMode
{
  STANDARD,
  FLICKERING,
  GRADIENT
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
  fMode mode;
  explodeInfo particles[fParticles * fParticles][trailLength];
};

std::vector<fireworkInfo> fireworks;

float lerp(float a, float b, float t)
{
  return a + t * (b - a);
}

int playSound()
{
  SDL_Init(SDL_INIT_AUDIO);

  // load WAV file
  SDL_AudioSpec wavSpec;
  Uint32 wavLength;
  Uint8 *wavBuffer;

  SDL_LoadWAV("firework.wav", &wavSpec, &wavBuffer, &wavLength);

  // open audio device
  SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

  // play audio
  // int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
  SDL_PauseAudioDevice(deviceId, 0);
  return 0;
}

void explode(fireworkInfo *firework)
{
  float r = 10.0;
  int a = 10;
  int index = 0;

  for (double theta = 0; theta < 2 * M_PI; theta += 2 * M_PI / fParticles)
  {
    for (double phi = 0; phi < M_PI; phi += M_PI / fParticles)
    {
      explodeInfo *particle = &firework->particles[index][0];
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
        particle->r = .01;
        particle->g = .5;
        particle->b = .8;
      }
      particle->a = 1;
      index++;
    }
  }

  for (int i = 1; i < trailLength; i++)
  {
    firework->particles[index][i].width = 0;
  }

  // playSound();

  glutGet(GLUT_ELAPSED_TIME);
}

void fireFirework(fMode mode)
{
  fireworkInfo firework;
  firework.max_y = (rand() / float(RAND_MAX)) * 400 + 200;
  firework.time = 0;
  firework.v_y = 50;
  firework.y = 0;
  firework.x = 0;
  firework.z = 0;
  firework.active = 1;
  firework.mode = mode;
  fireworks.push_back(firework);
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
  int i = 0;
  for (auto f : fireworks)
  {
    for (int index = 0; index < fParticles * fParticles; index++)
    {

      glPushMatrix();
      glTranslatef(f.particles[index][i].x, f.particles[index][i].y, f.particles[index][i].z);
      glScalef(f.particles[index][i].width, f.particles[index][i].width, f.particles[index][i].width);
      glColor4f(f.particles[index][i].r, f.particles[index][i].g, f.particles[index][i].b, f.particles[index][i].a);
      drawOneParticle();
      glPopMatrix();
    }
  }
}

void drawFirework()
{
  for (size_t i = 0; i < fireworks.size(); i++)
  {
    glPushMatrix();
    glTranslatef(fireworks[i].x, fireworks[i].y, fireworks[i].z);
    glScalef(2, 5, 2);
    glColor4f(1, 1, 1, fireworks[i].active);
    drawCylinder();
    glPopMatrix();
  }
}

void keyboard(unsigned char key, int x, int y)
{
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
  case 's':
    fireFirework(STANDARD);
    break;
  case 'f':
    fireFirework(FLICKERING);
    break;
  case 'g':
    fireFirework(GRADIENT);
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

  glColor3f(1.0, 1.0, 1.0);
  // cannon base
  glBegin(GL_QUADS);
  glVertex3f(-5.0, 0.0, -5.0);
  glVertex3f(-5.0, 0.0, 5.0);
  glVertex3f(5.0, 0.0, 5.0);
  glVertex3f(5.0, 0.0, -5.0);
  glEnd();
  // ground plane
  glBegin(GL_LINE_STRIP);
  glVertex3f(-40.0, 0.0, -40.0);
  glVertex3f(-40.0, 0.0, 40.0);
  glVertex3f(40.0, 0.0, 40.0);
  glVertex3f(40.0, 0.0, -40.0);
  glVertex3f(-40.0, 0.0, -40.0);
  glEnd();

  drawParticles();
  drawFirework();
  glFlush();
}

void explodeTimer(fireworkInfo *firework, float time)
{
  for (int i = 0; i < fParticles * fParticles; i++)
  {
    firework->particles[i][0].x += firework->particles[i][0].v_x * time;
    firework->particles[i][0].y += firework->particles[i][0].v_y * time;
    firework->particles[i][0].z += firework->particles[i][0].v_z * time;

    firework->particles[i][0].x += firework->particles[i][0].v_x * time;
    firework->particles[i][0].y += firework->particles[i][0].v_y * time;
    firework->particles[i][0].z += firework->particles[i][0].v_z * time;

    if (firework->mode == FLICKERING)
      firework->particles[i][0].a = (rand() / float(RAND_MAX));
    else
      firework->particles[i][0].a -= .01;

    if (firework->particles[i][0].y != 0)
    {
      firework->particles[i][0].v_y -= 9.8 * time;
    }
    if (firework->particles[i][0].y < 0)
    {
      firework->particles[i][0].y = 0;
      firework->particles[i][0].a_y = 0;
      firework->particles[i][0].a_x = 0;
      firework->particles[i][0].a_z = 0;
      firework->particles[i][0].v_y = 0;
      firework->particles[i][0].v_x = 0;
      firework->particles[i][0].v_z = 0;
    }
  }
  firework->time += time;
  if (firework->time > 20)
    firework->active = 2;
}

void fireworkTimer(fireworkInfo *firework, float time)
{
  {
    if (firework->active == 1)
    {
      firework->y += firework->v_y * time;
      if (firework->y >= firework->max_y)
      {
        firework->active = 0;
        explode(firework);
      }
    }
  }
}

void timer(int value)
{
  static int lastTime;
  int thisTime;
  float time;
  thisTime = glutGet(GLUT_ELAPSED_TIME);
  time = (thisTime - lastTime) / 500.0;
  lastTime = thisTime;

  for (auto &firework : fireworks)
  {
    if (firework.active == 2)
      break;
    if (firework.active == 0)
      explodeTimer(&firework, time);
    else
      fireworkTimer(&firework, time);
  }

  glutPostRedisplay();
  glutTimerFunc(50, &timer, 0);
}

int main(int argc, char *argv[])
{
  srand(time(NULL));
  glutInit(&argc, argv);
  glutInitWindowSize(500, 500);
  glutCreateWindow("Fireworks");
  glMatrixMode(GL_PROJECTION);
  gluPerspective(120.0, 1.0, 1.0, 1000.0);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glutDisplayFunc(&update);
  glutKeyboardFunc(&keyboard);
  glutTimerFunc(50, &timer, 0);
  glutMainLoop();
  return 0;
}
