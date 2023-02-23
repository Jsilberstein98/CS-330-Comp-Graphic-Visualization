#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

//GLM library
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h>

using namespace std;

int width, height;
const double PI = 3.14159;
const float toRadians = PI / 180.0f;

// Declare Input Callback Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void UProcessInput(GLFWwindow* window);

// Declare View Matrix
glm::mat4 viewMatrix;

// Camera Field of View
GLfloat fov = 45.0f;

void initCamera();

// Define Camera Attributes
glm::vec3 cameraPosition = glm::vec3(-2.0f, 2.5f, 3.0f); // Move 2 units back in z towards screen
glm::vec3 target = glm::vec3(1.0f, 0.0f, -2.0f); // What the camera points to
glm::vec3 cameraDirection = glm::normalize(cameraPosition - target); // direction z
glm::vec3 worldUp = glm::vec3(0.0, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));// right vector x
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight)); // up vector y
glm::vec3 CameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // 1 unit away from lense

// Camera Transformation Prototype
void transformCamera();

// Boolean array for keys and mouse buttons
bool keys[1024], mouseButtons[3];

// Input state booleans
bool isPanning = false, isOrbiting = false;

// Pitch and Yaw
GLfloat radius = 5.0f, rawYaw = 0.0f, rawPitch = 0.0f, degYaw, degPitch;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat lastX = 320, lastY = 240, xChange, yChange; // Center mouse cursor
bool firstMouseMove = true;

//Light source Position
glm::vec3 lightPosition(0.0f, 1.0f, 2.0f);

// Draw Primitive(s)
void draw()
{
	GLenum mode = GL_TRIANGLES;
	GLsizei indices = 36;
	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr); 

	GLsizei floorIndices = 6;
	glDrawElements(mode, floorIndices, GL_UNSIGNED_BYTE, nullptr);
}


// Create and Compile Shaders
static GLuint CompileShader(const string& source, GLuint shaderType)
{
	// Create Shader object
	GLuint shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();

	// Attach source code to Shader object
	glShaderSource(shaderID, 1, &src, nullptr);

	// Compile Shader
	glCompileShader(shaderID);

	// Return ID of Compiled shader
	return shaderID;

}

// Create Program Object
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader)
{
	// Compile vertex shader
	GLuint vertexShaderComp = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile fragment shader
	GLuint fragmentShaderComp = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// Create program object
	GLuint shaderProgram = glCreateProgram();

	// Attach vertex and fragment shaders to program object
	glAttachShader(shaderProgram, vertexShaderComp);
	glAttachShader(shaderProgram, fragmentShaderComp);

	// Link shaders to create executable
	glLinkProgram(shaderProgram);

	// Delete compiled vertex and fragment shaders
	glDeleteShader(vertexShaderComp);
	glDeleteShader(fragmentShaderComp);

	// Return Shader Program
	return shaderProgram;

}

