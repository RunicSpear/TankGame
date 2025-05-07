/*=================================================================================================================================*/
/*-------------------------------------------------------// Start //---------------------------------------------------------------*/
/*=================================================================================================================================*/
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
#include <map>

/*---------------------------------------------------// Function Prototypes //-----------------------------------------------------*/
// Function Prototypes

// Initialization
bool initGL(int argc, char **argv);
void initShader();
void initTexture(std::string filename, GLuint &textureID);
void loadMaze(const std::string &filename, int level);
void resetGame();

// Input Handling
void keyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void specialKeyboard(int key, int x, int y);
void specialKeyUp(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void handleKeys();

// Timer
void Timer(int value);

// Updates (Game Logic)
void updateTankMovement(Vector3f &tankVelocity, float &tankAngle);
void updateCameraPosition();
void updateTurretRotation();
void updateSteeringAngle(float deltaTime);
void updateBallPosition();
void updateParticles(float deltaTime);
void checkfall();
void fireBall();

// Rendering
void display(void);
void reshape(int width, int height);
void DrawMaze();
void DrawTank(float x, float y, float z);
void DrawBall(float x, float y, float z);
void drawParticles();
void drawHUD();
void render2dText(std::string text, float r, float g, float b, float x, float y);

// Screen size
int screenWidth = 1080;
int screenHeight = 1080;

/*----------------------------------------------------// Global Variables //-------------------------------------------------------*/

// Environment / Game State
float g = -9.81f;
float specularPower = 10.0f;
float remainingTime = 200.0f;
float flashAlpha = 1.0f;
float brightness = 1.0f;

bool flashIncreasing = false;
bool warningPlayed = false;
bool isPaused = false;
bool showMenu = false;
bool mainMenu = true;
bool gameWon = false;
bool isGameOver = false;
bool LowTimeWarning = false;
bool levelComplete = false;

const int finalLevel = 3;
int currentLevel = 1;
int selectedLevel = currentLevel;
bool levelCompleted[3] = {false, false, false};

// Maze System
const int MAZE_WIDTH = 15;
const int MAZE_HEIGHT = 15;
int MAZE[MAZE_HEIGHT][MAZE_WIDTH];

float centerX = (MAZE_WIDTH - 1);
float centerZ = (MAZE_HEIGHT - 1);

// Coin System
int coinsCollected = 0;
int totalCoins = 0;

float coinRotationAngle = 0.0f;
float coinBounce = 0.0f;

// Tank State
Vector3f tankPosition(centerX, 0.0f, centerZ);
Vector3f tankVelocity(0.0f, 0.0f, 0.0f);

float tankRotation = 0.0f;
float moveSpeed = 2.0f;
float rotationSpeed = 1.0f;

// Tank Movement and Physics
float deltaTime = 0.016f;

float maxSpeed = 10.0f;
float friction = 3.0f;

float moveDirection = 0.0f;
float turnDirection = 0.0f;

float wheelRotation = 0.0f;
float wheelRadius = 0.001f;

float steeringAngle = 0.0f;
float steeringSpeed = 10.0f;
float maxSteeringAngle = 30.0f;

float verticalVelocity = 0.0f;
float fallRotation = 0.0f;
float fallSpeed = 140.0f;
bool isfalling = false;
bool fallSoundPlayed = false;

// Jumping
bool isJumping = false;
bool isOnGround = true;

float jumpVelocity = 0.0f;
const float initialJumpVelocity = 5.0f;

// Camera
float cameraDistance = 7.0f;
float cameraPan = 0.0f;

bool isFirstPerson = false;
bool aiming = false;

// Turret
float turretBaseRotation = 0.0f;
float targetTurretRotation = 0.0f;

// Ball
float ballPosX = 0.0f;
float ballPosY = 6.0f;
float ballPosZ = 0.0f;

float ballRotationAngle = 0.0f;
bool ballActive = true;
bool isBallFired = false;

float ballLifeTime = 0.0f; // Time since ball was fired
float gravityDelay = 0.2f; // Delay before gravity affects the ball
Vector3f ballDirection;

GLuint shaderProgramID;
GLuint vertexPositionAttribute; // Vertex Position Attribute Location
GLuint vertexNormalAttribute;
GLuint vertexTexcoordAttribute; // Vertex Texcoord Attribute Location

// Material Properties
GLuint LightPositionUniformLocation; // Light Position Uniform
GLuint AmbientUniformLocation;
GLuint SpecularUniformLocation;
GLuint SpecularPowerUniformLocation;

// Lighting
Vector3f lightPosition = Vector3f(20.0, 20.0, 20.0);
Vector3f ambient = Vector3f(0.1, 0.1, 0.1);
Vector3f specular = Vector3f(1.0, 0.5, 0.0);

// Viewing
SphericalCameraManipulator cameraManip;
Matrix4x4 ModelViewMatrix;		  // ModelView Matrix
GLuint MVMatrixUniformLocation;	  // ModelView Matrix Uniform
Matrix4x4 ProjectionMatrix;		  // Projection Matrix
GLuint ProjectionUniformLocation; // Projection Matrix Uniform Location
GLuint TextureMapUniformLocation; // Texture Map Location

// Gluint Textures
GLuint crateTexture;
GLuint coinTexture;
GLuint tankTexture;
GLuint ballTexture;
GLuint donutTexture;
GLuint shadowTexture;

// Component Meshes
Mesh crateMesh; // Crate Mesh
Mesh coinMesh;	// Coin Mesh
Mesh donutMesh; // Donut Mesh
Mesh shadowMesh;

// Tank Mesh
Mesh chassisMesh;	 // Chassis Mesh
Mesh turretMesh;	 // Turret Mesh
Mesh frontWheelMesh; // Front Wheel Mesh
Mesh backWheelMesh;	 // Back Wheel Mesh

// Ball Mesh
Mesh ballMesh;

// Array of key states
bool keyStates[256];

std::map<std::pair<int, int>, int> donutFallTimers;

struct Particle
{
	Vector3f position;
	Vector3f velocity;
	float life;
};

const int MAX_PARTICLES = 100;
std::vector<Particle> particles;
bool spawnParticles = false;
Vector3f particleOrigin;

// Main Program Entry
int main(int argc, char **argv)
{
	// Load initial maze layout from file for the current level
	loadMaze("maze.txt", currentLevel);

	// init OpenGL
	if (!initGL(argc, argv))
		return -1;

	// Init OpenGL Shader
	initShader();

	// Init Key States to false;
	for (int i = 0; i < 256; i++)
		keyStates[i] = false;

	/*-----------------------------------------// Load Meshes and Textures //---------------------------*/
	// Load crate model and texture
	crateMesh.loadOBJ("../models/cube.obj");
	initTexture("../models/brick.bmp", crateTexture);

	// Load coin model and texture
	coinMesh.loadOBJ("../models/cube.obj");
	initTexture("../models/block.bmp", coinTexture);
	shadowMesh.loadOBJ("../models/cube.obj");
	initTexture("../models/shadow.bmp", shadowTexture);

	// Load tank components and apply tank textures
	chassisMesh.loadOBJ("../models/chassis.obj");
	turretMesh.loadOBJ("../models/turret.obj");
	frontWheelMesh.loadOBJ("../models/front_wheel.obj");
	backWheelMesh.loadOBJ("../models/back_wheel.obj");
	initTexture("../models/hamvee.bmp", tankTexture);

	// Load ball model and texture
	ballMesh.loadOBJ("../models/ball.obj");
	initTexture("../models/ball.bmp", ballTexture);

	// Load falling block model and texture
	donutMesh.loadOBJ("../models/cube.obj");
	initTexture("../models/donut.bmp", donutTexture);

	// Start main loop
	glutMainLoop();

	// Clean-Up
	glDeleteProgram(shaderProgramID);

	return 0;
}

// Function to Initialise OpenGL
bool initGL(int argc, char **argv)
{
	// Init GLUT
	glutInit(&argc, argv);

	// Set Display Mode
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	// Set Window Size
	glutInitWindowSize(screenWidth, screenHeight);

	// Window Position
	glutInitWindowPosition(200, 200);

	// Create Window
	glutCreateWindow("Andreas Tank Game :)");

	// Init GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	// Set Display function
	glutDisplayFunc(display);

	glutReshapeFunc(reshape);

	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyUp);

	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// Start start timer function after 100 milliseconds
	glutTimerFunc(100, Timer, 0);

	return true;
}

