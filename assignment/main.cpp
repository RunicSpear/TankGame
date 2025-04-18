// Includes
#include <GL/glew.h>
#include <GL/glut.h>
#include <Shader.h>
#include <Vector.h>
#include <Matrix.h>
#include <Mesh.h>
#include <Texture.h>
#include <SphericalCameraManipulator.h>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>


// Function Prototypes
bool initGL(int argc, char** argv);
void initShader();
void initGeometry();  //Function to init Geometry 
void drawGeometry();		
void display(void);
void keyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void specialKeyboard(int key, int x, int y);
void specialKeyUp(int key, int x, int y);
void handleKeys();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void Timer(int value);
void initTexture(std::string filename, GLuint & textureID);
void reshape(int width, int height);
void loadMaze(const std::string& filename, int level);
void DrawMaze();
void DrawTank(float x, float y, float z);
void render2dText(std::string text, float r, float g, float b, float x, float y);
		
				     
// Screen size
int screenWidth   	        = 1080;
int screenHeight   	        = 1080;


// Global Variables
float specularPower = 10.0;
float coinRotationAngle = 0.0f;
float coinBounce = 0.0f;
int currentLevel = 1;
const int MAZE_WIDTH = 15;
const int MAZE_HEIGHT = 15;
float centerX = (MAZE_WIDTH - 3) * 2.0f / 2;
float centerZ = (MAZE_HEIGHT - 1) * 2.0f / 2;
int MAZE[MAZE_HEIGHT][MAZE_WIDTH]; 
float tankX = 0.0f;
float tankZ = 0.0f;
float tankRotation = 0.0f;
float moveSpeed = 0.1f;
float rotationSpeed = 1.0f;
float g = 9.81;

bool isJumping = false;
float jumpHeight = 0.0f;
float maxJumpHeight = 1.0f; //1 block high
float jumpSpeed = 0.1f;
bool maxJump = false;

//Timer Variables
float remainingTime = 200.0f;

Vector3f tankPosition(centerX, 0.0f, centerZ);
Vector3f tankVelocity(0.0f, 0.0f, 0.0f);


GLuint shaderProgramID;
GLuint vertexPositionAttribute;		                       // Vertex Position Attribute Location
GLuint vertexNormalAttribute;		
GLuint vertexTexcoordAttribute;		                       // Vertex Texcoord Attribute Location


//Material Properties
GLuint LightPositionUniformLocation;                           // Light Position Uniform   
GLuint AmbientUniformLocation;
GLuint SpecularUniformLocation;
GLuint SpecularPowerUniformLocation;


//Lighting
Vector3f lightPosition = Vector3f(20.0,20.0,20.0);             
Vector3f ambient    = Vector3f(0.1,0.1,0.1);
Vector3f specular   = Vector3f(1.0,0.5,0.0);


//Viewing
SphericalCameraManipulator cameraManip;
Matrix4x4 ModelViewMatrix;		                     // ModelView Matrix
GLuint MVMatrixUniformLocation;		                     // ModelView Matrix Uniform
Matrix4x4 ProjectionMatrix;		                     // Projection Matrix
GLuint ProjectionUniformLocation;	                     // Projection Matrix Uniform Location

GLuint TextureMapUniformLocation;               	      // Texture Map Location

//Gluint Textures
GLuint crateTexture;				                  
GLuint coinTexture;
GLuint tankTexture;



//Mesh
Mesh crateMesh;			// Crate Mesh		                     
Mesh coinMesh;			// Coin Mesh

//Tank Mesh
Mesh chassisMesh;		// Chassis Mesh
Mesh turretMesh;		// Turret Mesh
Mesh frontWheelMesh;		// Front Wheel Mesh
Mesh backWheelMesh;		// Back Wheel Mesh


float maxSpeed = 1.0f;
float acceleration = 3.0f;
float friction = 3.0f;


//! Array of key states
bool keyStates[256];

	

