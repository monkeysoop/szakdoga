#include "CompShader.hpp"

#include <SDL2/SDL.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <stdexcept>
#include <iostream>



namespace szakdoga::core {
    GLuint CompShader::m_currently_used_id{0}; 

    CompShader::CompShader(
        const std::filesystem::path& comp_filename, 
        const std::vector<std::filesystem::path>& include_filenames,
        const std::map<std::string, std::string>& in_variables
    ) {
        m_program_id = glCreateProgram();

        if (m_program_id == 0) {
            throw std::runtime_error("Error during program creation");
        }

        m_shader_source_code = LoadShader(comp_filename, include_filenames);

        std::string configured_shader_source_code = ConfigureShader(in_variables);

        AttachShader(configured_shader_source_code);
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
            //SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error finding uniform: %s  program id: %d (possibly got optimised away)", name, m_program_id);
        }

        if (m_program_id != m_currently_used_id) {
            throw std::runtime_error("Warning getting the location of a uniform of a program that is currently not being used (uniform name: " + std::string(name) + ")");
        }

        return location;
    }

    GLuint CompShader::GetProgramID() {
        return m_program_id;
    }

    void CompShader::Recompile(const std::map<std::string, std::string>& in_variables) {
        glDeleteProgram(m_program_id);

        m_program_id = glCreateProgram();

        if (m_program_id == 0) {
            throw std::runtime_error("Error during program creation");
        }

        std::string configured_shader_source_code = ConfigureShader(in_variables);

        AttachShader(configured_shader_source_code);
    }


    std::string CompShader::LoadShader(const std::filesystem::path& filename, const std::vector<std::filesystem::path>& include_filenames) {
        std::string shader_source_code = "";

        std::ifstream shader_file{filename};
        if (!shader_file.is_open()) {
            throw std::runtime_error("Error while loading shader: " + filename.string());
        }

        std::string current_line = "";
        while (std::getline(shader_file, current_line)) {
            const std::string INCLUDE_TOKEN{"#include"};

            std::string::size_type include_pos = current_line.find(INCLUDE_TOKEN);

            if (include_pos != std::string::npos) {
                std::istringstream ss{current_line.substr(include_pos + INCLUDE_TOKEN.length() + 1)};
                std::string unquoted_include_filename;
                ss >> std::quoted(unquoted_include_filename); // this is very stupid but this essentially strips the quotes

                bool any_match = false;

                for (const std::filesystem::path& include_filename : include_filenames) {
                    if (include_filename.string() == unquoted_include_filename) {
                        std::string include_source_code = LoadShader(include_filename, include_filenames);

                        shader_source_code += include_source_code + "\n"; // the extra "\n" might be unnecesseary but couldn't hurt

                        any_match = true;
                        break;
                    }
                }

                if (!any_match) {
                    throw std::runtime_error("Error, included file not found, filename: " + unquoted_include_filename);
                }

            } else {
                shader_source_code += current_line + "\n";
            }
        }

        return shader_source_code;
    }

    std::string CompShader::ConfigureShader(const std::map<std::string, std::string>& in_variables) {
        std::string configured_shader_source_code = "";


        std::istringstream ss{m_shader_source_code};

        std::string current_line = "";
        while (std::getline(ss, current_line)) {
            const char IN_TOKEN = '@';

            std::string::size_type in_first = current_line.find(IN_TOKEN);
            std::string::size_type in_last = current_line.rfind(IN_TOKEN);

            if (in_first != std::string::npos && in_last != std::string::npos) {
                std::string::size_type len = in_last - in_first;

                if (len < 2) {
                    //SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error, incorrect usage of the %c symbol", IN_TOKEN);
                } else {
                    std::string in_var = current_line.substr((in_first + 1), (len - 1));

                    std::map<std::string, std::string>::const_iterator iter = in_variables.find(in_var);

                    if (iter == in_variables.end()) {
                        throw std::runtime_error("Error, couldn't find in variable: " + in_var + " in the map provided");
                    }

                    current_line.replace(in_first, (len + 1), iter->second);
                }
            }

            configured_shader_source_code += current_line + "\n";
        }

        return configured_shader_source_code;
    }    

    void CompShader::AttachShader(const std::string& configured_shader_source_code) {
        GLuint comp_id = glCreateShader(GL_COMPUTE_SHADER);

        if (comp_id == 0) {
            throw std::runtime_error("Error during shader creation");
        }

        const GLchar* source_data = static_cast<const GLchar*>(configured_shader_source_code.data());
        const GLint source_length = static_cast<GLint>(configured_shader_source_code.length());

        glShaderSource(comp_id, 1, &source_data, &source_length);

        glCompileShader(comp_id);

        GLint compile_result = GL_FALSE;
        GLint compile_info_log_length = 0;

        glGetShaderiv(comp_id, GL_COMPILE_STATUS, &compile_result);
        glGetShaderiv(comp_id, GL_INFO_LOG_LENGTH, &compile_info_log_length);

        if ((compile_result == GL_FALSE) || (compile_info_log_length != 0)) {
            std::string error_message(compile_info_log_length, '\0');

            glGetShaderInfoLog(comp_id, compile_info_log_length, NULL, error_message.data());

            std::string message{"Shader compile error: \n" + error_message};

            if (compile_result == GL_FALSE) {
                throw std::runtime_error(message);
            } else {
                std::cerr << message << std::endl;
            }
        }

        glAttachShader(m_program_id, comp_id);
        glLinkProgram(m_program_id);

        GLint link_result = GL_FALSE;
        GLint link_info_log_length = 0;

        glGetProgramiv(m_program_id, GL_LINK_STATUS, &link_result);
        glGetProgramiv(m_program_id, GL_INFO_LOG_LENGTH, &link_info_log_length);

        if ((link_result == GL_FALSE) || (link_info_log_length != 0)) {
            std::string error_message(link_info_log_length, '\0');
            glGetProgramInfoLog(m_program_id, link_info_log_length, nullptr, error_message.data());

            std::string message{"Shader linking error: \n" + error_message};

            if (compile_result == GL_FALSE) {
                throw std::runtime_error(message);
            } else {
                std::cerr << message << std::endl;
            }
        }

        glDeleteShader(comp_id);
    }
} // namespace szakdoga::core