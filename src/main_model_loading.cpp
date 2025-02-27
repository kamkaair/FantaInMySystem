#include "camera.h"			// Include Camera-class.
#include "mesh.h"
#include "material.h"
#include "utils.h"			// Utility functions
#include "UI.h"
#include "HDRI.h"
#include "textureLoading.h"

#include "glm/gtx/string_cast.hpp" // Include for printing mats and vecs
#include <glm/gtc/type_ptr.hpp>

//#include <glad/gl.h>				// Include glad
//#include <kgfw/GLUtils.h>			// Include GLUtils for checkGLError
//#include <kgfw/Object.h>			// Include kgfw::Object
#include <GLFW/glfw3.h>				// Include glfw for windows

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <unordered_map>

// Include STB-image library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class Application : public kgfw::Object
{
public:
	Application()
		: Object(__FUNCTION__)
		, m_shader(0)
		, m_cubemapShader(0)
		, m_BackgroundShader(0)
		, m_IrradianceShader(0)
		, m_Prefilter(0)
		, m_brdf(0)
		, m_icon(0)
		, m_backImage(0)
		, m_uiDraw(nullptr) // Initialize UI pointer
		, m_HDRI(nullptr)
		, m_texLoading(nullptr)
		, m_camera(nullptr)
	{
		bindShaders();

		// texloading function class
		m_texLoading = new TextureLoading();

		// Loads and computes all the HDRI maps
		m_HDRI = new HDRI(m_cubemapShader, m_BackgroundShader, m_IrradianceShader, m_Prefilter, m_brdf);

		// the UI class, contains ImGui and such
		m_uiDraw = new UI(m_shader, m_backImage, m_texLoading, m_HDRI);

		m_BackgroundShader->bind();
		m_BackgroundShader->setUniform("environmentMap", 0);

		// Create perspective-projection camera with screen size 640x480
		m_camera = new Camera(fov, 640/480, 0.1f, 100.0f);

		// Enable seamless cubemaps
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// Load and push back textures/materials
		// I recommend loading materials and meshes before loading a HDRTexture. Both stbi_set_flip_vertically_on_load(true) and even after setting it to (false) seem to screw up UV-maps
		m_texLoading->loadMaterials();
		m_texLoading->loadAllMeshes(m_uiDraw->getMeshes());

		// Load the texture for an icon
		Texture* iconTexture = m_texLoading->loadTexture((std::string(ASSET_DIR) + "/textures/LightBulbLitOutline.png").c_str());
		m_iconTexture = iconTexture;

		// stbi_set_flip_vertically_on_load(true);
		// Load the texture for the background texture
		Texture* backgroundImage = m_texLoading->loadTexture((std::string(ASSET_DIR) + "/textures/checkerboard.png").c_str());
		m_HDRI->setBackgroundTexture(backgroundImage);

		// Load the HDR texture and create all the HDRI maps
		m_HDRI->ProcessHDRI("/HDRI/newport_loft.hdr");

		// Set up lights and color
		initializeLights();

		// Alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Enable depth buffering
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		
		//Back face culling
		//glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void printCombinations(std::string str1, std::string str2) {
		std::cout << str1 << " " << str2 << std::endl;
	}

	~Application() {
		// Delete shaders
		delete m_shader;
		m_shader = 0;

		delete m_icon;
		m_icon = 0;

		delete m_backImage;
		m_backImage = 0;

		// Delete references
		delete m_HDRI;
		m_HDRI = 0;

		delete m_uiDraw;
		m_uiDraw = 0;

		delete m_texLoading;
		m_texLoading = 0;

		// Delete Camera
		delete m_camera;
		m_camera = 0;

		//Delete all textures
		for (Texture* texture : m_textures)
		{
			delete texture;
		}
		m_textures.clear();

		// Delete icon texture
		delete m_iconTexture;
		m_iconTexture = 0;

		//Destroy ImGui
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void bindShaders() {
		// Load the main vertex and fragment shaders
		std::string vertexShaderSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/vertShader.glsl");
		std::string fragmentShaderSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/fragShader.glsl");

		// Build and compile our shader program
		m_shader = new Shader(vertexShaderSource, fragmentShaderSource);

		// Load the cubemap shaders
		std::string CubemapVertexSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/cubemap_vert.glsl");
		std::string CubemapFragmentSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/cubemap_frag.glsl");
		m_cubemapShader = new Shader(CubemapVertexSource, CubemapFragmentSource);

		// Load the background shaders
		std::string backgroundVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/backgroundVert.glsl");
		std::string backgroundFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/backgroundFrag.glsl");
		m_BackgroundShader = new Shader(backgroundVertSource, backgroundFragSource);

		// Load the irradiance shaders
		std::string irradianceVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/cubemap_vert.glsl");
		std::string irradianceFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/irradianceFrag.glsl");
		m_IrradianceShader = new Shader(irradianceVertSource, irradianceFragSource);

		// Load the prefilter map shaders
		std::string prefilterVert = utils::loadShader(std::string(ASSET_DIR) + "/shaders/cubemap_vert.glsl");
		std::string prefilterFrag = utils::loadShader(std::string(ASSET_DIR) + "/shaders/prefilterFrag.glsl");
		m_Prefilter = new Shader(prefilterVert, prefilterFrag);

		// Load the brdf map shaders
		std::string brdfVert = utils::loadShader(std::string(ASSET_DIR) + "/shaders/brdfVert.glsl");
		std::string brdfFrag = utils::loadShader(std::string(ASSET_DIR) + "/shaders/brdfFrag.glsl");
		m_brdf = new Shader(brdfVert, brdfFrag);

		// Load the background shaders
		std::string iconVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/iconVert.glsl");
		std::string iconFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/iconFrag.glsl");
		m_icon = new Shader(iconVertSource, iconFragSource);

		// Load the background shaders
		std::string BackImageVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/backgroundImageVert.glsl");
		std::string BackImageFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/backgroundImageFrag.glsl");
		m_backImage = new Shader(BackImageVertSource, BackImageFragSource);
	}

	void render(GLFWwindow* window) {

		// Query the size of the framebuffer (window content) from glfw.
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// Get ratiod idiot
		m_camera->setAspectRatio(width, height);

		// Framebuffer callback for preserving aspect ratio
		framebuffer_size_callback(window, width, height);

		// Setup the opengl wiewport (specify area to draw)
		glViewport(0, 0, width, height);
		checkGLError();

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		checkGLError();

		// Use other shaders for the rest of the scene...
		glUseProgram(0); // Unbind any active shader
		m_shader->bind();

		// Clear the depth buffer to ensure the skybox doesn't interfere with object rendering
		//glClear(GL_DEPTH_BUFFER_BIT);

		// Render skybox, the background image or clear color
		switch (m_uiDraw->getBackgroundMode()) {
		case 0:
			m_HDRI->renderSkybox(m_camera);
			break;
		case 1:
			m_HDRI->renderBackgroundImage(m_camera, m_HDRI->getBackgroundTexture(), m_backImage);
			break;
		case 2:
			// The second case won't run background renders
			break;
		//default:
			// m_HDRI->renderSkybox(m_camera);
		}

		if (!m_uiDraw->getMeshes().empty()) {
			std::vector<GLuint> textureIds;
			for (Texture* texture : m_textures) {
				textureIds.push_back(texture->getTextureId());
			}

			for (Mesh* mesh : m_uiDraw->getMeshes()) {
				m_HDRI->setHDRITextures(m_shader);
				mesh->Render(m_shader, m_camera->getPosition(), m_uiDraw->getPointLightPos(), m_uiDraw->getPointLightColor(), m_camera->getViewMatrix(), m_camera->getModelMatrix(), m_camera->getProjectionMatrix());
			}
		}

		if (!isHidden) {
			renderIcons();
			m_uiDraw->ImGuiDraw(); // Render the ImGui window
		}
	}

	void initializeLights()
	{
		// Default point light positions
		m_uiDraw->getPointLightPos().push_back(glm::vec3(-2.72f, 1.20f, 3.68f));
		m_uiDraw->getPointLightPos().push_back(glm::vec3(2.70, 1.50, 3.10));
		m_uiDraw->getPointLightPos().push_back(glm::vec3(0.30f, 3.10f, -5.80f));

		// Default point light colors
		m_uiDraw->getPointLightColor().push_back(glm::vec3(0.07f, 0.18f, 1.00f));
		m_uiDraw->getPointLightColor().push_back(glm::vec3(0.77f, 0.11f, 0.91f));
		m_uiDraw->getPointLightColor().push_back(glm::vec3(0.10f, 0.89f, 0.5f));
	}

	void renderIcons()
	{
		for (size_t i = 0; i < m_uiDraw->getPointLightPos().size(); i++) {
			//glm::vec3 directionToCamera = glm::normalize(cameraPos - m_uiDraw->getPointLightPos()[i]);
			//directionToCamera.y = 0.0f; // Only if you want the plane to stay vertical

			// Scaling depending on the distance
			float iconDistance = glm::length(cameraPos - m_uiDraw->getPointLightPos()[i]);

			// Divide the scale by the icon's distance by the iconSize. Icon's scale will stay the same regardless of the position of the camera.
			float iconSize = 25.0f;
			float scale = iconDistance / iconSize;
			//float iconSize = 2.0f;
			//float scale = iconSize / iconDistance;

			// Model matrix for the lamp quad
			glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), m_uiDraw->getPointLightPos()[i]) * glm::scale(glm::mat4(1.0f), glm::vec3(scale));

			// Match the camera rotation to the icon's rotation
			glm::mat4 cameraRotation = glm::mat4(glm::mat3(m_camera->getViewMatrix()));
			modelMatrix *= glm::inverse(cameraRotation);
			// Flip the icon, since it's upside down
			modelMatrix *= glm::mat4(glm::mat3(-1.0f));

			m_icon->bind();
			m_icon->setUniform("projection", m_camera->getProjectionMatrix());
			m_icon->setUniform("view", m_camera->getViewMatrix());
			m_icon->setUniform("model", modelMatrix);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_iconTexture->getTextureId());

			m_renderCube->renderQuad();
		}
	}

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

		if (app && app->mouseEnabled == false) {

			//Min FOV
			app->fov -= (float)yoffset;
			if (app->fov < 1.0f)
				app->fov = 1.0f;

			//Max FOV
			if (app->fov > 45.0f)
				app->fov = 45.0f;

			// Apply the new FOV to the camera
			app->m_camera->setFOV(app->fov);

			//std::cout << "New FOV: " << app->fov << std::endl;
		}
	}

	static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
	{
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

		if (app && app->mouseEnabled == false) {
			float xpos = static_cast<float>(xposIn);
			float ypos = static_cast<float>(yposIn);

			if (app->firstMouse) {
				app->lastX = xpos;
				app->lastY = ypos;
				app->firstMouse = false;
			}

			float xoffset = xpos - app->lastX;
			float yoffset = app->lastY - ypos; // reversed since y-coordinates go from bottom to top
			app->lastX = xpos;
			app->lastY = ypos;

			//Mouse sensitivity
			float sensitivity = 0.1f;
			xoffset *= sensitivity;
			yoffset *= sensitivity;

			app->yaw += xoffset;
			app->pitch += yoffset;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (app->pitch > 89.0f)
				app->pitch = 89.0f;
			if (app->pitch < -89.0f)
				app->pitch = -89.0f;

			// update the front vector
			glm::vec3 front;
			front.x = cos(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
			front.y = sin(glm::radians(app->pitch));
			front.z = sin(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
			app->cameraFront = glm::normalize(front);

			//std::cout << glm::to_string(app->cameraFront) << std::endl;
		}
	}

	static void focus_callback(GLFWwindow* window, Application* app) {
		//Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

		// Toggle mouse cursor with 'E'
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !app->togglePressed && !ImGui::GetIO().WantTextInput) {
			app->togglePressed = true;
			app->mouseEnabled = !app->mouseEnabled; // Set mouseEnabled to what it's not

			if (app->mouseEnabled) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				app->m_uiDraw->setImGuiAlpha(0.9f);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				app->m_uiDraw->setImGuiAlpha(0.3f);
				app->firstMouse = true;
			}
		}
		// Using GLFW_RELEASE to avoid detecting multiple presses for the toggle
		else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE && !ImGui::GetIO().WantTextInput)
			app->togglePressed = false;
	}

	static void hide_callback(GLFWwindow* window, Application* app) {
		// Flip-flop for setting ImGui window hidden
		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !app->togglePressedHide && !ImGui::GetIO().WantTextInput) {
			app->togglePressedHide = true;
			//m_uiDraw->toggleIsHidden();
			app->isHidden = !app->isHidden;
		}
		else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE && !ImGui::GetIO().WantTextInput)
			app->togglePressedHide = false;
	}

	//static void movement_callback()

	void update(float deltaTime, GLFWwindow* window) {
		//Mesh rotation
		if (m_uiDraw->boolMeshRotation()) {
			if (!m_uiDraw->boolDoOnce()) {
				for (auto meshes : m_uiDraw->getMeshes()) { // Set rotation to 0.0f once
					meshes->setRotationX(0.0f), meshes->setRotationY(0.0f), meshes->setRotationZ(0.0f);
				}
				m_uiDraw->toggleDoOnce();
			}
			for (auto meshes : m_uiDraw->getMeshes()) { // Rotation loop
				meshes->setRotationX(meshes->getRotationX() + deltaTime);
			}
		}
		else if (!m_uiDraw->boolMeshRotation() && m_uiDraw->boolDoOnce())
		{
			for (auto meshes : m_uiDraw->getMeshes()) { // Set rotation to 0.0f, when enabling rotation
				meshes->setRotationX(0.0f), meshes->setRotationY(0.0f), meshes->setRotationZ(0.0f);
			}
			m_uiDraw->toggleDoOnce();
		}

		// Added to check, if we've got text inputs active. Don't go inside == true
		if (!ImGui::GetIO().WantTextInput)
		{
			// Camera movement
			// "Sprint"
			float cameraSpeed = 1.0 * deltaTime;
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				cameraSpeed = 2.5 * deltaTime;

			// Movement controls
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				cameraPos += cameraSpeed * cameraFront;
			else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				cameraPos -= cameraSpeed * cameraFront;
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
				cameraPos += cameraSpeed * cameraUp;
			else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				cameraPos -= cameraSpeed * cameraUp;
		}

		//Update camera position and LookAt direction
		m_camera->setPosition(cameraPos);
		m_camera->setLookAt(cameraPos + cameraFront);

		//Update view matrix
		m_camera->setViewMatrix(cameraPos + cameraFront);

		//m_camera->getProjectionMatrix();
	}

