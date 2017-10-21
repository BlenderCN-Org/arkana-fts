#include "dLib/aTest/TestHarness.h"

#include "3d/3d.h"
#include "3d/Shader.h"
#include "logging/MinimalLogger.h"
#include "dLib/dString/dPath.h"
#include "dLib/dCompressor/dCompressor.h"

#include <SDL.h>

using namespace FTS;

class OpenGLSetup : public TestSetup
{
public:
    void setup()
    {
        Logger* pLog = new MinimalLogger(1);
        pLog->stfu();

        // We need an OpenGL context, of course!
        if(SDL_Init(SDL_INIT_VIDEO) < 0)
            throw std::runtime_error("Could not initialize SDL");
        m_pWindow = SDL_CreateWindow("Test SDL screen", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WindowFlags::SDL_WINDOW_OPENGL);
        if(!m_pWindow)
            throw std::runtime_error("Could not open OpenGL window");
        m_glContext = SDL_GL_CreateContext(m_pWindow);
        m_bHasShaderIncludeExtension = glHasExtension("GL_ARB_shader_include");
    };

    void teardown()
    {
        SDL_GL_DeleteContext(m_pWindow);
        SDL_Quit();
        delete Logger::getSingletonPtr();
    };
    SDL_Window* m_pWindow = nullptr;
    SDL_GLContext m_glContext;
    bool m_bHasShaderIncludeExtension;
};

SUITE_WITHSETUP(Shaders, OpenGL);

TEST_INSUITE(Shaders, ConstructionDestruction)
{
    new ShaderManager();
    delete ShaderManager::getSingletonPtr();
}

TEST_INSUITE(Shaders, SimpleInclude)
{
    ShaderManager* mgr = new ShaderManager();
    
    String sIncludedFile =
"// This is a comment in the included file\n"
"\n"
"// This is yet another comment in the included file\n";
    mgr->loadShaderCode("Bla.shadinc", sIncludedFile);

    String sMainFile =
"#version 130\n"
"#include \"Bla.shadinc\"\n"
"// And this comment resides in the main file.\n"
"void main ()\n"
"{\n"
"    gl_Position = vec4 (0.0, 0.0, 0.0, 1.0);\n"
"}\n";
    mgr->loadShaderCode("Bla.vert", sMainFile);

    String sExpected =
"#version 130\n" + 
(Shaders.m_bHasShaderIncludeExtension ? String("") : String("#line 0\n")) +
sIncludedFile + "\n" +
(Shaders.m_bHasShaderIncludeExtension ? String("") : String("#line 2\n")) +
"// And this comment resides in the main file.\n"
"void main ()\n"
"{\n"
"    gl_Position = vec4 (0.0, 0.0, 0.0, 1.0);\n"
"}\n";

    CHECK_EQUAL(sExpected, mgr->getCompiledShaderSource("Bla.vert"));

    delete ShaderManager::getSingletonPtr();
}

TEST_INSUITE(Shaders, CyclicInclude)
{
    ShaderManager* mgr = new ShaderManager();
    
    String sIncludedFile1 = "// This is a comment in the first included file\n"
        "#include \"Bla2.shadinc\"\n";
    mgr->loadShaderCode("Bla1.shadinc", sIncludedFile1);
    String sIncludedFile2 = "// This is a comment in the second included file\n";
    mgr->loadShaderCode("Bla2.shadinc", sIncludedFile2);

    String sMainFile =
"#version 130\n"
"#include \"Bla1.shadinc\"\n"
"// And this comment resides in the main file.\n"
"void main ()\n"
"{\n"
"    gl_Position = vec4 (0.0, 0.0, 0.0, 1.0);\n"
"}\n";
    mgr->loadShaderCode("Bla.vert", sMainFile);

    String sExpected =
"#version 130\n" + 
(Shaders.m_bHasShaderIncludeExtension ? String("") : String("#line 0\n")) +
"// This is a comment in the first included file\n" +
(Shaders.m_bHasShaderIncludeExtension ? String("") : String("#line 0\n")) +
"// This is a comment in the second included file\n\n" +
(Shaders.m_bHasShaderIncludeExtension ? String("") : String("#line 4\n")) + "\n" +
(Shaders.m_bHasShaderIncludeExtension ? String("") : String("#line 2\n")) +
"// And this comment resides in the main file.\n"
"void main ()\n"
"{\n"
"    gl_Position = vec4 (0.0, 0.0, 0.0, 1.0);\n"
"}\n";

    CHECK_EQUAL(sExpected, mgr->getCompiledShaderSource("Bla.vert"));

    delete ShaderManager::getSingletonPtr();
}
