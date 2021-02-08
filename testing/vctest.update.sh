set -eu

pushd ..
./build.sh 2
cp ./bin/release-static/vvencapp.exe ../vctest/bin/windows/vvenc2 