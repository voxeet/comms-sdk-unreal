param([string]$File = "file.zip")
$id_of_release=(  C:\ProgramData\chocolatey\bin\curl.exe  -s -L `
-H "Accept: application/vnd.github+json" `
-H "Authorization: Bearer ${env:GH_TOKEN}" `
-H "X-GitHub-Api-Version: 2022-11-28" `
https://api.github.com/repos/DolbyIO/comms-sdk-unreal/releases/tags/${env:GITHUB_REF_NAME} | C:\ProgramData\chocolatey\bin\jq.exe ".id")

echo "upload file $File for tag: ${env:GITHUB_REF_NAME} for ID: ${id_of_release}"

C:\ProgramData\chocolatey\bin\curl.exe -L -X POST `
-H "Accept: application/vnd.github+json" `
-H "Authorization: token ${Env:GH_TOKEN}" `
-H "X-GitHub-Api-Version: 2022-11-28" `
-H "Content-Type: application/octet-stream" `
--data-binary "@$File" https://uploads.github.com/repos/DolbyIO/comms-sdk-unreal/releases/${id_of_release}/assets?name=$File