// Init Shader
void initShader()
{
	// Create shader
	shaderProgramID = Shader::LoadFromFile("shader.vert", "shader.frag");

	// Get a handle for our vertex position buffer
	vertexPositionAttribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");
	vertexNormalAttribute = glGetAttribLocation(shaderProgramID, "aVertexNormal");
	vertexTexcoordAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");

	MVMatrixUniformLocation = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform");
	ProjectionUniformLocation = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform");
	LightPositionUniformLocation = glGetUniformLocation(shaderProgramID, "LightPosition_uniform");
	AmbientUniformLocation = glGetUniformLocation(shaderProgramID, "Ambient_uniform");
	SpecularUniformLocation = glGetUniformLocation(shaderProgramID, "Specular_uniform");
	SpecularPowerUniformLocation = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");
	TextureMapUniformLocation = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
}

void initTexture(std::string filename, GLuint &textureID)
{
	// Generate texture and bind
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Get texture Data
	int width, height;
	char *data;
	Texture::LoadBMP(filename, width, height, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Cleanup data - copied to GPU
	delete[] data;
}

// Function to read through a text file to load the maze
void loadMaze(const std::string &filename, int level)
{
	std::ifstream file(filename); // Open the maze file for reading
	if (!file)
	{
		std::cerr << "Error: could not open maze file: " << filename << std::endl;
		return;
	}

	std::string line;
	bool levelFound = false;
	std::string targetLevel = "LEVEL " + std::to_string(level); // Construct the LEVEL header string (e.g. "LEVEL 1")

	// Search for the LEVEL header line in the file
	while (std::getline(file, line))
	{
		if (line.find(targetLevel) != std::string::npos)
		{
			levelFound = true;
			break;
		}
	}

	// If the specified level is not found, display error and exit function
	if (!levelFound)
	{
		std::cerr << "Error: Level " << level << " not found in file." << std::endl;
		return;
	}

	totalCoins = 0; // Reset coin count for the new level

	// Clear current maze (initialise all cells to 0)
	for (int i = 0; i < MAZE_HEIGHT; ++i)
		for (int j = 0; j < MAZE_WIDTH; ++j)
			MAZE[i][j] = 0;

	// Read the maze layout after the LEVEL header
	for (int i = 0; i < MAZE_HEIGHT && std::getline(file, line); ++i)
	{
		std::istringstream iss(line); // Use stringstream to parse each line
		for (int j = 0; j < MAZE_WIDTH; ++j)
		{
			iss >> MAZE[i][j]; // Read integer into the maze cell

			if (MAZE[i][j] == 2)
			{
				totalCoins++; // Count coin tiles
			}
		}
	}

	file.close(); // Close the file after reading

	// Print confirmation with the number of coins found
	std::cout << "Level " << level << " loaded with " << totalCoins << " coins." << std::endl;
}

/*------------------------------------------------------// Switching Levels Function //--------------------------------------------*/
void switchLevel(int direction)
{
		// Trigger win state if player advances past the final level
		if (currentLevel == finalLevel)
		{
			gameWon = true;
			levelComplete = false;
			std::cout << "Game Won" << std::endl;
			system("canberra-gtk-play -f smb_world_clear.wav &"); // Play win sound
			return;
		}

		// Increment or decrement the current level
		currentLevel += direction;

		// Wrap around to the last level if going below level 1
		if (currentLevel < 1)
		{
			currentLevel = 3;
		}

	// Reset Coin counter for new level
	coinsCollected = 0;

	// Reset tank position to the center of the maze
	tankPosition.x = centerX;
	tankPosition.z = centerZ;
	tankPosition.y = 0.0f;

	// Reset Time Remaining for new level
	remainingTime = 200;

	// Load maze data for the new level
	loadMaze("maze.txt", currentLevel);

	// Output confirmation to console
	std::cout << "Switched to level" << currentLevel << std::endl;
}

void resetGame()
{
	// Reset level state
	isGameOver = false;
	remainingTime = 200;
	coinsCollected = 0;
	// currentLevel = 1;
	mainMenu = true;
	fallRotation = 0.0f;
	loadMaze("maze.txt", currentLevel);

	// Reset tank position to the center of the maze
	tankPosition.x = centerX;
	tankPosition.z = centerZ;
	tankPosition.y = 0.0f;
}

/*----------------------------------------------------// KeyBoard Interaction //---------------------------------------------------*/
void keyboard(unsigned char key, int x, int y)
{
	// Quits program when esc is pressed
	if (key == 27) // esc key code
	{
		system("canberra-gtk-play -f smb_pause.wav &");
		// Toggle in-game menu on ESC key
		if (!gameWon && !isGameOver && !mainMenu)
		{
			showMenu = !showMenu;
			isPaused = showMenu;
		}
	}

	if (key == 'i' || key == 'I') {
		brightness += 0.2f;
		if (brightness > 3.0f) {
			brightness = 3.0f; // clamp max
		}
	}
	if (key == 'u' || key == 'U') {
		brightness -= 0.2f;
		if (brightness < 0.0f) {
			brightness = 0.0f; // Clamp min
		}
	}

	/*---------------------------------------------------// Menu Navigation Controls //------------------------------------------------*/
	if (showMenu)
	{
		// Load level 1
		if (key == '1')
		{
			selectedLevel = 1;
			currentLevel = 1;
			loadMaze("maze.txt", selectedLevel);
			showMenu = false;
			isPaused = false;
		}
		// Load level 2 if level 1 is completed
		else if (key == '2')
		{
			if (levelCompleted[0])
			{
				selectedLevel = 2;
				currentLevel = 2;
				loadMaze("maze.txt", selectedLevel);
				showMenu = false;
				isPaused = false;
			}
		}
		// Load level 3 if levels 1 and 2 are completed
		else if (key == '3')
		{
			if (levelCompleted[0] && levelCompleted[1])
			{
				selectedLevel = 3;
				currentLevel = 3;
				loadMaze("maze.txt", selectedLevel);
				showMenu = false;
				isPaused = false;
			}
		}
		// Restart levels
		else if (key == 'r' || key == 'R')
		{
			loadMaze("maze.txt", selectedLevel);
			showMenu = false;
			isPaused = false;
			resetGame();
			// mainMenu = true;
			fallSoundPlayed = false;
		}
		// Quit the game
		else if (key == 'q' || key == 'Q')
		{
			exit(0);
		}
	}

	/*-------------------------------------------------------// Main Menu Controls //-----------------------------------------*/
	if (mainMenu)
	{
		// Start level 1
		if (key == '1')
		{
			selectedLevel = 1;
			currentLevel = 1;
			loadMaze("maze.txt", selectedLevel);
			showMenu = false;
			isPaused = false;
			mainMenu = false;
		}
		// Start level 2 if level 1 is completed
		else if (key == '2')
		{
			if (levelCompleted[0])
			{
				selectedLevel = 2;
				currentLevel = 2;
				loadMaze("maze.txt", selectedLevel);
				showMenu = false;
				isPaused = false;
				mainMenu = false;
			}
		}
		// Start level 3 if levels 1 and 2 are completed
		else if (key == '3')
		{
			if (levelCompleted[0] && levelCompleted[1])
			{
				selectedLevel = 3;
				currentLevel = 3;
				loadMaze("maze.txt", selectedLevel);
				showMenu = false;
				isPaused = false;
				mainMenu = false;
			}
		}
		// Restart levels from main menu
		else if (key == 'r' || key == 'R')
		{
			loadMaze("maze.txt", currentLevel);
			showMenu = false;
			isPaused = false;
			mainMenu = false;
			fallSoundPlayed = false;
			currentLevel = 1;
			resetGame();
		}
		// Quit the game from main menu
		else if (key == 'q' || key == 'Q')
		{
			exit(0);
		}
	}

	/*-------------------------------------------------------// In-Game Controls (when not paused or game over)---------------------*/
	else if (!isPaused && !isGameOver)
	{
		// Next level
		if ((key == 'n' || key == 'N') && levelComplete && currentLevel < 3 )
		{
			if (currentLevel < 3)
			{
				switchLevel(1);
			}	
			levelComplete = false;
		}

		// Previous level
		if (key == 'p' || key == 'P')
		{
			switchLevel(-1);
		}
		// Enable first-person camera
		if (key == 'c' || key == 'C')
		{
			isFirstPerson = true;
		}
		// Disable first-person camera (switch to third-person)
		if (key == 'v' || key == 'V')
		{
			isFirstPerson = false;
		}
	}

	/*------------------------------------------------------------------// Game Over / Victory Screen Controls //---------------------------*/
	// Reset game
	if ((isGameOver || gameWon) && (key == 'r' || key == 'R'))
	{
		resetGame();
		gameWon = false;
		levelComplete = false;
		// mainMenu = true;
		fallSoundPlayed = false;
	}
	// Quit game
	if ((isGameOver || gameWon) && (key == 'q' || key == 'Q'))
	{
		exit(0);
		gameWon = false;
	}

	// Set key status
	keyStates[key] = true;
}

// Handle key up situation
void keyUp(unsigned char key, int x, int y)
{
	keyStates[key] = false;
}

void specialKeyboard(int key, int x, int y)
{
	keyStates[key] = true;
}

void specialKeyUp(int key, int x, int y)
{
	keyStates[key] = false;
}

/*--------------------------------------------------------// Mouse Interaction //--------------------------------------------------*/
void mouse(int button, int state, int x, int y)
{
	// Do not process mouse input if the game is paused, over, or in the main menu
	if (isPaused || isGameOver || mainMenu)
		return;

	// Left mouse button pressed
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			// Fire a ball from the turret
			fireBall();
		}
	}

	// Right mouse button press/ release for aiming mode
	if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			// Enter aiming mode
			aiming = true;
		}
		else
		{
			// Exit aiming mode
			aiming = false;
		}
	}

	// Update the scene
	glutPostRedisplay();
}

