#include <xmmintrin.h>
#include <string>
#include <map>
#include "Noise.h"
#include "SimpleParser.h"
#include "Profiler.h"
#include "Misc.h"

bool GenData::operator==(const GenData& genData) const
{
  if(noiseParams.frequency == genData.noiseParams.frequency &&
     noiseParams.octaves == genData.noiseParams.octaves &&
     noiseParams.lacunarity == genData.noiseParams.lacunarity &&
     noiseParams.persistence == genData.noiseParams.persistence &&
     noiseParams.extraParam == genData.noiseParams.extraParam &&
     noiseParams.extraParam2 == genData.noiseParams.extraParam2 &&
     noiseType == genData.noiseType &&
     scale == genData.scale)
  {
    return true;
  }

  return false;
}

real32
Noise::value(Vec2f point, real32 frequency)
{
  point *= frequency;

  int32 ix0 = (int32)floor(point.x);
  int32 iy0 = (int32)floor(point.y);

  real32 tx = point.x - ix0;
  real32 ty = point.y - iy0;

  ix0 &= hashMask;
  iy0 &= hashMask;

  int32 ix1 = ix0 + 1;
  int32 iy1 = iy0 + 1;

  iy1 &= hashMask;

  int32 h0 = hash[ix0];
  int32 h1 = hash[ix1];

  int32 h00 = hash[h0 + iy0];
  int32 h10 = hash[h1 + iy0];
  int32 h01 = hash[h0 + iy1];
  int32 h11 = hash[h1 + iy1];

  tx = smooth(tx);
  ty = smooth(ty);

  real32 upX = lerp((float)h00, (float)h10, tx);
  real32 downX = lerp((float)h01, (float)h11, tx);

  real32 middle = lerp(upX, downX, ty);

  return middle / 255.0f;
}

real32
Noise::perlin(real32 value, real32 frequency)
{
  value *= frequency;

  int32 ix0 = (int)floor(value);

  real32 t0 = value - ix0;
  real32 t1 = t0 - 1.0f;

  ix0 &= hashMask;

  int32 ix1 = ix0 + 1;
  ix1 &= hashMask;

  real32 g0 = gradients1D[hash[ix0]&gradients1DMask];
  real32 g1 = gradients1D[hash[ix1]&gradients1DMask];

  real32 v0 = g0 * t0;
  real32 v1 = g1 * t1;

  real32 t = smooth(t0);

  return lerp(v0, v1, t) * 2;
}

real32
Noise::perlin(Vec2f point, real32 frequency)
{
  point *= frequency;

  int32 ix0 = (int)floor(point.x);
  int32 iy0 = (int)floor(point.y);

  real32 tx0 = point.x - ix0;
  real32 tx1 = tx0 - 1.0f;
  real32 ty0 = point.y - iy0;
  real32 ty1 = ty0 - 1.0f;

  ix0 &= hashMask;
  int32 ix1 = ix0 + 1;

  iy0 &= hashMask;
  int32 iy1 = iy0 + 1;
  iy1 &= hashMask;

  int32 h0 = hash[ix0];
  int32 h1 = hash[ix1];

  Vec2f g00 = gradients2D[hash[h0 + iy0] & gradients2DMask];
  Vec2f g10 = gradients2D[hash[h1 + iy0] & gradients2DMask];
  Vec2f g01 = gradients2D[hash[h0 + iy1] & gradients2DMask];
  Vec2f g11 = gradients2D[hash[h1 + iy1] & gradients2DMask];

  real32 v00 = Vec2f::dotProduct(g00, tx0, ty0);
  real32 v10 = Vec2f::dotProduct(g10, tx1, ty0);
  real32 v01 = Vec2f::dotProduct(g01, tx0, ty1);
  real32 v11 = Vec2f::dotProduct(g11, tx1, ty1);

  real32 tx = smooth(tx0);
  real32 ty = smooth(ty0);

  real32 upX = lerp(v00, v10, tx);
  real32 downX = lerp(v01, v11, tx);

  return lerp(upX, downX, ty) * sqr2;
}

