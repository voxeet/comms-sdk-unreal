#!/bin/sh
AAR_PATH=$1
AAR_NO_PREFAB_PATH=$2
if [[ -z "$AAR_PATH" ]]; then
  AAR_PATH=~/.m2/repository/io/dolby/comms-sdk-android-cppsdk/2.6.0/*.aar
fi

AAR_TMP=aar_tmp
RELEASE_PATH="DolbyIO/sdk-release-android"

rm -rf ${RELEASE_PATH}
mkdir ${RELEASE_PATH}

unzip ${AAR_PATH} -d ${AAR_TMP}

cp -r ${AAR_TMP}/prefab/modules/sdk/* ${RELEASE_PATH}

for ARCH in arm64-v8a armeabi-v7a x86 x86_64
do
  LIB_PATH=${AAR_TMP}/jni/${ARCH}
  cp ${LIB_PATH}/libdolbyio_comms_media.so ${LIB_PATH}/libdolbyio_comms_sdk_android_cppsdk.so ${RELEASE_PATH}/libs/android.${ARCH}
done

if [[ "$AAR_NO_PREFAB_PATH" ]]; then
  cp ${AAR_NO_PREFAB_PATH} ${RELEASE_PATH}
fi

rm -rf $AAR_TMP
