# Sync the authoritative FundamentalUpdater_rev5 CSV to FundamentalWeb.
# This script does NOT modify FundamentalUpdater_rev5 or Task Scheduler.
#
# Run after FundamentalUpdater_rev5 finishes updating:
#   powershell -ExecutionPolicy Bypass -File .\tools\sync-rev5-csv.ps1
#
# Optional:
#   -RepoPath "C:\path\to\FundamentalWeb"

[CmdletBinding()]
param(
    [string]$RepoPath = "\\wsl.localhost\Ubuntu\home\thanimwas\FundamentalWeb"
)

$ErrorActionPreference = "Stop"

$source = "C:\Program Files\FundamentalUpdater_rev5\Data\Fundamental\fundamental_v4.csv"
$target = Join-Path $RepoPath "data\fundamental_v4.csv"

if (-not (Test-Path -LiteralPath $source)) {
    throw "Source CSV not found: $source"
}

if (-not (Test-Path -LiteralPath $RepoPath)) {
    throw "FundamentalWeb repository not found: $RepoPath"
}

$gitDir = Join-Path $RepoPath ".git"
if (-not (Test-Path -LiteralPath $gitDir)) {
    throw "Not a Git repository: $RepoPath"
}

$rows = @(Import-Csv -LiteralPath $source)

if ($rows.Count -eq 0) {
    throw "Source CSV is empty."
}

$headers = @(
    "symbol","last","percentChange","volume","value","marketCap",
    "pe","pbv","deRatio","dps","eps","roa","roe",
    "netProfitMargin","dividendYield","bookValuePerShare","listedShare"
)

$actualHeaders = @($rows[0].PSObject.Properties.Name)

if (($actualHeaders -join ",") -ne ($headers -join ",")) {
    throw "CSV header does not contain the expected 17 Fundamental V4 fields."
}

$aot = $rows | Where-Object { $_.symbol -eq "AOT" } | Select-Object -First 1

if ($null -eq $aot) {
    throw "AOT was not found in the source CSV."
}

Write-Host "Source: $source"
Write-Host "Rows:   $($rows.Count)"
Write-Host "AOT:    last=$($aot.last), percentChange=$($aot.percentChange), volume=$($aot.volume)"

Copy-Item -LiteralPath $source -Destination $target -Force

Push-Location $RepoPath
try {
    $changed = git diff --quiet -- data/fundamental_v4.csv
    if ($LASTEXITCODE -eq 0) {
        Write-Host "No CSV change. Nothing to commit."
        exit 0
    }

    git add -- data/fundamental_v4.csv
    git commit -m "Sync Fundamental V4 CSV from installed Rev5"
    git push

    if ($LASTEXITCODE -ne 0) {
        throw "git push failed."
    }

    Write-Host "SYNC COMPLETE."
    Write-Host "Render will deploy automatically from the new GitHub commit."
}
finally {
    Pop-Location
}
