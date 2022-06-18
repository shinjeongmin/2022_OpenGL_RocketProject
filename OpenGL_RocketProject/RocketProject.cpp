#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <vector>
#include <string>
#include <random>
#define GLEW_STATIC
#include <GL\glew.h>

#include <GLFW\/glfw3.h>
GLFWwindow* window;

#include <GLM\glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include "shader.h"
bool loadOBJ(const char*, std::vector<glm::vec3>&, std::vector<glm::vec2>&, std::vector<glm::vec3>&);

// define transformation component
class TransformComponent {
public:
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 scale;

	TransformComponent(glm::vec3 _pos, glm::vec3 _scale) {
		translation = glm::translate(glm::mat4(1.0f), _pos);
		rotation = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0,1,0));
		scale = glm::scale(glm::mat4(1.0f), _scale);
	}
	TransformComponent(glm::mat4 _translation, glm::mat4 _rotation, glm::mat4 _scale) {
		translation = _translation;
		rotation = _rotation;
		scale = _scale;
	}
	void rotate(float degree, glm::vec3 axis) {
		rotation = glm::rotate(glm::mat4(1.0f), glm::radians(degree), axis);
	}
	void setTransform(TransformComponent trans) {
		translation = trans.translation;
		rotation = trans.rotation;
		scale = trans.scale;
	}
	void setTransform(glm::mat4 _translation, glm::mat4 _rotation, glm::mat4 _scale) {
		translation = _translation;
		rotation = _rotation;
		scale = _scale;
	}
};

// global variable
glm::mat4 View = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.1f, 1000.0f);

