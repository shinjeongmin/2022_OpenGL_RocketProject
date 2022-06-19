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
GLuint loadBMP_custom(const char* imagepath);

// another funtion
void drawCylinder(GLuint, GLuint, GLuint, glm::mat4, int);

int main()
{
	// glfw 초기화
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	// 오픈지엘 창에 넣는 추가적인 정보
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // 오픈지엘 3.3 버전 사용
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 윈도우 생성
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

	// gl 설정 변경
	glEnable(GL_DEPTH_TEST); // depth test. depth값 자동 저장
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE); // back face culling. back face를 그리지 않음

	// background
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

	// ----- vertex buffer -----

	// VAO 생성 및 바인딩
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

	// VBO 생성 및 바인딩, vertex data 복사
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, cube_vertex_position_array.size() * sizeof(glm::vec3), &cube_vertex_position_array[0], GL_STATIC_DRAW);
	GLuint vertexbuffer2;
	glGenBuffers(1, &vertexbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	glBufferData(GL_ARRAY_BUFFER, cylinder_vertex_position_array.size() * sizeof(glm::vec3), &cylinder_vertex_position_array[0], GL_STATIC_DRAW);

	// ----- color -----

	// cube color vertex object
	std::vector<glm::vec3> cube_vertex_color_buffer_data;
	// setting ramdom color data
	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_real_distribution<float> range(0.0f, 1.0f);
	for (int i = 0; i < cube_vertex_position_array.size(); i++)
	{
		cube_vertex_color_buffer_data.push_back(glm::vec3(range(mt), range(mt), range(mt)));
	}

	// cylinder color vertex object
	std::vector<glm::vec3> cylinder_vertex_color_buffer_data;
	// setting ramdom color data
	for (int i = 0; i < cylinder_vertex_position_array.size(); i++)
	{
		cylinder_vertex_color_buffer_data.push_back(glm::vec3(range(mt), range(mt), range(mt)));
	}

	// 색 버퍼 생성, 바인딩, data 복사
	GLuint cubeVertexUVBuffer;
	glGenBuffers(1, &cubeVertexUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, cube_vertex_uv_array.size() * sizeof(glm::vec2), &cube_vertex_uv_array[0], GL_STATIC_DRAW);
	GLuint cylinderVertexUVBuffer;
	glGenBuffers(1, &cylinderVertexUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVertexUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, cylinder_vertex_uv_array.size() * sizeof(glm::vec2), &cylinder_vertex_uv_array[0], GL_STATIC_DRAW);

	GLuint ProgramID = LoadShaders("TextureVertexShader.vert", "TextureFragmentShader.frag"); // shader.cpp파일 필요
	
	// 유니폼 변수 생성
	GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");
	GLuint TextureID = glGetUniformLocation(ProgramID, "TextureSampler");

	// bmp 파일에서 텍스처를 가져와 적용
	GLuint Texture = loadBMP_custom("texture/bananamilk.bmp");

	// ---- in rendering loop, variables ----- 

	// MVP 행렬 생성
	glm::mat4 View = glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.1f, 100.0f);
	glm::mat4 Model = glm::mat4(1.0f);

	// Model Translation
	glm::mat4 Translation = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 1));
	glm::mat4 Rotation = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0, 1, 0));
	glm::mat4 Scaling = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	Model = Translation * Rotation * Scaling;

	// Second Model
	glm::mat4 Translation2 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2));
	glm::mat4 Rotation2 = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0, 1, 0));
	glm::mat4 Scaling2 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	glm::mat4 Model2 = Translation2 * Rotation2 * Scaling2;

	// 3 Model
	glm::mat4 Translation3 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 5));
	glm::mat4 Rotation3 = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0, 1, 0));
	glm::mat4 Scaling3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	glm::mat4 Model3 = Translation3 * Rotation3 * Scaling3;
	
	glm::mat4 MVP = Projection * View * Model;
	glm::mat4 MVP2 = Projection * View * Model2;
	glm::mat4 MVP3 = Projection * View * Model3;
	
	// 마우스 입력 관련 초기화
	float lastTime = glfwGetTime();
	float currentTime = lastTime;
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// camera position
	glm::vec3 CamPosition = glm::vec3(4, 3, 3);
	// camera movement
	float horizontalAngle = 0.0f;
	float VerticalAngle = 0.0f;
	float mouseSpeed = 0.001f;
	float CamMoveSpeed = 0.01f;

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // depth buffer bit 초기화로 depth test 통과처리

		glUseProgram(ProgramID);

		// vertex buffer를 이용한 위치 bind, enable array, attrib pointer
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// vertex color buffer를 이용한 bind, enable array, attrib pointer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVertexUVBuffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// 키보드 조작
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			Rotation = glm::rotate(Rotation, glm::radians(1.0f), glm::vec3(0, 1, 0));
		}
		// 마우스 조작
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			Rotation = glm::rotate(Rotation, glm::radians(-1.5f), glm::vec3(0, 1, 0));
		}

		// 현재 프레임의 cursor position을 받아와서 rotation을 변경하기
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glfwSetCursorPos(window, 1024 / 2, 768 / 2); // 프레임 마지막에 cursor 고정

		horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
		VerticalAngle += mouseSpeed * float(768 / 2 - ypos);
		if (VerticalAngle > 1.5f) VerticalAngle = 1.5f;
		if (VerticalAngle < -1.5f) VerticalAngle = -1.5f;

		glm::vec3 direction = glm::vec3(cos(VerticalAngle) * sin(horizontalAngle), sin(VerticalAngle), cos(VerticalAngle) * cos(horizontalAngle));

		// 카메라 업 벡터 정의
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

		// 변환된 이동, 회전, 크기를 Model에 적용하고 Model을 MVP에 적용
		Model = Translation * Rotation * Scaling;
		MVP = Projection * View * Model;
		MVP2 = Projection * View * Model2;
		MVP3 = Projection * View * Model3;

		glUniform1i(TextureID, 0); // 유니폼 샘플러변수ID에 인덱스 0번 넣기
		glActiveTexture(GL_TEXTURE0); // 0번 텍스처 인덱스 활성화
		glBindTexture(GL_TEXTURE_2D, Texture); // 0번 텍스처 인덱스에 넣을 텍스처 바인딩

		// 유니폼 변수 데이터 입력
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, cube_vertex_position_array.size());

		// 유니폼 변수2 데이터 입력
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, cube_vertex_position_array.size());

		drawCylinder(vertexbuffer2, cylinderVertexUVBuffer, MatrixID, MVP3, cylinder_vertex_position_array.size());

		// 인덱스 0번 넣어주기 완료

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

// 함수를 vertex bind / model control / draw 단게로 나누어서 설계하자.


void drawCylinder(GLuint vertexbuffer2, GLuint vertexColorBuffer2, GLuint MatrixID, glm::mat4 MVP3, int size) {
	// cylinder를 그리기 위한 rebinding
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// vertex color buffer를 이용한 bind, enable array, attrib pointer
	glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer2);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 유니폼 변수3 데이터 입력
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, size);
}

// .obj 파일에서 버텍스 위치, 텍스처 좌표, 노멀 벡터 데이터를 가져오는 함수
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

// BMP 파일을 텍스처 형태로 가져오는 매우 간단한 함수
GLuint loadBMP_custom(const char* imagepath)
{

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char* data;

	// Open the file
	FILE* file;
	fopen_s(&file, imagepath, "rb");
	if (!file) {
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
		getchar();
		return 0;
	}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		fclose(file);
		return 0;
	}
	// A BMP files always begins with "BM"
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		fclose(file);
		return 0;
	}
	// Make sure this is a 24bpp file
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    fclose(file); return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    fclose(file); return 0; }

	// Read the information about the image
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width * height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	// Everything is in memory now, the file can be closed.
	fclose(file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete[] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}