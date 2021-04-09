#include <fstream>
#include <iostream>
#include "Shader.hpp"

std::string readFile(std::string filename){
	std::ifstream infile(filename);
	std::string line;
	std::string result;
	while(std::getline(infile, line)) result += (line+"\n");
	return result;
}

Shader::Shader(std::string vertexShader, std::string fragmentShader){
	std::string vtString = readFile(vertexShader);
	std::string fragString = readFile(fragmentShader);

	const char* vtSource = vtString.c_str();
	const char* fragSource = fragString.c_str();
	
	int success;
	char infoLog[512];
	
	vertexID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexID, 1, &vtSource, nullptr);
	glCompileShader(vertexID);
	glGetShaderiv(vertexID, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertexID, 512, nullptr, infoLog);
		std::cout << "Vertex Shader Compile failed:\n" << infoLog << "\n";
	}
	
	fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentID, 1, &fragSource, nullptr);
	glCompileShader(fragmentID);
	glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragmentID, 512, nullptr, infoLog);
		std::cout << "fragment Shader Compile failed:\n" << infoLog << "\n";
	}
	
	programID = glCreateProgram();
	glAttachShader(programID, vertexID);
	glAttachShader(programID, fragmentID);
	glLinkProgram(programID);
}

void Shader::use(){
	glUseProgram(programID);
}

void Shader::setInt(const char* name, int value){
	glUniform1i(glGetUniformLocation(programID, name), value);
}

void Shader::setVec3(const char* name, glm::vec3 value){
	glUniform3fv(glGetUniformLocation(programID, name), 1, glm::value_ptr(value));
}

void Shader::setMat4(const char* name, glm::mat4 value){
	glUniformMatrix4fv(glGetUniformLocation(programID, name), 1, GL_FALSE, glm::value_ptr(value));
}

int Shader::getUniformID(const char* name){
	return glGetUniformLocation(programID, name);
}

void Shader::setFloat(const char* name, float value){
	glUniform1f(glGetUniformLocation(programID, name), value);
}

void Shader::setVec2(const char* name, glm::vec2 value)
{
	glUniform2fv(glGetUniformLocation(programID, name), 1, glm::value_ptr(value));
}
