#include "CodingStructure.h"

namespace vvenc {

class RdCost;

class Metrics {
private:
    Metrics() {}
public:
    enum { SSE, SAD, HAD, SSIM, HAD_2SAD, TOTAL, };
    static Metrics CalculateMetrics(const CodingStructure* cu, PictureType pictureType, RdCost& rdCost);
    double val[TOTAL];
};

}
