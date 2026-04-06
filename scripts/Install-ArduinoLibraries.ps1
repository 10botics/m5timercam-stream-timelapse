# Copies bundled Arduino libraries from this repo to the user sketchbook libraries folder.
# Run from repo root: .\scripts\Install-ArduinoLibraries.ps1

$ErrorActionPreference = 'Stop'

$repoRoot   = Split-Path -Parent $PSScriptRoot
$sourceDir  = Join-Path $repoRoot 'arduino_libraries'
$destParent = Join-Path $env:USERPROFILE 'Documents\Arduino\libraries'

if (-not (Test-Path -LiteralPath $sourceDir)) {
    Write-Error "Missing folder: $sourceDir"
}

New-Item -ItemType Directory -Path $destParent -Force | Out-Null

Get-ChildItem -LiteralPath $sourceDir -Directory | ForEach-Object {
    $name = $_.Name
    $src  = $_.FullName
    $dst  = Join-Path $destParent $name
    Write-Host "Installing: $name -> $dst"
    Copy-Item -LiteralPath $src -Destination $destParent -Recurse -Force
}

Write-Host ""
Write-Host "Done. Restart Arduino IDE if it was open, then open m5cam_stream_server and compile."
Write-Host "Next: teaching/INSTALL_SIMPLE.md or teaching/INSTALL.md"