// another funtion
TransformComponent drawObject(TransformComponent trasform, GLuint vertex_buffer, GLuint vertex_color_buffer, GLuint MatrixID, int size);

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

	// gl ���� ����
	glEnable(GL_DEPTH_TEST); // depth test. depth�� �ڵ� ����
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE); // back face culling. back face�� �׸��� ����

	// background
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

	// ----- vertex buffer -----

	// VAO ���� �� ���ε�
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// cube vertex object
	std::vector<glm::vec3> cube_vertex_position_array;
	std::vector<glm::vec2> cube_vertex_uv_array;
	std::vector<glm::vec3> cube_vertex_normal_array;
	loadOBJ("object/cube.obj", cube_vertex_position_array, cube_vertex_uv_array, cube_vertex_normal_array);
	// cylinder vertex object
	std::vector<glm::vec3> cylinder_vertex_position_array;
	std::vector<glm::vec2> cylinder_vertex_uv_array;
	std::vector<glm::vec3> cylinder_vertex_normal_array;
	loadOBJ("object/cylinder.obj", cylinder_vertex_position_array, cylinder_vertex_uv_array, cylinder_vertex_normal_array);
	// cone vertex object
	std::vector<glm::vec3> cone_vertex_position_array;
	std::vector<glm::vec2> cone_vertex_uv_array;
	std::vector<glm::vec3> cone_vertex_normal_array;
	loadOBJ("object/cone.obj", cone_vertex_position_array, cone_vertex_uv_array, cone_vertex_normal_array);
	// triangle vertex object
	GLfloat triangle_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	// each basic shape object VBO ���� �� ���ε�, vertex data ����
	GLuint cube_vertexbuffer;
	glGenBuffers(1, &cube_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, cube_vertex_position_array.size() * sizeof(glm::vec3), &cube_vertex_position_array[0], GL_STATIC_DRAW);
	GLuint cylinder_vertexbuffer;
	glGenBuffers(1, &cylinder_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cylinder_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, cylinder_vertex_position_array.size() * sizeof(glm::vec3), &cylinder_vertex_position_array[0], GL_STATIC_DRAW);
	GLuint cone_vertexbuffer;
	glGenBuffers(1, &cone_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cone_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, cone_vertex_position_array.size() * sizeof(glm::vec3), &cone_vertex_position_array[0], GL_STATIC_DRAW);
	GLuint triangle_vertexbuffer;
	glGenBuffers(1, &triangle_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangle_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertex_buffer_data), triangle_vertex_buffer_data, GL_STATIC_DRAW);

	// ----- color vertex -----

	// cube color vertex object
	std::vector<glm::vec3> cube_vertex_color_buffer_data;
	// setting ramdom color data
	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_real_distribution<float> range(0.0f, 1.0f);
	for (int i = 0; i < cube_vertex_position_array.size(); i++)
		cube_vertex_color_buffer_data.push_back(glm::vec3(range(mt), range(mt), range(mt)));

	// cylinder color vertex object
	std::vector<glm::vec3> cylinder_vertex_color_buffer_data;
	for (int i = 0; i < cylinder_vertex_position_array.size(); i++)
		cylinder_vertex_color_buffer_data.push_back(glm::vec3(range(mt), range(mt), range(mt)));

	// cone color vertex object
	std::vector<glm::vec3> cone_vertex_color_buffer_data;
	for (int i = 0; i < cone_vertex_position_array.size(); i++)
		cone_vertex_color_buffer_data.push_back(glm::vec3(range(mt), range(mt), range(mt)));
	
	// triangle color vertex object
	GLfloat triangle_vertex_color_buffer_data[] = {
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	// �� ���� ����, ���ε�, data ����
	GLuint cube_vertex_color_buffer;
	glGenBuffers(1, &cube_vertex_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, cube_vertex_color_buffer_data.size() * sizeof(glm::vec3), &cube_vertex_color_buffer_data[0], GL_STATIC_DRAW);
	GLuint cylinder_vertex_color_buffer;
	glGenBuffers(1, &cylinder_vertex_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cylinder_vertex_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, cylinder_vertex_color_buffer_data.size() * sizeof(glm::vec3), &cylinder_vertex_color_buffer_data[0], GL_STATIC_DRAW);
	GLuint cone_vertex_color_buffer;
	glGenBuffers(1, &cone_vertex_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cone_vertex_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, cone_vertex_color_buffer_data.size() * sizeof(glm::vec3), &cone_vertex_color_buffer_data[0], GL_STATIC_DRAW);
	GLuint triangle_vertex_color_buffer;
	glGenBuffers(1, &triangle_vertex_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangle_vertex_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertex_color_buffer_data), triangle_vertex_color_buffer_data, GL_STATIC_DRAW);

	GLuint ProgramID = LoadShaders("SimpleVertexShader.vert", "SimpleFragmentShader.frag"); // shader.cpp���� �ʿ�

	// ������ ���� ����
	GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");

	// ---- in rendering loop, variables ----- 

	// MVP ��� ���� -> ����ϱ� ���ϰ� �������� ����
	/*glm::mat4 View = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.1f, 100.0f);*/
	// Models
	glm::mat4 LeftCarriageModel = glm::mat4(1.0f);
	glm::mat4 RightCarriageModel = glm::mat4(1.0f);

	// Model Translation Initialization
	glm::mat4 Translation = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 3));
	glm::mat4 Rotation = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0, 1, 0));
	glm::mat4 Scaling = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	LeftCarriageModel = Translation * Rotation * Scaling;

	// ���콺 �Է� ���� �ʱ�ȭ
	float lastTime = glfwGetTime();
	float currentTime = lastTime;
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// camera position
	glm::vec3 CamPosition = glm::vec3(0, 0, -3);
	// camera movement
	float horizontalAngle = 0.0f;
	float VerticalAngle = 0.0f;
	float mouseSpeed = 0.001f;
	float CamMoveSpeed = 0.01f;

	// ----- Objects Transform Component ----- 
	TransformComponent leftcarriageTransform(glm::vec3(0, 0, 1), glm::vec3(0.5f, 1.0f, 0.5f));
	TransformComponent rightcarriageTransform(glm::vec3(3, 0, 1), glm::vec3(0.5f, 1.0f, 0.5f));
	TransformComponent barrelTransform(glm::vec3(1.5f, 2.0f, 0), glm::vec3(0.09f, 0.08f, 0.09f));
	barrelTransform.rotate(-45.0f, glm::vec3(1, 0, 0));
	TransformComponent planeTransform(glm::vec3(0, -1, -25), glm::vec3(30.0f, 0.01f, 30.0f)); // �� �ٴ��� y position�� -1

	// ���� ������ �߽����� �������� �����ġ�� ����
	TransformComponent rocketBodyTransform(glm::vec3(0,0,5), glm::vec3(0.05f, 0.05f, 0.05f));
	TransformComponent rocketHeadTransform(
		glm::translate(rocketBodyTransform.translation, glm::vec3(0,1.2,0)),
		glm::rotate(rocketBodyTransform.rotation, glm::radians(0.0f), glm::vec3(0, 1, 0)),
		glm::scale(rocketBodyTransform.scale, glm::vec3(11.0f, 5.0f, 11.0f))
	);
	TransformComponent rocketFin1Transform(glm::vec3(0, 0, 0), glm::vec3(1.0f, 1.0f, 1.0f));
	rocketFin1Transform.rotate(180, glm::vec3(0,1,0));

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // depth buffer bit �ʱ�ȭ�� depth test ���ó��

		glUseProgram(ProgramID);

		#pragma region Camera Control

		// ���� �������� cursor position�� �޾ƿͼ� rotation�� �����ϱ�
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glfwSetCursorPos(window, 1024 / 2, 768 / 2); // ������ �������� cursor ����

		horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
		VerticalAngle += mouseSpeed * float(768 / 2 - ypos);
		if (VerticalAngle > 1.5f) VerticalAngle = 1.5f;
		if (VerticalAngle < -1.5f) VerticalAngle = -1.5f;

		glm::vec3 direction = glm::vec3(cos(VerticalAngle) * sin(horizontalAngle), sin(VerticalAngle), cos(VerticalAngle) * cos(horizontalAngle));

		// ī�޶� �� ���� ����
		currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);

		glm::vec3 right = glm::vec3(sin(horizontalAngle - 3.14 / 2.0f), 0, cos(horizontalAngle - 3.14 / 2.0f));
		glm::vec3 up = glm::cross(right, direction);


		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{ // W - move straight 
			CamPosition += direction * deltaTime * CamMoveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{ // A - move left
			CamPosition += -right * deltaTime * CamMoveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{ // S - move back
			CamPosition += -direction * deltaTime * CamMoveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{ // D - move right
			CamPosition += right * deltaTime * CamMoveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{ // Q - move down
			CamPosition += -up * deltaTime * CamMoveSpeed / 2.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{ // E - move up
			CamPosition += up * deltaTime * CamMoveSpeed / 2.0f;
		}

		View = glm::lookAt(CamPosition, CamPosition + direction, up);

		#pragma endregion
		
		#pragma region Cannon Launcher Object

		leftcarriageTransform = drawObject(leftcarriageTransform, cube_vertexbuffer, cube_vertex_color_buffer, MatrixID, cube_vertex_position_array.size());
		rightcarriageTransform = drawObject(rightcarriageTransform, cube_vertexbuffer, cube_vertex_color_buffer, MatrixID, cube_vertex_position_array.size());
		barrelTransform = drawObject(barrelTransform, cylinder_vertexbuffer, cylinder_vertex_color_buffer, MatrixID, cylinder_vertex_position_array.size());
		planeTransform = drawObject(planeTransform, cube_vertexbuffer, cube_vertex_color_buffer, MatrixID, cube_vertex_position_array.size());

		#pragma endregion

		#pragma region

		rocketBodyTransform = drawObject(rocketBodyTransform, cylinder_vertexbuffer, cylinder_vertex_color_buffer, MatrixID, cylinder_vertex_position_array.size());
		rocketHeadTransform = drawObject(rocketHeadTransform, cone_vertexbuffer, cone_vertex_color_buffer, MatrixID, cone_vertex_position_array.size());
		rocketFin1Transform = drawObject(rocketFin1Transform, triangle_vertexbuffer, triangle_vertex_color_buffer, MatrixID, sizeof(triangle_vertex_buffer_data));

		#pragma endregion

		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

TransformComponent drawObject(TransformComponent transform, GLuint vertex_buffer, GLuint vertex_color_buffer, GLuint MatrixID, int size) {
	glm::mat4 Model = transform.translation * transform.rotation * transform.scale;
	glm::mat4 MVP = Projection * View * Model;

	// vertex buffer�� �̿��� ��ġ bind, enable array, attrib pointer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// vertex color buffer�� �̿��� bind, enable array, attrib pointer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_color_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// ������ ���� ������ �Է�
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, size);

	return transform;
}

// .obj ���Ͽ��� ���ؽ� ��ġ, �ؽ�ó ��ǥ, ��� ���� �����͸� �������� �Լ�
bool loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals)

{
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE* file;
	fopen_s(&file, path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf_s(file, "%s", lineHeader, sizeof(lineHeader));
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}