/*-------------------------------------------------------------------// Mouse Movement //-----------------------------------------------*/
void motion(int x, int y)
{
	// Only process motion if aiming is active
	if (!aiming)
		return;

	// Ignore motion if game is paused or in main menu
	if (mainMenu || isPaused)
		return;

	// Ignore motion if mouse is exactly at screen center
	if (x == screenWidth / 2 && y == screenHeight / 2)
		return;

	// Calculate mouse delta from the center of the screen
	float deltaX = -(x - screenWidth / 2); // Horizontal difference (inverted)
	float dy = (screenHeight / 2 - y);	   // Vertical difference

	// Calculate desired turret rotation angle in degrees
	targetTurretRotation = atan2(deltaX, dy) * (180.0f / M_PI);

	// Clamp rotation to range [-180, 180] for smooth interpolation
	if (targetTurretRotation > 180.0f)
		targetTurretRotation -= 360.0f;
	if (targetTurretRotation < -180.0f)
		targetTurretRotation += 360.0f;
}

/*---------------------------------------------------// HandleKeys Funuction //----------------------------------------------------*/
void handleKeys()
{
	// If the tank is falling, ignore input and update fall behavior only
	if (isfalling)
	{
		moveDirection = 0.0f;
		turnDirection = 0.0f;
		checkfall();									// Check if the tank has landed or needs to be reset
		updateTankMovement(tankVelocity, tankRotation); // Apply physics update

		return;
	}

	// Do not handle input if in the main menu or game is paused
	if (mainMenu || isPaused || isGameOver)
		return;
	if (levelComplete)
		return;

	// Handle forward/backward movement (W/S keys)
	if (keyStates['w'])
	{
		moveDirection = 1.0f; // Move forward
	}
	else if (keyStates['s'])
	{
		moveDirection = -1.0f; // Move backward
	}
	else
	{
		moveDirection = 0.0f; // No movement
	}

	// Apply friction if tank exceeds maximum speed
	if (tankVelocity.length() > maxSpeed)
	{
		// Reduce velocity by a factor of friction and time
		tankVelocity = tankVelocity - tankVelocity * friction * deltaTime;
	}

	// Update tank position using velocity and deltaTime
	tankPosition.x += tankVelocity.x * deltaTime;
	tankPosition.y += tankVelocity.y * deltaTime;
	tankPosition.z += tankVelocity.z * deltaTime;

	// Handle turning left/right (A/D keys)
	if (keyStates['a'])
	{
		turnDirection = 1.0f; // Turn Left
	}
	else if (keyStates['d'])
	{
		turnDirection = -1.0f; // Turn right
	}
	else
	{
		turnDirection = 0.0f; // No turning
	}

	// Handle jumping (Spacebar)
	if (keyStates[' '])
	{
		// Can only jump if currently on the ground
		if (isOnGround)
		{
			isJumping = true;
			isOnGround = false;
			jumpVelocity = initialJumpVelocity;					 // Apply initial upward force
			system("canberra-gtk-play -f smb_jump-super.wav &"); // Play jump sound
		}
	}

	// Apply final tank physics update and check for falling conditions
	updateTankMovement(tankVelocity, tankRotation);
	checkfall();
}