#define m(a, index) ((a).m128_f32[(3 - index)])
#define mi(a, index) ((a).m128i_i32[(3 - index)])

Vec4f
Noise::perlinFast(const Vec2f points[], real32 frequency)
{
  Vec4f resultVec;

  __m128 points_cX = _mm_set_ps(points[0].x,
				points[1].x,
				points[2].x,
				points[3].x);

  __m128 points_cY = _mm_set_ps(points[0].y,
				points[1].y,
				points[2].y,
				points[3].y);

  __m128i tempI;

  static const __m128 zero = _mm_set1_ps(0.0f);
  static const __m128 oneFX4 = _mm_set1_ps(1.0f);
  static const __m128 halfX4 = _mm_set1_ps(0.5f);
  static const __m128i oneIX4 = _mm_set1_epi32(1);
  static const __m128i hashMaskX4 = _mm_set1_epi32(hashMask);
  static const __m128i gradientMaskX4 = _mm_set1_epi32(gradients2DMask);
  static const __m128 sqr2X4 = _mm_set_ps1(sqr2);

  const __m128 freqX4 = _mm_set_ps1(frequency);

  // Multiplying by Frequency
  points_cX = _mm_mul_ps(points_cX, freqX4);
  points_cY = _mm_mul_ps(points_cY, freqX4);

  __m128i ix0;
  __m128i iy0;

  // Flooring
  ix0 = _mm_cvttps_epi32(points_cX);
  iy0 = _mm_cvttps_epi32(points_cY);

  ix0 = _mm_sub_epi32(ix0, _mm_and_si128(oneIX4, _mm_castps_si128(_mm_cmplt_ps(points_cX, zero))));
  iy0 = _mm_sub_epi32(iy0, _mm_and_si128(oneIX4, _mm_castps_si128(_mm_cmplt_ps(points_cY, zero))));

  // Getting remainder of stuff
  __m128 tx0 = _mm_sub_ps(points_cX, _mm_cvtepi32_ps(ix0));
  __m128 ty0 = _mm_sub_ps(points_cY, _mm_cvtepi32_ps(iy0));

  __m128 tx1 = _mm_sub_ps(tx0, oneFX4);
  __m128 ty1 = _mm_sub_ps(ty0, oneFX4);

  // Getting Other Corner Values
  ix0 = _mm_and_si128(ix0, hashMaskX4);
  __m128i ix1 = _mm_add_epi32(ix0, oneIX4);
  ix1 = _mm_and_si128(ix1, hashMaskX4);

  iy0 = _mm_and_si128(iy0, hashMaskX4);
  __m128i iy1 = _mm_add_epi32(iy0, oneIX4);
  iy1 = _mm_and_si128(iy1, hashMaskX4);

  int32 h0[4];
  int32 h1[4];

  __m128 g00x;
  __m128 g00y;

  __m128 g10x;
  __m128 g10y;

  __m128 g01x;
  __m128 g01y;

  __m128 g11x;
  __m128 g11y;

  for(int i = 0; i < 4; i++)
  {

    h0[i] = hash[mi(ix0, i)];
    h1[i] = hash[mi(ix1, i)];

    m(g00x, i) = gradients2D[hash[(h0[i] + mi(iy0, i))&hashMask] & gradients2DMask].x;
    m(g00y, i) = gradients2D[hash[(h0[i] + mi(iy0, i))&hashMask] & gradients2DMask].y;

    m(g10x, i) = gradients2D[hash[(h1[i] + mi(iy0, i))&hashMask] & gradients2DMask].x;
    m(g10y, i) = gradients2D[hash[(h1[i] + mi(iy0, i))&hashMask] & gradients2DMask].y;

    m(g01x, i) = gradients2D[hash[(h0[i] + mi(iy1, i))&hashMask] & gradients2DMask].x;
    m(g01y, i) = gradients2D[hash[(h0[i] + mi(iy1, i))&hashMask] & gradients2DMask].y;

    m(g11x, i) = gradients2D[hash[(h1[i] + mi(iy1, i))&hashMask] & gradients2DMask].x;
    m(g11y, i) = gradients2D[hash[(h1[i] + mi(iy1, i))&hashMask] & gradients2DMask].y;
  }

  __m128 v00 = _mm_add_ps(_mm_mul_ps(g00x, tx0), _mm_mul_ps(g00y, ty0));
  __m128 v10 = _mm_add_ps(_mm_mul_ps(g10x, tx1), _mm_mul_ps(g10y, ty0));

  __m128 v01 = _mm_add_ps(_mm_mul_ps(g01x, tx0), _mm_mul_ps(g01y, ty1));
  __m128 v11 = _mm_add_ps(_mm_mul_ps(g11x, tx1), _mm_mul_ps(g11y, ty1));

  __m128 tx = smoothFast(tx0);
  __m128 ty = smoothFast(ty0);

  __m128 upX = _mm_add_ps(v00,_mm_mul_ps(_mm_sub_ps(v10, v00),tx));
  __m128 downX = _mm_add_ps(v01,_mm_mul_ps(_mm_sub_ps(v11, v01),tx));

  __m128 result = _mm_mul_ps(_mm_add_ps(upX,_mm_mul_ps(_mm_sub_ps(downX, upX),ty)), sqr2X4);

  for(int i = 0; i < 4; i++)
  {
    resultVec[i] = m(result, i);
  }

  return resultVec;
}

