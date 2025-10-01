#pragma once
#include <core/def.h>
#include <core/math.h>
#include <core/hio.h>

namespace flux::aud
{

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

void make_device();
void end_make_device();
shared<track> load_track(const hio_path &path);
shared<clip> make_clip(shared<track> track);

} // namespace flux