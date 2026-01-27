$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path "$PSScriptRoot\..\..").Path

Push-Location $ProjectRoot

try {
    Remove-Item "build" -Force -Recurse -ErrorAction SilentlyContinue
    Remove-Item ".cache" -Force -Recurse -ErrorAction SilentlyContinue
    Remove-Item "compile_commands.json" -Force -Recurse -ErrorAction SilentlyContinue
}
finally {
    Pop-Location
}