/*-------------------------------------------------------// Timer Function //------------------------------------------------------*/
void Timer(int value)
{
	// Only update time and game state if not in menu, paused, gamer over, or game won
	if (mainMenu == 0 && !isGameOver && !isPaused && !gameWon && !levelComplete)
	{
		float previousTime = remainingTime;
		remainingTime -= 0.01f; // Decrease remaining time

		// If time runs out, trigger game over
		if (remainingTime < 0)
		{
			remainingTime = 0;
			isGameOver = true;
			system("canberra-gtk-play -f smb_gameover.wav &");
		}

		if (!warningPlayed && remainingTime <= 100.0f)
		{
			std::cout << "Play warning sound";
			system("canberra-gtk-play -f smb_warning.wav &");
			warningPlayed = true;
			LowTimeWarning = true;
		}
		else if (remainingTime <= 6.0f)
		{
			LowTimeWarning = false;
		}
	}

	if (LowTimeWarning)
	{
		if (flashIncreasing)
			flashAlpha += 0.05f;
		else
			flashAlpha -= 0.05f;

		if (flashAlpha <= 0.2f)
		{
			flashAlpha = 0.2f;
			flashIncreasing = true;
		}
		else if (flashAlpha >= 1.0f)
		{
			flashAlpha = 1.0f;
			flashIncreasing = false;
		}
	}
	else
	{
		flashAlpha = 1.0f;
		flashIncreasing = false;
	}

	// Update the coin animmation:
	// Rotate the coin for visual spinning effect
	coinRotationAngle += 2.0f;

	// Keep angle within 0-360 degrees
	if (coinRotationAngle >= 360.0f)
	{
		coinRotationAngle -= 360.0f;
	}

	// Bounce animation offset for vertical movement of coins
	coinBounce += 0.1f;

	// Redisplay the scene
	glutPostRedisplay();

	// Call function again after 10 milli seconds
	glutTimerFunc(10, Timer, 0);
}

/*------------------------------------------------// Tank Movement Function //-----------------------------------------------------*/
void updateTankMovement(Vector3f &tankVelocity, float &tankAngle)
{
	// Apply Turning
	tankRotation = tankRotation + turnDirection * rotationSpeed;

	// Convert rotation to direction
	float rad = tankRotation * (M_PI / 180.0f);
	Vector3f forward(sin(rad), 0.0f, cos(rad));

	// Update velocity
	tankVelocity = forward * (moveSpeed * moveDirection);

	// Apply Friction if no input
	if (moveDirection == 0.0f)
	{
		tankVelocity = tankVelocity * 0.9f;
	}

	// Update tank Position
	tankPosition = tankPosition + tankVelocity * deltaTime;

	// Calculate rotation amount
	wheelRotation += (moveSpeed * moveDirection * deltaTime) / wheelRadius;

	if (!isOnGround)
	{
		// Apply gravity
		jumpVelocity += g * deltaTime;
		tankPosition.y += jumpVelocity * deltaTime;

		// Check if tank lands
		if (tankPosition.y <= 0.0f)
		{
			tankPosition.y = 0.0f;
			isOnGround = true;
			isJumping = false;
			jumpVelocity = 0.0f;

			checkfall();
		}
	}

	if (isfalling && fallRotation < 90.0f)
	{
		fallRotation += deltaTime * fallSpeed; // fallSpeed could be ~45–90 degrees/sec
		if (fallRotation > 90.0f)
			fallRotation = 90.0f; // Clamp to 90
	}
}

/*-----------------------------------------------------// Camera Function //-------------------------------------------------------*/
void updateCameraPosition()
{
	// Only update the camera if the tank is not falling
	if (!isfalling)
	{
		/*---------------------------------------// FIRST PERSON CAMERA MODE //-----------------------------------------------------------*/
		if (isFirstPerson)
		{
			// Calculate tanks orientation in world space
			float targetpan = (tankRotation + turretBaseRotation) * (M_PI / 180.0f);
			float smoothing = 10.0f;
			cameraPan += (targetpan - cameraPan) * smoothing * deltaTime;

			// Cockpit camera setting
			float tilt = -1.3f;	 // Slight downward angle
			float radius = 0.2f; // Close to the tank

			// Position camera slightly above the tank to simulate a cockpit view
			float verticalOffset = 2.0f;
			float cameraY = tankPosition.y + verticalOffset;
			float cameraZ = tankPosition.z;
			float cameraX = tankPosition.x;

			// Focus point is directly above the tank chassis
			Vector3f focusPoint(cameraX, cameraY, cameraZ);

			// Set camera orientation and position
			cameraManip.setPanTiltRadius(cameraPan, tilt, radius);
			cameraManip.setFocus(focusPoint);
		}
		/*--------------------------------------------------------// THIRD PERSON CAMERA MODE //-----------------------------------------------*/
		else
		{
			// Calculate tanks orientation in world space
			float targetpan = (tankRotation + turretBaseRotation) * (M_PI / 180.0f);
			float smoothing = 5.0f;
			cameraPan += (targetpan - cameraPan) * smoothing * deltaTime;

			// Third person camera settings
			float radius = cameraDistance; // Camera follows from behind
			float tilt = -1.0f;			   // Higher angle than first person

			// Set camera to follow the tank from a distance
			cameraManip.setPanTiltRadius(cameraPan, tilt, radius);
			cameraManip.setFocus(tankPosition);
		}
	}
}

/*------------------------------------------------------// Rotating Turret Function //----------------------------------------------*/
void updateTurretRotation()
{
	// Do not rotate the turret while the tank is falling
	if (isfalling)
		return;

	// Calculate the angular difference between current turret rotation and target
	float angleDifference = targetTurretRotation - turretBaseRotation;

	// Normalise the angle difference to stay within [-180, 180]
	if (angleDifference > 180.0f)
		angleDifference -= 360.0f;
	if (angleDifference < -180.0f)
		angleDifference += 360.0f;

	// Apply smooth rotation if the difference is significant
	if (fabs(angleDifference) > 0.01f)
	{
		// Interpolate the turret rotation by 10% of the angle difference
		turretBaseRotation += angleDifference * 0.1f;
	}
}

/*--------------------------------------------------------// Steering Function //-------------------------------------------------*/
void updateSteeringAngle(float deltaTime)
{
	const float maxSteeringAngle = 15.0f; // Maximum angle the wheels can steer (in degrees)
	const float returnSpeed = 60.0f;	  // Speed at which the steering returns to center (degrees/sec)

	// Turn right (positive turn direction)
	if (turnDirection > 0.0f)
	{
		steeringAngle += steeringSpeed * deltaTime; // Increase angle over time
		if (steeringAngle > maxSteeringAngle)
			steeringAngle = maxSteeringAngle; // Clamp to maximum right
	}
	// Turn left (negative turn direction)
	else if (turnDirection < 0.0f)
	{
		steeringAngle -= steeringSpeed * deltaTime; // Decrease angle over time
		if (steeringAngle < -maxSteeringAngle)
			steeringAngle = -maxSteeringAngle; // Clamp to maximum left
	}
	// Return to center if no input
	else
	{
		if (steeringAngle > 0.0f)
		{
			steeringAngle -= returnSpeed * deltaTime;
			if (steeringAngle < 0.0f)
				steeringAngle = 0.0f;
		}
		else if (steeringAngle < 0.0f)
		{
			steeringAngle += returnSpeed * deltaTime;
			if (steeringAngle > 0.0f)
				steeringAngle = 0.0f;
		}
	}

	// Clamp to ensure steering angle is within bounds
	if (steeringAngle > maxSteeringAngle)
		steeringAngle = maxSteeringAngle;
	if (steeringAngle < -maxSteeringAngle)
		steeringAngle = -maxSteeringAngle;
}

