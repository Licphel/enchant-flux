#pragma once
#include <core/def.h>
#include <core/math.h>
#include <core/hio.h>

namespace flux::aud
{

enum device_option
{
    FX_AUDIO_LISTENER,
    FX_AUDIO_ROLLOFF,
    FX_AUDIO_REFERENCE_DIST,
    FX_AUDIO_MAX_DIST
};

struct track
{
    /* unstable */ unsigned int __track_id;
    double sec_len;

    ~track();
};

enum clip_op
{
    FX_CLIP_LOCATION,
    FX_CLIP_GAIN,
    FX_CLIP_PITCH,

    FX_CLIP_LOOP,
    FX_CLIP_PLAY,
    FX_CLIP_PAUSE,
    FX_CLIP_STOP
};

enum clip_status
{
    FX_CLIP_PLAYING,
    FX_CLIP_IDLE,
    FX_CLIP_END,
    FX_CLIP_PAUSED
};

struct clip
{
    shared<track> relying_track;
    /* unstable */ unsigned int __clip_id;

    ~clip();
    clip_status status();
    void set(clip_op param, double v);
    void set(clip_op param, const vec2 &v);
    void set(clip_op param, const vec3 &v);
    void operate(clip_op param);
};

void tk_make_device();
void tk_end_make_device();
shared<track> load_track(const hio_path &path);
shared<clip> make_clip(shared<track> track);

void tk_set_device_option(device_option opt, double v);
void tk_set_device_option(device_option opt, const vec2 &v);
void tk_set_device_option(device_option opt, const vec3 &v);

} // namespace flux