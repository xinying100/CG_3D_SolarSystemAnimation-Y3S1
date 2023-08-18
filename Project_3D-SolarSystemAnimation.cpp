/* Heng Chia Ying - CST2104280 */

#define _USE_MATH_DEFINES 
#include <cstdlib>
#include <cmath>
#include <string>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h> 
#include "getBMP.h"

// Global Variables
static int isDraw = 0, iswriteText = 1, isStar = 1, isFocus = 0, countStar = 0;
static const int longi  = 200;				     // Number of longitudinal slices.
static const int latit  = 200;					 // Number of latitudinal slices.

// Order: Sun, Mercury, Venus, Earth, Moon, Mars, Jupiter, Saturn, Uranus, Neptune
static const float PlanetRadius[10] = { 15.0, 3.0, 4.5, 5.0, 2.0, 3.5, 8.5, 8.0, 7.7, 7.0};
static float PlanetAngle[10]        = { 0,   280, 10,  0,   20,  35,  5,   290, 230, 240};
static float temp_Xposition[10],    self_angle = 0;
static const float PlanetSpace = 1, TorusSize  = 0.3;

static float model_Xangle = 0,   model_Yangle = 0,   model_Zangle = 0;
static float zoomInOut_Z  = 110, frustum_Zmin = 5.0, frustum_Zmax = 700.0;
static float camera_X     = 0,   camera_Y     = 40;
static float los_X        = 0,   los_Y        = 0,   los_Z        = 0;

static const long fontBig		 = (long)GLUT_BITMAP_TIMES_ROMAN_24;		// Font selection
static const long fontSmall		 = (long)GLUT_BITMAP_HELVETICA_18;
static const long fontSmallSmall = (long)GLUT_BITMAP_HELVETICA_12;
static int EarthYears = 0, EarthDays = 0;
static float earth_r;

// For Lighting
static GLfloat mat_specular[]  = { 1, 1, 1, 1 };
static GLfloat mat_shininess[] = { 10 };					// Lower value, shinnier
static GLfloat mat_lightpos0[] = { 0, 10, 0, 0 };			// Position of Light
static GLfloat mat_lightpos1[] = { -50, 100, 100, 0 };
static GLfloat mat_lightpos2[] = { 190, 120.0, 0.0, 0 };
static GLfloat mat_lightpos3[] = { 0, 40.0, 110.0, 0 };
static GLfloat light[]         = { 1, 1, 1, 0 };

// For texture
static const int numTexture      = 11;
static int countPlanet           = 0;
static unsigned int texture[numTexture];		// Array of texture indices.
static float* vertices           = NULL;		// Vertex array of the mapped on the planet.
static float* textureCoordinates = NULL;		// Texture co-ordinates mapped on the planet.

