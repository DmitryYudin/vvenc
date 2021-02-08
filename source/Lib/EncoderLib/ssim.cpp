#include "ssim.h"

#include <math.h>
#include <memory>

static double similarity(uint32_t x, uint32_t y, uint32_t xx, uint32_t yy, uint32_t xy,
    unsigned sample_max, unsigned w_win, unsigned h_win)
{
    double c1, c2;
    unsigned count = w_win*h_win;
    {
        float c1_tmp = .01 * sample_max * count;
        float c2_tmp = .03 * sample_max * count;
        c1 = round(c1_tmp*c1_tmp + .5);
        c2 = round(c2_tmp*c2_tmp + .5);
    }

    double ssim_n = (2.0 * x * y + c1) *
                    (2.0 * count * xy - 2.0 * x * y + c2);

    double ssim_d = ((double)x * x  + (double)y * y + c1) *
                    ((double)count * xx - (double)x * x + 
                     (double)count * yy - (double)y * y + c2);

    return ssim_n / ssim_d;
}

static
double ssim_win(const uint32_t sum_x[], const uint32_t sum_y[], const uint32_t sum_xx[], const uint32_t sum_yy[], const uint32_t sum_xy[],
    unsigned width, unsigned sample_max, unsigned w_win, unsigned h_win)
{
    uint32_t x = 0, y = 0, xx = 0, yy = 0, xy = 0;
    double ssim_total = 0;
    for (unsigned w = 0; w < w_win; w++) {
        x  += sum_x [w];
        y  += sum_y [w];
        xx += sum_xx[w];
        yy += sum_yy[w];
        xy += sum_xy[w];
    }
    for (unsigned w = w_win - 1; w < width;) {
        ssim_total += similarity(x, y, xx, yy, xy, sample_max, w_win, h_win);
        w++;
        if(w != width) {
            x  += sum_x [w] - sum_x [w - w_win];
            y  += sum_y [w] - sum_y [w - w_win];
            xx += sum_xx[w] - sum_xx[w - w_win];
            yy += sum_yy[w] - sum_yy[w - w_win];
            xy += sum_xy[w] - sum_xy[w - w_win];
        }
    }
    return ssim_total;
}

#define SCOPE_ARRAY(type, name, len) std::unique_ptr<type[]> name##_buf(new type[len]); auto name=name##_buf.get();

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

template<typename pixel_t>
double calculateSSIM(const pixel_t *px, unsigned stride_x, const pixel_t *py, unsigned stride_y, unsigned width, unsigned height,
    unsigned sample_max)
{
    unsigned w_win = 8, h_win = 8;
    //unsigned w_win = width, h_win = height;
    w_win = MIN(w_win, width);
    h_win = MIN(h_win, height);

    unsigned len = width + 1;
    SCOPE_ARRAY(uint32_t, sum_x,  len) memset(sum_x,  0, sizeof(uint32_t)*len);
    SCOPE_ARRAY(uint32_t, sum_y,  len) memset(sum_y,  0, sizeof(uint32_t)*len);
    SCOPE_ARRAY(uint32_t, sum_xx, len) memset(sum_xx, 0, sizeof(uint32_t)*len);
    SCOPE_ARRAY(uint32_t, sum_yy, len) memset(sum_yy, 0, sizeof(uint32_t)*len);
    SCOPE_ARRAY(uint32_t, sum_xy, len) memset(sum_xy, 0, sizeof(uint32_t)*len);

    for (unsigned h = 0; h < h_win; h++) {
        for (unsigned w = 0; w < width; w++) {
            sum_x [w] += px[w];
            sum_y [w] += py[w];
            sum_xx[w] += px[w] * px[w];
            sum_yy[w] += py[w] * py[w];
            sum_xy[w] += px[w] * py[w];
        }
        px += stride_x;
        py += stride_y;
    }

    double ssim_total = 0;
    unsigned ssim_count = 0;
    for (unsigned h = h_win - 1; h < height;) {
        ssim_total += ssim_win(sum_x, sum_y, sum_xx, sum_yy, sum_xy, width, sample_max, w_win, h_win);
        ssim_count += width - w_win + 1;

        h++;
        if(h != height) {
            const pixel_t *px_top = px - h_win*(intptr_t)stride_x;
            const pixel_t *py_top = py - h_win*(intptr_t)stride_y;
            for (unsigned w = 0; w < width; w++) {
                sum_x [w] += px[w]         - px_top[w];
                sum_y [w] += py[w]         - py_top[w];
                sum_xx[w] += px[w] * px[w] - px_top[w] * px_top[w];
                sum_yy[w] += py[w] * py[w] - py_top[w] * py_top[w];
                sum_xy[w] += px[w] * py[w] - px_top[w] * py_top[w];
            }
            px += stride_x;
            py += stride_y;
        }
    }

    ssim_total /= ssim_count;
    return ssim_total;
}

double CalculateSSIM(const uint8_t *px, unsigned stride_x, const uint8_t *py, unsigned stride_y, unsigned width, unsigned height,
    unsigned sample_max)
{
    return calculateSSIM(px, stride_x, py, stride_y, width, height, sample_max);
}
double CalculateSSIM(const int16_t *px, unsigned stride_x, const int16_t *py, unsigned stride_y, unsigned width, unsigned height,
    unsigned sample_max)
{
    return calculateSSIM(px, stride_x, py, stride_y, width, height, sample_max);
}