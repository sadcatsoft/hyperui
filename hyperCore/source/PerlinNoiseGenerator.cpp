#include "stdafx.h"

namespace HyperCore
{
#define B SAMPLE_SIZE
#define BM (SAMPLE_SIZE-1)

#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

#define s_curve(t) ( t * t * (3.0f - 2.0f * t) )
#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
    t = vec[i] + N;\
    b0 = ((int)t) & BM;\
    b1 = (b0+1) & BM;\
    r0 = t - (int)t;\
    r1 = r0 - 1.0f;

/********************************************************************************************/
PerlinNoiseGenerator::PerlinNoiseGenerator()
{
	myStart = true;
}
/********************************************************************************************/
PerlinNoiseGenerator::PerlinNoiseGenerator(int octaves,FLOAT_TYPE freq,FLOAT_TYPE amp,int seed)
{
	myOctaves = octaves;
	myFrequency = freq;
	myAmplitude = amp;
	mySeed = seed;

	myStart = true;
}
/********************************************************************************************/
FLOAT_TYPE PerlinNoiseGenerator::noise1(FLOAT_TYPE arg)
{
    int bx0, bx1;
    FLOAT_TYPE rx0, rx1, sx, t, u, v, vec[1];

    vec[0] = arg;

    if (myStart)
    {
		RandomNumberGenerator::getInstance()->setFixedSeed(mySeed);
		myStart = false;
		init();
    }

    setup(0, bx0,bx1, rx0,rx1);

    sx = s_curve(rx0);

    u = rx0 * g1[ p[ bx0 ] ];
    v = rx1 * g1[ p[ bx1 ] ];

    return lerp(sx, u, v);
}
/********************************************************************************************/
FLOAT_TYPE PerlinNoiseGenerator::noise2(FLOAT_TYPE vec[2])
{
    int bx0, bx1, by0, by1, b00, b10, b01, b11;
    FLOAT_TYPE rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
    int i, j;

    if (myStart)
    {
		RandomNumberGenerator::getInstance()->setFixedSeed(mySeed);
		myStart = false;
		init();
    }

    setup(0,bx0,bx1,rx0,rx1);
    setup(1,by0,by1,ry0,ry1);

    i = p[bx0];
    j = p[bx1];

    b00 = p[i + by0];
    b10 = p[j + by0];
    b01 = p[i + by1];
    b11 = p[j + by1];

    sx = s_curve(rx0);
    sy = s_curve(ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

    q = g2[b00];
    u = at2(rx0,ry0);
    q = g2[b10];
    v = at2(rx1,ry0);
    a = lerp(sx, u, v);

    q = g2[b01];
    u = at2(rx0,ry1);
    q = g2[b11];
    v = at2(rx1,ry1);
    b = lerp(sx, u, v);

    return lerp(sy, a, b);
}
/********************************************************************************************/
FLOAT_TYPE PerlinNoiseGenerator::noise3(FLOAT_TYPE vec[3])
{
    int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
    FLOAT_TYPE rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
    int i, j;

    if (myStart)
    {
		RandomNumberGenerator::getInstance()->setFixedSeed(mySeed);
		myStart = false;
		init();
    }

    setup(0, bx0,bx1, rx0,rx1);
    setup(1, by0,by1, ry0,ry1);
    setup(2, bz0,bz1, rz0,rz1);

    i = p[ bx0 ];
    j = p[ bx1 ];

    b00 = p[ i + by0 ];
    b10 = p[ j + by0 ];
    b01 = p[ i + by1 ];
    b11 = p[ j + by1 ];

    t  = s_curve(rx0);
    sy = s_curve(ry0);
    sz = s_curve(rz0);

#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

    q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
    q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
    a = lerp(t, u, v);

    q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
    q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
    b = lerp(t, u, v);

    c = lerp(sy, a, b);

    q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
    q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
    a = lerp(t, u, v);

    q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
    q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
    b = lerp(t, u, v);

    d = lerp(sy, a, b);

    return lerp(sz, c, d);
}
/********************************************************************************************/
void PerlinNoiseGenerator::normalize2(FLOAT_TYPE v[2])
{
    FLOAT_TYPE s;

    s = (FLOAT_TYPE)sqrt(v[0] * v[0] + v[1] * v[1]);
    s = 1.0f/s;
    v[0] = v[0] * s;
    v[1] = v[1] * s;
}
/********************************************************************************************/
void PerlinNoiseGenerator::normalize3(FLOAT_TYPE v[3])
{
    FLOAT_TYPE s;

    s = (FLOAT_TYPE)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    s = 1.0f/s;

    v[0] = v[0] * s;
    v[1] = v[1] * s;
    v[2] = v[2] * s;
}
/********************************************************************************************/
void PerlinNoiseGenerator::init(void)
{
    int i, j, k;

    for (i = 0 ; i < B ; i++)
    {
		p[i] = i;
		g1[i] = (FLOAT_TYPE)((RAND_CALL() % (B + B)) - B) / B;
		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (FLOAT_TYPE)((RAND_CALL() % (B + B)) - B) / B;
		normalize2(g2[i]);
		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (FLOAT_TYPE)((RAND_CALL() % (B + B)) - B) / B;
		normalize3(g3[i]);
    }

    while (--i)
    {
		k = p[i];
		p[i] = p[j = RAND_CALL() % B];
		p[j] = k;
    }

    for (i = 0 ; i < B + 2 ; i++)
    {
		p[B + i] = p[i];
		g1[B + i] = g1[i];
		for (j = 0 ; j < 2 ; j++)
			g2[B + i][j] = g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			g3[B + i][j] = g3[i][j];
    }

}
/********************************************************************************************/
FLOAT_TYPE PerlinNoiseGenerator::generate1DNoise(FLOAT_TYPE vec)
{
    int terms    = myOctaves;
    FLOAT_TYPE freq   = myFrequency;
    FLOAT_TYPE result = 0.0f;
    FLOAT_TYPE amp = myAmplitude;

    vec*=myFrequency;

    for( int i=0; i<terms; i++ )
    {
		result += noise1(vec)*amp;
		vec *= 2.0f;
		amp*=0.5f;
    }

    return result;
}

FLOAT_TYPE PerlinNoiseGenerator::generate2DNoise(FLOAT_TYPE vec[2])
{
    int terms    = myOctaves;
    FLOAT_TYPE freq   = myFrequency;
    FLOAT_TYPE result = 0.0f;
    FLOAT_TYPE amp = myAmplitude;

    vec[0]*=myFrequency;
    vec[1]*=myFrequency;

    for( int i=0; i<terms; i++ )
    {
		result += noise2(vec)*amp;
		vec[0] *= 2.0f;
		vec[1] *= 2.0f;
		amp*=0.5f;
    }


    return result;
}
/********************************************************************************************/
FLOAT_TYPE PerlinNoiseGenerator::generate3DNoise(FLOAT_TYPE vec[3])
{
    int terms    = myOctaves;
    FLOAT_TYPE freq   = myFrequency;
    FLOAT_TYPE result = 0.0f;
    FLOAT_TYPE amp = myAmplitude;

    vec[0]*=myFrequency;
    vec[1]*=myFrequency;
    vec[2]*=myFrequency;

    for( int i=0; i<terms; i++ )
    {
		result += noise3(vec)*amp;
		vec[0] *= 2.0f;
		vec[1] *= 2.0f;
		vec[2] *= 2.0f;
		amp*=0.5f;
    }

    return result;
}
/********************************************************************************************/
};