// Load image as a texture. 
void loadTextures()
{
	imageFile* image[numTexture];

	// Load the texture.
	image[0] = getBMP("Star.bmp");
	image[1] = getBMP("Sun.bmp");
	image[2] = getBMP("Mercury.bmp");
	image[3] = getBMP("Venus.bmp");
	image[4] = getBMP("Earth.bmp");
	image[5] = getBMP("Moon.bmp");
	image[6] = getBMP("Mars.bmp");
	image[7] = getBMP("Jupiter.bmp");
	image[8] = getBMP("Saturn.bmp");
	image[9] = getBMP("Uranus.bmp");
	image[10] = getBMP("Neptune.bmp");

	// Bind the specific image to respective texture[i]. 
	for (int i = 0; i < numTexture; i++)
	{
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[i]->width, image[i]->height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, image[i]->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
}

// Fill the vertexS array and texture with co-ordinates of the mapped sample points.
void fillVertexArray(float R)
{
	int k = 0;
	for (int j = 0; j <= latit; j++)
	{
		for (int i = 0; i <= longi; i++)
		{
			vertices[k++] = (R * cos(-M_PI / 2.0 + (float)j / latit * M_PI) * cos(2.0 * (float)i / longi * M_PI));
			vertices[k++] = (R * sin(-M_PI / 2.0 + (float)j / latit * M_PI));
			vertices[k++] = (-R * cos(-M_PI / 2.0 + (float)j / latit * M_PI) * sin(2.0 * (float)i / longi * M_PI));
		}
	}
}

void fillTextureCoorArray()
{
	int i, j, k = 0;
	for (int j = 0; j <= latit; j++)
	{
		for (int i = 0; i <= longi; i++)
		{
			textureCoordinates[k++] = (float)i / longi;
			textureCoordinates[k++] = (float)j / latit;
		}
	}
}

void lighting(float rgb[])
{
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, rgb);
	glMaterialfv(GL_FRONT, GL_SPECULAR, rgb);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void planetselfRotate(float distancefromsun)
{
	glTranslatef(distancefromsun, 0, 0);
	glTranslatef(-distancefromsun, 0, 0);
	glRotatef(self_angle, 0, 0, 1);
	glRotatef(90, 1, 0, 0);
}

void drawarrayelement(void)
{
	for (int j = 0; j < latit; j++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (int i = 0; i <= longi; i++)
		{
			glArrayElement((j + 1) * (longi + 1) + i);
			glArrayElement(j * (longi + 1) + i);
		}
		glEnd();
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawSun(void)
{
	/* Sun */
	GLfloat mat_ambient_diffuse[] = {1, 1, 0, 1};
	lighting(mat_ambient_diffuse);
	glPushMatrix();								// Push 1
	
	// Map Texture
	fillVertexArray(PlanetRadius[0]);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	drawarrayelement();
	
	temp_Xposition[0] = 0;
	glPopMatrix();								// Pop 1
}

void drawMercury(void)
{
	/* Mercury */
	glPushMatrix();															// Push 1			
	glRotatef(PlanetAngle[1], 0, 1, 0);
	
	// Mercury Start Position (Sun Radius + Mercury Radius + Space betweenSunMercury)
	temp_Xposition[1] = -(PlanetRadius[0] + PlanetRadius[1] + 30);
	glRotatef(90, 1, 0, 0);													// Rotate Torus in 90 degree
	GLfloat mat_ambient_diffuse_torus[] = { 0.78, 0.769, 0.063, 1 };		// Torus Color
	lighting(mat_ambient_diffuse_torus);
	glutSolidTorus(TorusSize, temp_Xposition[1], 200, 200);

	GLfloat mat_ambient_diffuse[] = { 1, 1, 0, 1 };					        // Mecury Color
	lighting(mat_ambient_diffuse);
	glTranslatef(temp_Xposition[1], 0, 0);

	// Map Texture
	planetselfRotate(temp_Xposition[1]);
	fillVertexArray(PlanetRadius[1]);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	drawarrayelement();
	glPopMatrix();															// Pop 1
}

void drawVenus(void)
{
	/* Venus */
	glPushMatrix();															// Push 1			
	glRotatef(PlanetAngle[2], 0, 1, 0);

	// Venus Start Position (+ Venus Radius + Space betweenMecuryVenus)
	temp_Xposition[2] = temp_Xposition[1] - (PlanetRadius[2] + (PlanetRadius[1] + PlanetRadius[2] + PlanetSpace));

	glRotatef(90, 1, 0, 0);													// Rotate Torus in 90 degree
	GLfloat mat_ambient_diffuse_torus[] = { 0.988, 0.984, 0.694, 1 };		// Torus Color
	lighting(mat_ambient_diffuse_torus);
	glutSolidTorus(TorusSize, temp_Xposition[2], 200, 200);

	GLfloat mat_ambient_diffuse[] = { 1, 0, 1, 1 };							// Venus Color
	lighting(mat_ambient_diffuse);
	glTranslatef(temp_Xposition[2], 0, 0);

	// Map Texture
	planetselfRotate(temp_Xposition[2]);
	fillVertexArray(PlanetRadius[2]);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	drawarrayelement();

	glPopMatrix();															// Pop 1
}

void drawEarthMoon(void)
{
	/* Earth */

	glPushMatrix();														// Push 1			
	glRotatef(PlanetAngle[3], 0, 1, 0);

	// Earth Start Position ( + Earth Radius + Moon Radius + Space betweenVenusEarth)
	temp_Xposition[3] = temp_Xposition[2] - (PlanetRadius[3] + PlanetRadius[4] 
		+ (PlanetRadius[2] + PlanetRadius[3] + PlanetSpace));
	earth_r = abs(temp_Xposition[3]);
	glRotatef(90, 1, 0, 0);												// Rotate the planet 90 degree

	GLfloat mat_ambient_diffuse_torus[] = { 0.404, 0.82, 0.682, 1 };	// Earth Torus Color
	lighting(mat_ambient_diffuse_torus);
	glutSolidTorus(TorusSize, temp_Xposition[3], 200, 200);

	GLfloat mat_ambient_diffuse[] = { 0, 1, 0, 1 };						// Earth Color
	lighting(mat_ambient_diffuse);
	glTranslatef(temp_Xposition[3], 0, 0);

	// Map Texture
	planetselfRotate(temp_Xposition[3]);
	glRotatef(90, 1, 0, 0);
	fillVertexArray(PlanetRadius[3]);
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	drawarrayelement();

	/* Moon */
	GLfloat mat_ambient_diffuse_moontorus[] = { 0.537, 0.78, 0.282, 1 };	// Moon Torus Color
	lighting(mat_ambient_diffuse_moontorus);
	glutSolidTorus(TorusSize-0.1, -(5 + 2 + 0.3), 200, 200);

	glRotatef(90, 1, 0, 0);
	glRotatef(PlanetAngle[4], 0, 1, 0);
	GLfloat mat_ambient_diffuse_moon[] = { 0, 0, 1, 1 };					// Moon Color
	lighting(mat_ambient_diffuse_moon);
	// Moon Start Position (Earth Radius + Moon Radius + Space betweenEarthMoon)
	glTranslatef(-(5 + 2 + 0.3), 0, 0);

	// Map Texture
	planetselfRotate(-(5 + 2 + 0.3));
	fillVertexArray(PlanetRadius[4]);
	glBindTexture(GL_TEXTURE_2D, texture[5]);
	drawarrayelement();
	temp_Xposition[4] = 0;

	glPopMatrix();																// Pop 1
}

void drawMars(void)
{
	/* Mars */
	glPushMatrix();																// Push 1			
	glRotatef(PlanetAngle[5], 0, 1, 0);

	// Venus Start Position (+ Mars Radius + Space betweenEarthMars)
	temp_Xposition[5] = temp_Xposition[3] - (PlanetRadius[5] + 
		(PlanetRadius[3] + PlanetRadius[4] + PlanetRadius[5] + PlanetSpace));
	glRotatef(90, 1, 0, 0);														// Rotate Mars in 90 degree
	GLfloat mat_ambient_diffuse_torus[] = { 0.831, 0.176, 0.122, 1 };			// Mars Torus Color
	lighting(mat_ambient_diffuse_torus);
	glutSolidTorus(TorusSize, temp_Xposition[5], 200, 200);

	GLfloat mat_ambient_diffuse[] = { 0, 1, 1, 1 };								// Mars Color
	lighting(mat_ambient_diffuse);
	glTranslatef(temp_Xposition[5], 0, 0);

	// Map Texture
	planetselfRotate(temp_Xposition[5]);
	fillVertexArray(PlanetRadius[5]);
	glBindTexture(GL_TEXTURE_2D, texture[6]);
	drawarrayelement();

	glPopMatrix();							// Pop 1
}

void drawJupiter(void)
{
	/* Jupiter */
	glPushMatrix();															// Push 1			
	glRotatef(PlanetAngle[6], 0, 1, 0);

	// Jupiter Start Position (+ Jupiter Radius + Space betweenMarsJupiter)
	temp_Xposition[6] = temp_Xposition[5] - (PlanetRadius[6] 
		+ (PlanetRadius[5] + PlanetRadius[6] + PlanetSpace));
	glRotatef(90, 1, 0, 0);													// Rotate the Torus 90 degree

	GLfloat mat_ambient_diffuse_torus[] = { 0.831, 0.412, 0.122, 1 };		// Jupiter Torus Color
	lighting(mat_ambient_diffuse_torus);
	glutSolidTorus(TorusSize, temp_Xposition[6], 200, 200);

	GLfloat mat_ambient_diffuse[] = { 0, 0.5, 0.5, 1 };						// Jupiter Color
	lighting(mat_ambient_diffuse);
	glTranslatef(temp_Xposition[6], 0, 0);

	// Map Texture
	planetselfRotate(temp_Xposition[6]);
	fillVertexArray(PlanetRadius[6]);
	glBindTexture(GL_TEXTURE_2D, texture[7]);
	drawarrayelement();

	glPopMatrix();															// Pop 1
}

void drawSaturn(void)
{
	/* Saturn */
	glPushMatrix();														// Push 1		
	glRotatef(PlanetAngle[7], 0, 1, 0);

	// Saturn Start Position (+ Saturn Radius + Space betweenJupiterSaturn)
	temp_Xposition[7] = temp_Xposition[6] - (PlanetRadius[7] 
		+ (PlanetRadius[6] + PlanetRadius[7] + PlanetSpace));
	
	glRotatef(90, 1, 0, 0);												// Rotate Torus in 90 degree
	GLfloat mat_ambient_diffuse_torus[] = { 0.831, 0.698, 0.122, 1 };	// Saturn Torus Color
	lighting(mat_ambient_diffuse_torus);
	glutSolidTorus(TorusSize, temp_Xposition[7], 200, 200);

	glRotatef(330, 1, 0, 0);
	GLfloat mat_ambient_diffuse_disk[] = { 1, 1, 1, 1 };				// Saturn Disk Color
	lighting(mat_ambient_diffuse_disk);
	glTranslatef(temp_Xposition[7], 0, 0);
	gluDisk(gluNewQuadric(), PlanetRadius[7] + 1, PlanetRadius[7] + 5, 200, 200);

	GLfloat mat_ambient_diffuse[] = { 0.5, 0, 0.5, 1 };					// Saturn Color
	lighting(mat_ambient_diffuse);

	// Map Texture
	planetselfRotate(temp_Xposition[7]);
	fillVertexArray(PlanetRadius[7]);
	glBindTexture(GL_TEXTURE_2D, texture[8]);
	drawarrayelement();
	glPopMatrix();														// Pop 1
}

void drawUranus(void)
{
	/* Uranus */
	glPushMatrix();														// Push 1			
	glRotatef(PlanetAngle[8], 0, 1, 0);

	// Uranus Start Position (+ Uranus Radius + Space betweenSaturnUranus)
	temp_Xposition[8] = temp_Xposition[7] - (PlanetRadius[8] 
		+ (PlanetRadius[7] + PlanetRadius[8] + PlanetSpace));

	glRotatef(90, 1, 0, 0);
	GLfloat mat_ambient_diffuse_torus[] = { 0.4, 0.89, 0.188, 1 };		// Uranus Torus Color
	lighting(mat_ambient_diffuse_torus);
	glutSolidTorus(TorusSize, temp_Xposition[8], 200, 200);

	glRotatef(30, 1, 0, 0);												// Rotate Disk in 30 degree into 120 degree
	GLfloat mat_ambient_diffuse_disk[] = { 1, 1, 1, 1 };				// Uranus Disk Color
	lighting(mat_ambient_diffuse_disk);
	glTranslatef(temp_Xposition[8], 0, 0);
	gluDisk(gluNewQuadric(), PlanetRadius[8] + 1, PlanetRadius[8] + 2, 200, 200);

	GLfloat mat_ambient_diffuse[] = { 0, 0.5, 0, 1 };					// Uranus Color
	lighting(mat_ambient_diffuse);

	// Map Texture
	planetselfRotate(temp_Xposition[8]);
	fillVertexArray(PlanetRadius[8]);
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	drawarrayelement();
	glPopMatrix();														// Pop 1
}

void drawNeptune(void)
{
	/* Neptune */
	glPushMatrix();														// Push 1		
	glRotatef(PlanetAngle[9], 0, 1, 0);

	// Neptune Start Position (+ Neptune Radius + Space betweenUranusNeptune)
	temp_Xposition[9] = temp_Xposition[8] - (PlanetRadius[9] 
		+ (PlanetRadius[8] + PlanetRadius[9] + PlanetSpace));

	glRotatef(90, 1, 0, 0);												// Rotate Torus in 90 degree
	GLfloat mat_ambient_diffuse_torus[] = { 0.212, 0.467, 0.878, 1 };	// Neptune Torus Color
	lighting(mat_ambient_diffuse_torus);
	glutSolidTorus(TorusSize, temp_Xposition[9], 200, 200);

	GLfloat mat_ambient_diffuse[] = { 0, 0, 0.5, 1 };					// Neptune Color
	lighting(mat_ambient_diffuse);
	glTranslatef(temp_Xposition[9], 0, 0);

	// Map Texture
	planetselfRotate(temp_Xposition[9]);
	fillVertexArray(PlanetRadius[9]);
	glBindTexture(GL_TEXTURE_2D, texture[10]);
	drawarrayelement();
	glPopMatrix();														// Pop 1
}

void drawStar(void)
{
	/* Star */
	glPushMatrix();						// Push 1		

	glBindTexture(GL_TEXTURE_2D, texture[0]);			// TopLeft Star
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1700.0, 0.0, -600);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 0.0, -600);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.0, 900.0, -600);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1700.0, 900.0, -600);
	glEnd();

	glPushMatrix();
	glTranslatef(-850, -450, 0);		// 3 translate to original location
	glRotatef(180, 0, 0, 1);			// 2 rotate recti
	glTranslatef(850, 450, 0);			// 1 translate to origin

	glBindTexture(GL_TEXTURE_2D, texture[0]);			// BottomLeft Star
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1700.0, -900.0, -600);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, -900.0, -600);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.0, 0.0, -600);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1700.0, 0.0, -600);
	glEnd();
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(850, 450, 0);
	glRotatef(180, 0, 0, 1);
	glTranslatef(-850, -450, 0);

	glBindTexture(GL_TEXTURE_2D, texture[0]);			// TopRight Star
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, -600);
	glTexCoord2f(1.0, 0.0); glVertex3f(1700.0, 0.0, -600);
	glTexCoord2f(1.0, 1.0); glVertex3f(1700.0, 900.0, -600);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 900.0, -600);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, texture[0]);			// BottomRight Star
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, -900.0, -600);
	glTexCoord2f(1.0, 0.0); glVertex3f(1700.0, -900.0, -600);
	glTexCoord2f(1.0, 1.0); glVertex3f(1700.0, 0.0, -600);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.0, -600);
	glEnd();

	int time_count_star = 30, num = 0, num2 = 0;
	if (countStar % time_count_star == time_count_star / 2) {
		num = rand() % 10;
		num2 = rand() % 10;
	}
	else if (countStar % time_count_star > time_count_star / 2)
	{	
		for (int i = 0; i < 5; i++) {
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.0 + (-1700 * ((num2+i) % 2)), 0.0, -550);
			glTexCoord2f(1.0, 0.0); glVertex3f((num2+i) * 170.0, 0.0, -550);
			glTexCoord2f(1.0, 1.0); glVertex3f((num2 + i) * 170.0, (num2 + i) * 170.0 - 800, -550);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.0 + (-1700 * ((num2 + i) % 2)), (num2 + i) * 170.0 - 800, -550);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.0 + (-1700 * ((num+i) % 2)), - (num2 + i) * 170.0 - 800, -550);
			glTexCoord2f(1.0, 0.0); glVertex3f((num2 + i) * 170.0, -(num2 + i) * 170.0 - 800, -550);
			glTexCoord2f(1.0, 1.0); glVertex3f((num2 + i) * 170.0, 0.0, - 550);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.0 + (-1700 * ((num + i) % 2)), 0.0, -550);
			glEnd();
		}
	}
	glPopMatrix();						// Pop 1
}

