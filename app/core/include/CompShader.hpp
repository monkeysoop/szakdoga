#pragma once

#include <GL/glew.h>

#include <filesystem>
#include <vector>
#include <string>
#include <map>



namespace szakdoga::core {
    class CompShader {
    public:
        CompShader(
            const std::filesystem::path& comp_filename, 
            const std::vector<std::filesystem::path>& include_filenames = {},
            const std::map<std::string, std::string>& in_variables = {}
        );
        ~CompShader();

        void Use();
        void Dispatch(unsigned x, unsigned y, unsigned z);
        void Barrier(GLbitfield barriers);
        GLint ul(const GLchar* name);
        GLuint GetProgramID();
        void Recompile(const std::map<std::string, std::string>& in_variables);

    private:
        GLuint m_program_id;
        std::string m_shader_source_code;
        static GLuint m_currently_used_id;

    private:
        std::string LoadShader(const std::filesystem::path& comp_filename, const std::vector<std::filesystem::path>& include_filenames);
        std::string ConfigureShader(const std::map<std::string, std::string>& in_variables);
        void AttachShader(const std::string& configured_shader_source_code);
    };
} // namespace szakdoga::core