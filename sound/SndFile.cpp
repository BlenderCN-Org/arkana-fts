/*
 * \file SndFile.cpp
 *
 * \author kabey
 * \brief Implementing the sound file object.
 */

#include "fts_Snd.h"
#include "SndFile.h"

#include "dLib/dFile/dFile.h"

#include <vorbis/vorbisfile.h>
#include <vector>

using namespace FTS;

// Wrapper functions for the vorbisfile call backs.
size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    return static_cast<File*>(datasource)->readNoEndian(ptr, size*nmemb);
}
int seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    uint64_t pos = 0;
    switch(whence) {
    case SEEK_SET:
        pos = offset ;
        break;
    case SEEK_CUR:
        pos = static_cast<File*>(datasource)->getCursorPos() + offset;
        break;
    case SEEK_END:
        pos = static_cast<File*>(datasource)->getSize();
        break;
    }
    static_cast<File*>(datasource)->setCursorPos(pos);
    return 0;
}

long tell_func(void *datasource)
{
    return (long) static_cast<File*>(datasource)->getCursorPos();
}

static ov_callbacks OV_CALLBACKS_WRAPPER = {
  (size_t (*)(void *, size_t, size_t, void *))  read_func,
  (int (*)(void *, ogg_int64_t, int))           seek_func,
  (int (*)(void *))                             NULL,
  (long (*)(void *))                            tell_func
};


#if D_SND_SYS == D_FTS_OpenAL

typedef unsigned char PCMDATA ;

FTS::SndFile::~SndFile()
{
}

/*! Construct a SndFile object according to a given file name. The file extenion
 * classifies the music file type. Supported types are wav and ogg. The SndFile is
 * a vehicle to extract the PCM data and convert it into a AL buffer.
 *
 * @author Klaus.Beyer
 *
 * @param[in] in_sName The name of the sound to be loaded (relative to the sounds directory).
 * @return SndFile * this
 * @note For testing raw is supported as PCM-16 2 channels.
 *
 * @throw LoggableException from the File::Open method.
 * @throw CorruptDataException if an errors occurs while reading the file
 * @throw OpenALException if an error occurs while creating the openAL sound.
 */
FTS::SndFile::SndFile(const Path& in_sName)
    : m_size(0)
    , m_format(AL_FORMAT_STEREO16)
    , m_samplingRate(44100)
    , m_alBuffer(0)
{
    Path sCompleteFilename = Path::datadir("Sounds") + in_sName;
    File::Ptr fraw = File::open(sCompleteFilename, File::Read);

    // We either need a trailing zero or we could tell the string constructor
    // the length. I chose the former to fix this.
    char ident[4] = {0};
    if(fraw->read(ident, 1, 3) != 3)
        throw CorruptDataException(sCompleteFilename, "Unexpected end of file");
    fraw->setCursorPos(0);

    if(String(ident).nieq("ogg")) {
        OggVorbis_File vf;
        int eof=0;
        int current_section;
        if(ov_open_callbacks(fraw.get(), &vf, NULL, 0, OV_CALLBACKS_WRAPPER) < 0)
            throw CorruptDataException(sCompleteFilename, "Input does not appear to be an Ogg bitstream.");

        vorbis_info *vi=ov_info(&vf,-1);

        /* Throw the comments plus a few lines about the bitstream we're
        decoding */
        {
            char **ptr=ov_comment(&vf,-1)->user_comments;
            while(*ptr){
                FTSMSGDBG("{1}", 3, *ptr);
                ++ptr;
            }

            FTSMSGDBG("Bitstream is {1} channel, {2}Hz", 3, String::nr((int32_t)vi->channels), String::nr((int32_t)vi->rate));
            FTSMSGDBG("Decoded length: {1} samples", 3, String::nr(ov_pcm_total(&vf,-1)));
            FTSMSGDBG("Encoded by: {1}",  3, ov_comment(&vf,-1)->vendor);
        }

        m_size = static_cast<size_t>(ov_pcm_total(&vf,-1) * vi->channels * 2) ;
        m_samplingRate = vi->rate ;
        std::vector<PCMDATA> pcm(m_size);
        size_t remainingSize = m_size;
        PCMDATA* currentPCMptr = &pcm[0];
        while(!eof){
            long ret=ov_read(&vf,(char*)currentPCMptr,(int)remainingSize ,0,2,1,&current_section);
            if (ret == 0) {
                /* EOF */
                eof=1;
            } else if (ret < 0) {
                /* error in the stream.  Not a problem, just reporting it in
                case we (the app) cares.  In this case, we don't. */
                FTSMSGDBG("ov_read returns error : section {1} remaining size {2}",3, String::nr(current_section), String::nr(remainingSize));
                eof=1;
                //throw CorruptDataException(sCompleteFilename, "ov_read returns error\n");
            } else {
                /* we don't bother dealing with sample rate changes, etc, but
                you'll have to*/
                currentPCMptr += ret ;
                remainingSize -= ret;
            }
        }

        alGenBuffers(1,&m_alBuffer);
        if(m_alBuffer == 0)
            throw OpenALException(sCompleteFilename);

        alBufferData(m_alBuffer ,vi->channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, &pcm[0], (int)m_size, m_samplingRate);
        ov_clear(&vf);
    } else {
        // No more .wav/.ao
        throw SndErrException(sCompleteFilename, "Only *.ogg sound files are supported.");
    }
}

void FTS::SndFile::deleteBuffer(ALuint bufferID)
{
    alDeleteBuffers(1, &bufferID); // delete the current buffer.
}
#endif