void writeBitmapString(void* font, const char* string)
{
	const char* c;							// String Pointer
	for (c = string; *c != '\0'; c++)		// Printing each char till the End Of String
		glutBitmapCharacter(font, *c);		// Not our function
}

void writeBitmapIntDays(void* font, int num)
{
	int temp, daysdigit = 0;
	int days[5];

	while (num != 0)
	{
		temp = num % 10;
		num  = num / 10;
		daysdigit++;
		days[daysdigit] = temp;
	}

	if (daysdigit == 0)
	{
		glutBitmapCharacter(font, 48);						// Print 0 day
		writeBitmapString((void*)fontSmall, " Days,");
	}
	else
	{
		// Print Number of Days for Earth
		for (int i = daysdigit; i > 0; i--)
		{
			glutBitmapCharacter(font, 48 + days[i]);
			if (i == 1)		writeBitmapString((void*)fontSmall, " Days,");
		}
	}
}

void writeBitmapIntYears(void* font, int num)
{
	int temp, yearsdigit = 0;
	int years[20];

	while (num != 0)
	{
		temp = num % 10;
		num  = num / 10;
		yearsdigit++;
		years[yearsdigit] = temp;
	}

	if (yearsdigit == 0)
	{
		glutBitmapCharacter(font, 48);				   // Print 0 years
		writeBitmapString((void*)fontSmall, " Years");
	}
	else
	{
		// Print Number of Years for Earth
		for (int i = yearsdigit; i > 0; i--)
		{
			glutBitmapCharacter(font, 48 + years[i]);
			if (i == 1)		writeBitmapString((void*)fontSmall, " Years");
		}
	}
}

