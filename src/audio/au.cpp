#include <audio/au.h>
#include <al/alc.h>
#include <al/al.h>
#include <gfx/gfx.h>
#include <vector>
#include <core/log.h>
#include <memory>

using namespace flux::gfx;

namespace flux::au
{

track::~track()
{
    alDeleteBuffers(1, &__track_id);
}

clip::~clip()
{
    alDeleteSources(1, &__clip_id);
}

clip_status clip::status()
{
    int sts;
    alGetSourcei(__clip_id, AL_SOURCE_STATE, &sts);
    if (sts == AL_STOPPED)
        return FX_CLIP_END;
    if (sts == AL_PLAYING)
        return FX_CLIP_PLAYING;
    if (sts == AL_INITIAL)
        return FX_CLIP_IDLE;
    return FX_CLIP_PAUSED;
}

void clip::set(clip_op param, double v)
{
    if (param == FX_CLIP_GAIN)
        alSourcef(__clip_id, AL_GAIN, v);
    else if (param == FX_CLIP_PITCH)
        alSourcef(__clip_id, AL_PITCH, v);
}

void clip::set(clip_op param, const vec2 &v)
{
    if (param == FX_CLIP_LOCATION)
        alSource3f(__clip_id, AL_POSITION, v.x, v.y, 0);
}

void clip::set(clip_op param, const vec3 &v)
{
    if (param == FX_CLIP_LOCATION)
        alSource3f(__clip_id, AL_POSITION, v.x, v.y, 0);
}

void clip::operate(clip_op param)
{
    if (param == FX_CLIP_PLAY)
        alSourcePlay(__clip_id);
    else if (param == FX_CLIP_LOOP)
    {
        alSourcei(__clip_id, AL_LOOPING, AL_TRUE);
        alSourcePlay(__clip_id);
    }
    else if (param == FX_CLIP_PAUSE)
        alSourcePause(__clip_id);
    else if (param == FX_CLIP_STOP)
        alSourceStop(__clip_id);
}

// global al usage
ALCdevice *al_dev;
ALCcontext *al_ctx;
std::vector<shared<clip>> clip_r;
double rolloff = 1.0;
double max_dist = 1.0;
double ref_dist = 1.0;

void __process_tracks()
{
    auto it = clip_r.begin();
    while (it != clip_r.end())
    {
        auto &cl = *it;
        if (cl->status() == FX_CLIP_END)
            it = clip_r.erase(it);
        else
            ++it;
    }
}

void tk_make_device()
{
    // init openal
    al_dev = alcOpenDevice(nullptr);
    al_ctx = alcCreateContext(al_dev, nullptr);
    alcMakeContextCurrent(al_ctx);
    tk_hook_event_tick([](double) { __process_tracks(); });
    tk_hook_event_dispose([]() {
        alcDestroyContext(al_ctx);
        alcCloseDevice(al_dev);
    });
}

void tk_end_make_device()
{
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    // nothing
}

shared<track> load_track(const hio_path &path)
{
    auto file = hio_read_bytes(path);

    size_t index = 0;

    if (file.size() < 12)
        prtlog_throw(FX_FATAL, "too small file: {}", path.absolute);

    if (file[index++] != 'R' || file[index++] != 'I' || file[index++] != 'F' || file[index++] != 'F')
        prtlog_throw(FX_FATAL, "not a wave file: {}", path.absolute);

    index += 4;

    if (file[index++] != 'W' || file[index++] != 'A' || file[index++] != 'V' || file[index++] != 'E')
        prtlog_throw(FX_FATAL, "not a wave file: {}", path.absolute);

    int samp_rate = 0;
    int16_t bps = 0;
    int16_t n_ch = 0;
    int byte_size = 0;
    ALenum format = 0;

    ALuint buffer;
    alGenBuffers(1, &buffer);

    while (index + 8 <= file.size())
    {
        std::string identifier(4, '\0');
        identifier[0] = file[index++];
        identifier[1] = file[index++];
        identifier[2] = file[index++];
        identifier[3] = file[index++];

        uint32_t chunk_size = *reinterpret_cast<const uint32_t *>(&file[index]);
        index += 4;

        if (index + chunk_size > file.size())
            prtlog_throw(FX_FATAL, "invalid chunk size: {}", path.absolute);

        if (identifier == "fmt ")
        {
            if (chunk_size != 16)
                prtlog_throw(FX_FATAL, "unknown format: {}", path.absolute);

            int16_t audio_format = *reinterpret_cast<const int16_t *>(&file[index]);
            index += 2;
            if (audio_format != 1)
                prtlog_throw(FX_FATAL, "unknown format: {}", path.absolute);

            n_ch = *reinterpret_cast<const int16_t *>(&file[index]);
            index += 2;
            samp_rate = *reinterpret_cast<const int32_t *>(&file[index]);
            index += 4;
            index += 4;
            index += 2;
            bps = *reinterpret_cast<const int16_t *>(&file[index]);
            index += 2;

            if (n_ch == 1)
            {
                if (bps == 8)
                    format = AL_FORMAT_MONO8;
                else if (bps == 16)
                    format = AL_FORMAT_MONO16;
                else
                    prtlog_throw(FX_FATAL, "can't play mono " + std::to_string(bps) + " sound.");
            }
            else if (n_ch == 2)
            {
                if (bps == 8)
                    format = AL_FORMAT_STEREO8;
                else if (bps == 16)
                    format = AL_FORMAT_STEREO16;
                else
                    prtlog_throw(FX_FATAL, "can't play stereo " + std::to_string(bps) + " sound.");
            }
            else
                prtlog_throw(FX_FATAL, "can't play audio with " + std::to_string(n_ch) + " channels");
        }
        else if (identifier == "data")
        {
            byte_size = chunk_size;
            const byte *data = &file[index];
            alBufferData(buffer, format, data, chunk_size, samp_rate);
            index += chunk_size;
        }
        else if (identifier == "JUNK" || identifier == "iXML")
            index += chunk_size;
        else
            index += chunk_size;
    }

    auto ptr = std::make_shared<track>();
    ptr->__track_id = buffer;
    ptr->sec_len = (double)byte_size / (samp_rate * bps / 8.0) / n_ch;

    return ptr;
}

shared<clip> make_clip(shared<track> track)
{
    unsigned int id;
    alGenSources(1, &id);
    alSourcei(id, AL_BUFFER, track->__track_id);
    alSourcef(id, AL_ROLLOFF_FACTOR, rolloff);
    alSourcef(id, AL_REFERENCE_DISTANCE, ref_dist);
    alSourcef(id, AL_MAX_DISTANCE, max_dist);

    auto ptr = std::make_shared<clip>();
    ptr->relying_track = track;
    ptr->__clip_id = id;
    clip_r.push_back(ptr);
    return ptr;
}

void tk_set_device_option(device_option opt, double v)
{
    if (opt == FX_AUDIO_ROLLOFF)
        rolloff = v;
    else if (opt == FX_AUDIO_REFERENCE_DIST)
        ref_dist = v;
    else if (opt == FX_AUDIO_MAX_DIST)
        max_dist = v;
}

void tk_set_device_option(device_option opt, const vec2 &v)
{
    if (opt == FX_AUDIO_LISTENER)
        alListener3f(AL_POSITION, v.x, v.y, 0);
}

void tk_set_device_option(device_option opt, const vec3 &v)
{
    if (opt == FX_AUDIO_LISTENER)
        alListener3f(AL_POSITION, v.x, v.y, v.z);
}

} // namespace flux::aud
