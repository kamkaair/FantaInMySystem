#pragma once
#include <vector>
#include <glad/gl.h>
#include <string>

#include "glm/gtx/string_cast.hpp" // Include for printing mats and vecs

class Material {
public:
    enum class alphaModes {
        opaque, mask, blend
    };

    Material(const std::vector<GLuint>& textures, const std::string& name, const int materialIndex, alphaModes inAlpha = alphaModes::opaque);

    ~Material();

    // Material properties
    glm::vec3 diffuseColor = glm::vec3(1.0f);
    float roughness = 0.5f;
    float metallic = 0.0f;
    float emission = 0.0f;
    float opacity = 1.0f;

    bool useDiffuseTexture = true;
    bool useMetallicTexture = true;
    bool useRoughnessTexture = true;
    bool useEmissionTexture = false;
    bool useOpacityTexture = false;

    alphaModes currentAlphaMode;

    std::vector<GLuint>& getTextures();
    const std::string& getName() const;
    int& getMaterialIndex();

private:
    std::vector<GLuint> m_materialTextures;
    std::string m_displayName;
    int m_materialIndex;
};