int writeText(void)
{
	if (iswriteText == 0) return 0;
	else
	{
		// Write labels.
		GLfloat mat_ambient_diffuse[] = { 1, 0, 0, 1 };		// Text Color
		lighting(mat_ambient_diffuse);

		glRasterPos3f(170, 130.0, 0.0);
		writeBitmapString((void*)fontSmall, " ");
		glRasterPos3f(190, 130.0, 0.0);
		writeBitmapString((void*)fontSmall, "Earth: ");
		glRasterPos3f(220, 130.0, 0.0);
		writeBitmapIntDays((void*)fontSmall, EarthDays);
		glRasterPos3f(260, 130.0, 0.0);
		writeBitmapIntYears((void*)fontSmall, EarthYears);

		GLfloat mat_ambient_diffuse1[] = { 1, 1, 1, 1 };		// Text Color
		lighting(mat_ambient_diffuse1);
		glRasterPos3f(-290, 130.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "User Info:");
		glRasterPos3f(-295, 125.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "SPACE: Start animation.");
		glRasterPos3f(-300, 120.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "r    : Reset camera position.");
		glRasterPos3f(-305, 115.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "s    : Reset animation.");
		glRasterPos3f(-310, 110.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "w    : Reset axis position.");
		glRasterPos3f(-315, 105.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "x    : Rotate along x-axis.");
		glRasterPos3f(-320, 100.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "y    : Rotate along y-axis.");
		glRasterPos3f(-325, 95.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "z    : Rotate along z-axis.");
		glRasterPos3f(-330, 90.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "e    : Focus on Earth. (press again to exit focus)");
		glRasterPos3f(-335, 85.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "+    : Zoom In.");
		glRasterPos3f(-340, 80.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "-    : Zoom Out.");
		glRasterPos3f(-345, 75.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "ARROW-UP       : Camera move up.");
		glRasterPos3f(-350, 70.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "ARROW-DOWN  : Camera move down.");
		glRasterPos3f(-355, 65.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "ARROW-LEFT    : Camera move left.");
		glRasterPos3f(-360, 60.0, 0.0);
		writeBitmapString((void*)fontSmallSmall, "ARROW-RIGHT   : Camera move right.");

		glRasterPos3f(-40, -150.0, 80.0);
		writeBitmapString((void*)fontBig, "Solar System");
		return 1;
	}
}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	drawStar();  glBindTexture(GL_TEXTURE_2D, 0);

	gluLookAt(camera_X, camera_Y, zoomInOut_Z, los_X, los_Y, los_Z, 0, 1, 0);

	glTranslatef(0, 0, -40);		     // Move everything backward 
	lighting(mat_specular);
	iswriteText = writeText();

	// Rotate entire model along their axis, when X Y Z keypress
	glRotatef(model_Xangle, 1, 0, 0);   // Entire model rotate along Xaxis
	glRotatef(model_Yangle, 0, 1, 0);   // Entire model rotate along Yaxis 
	glRotatef(model_Zangle, 0, 0, 1);   // Entire model rotate along Zaxis

	drawSun();
	drawMercury();
	drawVenus();
	drawEarthMoon();
	drawMars();
	drawJupiter();
	drawSaturn();
	drawUranus();
	drawNeptune();
	lighting(mat_specular);
	glutSwapBuffers();
}