real32
Noise::sumPerlin(const Vec2f& point, const NoiseParams& noiseParams)
{

  real32 amplitude = 1.0f;
  real32 range = 0;

  real32 frequency = noiseParams.frequency;

  real32 sum = 0;
  for(int32 i = 0; i < noiseParams.octaves; i++)
  {
    range += amplitude;

    // Profiler::get()->start("PerOctave");
    sum += perlin(point, frequency) * amplitude;
    // Profiler::get()->end("PerOctave");

    frequency *= noiseParams.lacunarity;
    amplitude *= noiseParams.persistence;
  }

  return sum/range;
}

Vec4f
Noise::sumPerlinFast(const Vec2f points[], const NoiseParams& noiseParams)
{
  Vec4f result;
  Vec4f sum;

  real32 amplitude = 1.0f;
  real32 range = 0;

  real32 frequency = noiseParams.frequency;

  for(int32 octave = 0; octave < noiseParams.octaves; octave++)
  {
    range += amplitude;

    // Profiler::get()->start("PerOctave");
    Vec4f octaveResult =  perlinFast(points, frequency);

    // NOTE have to remember that I don't have to normalize the value in function up level
    // extraParam == 1 - rigded noise
    if(noiseParams.extraParam == 0)
      sum += octaveResult * amplitude;
    else
    {
      octaveResult.x = 1.0f - abs(octaveResult.x);
      octaveResult.y = 1.0f - abs(octaveResult.y);
      octaveResult.z = 1.0f - abs(octaveResult.z);
      octaveResult.w = 1.0f - abs(octaveResult.w);

      sum += octaveResult * amplitude;
    }
    // Profiler::get()->end("PerOctave");

    frequency *= noiseParams.lacunarity;
    amplitude *= noiseParams.persistence;
  }
  result = sum/range;

  return result;
}

real32
Noise::sumValue(const Vec2f& point, const NoiseParams& noiseParams) {
  real32 sum = 0;

  real32 amplitude = 1.0f;
  real32 range = 0;
  real32 frequency = noiseParams.frequency;

  for(int32 i = 0; i < noiseParams.octaves; i++)
  {
    range += amplitude;

    sum += value(point, frequency) * amplitude;

    frequency *= noiseParams.lacunarity;
    amplitude *= noiseParams.persistence;
  }

  return sum / range;
}

