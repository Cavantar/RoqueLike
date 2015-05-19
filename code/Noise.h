#pragma once
#include "Vector.h"

enum NOISE_TYPE{
	NT_PERLIN,
	NT_VALUE
};

struct NoiseParams{
	float frequency;
	int octaves;
	float lacunarity;
	float persistence;
};

struct GenData{
	NOISE_TYPE noiseType;
	NoiseParams noiseParams;
        float scale;
};

class Noise {
public:
  static float random() { return (rand() % 255) * (1.0f / 255); }
  static float value(Vector2f point, float frequency);
  static float perlin(float value, float frequency);
  static float perlin(Vector2f point, float frequency);
  
  static float sumPerlin(Vector2f point, NoiseParams& noiseParams);
  static float sumValue(Vector2f point, NoiseParams& noiseParams);
  
  //static vector<glm::vec4> getMap(Vector2f offset, int sideLength, list<GenData>& genDatas);
private:
  
  static int hash[];
  static int hashMask;
  
  static float gradients1D[];
  static int gradients1DMask;
  
  static Vector2f gradients2D[];
  static int gradients2DMask;
  
  static float sqr2;
  
  static float smooth(float t) {
    return t * t * t *(t * (t * 6.0f - 15.0f) + 10.0f);
  }
};
