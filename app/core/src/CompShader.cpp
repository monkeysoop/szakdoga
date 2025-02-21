#include <string>
#include <fstream>

#include <SDL2/SDL.h>

#include "CompShader.hpp"

GLuint CompShader::m_currently_used_id{0}; 

void compileShaderFromSource(const GLuint loadedShader, std::string_view shaderCode) {
    // kod hozzarendelese a shader-hez
    const char* sourcePointer = shaderCode.data();
    GLint sourceLength = static_cast<GLint>(shaderCode.length());

    glShaderSource(loadedShader, 1, &sourcePointer, &sourceLength);

    // shader leforditasa
    glCompileShader(loadedShader);

    // ellenorizzuk, h minden rendben van-e
    GLint result = GL_FALSE;
    int infoLogLength;

    // forditas statuszanak lekerdezese
    glGetShaderiv(loadedShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(loadedShader, GL_INFO_LOG_LENGTH, &infoLogLength);

    if ((GL_FALSE == result) || (infoLogLength != 0)) {
        // hibauzenet elkerese es kiirasa
        std::string ErrorMessage(infoLogLength, '\0');
        glGetShaderInfoLog(loadedShader, infoLogLength, NULL, ErrorMessage.data());

        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, (result) ? SDL_LOG_PRIORITY_WARN : SDL_LOG_PRIORITY_ERROR, "[glLinkProgram] Shader compile error: %s", ErrorMessage.data());
    }
}
void loadShader(const GLuint loadedShader, const std::filesystem::path& _fileName) {
    // ha nem sikerult hibauzenet es -1 visszaadasa
    if (loadedShader == 0) {
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Shader needs to be inited before loading %s !", _fileName.c_str());
        return;
    }

    // shaderkod betoltese _fileName fajlbol
    std::string shaderCode = "";

    // _fileName megnyitasa
    std::ifstream shaderStream(_fileName);
    if (!shaderStream.is_open()) {
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error while loading shader %s!", _fileName.c_str());
        return;
    }

    // file tartalmanak betoltese a shaderCode string-be
    std::string line = "";
    while (std::getline(shaderStream, line)) {
        shaderCode += line + "\n";
    }

    shaderStream.close();

    compileShaderFromSource(loadedShader, shaderCode);
}

CompShader::CompShader(const std::filesystem::path& comp_filename) {
    m_program_id = glCreateProgram();

    if (m_program_id == 0)
        return;

    GLuint comp_id = glCreateShader(GL_COMPUTE_SHADER);

    if (comp_id == 0) {
        SDL_SetError("Error while initing shaders (glCreateShader)!");
    }

    loadShader(comp_id, comp_filename);

    // adjuk hozzá a programhoz a shadereket
    glAttachShader(m_program_id, comp_id);

    // illesszük össze a shadereket (kimenő-bemenő változók összerendelése stb.)
    glLinkProgram(m_program_id);

    // linkeles ellenorzese
    GLint infoLogLength = 0, result = 0;

    glGetProgramiv(m_program_id, GL_LINK_STATUS, &result);
    glGetProgramiv(m_program_id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (GL_FALSE == result || infoLogLength != 0) {
        std::string ErrorMessage(infoLogLength, '\0');
        glGetProgramInfoLog(m_program_id, infoLogLength, nullptr, ErrorMessage.data());
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, (result) ? SDL_LOG_PRIORITY_WARN : SDL_LOG_PRIORITY_ERROR, "[glLinkProgram] Shader linking error: %s", ErrorMessage.data());
    }

    // mar nincs ezekre szukseg
    glDeleteShader(comp_id);
}

CompShader::~CompShader() {
    glDeleteProgram(m_program_id);
}

void CompShader::Use() {
    m_currently_used_id = m_program_id;
    glUseProgram(m_program_id);
}

void CompShader::Dispatch(GLuint x, GLuint y, GLuint z) {
    glDispatchCompute(x, y, z);
}

void CompShader::Barrier(GLbitfield barriers) {
    glMemoryBarrier(barriers);
}

GLint CompShader::ul(const GLchar* name) {
    GLint location = glGetUniformLocation(m_program_id, name);
    if (location == -1) {
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error finding uniform: %s  program id: %d (possibly got optimised away)", name, m_program_id);
    }
    if (m_program_id != m_currently_used_id) {
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Warning getting the location of a uniform of a program that is currently not being used (uniform name: %s)", name);
    }
    return location;
}

GLuint CompShader::GetProgramID() {
    return m_program_id;
}