void loadMaze(const std::string& filename, int level)
{	
	std::ifstream file(filename);
	if(!file)
	{
		std::cerr << "Error: could not open maze file: " << filename << std::endl;
		return;
	}
	
	int startLine = (level - 1) * (MAZE_HEIGHT + 3) + 2;
	
	std::string line;
	int currentLine = 0;
	
	while (currentLine < startLine && std::getline(file, line))
	{
		currentLine++;
	}
	
	
	
	
	for (int i = 0; i < MAZE_HEIGHT; i++)
	{
		for (int j = 0; j < MAZE_WIDTH; j++)
		{
			file >> MAZE[i][j];
		}
	}
	
	file.close();
}

void switchLevel(int direction)
{
	currentLevel += direction;
	
	if (currentLevel < 1) {
		currentLevel = 2;
	}	
	if (currentLevel > 2) {
		currentLevel = 1;
	}
	
	loadMaze("maze.txt", currentLevel);
	std::cout << "Switched to level" << currentLevel << std::endl;
}	
			
				

void reshape(int width, int height)
{
	screenWidth = width;
	screenHeight = height;
	
	// Update OpenGL viewport
	glViewport(0, 0, screenWidth, screenHeight);
	
	// Update Projection Matrix
	ProjectionMatrix.perspective(90, (float)screenWidth / screenHeight, 0.0001, 100.0);
	glUniformMatrix4fv(ProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());
}	


//! Main Program Entry
int main(int argc, char** argv)
{	
	
	//Load Maze
	loadMaze("maze.txt", currentLevel);
	
	//init OpenGL
	if(!initGL(argc, argv))
		return -1;
		

	//Init OpenGL Shader
	initShader();

	//Init Key States to false;    
	for(int i = 0 ; i < 256; i++)
		keyStates[i] = false;

	//Init Mesh Geometry
	//Crate 
	crateMesh.loadOBJ("../models/cube.obj");    
	initTexture("../models/stone.bmp", crateTexture);
	
	//Coin
	coinMesh.loadOBJ("../models/coin.obj");
	initTexture("../models/coin.bmp", coinTexture);
	
	//Tank
	chassisMesh.loadOBJ("../models/pikachu.obj");
	turretMesh.loadOBJ("../models/turret.obj");
	frontWheelMesh.loadOBJ("../models/front_wheel.obj");
	backWheelMesh.loadOBJ("../models/back_wheel.obj");
	initTexture("../models/hamvee.bmp", tankTexture);

	//Init Camera Manipultor
	cameraManip.setPanTiltRadius(0.0f, 0.0f,20.f);
	cameraManip.setFocus(Vector3f(MAZE_WIDTH, 0.0f, MAZE_HEIGHT));

	//Start main loop
	glutMainLoop();

	//Clean-Up
	glDeleteProgram(shaderProgramID);

	return 0;
}

// Function to Initlise OpenGL
bool initGL(int argc, char** argv)
{
	//Init GLUT
	glutInit(&argc, argv);

	//Set Display Mode
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	//Set Window Size
	glutInitWindowSize(screenWidth, screenHeight);

	// Window Position
	glutInitWindowPosition(200, 200);

	//Create Window
	glutCreateWindow("Tank Assignment");

	// Init GLEW
	if (glewInit() != GLEW_OK) 
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	//Set Display function
	glutDisplayFunc(display);
	
	glutReshapeFunc(reshape);

	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyUp); 

	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	//Start start timer function after 100 milliseconds
	glutTimerFunc(100,Timer, 0);

	return true;
}

//Init Shader
void initShader()
{
	//Create shader
	shaderProgramID = Shader::LoadFromFile("shader.vert","shader.frag");

	// Get a handle for our vertex position buffer
	vertexPositionAttribute = glGetAttribLocation(shaderProgramID,  "aVertexPosition");
	vertexNormalAttribute = glGetAttribLocation(shaderProgramID,    "aVertexNormal");
	vertexTexcoordAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");

	//!
	MVMatrixUniformLocation         = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform"); 
	ProjectionUniformLocation       = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform"); 
	LightPositionUniformLocation    = glGetUniformLocation(shaderProgramID, "LightPosition_uniform"); 
	AmbientUniformLocation          = glGetUniformLocation(shaderProgramID, "Ambient_uniform"); 
	SpecularUniformLocation         = glGetUniformLocation(shaderProgramID, "Specular_uniform"); 
	SpecularPowerUniformLocation    = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");
	TextureMapUniformLocation       = glGetUniformLocation(shaderProgramID, "TextureMap_uniform"); 
}

