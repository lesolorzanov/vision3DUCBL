#include <iostream>

#include <stdio.h>   // Always a good idea.
#include <string.h>   // Always a good idea.
#include <locale.h>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <GL/gl.h>   // OpenGL itself.
#include <GL/glu.h>  // GLU support library.
#include <GL/glut.h> // GLUT support library.

#include <time.h>

using namespace cv;
using namespace std;

#include <OpenNI.h>

#include "OpenNIOpenCVWrapper.h"

openni::Device m_device;
openni::VideoStream m_depthStream, m_colorStream, m_irStream;
openni::VideoFrameRef        m_depthFrame;
openni::VideoFrameRef        m_colorFrame;
openni::VideoFrameRef        m_irFrame;
Mat cam_depthGRAY, cam_rgb, cam_ir;
Mat drawing;
int Window_ID;

#define DO_IR false

#if 0
#define WIDTH 640
#define HEIGHT 480
#define DEPTH 4
#define RGB 9
#define IR 5
#else
#define WIDTH 320
#define HEIGHT 240
#define DEPTH 1
#define RGB 0
#define IR 1
#endif

bool regMode = false;

unsigned int pix;

Mat src; Mat src_gray;

int windowWidth = WIDTH;
int windowHeight = HEIGHT;


void orthogonalStart()
{
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-windowWidth/2, windowWidth/2, -windowHeight/2, windowHeight/2);
    glMatrixMode(GL_MODELVIEW);
}

void orthogonalEnd()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void FreeTexture( GLuint texture )
{
    glDeleteTextures( 1, &texture );
}

void cbKeyPressed( unsigned char key, int x, int y)
{
    switch (key)
    {
        case 113: case 81: case 27: // Q (Escape) - We're outta here.
            glutDestroyWindow(Window_ID);
            exit(1);
            break; // exit doesn't return, but anyway...

        case 'i':
        case 'I':
            break;

        default:
            printf ("KP: No action for %d.\n", key);
            break;
    }
}


void dessineQuadTexture()
{

    GLuint texture;

    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );


    //even better quality, but this will do for now.
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );


    //to the edge of our shape.
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    //Generate the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0,GL_BGR, GL_UNSIGNED_BYTE, cam_rgb.data);
    glBindTexture( GL_TEXTURE_2D, texture );

    orthogonalStart();
    glColor3f(1.0, 1.0, 1.0);
    const int iw = windowWidth;
    const int ih = windowHeight;

    glPushMatrix();
    glTranslatef( -iw/2, -ih/2, 0 );
    glBegin(GL_QUADS);
        glTexCoord2i(0,1); glVertex2i(0, 0);
        glTexCoord2i(1,1); glVertex2i(iw, 0);
        glTexCoord2i(1,0); glVertex2i(iw, ih);
        glTexCoord2i(0,0); glVertex2i(0, ih);
    glEnd();

    glPopMatrix();

    orthogonalEnd();

    FreeTexture(texture);
}

void ourInit(void)
{
    // Color to clear color buffer to.
    glClearColor(0.1f, 0.1f, 0.1f,1.0f);

    // Depth to clear depth buffer to; type of test.
    //glClearDepth(1.0);
    //glDepthFunc(GL_LESS);

    // Enables Smooth Color Shading; try GL_FLAT for (lack of) fun.
    glShadeModel(GL_SMOOTH);
}

void OpenniStuff(){


    /*******************************
      opennni
      ********************************/

    OpenNI2WrapperOpenCV::GrabFrameFromStream ( m_depthStream, m_depthFrame );

    OpenNI2WrapperOpenCV::ConvertDepthFrameToStream ( m_depthFrame, cam_depthGRAY );

    int pasEch=10;
    int dx=WIDTH/pasEch,dy=HEIGHT/pasEch;

    for (int y = 0; y < cam_depthGRAY.rows-dy; y+=dy)
    {
       short* row_ptr = cam_depthGRAY.ptr<short>(y);
       short* row_ptrplus = cam_depthGRAY.ptr<short>(y+dy);
       for (int x = 0; x < cam_depthGRAY.cols-dx; x+=dx)
       {

           //here you must normalize xy and Z so that the normals are calculated
           //in a nice space XD

           //so first normalize the steps x goes from 0 to width=320 so a normalized would be
           //x=0 u=0; x=32 u=0.1, x=320 u=1
           //same thing for height

           //then; to normalize Z you hqve to find the min and the max in a loop outside this one
           //example Z=5200 (imagine it is the max, 52 centimeters)  u=1
           //example Z=600 (imagine it is the min, 6 centimeters)  u=0

           //only then you will have a nice unit cube to find a more approximate normal and that is it
           //you can place the teapot

           //then come back here and normalize Z (you must find the minimul and maximum)

           glBegin(GL_TRIANGLES);
           glVertex3f(inx,iny,x1);
           glVertex3f(inx+dx,iny,x1);
           glVertex3f(inx,iny+dy,x2);
           glEnd();



           glBegin(GL_TRIANGLES);
           glVertex3f(inx+dx,iny,x1);
           glVertex3f(inx,iny+dy,x2);
           glVertex3f(inx+dx,iny+dy,x2);
           glEnd();
       }
    }

    /*******************************
     END
      opennni
      ********************************/



}


