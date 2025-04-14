#include "Context.hpp"
#include "Texture2D.hpp"

#include <GL/glew.h>
#include <gtest/gtest.h>

#include <exception>
	

namespace szakdoga::test {
    TEST(Texture2D_Test, TestConstructor) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            Texture2D texture2D_1{128, 256, GL_RG32F};

            EXPECT_GT(texture2D_1.GetTextureID(), 0);
            EXPECT_TRUE(glIsTexture(texture2D_1.GetTextureID()));

            glBindTexture(GL_TEXTURE_2D, texture2D_1.GetTextureID());

            GLint width_1 = 0;
            GLint height_1 = 0;
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width_1);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height_1);

            EXPECT_EQ(width_1, 128);
            EXPECT_EQ(height_1, 256);

            GLint format_1 = 0;
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format_1);
            EXPECT_EQ(format_1, GL_RG32F);


            Texture2D texture2D_2{64, 512, GL_RGBA32F, 3};

            glBindTexture(GL_TEXTURE_2D, texture2D_2.GetTextureID());

            GLint format_2 = 0;
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format_2);
            EXPECT_EQ(format_2, GL_RGBA32F);

            GLint width_2 = 0;
            GLint height_2 = 0;
            unsigned levels = 0;
            while (true) {
                glGetTexLevelParameteriv(GL_TEXTURE_2D, levels, GL_TEXTURE_WIDTH, &width_2);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, levels, GL_TEXTURE_HEIGHT, &height_2);

                if ((width_2 == 0) || (height_2 == 0)) {
                    break;
                }

                levels++;
            }

            EXPECT_EQ(levels, 3);

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }

    TEST(Texture2D_Test, TestResize) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            Texture2D texture2D_1{128, 256, GL_RG32F};

            texture2D_1.Resize(64, 512);

            glBindTexture(GL_TEXTURE_2D, texture2D_1.GetTextureID());

            GLint width_1 = 0;
            GLint height_1 = 0;
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width_1);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height_1);

            EXPECT_EQ(width_1, 64);
            EXPECT_EQ(height_1, 512);

            Texture2D texture2D_2{64, 512, GL_RGBA32F, 3};

            texture2D_2.Resize(64, 512, 5);

            glBindTexture(GL_TEXTURE_2D, texture2D_2.GetTextureID());

            GLint width_2 = 0;
            GLint height_2 = 0;
            unsigned levels = 0;
            while (true) {
                glGetTexLevelParameteriv(GL_TEXTURE_2D, levels, GL_TEXTURE_WIDTH, &width_2);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, levels, GL_TEXTURE_HEIGHT, &height_2);

                if ((width_2 == 0) || (height_2 == 0)) {
                    break;
                }

                levels++;
            }

            EXPECT_EQ(levels, 5);

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }

    TEST(Texture2D_Test, TestBind) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            Texture2D texture2D{128, 256, GL_RG32F, 2};

            texture2D.Bind(3, GL_READ_ONLY, 1);

            GLint bound_id;
            glGetIntegeri_v(GL_IMAGE_BINDING_NAME, 3, &bound_id);
            EXPECT_EQ(static_cast<GLuint>(bound_id), texture2D.GetTextureID());

            GLint bound_level = 0;
            glGetIntegeri_v(GL_IMAGE_BINDING_LEVEL, 3, &bound_level);
            EXPECT_EQ(bound_level, 1);

            GLint access = 0;
            glGetIntegeri_v(GL_IMAGE_BINDING_ACCESS, 3, &access);
            EXPECT_EQ(static_cast<GLenum>(access), GL_READ_ONLY);

            GLint format = 0;
            glGetIntegeri_v(GL_IMAGE_BINDING_FORMAT, 3, &format);
            EXPECT_EQ(format, GL_RG32F);

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }
} // namespace szakdoga::test