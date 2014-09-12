#include <iostream>
#include <stdio.h>
#include <cmath>
#include <ctime>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <fstream>
#include <GL/glut.h>
using namespace std;

#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define RAD2DEG(rad) (rad * 180 / PI)

// Function Declarations
void drawScene();
void update(int value);
void drawSpider();
void drawCannon();
void drawBox(float len);
void drawBall(float rad);
void drawTriangle();
void initRendering();
void handleResize(int w, int h);
void handleKeypress1(unsigned char key, int x, int y);
void handleKeypress2(int key, int x, int y);
void handleMouseclick(int button, int state, int x, int y);
void handleLeftMouseDrag(int x,int y);
void handleRightMouseDrag(int x,int y);
void spider_init();
void laser_init();
void pause_toggle();
void print_score();
float move_checker(float pos_x,int direction);

// Global Variables
float ball_x = 1.0f;
float ball_y = 0.0f;
float ball_velx = 0.01f;
float ball_vely = 0.02f;
float ball_rad = 0.2f;
float box_len = 4.0f;
float spider_size = 0.1f;
float cannon_size = 0.20f;
float tri_x = 0.0f;
float tri_y = -box_len / 20 * 9;
float tri_len = box_len / 10;
float theta = 0.0f; 
float laser_vel = 0.02f;
float laser_y = tri_y;
int w,h,num_spiders,num_lasers;
int spider_time_count = 100;
int windowWidth;
int windowHeight;
int laser_flag;
float min_spider_vely = 0.01f;
float max_spider_vely = 0.02f;
float spider_min_pos = -box_len / 20 * 9;
float spider_max_pos = box_len / 20 * 9;
char color[2],move_component;
float red_x = -box_len / 20 * 9;
float green_x = box_len / 20 * 9;
int laser_ready = 1;
int laser_time_count = 100;
bool pause = 0;
int score = 0;
long long game_time = 0;
int spider_freq = 250;
bool game_over;
bool game_start;
bool sounds;
char scores[100];
int top_scores[10];

typedef struct spider_struct {
	float x;
	float y;
	float speed;
	char color;
	bool flag;
}spider_struct;

typedef struct laser_struct {
	float x;
	float y;
	float velx;
	float vely;
	float theta;
	bool flag;
}laser_struct;

spider_struct spiders[1000];
laser_struct lasers[100000];

#define assign_vel() min_spider_vely + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max_spider_vely-min_spider_vely)))
#define spider_pos() spider_min_pos +  static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(spider_max_pos-spider_min_pos)))

int main(int argc, char **argv) {
	if(strcmp(argv[1],"on") == 0)
		sounds = 1;
	//Reading in the highscores
	fstream scorefile;
	scorefile.open("highscores");
	int count_score = 0,t;
	while(scorefile >> t)
		top_scores[count_score++] = t;
	color[0]='r';color[1]='g';color[2]='b';	
	srand (static_cast <unsigned> (time(0)));

	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	w = glutGet(GLUT_SCREEN_WIDTH);
	h = glutGet(GLUT_SCREEN_HEIGHT);
	windowWidth = w / 3;
	windowHeight = h / 2;

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition((w - windowWidth) / 2, (h - windowHeight) / 2);

	glutCreateWindow("Arachnophobia");  // Setup the window
	initRendering();

	// Register callbacks
	glutDisplayFunc(drawScene);
	glutIdleFunc(drawScene);
	glutKeyboardFunc(handleKeypress1);
	glutSpecialFunc(handleKeypress2);
	glutMouseFunc(handleMouseclick);
	glutReshapeFunc(handleResize);
	glutTimerFunc(10, update, 0);

	glutMainLoop();
	return 0;
}

