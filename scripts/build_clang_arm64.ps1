# Usage: ./scripts/build_clang_arm64.ps1
$ErrorActionPreference = "Stop"

# Configuration ######################################################

$LLVM_VER    = "21.1.8"
$BUILD_DIR   = Join-Path (Get-Location) "build-release"
$URL_PREV    = "https://github.com/kimwalisch/primesieve/releases/download/v12.11/primesieve-12.11-win-arm64.zip"

# Clean and Init #####################################################

Remove-Item -Recurse -Force $BUILD_DIR -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null
Set-Location $BUILD_DIR

# Helper: Silent Download (.NET WebClient is faster than Invoke-WebRequest and has no progress bar)
function Download-File ($Url, $Dest) {
    Write-Host "Downloading $(Split-Path $Dest -Leaf)..."
    (New-Object System.Net.WebClient).DownloadFile($Url, $Dest)
}

# Compilation ########################################################

$Version = [regex]::Match((Get-Content "../include/primesieve.hpp"), 'PRIMESIEVE_VERSION "(.*)"').Groups[1].Value
Write-Host "Compiling Primesieve $Version with Clang" -ForegroundColor Cyan

# Gather source files
$Src = @("../src/*.cpp", "../src/arch/arm/*.cpp", "../src/app/*.cpp") | ForEach-Object { Get-Item $_ }

# Compiler options
$ClangArgs = @(
    "-I../include", "-I../src", "-O3",
    "-DNDEBUG", "-DENABLE_MULTIARCH_ARM_SVE",
    "-o", "primesieve.exe"
)
& clang++ $ClangArgs $Src

if ($LASTEXITCODE -ne 0) { throw "Compilation failed." }
& llvm-strip primesieve.exe

# Packaging ##########################################################

Write-Host "Packaging release..."
Download-File $URL_PREV "$BUILD_DIR\prev.zip"
$PkgName = "primesieve-$Version-win-arm64"
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
