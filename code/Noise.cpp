#include "Noise.h"
#include "MiscFunctions.h"
#include <iostream>

int
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
  222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};
int
Noise::hashMask = 255;

float
Noise::gradients1D[] = { 1.0f, -1.0f };
int
Noise::gradients1DMask = 1;

Vec2f
Noise::gradients2D[] = { 
  Vec2f(1.0f, 0), 
  Vec2f(-1.0f, 0),
  Vec2f(0, 1.0f),
  Vec2f(0, -1.0f),
  normalize(Vec2f(-1.0f,1.0f)),
  normalize(Vec2f(1.0f, 1.0f)),
  normalize(Vec2f(1.0f, -1.0f)),
  normalize(Vec2f(-1.0f, -1.0f))
};

int
Noise::gradients2DMask = 7;

float
Noise::sqr2 = pow(2.0f, 0.5f);

float
Noise::value(Vec2f point, float frequency) {
  point *= frequency;
  
  int ix0 = (int)floor(point.x);
  int iy0 = (int)floor(point.y);
  
  float tx = point.x - ix0;
  float ty = point.y - iy0;
  
  ix0 &= hashMask;
  iy0 &= hashMask;
  
  int ix1 = ix0 + 1;
  int iy1 = iy0 + 1;
  
  ix1 &= hashMask;
  iy1 &= hashMask;
  
  int h0 = hash[ix0];
  int h1 = hash[ix1];
  
  int h00 = hash[(h0 + iy0)&hashMask];
  int h10 = hash[(h1 + iy0)&hashMask];
  int h01 = hash[(h0 + iy1)&hashMask];
  int h11 = hash[(h1 + iy1)&hashMask];
  
  tx = smooth(tx);
  ty = smooth(ty);
  
  float upX = interp((float)h00, (float)h10, tx);
  float downX = interp((float)h01, (float)h11, tx);
  
  // std::cout << "upX: " << upX << std::endl;
  // std::cout << "downX: " << downX << std::endl;
  
  float middle = interp(upX, downX, ty);
  
  return middle / 255.0f;
}

float Noise::perlin(float value, float frequency) {
  value *= frequency;
  
  int ix0 = (int)floor(value);
  
  float t0 = value - ix0;
  float t1 = t0 - 1.0f;
  
  ix0 &= hashMask;
  
  int ix1 = ix0 + 1;
  ix1 &= hashMask;
  
  float g0 = gradients1D[hash[ix0]&gradients1DMask];
  float g1 = gradients1D[hash[ix1]&gradients1DMask];
  
  float v0 = g0 * t0;
  float v1 = g1 * t1;
  
  float t = smooth(t0);
  
  return interp(v0, v1, t) * 2;
}

float Noise::perlin(Vec2f point, float frequency) {
  point *= frequency;
  
  int ix0 = (int)floor(point.x);
  int iy0 = (int)floor(point.y);
  
  float tx0 = point.x - ix0;
  float tx1 = tx0 - 1.0f;
  float ty0 = point.y - iy0;
  float ty1 = ty0 - 1.0f;
  
  ix0 &= hashMask;
  int ix1 = ix0 + 1;
  ix1 &= hashMask;
  
  iy0 &= hashMask;
  int iy1 = iy0 + 1;
  iy1 &= hashMask;
  
  int h0 = hash[ix0];
  int h1 = hash[ix1];
  
  Vec2f g00 = gradients2D[hash[(h0 + iy0)&hashMask] & gradients2DMask];
  Vec2f g10 = gradients2D[hash[(h1 + iy0)&hashMask] & gradients2DMask];
  Vec2f g01 = gradients2D[hash[(h0 + iy1)&hashMask] & gradients2DMask];
  Vec2f g11 = gradients2D[hash[(h1 + iy1)&hashMask] & gradients2DMask];
  
  float v00 = dotProduct(g00, tx0, ty0);
  float v10 = dotProduct(g10, tx1, ty0);
  float v01 = dotProduct(g01, tx0, ty1);
  float v11 = dotProduct(g11, tx1, ty1);
  
  float tx = smooth(tx0);
  float ty = smooth(ty0);
  
  float upX = interp(v00, v10, tx);
  float downX = interp(v01, v11, tx);

  // std::cout << "upX: " << upX << std::endl;
  // std::cout << "downX: " << downX << std::endl;
  // std::cout << "ty: " << ty << std::endl;
  // std::cout << "interp: " << interp(upX, downX, ty) << std::endl;
  
  return interp(upX, downX, ty) * sqr2;
}

