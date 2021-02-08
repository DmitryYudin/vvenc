set -eu

CODECS="vvenc vvenc2"
PRMS="22 27 32 37"
VECTORS="akiyo_176x144_30fps.yuv akiyo_352x288_30fps.yuv foreman_176x144_30fps.yuv foreman_352x288_30fps.yuv"

. vctest.vectors.xiph.txt
#VECTORS=$vec_1280x720

../../vctest/core/bdrate.sh -i "$VECTORS" -c "$CODECS" -p "$PRMS" -o "bdrate.log" "$@"
