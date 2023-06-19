# clean tmp folder
rm -rf tmp

# and build for android
DIR=`pwd`

# it supposes that RunUAT.sh is in the PATH
RunUAT.sh BuildPlugin -Rocket              \
  -Plugin="$DIR/DolbyIO/DolbyIO.uplugin"   \
  -TargetPlatforms=Android                 \
  -StrictIncludes                          \
  -Package="$DIR/Output"                   \
  -Architecture_Android=x64+arm64
