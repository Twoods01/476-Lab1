/*
 *  CPE 471 Final Program - aMAZEing
 *  Generates a random maze for the user to navigate.
 *
 *  Created by Taylor Woods on 2/20/14
 *
 *****************************************************************************/
#ifdef __APPLE__
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#define GLFW_INCLUDE_GLU
#include "glfw3.h"
#include "CMeshLoaderSimple.h"
#include "GameObject.hpp"
#include "GLHandles.h"
#include "Airplane.hpp"
#include "Text.h"
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <vector>
#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr

#define NUM_PLANES 10
#define INIT_WIDTH 800
#define INIT_HEIGHT 600
#define pi 3.14159
#define PLANE_HEIGHT 1.25
#define WALL_COLLISION_SIZE .63

//Material selection constants
#define GROUND_MAT 3

//This needs to be forward declared for GLFW
void move(glm::vec3 delta);

using namespace std;
unsigned int const StepSize = 50;

//GL basics
int ShadeProg;
static float g_width, g_height;

//Handles to the shader data
GLHandles handles;

//Ground
GLuint GrndNormalBuffObj, GrndBuffObj, GIndxBuffObj, GrndTexBuffObj;
int g_GiboLen;
vector<glm::vec3> groundTiles;
static const float g_groundY = 0;
static const float g_groundSize = 20.0;

//Airplanes
vector<Airplane> planes;
timeval lastAdded;

//Text
vector<Text> text;
int playerScore = 0;
int frameRate;

//Light
glm::vec3 lightPos;

//Camera
bool overheadView = false;
float overheadHeight = 5.0;
float firstPersonHeight = 1;
glm::vec3 eye = glm::vec3(g_groundSize / 2, firstPersonHeight, g_groundSize / 2);
glm::vec3 lookAt = glm::vec3(g_groundSize / 2 + 1, firstPersonHeight, g_groundSize / 2 + 1);
glm::vec3 overheadEye = glm::vec3(-1, overheadHeight, -1);
glm::vec3 overheadLookAt = eye;
glm::vec3 upV = glm::vec3(0, 1, 0);
float pitch = -pi/4;
float yaw = pi/2;
//Save pitch and yaw when going to overheadView
float eyePitch;
float eyeYaw;

glm::mat4 ortho = glm::ortho(0.0f, (float)g_width,(float)g_height,0.0f, 0.1f, 100.0f);

//User interaction
glm::vec2 prevMouseLoc;

/* projection matrix */
void SetProjectionMatrix() {
   glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);
   safe_glUniformMatrix4fv(handles.uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls */
void SetView() {
   glm::mat4 view;
   if(overheadView)
      view = glm::lookAt(overheadEye, overheadLookAt, upV);
   else
      view = glm::lookAt(eye, lookAt, upV);
   safe_glUniformMatrix4fv(handles.uViewMatrix, glm::value_ptr(view));
}

//Generates a random float within the range min-max
float randomFloat(float min, float max)
{
   return (max - min) * (rand() / (double) RAND_MAX) + min;
}

int diffMs(timeval t1, timeval t2)
{
   return (((t1.tv_sec - t2.tv_sec) * 1000000) +
           (t1.tv_usec - t2.tv_usec))/1000;
}

/* Initialization of objects in the world */
void setWorld()
{
   groundTiles.clear();
   for(int i = 0; i < g_groundSize; i++)
   {
      for(int j = 0; j < g_groundSize; j++)
         groundTiles.push_back(glm::vec3(i, g_groundY, j));
   }
   
   lightPos= glm::vec3(5, 5, 0);
   
   //Send light data to shader
   safe_glUniform3f(handles.uLightColor, lightPos.x, lightPos.y, lightPos.z);
   safe_glUniform3f(handles.uLightColor, 1, 1, 1);
   
   gettimeofday(&lastAdded, NULL);
}


//Add a new plane
void addPlane()
{
   glm::vec3 pos = glm::vec3(randomFloat(.5, g_groundSize - .5),
                             PLANE_HEIGHT,
                             randomFloat(.5, g_groundSize - .5));
   glm::vec3 size = glm::vec3(randomFloat(.5, 1.0));
   float rot  = randomFloat(-180, 180);
   planes.push_back(Airplane(pos, size, rot, handles));
}

//Add a new plane
void addText()
{
   glm::vec3 pos = glm::vec3(randomFloat(.5, g_groundSize - .5),
                             PLANE_HEIGHT,
                             randomFloat(.5, g_groundSize - .5));
   glm::vec3 size = glm::vec3(randomFloat(.5, 1.0));
   float rot  = 0; //randomFloat(-180, 180);
   text.push_back(Text(pos, size, rot, handles));
}

/* Set up matrices to place model in the world */
void SetModel(glm::vec3 loc, glm::vec3 size, float rotation) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 1, 0));
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(handles.uModelMatrix, glm::value_ptr(ortho));
   safe_glUniformMatrix4fv(handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}

