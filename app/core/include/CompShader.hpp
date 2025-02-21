#pragma once

#include <GL/glew.h>

#include <filesystem>

class CompShader {
public:
    CompShader(const std::filesystem::path& comp_filename);
    ~CompShader();

    void Use();
    void Dispatch(GLuint x, GLuint y, GLuint z);
    void Barrier(GLbitfield barriers);
    GLint ul(const GLchar* name);
    GLuint GetProgramID();

private:
    GLuint m_program_id;
    static GLuint m_currently_used_id;
};