void initTexture(std::string filename, GLuint & textureID)
{

	
	//Generate texture and bind
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 


	//Get texture Data
	int width, height;
	char* data;
	Texture::LoadBMP(filename, width, height, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Cleanup data - copied to GPU
	delete[] data;
}




//! Display Loop
void display(void)
{
	
	//Handle keys
	handleKeys();

	//Set Viewport
	glViewport(0,0,screenWidth, screenHeight);

	// Clear the screen
	glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	//Use shader
	glUseProgram(shaderProgramID);

	//Projection Matrix - Perspective Projection
	ProjectionMatrix.perspective(90, 1.0, 0.0001, 100.0);

	//Set Projection Matrix
	glUniformMatrix4fv(	
		ProjectionUniformLocation,  //Uniform location
		1,							//Number of Uniforms
		false,						//Transpose Matrix
		ProjectionMatrix.getPtr());	//Pointer to ModelViewMatrixValues

	
	glUniform3f(LightPositionUniformLocation, lightPosition.x,lightPosition.y,lightPosition.z);
	glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
	glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
	glUniform1f(SpecularPowerUniformLocation, specularPower);

	DrawMaze();
	
	DrawTank(0.0f, 0.65f, 0.0f);
	
	//Unuse Shader
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_DEPTH_TEST);
	
	// Set up orthogonal projection for 2D text
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, 0, screenHeight);

	// Switch back to modelview matrix for 2D rendering
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	// Render remaining time on the screen
	std::string timeText = "Time: " + std::to_string(remainingTime);
	render2dText(timeText, 1.0f, 1.0f, 1.0f, screenWidth / 2.0f - 55.0f, screenHeight - 30.0f);
	
	//Render Text
	render2dText("Level: 1", 1.0f, 1.0f, 1.0f, 10.0f, screenHeight - 30.0f);
	
	
	glEnable(GL_DEPTH_TEST);
	
        // Restore previous matrix state
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);	
        glPopMatrix();


	//Redraw frame
	glutPostRedisplay();
	glutSwapBuffers();
}


void DrawMaze()
{


	for(int i=0; i<MAZE_HEIGHT; i++)
	{
	
		for( int j=0; j<MAZE_WIDTH; j++)
		{
		
			
			if(MAZE[i][j] >= 1)
			{
			
				//Apply Camera Manipluator to Set Model View Matrix on GPU
				ModelViewMatrix.toIdentity();
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				glUniformMatrix4fv(	
					MVMatrixUniformLocation,  	//Uniform location
					1,					        //Number of Uniforms
					false,				        //Transpose Matrix
					m.getPtr());	        //Pointer to Matrix Values
			

				//Set Colour after program is in use
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, crateTexture);
				glUniform1i(TextureMapUniformLocation, 0);

				// First crate
				m.translate(i*2.0, 0.0f, j*2.0);  
				glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
				crateMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			}
			
		        if(MAZE[i][j] == 2)
			{
				
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				
			

				//Set Colour after program is in use
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, coinTexture);
				glUniform1i(TextureMapUniformLocation, 0);
				
				float bounceHeight = 0.1f * sin(coinBounce);
				m.translate(i*2.0, 2.0f + bounceHeight, j*2.0);  
				m.scale(0.5f, 0.5f, 0.5f);
				m.rotate(coinRotationAngle, 0.0f, 1.0f, 0.0f);
				glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
				coinMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			
			}

		}
	
	}

}

void DrawTank(float x, float y, float z) {
	
	Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
	
	
	
	m.translate(tankPosition.x, y + jumpHeight, tankPosition.z);
	m.rotate(tankRotation, 0.0f, 1.0f, 0.0f);
	m.scale(0.1f, 0.1f, 0.1f); 
	

	//Bind the tank 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tankTexture);
	glUniform1i(TextureMapUniformLocation, 0);
	
	// Draw Chassis
	m.translate(x, y, z);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
	chassisMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
	
	// Draw Turret
	Matrix4x4 turretMatrix = m;
	turretMatrix.translate(0.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, turretMatrix.getPtr());
	turretMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
	
	// Draw Front Wheels
	Matrix4x4 frontWheelMatrix = m;
	frontWheelMatrix.translate(0.0f, 0.1f, 0.0f);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, frontWheelMatrix.getPtr());
	frontWheelMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
	
	// Draw Back Wheels
	Matrix4x4 backWheelMatrix = m;
	backWheelMatrix.translate(0.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, backWheelMatrix.getPtr());
	backWheelMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}




