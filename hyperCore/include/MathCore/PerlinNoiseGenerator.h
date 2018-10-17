#pragma once

#define SAMPLE_SIZE 1024
/********************************************************************************************/
class PerlinNoiseGenerator
{
public:

  HYPERCORE_API PerlinNoiseGenerator(int octaves, FLOAT_TYPE freq, FLOAT_TYPE amp, int seed);
  HYPERCORE_API PerlinNoiseGenerator();

  inline void setSeed(int seed)
  {
      mySeed = seed;
      myStart = true;
  }

  inline void setOctaves(int octaves)
  {
      myOctaves = octaves;
  }

  inline void setFreq(FLOAT_TYPE freq)
  {
      myFrequency = freq;
  }

  inline void setAmp(FLOAT_TYPE amp)
  {
      myAmplitude = amp;
  }

  inline FLOAT_TYPE getNoise(FLOAT_TYPE x)
  {
      return generate1DNoise(x);
  }

  inline FLOAT_TYPE getNoise(FLOAT_TYPE x,FLOAT_TYPE y)
  {
	FLOAT_TYPE vec[2];
	vec[0] = x;
	vec[1] = y;
	return generate2DNoise(vec);
  }

  inline FLOAT_TYPE getNoise(FLOAT_TYPE x, FLOAT_TYPE y, FLOAT_TYPE z)
  {
      FLOAT_TYPE vec[3];
      vec[0] = x;
      vec[1] = y;
      vec[2] = z;
      return generate3DNoise(vec);
  }

  inline FLOAT_TYPE getAmplitude() const { return myAmplitude; }

private:

  void initPerlin(int n,FLOAT_TYPE p);
  HYPERCORE_API FLOAT_TYPE generate1DNoise(FLOAT_TYPE vec);
  HYPERCORE_API FLOAT_TYPE generate2DNoise(FLOAT_TYPE vec[2]);
  HYPERCORE_API FLOAT_TYPE generate3DNoise(FLOAT_TYPE vec[3]);

  FLOAT_TYPE noise1(FLOAT_TYPE arg);
  FLOAT_TYPE noise2(FLOAT_TYPE vec[2]);
  FLOAT_TYPE noise3(FLOAT_TYPE vec[3]);
  void normalize2(FLOAT_TYPE v[2]);
  void normalize3(FLOAT_TYPE v[3]);
  void init();

private:
  int myOctaves;
  FLOAT_TYPE myFrequency;
  FLOAT_TYPE myAmplitude;
  int mySeed;

  int p[SAMPLE_SIZE + SAMPLE_SIZE + 2];
  FLOAT_TYPE g3[SAMPLE_SIZE + SAMPLE_SIZE + 2][3];
  FLOAT_TYPE g2[SAMPLE_SIZE + SAMPLE_SIZE + 2][2];
  FLOAT_TYPE g1[SAMPLE_SIZE + SAMPLE_SIZE + 2];
  bool myStart;

};
/********************************************************************************************/