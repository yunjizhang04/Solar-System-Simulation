#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <iostream>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <fstream> 

static unsigned int ss_id = 0;

//shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
	
out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	vertexColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec3 vertexColor;

void main() {
	FragColor = vec4(vertexColor, 1.0);
}
)";

//screenshot
void dump_framebuffer_to_ppm(std::string prefix, unsigned int width, unsigned int height) {
	int pixelChannel = 3;
	int totalPixelSize = pixelChannel * width * height * sizeof(GLubyte);
	GLubyte* pixels = new GLubyte[totalPixelSize];
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	std::string file_name = prefix + ".ppm";
	std::ofstream fout(file_name);
	fout << "P3\n" << width << " " << height << "\n" << 255 << std::endl;
	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			size_t cur = pixelChannel * ((height - i - 1) * width + j);
			fout << (int)pixels[cur] << " " << (int)pixels[cur + 1] << " " << (int)pixels[cur + 2] << " ";
		}
		fout << std::endl;
	}
	ss_id++;
	delete[] pixels;
	fout.flush();
	fout.close();
}

//callback and input handling declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//press p to get ppm
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		std::cout << "Capture Window " << ss_id << std::endl;
		int buffer_width, buffer_height;
		glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
		dump_framebuffer_to_ppm("solarSystem", buffer_width, buffer_height);
	}
}

//cube generation
void generateCubeData(float size, glm::vec3 color, std::vector<float>& vertices, std::vector<unsigned int>& indices)
{
	float halfSize = size / 2.0f;

	float cubeVertices[] = {
		-halfSize, -halfSize, -halfSize, color.r, color.g, color.b,
		halfSize, -halfSize, -halfSize, color.r, color.g, color.b,
		halfSize, halfSize, -halfSize, color.r, color.g, color.b,
		-halfSize, halfSize, -halfSize, color.r, color.g, color.b,
		-halfSize, -halfSize, halfSize, color.r, color.g, color.b,
		halfSize, -halfSize, halfSize, color.r, color.g, color.b,
		halfSize, halfSize, halfSize, color.r, color.g, color.b,
		-halfSize, halfSize, halfSize, color.r, color.g, color.b,
	};

	unsigned int cubeIndices[] = {
		//back
		0, 1, 2, 2, 3, 0,
		//front
		4, 5, 6, 6, 7, 4,
		//bottom
		0, 1, 5, 5, 4, 0,
		//top
		2, 3, 7, 7, 6, 2,
		//left
		0, 3, 7, 7, 4, 0,
		//right
		1, 2, 6, 6, 5, 1
	};
	vertices.insert(vertices.end(), std::begin(cubeVertices), std::end(cubeVertices));
	indices.insert(indices.end(), std::begin(cubeIndices), std::end(cubeIndices));
}

// Sun rotate
float get_sun_rotate_angle_around_itself(float day) {
	return (360.0f / 27.0f) * day;  // Sun completes a full rotation in 27 days
}
// Earth rotate
float get_earth_rotate_angle_around_sun(float day) {
	return (360.0f / 365.0f) * day;  // Earth completes a full revolution around the Sun in 365 days
}
float get_earth_rotate_angle_around_itself(float day) {
	return (360.0f / 1.0f) * day;  // Earth completes a full rotation in 1 day
}
// Moon rotate
float get_moon_rotate_angle_around_earth(float day) {
	return (360.0f / 27.0f) * day;  // Moon revolves around Earth in 27 days
}
float get_moon_rotate_angle_around_itself(float day) {
	return (360.0f / 27.0f) * day;  // Moon rotates around itself in 27 days
}


void bufferSetUp(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO, const std::vector<float>& vertices, const std::vector<unsigned int>& indices)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

