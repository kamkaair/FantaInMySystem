#include "camera.h"			// Include Camera-class.
#include "models.h"
#include "material.h"
#include "utils.h"			// Utility functions, has 
#include "UI.h"
#include "HDRI.h"
#include "inputs.h"
#include "GBuffer.h"
#include "icon.h"
#include "ScreenSpace.h"
#include "savefile.h"

// Include STB-image library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Inputs* g_input = 0;
int width = 640, height = 480;

class Application : public kgfw::Object
{
public:
	Application()
		: Object(__FUNCTION__)
		, m_cubemapShader(0)
		, m_BackgroundShader(0)
		, m_IrradianceShader(0)
		, m_Prefilter(0)
		, m_brdf(0)
		, m_icon(0)
		, m_backImage(0)
		, m_uiDraw(nullptr)
		, m_HDRI(nullptr)
		, m_camera(nullptr)
	{
		bindShaders();

		// Creates GBuffer
		m_GBuffer = new GBuffer(width, height);

		// Screen Spaced Ambient Occlusion initialization
		m_ssaoClass = new ScreenSpace(m_GBuffer);

		// Enable seamless cubemaps
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// Resource manager
		m_resoManager = new ResourceManager();
		m_scene = m_resoManager->getScene();

		// Loads and computes all the HDRI maps
		// TODO: move HDRI shaders into HDRI class
		m_HDRI = m_scene->createHDRI(m_cubemapShader, m_BackgroundShader, m_IrradianceShader, m_Prefilter, m_brdf);

		// the UI class, contains ImGui and such
		m_uiDraw = new UI(m_backImage, m_HDRI, m_GBuffer, m_ssaoClass, m_resoManager);

		m_BackgroundShader->bind();
		m_BackgroundShader->setUniform("environmentMap", 0);

		// Create perspective-projection camera
		const int fov = 40.0f;
		m_camera = new Camera(fov, width/height, 0.1f, 100.0f);
		m_scene->setActiveCamera(m_camera);

		// Input class
		g_input = new Inputs(m_uiDraw, m_camera);

		// Setup the default save, load the scene from a file
		setupDefaultSave();
		m_resoManager->fileLoad("demoScene.bin");

		// Icon class initialization
		m_iconClass = new Icon(m_meshRender, m_resoManager, m_camera);

		// Load the texture for an icon
		m_iconClass->loadIconTexture("/textures/LightBulbLitOutline.png");	// 0
		m_iconClass->loadIconTexture("/textures/crosshair.png");			// 1

		// Alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Enable depth buffering
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		
		//Back face culling
		glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
	}

