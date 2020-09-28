#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <GL/glut.h>
#include<GL/gl.h>
#include <math.h>

#define FPS 240
#define TO_RADIANS 3.14/180.0
#define scale 20

//width and height of the window ( Aspect ratio 16:9 )
const int width = 16*80;
const int height = 9*80;

//const int scale = 20;

float pitch = 0.0, yaw= 0.0;
float camX=500.0,camZ=255.0,camY=7.0;

void display();
void reshape(int w,int h);
void timer(int);
void passive_motion(int,int);
void camera();
void keyboard(unsigned char key,int x,int y);
void keyboard_up(unsigned char key,int x,int y);

GLuint texture[35];

struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
};


int ImageLoad(char *filename, Image *image) {
	FILE *file;
	unsigned long size; // ukuran image dalam bytes
	unsigned long i; // standard counter.
	unsigned short int plane; // number of planes in image

	unsigned short int bpp; // jumlah bits per pixel
	char temp; // temporary color storage for var warna sementara untuk memastikan filenya ada


	if ((file = fopen(filename, "rb")) == NULL) {
		printf("File Not Found : %s\n", filename);
		return 0;
	}
	// mencari file header bmp
	fseek(file, 18, SEEK_CUR);
	// read the width
	if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}
	//printf("Width of %s: %lu\n", filename, image->sizeX);
	// membaca nilai height
	if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}
	//printf("Height of %s: %lu\n", filename, image->sizeY);
	//menghitung ukuran image(asumsi 24 bits or 3 bytes per pixel).

	size = image->sizeX * image->sizeY * 3;
	// read the planes
	if ((fread(&plane, 2, 1, file)) != 1) {
		printf("Error reading planes from %s.\n", filename);
		return 0;
	}
	if (plane != 1) {
		printf("Planes from %s is not 1: %u\n", filename, plane);
		return 0;
	}
	// read the bitsperpixel
	if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s.\n", filename);

		return 0;
	}
	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
	}
	// seek past the rest of the bitmap header.
	fseek(file, 24, SEEK_CUR);
	// read the data.
	image->data = (char *) malloc(size);
	if (image->data == NULL) {
		printf("Error allocating memory for color-corrected image data");
		return 0;
	}
	if ((i = fread(image->data, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		return 0;
	}
	for (i = 0; i < size; i += 3) { // membalikan semuan nilai warna (gbr - > rgb)
		temp = image->data[i];
		image->data[i] = image->data[i + 2];
		image->data[i + 2] = temp;
	}
	// we're done.
	return 1;
}

Image * loadTexture(char* filename) {
	Image *imageLoad;
	
	// alokasi memmory untuk tekstur
	imageLoad = (Image *) malloc(sizeof(Image));
	if (imageLoad == NULL) {
		printf("Error allocating space for image");
		exit(0);
	}
	if (!ImageLoad(filename, imageLoad)) {
		exit(1);
	}
	return imageLoad;
}

struct Motion
{
    bool Forward,Backward,Left,Right,up,down;
};

Motion motion = {false,false,false,false,false,false};

void init()
{
    glutSetCursor(GLUT_CURSOR_NONE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glutWarpPointer(width/2,height/2);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 16/9, 0.1f, 9000.0f);
    glMatrixMode(GL_MODELVIEW);
    
    Image *image1 = loadTexture("devils_advocate_bk.bmp");//skybox back
    Image *image2 = loadTexture("devils_advocate_dn.bmp");//skybox down
	Image *image3 = loadTexture("devils_advocate_ft.bmp");//skybox front
	Image *image4 = loadTexture("devils_advocate_lf.bmp");//skybox left
	Image *image5 = loadTexture("devils_advocate_rt.bmp");//skybox right
	Image *image6 = loadTexture("devils_advocate_up.bmp");//skybox up
	Image *image7 = loadTexture("stonetile1.bmp");//tembok
	Image *image8 = loadTexture("arroway.de_plaster-11_b030.bmp");//jalan lorong
	Image *image9 = loadTexture("ahorn2.bmp");//atap
	Image *image10 = loadTexture("arroway.de_tiles-48_d100.bmp");//lantai
	Image *image11 = loadTexture("hardwood-material-rough-935875.bmp");//tembok kayu
	Image *image12 = loadTexture("wood-texture-skin-door-500x500.bmp");//pintu 2
	Image *image13 = loadTexture("wood-texture-door-skin-500x500.bmp");//pintu 1
	Image *image14 = loadTexture("pintudepan.bmp");//pintu depan
	Image *image15 = loadTexture("nako.bmp");//jendela
	Image *image16 = loadTexture("jendelagede.bmp");//kaca besar
	Image *image17 = loadTexture("dimerahin.bmp");//jalan lorong
	
    
    glGenTextures(35,texture);
	//--------------------------------0--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	//menyesuaikan ukuran textur ketika image lebih besar dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image1->data);
	
	//--------------------------------1--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image2->sizeX, image2->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image2->data);
	
	//--------------------------------2--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image3->sizeX, image3->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image3->data);
	
	//--------------------------------3--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image4->sizeX, image4->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image4->data);
	
	
	//--------------------------------4--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image5->sizeX, image5->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image5->data);
	
	
	//--------------------------------5--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image6->sizeX, image6->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image6->data);
	
	//--------------------------------6--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image7->sizeX, image7->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image7->data);
	
	//--------------------------------7--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[7]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image8->sizeX, image8->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image8->data);
	
	//--------------------------------8--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[8]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image9->sizeX, image9->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image9->data);
	
	//--------------------------------9--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image10->sizeX, image10->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image10->data);
	
	//--------------------------------10--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[10]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image11->sizeX, image11->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image11->data);
	
	//--------------------------------11--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[11]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image12->sizeX, image12->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image12->data);
	
	//--------------------------------12--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[12]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image13->sizeX, image13->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image13->data);
	
	//--------------------------------13--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[13]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image14->sizeX, image14->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image14->data);
	
	//--------------------------------14--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[14]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image15->sizeX, image15->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image15->data);
	//--------------------------------15--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[15]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image16->sizeX, image16->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image16->data);
	//--------------------------------16--------------------------------
	glBindTexture(GL_TEXTURE_2D, texture[16]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//menyesuaikan ukuran textur ketika image lebih kecil dari texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image17->sizeX, image17->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, image17->data);
}


/* This function just draws the scene. I used Texture mapping to draw
   a chessboard like surface. If this is too complicated for you ,
   you can just use a simple quadrilateral */

void kotak(float x1,float y1,float z1,float x2,float y2,float z2){
	//depan
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x1,y1,z1);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x2,y1,z1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x2,y2,z1);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x1,y2,z1);
	//atas
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x1,y2,z1);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x2,y2,z1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x2,y2,z2);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x1,y2,z2);
	//belakang
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x1,y2,z2);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x2,y2,z2);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x2,y1,z2);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x1,y1,z2);
	//bawah
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x1,y1,z2);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x2,y1,z2);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x2,y1,z1);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x1,y1,z1);
	//samping kiri
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x1,y1,z1);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x1,y2,z1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x1,y2,z2);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x1,y1,z2);
	//samping kanan
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x2,y1,z1);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x2,y2,z1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x2,y2,z2);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x2,y1,z2);
}

