#!/bin/sh
AAR_PATH=$1
AAR_NO_PREFAB_PATH=$2
AAR_SHARED_PATH=$3

if [[ -z "$AAR_PATH" ]]; then # use Maven
  AAR_PATH=~/.m2/repository/io/dolby/comms-sdk-android-cppsdk/2.7.0/*.aar
  AAR_NO_PREFAB_PATH=~/.m2/repository/io/dolby/comms-sdk-android-cppsdk-noprefab/2.7.0/*.aar
  AAR_SHARED_PATH=~/.m2/repository/io/dolby/comms-sdk-android-cppsdk-shared/2.7.0/*.aar
fi

AAR_TMP=aar_tmp
RELEASE_PATH="DolbyIO/sdk-release-android"

# clean release
rm -rf ${RELEASE_PATH}
mkdir ${RELEASE_PATH}

# get includes from main aar
unzip ${AAR_PATH} -d ${AAR_TMP}
cp -r ${AAR_TMP}/prefab/modules/sdk/include ${RELEASE_PATH}
rm -rf ${AAR_TMP}

# get libs from noprefab aar
unzip ${AAR_NO_PREFAB_PATH} -d ${AAR_TMP}
for ARCH in arm64-v8a
do
  LIBS_PATH=${RELEASE_PATH}/libs/android.${ARCH}
  mkdir -p ${LIBS_PATH}
  for LIB in media sdk
  do
    cp ${AAR_TMP}/jni/${ARCH}/libdolbyio_comms_${LIB}.so ${LIBS_PATH}
  done
done
rm -rf ${AAR_TMP}

# copy noprefab and shared aar
cp ${AAR_NO_PREFAB_PATH} ${RELEASE_PATH}
cp ${AAR_SHARED_PATH} ${RELEASE_PATH}
