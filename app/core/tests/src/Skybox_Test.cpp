#include "Context.hpp"
#include "Skybox.hpp"

#include <GL/glew.h>
#include <gtest/gtest.h>

#include <exception>
	

namespace szakdoga::test {
    TEST(Skybox_Test, TestConstructor) {
        try {
            using namespace szakdoga::test;
            Context context{};
            
            using namespace szakdoga::core;
            Skybox skybox{};
            
            EXPECT_GT(skybox.GetTextureID(), 0);
            EXPECT_TRUE(glIsTexture(skybox.GetTextureID()));
            
        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }
} // namespace szakdoga::test