#include "CodingStructure.h"

namespace vvenc {

class RdCost;

class Metrics {
//private:
    
public:
    enum { SSE_Y, SAD_Y, HAD_Y, SSIM_Y, HAD_2SAD_Y, SSE_YUV, SAD_YUV, HAD_YUV, SSIM_YUV, HAD_2SAD_YUV, TOTAL, };
    Metrics() { memset(val, 0, sizeof(val)); }
    static unsigned getNumberOfMetrics() { return TOTAL >> 1; };
    //static Metrics CalculateMetrics(const CodingStructure* cu, PictureType pictureType, RdCost& rdCost, const unsigned int limit = 3);
    static Metrics CalculateMetrics(const CodingUnit* cu, PictureType pictureType, RdCost& rdCost, const unsigned int limit = 3);
    double val[TOTAL];
    static double getWeightedDist(RdCost& rdCost, ComponentID compID, uint64_t unweightedDist);
    Metrics& operator+=(const Metrics& other)
    {
      for (int cnt = static_cast<int>(SSE_Y); cnt < static_cast<int>(TOTAL); ++cnt)
      {
        this->val[cnt] += other.val[cnt];
      }
      return *this;
    };
};

}
