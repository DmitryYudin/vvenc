#include "metrics.h"
#include "CodingStructure.h"
#include "RdCost.h"
#include "ssim.h"

#include <assert.h>

namespace vvenc {
double Metrics::getWeightedDist(RdCost& rdCost, ComponentID compID, uint64_t unweightedDist) { return COMP_Y == compID ? unweightedDist : rdCost.getDistortionWeight(compID) * unweightedDist; };

//Metrics Metrics::CalculateMetrics(const CodingStructure* cu, PictureType pictureType, RdCost& rdCost, const unsigned int limit)
Metrics Metrics::CalculateMetrics(const CodingUnit* cu, PictureType pictureType, RdCost& rdCost, const unsigned int limit)
{
    Metrics m;
    assert(cu->cs->sps->bitDepths[ CH_L ] == cu->cs->sps->bitDepths[ CH_C ]);
    enum { SSE, SAD, HAD, SSIM, HAD_2SAD, METRICS_TOTAL};
    double x[METRICS_TOTAL][MAX_NUM_COMP];
    memset(x, 0, sizeof(x));

    //for(unsigned i = 0; i < 3; i++) {
    for (unsigned i = 0; i < limit; i++) {
        ComponentID componentID = i == 0 ? COMP_Y : i == 1 ? COMP_Cb : COMP_Cr;
        
        const CompArea& compArea = cu->blocks[componentID];        //const CPelBuf orgPel = cu->cs.getOrgBuf(compArea);
        unsigned bd = cu->cs->sps->bitDepths[ componentID == COMP_Y ? CH_L : CH_C ];
        CPelBuf org = cu->cs->getOrgBuf(compArea);
        CPelBuf cur;
        switch(pictureType) {
            case PIC_PREDICTION: cur = cu->cs->getPredBuf(compArea); break;
            case PIC_RECONSTRUCTION: cur = cu->cs->getRecoBuf(compArea); break;
            default: break;
        }
        auto calcDist = [&rdCost](const CPelBuf& org, const CPelBuf& cur, int bitDepth, DFunc dfunc) -> Distortion {//picture boundary conditions should be handled somehow
            if(dfunc == DF_TOTAL_FUNCTIONS) {
                return (CalculateSSIM(org.buf, org.stride, cur.buf, cur.stride, org.width, org.height, (1<<bitDepth) - 1) * (1ll << 62));
            } else {
                DistParam distParam = rdCost.setDistParam( org, cur, bitDepth, dfunc );
                return distParam.distFunc( distParam );
            }
        };
        
        x[SSE][i] = getWeightedDist(rdCost, static_cast<ComponentID>(i), calcDist( org, cur, bd, DF_SSE ));
        x[SAD][i] = getWeightedDist(rdCost, static_cast<ComponentID>(i), calcDist( org, cur, bd, DF_SAD ));
        x[HAD][i] = getWeightedDist(rdCost, static_cast<ComponentID>(i), calcDist( org, cur, bd, DF_HAD ));
        x[HAD_2SAD][i] = getWeightedDist(rdCost, static_cast<ComponentID>(i), calcDist( org, cur, bd, DF_HAD_2SAD ));
        x[SSIM][i] = getWeightedDist(rdCost, static_cast<ComponentID>(i), calcDist( org, cur, bd, DF_TOTAL_FUNCTIONS ));
    }

    const unsigned metricsTotalNum = getNumberOfMetrics();
    for(unsigned i = 0; i < metricsTotalNum; i++) {
        const double *y = x[i];
        double val_y = 0, val_yuv = 0;
        switch(i) {
            case SSE:
            case SAD:
            case HAD:
            //case SSE_WTD:
            case HAD_2SAD: // Chroma is not initialized this time, so we ignore it for a while
                //val = (4*y[0] + y[1] + y[2])/6;
                val_y = y[0];
                val_yuv = y[0] + y[1] + y[2];
                break;
            case SSIM:
                val_y = y[0];
                break;
            default:
                break;
        }
        m.val[i] = val_y;
        m.val[i + metricsTotalNum] = val_yuv;
    }
    return m;
}

}