/*-----------------------------------------------------// Update position of the fired ball //-----------------------------------------*/
void updateBallPosition()
{
	// Return early if the ball is not active or hasn't been fired
	if (!ballActive)
		return;
	if (!isBallFired)
		return;

	// Update the lifetime of the ball since it was fired
	ballLifeTime += deltaTime;

	// Move the ball in the horizontal (XZ) plane
	ballPosX += ballDirection.x * deltaTime;
	ballPosZ += ballDirection.z * deltaTime;

	// Apply eased gravity effect after a short delay
	if (ballLifeTime >= gravityDelay)
	{
		float t = (ballLifeTime - gravityDelay);
		float easedGravity = g * (1.0f - expf(-3.0f * t)); // Smooth gravity acceleration
		verticalVelocity += easedGravity * 30.0f * deltaTime;
	}

	// Update the vertical position
	ballPosY += verticalVelocity * deltaTime;

	// If the ball hits the ground deactivate it
	if (ballPosY <= 0.9f)
	{
		ballPosY = 0.0f;
		ballActive = false;
		isBallFired = false;
	}

	// conmvert ball position to tile indices in the maze grid
	int ballTileX = (int)((ballPosX + 1.0f) / 2.0f);
	int ballTileZ = (int)((ballPosZ + 1.0f) / 2.0f);

	// Check if ball is within the maze bounds
	if (ballTileZ >= 0 && ballTileX < MAZE_HEIGHT && ballTileX >= 0 && ballTileZ < MAZE_WIDTH)
	{
		// If the ball hits a coin tile
		if (MAZE[ballTileX][ballTileZ] == 2)
		{
			MAZE[ballTileX][ballTileZ] = 1; // Remove Coin
			coinsCollected++;				// Increment the player's score

			// Play the coin collected sound
			system("canberra-gtk-play -f smb_coin.wav &");

			// Spawn visual particles
			spawnParticles = true;
			particleOrigin = Vector3f(ballTileX, ballPosY, ballPosZ);
			particles.clear();
			for (int i = 0; i < MAX_PARTICLES; ++i)
			{
				Particle p;
				p.position = particleOrigin;
				p.velocity = Vector3f(
					(rand() % 100 - 50) / 50.0f,  // Random X velocity
					(rand() % 100) / 50.0f,		  // Random Y velocity
					(rand() % 100 - 50) / 50.0f); // Random Z velocity
				p.life = 1.0f;					  // Each particle lives for 1 second
				particles.push_back(p);
			}

			std::cout << "Coins Collected: " << coinsCollected << std::endl;

			// If all coins are collected, mark level as comoplete and switch to next
			if (coinsCollected == totalCoins)
			{
				levelCompleted[currentLevel - 1] = true;
				levelComplete = true;

				if (currentLevel == 3) {
					gameWon = true;
					system("canberra-gtk-play -f smb_world_clear.wav &"); // Play win sound
				}
				// switchLevel(1); // Load next level
				if (currentLevel < 3)
				{
					system("canberra-gtk-play -f smb_stage_clear.wav &"); // Victory sound
				}
				coinsCollected = 0; // Reset for next level
			}
		}
	}
}

/*---------------------------------------------------// Updates active particles //------------------------------------*/
void updateParticles(float deltaTime)
{
	// If no particles are currently spawned, exit early
	if (!spawnParticles)
		return;

	// Update each particle's lifetime and position
	for (auto &p : particles)
	{
		p.life -= deltaTime;							  // Decrease particle lifetime
		p.position = p.position + p.velocity * deltaTime; // Move particle according to velocity
	}

	// Check if any particles are still alive
	bool anyAlive = false;
	for (const auto &p : particles)
	{
		if (p.life > 0.0f)
		{
			anyAlive = true; // At least one particle is still active
			break;
		}
	}
	// If all particles have expired, stop spawning particles
	if (!anyAlive)
		spawnParticles = false;
}

/*------------------------------------------------// Tank Falling Function //------------------------------------------------------*/
void checkfall()
{
	// Convert tank world position to maze indices
	int i = static_cast<int>(round(tankPosition.x / 2.0f));
	int j = static_cast<int>(round(tankPosition.z / 2.0f));

	bool onCrate = false;

	// If the tank is already in the air, skip further checkss
	if (!isOnGround)
		return;

	// Make sure the indices are within maze bounds before checking
	if (i >= 0 && j >= 0 && i < MAZE_WIDTH && j < MAZE_HEIGHT)
	{
		// If the current tile has a crate or valid platform (value >= 1), its safe
		if (MAZE[i][j] >= 1)
		{
			onCrate = true;
		}
	}

	// If not on a crate, the tank should fall
	if (!onCrate)
	{
		isfalling = true;
	}
	else
	{
		// Tank is safely grounded on a crate or platform
		isfalling = false;
		tankPosition.y = 0.0f;	 // Reset Y position to ground level
		verticalVelocity = 0.0f; // Reset vertical velocity
	}

	// If the tank is falling, apply gravity and update vertical position
	if (isfalling)
	{
		verticalVelocity += g * 10.0f * deltaTime;	   // Accelerate downward
		tankPosition.y = verticalVelocity * deltaTime; // Update Y position

		isGameOver = true; // Trigger game over state

		// Play falling sound once
		if (!fallSoundPlayed)
		{
			system("canberra-gtk-play -f plankton.wav &");
			fallSoundPlayed = true;
		}
	}
}

/*-----------------------------------------// Fires a projectile //--------------------------------------------------------*/
void fireBall()
{
	// Only fire if a ball is not active
	if (!isBallFired)
	{
		// Set initial position to the turret
		ballPosX = tankPosition.x;
		ballPosY = tankPosition.y + 2.0f; // Height above the tank
		ballPosZ = tankPosition.z;

		// Calculate direction based on tank and turret rotation
		float angle = (tankRotation + turretBaseRotation) * (M_PI / 180.0f);
		float dirX = sin(angle);
		float dirZ = cos(angle);

		// Normalise direction vector
		float length = sqrt(dirX * dirX + dirZ * dirZ);
		if (length != 0.0f)
		{
			dirX /= length;
			dirZ /= length;
		}

		// Fire slightly in front of tank barrel
		float offset = 0.8f;
		ballPosX = tankPosition.x + dirX * offset;
		ballPosZ = tankPosition.z + dirZ * offset;

		// Assign horizontal directional velocity to the ball
		ballDirection = Vector3f(dirX, 0.0f, dirZ) * 12.0f;

		// Reset vertical velocity and lifetime before launch
		verticalVelocity = 0.0f;
		ballLifeTime = 0.0f;

		// Activate ball
		isBallFired = true;
		ballActive = true;
		// Play firing sound effect
		system("canberra-gtk-play -f smb_fireball.wav &");
	}
}

