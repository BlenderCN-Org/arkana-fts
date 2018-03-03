#ifndef FTS_TRANSLATION_H
#define FTS_TRANSLATION_H


namespace FTS {

    class Translation {
    public:
        Translation(const class String& in_sFile);
        virtual ~Translation() ;
        String get(String in_sString);
        void setNoLogging(bool noLogging = true) {m_bLogging = !noLogging;} // Useful for unit testing
    private:
        class Configuration* m_confTranslationDefault;
        class Configuration* m_confTranslation;
        bool m_bLogging;
    };

}

#endif