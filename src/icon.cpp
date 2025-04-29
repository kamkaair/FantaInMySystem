#include "icon.h"

Icon::Icon(Mesh* mesh, TextureLoading* texLoading, UI* uiDraw, Inputs* inputs, Camera* camera) : m_meshRender(mesh), m_texLoading(texLoading), m_uiDraw(uiDraw), m_input(inputs), m_camera(camera), Object(__FUNCTION__) {}

Icon::~Icon() {
	for (auto meshes : m_iconTexture) {
		delete meshes;
		meshes = 0;
	}
}

glm::mat4 Icon::processIconMatrix(glm::vec3 targetPos, float iconSize) {
	//glm::vec3 directionToCamera = glm::normalize(cameraPos - m_uiDraw->getPointLightPos()[i]);
	//directionToCamera.y = 0.0f; // Only if you want the plane to stay vertical

	// Scaling depending on the distance
	float iconDistance = glm::length(m_input->getCameraPos() - targetPos);

	// Divide the scale by the icon's distance by the iconSize. Icon's scale will stay the same regardless of the position of the camera.
	//float iconSize = 25.0f;
	float scale = iconDistance / iconSize;
	//float iconSize = 2.0f;
	//float scale = iconSize / iconDistance;

	// Model matrix for the lamp quad
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), targetPos) * glm::scale(glm::mat4(1.0f), glm::vec3(scale));

	// Match the camera rotation to the icon's rotation
	glm::mat4 cameraRotation = glm::mat4(glm::mat3(m_camera->getViewMatrix()));
	modelMatrix *= glm::inverse(cameraRotation);
	// Flip the icon, since it's upside down
	modelMatrix *= glm::mat4(glm::mat3(-1.0f));

	return modelMatrix;
}

void Icon::renderIcons(Shader* m_icon, float iconSize, std::vector<glm::vec3> targetPos, int amount, int texIndex)
{
	for (size_t i = 0; i < m_uiDraw->getPointLightPos().size(); i++) {

		glm::mat4 modelMatrix = processIconMatrix(targetPos[i], iconSize);

		m_icon->bind();
		m_icon->setUniform("projection", m_camera->getProjectionMatrix());
		m_icon->setUniform("view", m_camera->getViewMatrix());
		m_icon->setUniform("model", modelMatrix);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_iconTexture[texIndex]->getTextureId());

		m_meshRender->renderQuad();
	}
}

void Icon::renderIcons(Shader* m_icon, float iconSize, glm::vec3 targetPos, int texIndex)
{
	glm::mat4 modelMatrix = processIconMatrix(targetPos, iconSize);

	m_icon->bind();
	m_icon->setUniform("projection", m_camera->getProjectionMatrix());
	m_icon->setUniform("view", m_camera->getViewMatrix());
	m_icon->setUniform("model", modelMatrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_iconTexture[texIndex]->getTextureId());

	m_meshRender->renderQuad();
}

void Icon::visualizeFocus(Shader* m_icon, float iconSize, UI* m_uiDraw, Inputs* input, Camera* m_camera)
{
	// Scaling depending on the distance
	float iconDistance = glm::length(input->getCameraPos() - input->getCameraFocus());

	// Divide the scale by the icon's distance by the iconSize. Icon's scale will stay the same regardless of the position of the camera.
	//float iconSize = 25.0f;
	float scale = iconDistance / iconSize;

	// Model matrix for the lamp quad
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), input->getCameraFocus()) * glm::scale(glm::mat4(1.0f), glm::vec3(scale));

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
	glBindTexture(GL_TEXTURE_2D, m_iconTexture[1]->getTextureId());

	m_meshRender->renderQuad();
}

void Icon::loadIconTexture(const char* path) {
	// Load the texture for an icon
	Texture* iconTexture = m_texLoading->loadTexture((std::string(ASSET_DIR) + path).c_str());
	m_iconTexture.push_back(iconTexture);
}