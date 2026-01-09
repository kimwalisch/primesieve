# Usage: ./scripts/build_clang_x64.ps1
$ErrorActionPreference = "Stop"

# Configuration ######################################################

$LLVM_VER    = "21.1.8"
$BUILD_DIR   = Join-Path (Get-Location) "build-release"
$LLVM_DIR    = Join-Path $BUILD_DIR "llvm-toolchain"
$7ZIP_DIR    = Join-Path $BUILD_DIR "7z-extra"
$URL_7ZR     = "https://www.7-zip.org/a/7zr.exe"
$URL_7Z_EXT  = "https://www.7-zip.org/a/7z2409-extra.7z"
$URL_LLVM    = "https://github.com/llvm/llvm-project/releases/download/llvmorg-$LLVM_VER/clang+llvm-$LLVM_VER-x86_64-pc-windows-msvc.tar.xz"
$URL_PREV    = "https://github.com/kimwalisch/primesieve/releases/download/v12.11/primesieve-12.11-win-x64.zip"

# Clean and Init #####################################################

Remove-Item -Recurse -Force $BUILD_DIR -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $7ZIP_DIR | Out-Null
Set-Location $BUILD_DIR

# Helper: Silent Download (.NET WebClient is faster than Invoke-WebRequest and has no progress bar)
function Download-File ($Url, $Dest) {
    Write-Host "Downloading $(Split-Path $Dest -Leaf)..."
    (New-Object System.Net.WebClient).DownloadFile($Url, $Dest)
}

# Setup 7-Zip ########################################################

Download-File $URL_7ZR "$BUILD_DIR\7zr.exe"
Download-File $URL_7Z_EXT "$BUILD_DIR\7z-extra.7z"

Write-Host "Extracting 7-Zip..."
& "$BUILD_DIR\7zr.exe" x "7z-extra.7z" -o"$7ZIP_DIR" -y | Out-Null
$7z = "$7ZIP_DIR\7za.exe"

# Setup LLVM #########################################################

Download-File $URL_LLVM "$BUILD_DIR\llvm.tar.xz"

Write-Host "Extracting LLVM..."
& $7z x "llvm.tar.xz" -y | Out-Null
& $7z x "llvm.tar" -o"$LLVM_DIR" -y | Out-Null

# Move contents from nested folder to root of $LLVM_DIR
$subDir = Get-ChildItem $LLVM_DIR -Directory | Select-Object -First 1
Move-Item "$($subDir.FullName)\*" $LLVM_DIR -Force
Remove-Item "llvm.tar.xz", "llvm.tar", $subDir.FullName -Recurse -Force
$env:Path = "$(Join-Path $LLVM_DIR 'bin');$env:Path"

# Compilation ########################################################

$Version = [regex]::Match((Get-Content "../include/primesieve.hpp"), 'PRIMESIEVE_VERSION "(.*)"').Groups[1].Value
Write-Host "Compiling Primesieve $Version with Clang $LLVM_VER..." -ForegroundColor Cyan

# Gather source files
$Src = @("../src/*.cpp", "../src/arch/x86/*.cpp", "../src/app/*.cpp") | ForEach-Object { Get-Item $_ }

# Splatting arguments for readability
$ClangArgs = @(
    "-I../include", "-I../src", "-O3", "-mpopcnt", "-DNDEBUG",
    "-DENABLE_MULTIARCH_AVX512_BW", "-DENABLE_MULTIARCH_AVX512_VBMI2",
    "-o", "primesieve.exe"
)
& clang++ $ClangArgs $Src

if ($LASTEXITCODE -ne 0) { throw "Compilation failed." }
& llvm-strip primesieve.exe

# Packaging ##########################################################

Write-Host "Packaging release..."
Download-File $URL_PREV "$BUILD_DIR\prev.zip"
$PkgName = "primesieve-$Version-win-x64"
Expand-Archive "prev.zip" -DestinationPath "$PkgName-tmp" -Force

# Verify Size and Move Binary
Write-Host "Old binary size: $((Get-Item "$PkgName-tmp/primesieve.exe").Length)"
Move-Item "primesieve.exe" "$PkgName-tmp" -Force
Write-Host "New binary size: $((Get-Item "$PkgName-tmp/primesieve.exe").Length)"

# Update version info (Regex replace)
$Date = Get-Date -Format "MMMM dd, yyyy"
$Year = Get-Date -Format "yyyy"
(Get-Content "$PkgName-tmp\README.txt") -replace "^primesieve.*", "primesieve $Version" -replace "^\w+ \d+, \d+", $Date | Set-Content "$PkgName-tmp\README.txt"
(Get-Content "$PkgName-tmp\COPYING")    -replace "Copyright \(c\) 2010 - \d+", "Copyright (c) 2010 - $Year"    | Set-Content "$PkgName-tmp\COPYING"

# Testing ############################################################

Write-Host "Running Tests..."
Set-Location "$PkgName-tmp"
Write-Host "================================================================================"

foreach ($arg in @("-v", "--cpu-info", "--test", "1e11")) { 
    & ./primesieve $arg
    if ($LASTEXITCODE -ne 0) { throw "Test failed: $arg" }
    Write-Host "================================================================================"
}

# Compress zip archive ###############################################

Compress-Archive -Path "README.txt", "primesieve.exe", "COPYING" -DestinationPath "..\$PkgName.zip" -Force
Set-Location ..
Rename-Item "$PkgName-tmp" $PkgName
Write-Host "Release binary built successfully!" -ForegroundColor Green
