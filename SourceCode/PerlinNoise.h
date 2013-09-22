//********************
//** PERLIN NOISE FUNCTIONS
//********************

class PerlinNoise {
  static double m_dCosine[50 + 2 * 99999 + 50];
public:
  PerlinNoise(); // Initialize cosine table
  static double Noise(int x, int y);
  static double SmoothNoise(int x, int y);
  static double Cosine_Interpolate(double a, double b, double x);
  static double InterpolatedNoise(double x, double y);
  static double PerlinNoise_2D(double x, double y, int nOctaves, double dPersistence);
  static double Noise1D(int x);
  static double SmoothNoise1D(int x);
  static double InterpolatedNoise1D(double x);
  static double PerlinNoise_1D(double x, int nOctaves, double dPersistence);
};