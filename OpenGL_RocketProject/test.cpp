#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define GLEW_STATIC
#include <GL\glew.h>

#include <GLFW\/glfw3.h>
GLFWwindow* window;

#include <GLM\glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include "shader.h"

int main()
{
	// glfw �ʱ�ȭ
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	// �������� â�� �ִ� �߰����� ����
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // �������� 3.3 ���� ���
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ������ ����
	window = glfwCreateWindow(1024, 768, "Playground", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// background
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

	// VAO ���� �� ���ε�
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// vertex buffer ���� ����
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	// VBO ���� �� ���ε�, vertex data ����
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW); 

	static const GLfloat g_vertex_color_buffer_data[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};

	// �� ���� ����, ���ε�, data ���� 
	GLuint vertexColorBuffer;
	glGenBuffers(1, &vertexColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_color_buffer_data), g_vertex_color_buffer_data, GL_STATIC_DRAW);

	GLuint ProgramID = LoadShaders("SimpleVertexShader.vert", "SimpleFragmentShader.frag"); // shader.cpp���� �ʿ�
	
	// ������ ���� ����
	GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");

	glm::mat4 View = glm::lookAt(glm::vec3(3, 3, 3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.1f, 100.0f);
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	do {
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(ProgramID);

		// vertex buffer�� �̿��� �ﰢ�� ��ġ bind, enable array, attrib pointer
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// vertex color buffer�� �̿��� bind, enable array, attrib pointer
		glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// ������ ���� ������ �Է�
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}