void setup(void)
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SMOOTH);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Texture
	glGenTextures(numTexture, texture);			// Texture IDs
	loadTextures();
	// Specify how texture values combine with current surface color values.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
	glEnable(GL_TEXTURE_2D);

	// Allocate space for vertex and texture coordinates array.
	vertices = new float[3 * (longi + 1) * (latit + 1)];
	textureCoordinates = new float[2 * (longi + 1) * (latit + 1)];
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, textureCoordinates);

	// Fill texture co-ordinates arrays.
	fillTextureCoorArray();
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT0, GL_POSITION, mat_lightpos0);

	glLightfv(GL_LIGHT1, GL_AMBIENT_AND_DIFFUSE, light);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT1, GL_POSITION, mat_lightpos1);

	glLightfv(GL_LIGHT2, GL_AMBIENT_AND_DIFFUSE, light);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT2, GL_POSITION, mat_lightpos2);

	glLightfv(GL_LIGHT3, GL_AMBIENT_AND_DIFFUSE, light);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT3, GL_POSITION, mat_lightpos3);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);
}

void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-14.0, 14.0, -7.0, 7.0, frustum_Zmin, frustum_Zmax);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void animate(int val)
{
	if (isDraw)
	{
		self_angle += 25;
		if (self_angle > 360)			self_angle -= 360;

		PlanetAngle[1] += 8;
		if (PlanetAngle[1] > 360)		PlanetAngle[1] -= 360;

		PlanetAngle[2] += 7;
		if (PlanetAngle[2] > 360)		PlanetAngle[2] -= 360;

		PlanetAngle[3] += 6;
		if (PlanetAngle[3] > 360)		PlanetAngle[3] -= 360;
		for (int i = 0; i < 6; i++)
		{
			EarthDays += 1;
			if (EarthDays > 365)
			{
				EarthDays  -= 365;
				EarthYears += 1;
			}
			glutPostRedisplay();
		}

		PlanetAngle[4] += 10;
		if (PlanetAngle[4] > 360)		PlanetAngle[4] -= 360;

		PlanetAngle[5] += 5;
		if (PlanetAngle[5] > 360)		PlanetAngle[5] -= 360;

		PlanetAngle[6] += 4;
		if (PlanetAngle[6] > 360)		PlanetAngle[6] -= 360;

		PlanetAngle[7] += 3;
		if (PlanetAngle[7] > 360)		PlanetAngle[7] -= 360;

		PlanetAngle[8] += 2;
		if (PlanetAngle[8] > 360)		PlanetAngle[8] -= 360;

		PlanetAngle[9] += 1;
		if (PlanetAngle[9] > 360)		PlanetAngle[9] -= 360;

		countStar++;

		if (isFocus)
		{
			los_X = earth_r * sin((PlanetAngle[3] - 90) * M_PI / 180.0);
			los_Z = earth_r * cos((PlanetAngle[3] - 90) * M_PI / 180.0);
			camera_X = earth_r * sin((PlanetAngle[3] - 90) * M_PI / 180.0);
			zoomInOut_Z = earth_r * cos((PlanetAngle[3] - 90) * M_PI / 180.0) + 5;
			//camera_X = (earth_r) * sin((PlanetAngle[3] - 90) * M_PI / 180.0) - 2;
			//zoomInOut_Z = (earth_r) * cos((PlanetAngle[3] - 90) * M_PI / 180.0) + 2 ;
		}

		glutPostRedisplay();
		glutTimerFunc(30, animate, val);
	}
}

