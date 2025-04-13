#include "camera.h"			// Include Camera-class.
#include "mesh.h"
#include "material.h"
#include "utils.h"			// Utility functions
#include "UI.h"
#include "HDRI.h"
#include "textureLoading.h"
#include "inputs.h"
#include "GBuffer.h"

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

// Added for SSAO
#include <random>

// Include STB-image library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Inputs* g_input = 0;

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

		m_GBuffer = new GBuffer(width, height);

		// texloading function class
		m_texLoading = new TextureLoading();

		// Loads and computes all the HDRI maps
		m_HDRI = new HDRI(m_cubemapShader, m_BackgroundShader, m_IrradianceShader, m_Prefilter, m_brdf);

		// the UI class, contains ImGui and such
		m_uiDraw = new UI(m_shader, m_backImage, m_texLoading, m_HDRI, m_GBuffer);

		m_BackgroundShader->bind();
		m_BackgroundShader->setUniform("environmentMap", 0);

		// Create perspective-projection camera
		m_camera = new Camera(fov, 640/480, 0.1f, 100.0f);

		// Input class
		g_input = new Inputs(m_uiDraw, m_camera);

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

		//bunchaHOOPLAA();

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

	~Application() {
		// Delete shaders
		delete m_shader;
		m_shader = 0;

		delete m_icon;
		m_icon = 0;

		delete m_backImage;
		m_backImage = 0;

		delete m_SSAO;
		m_SSAO = 0;

		delete m_geometryPass;
		m_geometryPass = 0;

		delete m_lightPass;
		m_lightPass = 0;

		// Delete references
		delete m_HDRI;
		m_HDRI = 0;

		delete m_uiDraw;
		m_uiDraw = 0;

		delete m_texLoading;
		m_texLoading = 0;

		delete g_input;
		g_input = 0;

		delete m_GBuffer;
		m_GBuffer = 0;

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

		// Load the billboard icon shaders
		std::string iconVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/iconVert.glsl");
		std::string iconFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/iconFrag.glsl");
		m_icon = new Shader(iconVertSource, iconFragSource);

		// Load the 2d-texture background shaders
		std::string BackImageVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/backgroundImageVert.glsl");
		std::string BackImageFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/backgroundImageFrag.glsl");
		m_backImage = new Shader(BackImageVertSource, BackImageFragSource);

		// Load the geometry passes (for deferred rendering)
		std::string GeometryVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/GeometryPassVert.glsl");
		std::string GeometryFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/GeometryPassFrag.glsl");
		m_geometryPass = new Shader(GeometryVertSource, GeometryFragSource);

		// Load the geometry passes (for deferred rendering)
		std::string LightingVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/DeferredLightVert.glsl");
		std::string LightingFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/DeferredLightFrag.glsl");
		m_lightPass = new Shader(LightingVertSource, LightingFragSource);

		// Load SSAO shaders
		std::string SSAOVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/SSAO-Vert.glsl");
		std::string SSAOFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/SSAO-Frag.glsl");
		m_SSAO = new Shader(SSAOVertSource, SSAOFragSource);
	}

	void render(GLFWwindow* window) {
		!m_uiDraw->getRenderMode() ? forwardRendering(window) : deferredRendering(window);
	}

	void forwardRendering(GLFWwindow* window) {
		// Query the size of the framebuffer (window content) from glfw.
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
		case 0: m_HDRI->renderSkybox(m_camera); break;
		case 1: m_HDRI->renderBackgroundImage(m_camera, m_HDRI->getBackgroundTexture(), m_backImage); break;
		}

		if (!m_uiDraw->getMeshes().empty()) {
			std::vector<GLuint> textureIds;
			for (Texture* texture : m_textures) {
				textureIds.push_back(texture->getTextureId());
			}

			// Forward rendering
			for (Mesh* mesh : m_uiDraw->getMeshes()) {
				m_HDRI->setHDRITextures(m_shader);

				//m_shader->bind();
				//glActiveTexture(GL_TEXTURE7);
				//glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
				//m_shader->setUniform("ssao", 7);

				mesh->Render(m_shader, m_camera->getPosition(), m_uiDraw->getPointLightPos(), m_uiDraw->getPointLightColor(), m_camera->getViewMatrix(), m_camera->getModelMatrix(), m_camera->getProjectionMatrix());
			}
		}

		//renderSSAO();

		if (!g_input->getImGuiVisibility()) {
			renderIcons(); // Render all the point lamp icons
			m_uiDraw->ImGuiDraw(); // Render the ImGui window
		}
	}

	void deferredRendering (GLFWwindow* window) {
		glfwGetFramebufferSize(window, &width, &height);
		m_camera->setAspectRatio(width, height);
		framebuffer_size_callback(window, width, height);
		glViewport(0, 0, width, height);
		checkGLError();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. Geometry pass: render scene's geometry/color data into gbuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer->getGBuffer());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// HDRI
		//switch (m_uiDraw->getBackgroundMode()) {
		//case 0: m_HDRI->renderSkybox(m_camera); break;
		//case 1: m_HDRI->renderBackgroundImage(m_camera, m_HDRI->getBackgroundTexture(), m_backImage); break;
		//}

		m_geometryPass->bind();
		for (Mesh* mesh : m_uiDraw->getMeshes()) {
			// HDRI textures to the lighting shader
			//m_lightPass->bind();
			//m_HDRI->setHDRITextures(m_lightPass);
			mesh->RenderGBuffer(m_geometryPass, m_camera->getViewMatrix(),
				m_camera->getModelMatrix(), m_camera->getProjectionMatrix());
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->getGBuffer());
		glReadBuffer(GL_COLOR_ATTACHMENT0);  // Check positions

		// 2. SSAO pass
		//renderSSAO();

		// HDRI
		switch (m_uiDraw->getBackgroundMode()) {
		case 0: m_HDRI->renderSkybox(m_camera); break;
		case 1: m_HDRI->renderBackgroundImage(m_camera, m_HDRI->getBackgroundTexture(), m_backImage); break;
		}

		for (Mesh* mesh : m_uiDraw->getMeshes()) {
			// HDRI textures to the lighting shader
			m_lightPass->bind();
			m_HDRI->setHDRITextures(m_lightPass);
		}

		// 3. Lighting pass: calculate lighting using gbuffer and SSAO
		glClear(GL_COLOR_BUFFER_BIT);
		m_lightPass->bind();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGPosition());
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGNormal());
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGAlbedo());
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGMetallicRoughness());
		//glActiveTexture(GL_TEXTURE3);
		//glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

		m_lightPass->setUniform("gPosition", 3);
		m_lightPass->setUniform("gNormal", 4);
		m_lightPass->setUniform("gAlbedoSpec", 5);
		m_lightPass->setUniform("gMetallicRoughness", 6);
		//m_lightPass->setUniform("ssao", 4);
		
		// Set light uniforms
		for (int i = 0; i < m_uiDraw->getPointLightPos().size(); i++) {
			m_lightPass->setUniform("pointLights[" + std::to_string(i) + "].position",
				m_uiDraw->getPointLightPos()[i]);

			m_lightPass->setUniform("pointLights[" + std::to_string(i) + "].color",
				m_uiDraw->getPointLightColor()[i]);
			
			// Set attenuation factors for the point light
			m_lightPass->setUniform("pointLights[" + std::to_string(i) + "].constant", 1.0f);   // Constant attenuation
			m_lightPass->setUniform("pointLights[" + std::to_string(i) + "].linear", 0.09f);    // Linear attenuation
			m_lightPass->setUniform("pointLights[" + std::to_string(i) + "].quadratic", 0.032f); // Quadratic attenuation
		}
		m_lightPass->setUniform("NUM_POINT_LIGHTS", (int)m_uiDraw->getPointLightPos().size());
		//m_lightPass->setUniform("viewPos", m_camera->getPosition());
		m_lightPass->setUniform("view", m_camera->getPosition());

		// Render quad
		m_meshRender->renderQuad();

		// Render UI
		if (!g_input->getImGuiVisibility()) {
			renderIcons();
			m_uiDraw->ImGuiDraw();
		}
	}

	float SSAOLerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	std::vector<glm::vec3> createSampleKernel() {
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;

		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / 64.0f;

			// scale samples s.t. they're more aligned to center of kernel
			scale = SSAOLerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
		return ssaoKernel;
	}

	GLuint createNoiseTexture() {
		std::vector<glm::vec3> ssaoNoise;
		// Added these scrunklers down here again, might have to make them use the same randomFloats as the createSampleKernel()
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;

		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
			ssaoNoise.push_back(noise);
		}
		GLuint noiseTexture; glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		return noiseTexture;
	}

	GLuint createSsaoFBO() {
		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		return ssaoFBO;
	}

	GLuint createSsaoColorBuffer() {
		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Framebuffer not complete!" << std::endl;

		return ssaoColorBuffer;
	}

	void bunchaHOOPLAA() {
		ssaoFBO = createSsaoFBO();
		ssaoKernel = createSampleKernel();

		noiseTexture = createNoiseTexture();
		ssaoColorBuffer = createSsaoColorBuffer();
	}

	void renderSSAO() {
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		m_SSAO->bind();

		// Send kernel + rotation 
		for (unsigned int i = 0; i < 64; ++i)
			m_SSAO->setUniform("samples[" + std::to_string(i) + "]", ssaoKernel[i]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		m_SSAO->setUniform("texNoise", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGPosition());
		m_SSAO->setUniform("gPosition", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGNormal());
		m_SSAO->setUniform("gNormal", 2);

		m_SSAO->setUniform("projection", m_camera->getProjectionMatrix());

		m_meshRender->renderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
			float iconDistance = glm::length(g_input->getCameraPos() - m_uiDraw->getPointLightPos()[i]);

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

			m_meshRender->renderQuad();
		}
	}

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		g_input->inputScroll(window, xoffset, yoffset, app->m_camera->getFOV());
	}

	static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
	{
		g_input->inputMouse(window, xposIn, yposIn);
	}

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

		// Keeping the movement inside the update loop
		g_input->inputMovement(window, deltaTime);
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

	Shader* m_SSAO;
	Shader* m_lightPass;
	Shader* m_geometryPass;


	// Class references
	Camera*         			m_camera;
	//Texture*					m_textureBack;			// Texture pointer
	Texture*					m_iconTexture = 0;
	Mesh*						m_meshRender;
	UI*							m_uiDraw;
	HDRI*						m_HDRI;
	TextureLoading*				m_texLoading;
	GBuffer*					m_GBuffer;

	GLuint						skyboxVAO = 0, skyboxVBO = 0, skyboxEBO = 0;
	GLuint						m_texture;

	GLuint						ssaoFBO = 0, ssaoColorBuffer = 0, noiseTexture = 0;
	std::vector<glm::vec3>		ssaoKernel;

	float fov = 40.0f;
	float randomFloat = 0.0f;
	int width = 640, height = 480;

	std::vector<Texture*>		m_textures;		// Vector of texture pointers
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

	//glClearColor(0.2, 0.2, 0.2, 1.0);

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

		//g_app->render(window);
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