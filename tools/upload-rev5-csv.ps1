# Upload the authoritative FundamentalUpdater_rev5 CSV directly to FundamentalWeb.
# This does NOT modify FundamentalUpdater_rev5 or GitHub.
#
# One-time setup on this PC:
#   [Environment]::SetEnvironmentVariable("FUNDAMENTALWEB_UPLOAD_TOKEN","<YOUR TOKEN>","User")
#
# Then run:
#   powershell -ExecutionPolicy Bypass -File "\\wsl.localhost\Ubuntu\home\thanimwas\FundamentalWeb\tools\upload-rev5-csv.ps1"

[CmdletBinding()]
param(
    [string]$Source = "C:\Program Files\FundamentalUpdater_rev5\Data\Fundamental\fundamental_v4.csv",
    [string]$ApiUrl = "https://fundamentalweb-backend.onrender.com/api/upload-csv"
)

$ErrorActionPreference = "Stop"

$token = [Environment]::GetEnvironmentVariable("FUNDAMENTALWEB_UPLOAD_TOKEN","User")
if ([string]::IsNullOrWhiteSpace($token)) { throw "FUNDAMENTALWEB_UPLOAD_TOKEN is not set for this Windows user." }
if (-not (Test-Path -LiteralPath $Source)) { throw "Source CSV not found: $Source" }

Write-Host "Uploading: $Source"
Write-Host "Target:    $ApiUrl"
$response = Invoke-RestMethod -Uri $ApiUrl -Method Post -InFile $Source -ContentType "text/csv; charset=utf-8" -Headers @{ "X-Upload-Token" = $token }
Write-Host "Server response:"
$response | ConvertTo-Json -Compress
