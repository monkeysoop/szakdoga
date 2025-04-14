#include "Context.hpp"
#include "Framebuffer.hpp"

#include <GL/glew.h>
#include <gtest/gtest.h>

#include <exception>
#include <filesystem>



namespace szakdoga::test {
    TEST(Framebuffer_Test, TestConstructor) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            Framebuffer framebuffer{300, 400};

            framebuffer.Bind();
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            EXPECT_EQ(status, GL_FRAMEBUFFER_COMPLETE);
            framebuffer.UnBind();

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }

    TEST(Framebuffer_Test, TestBind) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            Framebuffer framebuffer{1301, 1207};

            framebuffer.Bind();

            GLint viewport[4] = {0};
            glGetIntegerv(GL_VIEWPORT, viewport);
            EXPECT_EQ(viewport[2], 1301);
            EXPECT_EQ(viewport[3], 1207);

            GLint current_framebuffer = 0;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_framebuffer);
            EXPECT_GT(current_framebuffer, 0);

            framebuffer.UnBind();

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }

    TEST(Framebuffer_Test, TestUnBind) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            Framebuffer framebuffer{301, 207};

            framebuffer.Bind();
            framebuffer.UnBind();

            GLint current_framebuffer = 0;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_framebuffer);
            EXPECT_EQ(current_framebuffer, 0);

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }

    TEST(Framebuffer_Test, TestResize) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            Framebuffer framebuffer{101, 237};

            framebuffer.Bind();

            GLint viewport[4] = {0};
            glGetIntegerv(GL_VIEWPORT, viewport);
            EXPECT_EQ(viewport[2], 101);
            EXPECT_EQ(viewport[3], 237);

            framebuffer.UnBind();

            framebuffer.Resize(234, 145);

            framebuffer.Bind();

            glGetIntegerv(GL_VIEWPORT, viewport);
            EXPECT_EQ(viewport[2], 234);
            EXPECT_EQ(viewport[3], 145);

            framebuffer.Resize(45, 22);

            glGetIntegerv(GL_VIEWPORT, viewport);
            EXPECT_EQ(viewport[2], 45);
            EXPECT_EQ(viewport[3], 22);

            framebuffer.UnBind();

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }

    TEST(Framebuffer_Test, TestBlit) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            Framebuffer framebuffer{101, 237};

            framebuffer.Bind();

            glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
            glClear(GL_COLOR_BUFFER_BIT);

            framebuffer.UnBind();

            framebuffer.Blit();

            unsigned char pixel[4];
            glReadPixels(50, 118, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

            EXPECT_EQ(pixel[0], 255);
            EXPECT_EQ(pixel[1], 0);
            EXPECT_EQ(pixel[2], 0);

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }

    TEST(Framebuffer_Test, TestScreenshot) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            Framebuffer framebuffer{101, 237};

            framebuffer.Bind();

            glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
            glClear(GL_COLOR_BUFFER_BIT);

            framebuffer.UnBind();

            std::filesystem::path path{"Framebuffer_Test_testscreenshot.png"};

            framebuffer.Screenshot(path);

            EXPECT_TRUE(std::filesystem::exists(path));
            EXPECT_GT(std::filesystem::file_size(path), 0);

            std::filesystem::remove(path);

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }
} // namespace szakdoga::test