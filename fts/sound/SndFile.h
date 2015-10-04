/*
 * $Id:$
 * \file SndFile.h
 * \author KaBey
 * \brief Interface file for the sound file object
 * \todo -
 */

#pragma once

namespace FTS {
    class Path;

#if D_SND_SYS == D_FTS_OpenAL

/*! The sound file interface class.
 *
 * @author Klaus.Beyer
 */
class SndFile
{
public:
    explicit SndFile(const Path &fileName);
    size_t getSize() const {return m_size;}
    ALuint getBuffer() const {return m_alBuffer;}
    operator ALuint() const {return m_alBuffer;}
    static void deleteBuffer(ALuint bufferID);
    virtual ~SndFile();

private:
    size_t m_size;
    ALint m_format;
    ALint m_samplingRate;
    ALuint m_alBuffer;
};

#endif

} // namespace FTS
