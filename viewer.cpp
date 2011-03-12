#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>
#include <new> 
#include "minecraftmap.cpp"


struct rect
{
  int x;
  int y;
};

GLuint textures[256];




char * world_location;

void
assigntextures ()
{
  int width, height;
  char *data;
  FILE *file;
  int texturesize = 16;
  // open texture data
  file = fopen ("texture.raw", "rb");
  if (file == NULL)
    return;

  // allocate buffer
  width = texturesize * 16;
  height = texturesize * 16;
  data = (char *) malloc (width * height * 4);

  // read texture data
  fread (data, width * height * 4, 1, file);
  fclose (file);

  char *data2 = (char *) malloc (width * height * 4);

  // split image into individual textures
  for (int y = 0; y < 16; y++)
    {
      for (int x = 0; x < 16; x++)
	{
	  for (int y1 = 0; y1 < texturesize; y1++)
	    {
	      memcpy (data2 +
		      4 * ((y * 16 + x) * texturesize + y1) * texturesize,
		      data + 4 * ((y * texturesize + y1) * 16 +
				  x) * texturesize, 4 * texturesize);
	    }
	}
    }

  // allocate a texture name
  glGenTextures (16 * 16, textures);
//    for (int i = 0; i < 256; i++) {
  // select our current textureA
  for (int i = 0; i < 16 * 16; i++)
    {
      glBindTexture (GL_TEXTURE_2D, textures[i]);
      glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
      // select modulate to mix texture with color for shading
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // when texture area is small, bilinear filter the closest mipmap
      //    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      //             GL_LINEAR_MIPMAP_NEAREST );
      // when texture area is large, bilinear filter the first mipmap
      //   glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

      // if wrap is true, the texture wraps over at the edges (repeat)
      //       ... false, the texture ends at the edges (clamp)
      //  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
      //               GL_CLAMP );
      // glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
      //                 GL_CLAMP );

      // build our texture mipmaps
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texturesize, texturesize, 0,
		    GL_RGBA, GL_UNSIGNED_BYTE,
		    data2 + i * texturesize * texturesize * 4);

    }
//          gluBuild2DMipmaps( GL_TEXTURE_2D, 3, 512, 512,
  //                     GL_RGBA, GL_UNSIGNED_BYTE, data/* + 256 * 3 * i*/ );
  //}
  // free buffer
  free (data);

}

int getTexture(int blocktype, int side) {
    switch (blocktype) {
	    case 0: 
		 return -1;
	    case 1:
		 return 1;
	    case 2:
		 if (side == 0) {
			 return 30;
		 } else if (side == 5) {
		     return 2;
		 } else {
		     return 3;
		 }
	    case 3:
		 return 2;
	    case 4: return 16;
	    case 5: return 4;
            case 6: return -1;
            case 7: return 17;
	    case 8: return 14;
	    case 9: return 14;
	    case 10: return 237;
	    case 11: return 237;
	    case 12: return 18;
	    case 13: return 19;
	    case 14: return 32;
	    case 15: return 33;
	    case 16: return 34;
	    case 17: if (side == 0 || side == 5) { return 21; } else { return 20; };
	    case 18: return -1;
            case 19: return 48;
	    case 20: return -1;
	    case 21: return 160;
	    case 22: return 144;
	    case 23: if (side == 1) { return 46; } else if (side == 0 || side == 5) { return 62; } else { return 45; }
	    case 24: return 176;
	    case 25: return 74;
	    case 26: return -1;
            case 35: return 64;
   }
   return -1;
}
     				 