/*-----------------------------------------------// Display Loop //----------------------------------------------------------------*/
void display(void)
{
	// Handle keys
	handleKeys();

	// Set Viewport
	glViewport(0, 0, screenWidth, screenHeight);

	// Clear the screen
	glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use shader
	glUseProgram(shaderProgramID);

	glUseProgram(shaderProgramID);
	glUniform1f(glGetUniformLocation(shaderProgramID, "brightness"), brightness);

	// Projection Matrix - Perspective Projection
	ProjectionMatrix.perspective(90, 1.0, 0.0001, 100.0);

	// Set Projection Matrix
	glUniformMatrix4fv(
		ProjectionUniformLocation,	// Uniform location
		1,							// Number of Uniforms
		false,						// Transpose Matrix
		ProjectionMatrix.getPtr()); // Pointer to ModelViewMatrixValues

	glUniform3f(LightPositionUniformLocation, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
	glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
	glUniform1f(SpecularPowerUniformLocation, specularPower);

	// Apply the camera view using gluLookAt
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw 3D game elements
	DrawMaze();					// Draw the maze
	DrawTank(0.0f, 3.0f, 0.0f); // Draw the tank
	updateBallPosition();		// Update the ball's physics state
	DrawBall(0.0f, 6.0f, 0.0f); // Render the projectile
	drawParticles();			// Render coin particle over time
	updateParticles(deltaTime); // Update particles over time

	// Update tank Controls
	updateSteeringAngle(deltaTime);
	updateTurretRotation();
	updateCameraPosition();

	// Crosshair for aiming
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);

	// Tank collecting coins
	int tankTileX = (int)((tankPosition.x + 1.0f) / 2.0f);
	int tankTileZ = (int)((tankPosition.z + 1.0f) / 2.0f);
	if (tankTileZ >= 0 && tankTileX < MAZE_HEIGHT && tankTileX >= 0 && tankTileZ < MAZE_WIDTH)
	{
		if (MAZE[tankTileX][tankTileZ] == 2) // 2 indicates a coin tile
		{
			MAZE[tankTileX][tankTileZ] = 1; // Remove coin
			coinsCollected++;				// Increment collection count

			// Play coin collection sound
			system("canberra-gtk-play -f smb_coin.wav &");

			std::cout << "Coins collected: " << coinsCollected << std::endl;

			// If all coins are collected switch level
			if (coinsCollected == totalCoins)
			{
				levelCompleted[currentLevel - 1] = true; // Set level as completed
				levelComplete = true;
				if (currentLevel == 3) {
					gameWon = true;
					system("canberra-gtk-play -f smb_world_clear.wav &"); // Play win sound
				}
				// switchLevel(1);
				if (currentLevel < 3)
				{
					system("canberra-gtk-play -f smb_stage_clear.wav &"); // Play  level victory sound
				}
				coinsCollected = 0; // Reset coins for new level
			}
		}
	}

	// Unuse Shader
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

	drawHUD();
	// Redraw frame
	glutPostRedisplay();
	glutSwapBuffers();
}

void reshape(int width, int height)
{
	// Update global screen size
	screenWidth = width;
	screenHeight = height;

	// Update OpenGL viewport
	glViewport(0, 0, screenWidth, screenHeight);

	// Update Projection Matrix
	ProjectionMatrix.perspective(90, (float)width / height, 0.0001, 100.0);
	glUniformMatrix4fv(ProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());
}

/*-----------------------------------------------// Draw Maze Function //----------------------------------------------------------*/
void DrawMaze()
{

	// Determine which tile the tank is currently on
	int tankRow = (int)((tankPosition.x + 1.0f) / 2.0f);
	int tankCol = (int)((tankPosition.z + 1.0f) / 2.0f);

	// Loop through all tiles in the maze
	for (int i = 0; i < MAZE_HEIGHT; i++)
	{
		for (int j = 0; j < MAZE_WIDTH; j++)
		{
			// Draw crate for floor (1) and coin tile (2)
			if (MAZE[i][j] == 1 || MAZE[i][j] == 2)
			{
				// Apply Camera Manipluator to Set Model View Matrix on GPU
				ModelViewMatrix.toIdentity();
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				glUniformMatrix4fv(
					MVMatrixUniformLocation, // Uniform location
					1,						 // Number of Uniforms
					false,					 // Transpose Matrix
					m.getPtr());			 // Pointer to Matrix Values

				// Set Colour after program is in use
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, crateTexture);
				glUniform1i(TextureMapUniformLocation, 0);

				// First crate
				m.translate(i * 2.0, 0.0f, j * 2.0);
				glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
				crateMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			}

			// Draw coin on top of tile 2
			if (MAZE[i][j] == 2)
			{
				// Set up transformation matrix
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);

				// 1. Draw shadow
				{
					Matrix4x4 shadowMatrix = m;
					shadowMatrix.translate(i * 2.0, 1.4f, j * 2.0); // Slightly above the floor
					shadowMatrix.scale(0.3f, 0.01f, 0.3f);			// Flat and wide
					shadowMatrix.rotate(coinRotationAngle, 0.0f, 1.0f, 0.0f);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, shadowTexture); // Or shadow texture
					glUniform1i(TextureMapUniformLocation, 0);

					glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, shadowMatrix.getPtr());
					shadowMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
				}

				// 2. Draw animated bouncing and rotating coin
				float bounceHeight = 0.1f * sin(coinBounce);
				m.translate(i * 2.0, 2.0f + bounceHeight, j * 2.0);
				m.scale(0.3f, 0.3f, 0.3f);
				m.rotate(coinRotationAngle, 0.0f, 1.0f, 0.0f);
				glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, coinTexture);
				glUniform1i(TextureMapUniformLocation, 0);
				coinMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			}

			// Draw a dount tile (3) with shake and fall animation
			if (MAZE[i][j] == 3)
			{
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, donutTexture);
				glUniform1i(TextureMapUniformLocation, 0);

				std::pair<int, int> key = std::make_pair(i, j);
				int fallFrame = donutFallTimers[key];

				// If the tank is on this tile, start shaking/falling
				if (tankRow == i && tankCol == j && isOnGround)
				{
					fallFrame++;
					donutFallTimers[key] = fallFrame;
				}
				else if (fallFrame > 0)
				{
					fallFrame++;
					donutFallTimers[key] = fallFrame;
				}

				// Shake tile for first 35 frames
				float shakeOffset = 0.0f;
				float dropOffset = 0.0f;
				if (fallFrame > 0 && fallFrame < 35)
				{
					shakeOffset = 0.1f * sin(fallFrame * 0.5f);
				}
				else if (fallFrame >= 35)
				{
					dropOffset = -((fallFrame - 35) * 0.05f); // Begin to fall
				}

				// After 100 frames, delete the tile
				if (fallFrame >= 100)
				{
					MAZE[i][j] = 0;
					donutFallTimers.erase(key);
					continue; // skip drawing
				}

				// Apply translation for animation
				m.translate(i * 2.0 + shakeOffset, dropOffset, j * 2.0);
				glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
				donutMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			}
		}
	}
}

/*------------------------------------------------// Draw Tank Function //---------------------------------------------------------*/
void DrawTank(float x, float y, float z)
{
	// Start with the base ModelView matrix transformed by the camera
	Matrix4x4 m = cameraManip.apply(ModelViewMatrix);

	// Apply tank world position, rotation, and scale
	m.translate(tankPosition.x, tankPosition.y, tankPosition.z);
	m.rotate(tankRotation, 0.0f, 1.0f, 0.0f); // Rotate the tank around Y-axis
	m.scale(0.3f, 0.3f, 0.3f);				  // Scale tank to appropriate size
	if (isfalling || fallRotation > 0.0f)
	{
		m.rotate(fallRotation, 1.0f, 0.0f, 0.0f);
	}

	// Bind the tank texture and pass it to the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tankTexture);
	glUniform1i(TextureMapUniformLocation, 0);

	/*-------------------------------------------------// Draw Chassis //--------------------------------------------------------------*/
	m.translate(x, y, z); // Apply offset to base transformation
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
	chassisMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	/*-------------------------------------------------// Draw Turret //---------------------------------------------------------------*/
	Matrix4x4 turretMatrix = m;
	turretMatrix.translate(0.0f, 0.0f, 0.0f);				   // Relative to chassis center
	turretMatrix.rotate(turretBaseRotation, 0.0f, 1.0f, 0.0f); // Yaw rotation
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, turretMatrix.getPtr());
	turretMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	/*-------------------------------------------------// Draw Front Wheeels //--------------------------------------------------------*/
	Matrix4x4 frontWheelMatrix = m;
	frontWheelMatrix.translate(-0.1f, 1.0f, 2.2f);			  // Position in front of chassis center
	frontWheelMatrix.rotate(steeringAngle, 0.0f, 1.0f, 0.0f); // Steering wheels
	frontWheelMatrix.rotate(wheelRotation, 1.0f, 0.0f, 0.0f); // Rolling wheels
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, frontWheelMatrix.getPtr());
	frontWheelMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	/*-------------------------------------------------// Draw Back Wheels //----------------------------------------------------------*/
	Matrix4x4 backWheelMatrix = m;
	backWheelMatrix.translate(-0.1f, 1.1f, -1.3f);			  // Position behind chassis
	backWheelMatrix.rotate(-steeringAngle, 0.0f, 1.0f, 0.0f); // Opposite back wheel steering
	backWheelMatrix.rotate(wheelRotation, 1.0f, 0.0f, 0.0f);  // Rolling effect
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, backWheelMatrix.getPtr());
	backWheelMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

