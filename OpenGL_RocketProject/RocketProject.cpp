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
		rotation = glm::rotate(rotation, glm::radians(degree), axis);
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

	void printTranslationMat4() {
		printf("%f %f %f %f\n", translation[0][0], translation[0][1], translation[0][2], translation[0][3]);
		printf("%f %f %f %f\n", translation[1][0], translation[1][1], translation[1][2], translation[1][3]);
		printf("%f %f %f %f\n", translation[2][0], translation[2][1], translation[2][2], translation[2][3]);
		printf("%f %f %f %f\n\n", translation[3][0], translation[3][1], translation[3][2], translation[3][3]);
	}
	glm::vec3 getTranslationVector() {
		glm::vec3 vector3 = glm::vec3(translation[3][0], translation[3][1], translation[3][2]);
		return vector3;
	}

	glm::mat4 getModel() {
		return translation * rotation * scale;
	}
	glm::mat4 getMultiplayModel(glm::mat4 model) {
		return model * translation * rotation * scale;
	}
};

// global variable
glm::mat4 View = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.1f, 1000.0f);
bool rocketLaunch = false;
bool goal = false;
// Desktop
float rocketPositionEnergy = 0.1f;
float rocketKineticEnergy = -0.1f;
const float gravity = 0.0005f;
const float changeDegree = -0.3f;

float bodyDegree = -45.0f;
float headPosY = 1.2f;
float headPosY_delta = 0.13f;
float headPosZ = -0.9f;
float headPosZ_delta = 0.015f;

// Laptop
//float rocketPositionEnergy = 0.01f;
//float rocketKineticEnergy = -0.01f;
//const float gravity = 0.00001f;
//const float changeDegree = -0.05f;

//float bodyDegree = -45.0f;
//float headPosY = 1.2f;
//float headPosY_delta = 0.0013f;
//float headPosZ = -0.9f;
//float headPosZ_delta = 0.00015f;

// another funtion
TransformComponent drawObject(TransformComponent trasform, GLuint vertex_buffer, GLuint vertex_color_buffer, GLuint MatrixID, int size, GLuint texture);
TransformComponent launchRocket(TransformComponent transform, GLuint vertex_buffer, GLuint vertex_color_buffer, GLuint MatrixID, int size, GLuint texture);
TransformComponent launchRocketSubObject(TransformComponent body, TransformComponent transform, GLuint vertex_buffer, GLuint vertex_color_buffer, GLuint MatrixID, int size, GLuint texture);

