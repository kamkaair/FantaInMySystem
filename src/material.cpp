#include "material.h"

//Material::Material(const std::vector<GLuint>& textures) : m_texturesC(textures) {}

Material::Material(const std::vector<GLuint>& textures, const std::string& name, const int materialIndex) 
    : m_materialTextures(textures), m_displayName(name), m_materialIndex(materialIndex) {}

Material::~Material() {
    for (GLuint textureId : m_materialTextures) {
        glDeleteTextures(1, &textureId);
    }
    m_materialTextures.clear();
}

std::vector<GLuint>& Material::getTextures() { return m_materialTextures; }
const std::string& Material::getName() const { return m_displayName; }
int& Material::getMaterialIndex() { return m_materialIndex; }