/*------------------------------------------------// Draw Ball //-------------------------------------------------------------------*/
void DrawBall(float x, float y, float z)
{
	// Skip drawing if the ball isn't active or hasn't been fired yet
	if (!ballActive || !isBallFired)
		return;

	// Update ball rotation angle over time (rolling animation)
	ballRotationAngle += 270.0f * deltaTime; // 270 degrees per second

	// Keep the angle within 0-360 degrees
	if (ballRotationAngle > 360.0f)
		ballRotationAngle -= 360.0f;

	// Build the transformation matrix
	Matrix4x4 m = cameraManip.apply(ModelViewMatrix); // Start with camera-alinged modelview
	m.translate(ballPosX, ballPosY, ballPosZ);		  // Position the ball
	m.scale(0.18f, 0.18f, 0.18f);					  // Scale to appropriate size
	m.rotate(ballRotationAngle, 1.0f, 0.0f, 0.0f);	  // Roll along X-axis (forward spin)

	// Bind the ball texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ballTexture);
	glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, m.getPtr());
	ballMesh.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

/*----------------------------------------------// Render Particles //----------------------------------------------*/
void drawParticles()
{
	// If particle system is not active, skip rendering
	if (!spawnParticles)
		return;

	// Disable lighting so particles aren't affected by scene lighting
	glDisable(GL_LIGHTING);
	// Enable alpha blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set the particle point size
	glPointSize(4.0f);
	glBegin(GL_POINTS);

	// Iterate over all particles and render active ones
	for (const auto &p : particles)
	{
		if (p.life > 0.0f) // Only draw if particles are still alive
		{
			glColor4f(1.0f, 0.0f, 0.2f, p.life);				  // Fades with life
			glVertex3f(p.position.x, p.position.y, p.position.z); // Position in 3D space
		}
	}
	glEnd();
	// Reset colour to white (avoid affecting other draws)
	glColor4f(1, 1, 1, 1);
}