void drawSilinder(GLfloat radius, GLfloat height, GLfloat posx) {
    GLfloat x              = 0.0;
    GLfloat z              = 0.0;
    GLfloat angle          = 0.0;
    GLfloat angle_stepsize = 0.1;
    
    GLubyte R=255, G=160, B=160;
    
    glTranslatef(posx, -2, 0);
    /** Draw the tube */
    //glColor3ub(R-40,G-40,B-40);
    glBegin(GL_QUAD_STRIP);
    angle = 0.0;
        while( angle < 2*M_PI ) {
            x = radius * cos(angle);
            z = radius * sin(angle);
            glVertex3f(x, height , z);
            glVertex3f(x, 0.0 , z);
            angle = angle + angle_stepsize;
        }
        glVertex3f(radius, height, 0.0);
        glVertex3f(radius, 0.0, 0.0);
    glEnd();
    glTranslatef(-posx, 2, 0);
}

void skyBox(float x1,float y1,float z1,float x2,float y2,float z2){
	glPushMatrix();
		glColor3f(1,1,1);
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glBegin(GL_QUADS);
				//depan
				glTexCoord2f(0.0, 0.0);
				glVertex3f(x1,y1,z1);
				glTexCoord2f(0.0, 1.0);
				glVertex3f(x2,y1,z1);
				glTexCoord2f(1.0, 1.0);
				glVertex3f(x2,y2,z1);
				glTexCoord2f(1.0, 0.0);
				glVertex3f(x1,y2,z1);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glColor3f(1,1,1);
		glBindTexture(GL_TEXTURE_2D, texture[5]);
		glBegin(GL_QUADS);
					//atas
					glTexCoord2f(0.0, 0.0);
					glVertex3f(x1,y2,z1);
					glTexCoord2f(0.0, 1.0);
					glVertex3f(x2,y2,z1);
					glTexCoord2f(1.0, 1.0);
					glVertex3f(x2,y2,z2);
					glTexCoord2f(1.0, 0.0);
					glVertex3f(x1,y2,z2);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glColor3f(1,1,1);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glBegin(GL_QUADS);
					//belakang
					glTexCoord2f(0.0, 0.0);
					glVertex3f(x1,y2,z2);
					glTexCoord2f(0.0, 1.0);
					glVertex3f(x2,y2,z2);
					glTexCoord2f(1.0, 1.0);
					glVertex3f(x2,y1,z2);
					glTexCoord2f(1.0, 0.0);
					glVertex3f(x1,y1,z2);
		glEnd();
	glPopMatrix();
	
	glPushMatrix();
		glColor3f(1,1,1);
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glBegin(GL_QUADS);
					//bawah
					glTexCoord2f(0.0, 0.0);
					glVertex3f(x1,y1,z2);
					glTexCoord2f(1.0, 0.0);
					glVertex3f(x2,y1,z2);
					glTexCoord2f(1.0, 1.0);
					glVertex3f(x2,y1,z1);
					glTexCoord2f(0.0, 1.0);
					glVertex3f(x1,y1,z1);
		glEnd();
	glPopMatrix();
	
		glPushMatrix();
		glColor3f(1,1,1);
		glBindTexture(GL_TEXTURE_2D, texture[3]);
		glBegin(GL_QUADS);
					//samping kiri
					glTexCoord2f(0.0, 0.0);
					glVertex3f(x1,y1,z1);
					glTexCoord2f(1.0, 0.0);
					glVertex3f(x1,y2,z1);
					glTexCoord2f(1.0, 1.0);
					glVertex3f(x1,y2,z2);
					glTexCoord2f(0.0, 1.0);
					glVertex3f(x1,y1,z2);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glColor3f(1,1,1);
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glBegin(GL_QUADS);
					//samping kanan
					glTexCoord2f(0.0, 0.0);
					glVertex3f(x2,y1,z1);
					glTexCoord2f(1.0, 0.0);
					glVertex3f(x2,y2,z1);
					glTexCoord2f(1.0, 1.0);
					glVertex3f(x2,y2,z2);
					glTexCoord2f(0.0, 1.0);
					glVertex3f(x2,y1,z2);
		glEnd();
	glPopMatrix();
}


 void draw()
{
    glEnable(GL_TEXTURE_2D);
	skyBox(-100,-9,-100,700,300,500);//skybox
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texture[6]);//warna tembok
	
		glBegin(GL_QUADS);

		//pilar
		for(int i = 1; i <= 27; i++){
			if (i == 20 || i == 8){
				kotak (20+i*scale, 3*scale, 1*scale, 15+i*scale, -10, 1*scale-10);//tiang pintu depan
				kotak (20+i*scale, 3*scale, 6*scale, 15+i*scale, -10, 6*scale+10);//tiang pintu belakang
			}	else {
				kotak (i*scale, 3*scale, 1*scale, i*scale-5, -10, 1*scale-10);//tiang belakang
				kotak (i*scale, 3*scale, 6*scale, i*scale-5, -10, 6*scale+10);//tiang depan
			}
			if(8 < i && i < 20){
					kotak (i*scale, 1.5*scale, 2.8*scale-3, i*scale-5, -10, 2.8*scale);//tiang dalem depan
					kotak (i*scale, 1.5*scale, 4.2*scale-3, i*scale-5, -10, 4.2*scale);//tiang dalem belakang	
			}
			if((1 < i && i < 6)||(i > 23)){
				kotak (i*scale, 1.5*scale, 3.5*scale-3, i*scale-5, -10, 3.5*scale);//tiang tengah lab
			}
		}
		
		kotak(1*scale, 10-scale, 1*scale-10, 27*scale, 15-scale, 6*scale+10);//lantai
		kotak(1*scale, 1.5*scale, 1*scale-20, 27*scale, 1.5*scale-5, 6*scale+20);//lantai 2
		kotak(1*scale-9, 3*scale, 1*scale-19, 27*scale+9, 3*scale-1, 6*scale+19);//atap
		
		kotak(1*scale-4, 3*scale, 1*scale, 1*scale-2, -10, 6*scale);//tembok kiri
		kotak(27*scale-4, 3*scale, 1*scale, 27*scale, -10, 6*scale);//tembok kanan
		kotak(1*scale-5, 3*scale, 3+6*scale, 27*scale, -10, 5+6*scale);//tembok depan
		kotak(1*scale-4, 3*scale, 1*scale-4, 27*scale, -10, 1*scale-6);//tembok belakang
		
		kotak(5.45*scale-4, 3*scale, 2.8*scale-1, 19*scale, 1.5*scale, 2.8*scale+1);//tembok dalem belakang atas
		kotak(7*scale-4, 3*scale, 4.2*scale-4, 19*scale, 1.5*scale, 4.2*scale-2);//tembok dalem depan atas
		kotak(9*scale-4, 1.5*scale-1, 2.8*scale+1, 19*scale, -10, 2.8*scale);//tembok dalem belakang bawah
		kotak(9*scale-4, 1.5*scale-1, 4.2*scale-4, 19*scale, -10, 4.2*scale-3);//tembok dalem depan bawah
		
		kotak(5*scale-4, 1.5*scale-1, 1*scale, 5*scale-2, -10, 6*scale);//tembok lab kiri
		kotak(23*scale-4, 1.5*scale, 1*scale, 23*scale-2, -10, 6*scale);//tembok lab kanan 
		kotak(1*scale-4, 1.5*scale, 3.5*scale-1, 5*scale, -10, 3.5*scale-2);//tengahnya tembok lab kiri
		kotak(23*scale-4, 1.5*scale, 3.5*scale-1, 27*scale, -10, 3.5*scale-2);//tengahnya tembok lab kanan
		kotak(21*scale-4, 1.5*scale, 1*scale, 21*scale-2, -10, 2.5*scale);//tembok mushalla
		kotak(21*scale-4, 1.5*scale, 2.5*scale-2, 23*scale-2, -10, 2.5*scale);//tembok mushalla
		kotak(21*scale-4, 1.5*scale, 6*scale, 21*scale-2, -10, 4.8*scale);//tembok ruang pearson
		kotak(21*scale-4, 1.5*scale, 4.8*scale-2, 23*scale-2, -10, 4.8*scale);//tembok ruang pearson

		kotak(18*scale-4, 3*scale, 1*scale, 18*scale-2, -10, 2.8*scale);//tembok pembatas wc sama kelas
		kotak(19*scale, 3*scale, 1*scale, 19*scale-2, -10, 1.7*scale);//tembok wc 
		kotak(19*scale, 3*scale, 2.1*scale, 19*scale-2, -10, 2.8*scale);//tembok wc
		kotak(19*scale, 3*scale, 2.1*scale+2, 18*scale-2, -10, 2.1*scale);//tembok depan wc cowo bawah
		kotak(19*scale, 3*scale, 1.6*scale+2, 18*scale-2, -10, 1.6*scale);//tembok depan wc cewe bawah
		kotak(15*scale-4, 1.5*scale, 1*scale, 15*scale-2, -10, 2.8*scale);//tembok kelas bawah
		kotak(12*scale-4, 1.5*scale, 1*scale, 12*scale-2, -10, 2.8*scale);//tembok lab yang bagus
		kotak(11*scale-4, 1.5*scale, 1*scale, 11*scale-2, -10, 2.8*scale);//tembok lab bagus ke ruang proyektor
		kotak(15*scale-4, 1.5*scale, 6*scale, 15*scale-2, -10, 4.1*scale);//lab database
		kotak(12*scale-4, 1.5*scale, 6*scale, 12*scale-2, -10, 4.1*scale);//tembok lab db ke kelas
		kotak(11*scale-4, 1.5*scale, 6*scale, 11*scale-2, -10, 4.1*scale);//tembok ruang kopi ke kelas		
		
		kotak(19*scale, 3.5*scale, 4*scale+0.1, 19*scale-1.9, -10, 6*scale);//tembok lab database
		kotak(7*scale, 1.5*scale, 4.5*scale, 7*scale-2, -10, 6*scale);//tembok ke pintu kiri bawah
		kotak(9*scale-6, 1.5*scale, 4*scale, 9*scale-4, -10, 6*scale);//tembok ke pintu kiri bawah
		kotak(7*scale, 1.5*scale, 4.5*scale-2, 4*scale, -10, 4.5*scale);//ruang depan lab ICT
		kotak(9*scale-6, 1.5*scale, 1*scale, 9*scale-4, -10, 2.8*scale+1);//tembok kanan gudang bawah
		kotak(9*scale-4, 1.5*scale-1, 2.2*scale+2, 4.9*scale, -10, 2.2*scale);//tembok depan gudang & ruang dosen kecil bawah
		kotak(6*scale-2, 1.5*scale-1, 1*scale, 6*scale, -10, 2.2*scale);//tembok pemisah ruang dosen kecil bawah & gudang
		
		kotak(18*scale-4, 3*scale, 4*scale+1, 18*scale-2, 1.5*scale, 6*scale);//tembok pembatas elib-kelas
		kotak(15*scale-4, 3*scale, 4*scale+1, 15*scale-2, 1.5*scale, 6*scale);//tembok pembatas kelas
		kotak(12*scale-4, 3*scale, 4*scale+1, 12*scale-2, 1.5*scale, 6*scale);//tembok ruang management
		kotak(11*scale-4, 3*scale, 4*scale+1, 11*scale-2, 1.5*scale, 6*scale);//tembok ruang server
		kotak(9*scale-4, 3*scale, 4*scale+1, 9*scale-2, 1.5*scale, 6*scale);//tembok ruang rapat
		kotak(7*scale-4, 3*scale, 4*scale+1, 7*scale-2, 1.5*scale, 6*scale);//tembok ruang multimedia
		
		//sekat ruang lantai 2
		kotak(18*scale-4, 3*scale, 2.8*scale, 18*scale-2, 1.5*scale, 1*scale);//tembok pembatas xc kelas
		kotak(15*scale-4, 3*scale, 2.8*scale, 15*scale-2, 1.5*scale, 1*scale);//tembok pembatas kelas
		kotak(12*scale-4, 3*scale, 2.8*scale, 12*scale-2, 1.5*scale, 1*scale);//tembok ruang lab uppl
		kotak(11*scale-4, 3*scale, 2.8*scale, 11*scale-2, 1.5*scale, 1*scale);//tembok ruang gudang
		kotak(9*scale-4, 3*scale, 2.8*scale, 9*scale-2, 1.5*scale, 1*scale);//tembok ruang sekretariat
		kotak(7*scale-4, 3*scale, 2.8*scale, 7*scale-2, 1.5*scale, 1*scale);//tembok ruang tamu
		
		kotak(7*scale-4, 3*scale, 4*scale+4, 7*scale-2, 1.5*scale, 6*scale);//tembok deket pintu atas
		kotak(21*scale, 3*scale, 4.2*scale-4, 27*scale, 1.5*scale, 4.2*scale-2);//tembok RSG
		kotak(21*scale, 3*scale, 2.8*scale-4, 27*scale, 1.5*scale, 2.8*scale-2);//tembok kelas atas seberang RSG
		kotak(21*scale-2, 3*scale, 2.8*scale-2, 21*scale, 1.5*scale, 1*scale-2);//tembok yang ada mading atas
		kotak(24*scale-2, 3*scale, 2.8*scale-4, 24*scale, 1.5*scale, 1*scale-2);//tembok kelas kanan atas
		kotak(21*scale, 3*scale, 5.3*scale-2, 21*scale-2, 1.5*scale, 4.2*scale-4);//tembok seberang elib
		kotak(21*scale, 3*scale, 5.3*scale-4, 22.6*scale, 1.5*scale, 5.3*scale-2);//tembok abis pintu atas
		kotak(22.6*scale-1, 3*scale, 4.2*scale-4, 22.6*scale-3, 1.5*scale, 6.2*scale-2);//tembok depan RSG

		
		
		//tiang lorong
		for(int countLorong = 0; countLorong < 19; countLorong++){
			if(countLorong < 2 || countLorong == 10 || countLorong == 11){
				for(int countKedalem = 16; countKedalem < 21; countKedalem++){
					kotak((countLorong+6.3)*(scale*1.15)+6, 0.75*scale, (countKedalem-8)*scale+3, (countLorong+6.3)*(scale*1.15)+3, -10, (countKedalem-8)*scale+6);	
				}
				kotak((countLorong+6.3)*(scale*1.15)+6, 0.75*scale, 13.2*scale+3, (countLorong+6.3)*(scale*1.15)+3, -10, 13.2*scale+6);
			} else {
				kotak((countLorong+6.3)*(scale*1.15)+6, 0.75*scale, 12*scale+3, (countLorong+6.3)*(scale*1.15)+3, -10, 12*scale+6);//tiang dalem
				kotak((countLorong+6.3)*(scale*1.15)+6, 0.75*scale, 13.2*scale+3, (countLorong+6.3)*(scale*1.15)+3, -10, 13.2*scale+6);//tiang luar	
			}
		}
		
		//atap lorong
		kotak(18.7*scale+2, 0.75*scale+1, 6*scale+7, 21*scale-3, 0.75*scale, 12*scale+2);//atap lorong kanan
		kotak(7*scale+2, 0.75*scale+1, 6*scale+7, 9*scale-3, 0.75*scale, 12*scale+2);//atap lorong kiri
		kotak(7*scale+2, 0.75*scale+1, 12*scale+2, 28.5*scale-3, 0.75*scale, 13.5*scale+2);//atap lorong panjang
		
		//tangga kanan
		kotak(21*scale-3, 3*scale, 8.5*scale+5, 21*scale-5, -10, 8*scale);//TEMBOK KIRINYA
		kotak(22.5*scale, 3*scale, 8.5*scale+5, 22.5*scale+2, -10, 8*scale);//tembok kanannya
		kotak(20.8*scale-1, 3*scale, 8.5*scale+5, 22.5*scale+2, 3*scale+2, 7*scale);//atapnya
		kotak(22.5*scale+2, 1.5*scale+1, 6*scale+7, 21*scale-3, 1.5*scale-6, 8*scale+2);//lantai dari tangga ke pintu
		kotak(21*scale-3, 2*scale, 8*scale, 21*scale-5, 2*scale-15, 6*scale);//pager tembok kiri
		kotak(22.5*scale, 2*scale, 8*scale, 22.5*scale+1, 2*scale-15, 6*scale);//pager tembok kanan
		kotak(20.8*scale, 1.25*scale/2-3, 11.5*scale, 22.5*scale, 1.25*scale/2, 10.5*scale-2);//lantai tengah tengah tangganya
		kotak(21*scale-3, 1.25*scale/2-3, 11.5*scale, 22.5*scale, 2.25*scale/2, 11.4*scale);//pager biar ga jatoh
		kotak(21*scale-3, 2.1*scale, 11.5*scale, 21*scale-5, -10, 10.3*scale);//TEMBOK KIRINYA yang turunnya
		kotak(22.5*scale, 2.1*scale, 11.5*scale, 22.5*scale+2, -10, 10.3*scale);//tembok kanannya yang turunnya
		kotak(20.8*scale-1, 2.1*scale, 11.5*scale+5, 22.5*scale+2, 2.1*scale+2, 10.3*scale);//atapnya yang turunnya
		
		//atap miringnya
		for(int countAtapTurun = 5; countAtapTurun < 36; countAtapTurun++){
			//kanan
			kotak(21*scale-3, 3*scale-countAtapTurun/2, 8.5*scale+1+countAtapTurun, 21*scale-5, -10, 8.5*scale+countAtapTurun);//TEMBOK KIRINYA
			kotak(22.5*scale, 3*scale-countAtapTurun/2, 8.5*scale+1+countAtapTurun, 22.5*scale+2, -10, 8.5*scale+countAtapTurun);//tembok kanannya
			kotak(20.8*scale-1, 3*scale-countAtapTurun/2, 8.5*scale+1+countAtapTurun, 22.5*scale+2, 3*scale+2-countAtapTurun/2, 8.5*scale+countAtapTurun);//atapnya
			
			//kiri
			kotak(5.5*scale-3, 3*scale-countAtapTurun/2, 8.5*scale+1+countAtapTurun, 5.5*scale, -10, 8.5*scale+countAtapTurun);//TEMBOK KIRINYA
			kotak(7*scale, 3*scale-countAtapTurun/2, 8.5*scale+1+countAtapTurun, 7*scale+2, -10, 8.5*scale+countAtapTurun);//tembok kanannya
			kotak(5.5*scale-1, 3*scale-countAtapTurun/2, 8.5*scale+1+countAtapTurun, 7*scale+2, 3*scale+1-countAtapTurun/2, 8.5*scale+countAtapTurun);//atapnya
		}
		
		//anak tangga
		for(int countTangga = 0; countTangga < 9; countTangga++){
			//anak tangga kanan
			kotak(22.5*scale, (1.5*scale)-countTangga*2, (8*scale+8)+countTangga*5, 21.65*scale+1, (1.5*scale-2)-countTangga*2, (8*scale+2)+countTangga*5);
			kotak(21*scale, (0.5*scale)-countTangga*2, (10*scale+8)-countTangga*5, 21.65*scale+1, (0.5*scale-2)-countTangga*2, (10*scale+2)-countTangga*5);
			//anak tangga kiri
			kotak(5.5*scale, (1.5*scale)-countTangga*2, (8*scale+8)+countTangga*5, 6.25*scale-1, (1.5*scale-2)-countTangga*2, (8*scale+2)+countTangga*5);
			kotak(6.25*scale, (0.5*scale)-countTangga*2, (10*scale+8)-countTangga*5, 7*scale+1, (0.5*scale-2)-countTangga*2, (10*scale+2)-countTangga*5);	
		}

		//tangga kiri
		kotak(5.5*scale-3, 3*scale, 8.5*scale+5, 5.5*scale, -10, 8*scale);//TEMBOK KIRINYA
		kotak(7*scale, 3*scale, 8.5*scale+5, 7*scale+2, -10, 8*scale);//tembok kanannya
		kotak(5.5*scale-3, 3*scale, 8.5*scale+5, 7*scale+2, 3*scale+2, 7*scale);//atapnya
		kotak(7*scale+2, 1.5*scale+1, 6*scale+7, 5.5*scale-3, 1.5*scale-6, 8*scale+2);//lantai dari tangga ke pintu
		kotak(5.5*scale-3, 2*scale, 8*scale, 5.5*scale-5, 2*scale-15, 6*scale);//pager tembok kiri
		kotak(7*scale, 2*scale, 8*scale, 7*scale+1, 2*scale-15, 6*scale);//pager tembok kanan
		kotak(5.5*scale, 1.25*scale/2-3, 11.5*scale, 7*scale, 1.25*scale/2, 10.5*scale-2);//lantai tengah tengah tangganya
		kotak(5.5*scale-3, 1.25*scale/2-3, 11.5*scale, 7*scale, 2.25*scale/2, 11.4*scale);//pager biar ga jatoh
		kotak(5.5*scale-3, 2.1*scale, 11.5*scale, 5.5*scale, -10, 10.3*scale);//TEMBOK KIRINYA yang turunnya
		kotak(7*scale, 2.1*scale, 11.5*scale, 7*scale+2, -10, 10.3*scale);//tembok kanannya yang turunnya
		kotak(5.5*scale-3, 2.1*scale, 11.5*scale+5, 7*scale+2, 2.1*scale+2, 10.3*scale);//atapnya yang turunnya
		//draw_cylinder(1*scale, 1.5*scale);
		glEnd();
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[7]); //tekstur atap hitam
	glBegin(GL_QUADS);
				//atap lorong
		kotak(18.7*scale+3, 0.75*scale+2, 6*scale+7, 21*scale-4, 0.75*scale+1, 12*scale+1);//atap lorong kanan
		kotak(7*scale+3, 0.75*scale+2, 6*scale+7, 9*scale-3, 0.75*scale+1, 12*scale+1);//atap lorong kiri
		kotak(7*scale+3, 0.75*scale+2, 12*scale+2, 28.5*scale-3, 0.75*scale+1, 13.5*scale+1);//atap lorong panjang
		
		kotak(5.5*scale-4, 3*scale+1, 8.5*scale+6, 7*scale+3, 3*scale+3, 7*scale-1);//atapnya tangga kiri
		kotak(20.8*scale-2, 3*scale+1, 8.5*scale+6, 22.5*scale+3, 3*scale+3, 7*scale-1);//atapnya tangga kanan
		kotak(5.5*scale-4, 2.1*scale+1, 11.5*scale+6, 7*scale+3, 2.1*scale+3, 10.3*scale-1);//atapnya yang turunnya tangga kiri
		kotak(20.8*scale-2, 2.1*scale+1, 11.5*scale+6, 22.5*scale+3, 2.1*scale+3, 10.3*scale-1);//atapnya yang turunnya tangga kanan
		
		//atap miringnya
		for(int countAtapTurun = 5; countAtapTurun < 36; countAtapTurun++){
			//kanan
			kotak(20.8*scale-2, 3*scale+3-countAtapTurun/2, 8.5*scale+2+countAtapTurun, 22.5*scale+2, 3*scale+2-countAtapTurun/2, 8.5*scale+countAtapTurun);//atapnya
			
			//kiri
			kotak(5.5*scale-2, 3*scale+3-countAtapTurun/2, 8.5*scale+2+countAtapTurun, 7*scale+2, 3*scale+2-countAtapTurun/2, 8.5*scale+countAtapTurun);//atapnya
		}
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[8]);//tekstur atap
	glBegin(GL_QUADS);
		for(int countAtap = 0; countAtap < 48; countAtap++){//bikin atap bro
			if (countAtap > 35){
				kotak(1*scale+60-countAtap, 3*scale+countAtap, 1*scale-20+countAtap*1.5, 27*scale-60+countAtap, 3*scale+1+countAtap, 6*scale+20-countAtap*1.5);//atap
			} else {
				kotak(1*scale-10+countAtap, 3*scale+countAtap, 1*scale-20+countAtap*1.5, 27*scale+10-countAtap, 3*scale+1+countAtap, 6*scale+20-countAtap*1.5);//atap		
			}
		} 
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[9]); //tekstur lantai
	glBegin(GL_QUADS);
		kotak(1*scale+1, 10-scale+1, 1*scale-9, 27*scale-1, 15-scale+1, 6*scale+9);//lantai
		kotak(1*scale+0.1, 1.5*scale+1, 1*scale-19, 27*scale-0.1, 1.5*scale-4, 6*scale+19);//lantai 2
		kotak(22.5*scale+2, 1.5*scale+2, 6*scale+7, 21*scale-3, 1.5*scale-1, 8*scale+1);//lantai dari tangga ke pintu kanan
		kotak(7*scale+2, 1.5*scale+2, 6*scale+6, 5.5*scale-3, 1.5*scale-1, 8*scale+1);//lantai dari tangga ke pintu kiri
		//anak tangga
		for(int countTangga = 0; countTangga < 9; countTangga++){
			//anak tangga kanan
			kotak(22.5*scale-2, (1.5*scale+1)-countTangga*2, (8*scale+9)+countTangga*5, 21.65*scale+2, (1.5*scale-1)-countTangga*2, (8*scale+2)+countTangga*5);
			kotak(21*scale-2, (0.5*scale+1)-countTangga*2, (10*scale+9)-countTangga*5, 21.65*scale+2, (0.5*scale-1)-countTangga*2, (10*scale+2)-countTangga*5);
			//anak tangga kiri
			kotak(5.5*scale-2, (1.5*scale+1)-countTangga*2, (8*scale+9)+countTangga*5, 6.25*scale, (1.5*scale-1)-countTangga*2, (8*scale+2)+countTangga*5);
			kotak(6.25*scale-1, (0.5*scale+1)-countTangga*2, (10*scale+9)-countTangga*5, 7*scale+1, (0.5*scale-1)-countTangga*2, (10*scale+2)-countTangga*5);	
		}
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[10]); //tembok kayu
	glBegin(GL_QUADS);
		kotak(5.45*scale-4, 3*scale, 1*scale, 5.45*scale-2, 1.5*scale, 6*scale);//tembok ruang dosen
		kotak(26*scale, 2.7*scale, 4.2*scale-4, 26*scale-2, 1.5*scale, 2.8*scale-2);//mushalla atas
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[11]); //pintu 2 kayu
	glBegin(GL_QUADS);
		//lantai 2
		kotak(25*scale, 2.5*scale, 2.8*scale-4.1, 25.7*scale, 1.5*scale, 2.8*scale-1.9);//kelas ujung kanan atas
		kotak(22.5*scale, 2.5*scale, 2.8*scale-4.1, 23.2*scale, 1.5*scale, 2.8*scale-1.9);//sebelahnya kelas ujung kanan atas
		kotak(25*scale, 2.5*scale, 4.2*scale-4.1, 25.7*scale, 1.5*scale, 4.2*scale-1.9);//pintu RSG depan
		kotak(23*scale, 2.5*scale, 4.2*scale-4.1, 23.7*scale, 1.5*scale, 4.2*scale-1.9);//pintu RSG belakang
		kotak(17*scale, 2.5*scale, 2.8*scale+1.1, 17.7*scale, 1.5*scale, 2.8*scale-1.1);//ruang matkul OS teori
		kotak(14*scale, 2.5*scale, 2.8*scale+1.1, 14.7*scale, 1.5*scale, 2.8*scale-1.1);//ruang matkul OS teori sebelahnya
		kotak(15*scale, 2.5*scale, 4*scale+2.1, 15.7*scale, 1.5*scale, 4*scale-0.1);//seberangnya
		kotak(12*scale, 2.5*scale, 4*scale+2.1, 12.7*scale, 1.5*scale, 4*scale-0.1);//ruang management
		kotak(11*scale, 2.5*scale, 4*scale+2.1, 11.7*scale, 1.5*scale, 4*scale-0.1);//ruang server
		kotak(11*scale, 2.5*scale, 2.8*scale+1.1, 11.7*scale, 1.5*scale, 2.8*scale-1.1);//seberang ruang server
		kotak(9*scale, 2.5*scale, 4*scale+2.1, 9.7*scale, 1.5*scale, 4*scale-0.1);//ruang rapat
		kotak(10*scale, 2.5*scale, 2.8*scale+1.1, 10.7*scale, 1.5*scale, 2.8*scale-1.1);//sekretariat
		kotak(5.45*scale-4.1, 2.5*scale, 3*scale, 5.45*scale-1.9, 1.5*scale, 3.7*scale);//ruang dosen
		
		//lantai 1
		kotak(23*scale-1.9, 0.9*scale, 2.7*scale, 23*scale-4.1, -10, 3.2*scale);//pintu lab baru
		kotak(23*scale-1.9, 0.9*scale, 3.7*scale, 23*scale-4.1, -10, 4.2*scale);//pintu lab proyek
		kotak(22.2*scale, 0.9*scale, 4.8*scale+0.1, 22.7*scale, -10, 4.8*scale-2.1);//pintu RSG belakang
		kotak(22.2*scale, 0.9*scale, 2.6*scale-1.9, 22.7*scale, -10, 2.6*scale-4.1);//pintu mosholla
		kotak(17.2*scale-4, 0.9*scale, 2.8*scale+1.1, 17.7*scale, -10, 2.8*scale-0.1);//pintu kelas sebelah wc
		kotak(14.2*scale-4, 0.9*scale, 2.8*scale+1.1, 14.7*scale, -10, 2.8*scale-0.1);//pintu lab bagus
		kotak(11.2*scale-4, 0.9*scale, 2.8*scale+1.1, 11.7*scale, -10, 2.8*scale-0.1);//pintu ruang proyektor
		kotak(10.2*scale-4, 0.9*scale, 2.8*scale+1.1, 10.7*scale, -10, 2.8*scale-0.1);//pintu sebelah ruang proyektor
		kotak(15.2*scale-4, 0.9*scale, 4*scale+1.1, 15.7*scale, -10, 4*scale-0.1);//pintu lab database
		kotak(12.2*scale-4, 0.9*scale, 4*scale+1.1, 12.7*scale, -10, 4*scale-0.1);//pintu kelas sebelah lab database
		kotak(11.2*scale-4, 0.9*scale, 4*scale+1.1, 11.7*scale, -10, 4*scale-0.1);//pintu ruang kopi
		kotak(9.2*scale-4, 0.9*scale, 4*scale+1.1, 9.7*scale, -10, 4*scale-0.1);//pintu kelas sebelah ruang kopi
		kotak(5*scale-1.9, 0.9*scale, 2.7*scale, 5*scale-4.1, -10, 3.2*scale);//pintu lab ICT
		kotak(5*scale-1.9, 0.9*scale, 3.7*scale, 5*scale-4.1, -10, 4.2*scale);//pintu lab ICT
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[12]); //pintu 1 kayu
	glBegin(GL_QUADS);
	//lantai 2
		kotak(8*scale, 2.6*scale, 4*scale+2.1, 8.45*scale, 1.5*scale, 4*scale-0.1);//ruang pembunuhan
		kotak(8*scale, 2.6*scale, 2.8*scale+1.1, 8.45*scale, 1.5*scale, 2.8*scale-1.1);//ruang tamu cenah
		kotak(6*scale, 2.6*scale, 2.8*scale+1.1, 6.45*scale, 1.5*scale, 2.8*scale-1.1);//ruang pantry
		kotak(26*scale+0.1, 2.6*scale, 4.2*scale-4, 26*scale-2.1, 1.5*scale, 3.65*scale-2);//mushalla
		kotak(19*scale+0.1, 2.6*scale, 5.2*scale-4, 19*scale-2.1, 1.5*scale, 4.65*scale-2);//elib
		kotak(21*scale+0.1, 2.6*scale, 4.8*scale-4, 21*scale-2.1, 1.5*scale, 4.25*scale-2);//seberang elib ada pintu
		kotak(18*scale, 2.6*scale, 2.1*scale+2.1, 18.45*scale, 1.5*scale, 2.1*scale-0.1);//wc cowo atas
		kotak(18*scale, 2.6*scale, 1.7*scale+0.1, 18.45*scale, 1.5*scale, 1.7*scale-2.1);//wc cewe atas
				
		//lantai 1
		kotak(21*scale-1.9, 0.9*scale, 5*scale-4, 21*scale-4.1, -5, 5.45*scale-2);//ruang listrik
		kotak(8.2*scale-4, 0.9*scale-1, 2.2*scale+2.1, 8.55*scale, -5, 2.2*scale-0.1);//gudang 
		kotak(5.2*scale-4, 0.9*scale-1, 4.4*scale+2.1, 5.55*scale, -5, 4.4*scale-0.1);//gudang lagi
		kotak(5.2*scale-4, 0.9*scale-1, 2.2*scale+2.1, 5.55*scale, -5, 2.2*scale-0.1);//ruang dosen kecil
		kotak(5*scale-1.9, 0.9*scale, 1.1*scale, 5*scale-4.1, -5, 1.55*scale);//ruang dosen ke lab
		kotak(18*scale, 0.9*scale, 2.1*scale+2.1, 18.45*scale, -5, 2.1*scale-0.1);//wc cowo bawah
		kotak(18*scale, 0.9*scale, 1.7*scale+0.1, 18.45*scale, -5, 1.7*scale-2.1);//wc cewe bawah
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[13]); //pintu depan
	glBegin(GL_QUADS);
		kotak(19*scale, 1*scale, 6*scale+6, 21*scale-5, -10, 6*scale-1);//pintudepan kanan bawah
		kotak(7*scale, 1*scale, 6*scale+6, 9*scale-5, -10, 6*scale-1);//pintudepan kiri bawah
		kotak(19*scale, 1*scale, 1*scale-7, 21*scale-5, -10, 1*scale-3);//pintubelakang bawah
		kotak(5.5*scale-3, 3*scale, 6*scale+10.1, 7*scale-5, 1.5*scale, 6*scale-1);//pintudepan kiri atas
		kotak(21*scale-3, 3*scale, 6*scale+10.1, 22.7*scale-5, 1.5*scale, 6*scale-1);//pintudepan kanan atas
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[14]); //jendela
	glBegin(GL_QUADS);
		// jendela bawah
		for(int countJendela = 0; countJendela <= 25; countJendela++){
			if (countJendela == 6 || countJendela == 7){
				//atas depan
				kotak(1*scale+countJendela*scale, 2.8*scale, 6*scale+5.1, 1.375*scale+countJendela*scale, 2*scale, 6*scale+2.3);//tembok depan
				kotak(1.375*scale+countJendela*scale, 2.8*scale, 6*scale+5.1, 1.75*scale+countJendela*scale, 2*scale, 6*scale+2.3);//tembok depan
				//atas belakang
				kotak(1*scale+countJendela*scale, 2.8*scale, 1*scale-3, 1.375*scale+countJendela*scale, 2*scale, 1*scale-6.1);//tembok depan
				kotak(1.375*scale+countJendela*scale, 2.8*scale, 1*scale-3, 1.75*scale+countJendela*scale, 2*scale, 1*scale-6.1);//tembok depan
				//bawah belakang
				kotak(1*scale+countJendela*scale, 1.1*scale, 1*scale-3, 1.375*scale+countJendela*scale, 0.3*scale, 1*scale-6.1);//tembok depan
				kotak(1.375*scale+countJendela*scale, 1.1*scale, 1*scale-3, 1.75*scale+countJendela*scale, 0.3*scale, 1*scale-6.1);//tembok depan
			} else if (countJendela == 17){
				//atas depan
				kotak(1*scale+countJendela*scale, 2.8*scale, 6*scale+5.1, 1.375*scale+countJendela*scale, 2*scale, 6*scale+2.3);//tembok depan
				kotak(1.375*scale+countJendela*scale, 2.8*scale, 6*scale+5.1, 1.75*scale+countJendela*scale, 2*scale, 6*scale+2.3);//tembok depan
				//bawah depan
				kotak(1*scale+countJendela*scale, 1.1*scale, 6*scale+5.1, 1.375*scale+countJendela*scale, 0.3*scale, 6*scale+2.3);//tembok depan
				kotak(1.375*scale+countJendela*scale, 1.1*scale, 6*scale+5.1, 1.75*scale+countJendela*scale, 0.3*scale, 6*scale+2.3);//tembok depan	
			} else if (countJendela == 18 || countJendela == 19){
				
			} else {
				//atas depan
				kotak(1*scale+countJendela*scale, 2.8*scale, 6*scale+5.1, 1.375*scale+countJendela*scale, 2*scale, 6*scale+2.3);//tembok depan
				kotak(1.375*scale+countJendela*scale, 2.8*scale, 6*scale+5.1, 1.75*scale+countJendela*scale, 2*scale, 6*scale+2.3);//tembok depan
				//atas belakang
				kotak(1*scale+countJendela*scale, 2.8*scale, 1*scale-3, 1.375*scale+countJendela*scale, 2*scale, 1*scale-6.1);//tembok depan
				kotak(1.375*scale+countJendela*scale, 2.8*scale, 1*scale-3, 1.75*scale+countJendela*scale, 2*scale, 1*scale-6.1);//tembok depan	
				//bawah depan
				kotak(1*scale+countJendela*scale, 1.1*scale, 6*scale+5.1, 1.375*scale+countJendela*scale, 0.3*scale, 6*scale+2.3);//tembok depan
				kotak(1.375*scale+countJendela*scale, 1.1*scale, 6*scale+5.1, 1.75*scale+countJendela*scale, 0.3*scale, 6*scale+2.3);//tembok depan	
				//bawah belakang
				kotak(1*scale+countJendela*scale, 1.1*scale, 1*scale-3, 1.375*scale+countJendela*scale, 0.3*scale, 1*scale-6.1);//tembok depan
				kotak(1.375*scale+countJendela*scale, 1.1*scale, 1*scale-3, 1.75*scale+countJendela*scale, 0.3*scale, 1*scale-6.1);//tembok depan

			}
		}
		
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[15]); //jendela gede
	glBegin(GL_QUADS);
				//atas depan
				kotak(19*scale, 2.95*scale, 6*scale+5.1, 20.75*scale, 1.55*scale, 6*scale+2.3);//tembok depan
				//atas belakang
				kotak(19*scale, 2.95*scale, 1*scale-3, 20.75*scale, 1.55*scale, 1*scale-6.1);//tembok depan
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[15]); //jendela gede
	glBegin(GL_QUADS);
			for (int countJendelaDalem = 4; countJendelaDalem < 26; countJendelaDalem++){
			if (countJendelaDalem == 4){
				//bawah belakang
				kotak(1*scale+countJendelaDalem*scale, 1.2*scale, 2.25*scale+1.2, 1.375*scale+countJendelaDalem*scale, 0.8*scale, 2.25*scale-1.1);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 1.2*scale, 2.25*scale+1.2, 1.75*scale+countJendelaDalem*scale, 0.8*scale, 2.25*scale-1.1);//tembok depan
				//bawah depan
				kotak(1*scale+countJendelaDalem*scale, 1.2*scale, 4.4*scale-0.2, 1.375*scale+countJendelaDalem*scale, 0.8*scale, 4.5*scale+0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 1.2*scale, 4.4*scale-0.2, 1.75*scale+countJendelaDalem*scale, 0.8*scale, 4.5*scale+0.2);//tembok depan
			} else if (countJendelaDalem == 5){
				//atas belakang
				kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 2.8*scale+1.2, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 2.8*scale-1.1);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 2.8*scale+1.2, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 2.8*scale-1.1);//tembok depan
				//bawah depan
				kotak(1*scale+countJendelaDalem*scale, 1.2*scale, 4.4*scale-0.2, 1.375*scale+countJendelaDalem*scale, 0.8*scale, 4.5*scale+0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 1.2*scale, 4.4*scale-0.2, 1.75*scale+countJendelaDalem*scale, 0.8*scale, 4.5*scale+0.2);//tembok depan	
			} else if (countJendelaDalem == 6 || countJendelaDalem == 7){
				//atas depan
				kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 4.1*scale, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 4*scale-0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 4.1*scale, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 4*scale-0.2);//tembok depan
				//atas belakang
				kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 2.8*scale+1.2, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 2.8*scale-1.1);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 2.8*scale+1.2, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 2.8*scale-1.1);//tembok depan
			} else if (countJendelaDalem == 19 || countJendelaDalem == 18){
				
			} else if (countJendelaDalem == 20 || countJendelaDalem == 21){
				//atas depan
				kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 4.1*scale+0.2, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 4*scale-0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 4.1*scale+0.2, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 4*scale-0.2);//tembok depan
				//atas belakang
				kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 2.55*scale+0.5, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 2.75*scale-0.8);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 2.55*scale+0.5, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 2.75*scale-0.8);//tembok depan	
				//bawah depan
				kotak(1*scale+countJendelaDalem*scale, 1.2*scale, 4.8*scale+0.2, 1.375*scale+countJendelaDalem*scale, 0.8*scale, 4.7*scale-0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 1.2*scale, 4.8*scale+0.2, 1.75*scale+countJendelaDalem*scale, 0.8*scale, 4.7*scale-0.2);//tembok depan	
				//bawah belakang
				kotak(1*scale+countJendelaDalem*scale, 1.2*scale, 2.35*scale+0.5, 1.375*scale+countJendelaDalem*scale, 0.8*scale, 2.55*scale-0.8);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 1.2*scale, 2.35*scale+0.5, 1.75*scale+countJendelaDalem*scale, 0.8*scale, 2.55*scale-0.8);//tembok depan
			} else if (countJendelaDalem >= 22){
					//atas depan
				kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 4.1*scale+0.2, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 4*scale-0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 4.1*scale+0.2, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 4*scale-0.2);//tembok depan
				//atas belakang
				kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 2.55*scale+0.5, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 2.75*scale-0.8);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 2.55*scale+0.5, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 2.75*scale-0.8);//tembok depan	
			} else if (countJendelaDalem == 17){
				//atas depan
				kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 2.1*scale+0.2, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 2.1*scale-0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 2.1*scale+0.2, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 2.1*scale-0.2);//tembok depan
				//atas belakang
				kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 1.6*scale+2.2, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 1.6*scale-0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 1.6*scale+2.2, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 1.6*scale-0.2);//tembok depan	
				//bawah depan
				kotak(1*scale+countJendelaDalem*scale, 1.2*scale, 2.1*scale+0.2, 1.375*scale+countJendelaDalem*scale, 0.8*scale, 2.1*scale-0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 1.2*scale, 2.1*scale+0.2, 1.75*scale+countJendelaDalem*scale, 0.8*scale, 2.1*scale-0.2);//tembok depan	
				//bawah belakang
				kotak(1*scale+countJendelaDalem*scale, 1.2*scale, 1.6*scale+2.2, 1.375*scale+countJendelaDalem*scale, 0.8*scale, 1.6*scale-0.2);//tembok depan
				kotak(1.375*scale+countJendelaDalem*scale, 1.2*scale, 1.6*scale+2.2, 1.75*scale+countJendelaDalem*scale, 0.8*scale, 1.6*scale-0.2);//tembok depan
			}
			
			else {
			//atas depan
			kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 4.1*scale+0.2, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 4*scale-0.2);//tembok depan
			kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 4.1*scale+0.2, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 4*scale-0.2);//tembok depan
			//atas belakang
			kotak(1*scale+countJendelaDalem*scale, 2.9*scale, 2.8*scale+1.2, 1.375*scale+countJendelaDalem*scale, 2.5*scale, 2.8*scale-1.1);//tembok depan
			kotak(1.375*scale+countJendelaDalem*scale, 2.9*scale, 2.8*scale+1.2, 1.75*scale+countJendelaDalem*scale, 2.5*scale, 2.8*scale-1.1);//tembok depan	
			//bawah depan
			kotak(1*scale+countJendelaDalem*scale, 1.2*scale, 4.1*scale+0.2, 1.375*scale+countJendelaDalem*scale, 0.8*scale, 4*scale-0.2);//tembok depan
			kotak(1.375*scale+countJendelaDalem*scale, 1.2*scale, 4.1*scale+0.2, 1.75*scale+countJendelaDalem*scale, 0.8*scale, 4*scale-0.2);//tembok depan	
			//bawah belakang
			kotak(1*scale+countJendelaDalem*scale, 1.2*scale, 2.8*scale+1.2, 1.375*scale+countJendelaDalem*scale, 0.8*scale, 2.8*scale-1.1);//tembok depan
			kotak(1.375*scale+countJendelaDalem*scale, 1.2*scale, 2.8*scale+1.2, 1.75*scale+countJendelaDalem*scale, 0.8*scale, 2.8*scale-1.1);//tembok depan
			}
		}
		
		kotak(5*scale-1.8, 1.2*scale, 2.5*scale, 5*scale-4.2, 0.8*scale, 2.875*scale);//atas pintu lab ICT kanan
		kotak(5*scale-1.8, 1.2*scale, 3.25*scale, 5*scale-4.2, 0.8*scale, 2.875*scale);//atas pintu lab ICT kanan
		kotak(5*scale-1.8, 1.2*scale, 3.6*scale, 5*scale-4.2, 0.8*scale, 3.975*scale);//atas pintu lab ICT kiri
		kotak(5*scale-1.8, 1.2*scale, 4.35*scale, 5*scale-4.2, 0.8*scale, 3.975*scale);//atas pintu lab ICT kiri
		
		kotak(23*scale-1.8, 1.2*scale, 2.5*scale, 23*scale-4.2, 0.8*scale, 2.875*scale);//atas pintu lab ICT kanan
		kotak(23*scale-1.8, 1.2*scale, 3.25*scale, 23*scale-4.2, 0.8*scale, 2.875*scale);//atas pintu lab ICT kanan
		kotak(23*scale-1.8, 1.2*scale, 3.6*scale, 23*scale-4.2, 0.8*scale, 3.975*scale);//atas pintu lab ICT kiri
		kotak(23*scale-1.8, 1.2*scale, 4.35*scale, 23*scale-4.2, 0.8*scale, 3.975*scale);//atas pintu lab ICT kiri
		
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[16]); //tekstur lantai lorong
	glBegin(GL_QUADS);
						//lantai lorong
		kotak(19*scale, 10-scale, 6*scale+5, 22.6*scale, 12-scale, 12.5*scale-3);//lantai lorong kanan
		kotak(5.3*scale, 10-scale, 6*scale+5, 9*scale-5, 12-scale, 12.5*scale-3);//lantai lorong kiri
		kotak(7*scale, 10-scale, 12*scale+6, 28.3*scale-5, 12-scale, 13.5*scale-3);//lantai lorong panjang
	glEnd();
	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[6]);

	    glTranslatef(0, 1.2*scale, 3.5*scale);
	 	glRotatef(-180, 1, 0, 0);

	    drawSilinder(3 , 3.2*scale, 22.2*scale);
	    drawSilinder(3 , 3.2*scale, 21.6*scale);//tiang 3
	    drawSilinder(3 , 3.2*scale, 21*scale);
		drawSilinder(3 , 3.2*scale, 6*scale);// tiang kiri

    glDisable(GL_TEXTURE_2D);
	
	glFlush();	
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    camera();
    draw();
    glutSwapBuffers();
}

