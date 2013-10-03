#include "PerlinNoise.h"
#include <cmath>

//********************
//** PERLIN NOISE FUNCTIONS
//********************

double PerlinNoise::m_dCosine[50 + 2 * 99999 + 50];
PerlinNoise g_pnSingleton;

PerlinNoise::PerlinNoise() {
  int x;
  for(x = 0; x < 50 + 2 * 99999 + 50; ++x) {
    m_dCosine[x] = 0.0;
  }
  for(x = 0; x < 99999; ++x) {
    m_dCosine[50 + 99999 + x] = (1.0 - cos(double(x) / 99999.0 * 3.141592654)) * 0.5;
    m_dCosine[50 + 99999 - x] = m_dCosine[50 + 99999 + x];
  }
}

inline double PerlinNoise::Noise(int x, int y) {
  int n = x + y * 57;
  // int n = x + y * 15731;
  n = (n<<13) ^ n;
  return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

inline double PerlinNoise::Noise1D(int x) {
  int n = x + 6373 * 57;
  // int n = x + y * 15731;
  n = (n<<13) ^ n;
  return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

inline double PerlinNoise::SmoothNoise(int x, int y) {
  double dCorners = (Noise(x-1, y-1)+Noise(x+1, y-1)+Noise(x-1, y+1)+Noise(x+1, y+1)) / 16.0;
  double dSides   = (Noise(x-1, y)  +Noise(x+1, y)  +Noise(x, y-1)  +Noise(x, y+1) ) / 8.0;
  double dCenter  =  Noise(x, y) / 4.0;
  return dCorners + dSides + dCenter;
}

inline double PerlinNoise::SmoothNoise1D(int x) {
  return (Noise1D(x-1) + Noise1D(x) * 2.0 + Noise1D(x+1)) / 4.0;
}

inline double PerlinNoise::Cosine_Interpolate(double a, double b, double x) {
	// double ft = x * 3.141592654;
	// double f = (1.0 - cos(ft)) * 0.5;
  double f = m_dCosine[50 + 99999 + int(x * 99999.0)];
	return a * (1.0 - f) + b * f;
}

inline double PerlinNoise::InterpolatedNoise(double x, double y) {
  int integer_X = int(x);
  double fractional_X = x - double(integer_X);

  int integer_Y = int(y);
  double fractional_Y = y - double(integer_Y);

  double v1 = SmoothNoise(integer_X,     integer_Y);
  double v2 = SmoothNoise(integer_X + 1, integer_Y);
  double v3 = SmoothNoise(integer_X,     integer_Y + 1);
  double v4 = SmoothNoise(integer_X + 1, integer_Y + 1);

  double i1 = Cosine_Interpolate(v1, v2, fractional_X);
  double i2 = Cosine_Interpolate(v3, v4, fractional_X);

  return Cosine_Interpolate(i1 , i2 , fractional_Y);
}

inline double PerlinNoise::InterpolatedNoise1D(double x) {
  int integer_X = int(x);
  double fractional_X = x - double(integer_X);

  double v1 = SmoothNoise1D(integer_X);
  double v2 = SmoothNoise1D(integer_X + 1);

  return Cosine_Interpolate(v1, v2, fractional_X);
}

double PerlinNoise::PerlinNoise_2D(double x, double y, int nOctaves, double dPersistence) {
  double total = 0;

  double frequency = 1.0;
  double amplitude = 1.0;

  for(int i = 0; i < nOctaves; ++i) {
    total = total + InterpolatedNoise(x * frequency, y * frequency) * amplitude;
    frequency *= 2.0;
    amplitude *= dPersistence;
  }
  return total;
}

double PerlinNoise::PerlinNoise_1D(double x, int nOctaves, double dPersistence) {
  double total = 0;

  double frequency = 1.0;
  double amplitude = 1.0;

  for(int i = 0; i < nOctaves; ++i) {
    total = total + InterpolatedNoise1D(x * frequency) * amplitude;
    frequency *= 2.0;
    amplitude *= dPersistence;
  }
  return total;
}

//*********************
//*********************
//*********************
