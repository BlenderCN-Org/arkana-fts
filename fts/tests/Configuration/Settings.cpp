#include "dLib/dString/dString.h"
#include "Settings.h"

namespace FTS {

Settings::Settings(const String& in_DefaultLanguage)
{
    add("HRes", 1024);
    add("VRes", 768);
    add("MouseScrollSpeed", 75);
    add("KbdScrollSpeed", 50);
    add("DebugLevel", 1);
    add("ModelDetails", 2);
    add("BPP", 32);
    add("TextureFilter", 2);
    add("MenuBGMove", 0);
    add("MasterServerPort", 4711/*D_DEFAULT_SERVER_PORT*/);
    add("SoundVolumeMusic", 8);
    add("SoundVolumeSFXUnitReaction", 10);
    add("SoundVolumeSFXAction", 100);
    add("SoundVolumeSFXEnvironment", 100);
    add("SoundVolumeSFXAttention", 100);
    add("SoundVolumeSFXMagic", 100);
    add("MultiTexturing", true);
    add("Anisotropic", false);
    add("MenuMouseWarp", true);
    add("ComplexQuads", true);
    add("Fullscreen", false);
    add("SoundEnabled", true);
    add("ClearChatbox", true);
    add("Language", in_DefaultLanguage);
    add("MenuBGFile", "HelloWorld");
    add("MasterServerName", "arkana-fts.servegame.org"/*D_DEFAULT_SERVER_NAME*/);
    add("LastLogin", "");
    add("DefaultChannel", "Talk To Survive (main channel)");
    add("MaxShaderQuality", "Best");
    add("ModelRenderTechnique", "Shader");
}

Settings::~Settings(void)
{
}


}