#include "material.h"

//Material::Material(const std::vector<GLuint>& textures) : m_texturesC(textures) {}

Material::Material(const std::vector<GLuint>& textures, const std::string& name) : m_texturesC(textures), m_name(name) {}


Material::~Material() {
    for (GLuint textureId : m_texturesC) {
        glDeleteTextures(1, &textureId);
    }
    m_texturesC.clear();
}

const std::vector<GLuint>& Material::getTextures() const {
    return m_texturesC;
}

const std::string& Material::getName() const {
    return m_name;
}