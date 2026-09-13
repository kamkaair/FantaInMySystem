#include "shader.h"			// Include class header
#include <stdio.h>			// Include stdio.h, which contains printf-function
#include <kgfw/GLUtils.h>	// Include GLUtils for checkGLError
#include <iostream>

Shader::Shader(const std::string& vertexShaderString, const std::string& fragmentShaderString, const std::string& geometryShaderString)
	: Object(__FUNCTION__)
	, m_shaderProgram(0) {

	// Create and compile vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	checkGLError();

	// Vertex shader creation
	// Convert std::string to const GLchar* 
	const char* vertexShaderSource = vertexShaderString.c_str();
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		// If failed, get error string using glGetShaderInfoLog-function.
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		checkGLError();
		printf("ERROR: Shader compilation failed: \"%s\"\n", infoLog);
	}

	// Create and compile fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	checkGLError();

	// Fragment shader creation
	const char* fragmentShaderSource = fragmentShaderString.c_str();
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("ERROR: Shader compilation failed: \"%s\"\n", infoLog);
	}

	// Geometry shader creation
	int geometryShader = 0;
	if (geometryShaderString != "") {
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		const char* geometryShaderSource = geometryShaderString.c_str();
		glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
		glCompileShader(geometryShader);

		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
			printf("ERROR: Shader compilation failed: \"%s\"\n", infoLog);
		}
	}	

	// link shaders
	m_shaderProgram = glCreateProgram();
	glAttachShader(m_shaderProgram, vertexShader);
	glAttachShader(m_shaderProgram, fragmentShader);
	if(geometryShader != 0) { 
		glAttachShader(m_shaderProgram, geometryShader); }
	glLinkProgram(m_shaderProgram);

	// check for linking errors
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		// If failed, get error string using glGetProgramInfoLog-function.
		glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
		printf("ERROR: Shader link failed: \"%s\"\n", infoLog);
	}

	// After linking, the shaders can be deleted.
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	if(geometryShader != 0) { 
		glDeleteShader(geometryShader); }
}

Shader::~Shader() {
	// Delete shader program
	if (m_shaderProgram) {
		glDeleteProgram(m_shaderProgram);
		checkGLError();
	}
}

void Shader::bind() {
	glUseProgram(m_shaderProgram);
	checkGLError();
}

void Shader::deleteShader() {
	glDeleteProgram(m_shaderProgram);
	checkGLError();
}

void Shader::setUniform(const std::string& name, float x, float y, float z) {
	GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
	if (loc < 0) {
		std::cout << "Uniform " << name << " not found in program " << m_shaderProgram << std::endl;
		return; // Don't set the uniform value, if it not found
	}
	glUniform3f(loc, x, y, z);
	checkGLError();
}

void Shader::setUniform(const std::string& name, float x, float y, float z, float w) {
	GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
	if (loc < 0) {
		std::cout << "Uniform " << name << " not found in program " << m_shaderProgram << std::endl;
		return; // Don't set the uniform value, if it not found
	}
	glUniform4f(loc, x, y, z, w);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		glUniform3f(loc, x, y, z); // Set as 3 component value in case of 4 component set failed.
	}

	checkGLError();
}

//void Shader::setUniform(const std::string& name, const glm::mat4& m) {
//    GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
//    if (loc < 0) {
//        return; // Don't set the uniform value, if it not found
//    }
//    glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
//    checkGLError();
//}

void Shader::setUniform(const std::string& name, const glm::mat4& m) {
	GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
	if (loc < 0) {
		std::cout << "Uniform " << name << " not found in program " << m_shaderProgram << std::endl;
		return; // Don't set the uniform value if not found
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
	checkGLError();
}

void Shader::setUniform(const std::string& name, const glm::mat3& m) {
	GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
	if (loc < 0) {
		std::cout << "Uniform " << name << " not found in program " << m_shaderProgram << std::endl;
		return; // Don't set the uniform value if not found
	}
	glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
	checkGLError();
}


void Shader::setUniform(const std::string& name, int value) {
	GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
	if (loc < 0) {
		std::cout << "Uniform " << name << " not found in program " << m_shaderProgram << std::endl;
		return; // Don't set the uniform value, if it not found
	}
	glUniform1i(loc, value);
	checkGLError();
}

void Shader::setUniform(const std::string& name, float value) {
	GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
	if (loc < 0) {
		printf("Uniform not found: %s\n", name.c_str());
		return; // Don't set the uniform value if it's not found
	}
	glUniform1f(loc, value);
	checkGLError();
}

// Removing the `index` parameter from setUniform since we're handling array uniforms differently.
void Shader::setUniform(const std::string& name, const glm::vec3& value, int index) {
	std::string uniformName = name + "[" + std::to_string(index) + "]"; // Building array name
	GLint loc = glGetUniformLocation(m_shaderProgram, uniformName.c_str());
	if (loc >= 0) {
		glUniform3fv(loc, 1, &value[0]);
		checkGLError();
	}
	else {
		printf("Uniform not found: %s\n", uniformName.c_str());
	}

	if (loc == -1) {
		std::cout << "Uniform " << name << " not found in the shader!" << std::endl;
	}
}

void Shader::setUniform(const std::string& name, float value, int index) {
	std::string uniformName = name + "[" + std::to_string(index) + "]"; // Building array name
	GLint loc = glGetUniformLocation(m_shaderProgram, uniformName.c_str());
	if (loc >= 0) {
		glUniform1f(loc, value);
		checkGLError();
	}
	else {
		printf("Uniform not found: %s\n", uniformName.c_str());
	}
}

//void Shader::setUniform(const std::string& name, const glm::vec3& value) {
//	GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
//	if (loc < 0) {
//		printf("Uniform not found: %s\n", name.c_str());
//		return; // Don't set the uniform value if it's not found
//	}
//	glUniform3fv(loc, 1, &value[0]);
//	checkGLError();
//}

void Shader::setUniform(const std::string& name, const glm::vec3& value) {
	bind();  // Ensure shader is active
	GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
	if (loc < 0) {
		printf("Uniform not found: %s\n", name.c_str());
		return;
	}
	glUniform3fv(loc, 1, &value[0]);
	checkGLError();
}

void Shader::setUniform(const std::string& name, const glm::vec2& value) {
	bind();  // Ensure shader is active
	GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
	if (loc < 0) {
		printf("Uniform not found: %s\n", name.c_str());
		return;
	}
	glUniform2fv(loc, 1, &value[0]);
	checkGLError();
}

bool Shader::IsValid() const {
	return glIsProgram(m_shaderProgram);
}