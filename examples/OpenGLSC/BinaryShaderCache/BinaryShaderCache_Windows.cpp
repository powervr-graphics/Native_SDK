#include <fstream>
#include <string>
#include <regex>
#include <iostream>
#include <vector>

#include <DynamicGles.h>
#include <DynamicEgl.h>
#include <PVRUtils/PVRUtilsGles.h>

void initializeContext()
{
	EGLint configAttribs[] =
    {        
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE    
	};

	EGLint contextAttribs[] =    
	{
		EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE    
	};

	EGLDisplay display;    
	EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    display = egl::GetDisplay(EGL_DEFAULT_DISPLAY);
    egl::Initialize(display, NULL, NULL);

    EGLint numConfigs;
    egl::ChooseConfig(display, configAttribs, &config, 1, &numConfigs);

    context = egl::CreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

    EGLint pbufferAttribs[] =
    {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };
    surface = egl::CreatePbufferSurface(display, config, pbufferAttribs);

    egl::MakeCurrent(display, surface, surface, context);
}

std::string readFile(std::ifstream& fileStream) 
{
    std::string content;

    if(!fileStream.is_open()) 
	{
        std::cerr << "Could not read file. File does not exist." << std::endl;
        return "";
    }

    std::string line = "";
    while(!fileStream.eof()) 
	{
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

GLuint compileShader(GLuint type, std::ifstream& shaderFile)
{
	std::string shader = readFile(shaderFile);
	std::cout << shader.size() << "\n";
	const char* buffer = shader.c_str();

	// Create the shader and pass it the source.
	GLuint id = gl::CreateShader(static_cast<GLenum>(type));
	gl::ShaderSource(id, 1, &buffer, NULL);

	// Compile and check for errors.
	gl::CompileShader(id);
	GLint success;
	gl::GetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[1024];
		gl::GetShaderInfoLog(id, 1024, nullptr, infoLog);

		std::cerr << "Fail to compile shader\n";
	}

	return id;
}

void saveBinary(GLuint programID, std::ofstream& outputStream)
{
	// Get the size of the binary.
	GLint binarySize = 0;
	gl::GetProgramiv(programID, GL_PROGRAM_BINARY_LENGTH, &binarySize);

	// Allocate space for the binary, as well as the binary format.
	size_t bufferSize = sizeof(GLenum) + binarySize;
	char* binary = new char[sizeof(GLenum) + binarySize];

	// Get the binary from the driver, saving the format.
	GLenum binaryFormat;
	gl::GetProgramBinary(programID, binarySize, nullptr, &binaryFormat, binary + sizeof(GLenum));
	// Prefix the binary with the format.
	*((GLenum*)binary) = binaryFormat;

	outputStream.write(binary, binarySize);
}

void linkProgram(GLuint programID, std::vector<GLuint>& shaderID)
{
	for (auto id: shaderID) { gl::AttachShader(programID, id); }
	gl::LinkProgram(programID);
	GLint success;
	gl::GetProgramiv(programID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[1024];
		gl::GetProgramInfoLog(programID, 1024, nullptr, infoLog);

		std::cerr << "Fail to link shader\n";
	}
}

int main(int argc, char const* argv[])
{
	std::ofstream os ("BinaryShaderCache.bin", std::ios::binary | std::ios::out); 

	std::regex extReg("\\.[^.]*$");
	std::string fileName;

	std::vector<GLuint> shaderID{};

	initializeContext();

	for (size_t i = 1; i < argc; i++)
	{
		fileName = argv[i];
		std::ifstream inputShader (fileName, std::ios::in);
		auto extbBegin = std::sregex_iterator(fileName.begin(), fileName.end(), extReg);

		std::string fileExt = (*extbBegin).str();

		GLuint shaderType = NULL;
		if (fileExt == ".fsh")
		{
			shaderType = GL_FRAGMENT_SHADER;
		}
		else if (fileExt == ".vsh")
		{
			shaderType = GL_VERTEX_SHADER;
		}
		else continue;
		
		shaderID.push_back(compileShader(shaderType, inputShader));
	}
	
	GLuint programID = gl::CreateProgram();

	linkProgram(programID, shaderID);
	
	for (auto id : shaderID) { gl::DeleteShader(id); }

	saveBinary(programID, os);

	gl::DeleteProgram(programID);

	return 0;
}