//! Keyboard Interaction
void keyboard(unsigned char key, int x, int y)
{
	//Quits program when esc is pressed
	if (key == 27)	//esc key code
	{
		exit(0);
	}
	
	if (key == 'n' || key == 'N')
	{
		switchLevel(1);
	}
	
	if (key == 'p' || key == 'P')
	{
		switchLevel(-1);
	}	
	

    //Set key status
    keyStates[key] = true;  
  
}

//! Handle key up situation
void keyUp(unsigned char key, int x, int y)
{
    keyStates[key] = false;
}


//! Handle Keys
void handleKeys()
{

	float deltaTime = 0.016f;

	float radians = tankRotation * M_PI / 180.0f;
	Vector3f direction(sin(radians), 0.0f, cos(radians));
	
	 
	//keys should be handled here
	if(keyStates['w'])
    	{
        	tankVelocity = tankVelocity + direction * acceleration * deltaTime;
    	}
    	
    	if(keyStates['s'])
    	{
        	tankVelocity = tankVelocity - direction * acceleration * deltaTime;
    	}
    	
    	if (!keyStates['w'] && !keyStates['s'])
    	{
    		tankVelocity = tankVelocity - tankVelocity * friction * deltaTime;
	}
	
	if (tankVelocity.length() > maxSpeed)
	{
		tankVelocity = tankVelocity - tankVelocity * friction * deltaTime;
	}	
    	
    	
    	tankPosition.x += tankVelocity.x * deltaTime;
    	tankPosition.y += tankVelocity.y * deltaTime;
    	tankPosition.z += tankVelocity.z * deltaTime;

    	if(keyStates['a'])
    	{
    		tankRotation += rotationSpeed;
    	}
    	
    	if(keyStates['d'])
    	{
    		tankRotation -= rotationSpeed;
    	}  
    	
    	
    	if (keyStates[' '])
    	{
    		isJumping = true;
    		jumpHeight = 0.0f;
    	}	
    	
    	if (isJumping) 
    	{
    		if (jumpHeight < maxJumpHeight) 
    		{	
    			jumpHeight += jumpSpeed * deltaTime;
    		} else {
    			isJumping = false;	
 		}
 	}		
   
	cameraManip.setFocus(tankPosition);
	//cameraManip.setPanTiltRadius(tankRotation, 30.0f, 20.0f);
	
}


// Mouse Interaction
void mouse(int button, int state, int x, int y)
{
    cameraManip.handleMouse(button, state,x,y);
    glutPostRedisplay(); 
}

// Mouse Interaction
void motion(int x, int y)
{
    cameraManip.handleMouseMotion(x,y);
    glutPostRedisplay(); 
}

void specialKeyboard(int key, int x, int y) 
{
	keyStates[key] = true;
}

void specialKeyUp(int key, int x, int y)
{
	keyStates[key] = false;	
}

//! Timer Function
void Timer(int value)
{

	remainingTime -= 0.01f;	
	
	//Check if the time is up
	if (remainingTime < 0)
	{
		remainingTime = 0;
	}
	
	//if (timeRemaining == 0)
	//{
	//	std:cout <<"Time's up!" << std::endl;
	//}	
		
	// Update the coin rotation and bounce
	coinRotationAngle += 1.0f;
	
	if(coinRotationAngle >= 360.0f)
	{
    		coinRotationAngle -= 360.0f;
    	}
    	
    	coinBounce += 0.1f;
    
	// Redisplay the scene    
	glutPostRedisplay();
		
	//Call function again after 10 milli seconds
	glutTimerFunc(10,Timer, 0);
}

void render2dText(std::string text, float r, float g, float b, float x, float y) 
{
	glColor3f(r, g, b); //Set text Colour
	glRasterPos2f(x, y); // Set position in window coordinates
	
	for (unsigned int i = 0; i < text.size(); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
	}
}	


