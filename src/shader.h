#pragma once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include <string>           // Inlude std::string
#include <glm/glm.hpp>      // Include glm

class Shader : public kgfw::Object {
public:
	Shader(const std::string& vertexShaderString, const std::string& fragmentShaderString);
	~Shader();

	void bind();
	void deleteShader();

	void setUniform(const std::string& name, float x, float y, float z);
	void setUniform(const std::string& name, float x, float y, float z, float w);
    void setUniform(const std::string& name, const glm::mat4& m);
	void setUniform(const std::string& name, const glm::mat3& m);
	void setUniform(const std::string& name, int value);

	void setUniform(const std::string& name, float value);
	void setUniform(const std::string& name, const glm::vec3& value, int index);
	void setUniform(const std::string& name, float value, int index);
	void setUniform(const std::string& name, const glm::vec3& value);
	void setUniform(const std::string& name, const glm::vec2& value);

	bool IsValid() const;

private:
	GLint m_shaderProgram;	// Handle to the shader program
};