float Noise::sumPerlin(Vec2f point, NoiseParams& noiseParams) {
  float sum = perlin(point, noiseParams.frequency);
  
  float amplitude = 1.0f;
  float range = 1.0f;
  
  float frequency = noiseParams.frequency;
  
  for(int i = 1; i < noiseParams.octaves; i++) {
    // Temp Code For Doing Offsets
    // ---------------------------
    int h1 = hash[((int)floor(frequency))&hashMask];
    int h2 = hash[noiseParams.octaves&hashMask];
    
    float norm1 = (float)hash[(h1 + noiseParams.octaves)&hashMask] / hashMask;
    float norm2 = (float)hash[(h2 + ((int)floor(noiseParams.frequency)))&hashMask] / hashMask;
    norm1 = 1.0f - norm1 * 2.0f;
    norm2 = 1.0f - norm2 * 2.0f;
    
    Vec2f offsetVec = Vec2f(norm1, norm2);
    // ---------------------------
    
    frequency *= noiseParams.lacunarity;
    amplitude *= noiseParams.persistence;
    range += amplitude;
    sum += perlin(point , frequency) * amplitude;
  }
  
  return sum/range;
}

float Noise::sumValue(Vec2f point, NoiseParams& noiseParams) {
  float sum = value(point, noiseParams.frequency);
  
  float amplitude = 1.0f;
  float range = 1.0f;
  float frequency = noiseParams.frequency;
  
  for(int i = 1; i < noiseParams.octaves; i++) {
    
    // Temp Code For Doing Offsets
    // ---------------------------
    int h1 = hash[((int)floor(frequency))&hashMask];
    int h2 = hash[noiseParams.octaves&hashMask];
    
    float norm1 = (float)hash[(h1 + noiseParams.octaves)&hashMask] / hashMask;
    float norm2 = (float)hash[(h2 + ((int)floor(noiseParams.frequency)))&hashMask] / hashMask;
    norm1 = 1.0f - norm1 * 2.0f;
    norm2 = 1.0f - norm2 * 2.0f;
    
    Vec2f offsetVec = Vec2f(norm1, norm2);
    // ---------------------------
    
    frequency *= noiseParams.lacunarity;
    amplitude *= noiseParams.persistence;
    range += amplitude;
    sum += value(point , frequency) * amplitude;
  }
  
  return sum / range;
}

/*
vector<glm::vec4>
Noise::getMap(Vec2f offset, int sideLength, list<GenData>& genDatas) {
  int numbOfVertices = sideLength * sideLength;
  vector<glm::vec4> vertices;
  vertices.resize(numbOfVertices);
  
  float stepSize = 1.0f / (sideLength - 1);
  float greyValue;
  
  Vec2f point00 = Vec2f(-0.5f, 0.5f);
  Vec2f point10 = Vec2f(0.5f, 0.5f);
  Vec2f point01 = Vec2f(-0.5f, -0.5f);
  Vec2f point11 = Vec2f(0.5f, -0.5f);
  
  vector<float> values;
  values.resize(genDatas.size());
  
  for(int y = 0; y < sideLength; y++) {
    Vec2f point0 = glm::lerp(point00, point01, ((float)y ) * stepSize);
    Vec2f point1 = glm::lerp(point10, point11, ((float)y ) * stepSize);
    for(int x = 0; x < sideLength; x++) {
      Vec2f point = glm::lerp(point0, point1, ((float)x) * stepSize);
      greyValue = 0;
      for(auto it = genDatas.begin(); it != genDatas.end(); it++) {
	if(it->noiseType == NT_PERLIN) {	
	  values[distance(genDatas.begin(), it)] = (Noise::sumPerlin(point + offset, it->noiseParams) * 0.5f + 0.5f) * it->scale;
	} else {
	  values[distance(genDatas.begin(), it)] = Noise::sumValue(point + offset, it->noiseParams) * it->scale;
	}
      }			
      
      float finalValue = 0;
      
      //for(auto i = values.begin(); i != values.end(); i++) {
      //	finalValue += *i;
      //}
      
      finalValue = values[0] + values[1] * values[2];
      
      vertices[y * sideLength + x] = glm::vec4(point.x * 100, finalValue * 100.0f, -point.y * 100, 1.0f);
    }
  }
  
  return vertices;
}
*/