// Function to draw objects on the screen
void drawScene() {

	char c;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0f,1.0f,1.0f,1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	if(game_start == 0 && game_over == 0) {
		glTranslatef(0.0f, 0.0f, -5.0f);
		char text[50];
		sprintf(text, "ARACHNOPHOBIA");
		glColor3f(0, 100.0/255, 0);
		glRasterPos3f(-box_len / 4, box_len / 10, 0);
		for(int i = 0; text[i] != '\0'; i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
		sprintf(text, "Hit ENTER to continue!");
		glColor3f(1, 0, 0);
		glRasterPos3f(-box_len / 4 - box_len / 40, 0, 0);
		for(int i = 0; text[i] != '\0'; i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
	}

	else if(game_over == 0 && game_start == 1) {

		// Draw Box
		glTranslatef(0.0f, 0.0f, -5.0f);
		glColor3f(0.0f, 0.0f, 1.0f);
		drawBox(box_len);

		// Draw line of sepration
		glPushMatrix();
		glLineWidth(2.5); 
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_LINES);
		glVertex3f(-box_len / 2, -box_len / 10 * 4, 0.0);		//The box is drawn with its center at origin so to get 10 % of the box for the 
		glVertex3f(box_len / 2, -box_len / 10 * 4, 0.0);		//line, it's -4/5*box_len. Similarly for the x coordinates.
		glEnd();
		glPopMatrix();

		// Draw red box
		glPushMatrix();
		glTranslatef(red_x, -box_len / 20 * 9, 0.0f);
		glBegin(GL_QUADS);
		if(move_component == 'r')
			glColor3f(100.0/255,0,0);
		else
			glColor3f(1.0f, 0.0f, 0.0f);
		drawBox(box_len / 10);
		glEnd();
		glPopMatrix();

		// Draw green box
		glPushMatrix();
		glTranslatef(green_x, -box_len / 20 * 9, 0.0f);
		glBegin(GL_QUADS);
		if(move_component == 'g')
			glColor3f(0,100.0/255,0);
		else
		glColor3f(0.0f, 1.0f, 0.0f);
		drawBox(box_len / 10);
		glEnd();
		glPopMatrix();

		// Draw Cannon
		glPushMatrix();
		glTranslatef(tri_x, tri_y, 0.0f);
		glRotatef(theta, 0.0f, 0.0f, 1.0f);
		drawCannon();
		glPopMatrix();

		//Print Score
		glPushMatrix();
		print_score();
		glPopMatrix();

		// Draw lasers
		for(int i=0;i<num_lasers;i++) {
			if(lasers[i].flag == 1)
			{
				glLineWidth(2.5); 
				glColor3f(1.0, 0.0, 0.0);
				glBegin(GL_LINES);
				glVertex3f(lasers[i].x, lasers[i].y, 0.0);
				glVertex3f(lasers[i].x + box_len / 10 * cos(lasers[i].theta), lasers[i].y + (box_len / 10 * sin(lasers[i].theta)), 0.0);
				glEnd();
			}
		}

		// Draw spiders
		for(int i=0;i<num_spiders;i++) {
			if(spiders[i].flag == 1) {
				glPushMatrix();
				glTranslatef(spiders[i].x, spiders[i].y, 0.0f);
				c=spiders[i].color;
				if(c=='r')
					glColor3f(1.0f, 0.0f, 0.0f);
				else if(c=='g')
					glColor3f(0.0f, 1.0f, 0.0f);
				else
					glColor3f(0.0f, 0.0f, 0.0f);
				drawSpider();
				glPopMatrix();
			}
		}
	}
	else if(game_over == 1) {
		glTranslatef(0.0f, 0.0f, -5.0f);
		char text[50];
		sprintf(text, "Score %d", score);
		glColor3f(0, 0, 1);
		glRasterPos3f(-box_len / 8, box_len / 10, 0);
		for(int i = 0; text[i] != '\0'; i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
		if(score >= top_scores[9]) {
			int pos=0;
			for(int i=0;i<10;i++)
				if(top_scores[i] <= score)
				{
					pos = i+1;
					break;
				}
			if(pos == 1)
				sprintf(text, "You got the highest score!!");
			else if(pos == 2)
				sprintf(text, "You got the 2nd highest score!!");
			else if(pos == 3)
				sprintf(text, "You got the 3rd highest score!!");
			else 
				sprintf(text, "You got the %dth highest score!!",pos);
			glColor3f(0, 1, 0);
			glRasterPos3f(-box_len / 4 - 0.4f, box_len / 10 + 0.4, 0);
			for(int i = 0; text[i] != '\0'; i++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
			for(int i=9;i>=pos;i--)
				top_scores[i]=top_scores[i-1];
			top_scores[pos-1] = score;
			ofstream scores("highscores");
			for(int i=0;i<10;i++) 
				scores << top_scores[i] << endl;
		}
		sprintf(text, "Press ESC or Q to quit game");
		glColor3f(0, 0, 0);
		glRasterPos3f(-box_len / 4, 0, 0);
		for(int i = 0; text[i] != '\0'; i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
	}

	glPopMatrix();
	glutSwapBuffers();
}

//Generate a new spider
void spider_init() {
	spiders[num_spiders].x = spider_pos();
	spiders[num_spiders].y = spider_max_pos;
	spiders[num_spiders].speed = assign_vel();
	spiders[num_spiders].color = color[rand() % 3];
	spiders[num_spiders].flag = 1;
}

// Function to handle all calculations in the scene
// updated evry 10 milliseconds
void update(int value) {
	if(pause == 0 && game_over == 0 && game_start == 1) {
		if(laser_time_count == 100) {
			laser_time_count = 0;
			laser_ready = 1;
		}

		if(game_time % 1000 == 0) {
			if(min_spider_vely < 0.02f)
				min_spider_vely += 0.002f;
			if(max_spider_vely < 0.04f)
				max_spider_vely += 0.002f;
			if(spider_freq > 100)
				spider_freq -= 20;
		}


		if(spider_time_count >= spider_freq) {					// a spider appears after every 2.5 seconds
			spider_time_count = 0;
			spider_init();
			num_spiders ++;
		}

		//Updating positions of spiders and catching them in the green or red boxes
		for(int i=0;i<num_spiders;i++) { 

			if(spiders[i].flag == 1) {
				spiders[i].y -= spiders[i].speed;
				if(spiders[i].y <= -box_len / 2 + spiders[i].speed ) {
					spiders[i].speed = 0;
					score -= 5;
					if(abs(spiders[i].x - tri_x) <= cannon_size) {
						game_over = 1;
						drawScene();
					}	
				}
				else if(spiders[i].y <= -box_len / 2 + box_len / 10 && spiders[i].y > -box_len / 2) { 
					if(abs(spiders[i].x - green_x) <= box_len / 20 && spiders[i].speed != 0) {
						if(spiders[i].color == 'g')
							score += 1;
						else
							score -= 1;
						spiders[i].flag = 0;
					}
					if(abs(spiders[i].x - red_x) <= box_len / 20 && spiders[i].speed != 0) {
						if(spiders[i].color == 'r')
							score += 1;
						else
							score -= 1;
						spiders[i].flag = 0;
					}
				}
			}
		}

		//Update position of laser
		for(int i=0;i<num_lasers;i++) {
			if(lasers[i].flag == 1) {
				lasers[i].x += lasers[i].velx;
				lasers[i].y += lasers[i].vely;
				if(lasers[i].x + box_len / 10 * cos(lasers[i].theta) > box_len / 2 
						|| lasers[i].x + box_len / 10 * cos(lasers[i].theta) < -box_len / 2) {
					lasers[i].velx *= -1;
					lasers[i].theta = PI - lasers[i].theta;
				}
				if(lasers[i].y > box_len / 2) {
					lasers[i].flag = 0;
				}
			}
		}

		//Handle killing of spiders with the lasers
		for(int i=0;i<num_spiders;i++) { 
			if(spiders[i].flag == 1) { 
				for(int j=0;j<num_lasers;j++) {
					if(lasers[j].flag == 1) {
						if((lasers[j].x - spiders[i].x)*(lasers[j].x - spiders[i].x) 
								+ (lasers[j].y - spiders[i].y)*(lasers[j].y - spiders[i].y) 
								<= 4*spider_size*spider_size) {
							if(spiders[i].color == 'b')
								score += 1;
							else
								score -= 1;
							lasers[j].flag = 0;
							spiders[i].flag = 0;
						}
					}
				} 
			}
		}

		glutTimerFunc(10, update, 0);
		spider_time_count ++;
		game_time ++;
		laser_time_count ++;
	}
}

void drawSpider() {

	drawBall(spider_size);
	glBegin(GL_LINES);

	glVertex2f(-spider_size * cos(PI/5),spider_size * sin(PI/5));
	glVertex2f(-1.5 * spider_size * cos(PI/5),1.5 * spider_size * sin(PI/5));
	glVertex2f(-1.5 * spider_size * cos(PI/5),1.5 * spider_size * sin(PI/5));
	glVertex2f(-1.5 * spider_size * cos(PI/5),1.5 * spider_size * sin(PI/5) + spider_size * 3 / 2);

	glVertex2f(spider_size * cos(PI/5),spider_size * sin(PI/5));
	glVertex2f(1.5 * spider_size * cos(PI/5),1.5 * spider_size * sin(PI/5));
	glVertex2f(1.5 * spider_size * cos(PI/5),1.5 * spider_size * sin(PI/5));
	glVertex2f(1.5 * spider_size * cos(PI/5),1.5 * spider_size * sin(PI/5) + spider_size * 3 / 2);

	glVertex2f(spider_size * cos(PI/4),-spider_size * sin(PI/4));
	glVertex2f(1.5 * spider_size * cos(PI/4),-1.5 * spider_size * sin(PI/4));
	glVertex2f(1.5 * spider_size * cos(PI/4),-1.5 * spider_size * sin(PI/4));
	glVertex2f(1.5 * spider_size * cos(PI/4),-1.5 * spider_size * sin(PI/4) - spider_size);

	glVertex2f(-spider_size * cos(PI/4),-spider_size * sin(PI/4));
	glVertex2f(-1.5 * spider_size * cos(PI/4),-1.5 * spider_size * sin(PI/4));
	glVertex2f(-1.5 * spider_size * cos(PI/4),-1.5 * spider_size * sin(PI/4));
	glVertex2f(-1.5 * spider_size * cos(PI/4),-1.5 * spider_size * sin(PI/4) - spider_size);

	glVertex2f(-spider_size,0.2 * spider_size);
	glVertex2f(-1.5 * spider_size,0.2 * spider_size);
	glVertex2f(-1.5 * spider_size,0.2 * spider_size);
	glVertex2f(-1.8 * spider_size,0.8 * spider_size);

	glVertex2f(-spider_size,-0.2 * spider_size);
	glVertex2f(-1.5 * spider_size,-0.2 * spider_size);
	glVertex2f(-1.5 * spider_size,-0.2 * spider_size);
	glVertex2f(-1.8 * spider_size,-0.8 * spider_size);

	glVertex2f(spider_size,0.2 * spider_size);
	glVertex2f(1.5 * spider_size,0.2 * spider_size);
	glVertex2f(1.5 * spider_size,0.2 * spider_size);
	glVertex2f(1.8 * spider_size,0.8 * spider_size);

	glVertex2f(spider_size,-0.2 * spider_size);
	glVertex2f(1.5 * spider_size,-0.2 * spider_size);
	glVertex2f(1.5 * spider_size,-0.2 * spider_size);
	glVertex2f(1.8 * spider_size,-0.8 * spider_size);

	glEnd();
	glTranslatef(0.0f, - spider_size, 0.0f);
	drawBall(spider_size / 2);
}

void print_score() {
	char text[32];
	sprintf(text, "Score %d", score);
	glColor3f(0, 0, 1);
	glRasterPos3f(box_len/ 10 * 3 , box_len / 10 * 4 , 0);
	for(int i = 0; text[i] != '\0'; i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}
void drawCannon() {

	if(move_component == 'b')
		glColor3f(200.0/255,200.0/255,0);
	else
		glColor3f(1,1,0);
	drawBall(cannon_size);
	glTranslatef(0,cannon_size,0);

	glBegin(GL_QUADS);
	if(laser_ready == 1)
		glColor3f(1,1,0);
	else
		glColor3f(0,0,0);
	drawBox(cannon_size/2);
	glEnd();
	glTranslatef(0,cannon_size/2,0);

	glBegin(GL_QUADS);
	if(laser_ready == 1)
		glColor3f(1,1,0);
	else
		glColor3f(0,0,0);
	drawBox(cannon_size/2);
	glEnd();
	glTranslatef(0,cannon_size/2,0);

	glBegin(GL_QUADS);
	if(laser_ready == 1)
		glColor3f(1,1,0);
	else
		glColor3f(0,0,0);
	drawBox(cannon_size/2);
	glEnd();
	/*glTranslatef(0,-2*cannon_size,0);

	  glBegin(GL_LINES);
	  glEnable(GL_LINE_SMOOTH);
	  glLineWidth(600);
	  glColor3f(0,0,1);
	  glVertex2f(cannon_size,0);
	  glVertex2f(1.5*cannon_size,0);
	  glEnd();*/
}

void drawBox(float len) {

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glVertex2f(-len / 2, -len / 2);
	glVertex2f(len / 2, -len / 2);
	glVertex2f(len / 2, len / 2);
	glVertex2f(-len / 2, len / 2);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void drawBall(float rad) {

	glBegin(GL_TRIANGLE_FAN);
	for(int i=0 ; i<360 ; i++) {
		glVertex2f(rad * cos(DEG2RAD(i)), rad * sin(DEG2RAD(i)));
	}
	glEnd();
}

void drawTriangle() {

	glBegin(GL_TRIANGLES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, tri_len, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-tri_len, -tri_len, 0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(tri_len, -tri_len, 0.0f);
	glEnd();
}

// Initializing some openGL 3D rendering options
void initRendering() {

	glEnable(GL_DEPTH_TEST);        		// Enable objects to be drawn ahead/behind one another
	glEnable(GL_COLOR_MATERIAL);    		// Enable coloring
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // Setting a background color
}

// Function called when the window is resized
void handleResize(int w, int h) {

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float)w / (float)h, 0.1f, 200.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void laser_init() {
	if(game_start == 1) {
		if(sounds == 1)
			system("aplay LASER.WAV &");
		laser_ready = 0;
		lasers[num_lasers].flag = 1;		//Spacebar is pressed
		lasers[num_lasers].y = tri_y;
		lasers[num_lasers].x = tri_x; 
		lasers[num_lasers].theta = PI / 2 - DEG2RAD(-theta);
		lasers[num_lasers].velx = laser_vel * cos(PI / 2 - DEG2RAD(-theta));
		lasers[num_lasers].vely = laser_vel * sin(PI / 2 - DEG2RAD(-theta)); 
	}
}

void handleKeypress1(unsigned char key, int x, int y) {

	if(key == 13) {
		game_start = 1;
		update(0);
		//drawScene();
	}

	if (key == 27 || key == 'q' || key == 'Q') {    // escape key or 'q' is pressed
		if(game_over == 1)
			exit(0);
		else {
			game_over = 1;
			drawScene();
		}
	}

	if (key == 32 && laser_ready == 1) {
		laser_init();
		num_lasers ++;
	}
	if(key == 'p' || key == 'P') {
		pause_toggle();
		update(0);
	}
	if (key == 'r' || key == 'R') {
		move_component = 'r';     				// r or R is pressed
	}
	if (key == 'g' || key == 'G') {
		move_component = 'g';     				// g or G is pressed
	}
	if (key == 'b' || key == 'B') {
		move_component = 'b';     				// b or B is pressed
	}
}

void pause_toggle() {
	if(pause == 1)
		pause = 0; 
	else 
		pause = 1;
}

float move_checker(float pos_x,int direction) {				//direction = 1 corresponds to right and direction =  -1 corresponds to left
	float min;
	if(direction == 1)
		min = box_len / 2;
	else
		min = -box_len / 2;
	for(int i=0;i<num_spiders;i++) {
		if(spiders[i].flag == 1 && spiders[i].y <= box_len/40-box_len/2) {
			if(direction == 1) {
				if(spiders[i].x > pos_x && spiders[i].x < min)
					min = spiders[i].x;
			}
			else {
				if(spiders[i].x < pos_x && spiders[i].x > min)
					min = spiders[i].x;
			}
		}
	}
	return min;
}

void handleKeypress2(int key, int x, int y) {

	if(move_component == 'b') {
		if (key == GLUT_KEY_LEFT && tri_x - move_checker(tri_x,-1) >= box_len / 20)//tri_x > - box_len / 2)
			tri_x -= 0.1;
		if (key == GLUT_KEY_RIGHT && move_checker(tri_x,1) - tri_x >= box_len / 20)
			tri_x += 0.1;
	}
	if(move_component == 'r') {
		if (key == GLUT_KEY_LEFT && red_x - move_checker(red_x,-1) >= box_len / 20)
			red_x -= 0.1;
		if (key == GLUT_KEY_RIGHT && move_checker(red_x,1) - red_x >= box_len / 20)
			red_x += 0.1;
	}
	if(move_component == 'g') {
		if (key == GLUT_KEY_LEFT && green_x - move_checker(green_x,-1) >= box_len / 20)
			green_x -= 0.1;
		if (key == GLUT_KEY_RIGHT && move_checker(green_x,1) - green_x >= box_len / 20)
			green_x += 0.1;
	}
	if (key == GLUT_KEY_UP)
		if(theta < 75)
			theta += 15;
	if (key == GLUT_KEY_DOWN)
		if(theta > -75)
			theta -= 15;
}

void handleMouseclick(int button, int state, int x, int y) {

	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON) {
			glutMotionFunc(handleLeftMouseDrag);
			if(theta < 75)
				theta += 15;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			glutMotionFunc(handleRightMouseDrag);
			if(theta > -75)
				theta -= 15;
		}
	}
}

void handleLeftMouseDrag(int x, int y) { 

	float newx = x - (60 + (((tri_x+box_len/2)/box_len)*340));
	float newredx = x - (60 + (((red_x+box_len/2)/box_len)*340));
	float newgreenx = x - (60 + (((green_x+box_len/2)/box_len)*340));

	if(abs(newx) < abs(newredx) && abs(newx) < abs(newgreenx))
		move_component = 'b';
	if(abs(newx) > abs(newredx) && abs(newredx) < abs(newgreenx))
		move_component = 'r';
	if(abs(newgreenx) < abs(newredx) && abs(newx) > abs(newgreenx))
		move_component = 'g';

	//Move the cannon with the mouse
	if(move_component == 'b') {
		if(newx <= (60+((cannon_size+box_len/2)/box_len*340))  && newx >= 0 
				&& move_checker(tri_x,1) >= (((float)x-60)*box_len/340)-box_len/2) 
			tri_x = (((float)x-60)*box_len/340)-box_len/2;

		else if(newx >= -(60+((cannon_size+box_len/2)/box_len*340))  && newx <= 0 
				&& move_checker(tri_x,-1) <= (((float)x-60)*box_len/340)-box_len/2) 
			tri_x = (((float)x-60)*box_len/340)-box_len/2;
	}

	//Move the red box with the mouse
	else if(move_component == 'r') {
		if(newredx <= (60+((box_len/10+box_len/2)/box_len*340))  && newredx >= 0 
				&& move_checker(red_x,1) >= (((float)x-60)*box_len/340)-box_len/2) 
			red_x = (((float)x-60)*box_len/340)-box_len/2;

		else if(newredx >= -(60+((box_len/10+box_len/2)/box_len*340))  && newredx <= 0 
				&& move_checker(red_x,-1) <= (((float)x-60)*box_len/340)-box_len/2) 
			red_x = (((float)x-60)*box_len/340)-box_len/2;
	}

	//Move the green box with the mouse
	else if(move_component == 'g') {
		if(newgreenx <= (60+((box_len/10+box_len/2)/box_len*340))  && newgreenx >= 0 
				&& move_checker(green_x,1) >= (((float)x-60)*box_len/340)-box_len/2) 
			green_x = (((float)x-60)*box_len/340)-box_len/2;

		else if(newgreenx >= -(60+((box_len/10+box_len/2)/box_len*340))  && newgreenx <= 0 
				&& move_checker(green_x,-1) <= (((float)x-60)*box_len/340)-box_len/2) 
			green_x = (((float)x-60)*box_len/340)-box_len/2;
	}
}

void handleRightMouseDrag(int x,int y) {
	x -= (60 + (((tri_x+box_len/2)/box_len)*340));
	if(x <=0) 
		theta = 90 - RAD2DEG(atan(-(y-tri_y)/(x)));
	else
		theta = 270 - RAD2DEG(atan(-(y-tri_y)/(x)));
}