/* Set up matrices for ground plane */
void setGround(glm::vec3 loc)
{
   glm::mat4 ctm = glm::translate(glm::mat4(1.0f), loc);
   safe_glUniformMatrix4fv(handles.uModelMatrix, glm::value_ptr(ctm));
   safe_glUniformMatrix4fv(handles.uNormMatrix, glm::value_ptr(glm::mat4(1.0f)));
}

/* Code to create a large ground plane,
 * represented by a list of vertices and a list of indices  */
static void initGround() {
   
   float GrndPos[] = {
      0, g_groundY, 0,
      0, g_groundY, 1,
      1, g_groundY, 1,
      1, g_groundY, 0,
   };
   
   unsigned short idx[] =
   {
      2, 1, 0,
      3, 2, 0,
   };
   
   float grndNorm[] =
   {
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
   };
   
   static GLfloat GrndTex[] = {
      0, 0,
      0, 1,
      1, 0,
      1, 1
   };
   
   g_GiboLen = 6;
   glGenBuffers(1, &GrndBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);
   
   glGenBuffers(1, &GIndxBuffObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
   
   glGenBuffers(1, &GrndNormalBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, GrndNormalBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(grndNorm), grndNorm, GL_STATIC_DRAW);
   
   glGenBuffers(1, &GrndTexBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);
}

/* Initialize the geometry */
void InitGeom() {
   initGround();
}

/*function to help load the shaders (both vertex and fragment */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
   GLuint VS; //handles to shader object
   GLuint FS; //handles to frag shader object
   GLint vCompiled, fCompiled, linked; //status of shader
   
   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);
   
   //load the source
   glShaderSource(VS, 1, &vShaderName, NULL);
   glShaderSource(FS, 1, &fShaderName, NULL);
   
   //compile shader and print log
   glCompileShader(VS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
   printShaderInfoLog(VS);
   
   //compile shader and print log
   glCompileShader(FS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
   printShaderInfoLog(FS);
   
   if (!vCompiled || !fCompiled) {
      printf("Error compiling either shader %s or %s", vShaderName, fShaderName);
      return 0;
   }
   
   //create a program object and attach the compiled shader
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);
   
   glLinkProgram(ShadeProg);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
   printProgramInfoLog(ShadeProg);
   
   glUseProgram(ShadeProg);
   
   /* get handles to attribute and uniform data in shader */
   handles.aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
   handles.aNormal = safe_glGetAttribLocation(ShadeProg,	"aNormal");
   handles.uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   handles.uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   handles.uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
   handles.uNormMatrix = safe_glGetUniformLocation(ShadeProg, "uNormalMatrix");
   handles.uLightPos = safe_glGetUniformLocation(ShadeProg, "uLightPos");
   handles.uLightColor = safe_glGetUniformLocation(ShadeProg, "uLColor");
   handles.uEyePos = safe_glGetUniformLocation(ShadeProg, "uEyePos");
   handles.uMatAmb = safe_glGetUniformLocation(ShadeProg, "uMat.aColor");
   handles.uMatDif = safe_glGetUniformLocation(ShadeProg, "uMat.dColor");
   handles.uMatSpec = safe_glGetUniformLocation(ShadeProg, "uMat.sColor");
   handles.uMatShine = safe_glGetUniformLocation(ShadeProg, "uMat.shine");
   
   printf("sucessfully installed shader %d\n", ShadeProg);
   return 1;
}