real32
Noise::worley(Vec2f point, real32 frequency, WORLEY_TYPE worleyType, DISTANCE_TYPE distanceType)
{
  point *= frequency;

  Vec2i currentChunkPosition = Vec2i(floor(point.x), floor(point.y));

  std::list<real32> distanceList;
  for(auto it = chunksToCheck.begin(); it != chunksToCheck.end(); it++)
  {
    Vec2i& chunkOffset = *it;
    std::vector<Vec2f> cubeMarksOffset = worleyGetPoints(currentChunkPosition + chunkOffset);
    addDistances(point, cubeMarksOffset, distanceList, distanceType);
  }

  distanceList.sort();

  real32 f1 = 0;
  real32 f2 = 1.0f;

  if(distanceList.size() > 1)
  {
    f1 = *distanceList.begin();
    f2 = *(++distanceList.begin());
  }
  else
  {
    std::cout << "we";
  }

  real32 worleyResult;

  switch (worleyType)
  {
  case WT_F2SUBF1: worleyResult = f2 - f1; break;
  case WT_F1: worleyResult = f1; break;
  case WT_F2: worleyResult = f2; break;
  case WT_F2ADDF1: worleyResult = f2 + f1; break;
  case WT_F1MULF2: worleyResult = f1 * f2; break;
  default: std::cout << "No such worley type \n";
  }

  return worleyResult;
}

std::vector<Vec2f>
Noise::worleyGetPoints(Vec2i cubePosition)
{
  static real32 lambda = 2.0f;
  static int32 numbOfFeaturePoints = 8;

  static real32 probabilities[8] = {poisson(lambda, 0), poisson(lambda, 1), poisson(lambda, 2), poisson(lambda, 3),
				    poisson(lambda, 4), poisson(lambda, 5), poisson(lambda, 6), poisson(lambda, 7)};

  // for(int i = 0; i < numbOfFeaturePoints; i++)
  // {
  //   std::cout << i << " " << probabilities[i] << std::endl;
  // }

  std::vector<Vec2f> result;
  int32 cubeHash = hash[(hash[cubePosition.x & hashMask] + hash[cubePosition.y & hashMask]) & hashMask];
  real32 normalizedHash = (real32)cubeHash / 255.0f;

  real32 currentSum = 0;
  int32 numbOfPoints = 0;
  for(int i = 0; i < numbOfFeaturePoints; i++)
  {
    currentSum += probabilities[i];
    if(normalizedHash < currentSum)
    {
      numbOfPoints = i;
      break;
    }
  }

  // std::cout << numbOfPoints << std::endl;

  result.resize(numbOfPoints);
  srand(cubeHash);

  for(int i = 0; i < numbOfPoints ; i++ )
  {
    result[i] = Vec2f(cubePosition.x + getRandNormalized(), cubePosition.y + getRandNormalized());
  }

  return result;
}

float
Noise::getRandNormalized()
{
  return ((double)rand() / (double)RAND_MAX);
}

real32
Noise::poisson(real32 lambda, real32 m)
{
  real32 result = 1.0f / ((pow(lambda, -m) * exp(lambda)) * factorial(m));
  return result;
}

int32
Noise::factorial(int32 value)
{
  int32 result = 1;
  for(int i = 2; i <= value; i++)
  {
    result *= i;
  }
  return result;
}

real32
Noise::sumWorley(const Vec2f& point, const NoiseParams& noiseParams)
{
  real32 amplitude = 1.0f;
  real32 range = 0;

  real32 frequency = noiseParams.frequency;

  real32 sum = 0;
  for(int32 i = 0; i < noiseParams.octaves; i++)
  {
    range += amplitude;
    sum += worley(point, frequency, (WORLEY_TYPE)noiseParams.extraParam, (DISTANCE_TYPE)noiseParams.extraParam2) * amplitude;

    frequency *= noiseParams.lacunarity;
    amplitude *= noiseParams.persistence;
  }

  return sum / range;
}