private:
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	// Shaders
	Shader* m_shader;				// Pointer to the Shader object
	Shader* m_cubemapShader;		// Pointer to the Shader object
	Shader* m_BackgroundShader;		// Pointer to the Shader object
	Shader* m_IrradianceShader;
	Shader* m_Prefilter;
	Shader* m_brdf;
	Shader* m_icon;
	Shader* m_backImage;

	// Class references
	Camera*         			m_camera;
	//Texture*					m_textureBack;			// Texture pointer
	Texture*					m_iconTexture;
	Mesh*						m_renderCube;
	UI*							m_uiDraw;
	HDRI*						m_HDRI;
	TextureLoading*				m_texLoading;

	GLuint						skyboxVAO = 0, skyboxVBO = 0, skyboxEBO = 0;
	GLuint						m_texture;

	// Camera movement
	glm::vec3 cameraPos =		glm::vec3(0.0f, 0.5f, 1.0f), cameraFront = glm::vec3(0.0f, 0.0f, -1.0f), cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	// Is mouse active?
	bool firstMouse = true;
	bool mouseEnabled = false;
	float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float pitch = 0.0f;
	float lastX = 800.0f / 2.0;
	float lastY = 600.0 / 2.0;
	float fov = 40.0f;

	std::vector<Texture*>		m_textures;		// Vector of texture pointers

	bool togglePressed =		false;
	bool togglePressedHide =	false;
	bool isHidden =				false;
};