/* helper function to set up material for shading */
void SetMaterial(int i) {
   
   glUseProgram(ShadeProg);
   switch (i) {
      case 0:
         safe_glUniform3f(handles.uMatAmb, 0.2, 0.2, 0.2);
         safe_glUniform3f(handles.uMatDif, 0.4, 0.4, 0.4);
         safe_glUniform3f(handles.uMatSpec, 0.2, 0.2, 0.2);
         safe_glUniform1f(handles.uMatShine, .2);
         break;
      case GROUND_MAT:
         safe_glUniform3f(handles.uMatAmb, 0.1, 0.3, 0.1);
         safe_glUniform3f(handles.uMatDif, 0.1, 0.3, 0.1);
         safe_glUniform3f(handles.uMatSpec, 0.3, 0.3, 0.4);
         safe_glUniform1f(handles.uMatShine, 1.0);
         break;
   }
}

/* Some OpenGL initialization */
void Initialize ()
{
	// Start Of User Initialization
	glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
	// Black Background
 	glClearDepth (1.0f);	// Depth Buffer Setup
 	glDepthFunc (GL_LEQUAL);	// The Type Of Depth Testing
	glEnable (GL_DEPTH_TEST);// Enable Depth Testing
}

/* Main display function */
void Draw (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Start our shader
 	glUseProgram(ShadeProg);
   
   /* set up the projection and camera - do not change */
   SetProjectionMatrix();
   SetView();
   
   safe_glUniform3f(handles.uEyePos, eye.x, eye.y, eye.z);
   
   //-------------------------------Ground Plane --------------------------
   safe_glEnableVertexAttribArray(handles.aPosition);
   safe_glEnableVertexAttribArray(handles.aNormal);
   SetMaterial(GROUND_MAT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   for (std::vector<glm::vec3>::iterator it = groundTiles.begin(); it != groundTiles.end(); ++ it) {
      setGround(glm::vec3(it->x, it->y, it->z));
      
      glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      safe_glVertexAttribPointer(handles.aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      
      safe_glEnableVertexAttribArray(handles.aNormal);
      glBindBuffer(GL_ARRAY_BUFFER, GrndNormalBuffObj);
      safe_glVertexAttribPointer(handles.aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
      
      glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);
   }
   
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   SetMaterial(2);
   //--------------------------------Airplanes-------------------------------
   for (std::vector<Airplane>::iterator it = planes.begin(); it != planes.end(); ++ it) {
      it->draw();
   }
   
   for (std::vector<Text>::iterator it = text.begin(); it != text.end(); ++ it) {
      it->draw();
   }
   
   
   //clean up
	safe_glDisableVertexAttribArray(handles.aPosition);
	safe_glDisableVertexAttribArray(handles.aNormal);
   
	//Disable the shader
	glUseProgram(0);
}

/* Reshape - note no scaling as perspective viewing*/
void ReshapeGL (GLFWwindow* window, int width, int height)
{
	g_width = (float)width;
	g_height = (float)height;
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));
}

/* Convert pixel x coordinates into world space */
float p2wx(int in_x) {
   //fill in with the correct return value
   float l, r;
   float c, d;
   
   if(g_width > g_height)
   {
      l = -1 *(g_width) / (g_height);
      r = g_width / g_height;
   }
   else{
      l = -1;
      r = 1;
   }
   
   c = (1 - g_width) / (l - r);
   d = ((g_width - 1) / (l - r)) * l;
   
   return (in_x - d) / c;
}

/* Convert pixel y coordinates into world space */
float p2wy(int in_y) {
   float b, t;
   float e, f;
   //flip glut y
   in_y = g_height - in_y;
   
   if(g_width > g_height)
   {
      b = -1;
      t = 1;
   }
   else{
      b = -1 * (g_height) / (g_width);
      t = g_height / g_width;
   }
   e = (1 - g_height) / (b - t);
   f = ((g_height - 1) / (b - t)) * b;
   
   return (in_y - f) / e;
}

