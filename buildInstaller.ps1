if ($env:APPVEYOR) {
    $localFolder = "$env:APPVEYOR_BUILD_FOLDER\temp"
} else {
    $localFolder = ".\temp"
}

# Create temp directory to store stuff in
if ((Test-Path $localFolder) -eq $false) {
  md $localFolder
}

$compilerName = "compiler"

echo "Downloading Compiler"
(New-Object System.Net.WebClient).DownloadFile("http://first.wpi.edu/FRC/roborio/toolchains/FRC-2017-Windows-Toolchain-4.9.3.zip", "$localFolder\$compilerName.zip")

# unzip everything
Add-Type -assembly "system.io.compression.filesystem"

function Unzip($file)
{
  [io.compression.zipfile]::ExtractToDirectory("$localFolder\$file.zip", "$localFolder\$file")
}

echo "Extracting Compiler"
Unzip($compilerName)

echo "Adding compiler to path"
$comp = "$localFolder\compiler\frc\bin"

$exePath = "$env:USERPROFILE\innosetup-5.5.9-unicode.exe"
 
Write-Host "Downloading InnoSetup 5.5.9..."
(New-Object Net.WebClient).DownloadFile('http://files.jrsoftware.org/is/5/innosetup-5.5.9-unicode.exe', $exePath)
 
Write-Host "Installing..."
cmd /c start /wait $exePath /silent
 
Write-Host "Installed InnoSetup 5.5.9" -ForegroundColor Green

$env:PATH += ";C:\Program Files (x86)\Inno Setup 5"
$env:PATH += ";C:\Program Files\Inno Setup 5"

if ($env:APPVEYOR) {
    .\gradlew installer windowsZip -PtoolChainPath="$comp" -PjenkinsBuild
} else {
    .\gradlew installer windowsZip -PtoolChainPath="$comp"
}
