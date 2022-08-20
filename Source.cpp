#include <iostream>             // cout, cerr
#include <cstdlib>              // EXIT_FAILURE
#include <GL/glew.h>            // GLEW library
#include <GLFW/glfw3.h>         // GLFW library

#define STB_IMAGE_IMPLEMENTATION
#include "STB_Image.h"          // Image Utility

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// #include <shader_m.h> // Learn OpenGL Shader Class
#include "Camera.h" // Camera class
#include "Cylinder.h" // Cylinder 

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Cody Gregory Final"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao1; // VAO's to handle various objects in scene --- See UCreateMesh()
		GLuint vao2;
		GLuint vao3;
		GLuint vao4;
		GLuint vao5;
		GLuint vao6;

		GLuint vbo1; // VBO's to buffer various objects in Scene --- See UCreateMesh()
		GLuint vbo2;
		GLuint vbo3;
		GLuint vbo4;
		GLuint vbo5;
		GLuint vbo6;
		GLuint vbo7;
		GLuint vbo8;

		GLuint nVertices;      // Vertex Data for Drawing
		GLuint nPlaneVertices;
		GLuint nFloorVertices;
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;

	// Mesh Data
	GLMesh gMesh;

	// Declare Cylinders
	Cylinder cylinder(0.1f, 0.1f, 3.0f, 32, 32, true);
	Cylinder cylinderCup(0.15f, 0.20f, 0.75f, 32, 32, true);
	Cylinder cylinderStraw(0.01f, 0.01f, 0.40f, 32, 32, true);

	// Texture Scalars
	glm::vec2 gUVScale(1.0f, 1.0f);
	glm::vec2 gUVCupScale(1.1f, 1.1f);
	glm::vec2 gUVStrawScale(0.6f, 0.6f);
	glm::vec2 gUVFloorScale(2.0f, 2.0f);

	// Texture ID's
	GLuint gTextureId;
	GLuint gTextureIdTable;
	GLuint gTextureIdFloor;
	GLuint gTextureIdCup;
	GLuint gTextureIdStraw;
	GLuint gTextureIdPyr;

	GLint gTexWrapMode = GL_REPEAT;

	// Booleans to control object texture/scale
	bool floorTex = true;
	bool cupTex = true;
	bool strawTex = true;

	// Shader program
	GLuint gProgramId;

	// Camera
	Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// Projection 
	bool perspective = true;

	// Timing
	float gDeltaTime = 0.0f; // time between current frame and last frame
	float gLastFrame = 0.0f;

	// Lighting Colors
	glm::vec3 gObjectColor(1.0f, 1.0f, 1.0f);
	glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 gLightColorFill(1.0f, 1.0f, 1.0f);

	// Light Positions and Scales
	glm::vec3 gLightPosition(1.5f, 2.5f, 3.0f);
	glm::vec3 gLightScale(0.3f);
	glm::vec3 gFillLightPosition(0.0f, 5.0f, 0.0f);
	glm::vec3 gFillLightScale(0.3f);
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 position;          // Attribute Pointer 0
layout(location = 1) in vec3 normal;            // Attribute Pointer 1
layout(location = 2) in vec2 textureCoordinate; // Attribute Pointer 2

// Variables sent to Fragment Shader
out vec3 vertexNormal;
out vec3 vertexFragmentPos;
out vec2 vertexTextureCoordinate;

// Variables for the transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);

	// Define out variables
	vertexFragmentPos = vec3(model * vec4(position, 1.0f));
	vertexNormal = mat3(transpose(inverse(model))) * normal;
	vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,

	// Variables received from Vertex Shader
	in vec3 vertexNormal;
in vec3 vertexFragmentPos;
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // Out value to be rendered

// Light Variables
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform vec3 lightPosFill;
uniform vec3 lightColorFill;

uniform sampler2D uTexture; // Texture Object

// Scalars
uniform vec2 uvScale;
uniform vec2 uvScaleCup;
uniform vec2 uvScaleStraw;
uniform vec2 uvScaleFloor;
uniform vec2 uvScaleToy;

// Boolean values for various Textures / Scalars
uniform bool isFloor;
uniform bool isCup;
uniform bool isStraw;
uniform bool isToy;

