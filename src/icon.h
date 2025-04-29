#pragma	once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glm/glm.hpp>      // Include glm

#include "UI.h"
#include "inputs.h"
#include "shader.h"
#include "textureLoading.h"
#include "camera.h"

class Icon : public kgfw::Object {
public:
	Icon(Mesh* mesh, TextureLoading* texLoading, UI* uiDraw, Inputs* inputs, Camera* camera);
	~Icon();

	void renderIcons(Shader* m_icon, float iconSize, std::vector<glm::vec3> targetPos, int amount, int texIndex);
	void renderIcons(Shader* m_icon, float iconSize, glm::vec3 targetPos, int texIndex);
	void visualizeFocus(Shader* m_icon, float iconSize, UI* m_uiDraw, Inputs* input, Camera* m_camera);
	void loadIconTexture(const char* path);

	glm::mat4 processIconMatrix(glm::vec3 targetPos, float iconSize);


private:
	std::vector<Texture*> m_iconTexture;
	Mesh* m_meshRender;
	TextureLoading* m_texLoading;
	UI* m_uiDraw;
	Inputs* m_input;
	Camera* m_camera;
};