void cbRenderScene(void)
{
    OpenNI2WrapperOpenCV::GrabFrameFromStream ( m_colorStream, m_colorFrame );
    OpenNI2WrapperOpenCV::ConvertColorFrameToStream ( m_colorFrame, cam_rgb );


    // active textures
    glEnable(GL_TEXTURE_2D);

    // active z-buffering pour masquage partie cachee
    glEnable(GL_DEPTH_TEST);

    // Clear the color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    dessineQuadTexture();
    glClear(GL_DEPTH_BUFFER_BIT);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-0.5f*WIDTH,0.5f*WIDTH,-0.5f*HEIGHT,0.5f*HEIGHT,0.0f,1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt( 0., 0., -1.0, 0,0,1, 0., -1., 0.);

    //glutWireTeapot(2);

    OpenniStuff();

    glPopMatrix();
    // All done drawing.  Let's show it.
    glutSwapBuffers();


    //imshow("rgb",cam_rgb);

    //cv::waitKey(1);
}

int main(int argc, char** argv) {


    // pour eviter pb de . et , dans les floats
    setlocale(LC_NUMERIC, "C");

    // initialisation de glut ???
    glutInit(&argc, argv);
    ourInit();
    // To see OpenGL drawing, take out the GLUT_DOUBLE request.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);

    // Open a window
    Window_ID = glutCreateWindow("OpenGL");

    // Register the callback function to do the drawing.
    glutDisplayFunc(&cbRenderScene);

    // If there's nothing to do, draw.
    glutIdleFunc(&cbRenderScene);

    // And let's get some keyboard input.
    glutKeyboardFunc(&cbKeyPressed);

    // OK, OpenGL's ready to go.  Let's call our own init function.


    OpenNI2WrapperOpenCV::InitializeDevice ( NULL, m_device );

    cam_depthGRAY.create(HEIGHT,WIDTH,CV_16SC1);
    OpenNI2WrapperOpenCV::OpenStream ( m_device, openni::SENSOR_DEPTH, m_depthStream );
    OpenNI2WrapperOpenCV::EnumerateStreamVideoModes ( m_depthStream );
    OpenNI2WrapperOpenCV::StartStreamWithVideoMode ( m_depthStream, DEPTH );

    if ( ! DO_IR ) {
        cam_rgb.create(HEIGHT,WIDTH,CV_8UC3);
        OpenNI2WrapperOpenCV::OpenStream ( m_device, openni::SENSOR_COLOR, m_colorStream );
        OpenNI2WrapperOpenCV::EnumerateStreamVideoModes ( m_colorStream );
        OpenNI2WrapperOpenCV::StartStreamWithVideoMode ( m_colorStream, RGB );
    } else {
        cam_ir.create(HEIGHT,WIDTH,CV_8UC3);
        OpenNI2WrapperOpenCV::OpenStream ( m_device, openni::SENSOR_IR, m_irStream );
        OpenNI2WrapperOpenCV::EnumerateStreamVideoModes ( m_irStream );
        OpenNI2WrapperOpenCV::StartStreamWithVideoMode ( m_irStream, IR );
    }

    // Pass off control to OpenGL.
    // Above functions are called as appropriate.
    glutMainLoop();

    if ( ! DO_IR ) {
        m_colorStream.stop();
        m_colorStream.destroy();
    } else {
        m_irStream.stop();
        m_irStream.destroy();
    }
    m_depthStream.stop();
    m_depthStream.destroy();
    m_device.close();
    openni::OpenNI::shutdown();
}