/* Tracks mouse movement for the camera */
void mouse(GLFWwindow* window, double x, double y)
{
   glm::vec2 currentPos = glm::vec2(x, y);
   glm::vec2 delta = currentPos - prevMouseLoc;
   
   pitch += delta.y * (pi / g_height);
   yaw += delta.x * (pi / g_width);
   
   if(pitch > (4*pi/9))
      pitch = 4*pi/9;
   else if(pitch < -(4*pi/9))
      pitch = -4*pi/9;
   
   if(!overheadView)
   {
      lookAt.x = cos(pitch) * cos(yaw) + eye.x;
      lookAt.y = sin(pitch) + eye.y;
      lookAt.z = cos(pitch)*(cos((pi/2) - yaw)) + eye.z;
   }
   else
   {
      overheadLookAt.x = cos(pitch) * cos(yaw) + overheadEye.x;
      overheadLookAt.y = sin(pitch) + overheadEye.y;
      overheadLookAt.z = cos(pitch)*(cos((pi/2) - yaw)) + overheadEye.z;
   }
   
   prevMouseLoc = currentPos;
}

static void error_callback(int error, const char* description)
{
   fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   glm::vec3 delta;
   if(action == GLFW_PRESS || action == GLFW_REPEAT)
   {
      switch( key ) {
         case GLFW_KEY_W:
            if(!overheadView)
               delta = glm::normalize(lookAt - eye);
            else
               delta = glm::normalize(overheadLookAt - overheadEye);
            move(delta);
            break;
         case GLFW_KEY_S:
            if(!overheadView)
               delta = glm::normalize(-(lookAt - eye));
            else
               delta = glm::normalize(-(overheadLookAt - overheadEye));
            move(delta);
            break;
         case GLFW_KEY_D:
            if(!overheadView)
               delta = glm::normalize(glm::cross(upV, -(lookAt - eye)));
            else
               delta = glm::normalize(glm::cross(upV, -(overheadLookAt - overheadEye)));
            move(delta);
            break;
         case GLFW_KEY_A:
            if(!overheadView)
               delta = glm::normalize(glm::cross(upV, (lookAt - eye)));
            else
               delta = glm::normalize(glm::cross(upV, (overheadLookAt - overheadEye)));
            move(delta);
            break;
         case GLFW_KEY_R:
            setWorld();
            break;
         case GLFW_KEY_Q:
            exit( EXIT_SUCCESS );
            break;
         case GLFW_KEY_SPACE:
            overheadView = !overheadView;
            if(overheadView)
            {
               overheadEye.x = eye.x;
               overheadEye.z = eye.z;
               eyePitch = pitch;
               eyeYaw = yaw;
               pitch = -pi/2 + .1;
               overheadLookAt.x = eye.x;// cos(pitch) * cos(yaw) + eye.x;
               overheadLookAt.y = 0;
               overheadLookAt.z = eye.z;//cos(pitch)*(cos((pi/2) - yaw)) + eye.z;
            }
            else
            {
               pitch = eyePitch;
               yaw = eyeYaw;
            }
            SetView();
            break;
      }
   }
}

/*Given a position and a distance from that position calculates
 *If that position would be in the world
 *False if no collision*/
bool detectCollision(glm::vec3 eye, glm::vec3 delta)
{
   float moveX = eye.x + (.1) * delta.x;
   float moveZ = eye.z + (.1) * delta.z;
   
   //Keep the player inside the world
   if ((moveX <= g_groundSize - .5) && (moveX >= .5) && (moveZ <= g_groundSize - .5) && (moveZ >= .5))
   {
      //Check for collision with a plane
      for (std::vector<Airplane>::iterator it = planes.begin(); it != planes.end(); ++ it) {
         if ((moveX <= it->getPos().x + WALL_COLLISION_SIZE && moveX >= it->getPos().x - WALL_COLLISION_SIZE) &&
             (moveZ <= it->getPos().z + WALL_COLLISION_SIZE && moveZ >= it->getPos().z -WALL_COLLISION_SIZE)) {
            it->kill();
            return true;
         }
      }
      return false;
   }
   return true;
}

