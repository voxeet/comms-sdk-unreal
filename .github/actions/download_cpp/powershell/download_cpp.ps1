Write-Output "Download asset"
curl -Uri ${Env:INPUT_URL}/${Env:INPUT_TAG}/${Env:INPUT_ASSET_NAME} -UseBasicParsing -Out ${Env:GITHUB_WORKSPACE}/${Env:INPUT_ASSET_NAME}
Expand-Archive -Force ${Env:GITHUB_WORKSPACE}/${Env:INPUT_ASSET_NAME} -DestinationPath ${Env:GITHUB_WORKSPACE}/${Env:CPP_SDK_DESTINATION}
rm ${Env:GITHUB_WORKSPACE}/${Env:INPUT_ASSET_NAME}
