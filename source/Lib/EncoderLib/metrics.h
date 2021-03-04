#include "CodingStructure.h"

namespace vvenc {

class RdCost;

class Metrics {
private:
    Metrics() { memset(val, 0, sizeof(val)); }
public:
    enum { SSE_Y = 0, SAD_Y, HAD_Y, SSIM_Y, HAD_2SAD_Y, SSE_YUV, SAD_YUV, HAD_YUV, SSIM_YUV, HAD_2SAD_YUV, TOTAL, };
    static Metrics CalculateMetrics(const CodingStructure* cu, PictureType pictureType, RdCost& rdCost);
    double val[TOTAL];
    static double getWeightedDist(RdCost& rdCost, ComponentID compID, uint64_t unweightedDist);
};

}
