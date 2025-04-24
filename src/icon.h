#pragma	once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glm/glm.hpp>      // Include glm

#include "UI.h"
#include "inputs.h"
#include "shader.h"
#include "textureLoading.h"

class Icon : public kgfw::Object {
public:
	Icon(Mesh* mesh, TextureLoading* texLoading);
	~Icon();

	void renderIcons(Shader* m_icon, float iconSize, UI* m_uiDraw, Inputs* input, Camera* m_camera);
	void visualizeFocus(Shader* m_icon, float iconSize, UI* m_uiDraw, Inputs* input, Camera* m_camera);
	void loadIconTexture(const char* path);

private:
	std::vector<Texture*> m_iconTexture;
	Mesh* m_meshRender;
	TextureLoading* m_texLoading;
};