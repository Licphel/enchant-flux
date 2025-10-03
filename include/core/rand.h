#pragma once
#include <core/def.h>
#include <core/bin.h>

namespace flux
{
    
struct random
{
    struct _impl;
    unique<_impl> __p;

    random();
    ~random();

    // here, the implementation may be complicated.
    // but this single seed provides randomness.
    void set_seed(long seed);

    // equivalent to next() < 0.5.
    bool next_bool();
    double next();
    double next(double min, double max);
    // generates a guassian-distributed double in [min, max].
    double next_guassian(double min, double max);
    int next_int(int bound);
    int next_int(int min, int max);

    void write(byte_buf& buf);
    void read(byte_buf& buf);

    shared<random> copy();
    shared<random> copy(int seed_addon);
};

// get a global random generator, when we don't care the seed.
shared<random> get_grand();
// make a new random generator, with a random seed or a specific seed.
shared<random> make_random();
shared<random> make_random(long seed);

struct noise
{
    long seed;
    virtual ~noise() = default;
    virtual double generate(double x, double y, double z) = 0;
};

shared<noise> make_perlin(long seed);
shared<noise> make_voronoi(long seed);

} // namespace flux