	~Application() {
		// Delete shaders
		utils::deleteObject(m_icon);
		utils::deleteObject(m_backImage);

		// Delete references
		utils::deleteObject(m_HDRI);
		utils::deleteObject(m_uiDraw);
		utils::deleteObject(g_input);
		utils::deleteObject(m_GBuffer);
		utils::deleteObject(m_iconClass);
		utils::deleteObject(m_ssaoClass);
		utils::deleteObject(m_scene);
		utils::deleteObject(m_resoManager);

		// Delete Camera
		utils::deleteObject(m_camera);

		//Destroy ImGui
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void setupDefaultSave() {
		// Preset light positions, colors and light strength
		const std::string backgroundTex = "/textures/checkerboard.png", hdriPath = "/HDRI/newport_loft.hdr";

		std::vector<FileLights> fileLights;
		fileLights.push_back(FileLights{ glm::vec3(-2.72f, 1.20f, 3.68f), glm::vec3(0.07f, 0.18f, 1.00f), 4.0f });
		fileLights.push_back(FileLights{ glm::vec3(2.70, 1.50, 3.10), glm::vec3(0.77f, 0.11f, 0.91f), 2.0f });
		fileLights.push_back(FileLights{ glm::vec3(0.30f, 3.10f, -5.80f), glm::vec3(0.10f, 0.89f, 0.5f), 6.0f });

		std::vector<MaterialPaths> materialPath; // Path, use map and value
		materialPath.push_back(MaterialPaths{std::string("CheckerboardFloor"),
			useTexture<glm::vec3>("/textures/checkerboard.png"),			// 1. Diffuse
			useTexture<float>("/textures/checkerboard.png"),				// 2. Metallic
			useTexture<float>("/textures/checkerboard.png"),				// 3. Roughness
			useValue<float>(0.0f),											// 4. Emission
			useTexture<float>("/textures/blending_window.png"),				// 5. Normal
			useTexture<std::string>("/textures/checkerboardNormal.png") });	// 6. Opacity

		materialPath.push_back(MaterialPaths{ std::string("Lantern"),
			useTexture<glm::vec3>("/textures/OldLantern/Lantern_Diffuse.jpg"),
			useTexture<float>("/textures/OldLantern/Lantern_Metallic.jpg"),
			useTexture<float>("/textures/OldLantern/Lantern_Roughness.jpg"),
			useTexture<float>("/textures/OldLantern/Lantern_Emissive.jpg", 2.0f),
			useValue<float>(1.0f),
			useTexture<std::string>("/textures/OldLantern/Lantern_Normal.png") });	 

		materialPath.push_back(MaterialPaths{ std::string("MP18_Material"),
			useTexture<glm::vec3>("/textures/MP18/Metallic_Diffuse.png"),
			useTexture<float>("/textures/MP18/Metallic_Metallic.png"),
			useTexture<float>("/textures/MP18/Metallic_Roughness.png"),
			useValue<float>(0.0f),
			useValue<float>(1.0f),
			useTexture<std::string>("/textures/MP18/Metallic_Normal.png") }); // Tiny bit transparent	

		materialPath.push_back(MaterialPaths{std::string("Barrel_Material"),
			useTexture<glm::vec3>("/textures/Barrel/Barrel_Diffuse.png"),
			useTexture<float>("/textures/Barrel/Barrel_Metallic.png"),
			useTexture<float>("/textures/Barrel/Barrel_Roughness.png"),
			useValue<float>(0.0f),
			useValue<float>(1.0f),
			useTexture<std::string>("/textures/Barrel/Barrel_Normal.png") }); // A bitt transparnet hege 

		std::vector<FileModels> fileModels;
		fileModels.push_back({ "/models/plane.obj",{{"Plane", glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(6.0f),glm::vec3(0.0f), 0}} });
		fileModels.push_back({ "/models/old_lantern.obj",{{"OldLantern", glm::vec3(0.0f, -1.0f, -3.0f), glm::vec3(5.0f),glm::vec3(0.0f), 1}} });
		fileModels.push_back({ "/models/MP18Low.obj",{{"MP18", glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f),glm::vec3(0.0f), 2}} });
		fileModels.push_back({ "/models/barrel.obj",{{"Barrel", glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1.0f),glm::vec3(0.0f), 3}} });

		float radius = 10.0f, theta = 0.0f, phi = 3.14159265359f / 4.0f;
		float pitch = 0.0f, yaw = -90.0, lastX = 800.0f / 2.0, lastY = 600.0 / 2.0;
		double xPos = 0.0f, yPos = 0.0f;

		// Default values for the camera position
		FileCamera fileCamera({ glm::vec3(0.0f, 0.5f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			radius, theta, phi,
			pitch, yaw, lastX, lastY, xPos, yPos, false });
		
		// Create and serialize an object
		SaveFile original(fileLights, materialPath, fileModels, fileCamera, backgroundTex, hdriPath);
		original.serialize(std::string(ASSET_DIR) + "/Saves/demoScene.bin");
	}

	void bindShaders() {
		m_cubemapShader = utils::makeShader("cubemap_vert.glsl", "cubemap_frag.glsl");
		m_BackgroundShader = utils::makeShader("backgroundVert.glsl", "backgroundFrag.glsl");
		m_IrradianceShader = utils::makeShader("cubemap_vert.glsl", "irradianceFrag.glsl");
		m_Prefilter = utils::makeShader("cubemap_vert.glsl", "prefilterFrag.glsl");
		m_brdf = utils::makeShader("brdfVert.glsl", "brdfFrag.glsl");
		m_icon = utils::makeShader("iconVert.glsl", "iconFrag.glsl");
		m_backImage = utils::makeShader("backgroundImageVert.glsl", "backgroundImageFrag.glsl");
	}

