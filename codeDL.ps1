
$dest = (Get-Location).Path

curl.exe -L "https://github.com/Kiliken/ogre-vsbt-setup/archive/refs/heads/main.tar.gz" -o "$($dest)\main.tar.gz" --progress-bar

tar -xzf "$($dest)\main.tar.gz" -C "$($dest)"

Move-Item "$($dest)\ogre-vsbt-setup-main\*" $PSScriptRoot -Force
Remove-Item "$($dest)\ogre-vsbt-setup-main" -Recurse -Force
Remove-Item "$($dest)\main.tar.gz" -Force
Rename-Item "$($dest)\gitignore.txt" ".gitignore"