int
redraw ()
{
  glClearColor (1.0, 1.0, 1.0, 1.0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluPerspective (90.0, 0.8, 10., 10000.0);
  gluLookAt (0., 100., 0, 50., 48., 50., 0., 1., 0.);

/*	   glBegin(GL_QUADS);
	     glColor3f(1., 0., 0.); glVertex3f(-.75, -.75, 0.);
	       glColor3f(0., 1., 0.); glVertex3f( .75, -.75, 0.);
	         glColor3f(0., 0., 1.); glVertex3f( .75,  .75, 0.);
		   glColor3f(1., 1., 0.); glVertex3f(-.75,  .75, 0.);
		    glEnd();*/
//      glMatrixMode(GL_MODELVIEW);
//      glLoadIdentity();
//      gluLookAt(0, 0, 1000, 0, 0, 0, 0, 10, 0);
  glColor3f (255.0, 0.0, 0.0);
/*	glBegin(GL_LINE_LOOP);
	glVertex2i(100,100);
	glVertex2i(-100,100);
	glVertex2i(-100,-100);
        glEnd();*/

  assigntextures ();
  glEnable (GL_TEXTURE_2D);
  World *a = new World(world_location);
  const unsigned char *blocks[32][32];
  memset (blocks, 0, sizeof(blocks));
  for (int x = 0; x< 32; x++) {
	  for (int z = 0; z < 32; z++) {
		  fprintf(stderr, "%p\n", a);
		  unsigned char * tmp = a->chunk(x,z);
		  if (tmp) {
			  blocks[x][z] = getNBT("Level.Blocks", tmp + 3);
		  } else {
			  blocks[x][z] = 0;
		  }
          }
  }


  for (int x = 0; x < 16 * 16; x++)
    {
      for (int y = 0; y < 128; y++)
	{
	  for (int z = 0; z < 16 * 16; z++)
	    {
	      if (blocks[x/16][z/16]) {
		      int block = blocks[x/16][z/16][y + (z % 16) * 128 + (x % 16) * 128 * 16];

		      int texture;
		      texture = getTexture(block, 0);
		      if (texture != -1) {
			  glBindTexture (GL_TEXTURE_2D, textures[texture]);
		      
			  glBegin (GL_QUADS);
			  glTexCoord2d (0.0, 0.0);
			  glVertex3d (x, y + 1, z);
			  glTexCoord2d (1.0, 0.0);
			  glVertex3d (x + 1, y + 1, z);
			  glTexCoord2d (1.0, 1.0);
			  glVertex3d (x + 1, y + 1, z + 1);
			  glTexCoord2d (0.0, 1.0);
			  glVertex3d (x, y + 1, z + 1);
			  glEnd ();
		      }
		      texture = getTexture(block, 1);
		      if (texture != -1) {
			  glBindTexture (GL_TEXTURE_2D, textures[texture]);
			  glBegin (GL_QUADS);
			  glTexCoord2d (0.0, 0.0);
			  glVertex3d (x, y, z);
			  glTexCoord2d (1.0, 0.0);
			  glVertex3d (x + 1, y, z);
			  glTexCoord2d (1.0, 1.0);
			  glVertex3d (x + 1, y + 1, z);
			  glTexCoord2d (0.0, 1.0);
			  glVertex3d (x, y + 1, z);
			  glEnd ();
		      }
		      texture = getTexture(block, 2);
		      if (texture != -1) {
			  glBindTexture (GL_TEXTURE_2D, textures[texture]);
			  glBegin (GL_QUADS);
			  glTexCoord2d (0.0, 0.0);
			  glVertex3d (x, y, z);
			  glTexCoord2d (1.0, 0.0);
			  glVertex3d (x, y, z + 1);
			  glTexCoord2d (1.0, 1.0);
			  glVertex3d (x, y + 1, z + 1);
			  glTexCoord2d (0.0, 1.0);
			  glVertex3d (x, y + 1, z);
			  glEnd ();
		      }
		    }
	    }
	}
    }
}


int
main (int argc, char *argv[])
{

	world_location = argv[1];
  Display *dpy = XOpenDisplay (NULL);

  if (dpy == NULL)
    {
      printf ("\n\tcannot connect to X server\n\n");
      exit (0);
    }

  Window root = DefaultRootWindow (dpy);

  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo *vi = glXChooseVisual (dpy, 0, att);

  if (vi == NULL)
    {
      printf ("\n\tno appropriate visual found\n\n");
      exit (0);
    }
  else
    {
      printf ("\n\tvisual %p selected\n", (void *) vi->visualid);
    }				/* %p creates hexadecimal output like in glxinfo */


  Colormap cmap = XCreateColormap (dpy, root, vi->visual, AllocNone);

  XSetWindowAttributes swa;
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask;

  Window win =
    XCreateWindow (dpy, root, 0, 0, 1900, 1000, 0, vi->depth, InputOutput,
		   vi->visual, CWColormap | CWEventMask, &swa);

  XMapWindow (dpy, win);
  XStoreName (dpy, win, "Minecraft World Viewer");

  GLXContext glc = glXCreateContext (dpy, vi, NULL, GL_TRUE);
  glXMakeCurrent (dpy, win, glc);

  glEnable (GL_DEPTH_TEST);
  while (1)
    {
      XEvent xev;
      XNextEvent (dpy, &xev);

      if (xev.type == Expose)
	{
	  XWindowAttributes gwa;
	  XGetWindowAttributes (dpy, win, &gwa);
	  glViewport (0, 0, gwa.width, gwa.height);
	  redraw ();
	  glXSwapBuffers (dpy, win);
	}

      else if (xev.type == KeyPress)
	{
	  glXMakeCurrent (dpy, None, NULL);
	  glXDestroyContext (dpy, glc);
	  XDestroyWindow (dpy, win);
	  XCloseDisplay (dpy);
	  exit (0);
	}
    }				/* this closes while(1) { */
}				/* this is the } which closes int main(int argc, char *argv[]) { */
