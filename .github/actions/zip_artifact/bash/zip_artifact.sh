#!/bin/bash
set -e
ZIP_OBJECT=$(basename ${INPUT_SRC_DIR_PATH})

cd ${GITHUB_WORKSPACE}/${INPUT_SRC_DIR_PATH}/.. \
&& zip -rq ${GITHUB_WORKSPACE}/${INPUT_ZIP_NAME} $ZIP_OBJECT \
&& echo "Package name: ${INPUT_ZIP_NAME}" \
&& echo "SHASUM256: $(cd ${GITHUB_WORKSPACE}/ && shasum -a 256 ${INPUT_ZIP_NAME})" \
&& echo "C++ SDK ver.: ${CPP_SDK_TAG}"