real32
Noise::getClosest(const Vec2f& point, std::vector<Vec2f>& marks)
{
  real32 distanceToClosest = -1;
  int32 closestIndex = -1;

  Vec2i currentCubePosition = Vec2i(floor(point.x), floor(point.y));
  // Because Marks are normalized
  // point -= Vec2f(currentCubePosition.x, currentCubePosition.y);

  for(auto it = marks.begin(); it != marks.end(); it++)
  {
    const Vec2f& mark = *it;

    real32 currentDistance = (point - mark).getLength();

    if(it == marks.begin())
    {
      distanceToClosest = currentDistance;
      closestIndex = 0;
    }
    else if(distanceToClosest > currentDistance)
    {
      distanceToClosest = currentDistance;
      closestIndex = std::distance(marks.begin(), it);
    }
  }

  if(closestIndex >= 0) marks.erase(marks.begin() + closestIndex);
  else distanceToClosest = 0;

  return distanceToClosest;
}

void
Noise::addDistances(const Vec2f& point, std::vector<Vec2f>& marks, std::list<real32>& distanceList, DISTANCE_TYPE distanceType)
{
  Vec2i currentCubePosition = Vec2i(floor(point.x), floor(point.y));

  for(auto it = marks.begin(); it != marks.end(); it++)
  {
    const Vec2f& mark = *it;

    Vec2f deltaVector = (point - mark);

    real32 distance;

    switch(distanceType)
    {
    case DT_EUCLDIAN_SQUARED : distance = (deltaVector.x * deltaVector.x) + (deltaVector.y * deltaVector.y); break;
    case DT_MANHATTAN : distance = abs(deltaVector.x) + abs(deltaVector.y); break;
    case DT_EUCLIDIAN : distance = deltaVector.getLength(); break;
    case DT_MANHATTAN_SQUARED : distance = abs(deltaVector.x * deltaVector.x) + abs(deltaVector.y * deltaVector.y); break;
    case DT_CHEBYSHEV_MAX : distance = max(abs(deltaVector.x), abs(deltaVector.y)); break;
    case DT_CHEBYSHEV_MIN : distance = min(abs(deltaVector.x), abs(deltaVector.y)); break;
    }

    distanceList.push_back(distance);
  }
}

std::vector<Vec4f>
Noise::getMap(Vec2f offset, int32 sideLength, std::list<GenData>& genDatas,
	      const std::string& expression)
{
  int32 numbOfVertices = sideLength * sideLength;
  std::vector<Vec4f> vertices;
  vertices.resize(numbOfVertices);

  real32 stepSize = 1.0f / (sideLength - 1);
  real32 greyValue;

  Vec2f point00 = Vec2f(-0.5f, 0.5f);
  Vec2f point10 = Vec2f(0.5f, 0.5f);
  Vec2f point01 = Vec2f(-0.5f, -0.5f);
  Vec2f point11 = Vec2f(0.5f, -0.5f);

  std::vector<float> values;
  values.resize(genDatas.size());

  // Parser Stuff ----------------
  SimpleParser simpleParser;
  VariableMap variableMap;
  simpleParser.setVariableMap(&variableMap);
  EntryList reversePolish = simpleParser.getReversePolish(expression);
  // -----------------------------

  std::vector<VariableMap> variableMapBuffer;

  for(int32 y = 0; y < sideLength; y++)
  {

    Vec2f point0 = Vec2f::lerp(point00, point01, ((float)y ) * stepSize);
    Vec2f point1 = Vec2f::lerp(point10, point11, ((float)y ) * stepSize);

    for(int x = 0; x < sideLength; x++)
    {
      Vec2f point = Vec2f::lerp(point0, point1, ((float)x) * stepSize);
      Vec2f realPosition = point + offset;

      for(auto it = genDatas.begin(); it != genDatas.end(); it++)
      {
	int index = std::distance(genDatas.begin(), it);
	values[index] = (Noise::sumPerlin(realPosition, it->noiseParams) * 0.5f + 0.5f) * it->scale;
      }

      real32 finalValue = 0;

      for(int32 j = 0; j < genDatas.size(); j++)
      {
	variableMap["Map" + std::to_string(j+1)] = &values[j];
      }

      variableMap["x"] = &realPosition.x;
      variableMap["y"] = &realPosition.y;

      EntryList reversePolishCopy = reversePolish;
      finalValue = simpleParser.evaluateExpression(reversePolishCopy);

      vertices[(y * sideLength) + x] = Vec4f(point.x * 100, finalValue * 100.0f, -point.y * 100, 1.0f);
    }
  }

  return vertices;
}

