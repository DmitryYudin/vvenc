#include "metrics.h"
#include "CodingStructure.h"
#include "RdCost.h"
#include "ssim.h"

#include <assert.h>

namespace vvenc {

Metrics Metrics::CalculateMetrics(const CodingStructure* cu, PictureType pictureType, RdCost& rdCost)
{
    Metrics m;
    assert(cu->sps->bitDepths[ CH_L ] == cu->sps->bitDepths[ CH_C ]);

    double x[TOTAL][3];
    memset(x, 0, sizeof(x));

    for(unsigned i = 0; i < 3; i++) {
        ComponentID componentID = i == 0 ? COMP_Y : i == 1 ? COMP_Cb : COMP_Cr;

        unsigned bd = cu->sps->bitDepths[ componentID == COMP_Y ? CH_L : CH_C ];
        CPelBuf org = cu->getOrgBuf(componentID);
        CPelBuf cur;
        switch(pictureType) {
            case PIC_PREDICTION: cur = cu->getPredBuf(componentID); break;
            case PIC_RECONSTRUCTION: cur = cu->getRecoBuf(componentID); break;
            default: break;
        }
        auto calcDist = [&rdCost](const CPelBuf& org, const CPelBuf& cur, int bitDepth, DFunc dfunc) -> double {
            if(dfunc == DF_TOTAL_FUNCTIONS) {
                return CalculateSSIM(org.buf, org.stride, cur.buf, cur.stride, org.width, org.height, (1<<bitDepth) - 1);
            } else {
                DistParam distParam = rdCost.setDistParam( org, cur, bitDepth, dfunc );
                return distParam.distFunc( distParam );                
            }
        };

        x[SSE][i] = calcDist( org, cur, bd, DF_SSE );
        x[SAD][i] = calcDist( org, cur, bd, DF_SAD );
        x[HAD][i] = calcDist( org, cur, bd, DF_HAD );
        x[HAD_2SAD][i] = calcDist( org, cur, bd, DF_HAD_2SAD );
        x[SSIM][i] = calcDist( org, cur, bd, DF_TOTAL_FUNCTIONS );
    }

    for(unsigned i = 0; i < TOTAL; i++) {
        const double *y = x[i];
        double val = 0;
        switch(i) {
            case SSE:
            case SAD:
            case HAD:
            case HAD_2SAD: // Chroma is not initialized this time, so we ignore it for a while
                //val = (4*y[0] + y[1] + y[2])/6;
                val = y[0];
                break;
            case SSIM:
                val = y[0];
                break;
            default:
                break;
        }
        m.val[i] = val;
    }
    return m;
}

}