int main()
{
	#pragma region window setting

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

	#pragma endregion

	#pragma region vertex buffer

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

	// each basic shape object VBO 생성 및 바인딩, vertex data 복사
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
	
	#pragma endregion

	#pragma region color vertex

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

	// 색 버퍼 생성, 바인딩, data 복사
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

	#pragma endregion

	#pragma region uv vertex

	GLuint cube_vertex_uv_buffer;
	glGenBuffers(1, &cube_vertex_uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, cube_vertex_uv_array.size() * sizeof(glm::vec3), &cube_vertex_uv_array[0], GL_STATIC_DRAW);
	GLuint cylinder_vertex_uv_buffer;
	glGenBuffers(1, &cylinder_vertex_uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cylinder_vertex_uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, cylinder_vertex_uv_array.size() * sizeof(glm::vec3), &cylinder_vertex_uv_array[0], GL_STATIC_DRAW);
	GLuint cone_vertex_uv_buffer;
	glGenBuffers(1, &cone_vertex_uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cone_vertex_uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, cone_vertex_uv_array.size() * sizeof(glm::vec3), &cone_vertex_uv_array[0], GL_STATIC_DRAW);
	GLuint triangle_vertex_uv_buffer;
	glGenBuffers(1, &triangle_vertex_uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangle_vertex_uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertex_color_buffer_data), triangle_vertex_color_buffer_data, GL_STATIC_DRAW);

	#pragma endregion

	GLuint ProgramShaderID = LoadShaders("SimpleVertexShader.vert", "SimpleFragmentShader.frag"); // shader.cpp파일 필요
	GLuint ProgramTextureID = LoadShaders("TextureVertexShader.vert", "TextureFragmentShader.frag");

	// 유니폼 변수 생성
	GLuint MatrixID = glGetUniformLocation(ProgramShaderID, "MVP");
	GLuint TextureID = glGetUniformLocation(ProgramTextureID, "TextureSampler");

	// bmp 파일에서 텍스처를 가져와 적용
	GLuint Texture_checkpattern = loadBMP_custom("texture/checkpattern.bmp");
	GLuint Texture_bananamilk = loadBMP_custom("texture/bananamilk.bmp");
	GLuint Texture_cubeTex = loadBMP_custom("texture/cubeTex.bmp");
	GLuint Texture_greenStripe = loadBMP_custom("texture/greenStripe.bmp");
	GLuint Texture_greenDiamond = loadBMP_custom("texture/greenDiamond.bmp");
	GLuint Texture_greenHexagon = loadBMP_custom("texture/greenHexagon.bmp");

	// ---- in rendering loop, variables ----- 

	// MVP 행렬 생성 -> 사용하기 편하게 전역으로 변경
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

	// 마우스 입력 관련 초기화
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
	//float CamMoveSpeed = 0.05f; // Laptop
	float CamMoveSpeed = 0.1f; // PC

	// ----- Objects Transform Component ----- 
	TransformComponent leftcarriageTransform(glm::vec3(0, 0, 1), glm::vec3(0.5f, 1.0f, 0.5f));
	TransformComponent rightcarriageTransform(glm::vec3(3, 0, 1), glm::vec3(0.5f, 1.0f, 0.5f));
	TransformComponent barrelTransform(glm::vec3(1.5f, 2.0f, 0), glm::vec3(0.09f, 0.08f, 0.09f));
	barrelTransform.rotate(-45.0f, glm::vec3(1, 0, 0));
	TransformComponent planeTransform(glm::vec3(0, -1, -25), glm::vec3(30.0f, 0.01f, 30.0f)); // ※ 바닥의 y position은 -1

	// 로켓 상대 위치 변수
	const glm::vec3 head_vec3 = glm::vec3(0, 10.2f, 0);
	const glm::vec3 fin1_vec3 = glm::vec3(12, -10.0f, 0);
	const glm::vec3 fin2_vec3 = glm::vec3(0, -10.0f, 12);
	const glm::vec3 fin3_vec3 = glm::vec3(-12, -10.0f, 0);
	const glm::vec3 fin4_vec3 = glm::vec3(0, -10.0f, -12);

	// 로켓 몸통을 중심으로 나머지는 상대위치로 설정
	TransformComponent rocketBodyTransform(glm::vec3(1.5f, 2.5f, 0.0f), glm::vec3(0.05f, 0.05f, 0.05f));
	rocketBodyTransform.rotate(bodyDegree, glm::vec3(1, 0, 0));
	TransformComponent rocketHeadTransform(
		glm::translate(glm::mat4(1), glm::vec3(0, 23.2f, 0)),
		glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0, 1, 0)),
		glm::scale(glm::mat4(1), glm::vec3(12.5f, 5.0f, 12.5f))
	);
	TransformComponent rocketFin1Transform(
		glm::translate(glm::mat4(1), fin1_vec3),
		glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0, 1, 0)),
		glm::scale(glm::mat4(1), glm::vec3(10.0f, 10.0f, 10.0f))
	);
	TransformComponent rocketFin2Transform(
		glm::translate(glm::mat4(1), fin2_vec3),
		glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0, 1, 0)),
		glm::scale(glm::mat4(1), glm::vec3(10.0f, 10.0f, 10.0f))
	);
	TransformComponent rocketFin3Transform(
		glm::translate(glm::mat4(1), fin3_vec3),
		glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0, 1, 0)),
		glm::scale(glm::mat4(1), glm::vec3(10.0f, 10.0f, 10.0f))
	);
	TransformComponent rocketFin4Transform(
		glm::translate(glm::mat4(1), fin4_vec3),
		glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0, 1, 0)),
		glm::scale(glm::mat4(1), glm::vec3(10.0f, 10.0f, 10.0f))
	);

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // depth buffer bit 초기화로 depth test 통과처리

		glUseProgram(ProgramTextureID);

		#pragma region Camera Control

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

		#pragma endregion
		
		#pragma region Cannon Launcher Object

		leftcarriageTransform = drawObject(leftcarriageTransform, cube_vertexbuffer, cube_vertex_uv_buffer, MatrixID, cube_vertex_position_array.size(), Texture_greenHexagon);
		rightcarriageTransform = drawObject(rightcarriageTransform, cube_vertexbuffer, cube_vertex_uv_buffer, MatrixID, cube_vertex_position_array.size(), Texture_greenHexagon);
		barrelTransform = drawObject(barrelTransform, cylinder_vertexbuffer, cylinder_vertex_uv_buffer, MatrixID, cylinder_vertex_position_array.size(), Texture_greenStripe);
		planeTransform = drawObject(planeTransform, cube_vertexbuffer, cube_vertex_uv_buffer, MatrixID, cube_vertex_position_array.size(), Texture_checkpattern);

		#pragma endregion

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			rocketLaunch = true;

		#pragma region Rocket Object

		// body
		rocketBodyTransform = launchRocket(rocketBodyTransform, cylinder_vertexbuffer, cylinder_vertex_uv_buffer, MatrixID, cylinder_vertex_position_array.size(), Texture_greenDiamond);
		//setRocketSubObjPosition(rocketBodyTransform, &rocketHeadTransform, &rocketFin1Transform, &rocketFin2Transform, &rocketFin3Transform, &rocketFin4Transform);

		// head
		rocketHeadTransform = launchRocketSubObject(rocketBodyTransform, rocketHeadTransform, cone_vertexbuffer, cone_vertex_color_buffer, MatrixID, cone_vertex_position_array.size(), Texture_greenDiamond);

		rocketFin1Transform = launchRocketSubObject(rocketBodyTransform, rocketFin1Transform, triangle_vertexbuffer, triangle_vertex_uv_buffer, MatrixID, sizeof(triangle_vertex_buffer_data), Texture_greenHexagon);
		rocketFin1Transform.rotate(180, glm::vec3(0, 1, 0));
		rocketFin1Transform = launchRocketSubObject(rocketBodyTransform, rocketFin1Transform, triangle_vertexbuffer, triangle_vertex_uv_buffer, MatrixID, sizeof(triangle_vertex_buffer_data), Texture_greenHexagon);
		rocketFin2Transform = launchRocketSubObject(rocketBodyTransform, rocketFin2Transform, triangle_vertexbuffer, triangle_vertex_uv_buffer, MatrixID, sizeof(triangle_vertex_buffer_data), Texture_greenHexagon);
		rocketFin2Transform.rotate(180, glm::vec3(0, 1, 0));
		rocketFin2Transform = launchRocketSubObject(rocketBodyTransform, rocketFin2Transform, triangle_vertexbuffer, triangle_vertex_uv_buffer, MatrixID, sizeof(triangle_vertex_buffer_data), Texture_greenHexagon);
		rocketFin3Transform = launchRocketSubObject(rocketBodyTransform, rocketFin3Transform, triangle_vertexbuffer, triangle_vertex_uv_buffer, MatrixID, sizeof(triangle_vertex_buffer_data), Texture_greenHexagon);
		rocketFin3Transform.rotate(180, glm::vec3(0, 1, 0));
		rocketFin3Transform = launchRocketSubObject(rocketBodyTransform, rocketFin3Transform, triangle_vertexbuffer, triangle_vertex_uv_buffer, MatrixID, sizeof(triangle_vertex_buffer_data), Texture_greenHexagon);
		rocketFin4Transform = launchRocketSubObject(rocketBodyTransform, rocketFin4Transform, triangle_vertexbuffer, triangle_vertex_uv_buffer, MatrixID, sizeof(triangle_vertex_buffer_data), Texture_greenHexagon);
		rocketFin4Transform.rotate(180, glm::vec3(0, 1, 0));
		rocketFin4Transform = launchRocketSubObject(rocketBodyTransform, rocketFin4Transform, triangle_vertexbuffer, triangle_vertex_uv_buffer, MatrixID, sizeof(triangle_vertex_buffer_data), Texture_greenHexagon);

		#pragma endregion

		glUniform1i(TextureID, 0); // 유니폼 샘플러변수ID에 인덱스 0번 넣기
		glActiveTexture(GL_TEXTURE0); // 0번 텍스처 인덱스 활성화

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

