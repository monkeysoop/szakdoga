#pragma once

#include <GL/glew.h>

#include <filesystem>
#include <vector>
#include <string>



namespace szakdoga::core {
    class CompShader {
    public:
        CompShader(const std::filesystem::path& comp_filename, const std::vector<std::filesystem::path>& include_filenames = {});
        ~CompShader();

        void Use();
        void Dispatch(unsigned x, unsigned y, unsigned z);
        void Barrier(GLbitfield barriers);
        GLint ul(const GLchar* name);
        GLuint GetProgramID();

    private:
        GLuint m_program_id;
        static GLuint m_currently_used_id;

    private:
        std::string LoadShader(const std::filesystem::path& comp_filename, const std::vector<std::filesystem::path>& include_filenames);
        void CompileShader(GLuint shader_id, const std::string& source_code);
    };
} // namespace szakdoga::core