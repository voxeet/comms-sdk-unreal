#!/bin/bash
ID_OF_RELEASE=$(curl --fail -s -L \
  -H "Accept: application/vnd.github+json" \
  -H "Authorization: Bearer ${GH_TOKEN}"\
  -H "X-GitHub-Api-Version: 2022-11-28" \
    https://api.github.com/repos/DolbyIO/comms-sdk-unreal/releases/tags/${GITHUB_REF_NAME} | jq ".id")

echo "upload file $1 for tag: ${GITHUB_REF_NAME} for ID: ${ID_OF_RELEASE}"
curl -L \
  -X POST \
  -H "Accept: application/vnd.github+json" \
  -H "Authorization: token ${GH_TOKEN}"\
  -H "X-GitHub-Api-Version: 2022-11-28" \
  -H "Content-Type: application/octet-stream" \
  https://uploads.github.com/repos/DolbyIO/comms-sdk-unreal/releases/${ID_OF_RELEASE}/assets?name=${1}\
  --data-binary "@${1}"