//main
int main(int argc, char** argv[]) {

	// glfw: initialize and configure
	//-------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	//---------------------
	GLFWwindow* window = glfwCreateWindow(1024, 576, "Solar System", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	//-------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//compiling vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//compiling fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//link shaders into program
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//clean up
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//vertex data (single cube)
	float vertices[] = {
			//Front face (z = 1) - Red
			-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
			-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

			//Back face (z = -1) - Green
			-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,

			//Left face (x = -1) - Blue
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

			//Right face (x = 1) - Aqua
			0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
			0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 1.0f,

			//Top face (y = 1) - Fuchsia
			-0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
			-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,

			//Bottom face (y = -1) - yellow
			-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
			0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
			0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f,
			-0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f,
	};

	unsigned int indices[] = {
		//front
		0, 1, 2, 2, 3, 0,
		// back
		4, 5, 6, 6, 7, 4,
		//left
		8, 9, 10, 10, 11, 8,
		//right
		12, 13, 14, 14, 15, 12,
		//top
		16, 17, 18, 18, 19, 16,
		//bottom
		20, 21, 22, 22, 23, 20
	};

	//cube data for Sun Earth and Moon
	std::vector<float> vertices_sun, vertices_earth, vertices_moon;
	std::vector<unsigned int> indices_sun, indices_earth, indices_moon;

	//sun (12 x 12 x 12) - yellow
	generateCubeData(20.0f, glm::vec3(1.0f, 1.0f, 0.0f), vertices_sun, indices_sun);
	//Earth (6 x 6 x 6) - blue
	generateCubeData(8.0f, glm::vec3(0.0f, 0.0f, 1.0f), vertices_earth, indices_earth);
	//moon (3 x 3 x 3) - grey
	generateCubeData(4.0f, glm::vec3(0.5f, 0.5f, 0.5f), vertices_moon, indices_moon);

	//buffers + VAO
	unsigned int VBO_Sun, VAO_Sun, EBO_Sun,
				 VBO_Earth, VAO_Earth, EBO_Earth,
				 VBO_Moon, VAO_Moon, EBO_Moon;

	//setting up for each cube
	bufferSetUp(VAO_Sun, VBO_Sun, EBO_Sun, vertices_sun, indices_sun);
	bufferSetUp(VAO_Earth, VBO_Earth, EBO_Earth, vertices_earth, indices_earth);
	bufferSetUp(VAO_Moon, VBO_Moon, EBO_Moon, vertices_moon, indices_moon);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.4f, 0.5f, 1.0f);

	//glViewport(0, 0, 800, 600);

	float day = 0;
	float counter = 1.0f / 24;

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		//incrementation
		day += counter;

		// input
		processInput(window);
	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//camera
		glm::mat4 view = glm::lookAt(glm::vec3(30.0f, 20.0f, 90.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 1000.0f);

		unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

		//rotation angle
		float earth_rotation = get_earth_rotate_angle_around_itself(day);
		float sun_rotation = get_sun_rotate_angle_around_itself(day);
		float moon_rotation = get_moon_rotate_angle_around_itself(day);
		//orbit rotations
		float earth_orbit_rotation = get_earth_rotate_angle_around_sun(day);
		float moon_orbit_rotation = get_moon_rotate_angle_around_earth(day);

		glUseProgram(shaderProgram);

		//earth
		glm::mat4 model_earth = glm::mat4(1.0f);
		glBindVertexArray(VAO_Earth);
		model_earth = glm::rotate(model_earth, glm::radians(earth_orbit_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		model_earth = glm::translate(model_earth, glm::vec3(24.0f, 0.0f, 0.0f));
		model_earth = glm::rotate(model_earth, glm::radians(-earth_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		model_earth = glm::rotate(model_earth, glm::radians(-23.4f), glm::vec3(0.0f, 0.0f, 1.0f));
		model_earth = glm::rotate(model_earth, glm::radians(earth_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_earth));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model_earth));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		//sun
		glm::mat4 model_sun = glm::mat4(1.0f);
		glBindVertexArray(VAO_Sun);
		model_sun = glm::rotate(model_sun, glm::radians(sun_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_sun));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model_sun));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		//moon
		glm::mat4 model_moon = model_earth;
		glBindVertexArray(VAO_Moon);
		model_moon = glm::rotate(model_moon, glm::radians(moon_orbit_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		model_moon = glm::translate(model_moon, glm::vec3(10.0f, 0.0f, 0.0f));
		model_moon = glm::rotate(model_moon, glm::radians(moon_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_moon));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model_moon));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//clean up
	glDeleteVertexArrays(1, &VAO_Sun);
	glDeleteBuffers(1, &VBO_Sun);
	glDeleteBuffers(1, &EBO_Sun);

	glDeleteVertexArrays(1, &VAO_Earth);
	glDeleteBuffers(1, &VBO_Earth);
	glDeleteBuffers(1, &EBO_Earth);

	glDeleteVertexArrays(1, &VAO_Moon);
	glDeleteBuffers(1, &VBO_Moon);
	glDeleteBuffers(1, &EBO_Moon);

	glDeleteProgram(shaderProgram);

	glfwTerminate();
	return 0;
}