int main(void) {
	width = 640; height = 480;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "The Scene", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	// Set input callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//Make window's context current
	glfwMakeContextCurrent(window);

	//initialize GLEW
	if (glewInit() != GLEW_OK)
		cout << "Error with GLEW!" << endl;


	GLfloat lampVertices[] = {
		-5.5, 0.0, -2.0,
		-4.0, 0.0, -1.0,
		-5.5, 3.0, -2.0,
		-4.0, 3.0, -1.0,
		-4.5, 3.0, -0.2,
		-6.2, 3.0, -1.1,
		-4.5, 0.0, -0.2,
		-6.2, 0.0, -1.1
	};

	GLfloat floorVertices[]{
		//Plane coordinates
		- 8.0, 3.0, -10.0,	// Back right of plane - 0
		1.0, 1.0, 1.0,
		0.0, 0.0,
		0.0, 0.0, 1.0,

		-8.0, 3.0, 10.0,	// Front right of plane - 1 
		1.0, 1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0, 1.0,

		8.0, 3.0, 10.0,		// Front left of plane - 2
		1.0, 1.0, 1.0,
		0.0, 1.0,
		0.0, 0.0, 1.0,

		8.0, 3.0, -10.0,	// Back left of plane - 3
		1.0, 1.0, 1.0,
		1.0, 1.0,
		0.0, 0.0, 1.0
	};

	GLfloat floorIndices[]{
		//Floor 
		0, 1, 2,	//Front triangle
		0, 2, 3	// Back Triangle
	};


	GLfloat vertices[] = {
		//Pasta Box cooridnates
		-5.5, 0.0, -2.0,	//Front left corner of pasta box - Bottom - 0
		1.0, 1.0, 1.0,
		1.0, 1.0,
		0.0f, 0.0f, 1.0f,

		-4.0, 0.0, -1.0,	//Front right corner of pasta box - bottom - 1
		1.0, 1.0, 1.0,
		0.0, 1.0,
		0.0f, 0.0f, 1.0f,

		-5.5, 3.0, -2.0,	//Front left corner of pasta box - Top - 2
		1.0, 1.0, 1.0,
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-4.0, 3.0, -1.0,	//Front right corner of pasta box - Top - 3
		1.0, 1.0, 1.0,
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-4.5, 3.0, -0.2,	//back right corner of pasta box - Top - 4
		1.0, 1.0, 1.0,
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-6.2, 3.0, -1.1,	//Back left corner of pasta box - Top - 5
		1.0, 1.0, 1.0,
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-4.5, 0.0, -0.2,	//back right corner of pasta box - bottom - 6
		1.0, 1.0, 1.0,
		1.0, 1.0,
		0.0f, 0.0f, 1.0f,

		-6.2, 0.0, -1.1,	//back left corner of pasta box - bottom - 7
		1.0, 1.0, 1.0,
		0.0, 1.0,
		0.0f, 0.0f, 1.0f

	};
	//define element indices
	GLubyte indices[] = {
		//Pasta Box 
		0,1,2, // Triangle 1 - Front left
		1,2,3, // Triangle 2 - Front right
		2,3,4, // Triangle 3 - Top right
		2,4,5, // Triangle 4 - Top left
		1,4,6, // Triangle 5 - Right face right
		1,3,4, // Triangle 6 - Right face left
		0,2,7, // Triangle 7 - Left face Right
		2,5,7, // Triangle 8 - Left face left
		4,6,7, // Triangle 9 - back right triangle
		4,5,7,  //Triangle 10 - Back left triangle
		0,1,6,
		0,6,7
	};

	glm::vec3 planePositions[] = {
		glm::vec3(0.0f, 0.0f, 0.5f),
		glm::vec3(0.5f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -0.5f),
		glm::vec3(-0.5f, 0.0f, 0.0f)
	};

	glm::float32 planeRotations[] = {
		250.0f, 90.0f, 180.0f, 270.0f
	};

	//enable depth buffer
	glEnable(GL_DEPTH_TEST);

	// wireFrame Mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GLuint pastaVBO, pastaEBO, pastaVAO, floorVAO, floorEBO, floorVBO, lampVAO, lampEBO, lampVBO, sauce1VAO, sauce2VAO, sauce1VBO, sauce2VBO, sauce1EBO, sauce2EBO, oil1VAO, oil2VAO, oil1VBO, oil2VBO, oil1EBO, oil2EBO, pepper1VAO, pepper2VAO, pepper1VBO, pepper2VBO, pepper1EBO, pepper2EBO;

	glGenBuffers(1, &pastaVBO); // Creates VBO
	glGenBuffers(1, &pastaEBO); // Creates EBO

	glGenBuffers(1, &floorVBO); // Creates VBO
	glGenBuffers(1, &floorEBO); // Creates EBO

	glGenBuffers(1, &sauce1VBO); // Creates VBO
	glGenBuffers(1, &sauce1EBO); // Creates EBO

	glGenBuffers(1, &sauce2VBO); // Creates VBO
	glGenBuffers(1, &sauce2EBO); // Creates EBO

	glGenBuffers(1, &oil1VBO); // Creates VBO
	glGenBuffers(1, &oil1EBO); // Creates EBO

	glGenBuffers(1, &oil2VBO); // Creates VBO
	glGenBuffers(1, &oil2EBO); // Creates EBO

	glGenBuffers(1, &pepper1VBO); // Creates VBO
	glGenBuffers(1, &pepper1EBO); // Creates EBO

	glGenBuffers(1, &pepper2VBO); // Creates VBO
	glGenBuffers(1, &pepper2EBO); // Creates EBO

	glGenBuffers(1, &lampVBO); // Creates VBO
	glGenBuffers(1, &lampEBO); // Creates EBO

	glGenVertexArrays(1, &pastaVAO); //Creates VAO
	glGenVertexArrays(1, &floorVAO); //Creates VAO
	glGenVertexArrays(1, &sauce1VAO); //Creates VAO
	glGenVertexArrays(1, &sauce2VAO); //Creates VAO
	glGenVertexArrays(1, &oil1VAO); //Creates VAO
	glGenVertexArrays(1, &oil2VAO); //Creates VAO
	glGenVertexArrays(1, &pepper1VAO); //Creates VAO
	glGenVertexArrays(1, &pepper2VAO); //Creates VAO
	glGenVertexArrays(1, &lampVAO); //Creates VAO



	glBindVertexArray(pastaVAO);

		// VBO and EBO Placed in User-Defined VAO
		glBindBuffer(GL_ARRAY_BUFFER, pastaVBO); // Select VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pastaEBO); // Select EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
		// Specify attribute location and layout to GPU
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
		glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)




	//Floor
	glBindVertexArray(floorVAO);

		// VBO and EBO Placed in User-Defined VAO
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO); // Select VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO); // Select EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW); // Load vertex attributes
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW); // Load indices 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
		glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)




	//Sauce body
	glBindVertexArray(sauce1VAO);

		// VBO and EBO Placed in User-Defined VAO
		glBindBuffer(GL_ARRAY_BUFFER, sauce1VBO); // Select VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sauce1EBO); // Select EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)



	//Sauce lid
	glBindVertexArray(sauce2VAO);

		// VBO and EBO Placed in User-Defined VAO
		glBindBuffer(GL_ARRAY_BUFFER, sauce2VBO); // Select VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sauce2EBO); // Select EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)



	//Oil body
	glBindVertexArray(oil1VAO);

		// VBO and EBO Placed in User-Defined VAO
		glBindBuffer(GL_ARRAY_BUFFER, oil1VBO); // Select VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oil1EBO); // Select EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)



	//Oil cap
	glBindVertexArray(oil2VAO);

		// VBO and EBO Placed in User-Defined VAO
		glBindBuffer(GL_ARRAY_BUFFER, oil2VBO); // Select VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oil1EBO); // Select EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)



	//Pepper body
	glBindVertexArray(pepper1VAO);

		// VBO and EBO Placed in User-Defined VAO
		glBindBuffer(GL_ARRAY_BUFFER, pepper1VBO); // Select VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oil1EBO); // Select EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);


	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)



	//Pepper cap
	glBindVertexArray(pepper2VAO);

		// VBO and EBO Placed in User-Defined VAO
		glBindBuffer(GL_ARRAY_BUFFER, oil1VBO); // Select VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oil1EBO); // Select EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)



	glBindVertexArray(lampVAO);

		// VBO and EBO Placed in User-Defined VAO
		glBindBuffer(GL_ARRAY_BUFFER, lampVBO); // Select VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lampEBO); // Select EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW); // Load vertex attributes
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)


	//load textures
	int counterTexWidth, counterTexHeight;
	int pastaTexWidth, pastaTexHeight;
	unsigned char* counterImage = SOIL_load_image("counter.png", &counterTexWidth, &counterTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* pastaImage = SOIL_load_image("pasta.png", &pastaTexWidth, &pastaTexHeight, 0, SOIL_LOAD_RGB);

	//Generate Counter textures
	GLuint counterTexture;
	glGenTextures(1, &counterTexture);
	glBindTexture(GL_TEXTURE_2D, counterTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, counterTexWidth, counterTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, counterImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(counterImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Generate pasta texture
	GLuint pastaTexture;
	glGenTextures(1, &pastaTexture);
	glBindTexture(GL_TEXTURE_2D, pastaTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pastaTexWidth, pastaTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pastaImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(pastaImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"layout(location = 1) in vec3 aColor;"
		"layout(location = 2) in vec2 texCoord; "
		"layout(location = 3) in vec3 normal;"
		"out vec3 oColor;"
		"out vec2 oTexCoord;"
		"out vec3 oNormal;"
		"out vec3 fragPos;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"oColor = aColor;"
		"oTexCoord = texCoord;"
		"oNormal = mat3(transpose(inverse(model))) * normal;"
		"fragPos = vec3(model * vec4(vPosition, 1.0f));"
		"}\n";

	// Fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"
		"in vec3 oColor;"
		"in vec2 oTexCoord;"
		"in vec3 oNormal;"
		"in vec3 fragPos;"
		"out vec4 fragColor;"
		"uniform sampler2D myTexture;"
		"uniform vec3 objectColor;"
		"uniform vec3 lightColor;"
		"uniform vec3 lightPos;"
		"uniform vec3 viewPos;"
		"void main()\n"
		"{\n"
		"//ambient\n"
		"float ambientStrength = 0.8f;"
		"vec3 ambient = ambientStrength * lightColor;"
		"//Diffuse\n"
		"vec3 norm = normalize(oNormal);"
		"vec3 lightDir = normalize(lightPos - fragPos);"
		"float diff = max(dot(norm, lightDir), 0.0);"
		"vec3 diffuse = diff * lightColor;"
		"//Specularity\n"
		"float specularStrength = 1.5f;"
		"vec3 viewDir = normalize(viewPos - fragPos);"
		"vec3 reflectDir = reflect(-lightDir, norm);"
		"float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);"
		"vec3 specular = specularStrength * spec * lightColor;"
		"vec3 result = (ambient + diffuse + specular) * objectColor;"
		"fragColor = texture(myTexture, oTexCoord) * vec4(result, 1.0f);"
		"}\n";


	// lamp Vertex shader source code
	string lampVertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"}\n";

	// lamp Fragment shader source code
	string lampFragmentShaderSource =
		"#version 330 core\n"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		"fragColor = vec4(1.0f);"
		"}\n";

	// Creating Shader Program
	GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	GLuint lampShaderProgram = CreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource); 

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) {
		
		//set Frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Resize window
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use Shader Program exe and select VAO before drawing 
		glUseProgram(shaderProgram); // Call Shader per-frame when updating attributes

		// Declare transformations (can be initialized outside loop)		
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		
		// Define LookAt Matrix

		viewMatrix = glm::lookAt(cameraPosition, target, worldUp);
		viewMatrix = glm::translate(viewMatrix, glm::vec3(1.0f, 0.0f, -5.0f));
		viewMatrix = glm::rotate(viewMatrix, 145.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

		projectionMatrix = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);		//(Field Of View, Width and height in floating point values, near plane, Far plane)

		// Get matrix's uniform location and set matrix
		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

		//get light and object color, and light position location
		GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
		GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
		GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
		GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

		//Assign Light and object Colors
		glUniform3f(objectColorLoc, 0.392f, 0.4901f, 0.0f);
		glUniform3f(lightColorLoc, 0.15, 1.0, 0.0);

		//set light position
		glUniform3f(lightPosLoc, lightPosition.x, lightPosition.y, lightPosition.z);

		//Specify view Position
		glUniform3f(viewPosLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

		// Pass transformation to shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glBindTexture(GL_TEXTURE_2D, pastaTexture);
		glBindVertexArray(pastaVAO); // User-defined VAO must be called before draw. 

		for (GLuint i = 0; i < 1; i++)
		{
			//Declare identity matrix
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, glm::vec3(6.0,0.0,0.0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
			modelMatrix = glm::rotate(modelMatrix, planeRotations[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, 170.f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			

			// Draw primitive(s)
			draw();
		}

		// Unbind Shader exe and VOA after drawing per frame
		glBindVertexArray(0); //Incase different VAO will be used after

		glBindTexture(GL_TEXTURE_2D, counterTexture);
		glBindVertexArray(floorVAO); // User-defined VAO must be called before draw. 
		for (GLuint i = 0; i < 1; i++) {
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -0.5, 0.0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(1.f, 1.f, 1.f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			draw();
		}



		glBindVertexArray(0); //Incase different VAO wii be used after

		glUseProgram(0); // Incase different shader will be used after

		glUseProgram(lampShaderProgram);

			GLint lampModelLoc = glGetUniformLocation(lampShaderProgram, "model");
			GLint lampViewLoc = glGetUniformLocation(lampShaderProgram, "view");
			GLint lampProjLoc = glGetUniformLocation(lampShaderProgram, "projection");

			// Pass transformation to shader
			glUniformMatrix4fv(lampViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
			glUniformMatrix4fv(lampProjLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			glBindVertexArray(lampVAO); // User-defined VAO must be called before draw. 
			for (GLuint i = 0; i < 1; i++) {
				glm::mat4 modelMatrix;
				modelMatrix = glm::translate(modelMatrix, planePositions[i] / glm::vec3(8., 8., 8.) + (lightPosition + glm::vec3(-2.0,1.2,-4.5)));
				modelMatrix = glm::scale(modelMatrix, glm::vec3(0.125f, 0.125f, 0.125f));
				modelMatrix = glm::rotate(modelMatrix, 215.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
				glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

				draw();
			}



			glBindVertexArray(0); //Incase different VAO wii be used after

		/* Swap front and back buffers */ 
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		//poll camera transformations
		transformCamera();

		UProcessInput(window);

	}
	//Clear GPU resources
	glDeleteVertexArrays(1, &pastaVAO);
	glDeleteBuffers(1, &pastaVBO);
	glDeleteBuffers(1, &pastaEBO);
	glDeleteVertexArrays(1, &floorVAO);
	glDeleteBuffers(1, &floorVBO);
	glDeleteBuffers(1, &floorEBO);


	glfwTerminate();
	return 0;
}



//Define Input callback functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {


	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

	//clamp FOV
	if (fov >= 1.f && fov <= 45.f)
		fov -= yoffset * 0.075f;

	//default FOV
	if (fov < 1.f)
		fov = 1.f;
	if (fov > 45.f)
		fov = 45.f;
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {

	//Display mouse X and Y coordinates
	//cout << "Mouse X: " << xpos << endl;
	//cout << "Mouse Y: " << ypos << endl;

	if (firstMouseMove) {
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	//calculate the mouse cursor offset
	xChange = xpos - lastX;
	yChange = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	//Pan camera
	if (isPanning) {

		if (cameraPosition.z < 0.0f)
			CameraFront.z = -1.0f;
		else
			CameraFront.z = -1.0f;

		GLfloat cameraSpeed = xChange * deltaTime;
		cameraPosition += cameraSpeed * cameraRight;

		cameraSpeed = yChange * deltaTime;
		cameraPosition += cameraSpeed * cameraUp;
	}

	//Orbit camera
	if (isOrbiting) {
		rawYaw += xChange;
		rawPitch += yChange;

		//Convert Yaw and Pitch to degrees
		degYaw = glm::radians(rawYaw);
		//degPitch = glm::radians(rawPitch);
		degPitch = glm::clamp(glm::radians(rawPitch), -glm::pi<float>() / 2.0f + 0.1f, glm::pi<float>() / 2.0f - 0.1f);

		//Azimuth Altitude formula
		cameraPosition.x = target.x + radius * cosf(degPitch) * sinf(degYaw);
		cameraPosition.y = target.y + radius * sinf(degPitch);
		cameraPosition.z = target.z + radius * cosf(degPitch) * cosf(degYaw);
	}

}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

	if (action == GLFW_PRESS)
		mouseButtons[button] = true;
	else if (action == GLFW_RELEASE)
		mouseButtons[button] = false;
}

//define getTarget function
glm::vec3 getTarget() {

	if (isPanning)
		target = cameraPosition + CameraFront;

	return target;
}

//define transform camera function
void transformCamera() {

	//Specify buttons for panning the camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE])
		isPanning = true;
	else
		isPanning = false;

	//Orbit camera functions
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_LEFT])
		isOrbiting = true;
	else
		isOrbiting = false;

	//Reset camera to default original view in perspective
	if (keys[GLFW_KEY_P])
		initCamera();
}

void initCamera() {
	cameraPosition = glm::vec3(-2.0f, 2.0f, 3.0f); // Camera Positioning
	target = glm::vec3(1.0f, 0.0f, -2.0f); //Define camera's target
	cameraDirection = glm::normalize(cameraPosition - target); // Define camera's facing direction
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	CameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
}


void UProcessInput(GLFWwindow* window) {
	static const float cameraSpeed = 3.5f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraOffset = cameraSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPosition += cameraOffset * CameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPosition -= cameraOffset * CameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPosition -= glm::normalize(glm::cross(CameraFront, cameraUp)) * cameraOffset;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPosition += glm::normalize(glm::cross(CameraFront, cameraUp)) * cameraOffset;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		cameraPosition += cameraOffset * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cameraPosition -= cameraOffset * cameraUp;

}