// Global pointer to the application
Application* g_app = 0;

void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::cout << "OpenGL Debug Message: " << message << std::endl;
}

int main(void) {
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	// Set c++-lambda as error call back function for glfw.
	glfwSetErrorCallback([](int error, const char* description) {
		fprintf(stderr, "Error %d: %s\n", error, description);
	});

	// Try to initialize glfw
	if (!glfwInit()) {
		return -1;
	}

	// Create window and check that creation was succesful.
	GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Craziness", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	// Set current context
	glfwMakeContextCurrent(window);
	// Load GL functions using glad
	gladLoadGL(glfwGetProcAddress);

	// Create application
	g_app = new Application();

	glClearColor(0.2, 0.2, 0.2, 1.0);

	// Disable cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the window's user pointer to our Application instance
	glfwSetWindowUserPointer(window, g_app);

	// Mouse callback
	glfwSetCursorPosCallback(window, Application::mouse_callback);

	// Mouse scroll callback
	glfwSetScrollCallback(window, Application::scroll_callback);

	// Specify the key callback as c++-lambda to glfw
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		// Close window if escape is pressed by the user.
		//if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		//	glfwSetWindowShouldClose(window, GLFW_TRUE);
		//}

		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_E:
			Application::focus_callback(window, g_app);
			break;
		case GLFW_KEY_H:
			Application::hide_callback(window, g_app);
			break;
		}

		// then before rendering, configure the viewport to the original framebuffer's screen dimensions
		int scrWidth, scrHeight;
		glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
		glViewport(0, 0, scrWidth, scrHeight);
	});

	//ImGui initialization
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Get time using glfwGetTime-function, for delta time calculation.
	float prevTime = (float)glfwGetTime();
	while (!glfwWindowShouldClose(window)) {

		// Render the game frame and swap OpenGL back buffer to be as front buffer.
		g_app->render(window);
		glfwSwapBuffers(window);

		// Poll other window events.
		glfwPollEvents();

		// Compute application frame time (delta time) and update application
		float curTime = (float)glfwGetTime();
		float deltaTime = curTime - prevTime;
		prevTime = curTime;
		g_app->update(deltaTime, window);
	}

	// Delete application
	delete g_app;
	g_app = 0;

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glDebugOutput, nullptr);

	// Destroy window
	glfwDestroyWindow(window);

	// Terminate glfw
	glfwTerminate();

	return 0;
}