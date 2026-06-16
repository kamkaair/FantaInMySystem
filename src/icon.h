#pragma	once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glm/glm.hpp>      // Include glm

#include "inputs.h"
#include "camera.h"
#include "savefileStructs.h"

class Mesh;
class Shader;
class ResourceManager;
class UI;

class Icon : public kgfw::Object {
public:
	Icon(Mesh* mesh, ResourceManager* reso, Inputs* inputs, Camera* camera);
	~Icon();

	void renderIcons(Shader* m_icon, float iconSize, std::vector<FileLights> targetPos, int texIndex);
	void renderIcons(Shader* m_icon, float iconSize, glm::vec3 targetPos, int texIndex);
	void visualizeFocus(Shader* m_icon, float iconSize, Inputs* input, Camera* m_camera);
	void loadIconTexture(const char* path);

	glm::mat4 processIconMatrix(glm::vec3 targetPos, float iconSize);

private:
	std::vector<Texture*> m_iconTexture;
	Mesh* m_meshRender;
	ResourceManager* m_resoManager;
	UI* m_uiDraw;
	Inputs* m_input;
	Camera* m_camera;
};