void reshape(int w,int h)
{
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60,16.0/9.0,1,9000);
    glMatrixMode(GL_MODELVIEW);
    glDepthMask(GL_TRUE);

}


/*this funtion is used to keep calling the display function periodically
  at a rate of FPS times in one second. The constant FPS is defined above and
  has the value of 60
*/
void timer(int)
{
    glutPostRedisplay();
    glutWarpPointer(width/2,height/2);
    glutTimerFunc(1000/FPS,timer,0);
}

void passive_motion(int x,int y)
{
    /* two variables to store X and Y coordinates, as observed from the center
      of the window
    */
    int dev_x,dev_y;
    dev_x = (width/2)-x;
    dev_y = (height/2)-y;

    /* apply the changes to pitch and yaw*/
    yaw+=(float)dev_x/10.0;
    pitch+=(float)dev_y/10.0;
}

void camera()
{

    if(motion.Forward)
    {
    	if(camX > 575){
    		camX = 575;
		} else if (camZ > 375){
			camZ = 375;
		} 
		else {
	        camX += cos((yaw+90)*TO_RADIANS)/5.0;
        	camZ -= sin((yaw+90)*TO_RADIANS)/5.0;
		}
    }
    if(motion.Backward)
    {
        camX += cos((yaw+90+180)*TO_RADIANS)/5.0;
        camZ -= sin((yaw+90+180)*TO_RADIANS)/5.0;
    }
    if(motion.Left)
    {
        camX += cos((yaw+90+90)*TO_RADIANS)/5.0;
        camZ -= sin((yaw+90+90)*TO_RADIANS)/5.0;
    }
    if(motion.Right)
    {
        camX += cos((yaw+90-90)*TO_RADIANS)/5.0;
        camZ -= sin((yaw+90-90)*TO_RADIANS)/5.0;
    }
    if(motion.up)
    {
    	if (camY > 290){
    		camY = 290;
		} else camY++;
    }
	if(motion.down)
    {
    	if (camY <= 7){
    		camY = 7;
		}else camY--;
    }

    /*limit the values of pitch
      between -60 and 70
    */
    if(pitch>=70)
        pitch = 70;
    if(pitch<=-60)
        pitch=-60;

    glRotatef(-pitch,1.0,0.0,0.0); // Along X axis
    glRotatef(-yaw,0.0,1.0,0.0);    //Along Y axis

    glTranslatef(-camX,-camY,-camZ);
}

void keyboard(unsigned char key,int x,int y)
{
    switch(key)
    {
    case 'W':
    case 'w':
        motion.Forward = true;
        break;
    case 'A':
    case 'a':
        motion.Left = true;
        break;
    case 'S':
    case 's':
        motion.Backward = true;
        break;
    case 'D':
    case 'd':
        motion.Right = true;
        break;
    case 'F':
    case 'f':
        motion.up = true;
        break;
    case 'V':
    case 'v':
        motion.down = true;
        break;
    }
}
void keyboard_up(unsigned char key,int x,int y)
{
    switch(key)
    {
    case 'W':
    case 'w':
        motion.Forward = false;
        break;
    case 'A':
    case 'a':
        motion.Left = false;
        break;
    case 'S':
    case 's':
        motion.Backward = false;
        break;
    case 'D':
    case 'd':
        motion.Right = false;
        break;
    case 'F':
    case 'f':
        motion.up = false;
        break;
    case 'V':
    case 'v':
        motion.down = false;
        break;
    }
}

int main(int argc,char**argv)
{
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("GEDUNG JTK (171524020)");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(passive_motion);
    glutTimerFunc(0,timer,0);    //more info about this is given below at definition of timer()
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboard_up);

    glutMainLoop();
    return 0;
}