	void render(GLFWwindow* window) {
		!m_uiDraw->getRenderMode() ? forwardRendering(window) : deferredRendering(window);
	}

	void forwardRendering(GLFWwindow* window) {		
		glfwGetFramebufferSize(window, &width, &height); // Query the size of the framebuffer (window content) from glfw.
		m_camera->setAspectRatio(width, height); // Get ratiod idiot
		framebuffer_size_callback(window, width, height); // Framebuffer callback for preserving aspect ratio

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		checkGLError();

		glUseProgram(0); // Unbind any active shader
		m_GBuffer->getForwardShader()->bind();

		// 1. Render skybox, the background image or clear color
		glDisable(GL_DEPTH_TEST);
		switch (m_uiDraw->getBackgroundMode()) {
		case 0: m_HDRI->renderSkybox(m_camera); break;
		case 1: m_HDRI->renderBackgroundImage(m_camera, m_HDRI->getBackgroundTexture(), m_backImage); break;
		}
		glEnable(GL_DEPTH_TEST);

		// 2. Render opaque objects
		if (!m_scene->getModels().empty()) {
			m_scene->sortTransparentMeshes();
			for (Mesh* mesh : m_scene->getOpaqueMeshes()) {
				m_HDRI->setHDRITextures(m_GBuffer->getForwardShader());
				mesh->Render(m_GBuffer->getForwardShader(), m_camera, m_scene->getLights());
			}

			// 3. Render transparent objects
			glDepthMask(GL_FALSE); // Disabled depth mask, the results look pretty cool
			for (auto& trans : m_scene->getTransparentMeshes()) {
				m_HDRI->setHDRITextures(m_GBuffer->getForwardShader());
				trans.second->Render(m_GBuffer->getForwardShader(), m_camera, m_scene->getLights());
			}
			glDepthMask(GL_TRUE);
		}

		// 4. Render icons and UI
		if (!g_input->getImGuiVisibility()) {
			m_iconClass->renderIcons(m_icon, 25.0f, m_scene->getLights(), 0);
			m_iconClass->renderIcons(m_icon, 100.0f, m_camera->cameraFocus, 1);
			m_uiDraw->ImGuiDraw();
		}
	}

