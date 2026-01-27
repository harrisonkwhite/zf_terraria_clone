$ErrorActionPreference = "Stop"

$ProjectRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
$BuildDir = Join-Path $ProjectRoot "build/release"

cmake -S "$ProjectRoot" -B "$BuildDir" -G Ninja -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { exit 1 }

$CompileCommandsSrc = Join-Path $BuildDir "compile_commands.json"
$CompileCommandsDest = Join-Path $ProjectRoot "compile_commands.json"

if (Test-Path $CompileCommandsSrc) {
    Copy-Item $CompileCommandsSrc $CompileCommandsDest -Force
}

cmake --build "$BuildDir"
if ($LASTEXITCODE -ne 0) { exit 1 }
