#pragma once

#include <xmmintrin.h>
#include <vector>
#include <list>
#include <unordered_map>

#include "jpb.h"
#include "Vector.h"

enum NOISE_TYPE{
  NT_PERLIN,
  NT_VALUE,
  NT_WORLEY
};

struct NoiseParams{
  real32 frequency;
  int32 octaves;
  real32 lacunarity;
  real32 persistence;

  // Extra Params(May not apply to all noise types)
  int32 extraParam;
  int32 extraParam2;
};

struct DllExport GenData{
  NOISE_TYPE noiseType;
  NoiseParams noiseParams;
  real32 scale;

  bool operator==(const GenData& genData) const;
  bool operator!=(const GenData& genData) const { return !(*this == genData); }
};

// Used for determining location
enum DISTANCE_TYPE{
  DT_EUCLDIAN_SQUARED = 0,
  DT_MANHATTAN,
  DT_EUCLIDIAN,
  DT_MANHATTAN_SQUARED,
  DT_CHEBYSHEV_MAX,
  DT_CHEBYSHEV_MIN,
};

enum WORLEY_TYPE{
  WT_F2SUBF1,
  WT_F1,
  WT_F2,
  WT_F2ADDF1,
  WT_F1MULF2,
};

typedef std::unordered_map<int, GenData> GenDataMap;

class DllExport Noise {
 public:
  static real32 random() { return (rand() % 255) * (1.0f / 255); }
  static real32 value(Vec2f point, real32 frequency);
  static real32 perlin(real32 value, real32 frequency);

  static real32 worley(Vec2f point, real32 frequency, WORLEY_TYPE worleyType, DISTANCE_TYPE distanceType);

  static real32 perlin(Vec2f point, real32 frequency);
  static Vec4f perlinFast(const Vec2f points[], real32 frequency);

  static real32 sumPerlin(const Vec2f& point, const NoiseParams& noiseParams);

  // extraParam1:
  // 0 - normal
  // 1 - rigged Noise
  static Vec4f sumPerlinFast(const Vec2f point[], const NoiseParams& noiseParams);

  static real32 sumValue(const Vec2f& point, const NoiseParams& noiseParams);

  static real32 sumWorley(const Vec2f& point, const NoiseParams& noiseParams);

  static std::vector<Vec4f> getMap(Vec2f offset, int32 sideLength, std::list<GenData>& genDatas,
				   const std::string& expression);

  // if it's with bounds the resulting vertices are bigger then reguested sideLength by 2 (n+2) * (n+2)
  // base Width modifier defines the range of interpolation by default we're taking values form square 1x1
  // if modifier will be 2 the square will 2x2 and so on.
  static std::vector<Vec4f> getMapFast(Vec2f offset, int32 sideLength, const std::unordered_map<int32,GenData>& genDataMap,
				       const std::string& expression, real32 baseWidthModifier = 1.0f, bool withBounds=false);

 private:
  static int32 hash[];
  static int32 hashMask;

  static real32 gradients1D[];
  static int32 gradients1DMask;

  static Vec2f gradients2D[];
  static int32 gradients2DMask;

  static real32 sqr2;

  static std::list<Vec2i> chunksToCheck;


  static real32 smooth(real32 t);
  static inline __m128 smoothFast(const __m128& t);

  // For Worley Noise
  static real32 getClosest(const Vec2f& point, std::vector<Vec2f>& marks);
  static void addDistances(const Vec2f& point, std::vector<Vec2f>& marks, std::list<real32>& distanceList, DISTANCE_TYPE distanceType);

  // Returns points from given chunk Position
  static std::vector<Vec2f> worleyGetPoints(Vec2i cubePosition);
  static float getRandNormalized();
  static real32 poisson(real32 lambda, real32 m);
  static int32 factorial(int32 value);
};