void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case ' ':
		if (isDraw) isDraw = 0;
		else { isDraw = 1; animate(1); }
		break;
	case 'w':								// Solar System's model initial angle
		model_Xangle = 0; model_Yangle = 0; model_Zangle = 0;
		break;
	case 'x':								// Solar System rotate along X-axis
		model_Xangle += 2;
		if (model_Xangle > 360)	model_Xangle -= 360;
		break;
	case 'y':								// Solar System rotate along Y-axis
		model_Yangle += 2;
		if (model_Yangle > 360)	model_Yangle -= 360;
		break;
	case 'z':								// Solar System rotate along Z-axis
		model_Zangle += 2;
		if (model_Zangle > 360) model_Zangle -= 360;
		break;
	case 's':								// Reset the Animation
	{
		// Reset the entire Solar System to original position
		camera_X     = 0, camera_Y     = 40;  zoomInOut_Z  = 110;
		los_X        = 0, los_Y        = 0,   los_Z        = 0;
		model_Xangle = 0; model_Yangle = 0;   model_Zangle = 0;
		// Reset all planets to its original position
		PlanetAngle[0] = 0;   PlanetAngle[1] = 280;   PlanetAngle[2] = 10;
		PlanetAngle[3] = 0;   PlanetAngle[4] = 20;    PlanetAngle[5] = 35;
		PlanetAngle[6] = 5;   PlanetAngle[7] = 290;   PlanetAngle[8] = 230;
		PlanetAngle[9] = 240;
		// Reset Earth days & years
		EarthDays = 0;		EarthYears = 0;

		if (isDraw)       isDraw      = 0;
		if (!iswriteText) iswriteText = 1;
		if (isFocus)      isFocus     = 0;
		break;
	}
	case '+':								// Zoom Into the scene (camera toward scene nearer)
		if (zoomInOut_Z > frustum_Zmin )	zoomInOut_Z -= 5;
		if (camera_Y > 0)					camera_Y    -= 2;
		iswriteText = 0;
		break;
	case '-':								// Zoom Out the scene (camera go further away)
		if (zoomInOut_Z < frustum_Zmax)		zoomInOut_Z += 5;
		if (camera_Y < 40)					camera_Y    += 2;
		iswriteText = 0;
		break;
	case 'r':								// Reset the scene (original)
		camera_X = 0, camera_Y = 40; zoomInOut_Z = 110;
		los_X = 0, los_Y = 0, los_Z = 0;
		if (!iswriteText) iswriteText = 1;
		break;
	case 'e':								// Focus Mercury
		if (!isFocus)
		{
			los_X         = earth_r * sin((PlanetAngle[3]- 90) * M_PI / 180.0);
			los_Z         = earth_r * cos((PlanetAngle[3] - 90) * M_PI / 180.0);
			//camera_X    = (earth_r) * sin((PlanetAngle[3] - 90) * M_PI / 180.0) - 2 ;
			//zoomInOut_Z = (earth_r) * cos((PlanetAngle[3] - 90) * M_PI / 180.0) + 2;
			camera_X      = earth_r * sin((PlanetAngle[3] - 90) * M_PI / 180.0);
			zoomInOut_Z   = earth_r * cos((PlanetAngle[3] - 90) * M_PI / 180.0) + 5;
			camera_Y      = 5;
			iswriteText   = 0;
			isFocus       = 1;
		}
		else
		{
			camera_X = 0, camera_Y = 40;  zoomInOut_Z = 110;
			los_X = 0, los_Y = 0, los_Z = 0;
			model_Xangle = 0; model_Yangle = 0;   model_Zangle = 0;
			if (!iswriteText)  iswriteText = 1;
			if (isFocus)       isFocus = 0;
		}
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void specialKeyInput(int key, int x, int y)			// KeyInput for Arrow Keys
{
	if (key == GLUT_KEY_UP && camera_Y < 80)
		{ camera_Y += 2; los_Y += 2; iswriteText = 0; }
	if (key == GLUT_KEY_DOWN && camera_Y > 0)
		{ camera_Y -= 2; los_Y -= 2; iswriteText = 0; }
	if (key == GLUT_KEY_LEFT && camera_X > temp_Xposition[9])
		{ camera_X -= 2; los_X -= 2; iswriteText = 0; }
	if (key == GLUT_KEY_RIGHT && camera_X < -temp_Xposition[9])
		{ camera_X += 2; los_X += 2; iswriteText = 0; }
	glutPostRedisplay();
}