/*---------------------------------------------------// Draw border box in screen-space //----------------------------------------------------------------*/
void drawBorderBox(float x, float y, float width, float height, float r, float g, float b, float alpha = 1.0f, float lineWidth = 2.0f)
{
	// Switch to projection matrix to set up 2D orthographic projection
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();								 // Save current projection matrix
	glLoadIdentity();							 // Reset to identity
	gluOrtho2D(0, screenWidth, 0, screenHeight); // 2D projection: origin bottom-left

	// Switch to modelview matrix for drawing
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set border colour and transparency
	glColor4f(r, g, b, alpha);
	glLineWidth(lineWidth);

	// Draw rectangle outline as a loop of lines
	glBegin(GL_LINE_LOOP);
	glVertex2f(x, y);				   // Bottom-left
	glVertex2f(x + width, y);		   // Bottom-right
	glVertex2f(x + width, y + height); // Top-right
	glVertex2f(x, y + height);		   // Top-left
	glEnd();

	// Restore rendering state
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	// Restore previous modelview and projection matrices
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

/*----------------------------------------------------// Semi-transparent rectangular box for text overlays //--------------------------------*/
void drawTextBox(float x, float y, float width, float height, float r, float g, float b, float alpha = 0.5f)
{
	// Switch to projection matrix to set up 2D orthographic projection
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, 0, screenHeight);

	// Switch to modelview matrix for drawing
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set fill colour and transparency
	glColor4f(0.0f, 0.0f, 0.0f, alpha);

	// Draw a solid rectangle for the background
	glBegin(GL_QUADS);
	glVertex2f(x, y);				   // Bottom-left
	glVertex2f(x + width, y);		   // Bottom-right
	glVertex2f(x + width, y + height); // Top-right
	glVertex2f(x, y + height);		   // Top-left
	glEnd();

	// Restore render state
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	// Restore matrices
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawHUD()
{
	const int padding = 20;
	const int extraPadding = 30;
	const int charWidth = 8;
	const int statusBoxHeight = 30;
	const int helpBoxHeight = 25;

	// Setup for 2D orthographic rendering
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, 0, screenHeight);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	/*---------------------------------------------|| Render Text ||---------------------------------------------------------------*/
	/*-----------------------------------|| HUD DURING GAMEPLAY ||-------------------------------------------------------------*/
	if (mainMenu == 0)
	{
		if (!gameWon)
		{
			// --- Top-Left: Level & Coin Status ---
			std::string levelText = "Level: " + std::to_string(currentLevel);
			std::string coinText = "Coins: " + std::to_string(coinsCollected) + "/" + std::to_string(totalCoins);
			std::string statusText = levelText + "   " + coinText;
			int statusWidth = charWidth * statusText.length();
			drawTextBox(10, screenHeight - 40, statusWidth + 2 * padding, statusBoxHeight, 1.0f, 1.0f, 1.0f, 0.8);
			render2dText(statusText, 1.0f, 1.0f, 1.0f, 10 + padding, screenHeight - 28);

			// --- Top-Right: Time ---
			std::string timeText = "Time: " + std::to_string(static_cast<int>(remainingTime)) + "s";
			int timeWidth = charWidth * timeText.length();
			drawTextBox(screenWidth - timeWidth - 2 * padding - 10, screenHeight - 40, timeWidth + 2 * padding, statusBoxHeight, 1.0f, 1.0f, 1.0f, 0.8);
			render2dText(timeText, 1.0f, 1.0f, 1.0f, screenWidth - timeWidth - padding - 10, screenHeight - 28);

			// --- Bottom-Left: Controls ---
			std::string helpText = "WASD: Move  |  Mouse: Aim  |  LMB: Shoot |	RMB: Aim | C: Cockpit View | V: Third-Person View";
			int helpWidth = charWidth * helpText.length();
			drawTextBox(10, 10, helpWidth + 3 * padding + extraPadding, helpBoxHeight, 1.0f, 1.0f, 1.0f, 0.8);
			render2dText(helpText, 1.0f, 1.0f, 1.0f, 10 + padding, 20);
		}
	}
	/*----------------------------------------|| GAME OVER SCREEN ||-------------------------*/
	if (isGameOver)
	{
		if (!gameWon)
		{
			std::string gameOverText = "GAME OVER";
			std::string resetText = "Press R to Reset or Q to Quit";

			int GamerOverWidth = 12 * gameOverText.length(); // adjust if needed for your font size
			int resetWidth = 10 * resetText.length();
			int centerX = screenWidth / 2;
			int centerY = screenHeight / 2;
			int boxWidth = std::max(GamerOverWidth, resetWidth) + 40;
			int boxHeight = 70;

			drawTextBox(centerX - boxWidth / 2, centerY - 30, boxWidth, boxHeight, 1.0f, 1.0f, 1.0f, 0.6f);
			render2dText(gameOverText, 1.0f, 0.0f, 0.0f, centerX - GamerOverWidth / 2, centerY + 10);
			render2dText(resetText, 1.0f, 1.0f, 1.0f, centerX - resetWidth / 2 + 20 , centerY - 15);
		}
	}

	/*------------------------------------------|| Pause MENU ||----------------------------------------*/
	if (showMenu)
	{
		std::string title = "PAUSED - Select Level";

		// Display unlock status based on previous level completion
		std::string level2Text = levelCompleted[0] ? "2: Level 2 (unlocked)" : "2: Level 2 (locked)";
		std::string level3Text = (levelCompleted[0] && levelCompleted[1]) ? "3: Level 3 (unlocked)" : "3: Level 3 (locked)";

		std::string options =
			"1: Level 1    " + level2Text + "     " + level3Text + "\n"
																   "R: Restart	   Q: Quit    ESC: Resume";

		int boxWidth = 10 * options.length(); // Adjust if needed
		int centerX = screenWidth / 2;
		int centerY = screenHeight / 2;

		drawTextBox(centerX - boxWidth / 2 - 10, centerY - 20, boxWidth, 90, 1.0f, 1.0f, 1.0f, 0.8);
		render2dText(title, 1.0f, 1.0f, 1.0f, centerX - title.length() * 6, centerY + 50);
		render2dText("1: Level 1", 0.8f, 0.8f, 0.8f, centerX - 230, centerY + 20);
		render2dText(level2Text, 0.8f, 0.8f, 0.8f, centerX - 220 + 130, centerY + 20);
		render2dText(level3Text, 0.8f, 0.8f, 0.8f, centerX - 160 + 260, centerY + 20);
		render2dText("R: Restart    Q: Quit    ESC: Resume", 0.8f, 0.8f, 0.8f, centerX - 150, centerY - 10);
	}

	/*---------------------------------------|| MAIN MENU ||-------------------------------------------------*/
	if (mainMenu)
	{

		std::string title = "Andreas Tank Game - Select Level";

		int winWidth = 795;
		int boxHeight = 95;
		// Dynamic level text based on completion
		std::string level2Text = levelCompleted[0] ? "2: Level 2 (unlocked)" : "2: Level 2 (locked)";
		std::string level3Text = (levelCompleted[0] && levelCompleted[1]) ? "3: Level 3 (unlocked)" : "3: Level 3 (locked)";

		std::string options =
			"1: Level 1    " + level2Text + "     " + level3Text + "\n"
																   "R: Restart	   Q: Quit";

		int boxWidth = 10 * options.length(); // Adjust if needed
		int centerX = screenWidth / 2;
		int centerY = screenHeight / 2;

		// White border outline
		drawBorderBox(centerX - winWidth / 2 - 10, centerY - 22, boxWidth + 5, boxHeight, 1.0f, 1.0f, 1.0f, 1.0f, 3.0f);
		drawTextBox(centerX - boxWidth / 2 - 10, centerY - 20, boxWidth, 90, 1.0f, 1.0f, 1.0f, 0.8);

		// Menu text
		render2dText(title, 1.0f, 1.0f, 1.0f, centerX - 120, centerY + 50);
		render2dText("1: Level 1", 0.8f, 0.8f, 0.8f, centerX - 200, centerY + 20);
		render2dText(level2Text, 0.8f, 0.8f, 0.8f, centerX - 220 + 130, centerY + 20);
		render2dText(level3Text, 0.8f, 0.8f, 0.8f, centerX - 160 + 260, centerY + 20);
		render2dText("R: Restart            Q: Quit Game", 0.8f, 0.8f, 0.8f, centerX - 120, centerY - 10);
	}

	if (levelComplete && !gameWon)
	{
		std::string winText = "Level Complete!";
		std::string subText = "Press N to continue to the next level.";
		int winWidth = 500;
		int boxHeight = 100;
		int centerX = screenWidth / 2;
		int centerY = screenHeight / 2;

		// Flashing box effect if desired
		 float alpha = 0.5f + 0.5f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.005f);

		drawBorderBox(centerX - winWidth / 2, centerY - 22, winWidth, boxHeight, 1.0f, 1.0f, 0.0f, 1.0f, 0.3f);
		drawTextBox(centerX - winWidth / 2, centerY - 22, winWidth, boxHeight, 0.0f, 0.0f, 0.0f, 8.0f);
		render2dText(winText, 1.0f, 1.0f, 1.0f, centerX - winText.length() * 10 + 80, centerY + 40);
		render2dText(subText, 1.0f, 1.0f, 1.0f, centerX - subText.length() * 5 + 40, centerY);
	} else if (gameWon)
	{
		/*------------------------------------------|| VICTORY SCREEN ||---------------------------------------*/
		std::string winText = "CONGRATULATIONS!";
		std::string subText = "You completed all levels!";
		std::string instructionText = "Press R to restart or Q to quit.";

		int winWidth = 400;
		int boxHeight = 100;
		int centerX = screenWidth / 2;
		int centerY = screenHeight / 2;

		// White border outline
		drawBorderBox(centerX - winWidth / 2, centerY - 22, winWidth, boxHeight, 0.0f, 1.0f, 0.0f, 1.0f, 3.0f);

		// Inner Text box
		drawTextBox(centerX - winWidth / 2, centerY - 22, winWidth, boxHeight, 1.0f, 1.0f, 1.0f, 0.8f);
		render2dText(winText, 0.0f, 1.0f, 0.0f, centerX - winText.length() - 80, centerY + 50);
		render2dText(subText, 1.0f, 1.0f, 1.0f, centerX - subText.length() - 80, centerY + 20);
		render2dText(instructionText, 1.0f, 1.0f, 1.0f, centerX - instructionText.length() - 90, centerY);
	}

	if (LowTimeWarning && !gameWon && !isGameOver)
	{
		std::string warnText = "HURRY UP!";
		std::string subText = "Only 100 seconds left!";
		std::string instructionText = "";

		int boxWidth = 350;
		int boxHeight = 100;
		int centerX = screenWidth / 2;
		int centerY = screenHeight - 150; // Position it higher so it doesn't overlap other UI

		// Red border
		drawBorderBox(centerX - boxWidth / 2, centerY - 22, boxWidth, boxHeight, 1.0f, 0.0f, 0.0f, 1.0f, flashAlpha);

		// Inner red-transparent box
		drawTextBox(centerX - boxWidth / 2, centerY - 22, boxWidth, boxHeight, 1.0f, 0.0f, 0.0f, flashAlpha);

		render2dText(warnText, 1.0f, 1.0f, 1.0f, centerX - warnText.length() * 5, centerY + 40);
		render2dText(subText, 1.0f, 1.0f, 1.0f, centerX - subText.length() - 65, centerY + 15);
	}

	// Restore OpenGL states
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

/*------------------------------------------------// Set Up Render 2d Text Function //---------------------------------------------*/
void render2dText(std::string text, float r, float g, float b, float x, float y)
{
	glColor3f(r, g, b);	 // Set text Colour
	glRasterPos2f(x, y); // Set position in window coordinates

	for (unsigned int i = 0; i < text.size(); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
	}
}
/*=================================================================================================================================*/
/*--------------------------------------------------------------// END //----------------------------------------------------------*/
/*=================================================================================================================================*/