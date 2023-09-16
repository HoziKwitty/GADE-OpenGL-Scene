#include "Shader.h"

Shader::Shader()
{
	shaderID = 0;
	uniformModel = 0;
	uniformProjection = 0;
	uniformView = 0;
}

void Shader::CreateFromString(const char* vertexCode, const char* fragmentCode)
{
	compileShader(vertexCode, fragmentCode);
}

void Shader::CreateFromFiles(const char* vertexLocation, const char* fragmentLocation)
{
	std::string vertexString = ReadFile(vertexLocation);
	std::string fragmentString = ReadFile(fragmentLocation);
	const char* vertexCode = vertexString.c_str();
	const char* fragmentCode = fragmentString.c_str();

	compileShader(vertexCode, fragmentCode);
}

std::string Shader::ReadFile(const char* fileLocation)
{
	std::string content;
	std::ifstream fileStream(fileLocation, std::ios::in);

	if (!fileStream.is_open()) {
		printf("Failed to read %s! File doesn't exist.", fileLocation);
		return "";
	}

	std::string line = "";
	while (!fileStream.eof())
	{
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

void Shader::compileShader(const char* vertexCode, const char* fragmentCode)
{
	// Creating the prog
	shaderID = glCreateProgram();

	// Make sure the shader is created correctly.
	if (!shaderID)
	{
		printf("Failed to create shader\n");
		return;
	}

	// Adding shader to the program
	// Pass in the prog, string vShader
	// Indicate the type of shader
	addShader(shaderID, vertexCode, GL_VERTEX_SHADER);

	// Adding shader to the program
	// Pass in the prog, string fShader
	// Indicate the type of shader
	addShader(shaderID, fragmentCode, GL_FRAGMENT_SHADER);

	#pragma region ErrorChecking
	// Getting error codes from the creation of the shaders
	GLint result = 0; // result of the two functions
	GLchar eLog[1024] = { 0 }; // logging the error

	// Checking if the program is linked correctly
	glLinkProgram(shaderID);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // get the info. Check if the prog is linked

	// Check if the result is false.
	if (!result)
	{
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error linking program: '%s'\n", eLog);
		return;
	}

	// Validate the program/
	// Checking if openGL was setup correctly for the shader
	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);

	if (!result)
	{
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error validating program: '%s'\n", eLog);
		return;
	}
	#pragma endregion

	//uniformModel = glGetUniformLocation(shader, "xMove");
	uniformModel = glGetUniformLocation(shaderID, "model");
	uniformProjection = glGetUniformLocation(shaderID, "projection");
	uniformView = glGetUniformLocation(shaderID, "view");
	uniformAmbientColour = glGetUniformLocation(shaderID, "directionalLight.colour");
	uniformAmbientIntensity = glGetUniformLocation(shaderID, "directionalLight.ambientIntensity");
	uniformDirection = glGetUniformLocation(shaderID, "directionalLight.direction");
	uniformDiffuseIntensity = glGetUniformLocation(shaderID, "directionalLight.diffuseIntensity");
	uniformSpecularIntensity = glGetUniformLocation(shaderID, "material.specularIntensity");
	uniformShininess = glGetUniformLocation(shaderID, "material.shininess");
	uniformEyePosition = glGetUniformLocation(shaderID, "eyePosition");
}

GLuint Shader::getProjectionLocation()
{
	return uniformProjection;
}

GLuint Shader::getModelLocation()
{
	return uniformModel;
}

GLuint Shader::getViewLocation()
{
	return uniformView;
}

GLuint Shader::getAmbientIntensityLocation()
{
	return uniformAmbientIntensity;
}

GLuint Shader::getAmbientColourLocation()
{
	return uniformAmbientColour;
}

GLuint Shader::getDiffuseIntensityLocation()
{
	return uniformDiffuseIntensity;
}

GLuint Shader::getDirectionLocation()
{
	return uniformDirection;
}

GLuint Shader::getSpecularIntensityLocation()
{
	return uniformSpecularIntensity;
}

GLuint Shader::getShininessLocation()
{
	return uniformShininess;
}

GLuint Shader::getEyePositionLocation()
{
	return uniformEyePosition;
}

void Shader::useShader()
{
	if (!shaderID)
	{
		printf("Failed to create shader\n");
		return;
	}
	glUseProgram(shaderID);
}

void Shader::clearShader()
{
	if (shaderID != 0)
	{
		glDeleteProgram(shaderID);
		shaderID = 0;
	}

	uniformModel = 0;
	uniformProjection = 0;

}

void Shader::addShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	GLuint theShader = glCreateShader(shaderType);

	const GLchar* theCode[1];
	theCode[0] = shaderCode;

	GLint codeLength[1];
	codeLength[0] = (GLint)strlen(shaderCode);

	// Passing the created shader code into the shader source
	glShaderSource(theShader, 1, theCode, codeLength);
	glCompileShader(theShader);

	#pragma region ErrorCheck
	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		glGetShaderInfoLog(theShader, 1024, NULL, eLog);
		fprintf(stderr, "Error compiling the %d shader: '%s'\n", shaderType, eLog);
		return;
	}
	#pragma endregion

	// Attaching the shader to the program
	glAttachShader(theProgram, theShader);
}

Shader::~Shader()
{
	clearShader();
}