	void deferredRendering(GLFWwindow* window) {
		glfwGetFramebufferSize(window, &width, &height);
		m_camera->setAspectRatio(width, height);
		m_GBuffer->setResolution(width, height);
		framebuffer_size_callback(window, width, height);
		checkGLError();

		// Clear everything from the default fb
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		// 1. Geometry pass: render scene's geometry/color data into gbuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer->getGBuffer());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!m_scene->getModels().empty()) {
			for (Mesh* mesh : m_scene->getOpaqueMeshes()) {
				mesh->RenderGBuffer(m_GBuffer->getGeometryPass(), m_camera);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. Screen Space Ambient Occlusion pass
		if (m_ssaoClass->getSSAO_Settings().useSSAO)
			m_ssaoClass->renderSSAO(m_camera, m_meshRender, 64);

		// 3. Lighting pass
		deferredLightPass();

		// 4. Screen Space Reflection pass
		if (m_ssaoClass->getSSR_Settings().useSSR)
			m_ssaoClass->renderSSR(m_camera, m_meshRender);

		// 5. Render the final image
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer->getCompositeFBO());

		// 5.1 Render the background
		switch (m_uiDraw->getBackgroundMode()) {
			case 0: m_HDRI->renderSkybox(m_camera); break;
			case 1: m_HDRI->renderBackgroundImage(m_camera, m_HDRI->getBackgroundTexture(), m_backImage); break;
		}

		// 5.2 Composite shader (includes deferred post processing)
		m_ssaoClass->renderCompositeShader(m_meshRender); // Into the default framebuffer

		// 5.3 Transparent meshes
		renderTransparentPass();

		// 5.4 Copy the composite shader's colorbuffer into the default fb
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->getCompositeFBO());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// 6. UI into the default fb
		if (!g_input->getImGuiVisibility())
			m_uiDraw->ImGuiDraw();
	}

	void renderTransparentPass() {
		if (!m_scene->getModels().empty()) {
			glEnable(GL_BLEND); // Enable blending for the transparency and don't use depth mask
			glDepthMask(GL_FALSE);

			// Render transparent objects
			m_scene->sortTransparentMeshes();
			for (auto& trans : m_scene->getTransparentMeshes()) {
				m_HDRI->setHDRITextures(m_GBuffer->getForwardShader());
				trans.second->Render(m_GBuffer->getForwardShader(), m_camera, m_scene->getLights());
			}

			// Render transparent icons with the blending and depth
			if (!g_input->getImGuiVisibility()) {
				m_iconClass->renderIcons(m_icon, 25.0f, m_scene->getLights(), 0);
				m_iconClass->renderIcons(m_icon, 100.0f, m_camera->cameraFocus, 1);
			}			

			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}
	}

	void deferredLightPass() {
		// Get baked lighting
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer->getLightingFBO());
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		// LIGHT PASS
		m_GBuffer->getLightPass()->bind();
		m_HDRI->setHDRITextures(m_GBuffer->getLightPass());

		utils::bindTexture(GL_TEXTURE3, m_GBuffer->getLightPass(), m_GBuffer->getGPosition(), "gPosition");
		utils::bindTexture(GL_TEXTURE4, m_GBuffer->getLightPass(), m_GBuffer->getGNormal(), "gNormal");
		utils::bindTexture(GL_TEXTURE5, m_GBuffer->getLightPass(), m_GBuffer->getGAlbedo(), "gAlbedoSpec");
		utils::bindTexture(GL_TEXTURE6, m_GBuffer->getLightPass(), m_GBuffer->getGMetallicRoughness(), "gMetallicRoughness");
		utils::bindTexture(GL_TEXTURE7, m_GBuffer->getLightPass(), m_ssaoClass->getSsaoBlurColorBuffer(), "uSSAO");

		// Set light uniforms + view
		for (int i = 0; i < m_scene->getLights().size(); i++) {
			glm::vec3 lightPosWorld = m_scene->getLights()[i].pos;
			glm::vec3 lightPosView = glm::vec3(m_camera->getViewMatrix() * glm::vec4(lightPosWorld, 1.0f));

			m_GBuffer->getLightPass()->setUniform("pointLights[" + std::to_string(i) + "].position", lightPosView);

			m_GBuffer->getLightPass()->setUniform("pointLights[" + std::to_string(i) + "].color", m_scene->getLights()[i].color);

			// Set attenuation factors for the point light
			m_GBuffer->getLightPass()->setUniform("pointLights[" + std::to_string(i) + "].constant", 1.0f);
			m_GBuffer->getLightPass()->setUniform("pointLights[" + std::to_string(i) + "].linear", 0.09f);
			m_GBuffer->getLightPass()->setUniform("pointLights[" + std::to_string(i) + "].quadratic", 0.032f);
			m_GBuffer->getLightPass()->setUniform("pointLights[" + std::to_string(i) + "].strength", m_scene->getLights()[i].strength);
		}

		m_GBuffer->getLightPass()->setUniform("NUM_POINT_LIGHTS", (int)m_scene->getLights().size());
		if (m_uiDraw->getLightOrientation())
			m_GBuffer->getLightPass()->setUniform("inverseView", glm::inverse(m_camera->getViewMatrix()));

		// Render quad, applies the lighting pass
		m_meshRender->renderQuad();
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		if (g_input->getMovementMode()) {
			g_input->inputScrollFOV(window, xoffset, yoffset, app->m_camera->getFOV());
		}
		else {
			g_input->inputScrollRadius(window, xoffset, yoffset, app->m_camera->getFOV());
		}
	}

	static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {	
		if (g_input->getMovementMode()) {
			g_input->inputMouse(window, xposIn, yposIn);
		}
		else {
			g_input->orbitCursorLeft(window, xposIn, yposIn);
			g_input->orbitCursorRight(window, xposIn, yposIn);
		}
	}

	static void mouse_button_right_callback(GLFWwindow* window, int button, int action, int mods) {
		if ((button == GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && (!g_input->getMovementMode())) {
			g_input->setMouseRightEnabled(true);
			if (action == GLFW_RELEASE) {
				g_input->setMouseRightEnabled(false);
			}
		}
	}

	static void mouse_button_left_callback(GLFWwindow* window, int button, int action, int mods) {
		if ((button == GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && (!g_input->getMovementMode())) {
			g_input->setMouseLeftEnabled(true);
			if (action == GLFW_RELEASE) {
				g_input->setMouseLeftEnabled(false);
			}
		}
	}

	void update(float deltaTime, GLFWwindow* window) {
		//Mesh rotation
		g_input->setDeltaTime(deltaTime);

		if (m_uiDraw->boolMeshRotation()) {
			if (!m_uiDraw->boolDoOnce()) {
				for (auto models : m_scene->getModels()) {
					for (auto meshes : models->getMeshes()) { // Set rotation to 0.0f once
						meshes->setRotationX(0.0f), meshes->setRotationY(0.0f), meshes->setRotationZ(0.0f);
					}			
				}	
				m_uiDraw->toggleDoOnce();
			}
			for (auto models : m_scene->getModels()) {
				for (auto meshes : models->getMeshes()) { // Rotation loop
					meshes->setRotationX(meshes->getRotationX() + deltaTime);
				}
			}
		}
		else if (!m_uiDraw->boolMeshRotation() && m_uiDraw->boolDoOnce())
		{
			for (auto models : m_scene->getModels()) {
				for (auto meshes : models->getMeshes()) { // Set rotation to 0.0f, when enabling rotation
					meshes->setRotationX(0.0f), meshes->setRotationY(0.0f), meshes->setRotationZ(0.0f);
				}
			}
			m_uiDraw->toggleDoOnce();
		}
		
		// Keeping the movement inside the update loop
		g_input->movementControls(window, deltaTime);
	}

private:
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	// Pointers to the Shader objects
	//Shader* m_shader;				

	Shader* m_cubemapShader;
	Shader* m_BackgroundShader;
	Shader* m_IrradianceShader;
	Shader* m_Prefilter;
	Shader* m_brdf;

	Shader* m_icon;
	Shader* m_backImage;

	// Class references
	Camera*         			m_camera;
	Mesh*						m_meshRender;
	UI*							m_uiDraw;
	HDRI*						m_HDRI;
	GBuffer*					m_GBuffer;
	Icon*						m_iconClass;
	ScreenSpace*				m_ssaoClass;
	SaveFile*					m_saveFile;
	Scene*						m_scene;
	ResourceManager*			m_resoManager;
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
	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL Craziness", NULL, NULL);
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

	if (!g_input->getMouseEnabled())
		g_input->minimizeWindow(window);

	// Disable cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the window's user pointer to our Application instance
	glfwSetWindowUserPointer(window, g_app);

	// Mouse callback
	glfwSetCursorPosCallback(window, Application::mouse_callback);

	// Mouse scroll callback
	glfwSetScrollCallback(window, Application::scroll_callback);

	// Mouse button callback
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			Application::mouse_button_left_callback(window, button, action, mods);
			break;

		case GLFW_MOUSE_BUTTON_RIGHT:
			Application::mouse_button_right_callback(window, button, action, mods);
			break;
		}
	});

	// Specify the key callback as c++-lambda to glfw
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			// Close window if escape is pressed by the user.
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_E:
			// ImGui focus toggle
			g_input->inputFocus(window);
			break;
		case GLFW_KEY_H:
			// Hide ImGui
			g_input->inputHide(window);
			break;
		case GLFW_KEY_V:	// Enable free mode
			g_input->toggleMovementMode(window);
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
		//g_app->deferredRendering(window);
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