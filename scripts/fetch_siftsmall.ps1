<#
.SYNOPSIS
    Downloads the siftsmall dataset used by the Phase 0 Recall@100 validation.

.DESCRIPTION
    Fetches siftsmall.tar.gz from the TEXMEX corpus and extracts it into
    data/siftsmall/ at the repository root. The data/ directory is gitignored.
    Re-running is a no-op once the three required files are present and the
    right size; pass -Force to download again.
#>
[CmdletBinding()]
param([switch]$Force)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$dataDir = Join-Path $repoRoot 'data'
$destDir = Join-Path $dataDir 'siftsmall'
$archive = Join-Path $dataDir 'siftsmall.tar.gz'
$url = 'ftp://ftp.irisa.fr/local/texmex/corpus/siftsmall.tar.gz'

# Sizes follow from the .fvecs/.ivecs layout (4-byte dimension + 4 bytes per
# element), so a wrong size means a truncated or wrong download.
$expected = [ordered]@{
    'siftsmall_base.fvecs'        = 5160000   # 10000 * (4 + 128*4)
    'siftsmall_query.fvecs'       = 51600     # 100   * (4 + 128*4)
    'siftsmall_groundtruth.ivecs' = 40400     # 100   * (4 + 100*4)
}

function Test-Dataset {
    foreach ($name in $expected.Keys) {
        $path = Join-Path $destDir $name
        if (-not (Test-Path -LiteralPath $path)) { return $false }
        if ((Get-Item -LiteralPath $path).Length -ne $expected[$name]) { return $false }
    }
    return $true
}

if ((Test-Dataset) -and -not $Force) {
    Write-Host "siftsmall already present in $destDir - nothing to do."
    exit 0
}

New-Item -ItemType Directory -Force -Path $dataDir | Out-Null

Write-Host "Downloading $url ..."
try {
    # Invoke-WebRequest in Windows PowerShell 5.1 does not speak FTP.
    $client = New-Object System.Net.WebClient
    $client.DownloadFile($url, $archive)
} catch {
    Write-Error @"
Download failed: $($_.Exception.Message)

If FTP is blocked on this network, download siftsmall.tar.gz manually from
http://corpus-texmex.irisa.fr/ and extract it so the files land in:
  $destDir
"@
    exit 1
}

Write-Host "Extracting into $dataDir ..."
& tar -xzf $archive -C $dataDir
if ($LASTEXITCODE -ne 0) { Write-Error "tar failed with exit code $LASTEXITCODE"; exit 1 }

Remove-Item -LiteralPath $archive -Force

foreach ($name in $expected.Keys) {
    $path = Join-Path $destDir $name
    if (-not (Test-Path -LiteralPath $path)) { Write-Error "missing after extraction: $path"; exit 1 }
    $size = (Get-Item -LiteralPath $path).Length
    if ($size -ne $expected[$name]) {
        Write-Error "$name is $size bytes, expected $($expected[$name])"
        exit 1
    }
    Write-Host ("  ok  {0,-28} {1,10:N0} bytes" -f $name, $size)
}

Write-Host "siftsmall ready in $destDir"