TransformComponent drawObject(TransformComponent transform, GLuint vertex_buffer, GLuint vertex_color_buffer, GLuint MatrixID, int size, GLuint texture) {
	glm::mat4 Model = transform.translation * transform.rotation * transform.scale;
	glm::mat4 MVP = Projection * View * Model;

	//
	glm::mat4 CylinderLocalModel;
	glm::mat4 ConeLocalModel;
	glm::mat4 ConeWorldModel = CylinderLocalModel * ConeLocalModel;
	//

	// vertex buffer를 이용한 위치 bind, enable array, attrib pointer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// vertex color buffer를 이용한 bind, enable array, attrib pointer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_color_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindTexture(GL_TEXTURE_2D, texture); // 0번 텍스처 인덱스에 넣을 텍스처 바인딩

	// 유니폼 변수 데이터 입력
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, size);

	return transform;
}

TransformComponent launchRocket(TransformComponent transform, GLuint vertex_buffer, GLuint vertex_color_buffer, GLuint MatrixID, int size, GLuint texture) {
	if (rocketLaunch && !goal) {
		rocketLaunch = true;
		transform.translation = glm::translate(transform.translation, glm::vec3(0, rocketPositionEnergy, rocketKineticEnergy));
		transform.rotation = glm::rotate(transform.rotation, glm::radians(changeDegree), glm::vec3(1, 0, 0));
		bodyDegree += changeDegree;
		rocketPositionEnergy -= gravity;
		if (transform.getTranslationVector().y < 0.01f) {
			goal = true;
		}
	}

	glm::mat4 Model = transform.translation * transform.rotation * transform.scale;
	glm::mat4 MVP = Projection * View * Model;

	// vertex buffer를 이용한 위치 bind, enable array, attrib pointer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// vertex color buffer를 이용한 bind, enable array, attrib pointer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_color_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindTexture(GL_TEXTURE_2D, texture); // 0번 텍스처 인덱스에 넣을 텍스처 바인딩

	// 유니폼 변수 데이터 입력
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, size);

	return transform;
}

TransformComponent launchRocketSubObject(TransformComponent body, TransformComponent transform, GLuint vertex_buffer, GLuint vertex_color_buffer, GLuint MatrixID, int size, GLuint texture) {
	glm::mat4 Model = transform.getMultiplayModel(body.getModel());
	glm::mat4 MVP = Projection * View * Model;

	// vertex buffer를 이용한 위치 bind, enable array, attrib pointer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// vertex color buffer를 이용한 bind, enable array, attrib pointer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_color_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindTexture(GL_TEXTURE_2D, texture); // 0번 텍스처 인덱스에 넣을 텍스처 바인딩

	// 유니폼 변수 데이터 입력
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, size);

	return transform;
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