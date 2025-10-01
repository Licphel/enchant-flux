#include <core/rand.h>
#include <random>

namespace flux
{

struct random::_impl
{
    long a, b;
};

random::random() : __p(std::make_unique<_impl>())
{
}

random::~random() = default;

void random::set_seed(long seed)
{
    __p->a = (seed + 13) / 2;
    __p->b = (seed - 9) ^ 39;
}

bool random::next_bool()
{
    return next() < 0.5;
}

double random::next()
{
    static std::mt19937_64 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

double random::next(double min, double max)
{
    return next() * (max - min) + min;
}

double random::next_guassian(double min, double max)
{
    double x, y, w;
    do
    {
        x = next() * 2 - 1;
        y = next() * 2 - 1;
        w = x * x + y * y;
    } while (w >= 1 || w == 0);

    double c = sqrt(-2 * log(w) / w);
    return (y * c) * (max - min) + min;
}

int random::next_int(int bound)
{
    static std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, bound - 1);
    return dist(rng);
}

int random::next_int(int min, int max)
{
    return next_int(max + 1 - min) + min;
}

void random::write(byte_buf &buf)
{
    buf.write<long>(__p->a);
    buf.write<long>(__p->b);
}

void random::read(byte_buf &buf)
{
    __p->a = buf.read<long>();
    __p->b = buf.read<long>();
}

shared<random> random::copy()
{
    return copy(0);
}

shared<random> random::copy(int seed_addon)
{
    auto ptr = std::make_shared<random>();
    ptr->__p->a = __p->a + seed_addon;
    ptr->__p->b = __p->b - seed_addon;
    return ptr;
}

shared<random> __grand = make_random();

shared<random> get_grand()
{
    return __grand;
}

shared<random> make_random()
{
    static std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, INT_MAX);
    return make_random(dist(rng));
}

shared<random> make_random(long seed)
{
    auto ptr = std::make_shared<random>();
    ptr->set_seed(seed);
    return ptr;
}

struct __noise_perlin : noise
{
    int p[512];

    void init()
    {
        auto rd = make_random(seed);

        int perm[256];
        for (int i = 0; i < 256; i++)
        {
            perm[i] = i;
        }

        for (int i = 255; i > 0; i--)
        {
            int j = rd->next_int(i + 1);
            int temp = perm[i];
            perm[i] = perm[j];
            perm[j] = temp;
        }

        for (int i = 0; i < 256; i++)
        {
            p[i] = p[i + 256] = perm[i];
        }
    }

    inline int fast_floor(double x) const
    {
        int xi = (int)x;
        return x < xi ? xi - 1 : xi;
    }

    inline double fade(double t) const
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    inline double lerp(double t, double a, double b) const
    {
        return a + t * (b - a);
    }

    inline double grad(int hash, double x, double y, double z) const
    {
        int h = hash & 15;

        switch (h & 7)
        {
        case 0:
            return x + y;
        case 1:
            return -x + y;
        case 2:
            return x - y;
        case 3:
            return -x - y;
        case 4:
            return x + z;
        case 5:
            return -x + z;
        case 6:
            return x - z;
        case 7:
            return -x - z;
        default:
            return 0;
        }
    }

    double generate(double x, double y, double z) override
    {
        int X = fast_floor(x) & 255;
        int Y = fast_floor(y) & 255;
        int Z = fast_floor(z) & 255;

        x -= fast_floor(x);
        y -= fast_floor(y);
        z -= fast_floor(z);

        double u = fade(x);
        double v = fade(y);
        double w = fade(z);

        int A = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        double result = lerp(w,
                             lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
                                  lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))),
                             lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)),
                                  lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));

        return (result + 1.0) * 0.5;
    }
};

struct __noise_voronoi : noise
{
    inline int floor(double v)
    {
        int i = (int)v;
        return v >= i ? i : i - 1;
    }

    inline double seedl(int x, int y, int z, long seed)
    {
        long v1 = (x + 2687 * y + 433 * z + 941 * seed) & INT_MAX;
        long v2 = (v1 * (v1 * v1 * 113 + 653) + 2819) & INT_MAX;
        return 1 - (double)v2 / INT_MAX;
    }

    double generate(double x, double y, double z) override
    {
        int x0 = floor(x);
        int y0 = floor(y);
        int z0 = floor(z);

        double xc = 0;
        double yc = 0;
        double zc = 0;
        double md = INT_MAX;

        for (int k = z0 - 2; k <= z0 + 2; k++)
            for (int j = y0 - 2; j <= y0 + 2; j++)
                for (int i = x0 - 2; i <= x0 + 2; i++)
                {
                    double xp = i + seedl(i, j, k, seed);
                    double yp = j + seedl(i, j, k, seed + 1);
                    double zp = k + seedl(i, j, k, seed + 2);
                    double xd = xp - x;
                    double yd = yp - y;
                    double zd = zp - z;
                    double d = xd * xd + yd * yd + zd * zd;

                    if (d < md)
                    {
                        md = d;
                        xc = xp;
                        yc = yp;
                        zc = zp;
                    }
                }

        return seedl(floor(xc), floor(yc), floor(zc), 0);
    }
};

shared<noise> make_perlin(long seed)
{
    auto ptr = std::make_shared<__noise_perlin>();
    ptr->seed = seed;
    ptr->init();
    return ptr;
}

shared<noise> make_voronoi(long seed)
{
    auto ptr = std::make_shared<__noise_voronoi>();
    ptr->seed = seed;
    return ptr;
}

} // namespace flux