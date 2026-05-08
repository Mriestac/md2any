$ErrorActionPreference = "Stop"

$exePath = Join-Path (Resolve-Path ".\build-ninja").Path "md2any.exe"
$packageDir = Join-Path (Resolve-Path ".").Path "dist\md2any"
$packageExe = Join-Path $packageDir "md2any.exe"

if (-not (Test-Path $exePath)) {
    throw "Executable not found: $exePath. Run scripts/build.ps1 first."
}

if (Test-Path $packageDir) {
    Remove-Item -LiteralPath $packageDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $packageDir | Out-Null
Copy-Item -Path $exePath -Destination $packageExe -Force

windeployqt $packageExe --qmldir .\src\qml
