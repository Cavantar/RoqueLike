#include <vector>
#include <iostream>

#include "Noise.h"
#include "Profiler.h"

#include "SimpleParser.h"

void noiseTest()
{
  int testCount = 5;

  for(int testIt = 0; testIt < testCount; testIt++)
  {
    Profiler::create();
    Profiler::get()->startFrame();

    std::list<GenData> genDataList;
    std::unordered_map<int32,GenData> genDataMap;

    GenData genData = { NT_WORLEY, {0.2f, 1, 2.0f, 0.4f}, 2.0f };
    genDataList.push_back(genData);
    genDataList.push_back(genData);
    genDataList.push_back(genData);

    genDataMap[1] = genData;
    // genDataMap[2] = genData;
    //genDataMap[3] = genData;


    int a = 0;

    for(int i = 0; i < 10; i++)
    {
      Profiler::get()->start("NoiseMapAq");
      std::vector<Vec4f>& map = Noise::getMapFast(Vec2f(0, 0), 65, genDataMap, "Map1");
      // std::vector<Vec4f>& map = Noise::getMap(Vec2f(0, 0), 65, genDataList, "1");
      Profiler::get()->end("NoiseMapAq");
      // getMapFast: 13016042
      // WithParser: 6811608
      // WithoutParser: 4805008

      // getMap:
      // default: 3951405
      // with Parser: 13718126
      // withoutVariableInsertion:  5734096
      a += map[0].x;
    }

    Profiler::get()->endFrame();
    Profiler::get()->showData();

    Profiler::destroy();
  }
}

void parserTest()
{
  SimpleParser sp;

  std::string expression;

  StringList validVariables{"one", "two", "four", "test1"};

  do
  {
    std::cout << "Please Enter an expression: \n";
    std::getline(std::cin, expression);
    if(expression == "q") break;
    bool correctness = SimpleParser::isExpressionCorrect(expression, validVariables);
    std::cout << "Expression iiiiss: " << (correctness ? "Correct" : "Incorrect") << std::endl;

    VariableMap variableMap;
    real32 test1 = 5;
    variableMap["test1"] = &test1;
    sp.setVariableMap(&variableMap);

    float result = sp.evaluateExpression(expression);
    std::cout << std::endl << expression << " = " << result << std::endl;

  } while (expression != "q");
}


void blendTest()
{

  std::cout << blendSmooth(0, 1.0f, 0.4f, 0.2f) << std::endl;
  std::cout << blendSmooth(0, 1.0f, 0.25f, 0.2f) << std::endl;
  std::cout << blendSmooth(0, 1.0f, 0.3f, 0.2f) << std::endl;
  std::cout << blendSmooth(0, 1.0f, 0.35f, 0.2f) << std::endl;

}

void matTest()
{
  Mat4 testMat;
  testMat.m[0] = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
  testMat.showData();

  testMat.transpose();
  testMat.showData();

  Vec3f testVec(1.0f, 0, 0);
  testVec.showData();

  Mat3 testMat3;

  Vec3f resultVec = testMat3 * testVec;
  testVec.showData();

  testMat3 = Mat3::createRotationMatrix(45.0f, Vec3f(0, 0, 1.0f));

  resultVec = testMat3 * testVec;
  resultVec.showData();


  // (Vec3f::rotateAround(Vec3f(1.0f, 0, 0), 45.0f, Vec3f(0, 1.0f, 0))).showData();
}

int main()
{
  noiseTest();
  // parserTest();
  // matTest();


  return 0;
}