void main()
{
	// Ambient Light
	float ambientStrength = 0.3f; // Set Key light ambient strength to 30%
	vec3 ambient = ambientStrength * lightColor;

	// Fill light
	float ambientStrengthFill = 0.1f; // Set fill light ambient strength to 10%
	vec3 ambient2 = ambientStrengthFill * lightColorFill;

	// Diffuse Light
	vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit

	vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance between light source and fragments on object
	vec3 lightDirection2 = normalize(lightPosFill - vertexFragmentPos);

	float impact = max(dot(norm, lightDirection), 0.0); // Calculate diffuse impact by generating dot product of normal and light
	float impact2 = max(dot(norm, lightDirection2), 0.0); // Calculate diffuse impact by generating dot product of normal and light

	vec3 diffuse = impact * lightColor; // Generate diffuse light color
	vec3 diffuse2 = impact2 * lightColor;

	// Specular Light
	float specularIntensity = 0.8f; // Set Specular Light strength to 80%
	float specularIntensity2 = 0.1f; // Set Fill light intensity to 10%

	float highlightSize = 16.0f; // Set Specular Highlight size

	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction

	vec3 reflectDir = reflect(-lightDirection, norm); // Calculate reflection vector
	vec3 reflectDir2 = reflect(-lightDirection2, norm); // Calculate reflection vector

	// Calculate Specular component
	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize);

	vec3 specular = specularIntensity * specularComponent * lightColor;
	vec3 specular2 = specularIntensity2 * specularComponent2 * lightColorFill;

	// Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

	// Adjust scalar used depending on texture
	if (isFloor) {
		textureColor = texture(uTexture, vertexTextureCoordinate * uvScaleFloor);
	}
	if (isCup) {
		textureColor = texture(uTexture, vertexTextureCoordinate * uvScaleCup);
	}
	if (isStraw) {
		textureColor = texture(uTexture, vertexTextureCoordinate * uvScaleStraw);
	}
	if (isToy) {
		textureColor = texture(uTexture, vertexTextureCoordinate * uvScaleToy);
	}

	// Calculate Phong result
	vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;
	vec3 phong2 = (ambient2 + diffuse2 + specular2) * textureColor.xyz;

	vec3 totalLight = phong + phong2; // Add vectors to generate multiple lights

	fragmentColor = vec4(totalLight, 1.0); // Set lights to be sent to GPU
}
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up
void flipImageVertically(unsigned char* image, int width, int height, int channels) {
	for (int j = 0; j < height / 2; j++) {
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; i--) {
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			index1++;
			index2++;
		}
	}
}


int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the mesh
	UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

	// Create the shader program
	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	// Load Textures
	const char* texFilename = "textures/Table_Texture.jpg";
	if (!UCreateTexture(texFilename, gTextureIdTable)) {
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	const char* texFilename2 = "textures/Wood_Floor.jpg";
	if (!UCreateTexture(texFilename2, gTextureIdFloor)) {
		cout << "Failed to load texture " << texFilename2 << endl;
		return EXIT_FAILURE;
	}

	const char* texFilename3 = "textures/Table_Leg.jpg";
	if (!UCreateTexture(texFilename3, gTextureId)) {
		cout << "Failed to load texture " << texFilename3 << endl;
		return EXIT_FAILURE;
	}

	const char* texFilename4 = "textures/Coke_Logo.jpg";
	if (!UCreateTexture(texFilename4, gTextureIdCup)) {
		cout << "Failed to load texture " << texFilename4 << endl;
		return EXIT_FAILURE;
	}

	const char* texFilename5 = "textures/Straw.jpg";
	if (!UCreateTexture(texFilename5, gTextureIdStraw)) {
		cout << "Failed to load texture " << texFilename5 << endl;
		return EXIT_FAILURE;
	}

	const char* texFilename6 = "textures/Toy_Lego.jpg";
	if (!UCreateTexture(texFilename6, gTextureIdPyr)) {
		cout << "Failed to load texture " << texFilename6 << endl;
		return EXIT_FAILURE;
	}

	// Tell OpenGL for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gProgramId);

	// Set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		// per-frame timing
		// --------------------
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// input
		// -----
		UProcessInput(gWindow);

		// Render this frame
		URender();

		glfwPollEvents();
	}

	// Release mesh data
	UDestroyMesh(gMesh);

	// Release texture
	UDestroyTexture(gTextureIdTable);
	UDestroyTexture(gTextureIdFloor);
	UDestroyTexture(gTextureIdCup);
	UDestroyTexture(gTextureIdStraw);
	UDestroyTexture(gTextureIdPyr);

	// Release shader program
	UDestroyShaderProgram(gProgramId);

	exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// Some test features include: Brackers [] to adjust texture scale. P and O to toggle between Perspective and Orthogonal Views. 