std::vector<Vec4f>
Noise::getMapFast(Vec2f offset, int32 sideLength, const std::unordered_map<int32, GenData>& genDataMap,
		  const std::string& expression, real32 baseWidthModifier, bool withBounds)
{
  int32 numbOfVertices = -1;

  // if there should be bounds we add the vertices at the bounds
  if(!withBounds)
    numbOfVertices = sideLength * sideLength;
  else
    numbOfVertices = (sideLength + 2) * (sideLength + 2);

  std::vector<Vec4f> vertices;
  vertices.resize(numbOfVertices);

  real32 stepSize = 1.0f / (sideLength - 1);
  real32 greyValue;

  Vec2f point00 = Vec2f(-0.5f, 0.5f) * baseWidthModifier;
  Vec2f point10 = Vec2f(0.5f, 0.5f) * baseWidthModifier;
  Vec2f point01 = Vec2f(-0.5f, -0.5f) * baseWidthModifier;
  Vec2f point11 = Vec2f(0.5f, -0.5f) * baseWidthModifier;

  // Adding appropriate stepSize to extend the range of interpolation values
  if(withBounds)
  {
    // That works somehow it makes the internal the width that it should be
    // For proper rendering it would have to offset by the sideLength when returning
    // It works my way when sideLength is added at the begginning.

    sideLength += 2;
  }

  // Parser Stuff ----------------
  SimpleParser simpleParser;
  VariableMap variableMap;

  // I set variable Map first becauuuse ???
  simpleParser.setVariableMap(&variableMap);
  EntryList reversePolish = simpleParser.getReversePolish(expression);
  // -----------------------------

  std::map<uint32, Vec4f> values;
  std::vector<VariableMap> variableMapBuffer;

  std::unordered_map<int32, real32> currentPointMapValues;
  Vec2f currentPointRealPosition;

  // Setting pointer to variables in parser - because map insertion is too expensive
  // to be done every sampled point
  // Increased efficiency by the factor of 2
  for(auto it = genDataMap.begin(); it != genDataMap.end(); it++)
  {
    int32 mapIndex = it->first;
    std::string mapName = "Map" + std::to_string(mapIndex);
    variableMap[mapName] = &currentPointMapValues[mapIndex];
  }

  variableMap["x"] = &currentPointRealPosition.x;
  variableMap["y"] = &currentPointRealPosition.y;

  for(int32 valIndex = 0; valIndex < numbOfVertices; valIndex += 4)
  {
    Vec2f points[4];
    Vec2f realPositions[4];

    for(int i = 0; i < 4; i++)
    {
      int32 currValIndex = valIndex + i;

      int32 x = currValIndex % sideLength;
      int32 y = currValIndex / sideLength;

      Vec2f point0 = Vec2f::lerp(point00, point01, ((float)y ) * stepSize);
      Vec2f point1 = Vec2f::lerp(point10, point11, ((float)y ) * stepSize);

      points[i] = Vec2f::lerp(point0, point1, ((float)x) * stepSize);
      realPositions[i] = points[i] + offset;
    }

    // Calculating map Values for 4 pixels
    //------------------------------------
    for(auto it = genDataMap.begin(); it != genDataMap.end(); it++) {
      int index = it->first;
      const GenData& genData = it->second;

      switch(genData.noiseType)
      {
      case NT_PERLIN :
	{
	  values[index] = Noise::sumPerlinFast(realPositions, genData.noiseParams);

	  // It's not rigged noise
	  if(genData.noiseParams.extraParam == 0)
	  {
	    values[index] *= 0.5f;
	    values[index] += 0.5f;
	  }
	} break;
      case NT_VALUE :
	{
	  for(int i = 0; i < 4; i++)
	  {
	    values[index][i] = Noise::sumValue(realPositions[i], genData.noiseParams);
	  }
	} break;
      case NT_WORLEY :
	{
	  for(int i = 0; i < 4; i++)
	  {
	    values[index][i] = Noise::sumWorley(realPositions[i], genData.noiseParams);
	  }
	} break;
      default :
	std::cout << "Error in Noise getMapFast No such noise type. \n";
      }

      values[index] *= genData.scale;
    }

    for(int i = 0; i < 4; i++)
    {
      int32 currValIndex = valIndex + i;

      int32 x = currValIndex % sideLength;
      int32 y = currValIndex / sideLength;

      real32 finalValue = 0;

      for(auto it = values.begin(); it != values.end(); it++)
      {
	int32 index = it->first;
	currentPointMapValues[index] = values[index][i];
      }
      currentPointRealPosition = realPositions[i] * 50;

      EntryList reversePolishCopy = reversePolish;
      finalValue = simpleParser.evaluateExpression(reversePolishCopy);

      // Fucking fix this bullshit
      if(currValIndex < numbOfVertices)
	vertices[currValIndex] = Vec4f(points[i].x * 100, finalValue * 100.0f, -points[i].y * 100, 1.0f);

      else break;
    }
  }

  return vertices;
}

