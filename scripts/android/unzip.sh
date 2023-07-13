AAR=$1

if [[ -z "$AAR" ]]; then
  VERSION=$2
  if [[ -z "$VERSION" ]]; then
    VERSION="2.5.4"
  fi
  AAR=~/.m2/repository/io/dolby/comms-sdk-android-cppsdk/$VERSION/*.aar
fi

rm -rf .tmp/aar || echo "skipping cleaning .tmp/aar"
mkdir -p .tmp/aar || echo "tmp already exists, skipping"

# extract all the files we will need for compilation only ...
pushd .tmp/aar
echo "using aar " $AAR
unzip "$AAR"
popd

# now copy the files
# Note : we are using root/sdk-release-android but a sdk-release/$platform would be preferrable
RELEASE=./DolbyIO/sdk-release-android
rm -rf $RELEASE
mkdir $RELEASE

mkdir -p $RELEASE/libs/android.arm64-v8a
mkdir -p $RELEASE/libs/android.x86_64

# copy the shared object
cp -r .tmp/aar/prefab/modules/sdk/* $RELEASE
cp -r .tmp/aar/jni/arm64-v8a/libdolbyio_comms_media.so $RELEASE/libs/android.arm64-v8a
cp -r .tmp/aar/jni/arm64-v8a/libdolbyio_comms_sdk_android_cppsdk.so $RELEASE/libs/android.arm64-v8a
cp -r .tmp/aar/jni/x86_64/libdolbyio_comms_media.so $RELEASE/libs/android.x86_64
cp -r .tmp/aar/jni/x86_64/libdolbyio_comms_sdk_android_cppsdk.so $RELEASE/libs/android.x86_64