void move(glm::vec3 delta)
{
   //Don't move if there is a collision, unless we're in overheadView
   //In which case collision detection is ignored
   if(overheadView)
   {
      overheadEye.x += (.1) * delta.x;
      overheadEye.z += (.1) * delta.z;
      overheadLookAt.x += (.1) * delta.x;
      overheadLookAt.z += (.1) * delta.z;
   }
   else if(!detectCollision(eye, delta))
   {
      eye.x += (.1) * delta.x;
      eye.z += (.1) * delta.z;
      lookAt.x += (.1) * delta.x;
      lookAt.z += (.1) * delta.z;
   }
}

void Animate()
{
   timeval curTime;
   gettimeofday(&curTime, NULL);
   if(planes.size() < NUM_PLANES && diffMs(curTime, lastAdded) >= 5000)
   {
      addPlane();
      lastAdded = curTime;
   }
   
   for (std::vector<Airplane>::iterator it = planes.begin(); it < planes.end(); ++ it)
   {
      it->step();
      
      //Plane has crashed, remove it from the vector
      if(it->getPos().y < g_groundY - .5)
      {
         it = planes.erase(it);
      }
      //Plane flew off the left or right border
      else if(it->getPos().x > g_groundSize - .5 || it->getPos().x < .5)
      {
         it->bounce(glm::vec3(1, 0, 0));
      }
      //Plane flew off the top or bottom border
      else if(it->getPos().z > g_groundSize - .5 || it->getPos().z < .5)
      {
         it->bounce(glm::vec3(0, 0, 1));
      }
      else
      {
         //Plane in the world, make sure it didn't hit another plane
         for (std::vector<Airplane>::iterator it2 = planes.begin(); it2 != planes.end(); ++ it2)
         {
            if(it != it2)
            {
               if ((it2->getPos().x <= it->getPos().x + WALL_COLLISION_SIZE &&
                    it2->getPos().x >= it->getPos().x - WALL_COLLISION_SIZE) &&
                   (it2->getPos().z <= it->getPos().z + WALL_COLLISION_SIZE &&
                    it2->getPos().z >= it->getPos().z -WALL_COLLISION_SIZE) &&
                   it->getPos().y == it2->getPos().y) {
                  glm::vec3 planeOrigVel = it->getVel();
                  it->bounce(it2->getVel());
                  it2->bounce(planeOrigVel);
               }
            }
         }
         //Make sure it didn't hit the player
         if ((eye.x <= it->getPos().x + WALL_COLLISION_SIZE &&
              eye.x >= it->getPos().x - WALL_COLLISION_SIZE) &&
             (eye.z <= it->getPos().z + WALL_COLLISION_SIZE &&
              eye.z >= it->getPos().z -WALL_COLLISION_SIZE) &&
             !overheadView)
         {
            it->kill();
         }
      }
   }
}

int main( int argc, char *argv[] )
{
   GLFWwindow* window;
   
   g_width = INIT_WIDTH;
   g_height = INIT_HEIGHT;
   
   glfwSetErrorCallback(error_callback);
   
   if (!glfwInit())
      exit(EXIT_FAILURE);
   
   window = glfwCreateWindow(g_width, g_height, "Crash the Planes!", NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      exit(EXIT_FAILURE);
   }
   
   glfwMakeContextCurrent(window);
   glfwSetKeyCallback(window, key_callback);
   glfwSetCursorPosCallback(window, mouse);
   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   glfwSetWindowSizeCallback(window, ReshapeGL);
   
   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"Phong_vert.glsl"), textFileRead((char *)"Phong_frag.glsl"))) {
      printf("Error installing shader!\n");
      return 0;
   }
   Initialize();
   InitGeom();
   setWorld();
   
   while (!glfwWindowShouldClose(window))
   {
      Animate();
      Draw();
      glfwSwapBuffers(window);
      glfwPollEvents();
   }
   
   glfwDestroyWindow(window);
   
   glfwTerminate();
   exit(EXIT_SUCCESS);
   return 0;
}
