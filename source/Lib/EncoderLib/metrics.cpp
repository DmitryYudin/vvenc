#include "metrics.h"
#include "CodingStructure.h"
#include "RdCost.h"
#include "ssim.h"

#include <assert.h>

namespace vvenc {
double Metrics::getWeightedDist(RdCost& rdCost, ComponentID compID, uint64_t unweightedDist)
{
    return COMP_Y == compID ? static_cast<double>(unweightedDist) : std::floor(rdCost.getDistortionWeight(compID) * unweightedDist);
};

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
        auto calcDist = [&rdCost, &cu, i](const CPelBuf& org, const CPelBuf& cur, int bitDepth, DFunc dfunc) -> double {

            if ( std::any_of(cu->cus.begin(), cu->cus.end(), [i](const CodingUnit * const cu) { return cu->blocks[i].area() == 0; }) ) {
                return 0.;
            }

            if(dfunc == DF_TOTAL_FUNCTIONS) {
                return CalculateSSIM(org.buf, org.stride, cur.buf, cur.stride, org.width, org.height, (1<<bitDepth) - 1);
            } else {
                DistParam distParam = rdCost.setDistParam( org, cur, bitDepth, dfunc );
                return distParam.distFunc( distParam );                
            }
        };
        
        x[SSE_Y][i] = calcDist( org, cur, bd, DF_SSE );
        x[SSE_YUV][i] = x[SSE_Y][i];
        x[SAD_Y][i] = calcDist(org, cur, bd, DF_SAD);
        x[SAD_YUV][i] = x[SAD_Y][i];
        x[HAD_Y][i] = calcDist( org, cur, bd, DF_SSE);//DF_HAD );
        x[HAD_YUV][i] = x[HAD_Y][i];
        x[HAD_2SAD_Y][i] = calcDist( org, cur, bd, DF_SSE); //DF_HAD_2SAD );
        x[HAD_2SAD_YUV][i] = x[HAD_2SAD_Y][i];
        x[SSIM_Y][i] = calcDist( org, cur, bd, DF_TOTAL_FUNCTIONS );
        x[SSIM_YUV][i] = x[SSIM_Y][i];
    }
    
    auto combineYUV = [&rdCost](const double * yuvm) {
        return yuvm[COMP_Y] + getWeightedDist(rdCost, COMP_Cb, yuvm[COMP_Cb])
                            + getWeightedDist(rdCost, COMP_Cr, yuvm[COMP_Cr]);
    };

    for(unsigned i = 0; i < TOTAL; i++) {
        const auto y = x[i];
        double val = 0;
        switch(i) {
            case SSE_Y:
            case SAD_Y:
            case HAD_Y:
            case SSIM_Y:
            case HAD_2SAD_Y:
                val = y[COMP_Y];
                break;
            case SSE_YUV:
            case SAD_YUV:
            case HAD_YUV:
            case HAD_2SAD_YUV:
            case SSIM_YUV:
              val = combineYUV(y);
              break;
            default:
                break;
        }
        m.val[i] = val;
    }
    return m;
}

}
