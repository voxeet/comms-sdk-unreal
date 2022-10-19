$Env:ZIP_OBJECT = (Split-Path ${Env:INPUT_SRC_DIR_PATH} -Leaf)

cd ${Env:GITHUB_WORKSPACE}/${Env:INPUT_SRC_DIR_PATH}/..
Compress-Archive -Path $Env:ZIP_OBJECT -DestinationPath ${Env:GITHUB_WORKSPACE}/${Env:INPUT_ZIP_NAME}

cd $Env:GITHUB_WORKSPACE
$Env:WINDOWSSHA256 = (Get-FileHash -Algorithm SHA256 ${Env:INPUT_ZIP_NAME}).hash

Write-Output "Package name: ${Env:INPUT_ZIP_NAME}"
Write-Output "SHASUM256: ${Env:WINDOWSSHA256} ${Env:INPUT_ZIP_NAME})"
Write-Output "C++ SDK ver.: ${Env:CPP_SDK_TAG}"