void printInteraction(void)
{
	std::cout << "Interaction:" << std::endl;
	std::cout << "Press SPACEBAR to start animation." << std::endl;
	std::cout << "Press r to reset the camera initial position." << std::endl;
	std::cout << "Press s to reset the animation to the starting point." << std::endl;
	std::cout << "Press w to reset the entire Solar System to initial axis position." << std::endl;
	std::cout << "Press x to rotate the entire Solar System along x-axis." << std::endl;
	std::cout << "Press y to rotate the entire Solar System along y-axis." << std::endl;
	std::cout << "Press z to rotate the entire Solar System along z-axis." << std::endl;
	std::cout << "Press e to focus on Earth. (Press again to exit focus)" << std::endl;
	std::cout << "Press + to zoom in the scene." << std::endl;
	std::cout << "Press - to zoom out the scene." << std::endl;
	std::cout << "Press ARROWKEY (UP, DOWN, LEFT, RIGHT) to move the camera and line of sight." << std::endl;
}

int main(int argc, char** argv)
{
	printInteraction();
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(1400, 700);
	glutInitWindowPosition(70, 30);
	glutCreateWindow("CST2104280_Solar System.cpp");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);				// Function for ASCII key entry
	glutSpecialFunc(specialKeyInput);		// Function for non-ASCII key entry

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}
