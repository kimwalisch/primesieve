# Usage: ./scripts/build_clang_win_x64.ps1
# Self-contained build script for Windows x64 using Clang 21.1.8

$ErrorActionPreference = "Stop"

Remove-Item -Recurse -Force "build-release" -ErrorAction SilentlyContinue
mkdir build-release
Set-Location "build-release"

$LLVM_VERSION = "21.1.8"
$LLVM_LOCAL_DIR = Join-Path (Get-Location) "llvm-toolchain"
$7ZIP_LOCAL_DIR = Join-Path (Get-Location) "7zip-portable"
$7ZR_URL = "https://www.7-zip.org/a/7zr.exe"
$7Z_EXTRA_URL = "https://www.7-zip.org/a/7z2409-extra.7z"
$LLVM_URL = "https://github.com/llvm/llvm-project/releases/download/llvmorg-$LLVM_VERSION/clang+llvm-$LLVM_VERSION-x86_64-pc-windows-msvc.tar.xz"
$PrevReleaseUrl = "https://github.com/kimwalisch/primesieve/releases/download/v12.11/primesieve-12.11-win-x64.zip"

###########################################################################

New-Item -ItemType Directory -Force -Path $7ZIP_LOCAL_DIR | Out-Null

# Stage A: Download 7zr.exe (standalone that can extract .7z)
Write-Host "Downloading 7zr.exe..."
Invoke-WebRequest -Uri $7ZR_URL -OutFile "$7ZIP_LOCAL_DIR\7zr.exe"

# Stage B: Download 7-Zip Extra (.7z format)
Write-Host "Downloading 7-Zip Extra..."
$7zExtraFile = Join-Path $7ZIP_LOCAL_DIR "7z-extra.7z"
Invoke-WebRequest -Uri $7Z_EXTRA_URL -OutFile $7zExtraFile

# Stage C: Use 7zr to extract 7-Zip Extra
Write-Host "Extracting 7-Zip Extra using 7zr..."
& "$7ZIP_LOCAL_DIR\7zr.exe" x $7zExtraFile -o"$7ZIP_LOCAL_DIR" -y | Out-Null

# Cleanup temp 7z file
Remove-Item $7zExtraFile -Force
Write-Host "7-Zip Portable is ready." -ForegroundColor Green
$7z = "$7ZIP_LOCAL_DIR\7za.exe"

###########################################################################

$xzFile = Join-Path (Get-Location) "llvm_temp.tar.xz" 
$tarFile = Join-Path (Get-Location) "llvm_temp.tar"

Write-Host "Downloading LLVM..."
(New-Object System.Net.WebClient).DownloadFile($LLVM_URL, $xzFile)

Write-Host "Extracting LLVM (this may take a moment)..."
# Using 7z (standard on GH runners and common on Windows)
& $7z x $xzFile -y | Out-Null
& $7z x $tarFile -o"$LLVM_LOCAL_DIR" -y | Out-Null

# The tar contains a long-named subfolder; move its contents up to 'llvm-toolchain'
$subfolder = Get-ChildItem -Path $LLVM_LOCAL_DIR -Directory | Select-Object -First 1
Move-Item -Path "$($subfolder.FullName)\*" -Destination $LLVM_LOCAL_DIR -Force

# Cleanup temp files
Remove-Item $xzFile, $tarFile -Force
Remove-Item $subfolder.FullName -Recurse -Force
Write-Host "LLVM extracted to: $LLVM_LOCAL_DIR" -ForegroundColor Green

###########################################################################

# Add the local LLVM bin to the path for this session
$env:Path = "$(Join-Path $LLVM_LOCAL_DIR 'bin');$env:Path"

# === Version Setup ===
$VersionLine = Select-String -Path "../include/primesieve.hpp" -Pattern 'PRIMESIEVE_VERSION "(.*)"'
$Version = $VersionLine.Matches.Groups[1].Value
$FullDate = Get-Date -Format "MMMM dd, yyyy"
$Year = Get-Date -Format "yyyy"

Write-Host "Compiling with Clang $LLVM_VERSION..." -ForegroundColor Cyan

# Manually resolve wildcards for PowerShell
$SrcFiles = Get-Item "../src/*.cpp"
$ArchFiles = Get-Item "../src/arch/x86/*.cpp"
$AppFiles = Get-Item "../src/app/*.cpp"

& clang++ -I../include -I../src -O3 -mpopcnt -DNDEBUG `
    -DENABLE_MULTIARCH_AVX512_BW -DENABLE_MULTIARCH_AVX512_VBMI2 `
    $SrcFiles $ArchFiles $AppFiles `
    -o primesieve.exe

if ($LASTEXITCODE -ne 0) { throw "Compilation failed." }
& llvm-strip primesieve.exe

###########################################################################

# === Packaging ===
Write-Host "Packaging release..."
Invoke-WebRequest -Uri $PrevReleaseUrl -OutFile "prev.zip"
Expand-Archive -Path "prev.zip" -DestinationPath "primesieve-$Version-win-x64-tmp" -Force

Write-Host ""
Write-Host "Old primesieve binary size:"
Get-Item primesieve-$Version-win-x64-tmp/primesieve.exe | Select-Object Name, Length

Move-Item -Force "primesieve.exe" "primesieve-$Version-win-x64-tmp"
Write-Host "New primesieve binary size:"
Get-Item primesieve-$Version-win-x64-tmp/primesieve.exe | Select-Object Name, Length

# Update metadata
(Get-Content "primesieve-$Version-win-x64-tmp\README.txt") -replace "^primesieve.*", "primesieve $Version" -replace "^\w+ \d+, \d+", $FullDate | Set-Content "primesieve-$Version-win-x64-tmp\README.txt"
(Get-Content "primesieve-$Version-win-x64-tmp\COPYING") -replace "Copyright \(c\) 2010 - \d+", "Copyright (c) 2010 - $Year" | Set-Content "primesieve-$Version-win-x64-tmp\COPYING"

Write-Host "Running Tests..."
Set-Location "primesieve-$Version-win-x64-tmp"
./primesieve -v
if ($LASTEXITCODE -ne 0) { throw "Tests failed." }
./primesieve --cpu-info
if ($LASTEXITCODE -ne 0) { throw "Tests failed." }
./primesieve --test
if ($LASTEXITCODE -ne 0) { throw "Tests failed." }
./primesieve 1e11
if ($LASTEXITCODE -ne 0) { throw "Tests failed." }

Compress-Archive `
  -Path (Get-Item README.txt, primesieve.exe, COPYING) `
  -DestinationPath "primesieve-$Version-win-x64.zip" `
  -Force

Move-Item "primesieve-$Version-win-x64.zip" .. -Force
Set-Location ..
Rename-Item "primesieve-$Version-win-x64-tmp" "primesieve-$Version-win-x64"

Write-Host ""
Write-Host "Release binary built successfully!" -ForegroundColor Green
