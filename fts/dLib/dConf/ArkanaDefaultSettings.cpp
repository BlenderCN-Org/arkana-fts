#include "dLib/dConf/ArkanaDefaultSettings.h"

#include "3d/Resolution.h"

namespace FTS {

ArkanaDefaultSettings::ArkanaDefaultSettings()
{
    Resolution currRes = Resolution();
    add("HRes", currRes.w != 1 ? currRes.w : 1024);
    add("VRes", currRes.h != 1 ? currRes.h : 768);
    add("MouseScrollSpeed", 75);
    add("KbdScrollSpeed", 50);
    add("DebugLevel", 1);
    add("ModelDetails", 2);
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
    add("Fullscreen", true);
    add("SoundEnabled", true);
    add("ClearChatbox", true);
    String sDefLang;
    #if WINDOOF
    LANGID lang = GetUserDefaultUILanguage();
    switch(PRIMARYLANGID(lang)) {
        case LANG_FRENCH: sDefLang = "French";  break;
        case LANG_GERMAN: sDefLang = "German";  break;
        default:          sDefLang = "English"; break;
    }
    #else
    char *p = getenv("LANG");
    if(String(p).nieq("fr", 2)) {
        sDefLang = "French";
    } else if(String(p).nieq("de", 2)) {
        sDefLang = "German";
    } else {
        sDefLang = "English";
    }
    #endif
    add("Language", sDefLang);
    add("MenuBGFile", "HelloWorld");
    add("MasterServerName", "arkana-fts.servegame.org"/*D_DEFAULT_SERVER_NAME*/);
    add("LastLogin", "");
    add("DefaultChannel", "Talk To Survive (main channel)");
    add("MaxShaderQuality", "Best");
    add("ModelRenderTechnique", "Shader");
}

ArkanaDefaultSettings::~ArkanaDefaultSettings(void)
{
}


}