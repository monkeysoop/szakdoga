#include "Context.hpp"
#include "CompShader.hpp"

#include <GL/glew.h>
#include <gtest/gtest.h>

#include <exception>



namespace szakdoga::test {
    TEST(CompShader_Test, TestConstructor) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            CompShader comp_shader{
                std::filesystem::path{"assets"} / "test.comp", 
                {
                    std::filesystem::path{"assets"} / "test_1.include",
                    std::filesystem::path{"assets"} / "test_2.include",
                }, 
                {
                    {"VAR_A", "123"},
                    {"VAR_B", "111"}
                }
            };

            EXPECT_GT(comp_shader.GetProgramID(), 0);
            EXPECT_TRUE(glIsProgram(comp_shader.GetProgramID()));

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }

    TEST(CompShader_Test, TestRecompile) {
        try {
            using namespace szakdoga::test;
            Context context{};

            using namespace szakdoga::core;
            CompShader comp_shader{
                std::filesystem::path{"assets"} / "test.comp", 
                {
                    std::filesystem::path{"assets"} / "test_1.include",
                    std::filesystem::path{"assets"} / "test_2.include",
                }, 
                {
                    {"VAR_A", "123"},
                    {"VAR_B", "222"}
                }
            };

            EXPECT_GT(comp_shader.GetProgramID(), 0);
            EXPECT_TRUE(glIsProgram(comp_shader.GetProgramID()));

            comp_shader.Recompile(
                {
                    {"VAR_A", "456"},
                    {"VAR_B", "111"}
                }
            );

            EXPECT_GT(comp_shader.GetProgramID(), 0);
            EXPECT_TRUE(glIsProgram(comp_shader.GetProgramID()));

        } catch (const std::exception& e) {
            FAIL() << "Exception while running test(s): " << e.what();
        }
    }
} // namespace szakdoga::test