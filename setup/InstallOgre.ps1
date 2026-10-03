
$BuildProj = $Args[1]
$OgreVersion = $Args[2]
$VSTVersion = $Args[3]

$ProjectRoot = "$($PSScriptRoot)\.."

$OgreDownloadUrl = "https://dl.cloudsmith.io/public/ogrecave/ogre/raw/versions/v$OgreVersion/ogre-sdk-v$OgreVersion-msvc$VSTVersion-x64.zip"


if ( -not (Test-Path "$($PSScriptRoot)\OGRE.zip")){
	Write-Output "[ogre-vsbt-setup] Downloading OGRE..."
	curl.exe -L "$OgreDownloadUrl" -o "$($PSScriptRoot)\OGRE.zip" --progress-bar
}

New-Item -Path "$($ProjectRoot)\OGRE" -ItemType Directory
New-Item -Path "$($ProjectRoot)\bin" -ItemType Directory

Add-Type -AssemblyName System.IO.Compression.FileSystem
Write-Output "[ogre-vsbt-setup] Extracting ogre-sdk-v$OgreVersion-msvc$VSTVersion-x64..."
[System.IO.Compression.ZipFile]::ExtractToDirectory("$($PSScriptRoot)\OGRE.zip", "$($ProjectRoot)\OGRE")


Copy-Item -Path "$($ProjectRoot)\OGRE\bin" -Destination "$($ProjectRoot)\bin" -Recurse
Rename-Item -Path "$($ProjectRoot)\bin\bin" -NewName "Release"
Copy-Item -Path "$($ProjectRoot)\OGRE\Media" -Destination "$($ProjectRoot)" -Recurse
Copy-Item -Path "$($PSScriptRoot)\plugins.cfg" -Destination "$($ProjectRoot)\bin\Release" -Force
Copy-Item -Path "$($PSScriptRoot)\resources.cfg" -Destination "$($ProjectRoot)\bin\Release" -Force

return 0