void UProcessInput(GLFWwindow* window)
{
	static const float cameraSpeed = 2.5f;

	// Exit
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Navigate Scene
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);

	// Added controls for up and down control using the Q and E keys. Press E for Up, Q for Down.
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, gDeltaTime);

	// Toggles between Perspective and Ortho Projections (P will do both, O is also Ortho)
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		glfwWaitEvents;
		perspective = !perspective;
		glfwWaitEventsTimeout(2);
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		perspective = false;
	}

	// Testing function to view different scale of a texture
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
	{
		gUVScale += 0.1f;
		cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
	{
		gUVScale -= 0.1f;
		cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
	}

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed" << endl;
		else
			cout << "Left mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed" << endl;
		else
			cout << "Middle mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed" << endl;
		else
			cout << "Right mouse button released" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event" << endl;
		break;
	}
}


// Function called to render a frame
void URender()
{
	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Scales an Object
	glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));

	// Scale Pyramid
	glm::mat4 scalePyr = glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));

	// 2. Rotates an Object
	glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));

	// Rotate Pyramid 
	glm::mat4 rotationPyr = glm::rotate(40.0f, glm::vec3(0.0, 1.0f, 0.0f));

	// Rotates Cylinders
	glm::mat4 rotationCyl = glm::rotate(300.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotationCylStraw = glm::rotate(280.0f, glm::vec3(1.0, 1.0f, 1.0f));

	// 3. Places an Object at the Origin
	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	// Translate Pyramid
	glm::mat4 translationPyr = glm::translate(glm::vec3(1.0f, -0.8f, 1.0f));

	// Translation for Cylinder Objects
	glm::mat4 translationCyl = glm::translate(glm::vec3(-3.5f, -2.70f, 1.5f));
	glm::mat4 translationCyl1 = glm::translate(glm::vec3(3.5f, -2.70f, 1.5f));
	glm::mat4 translationCyl2 = glm::translate(glm::vec3(3.5f, -2.70f, -1.5f));
	glm::mat4 translationCyl3 = glm::translate(glm::vec3(-3.5f, -2.70f, -1.5f));
	glm::mat4 translationCylCup = glm::translate(glm::vec3(-1.0f, -0.62f, 1.0f));
	glm::mat4 translationCylStraw = glm::translate(glm::vec3(-0.95f, -0.2f, 1.0f));

	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;

	// camera/view transformation
	glm::mat4 view = gCamera.GetViewMatrix();

	// Creates projections
	glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f); // Perspective
	glm::mat4 projectionO = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f); // Orthogonal

	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(gProgramId, "model");
	GLint viewLoc = glGetUniformLocation(gProgramId, "view");
	GLint projLoc = glGetUniformLocation(gProgramId, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Reference matrix uniforms from the Shader program for the color, light color, light position, and camera position
	GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
	GLint fillLightColorLoc = glGetUniformLocation(gProgramId, "lightColorFill");
	GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
	GLint fillLightPositionLoc = glGetUniformLocation(gProgramId, "lightPosFill");
	GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

	// Pass color, light, and camera data to the Shader program's corresponding uniforms
	glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
	glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
	glUniform3f(fillLightColorLoc, gLightColorFill.r, gLightColorFill.g, gLightColorFill.b);
	glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
	glUniform3f(fillLightPositionLoc, gFillLightPosition.x, gFillLightPosition.y, gFillLightPosition.z);

	// Update Camera
	const glm::vec3 cameraPosition = gCamera.Position;
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	// Toggle perspectives
	if (perspective == false) {
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionO));
	}

	// Set texture to base (non conditional) fragment shader
	GLint floorTexLoc = glGetUniformLocation(gProgramId, "isFloor");
	glUniform1i(floorTexLoc, !floorTex);
	GLint cupTexLoc = glGetUniformLocation(gProgramId, "isCup");
	glUniform1i(cupTexLoc, !cupTex);
	GLint strawTexLoc = glGetUniformLocation(gProgramId, "isStraw");
	glUniform1i(strawTexLoc, !strawTex);

	// Set scale to base (non conditional) scalar
	GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));


	// Pyramid
	glBindVertexArray(gMesh.vao1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureIdPyr); // Set Texture

	glm::mat4 modelPyr = translationPyr * rotationPyr * scalePyr; // Set Model
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPyr)); // Send model data to GPU

	glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices); // Draw Pyramid

	// Set Model back to normal
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Table
	glBindVertexArray(gMesh.vao2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureIdTable); // Set Texture

	glDrawArrays(GL_TRIANGLES, 0, gMesh.nPlaneVertices); // Draw Object

	// Floor
	glBindVertexArray(gMesh.vao4); // Set VAO

	glBindTexture(GL_TEXTURE_2D, gTextureIdFloor); // Set Texture

	if (floorTex) { // Set scale for floor and draw 
		GLint UVScaleFloorLoc = glGetUniformLocation(gProgramId, "uvScaleFloor");
		glUniform2fv(UVScaleFloorLoc, 1, glm::value_ptr(gUVFloorScale));
		glUniform1i(floorTexLoc, floorTex);
		glDrawArrays(GL_TRIANGLES, 0, gMesh.nFloorVertices);
	}

	// Cylinders
	glBindVertexArray(gMesh.vao3); // Set VAO 

	glActiveTexture(GL_TEXTURE0); // Set Texture
	glBindTexture(GL_TEXTURE_2D, gTextureId); // Set Texture
	glm::mat4 modelCyl = translationCyl * rotationCyl * scale; // Set Model
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCyl)); // Send model data to GPU
	glDrawElements(GL_TRIANGLES, cylinder.getIndexCount(), GL_UNSIGNED_INT, (void*)0); // Draw

	glm::mat4 modelCyl1 = translationCyl1 * rotationCyl * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCyl1));

	glDrawElements(GL_TRIANGLES, cylinder.getIndexCount(), GL_UNSIGNED_INT, (void*)0);

	glm::mat4 modelCyl2 = translationCyl2 * rotationCyl * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCyl2));

	glDrawElements(GL_TRIANGLES, cylinder.getIndexCount(), GL_UNSIGNED_INT, (void*)0);

	glm::mat4 modelCyl3 = translationCyl3 * rotationCyl * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCyl3));

	glDrawElements(GL_TRIANGLES, cylinder.getIndexCount(), GL_UNSIGNED_INT, (void*)0);

	// Set scalar for cup texture and draw
	if (cupTex) {
		GLint UVCupScaleLoc = glGetUniformLocation(gProgramId, "uvScaleCup"); // Cup Scalar Location
		glUniform2fv(UVCupScaleLoc, 1, glm::value_ptr(gUVCupScale)); // Set Cup Scalar Value
		glUniform1i(cupTexLoc, cupTex); // Use Cup Scalar

		glBindVertexArray(gMesh.vao5); // Cup
		glBindTexture(GL_TEXTURE_2D, gTextureIdCup);

		glm::mat4 modelCylCup = translationCylCup * rotationCyl * scale;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCylCup));

		glDrawElements(GL_TRIANGLES, cylinderCup.getIndexCount(), GL_UNSIGNED_INT, (void*)0);
	}

	// Set scalar for straw texture and draw
	if (strawTex) {
		GLint UVStrawScaleLoc = glGetUniformLocation(gProgramId, "uvScaleStraw"); // Straw Scalar Location
		glUniform2fv(UVStrawScaleLoc, 1, glm::value_ptr(gUVStrawScale)); // Set Straw Scalar Value
		glUniform1i(strawTexLoc, strawTex); // Use Straw Scalar

		glBindVertexArray(gMesh.vao6); // Straw
		glBindTexture(GL_TEXTURE_2D, gTextureIdStraw);
		glm::mat4 modelCylStraw = translationCylStraw * rotationCylStraw * scale;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCylStraw));

		glDrawElements(GL_TRIANGLES, cylinderStraw.getIndexCount(), GL_UNSIGNED_INT, (void*)0);
	}
	// }

	glBindVertexArray(0); // Clear VAO

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
	// Unused Test Pyramid
	GLfloat verts[] = {
		// Vertices (X, Y, Z)       // Normals (X,Y,Z)         // Texture Coords (U,V)

		// Back Face of Pyramid
		 -1.0f, -1.0f, -1.0f,       0.0f, 0.0f, -1.0f,         0.0f, 0.0f, // 0 Bottom Left Back
		  1.0f, -1.0f, -1.0f,       0.0f, 0.0f, -1.0f,         1.0f, 0.0f, // 1 Bottom Right Back
		   0.0f,  1.0f, 0.0f,       0.0f, 0.0f, -1.0f,         0.0f, 1.0f, // 2 Point

		// Front Face of Pyramid
		  -1.0f, -1.0f, 1.0f,       0.0f, 0.0f, 1.0f,          0.0f, 0.0f, // 3 Bottom Left Front
		   1.0f, -1.0f, 1.0f,       0.0f, 0.0f, 1.0f,          1.0f, 0.0f, // 4 Bottom Right Front
		   0.0f,  1.0f, 0.0f,       0.0f, 0.0f, 1.0f,          0.0f, 1.0f, // 5 Point

		// Left Face of Pyramid
		  -1.0f, -1.0f, 1.0f,       -1.0f, 0.0f, 1.0f,         1.0f, 0.0f, // 6 Bottom Left Front
		 -1.0f, -1.0f, -1.0f,       -1.0f, 0.0f, 1.0f,         0.0f, 0.0f, // 7 Bottom Left Back
		   0.0f,  1.0f, 0.0f,       -1.0f, 0.0f, 1.0f,         0.0f, 1.0f, // 8 Point

		// Right Face of Pyramid
		  1.0f, -1.0f, -1.0f,       1.0f, 0.0f, 0.0f,          0.0f, 0.0f,  // 9 Bottom Right Back
		   1.0f, -1.0f, 1.0f,       1.0f, 0.0f, 0.0f,          1.0f, 0.0f,  // 10 Bottom Right Front
		   0.0f,  1.0f, 0.0f,       1.0f, 0.0f, 0.0f,          0.0f, 1.0f,  // 11 Point

		// Bottom Right Base of Pyramid
		  1.0f, -1.0f, -1.0f,       0.0f, -1.0f, 0.0f,         0.0f, 0.0f,  // 12 Bottom Right Back
		   1.0f, -1.0f, 1.0f,       0.0f, -1.0f, 0.0f,         0.0f, 1.0f,  // 13 Bottom Right Front
		 -1.0f,  -1.0f, 1.0f,       0.0f, -1.0f, 0.0f,         1.0f, 1.0f,  // 14 Bottom Left Front

		// Bottom Left Base of Pyramid
		  -1.0f, -1.0f, 1.0f,       0.0f, -1.0f, 0.0f,         0.0f, 1.0f,  // 15 Bottom Left Front
		 -1.0f, -1.0f, -1.0f,       0.0f, -1.0f, 0.0f,         0.0f, 0.0f,  // 16 Bottom Left Back
		 1.0f,  -1.0f, -1.0f,       0.0f, -1.0f, 0.0f,         1.0f, 0.0f   // 17 Bottom Right Back
	};

	// Table Vertices Data (Position and Tex UV Coords)
	GLfloat vertsPlane[] = {
		// Position              Normals                Texture Coords
		// Top
		4.0f, -1.0f, -2.0f,     0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
		4.0f, -1.0f, 2.0f,      0.0f, 1.0f, 0.0f,       0.0f, 1.0f,
		-4.0f,  -1.0f, 2.0f,    0.0f, 1.0f, 0.0f,       1.0f, 1.0f,

		-4.0f, -1.0f, 2.0f,     0.0f, 1.0f, 0.0f,       0.0f, 1.0f,
		-4.0f, -1.0f, -2.0f,    0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
		4.0f, -1.0f, -2.0f,     0.0f, 1.0f, 0.0f,       1.0f, 0.0f,

		// Bottom
		4.0f, -1.2f, -2.0f,     0.0f, 0.0f, 0.0f,       0.0f, 0.0f,
		4.0f, -1.2f, 2.0f,      0.0f, 0.0f, 0.0f,       0.0f, 1.0f,
		-4.0f,  -1.2f, 2.0f,    0.0f, 0.0f, 0.0f,       1.0f, 1.0f,

		-4.0f, -1.2f, 2.0f,     0.0f, 0.0f, 0.0f,       0.0f, 1.0f,
		-4.0f, -1.2f, -2.0f,    0.0f, 0.0f, 0.0f,       0.0f, 0.0f,
		4.0f, -1.2f, -2.0f,     0.0f, 0.0f, 0.0f,       1.0f, 0.0f,

		// Front Edge
		-4.0f, -1.2f, 2.0f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
		4.0f, -1.2f, 2.0f,      0.0f, 0.0f, 1.0f,       1.0f, 0.0f,
		4.0f, -1.0f, 2.0f,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f,

		-4.0f, -1.2f, 2.0f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
		4.0f, -1.0f, 2.0f,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
		-4.0f, -1.0f, 2.0f,     0.0f, 0.0f, 1.0f,       0.0f, 1.0f,

		// Back Edge
		-4.0f, -1.2f, -2.0f,    0.0f, 0.0f, 0.0f,       0.0f, 0.0f,
		4.0f, -1.2f, -2.0f,     0.0f, 0.0f, 0.0f,       1.0f, 0.0f,
		4.0f, -1.0f, -2.0f,     0.0f, 0.0f, 0.0f,       1.0f, 1.0f,

		-4.0f, -1.2f, -2.0f,    0.0f, 0.0f, 0.0f,       0.0f, 0.0f,
		4.0f, -1.0f, -2.0f,     0.0f, 0.0f, 0.0f,       1.0f, 1.0f,
		-4.0f, -1.0f, -2.0f,    0.0f, 0.0f, 0.0f,       0.0f, 1.0f,

		// Right Edge
		4.0f, -1.2f, 2.0f,      1.0f, 1.0f, 0.0f,		1.0f, 0.0f,
		4.0f, -1.2f, -2.0f,     1.0f, 1.0f, 0.0f,		1.0f, 0.0f,
		4.0f, -1.0f, -2.0f,     1.0f, 1.0f, 0.0f,		1.0f, 1.0f,

		4.0f, -1.0f, 2.0f,      1.0f, 1.0f, 0.0f,		1.0f, 0.0f,
		4.0f, -1.2f, 2.0f,      1.0f, 1.0f, 0.0f,		1.0f, 1.0f,
		4.0f, -1.0f, -2.0f,     1.0f, 1.0f, 0.0f,		1.0f, 0.0f,

		// Left Edge
		-4.0f, -1.2f, 2.0f,     0.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-4.0f, -1.2f, -2.0f,    0.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-4.0f, -1.0f, -2.0f,    0.0f, 0.0f, 0.0f,		1.0f, 1.0f,

		-4.0f, -1.0f, 2.0f,     0.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-4.0f, -1.2f, 2.0f,     0.0f, 0.0f, 0.0f,		1.0f, 1.0f,
		-4.0f, -1.0f, -2.0f,    0.0f, 0.0f, 0.0f,		1.0f, 0.0f,
	};

	// Floor Vertices Data (Position and Tex UV Coords)
	GLfloat vertsFloor[] = {
		// Position             Normals                 Texture Coords
		8.0f, -4.2f, -6.0f,     0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
		8.0f, -4.2f, 6.0f,      0.0f, 1.0f, 0.0f,       0.0f, 1.0f,
		-8.0f,  -4.2f, 6.0f,    0.0f, 1.0f, 0.0f,       1.0f, 1.0f,

		-8.0f, -4.2f, 6.0f,     0.0f, 1.0f, 0.0f,       0.0f, 1.0f,
		-8.0f, -4.2f, -6.0f,    0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
		8.0f, -4.2f, -6.0f,     0.0f, 1.0f, 0.0f,       1.0f, 0.0f
	};

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;

	// Strides between vertex coordinates
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

	// Vertex Data for Drawing
	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
	mesh.nPlaneVertices = sizeof(vertsPlane) / (sizeof(vertsPlane[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
	mesh.nFloorVertices = sizeof(vertsFloor) / (sizeof(vertsFloor[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

	// Generate VAO1 for Pyramid
	glGenVertexArrays(1, &mesh.vao1);
	glBindVertexArray(mesh.vao1);

	// Create VBO for Pyramid
	glGenBuffers(1, &mesh.vbo1);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo1); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	// Create Vertex Attribute Pointers (Specify which verts are for position and which are for texture and normals)
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);

	// Generate VAO2 for Table
	glGenVertexArrays(1, &mesh.vao2);
	glBindVertexArray(mesh.vao2);

	// Create VBO for Table
	glGenBuffers(1, &mesh.vbo2);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo2); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertsPlane), vertsPlane, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	// Vertex Attribute Pointers for Table
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);

	// Generate VAO4 for Floor
	glGenVertexArrays(1, &mesh.vao4);
	glBindVertexArray(mesh.vao4);

	// Create VBO for Floor
	glGenBuffers(1, &mesh.vbo5);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo5);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertsFloor), vertsFloor, GL_STATIC_DRAW);

	// Vertex Attribute Pointers for Floor
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);

	// Create VAO3 Cylinder (Table Legs)
	glGenVertexArrays(1, &mesh.vao3);
	glBindVertexArray(mesh.vao3);

	// 1st VBO for Cylinder
	glGenBuffers(1, &mesh.vbo3);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo3);
	glBufferData(GL_ARRAY_BUFFER, cylinder.getInterleavedVertexSize(), cylinder.getInterleavedVertices(), GL_STATIC_DRAW);

	// 2nd VBO for Cylinder
	glGenBuffers(1, &mesh.vbo4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder.getIndexSize(), cylinder.getIndices(), GL_STATIC_DRAW);

	// Enable Attribute Arrays
	glEnableVertexAttribArray(0); // Vertices
	glEnableVertexAttribArray(1); // Normals
	glEnableVertexAttribArray(2); // Texture

	GLint cylStride = cylinder.getInterleavedStride(); // Stride for Cylinders

	// Vertex Attribute Pointers for Cylinder
	glVertexAttribPointer(0, 3, GL_FLOAT, false, cylStride, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, cylStride, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 2, GL_FLOAT, false, cylStride, (void*)(sizeof(float) * 6));

	// Create VAO Cylinder (Cup)
	glGenVertexArrays(1, &mesh.vao5);
	glBindVertexArray(mesh.vao5);

	// 1st VBO for Cylinder
	glGenBuffers(1, &mesh.vbo6);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo6);
	glBufferData(GL_ARRAY_BUFFER, cylinderCup.getInterleavedVertexSize(), cylinderCup.getInterleavedVertices(), GL_STATIC_DRAW);

	// 2nd VBO for Cylinder
	glGenBuffers(1, &mesh.vbo7);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo7);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinderCup.getIndexSize(), cylinderCup.getIndices(), GL_STATIC_DRAW);

	// Enable Attribute Arrays
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// Vertex Attribute Pointers for Cylinder
	glVertexAttribPointer(0, 3, GL_FLOAT, false, cylStride, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, cylStride, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 2, GL_FLOAT, false, cylStride, (void*)(sizeof(float) * 6));

	// Generate VAO for Straw
	glGenVertexArrays(1, &mesh.vao6);
	glBindVertexArray(mesh.vao6);

	// 1st VBO for Cylinder
	glGenBuffers(1, &mesh.vbo8);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo8);
	glBufferData(GL_ARRAY_BUFFER, cylinderStraw.getInterleavedVertexSize(), cylinderStraw.getInterleavedVertices(), GL_STATIC_DRAW);

	// 2nd VBO for Cylinder
	glGenBuffers(1, &mesh.vbo7);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo7);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinderStraw.getIndexSize(), cylinderStraw.getIndices(), GL_STATIC_DRAW);

	// Enable Attribute Arrays
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// Declare Vertex Attribute Pointers for Cylinder
	glVertexAttribPointer(0, 3, GL_FLOAT, false, cylStride, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, cylStride, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 2, GL_FLOAT, false, cylStride, (void*)(sizeof(float) * 6));
}


void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao1);
	glDeleteVertexArrays(1, &mesh.vao2);
	glDeleteVertexArrays(1, &mesh.vao3);
	glDeleteVertexArrays(1, &mesh.vao4);
	glDeleteVertexArrays(1, &mesh.vao5);
	glDeleteVertexArrays(1, &mesh.vao6);
	glDeleteBuffers(1, &mesh.vbo1);
	glDeleteBuffers(1, &mesh.vbo2);
	glDeleteBuffers(1, &mesh.vbo3);
	glDeleteBuffers(1, &mesh.vbo4);
	glDeleteBuffers(1, &mesh.vbo5);
	glDeleteBuffers(1, &mesh.vbo6);
	glDeleteBuffers(1, &mesh.vbo7);
	glDeleteBuffers(1, &mesh.vbo8);
}

// Generate and load the texture
bool UCreateTexture(const char* filename, GLuint& textureId) {
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image) {
		flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// Set the Texture Wrapping Parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Set the texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		}
		else if (channels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		}
		else {
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}

// Clear texture from memory
void UDestroyTexture(GLuint textureId) {
	glGenTextures(1, &textureId);
}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}


void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}