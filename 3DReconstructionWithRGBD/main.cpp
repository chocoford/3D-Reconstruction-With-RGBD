#include "stdafx.h"
#include <strsafe.h>
#include "KinectSensor.h"
#include "resource.h"

#include<glad\glad.h>
#include<GLFW\glfw3.h>

#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>

#include"shader.h"
#include "camera.h"

#include "Triangulator.h"
#include "header.h"

#include<iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// vertices
const int vertexStride = 9; //xyz normal rgb
const int rowNum = kinectHeight;
const int colNum = kinectWidth;
const int vertexCount = rowNum * colNum;
const int verticesSize = rowNum * colNum * vertexStride;

int main()
{
	KinectSensor application;

	// Look for a connected Kinect, and create it if found
	application.CreateFirstConnected();

	Triangulator triangulator;


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);



	GLFWwindow* window = glfwCreateWindow(1440, 900, "3D Reconstruction With RGBD", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// build and compile our shader program
	// ------------------------------------
	Shader ourShader("shader.vs", "shader.fs"); // you can name your shader files however you like

	//preconstruct point
	float *vertices = new float[verticesSize];
	long indexCount = (colNum - 1) * (rowNum * 2 + 2);
	unsigned int *indices = new unsigned int[indexCount];
	

	for (int i = 0; i < rowNum; i++) {
		for (int j = 0; j < colNum; j++)
		{
			vertices[vertexStride * (colNum * i + j) + 0] = ((float)j - (float)colNum / 2.0) * 0.01; // x
			vertices[vertexStride * (colNum * i + j) + 1] = -((float)i - (float)rowNum / 2.0) * 0.01; // y
			vertices[vertexStride * (colNum * i + j) + 2] = 0;                        //depth
			vertices[vertexStride * (colNum * i + j) + 3] = 0;                        //nx
			vertices[vertexStride * (colNum * i + j) + 4] = 0;                        //ny
			vertices[vertexStride * (colNum * i + j) + 5] = -1;                        //nz
			vertices[vertexStride * (colNum * i + j) + 6] = 0;                        //R
			vertices[vertexStride * (colNum * i + j) + 7] = 0;                        //G
			vertices[vertexStride * (colNum * i + j) + 8] = 0;                        //B
			// initialize indices values;
			indices[(640 * i + j)] = 0;
		}
	}



	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);	
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	//glPointSize(1);
	glEnable(GL_DEPTH_TEST);
	

	short nvCalculagteFlag = 0;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// preprocess
		nvCalculagteFlag = (nvCalculagteFlag+1) % 2;

		application.Update();

		for (long i = 0; i < vertexCount; i++) {
				

			vertices[vertexStride * i + 2] = application.depthValues[i] > 0 ? -(float)application.depthValues[i] * 0.005 : -20;

			vertices[vertexStride * i + 6] = (float)application.colorsRGBValues[3 * i + 0] / 255.0;//R
			vertices[vertexStride * i + 7] = (float)application.colorsRGBValues[3 * i + 1] / 255.0;//G
			vertices[vertexStride * i + 8] = (float)application.colorsRGBValues[3 * i + 2] / 255.0;//B
		}

		// calculate normal vector.
		if (1)
		for (long i = 0; i < vertexCount; i++)
		{
			int col = i % colNum;
			int row = i / colNum;

			if (col > 0 && col < colNum - 1 && row > 0 && row < rowNum - 1) {
				//center point
				float cx = vertices[vertexStride * i];
				float cy = vertices[vertexStride * i + 1];
				float cz = vertices[vertexStride * i + 2];
				glm::vec3 center = glm::vec3(cx, cy, cz);

				//upper
				float ux = vertices[vertexStride * (i + colNum)];
				float uy = vertices[vertexStride * (i + colNum) + 1];
				float uz = vertices[vertexStride * (i + colNum) + 2];
				glm::vec3 upper = glm::vec3(ux, uy, uz) - center;


				//down
				float dx = vertices[vertexStride * (i - colNum)];
				float dy = vertices[vertexStride * (i - colNum) + 1];
				float dz = vertices[vertexStride * (i - colNum) + 2];
				glm::vec3 down = glm::vec3(dx, dy, dz) - center;

				//left
				float lx = vertices[vertexStride * (i - 1)];
				float ly = vertices[vertexStride * (i - 1) + 1];
				float lz = vertices[vertexStride * (i - 1) + 2];
				glm::vec3 left = glm::vec3(lx, ly, lz) - center;

				//right
				float rx = vertices[vertexStride * (i + 1)];
				float ry = vertices[vertexStride * (i + 1) + 1];
				float rz = vertices[vertexStride * (i + 1) + 2];
				glm::vec3 right = glm::vec3(rx, ry, rz) - center;

				//glm::vec3 upLeft = glm::cross(upper, left);
				//glm::vec3 downRight = glm::cross(down, right);

				//glm::vec3 normal = (upLeft);

				glm::vec3 normal = glm::vec3(0, 0, -1);

				vertices[vertexStride * i + 3] = normal.x;//nx
				vertices[vertexStride * i + 4] = normal.y;//ny
				vertices[vertexStride * i + 5] = normal.z;;//nz
			}
		}


		//triangulator.triangulate(vertices);

		long index = 0;
		for (int i = 0; i < rowNum - 1; i++)
		{
			for (int j = 0; j < colNum; j++) {
				if (j == 0)
				{
					indices[index++] = colNum * i + j;
				}
				indices[index++] = colNum * i + j;
				indices[index++] = colNum * (i + 1) + j;
				if (j == colNum - 1)
				{
					indices[index++] = colNum * (i + 1) + j;
				}
			}
		}




		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(float), vertices, GL_DYNAMIC_DRAW);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexStride * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexStride * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertexStride * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		glBindVertexArray(0);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);


		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw our first triangle
		ourShader.use();

		// create transformations
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 4000.0f);
		// retrieve the matrix uniform locations
		unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		ourShader.setMat4("projection", projection);

		// render box
		glBindVertexArray(VAO);
		//glDrawArrays(GL_POINTS, 0, 640 * 480);
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, 639 * (480 * 2 + 2));
		glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
		
	application.DisConnected();

	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);

}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}