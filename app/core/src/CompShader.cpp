#include <SDL2/SDL.h>

#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include <iterator>

#include "CompShader.hpp"



GLuint CompShader::m_currently_used_id{0}; 


CompShader::CompShader(const std::filesystem::path& comp_filename, const std::vector<std::filesystem::path>& include_filenames) {
    m_program_id = glCreateProgram();

    if (m_program_id == 0) {
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error creating program!");
    }

    GLuint comp_id = glCreateShader(GL_COMPUTE_SHADER);

    if (comp_id == 0) {
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error creating shader!");
    }

    std::string shader_source_code = LoadShader(comp_filename, include_filenames);

    CompileShader(comp_id, shader_source_code);

    glAttachShader(m_program_id, comp_id);

    glLinkProgram(m_program_id);

    GLint result = GL_FALSE;
    GLint info_log_length = 0;

    glGetProgramiv(m_program_id, GL_LINK_STATUS, &result);
    glGetProgramiv(m_program_id, GL_INFO_LOG_LENGTH, &info_log_length);

    if ((result == GL_FALSE) || (info_log_length != 0)) {
        std::string error_message(info_log_length, '\0');
        glGetProgramInfoLog(m_program_id, info_log_length, nullptr, error_message.data());
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, (result) ? SDL_LOG_PRIORITY_WARN : SDL_LOG_PRIORITY_ERROR, "[glLinkProgram] Shader linking error: %s", error_message.data());
    }

    glDeleteShader(comp_id);
}

CompShader::~CompShader() {
    glDeleteProgram(m_program_id);
}

void CompShader::Use() {
    m_currently_used_id = m_program_id;
    glUseProgram(m_program_id);
}

void CompShader::Dispatch(unsigned x, unsigned y, unsigned z) {
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

std::string CompShader::LoadShader(const std::filesystem::path& comp_filename, const std::vector<std::filesystem::path>& include_filenames) {
    std::string shader_source_code = "";

    std::ifstream shader_file(comp_filename);
    if (!shader_file.is_open()) {
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error while loading shader: %s!", comp_filename.c_str());
    }

    std::string current_line = "";
    while (std::getline(shader_file, current_line)) {
        const std::string include_token{"#include"};
        
        std::string::size_type include_pos = current_line.find(include_token);

        if (include_pos != std::string::npos) {
            std::istringstream ss{current_line.substr(include_pos + include_token.length() + 1)};
            std::string unquoted_include_filename;
            ss >> std::quoted(unquoted_include_filename); // this is very stupid but this essentially strips the quotes

            bool any_match = false;

            for (const std::filesystem::path& include_filename : include_filenames) {
                if (include_filename.string() == unquoted_include_filename) {
                    std::ifstream include_file(include_filename);

                    if (!include_file.is_open()) {
                        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error while loading included shader: %s!", include_filename.c_str());
                    }

                    std::string include_source_code{
                        std::istreambuf_iterator<char>(include_file),
                        std::istreambuf_iterator<char>()
                    };

                    shader_source_code += include_source_code + "\n"; // the extra "\n" might be unnecesseary but couldn't hurt

                    any_match = true;
                    break;
                }
            }
            if (!any_match) {
                SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error, included file not found, filename: %s!", unquoted_include_filename.c_str());
            }

        } else {
            shader_source_code += current_line + "\n";
        }
    }

    return shader_source_code;
}

void CompShader::CompileShader(GLuint shader_id, const std::string& source_code) {
    const GLchar* source_data = static_cast<const GLchar*>(source_code.data());
    const GLint source_length = static_cast<GLint>(source_code.length());

    glShaderSource(shader_id, 1, &source_data, &source_length);

    glCompileShader(shader_id);

    GLint result = GL_FALSE;
    GLint info_log_length;

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

    if ((result == GL_FALSE) || (info_log_length != 0)) {
        std::string error_message(info_log_length, '\0');

        glGetShaderInfoLog(shader_id, info_log_length, NULL, error_message.data());

        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, (result) ? SDL_LOG_PRIORITY_WARN : SDL_LOG_PRIORITY_ERROR, "[glLinkProgram] Shader compile error: %s", error_message.data());
    }
}