real32
Noise::smooth(real32 t) {
  return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

__m128
Noise::smoothFast(const __m128& t) {

  static const __m128 value6 = _mm_set1_ps(6.0f);
  static const __m128 value15 = _mm_set1_ps(15);
  static const __m128 value10 = _mm_set1_ps(10);

  __m128 result = t;
  __m128 t2 = _mm_mul_ps(t, t);

  result = _mm_mul_ps(result, value6);
  result = _mm_sub_ps(result, value15);

  result = _mm_mul_ps(t, result);
  result = _mm_add_ps(result, value10);

  result = _mm_mul_ps(result, t2);
  result = _mm_mul_ps(result, t);

  return result;
}

int32
Noise::hash[] = {
  151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
  140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
  247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
  57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
  74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
  60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
  65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
  200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
  52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
  207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
  119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
  129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
  218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
  81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
  184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
  222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,

  151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
  140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
  247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
  57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
  74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
  60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
  65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
  200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
  52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
  207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
  119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
  129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
  218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
  81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
  184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
  222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

int32
Noise::hashMask = 255;

real32
Noise::gradients1D[] = { 1.0f, -1.0f };
int32
Noise::gradients1DMask = 1;

Vec2f
Noise::gradients2D[] = {
  Vec2f(1.0f, 0),
  Vec2f(-1.0f, 0),
  Vec2f(0, 1.0f),
  Vec2f(0, -1.0f),
  Vec2f::normalize(Vec2f(-1.0f,1.0f)),
  Vec2f::normalize(Vec2f(1.0f, 1.0f)),
  Vec2f::normalize(Vec2f(1.0f, -1.0f)),
  Vec2f::normalize(Vec2f(-1.0f, -1.0f))
};
int32
Noise::gradients2DMask = 7;

real32
Noise::sqr2 = pow(2.0f, 0.5f);

std::list<Vec2i> Noise::chunksToCheck{
  Vec2i(-1, -1),
    Vec2i(0, -1),
    Vec2i(1, -1),

    Vec2i(-1, 0),
    Vec2i(0, 0),
    Vec2i(1, 0),

    Vec2i(-1, 1),
    Vec2i(0, 1),
    Vec2i(1, 1)
    };
