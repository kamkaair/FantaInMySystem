#pragma once
#include <vector>
#include <glad/gl.h>
#include <string>

#include "glm/gtx/string_cast.hpp" // Include for printing mats and vecs

class Material {
public:
    //Material(const std::vector<GLuint>& textures);
    Material(const std::vector<GLuint>& textures, const std::string& name, const int materialIndex);  // Updated constructor

    ~Material();

    glm::vec3 diffuseColor = glm::vec3(1.0f);
    float roughness = 0.5f;
    float metallic = 0.0f;

    bool useDiffuseTexture = true;
    bool useMetallicTexture = true;
    bool useRoughnessTexture = true;

    const std::vector<GLuint>& getTextures() const;
    const std::string& getName() const;
    int getMaterialIndex() const;

private:
    std::vector<GLuint> m_texturesC;
    std::string m_name;
    int m_materialIndex;
};