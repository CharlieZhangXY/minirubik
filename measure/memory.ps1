param(
    [string]$RipesPath = 'C:\Users\user\Desktop\Ripes-v2.2.6-106-g5b8a616-win-x86_64\Ripes.exe'
)
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $RipesPath -PathType Leaf)) {
    throw 'Ripes.exe not found. Specify its location with -RipesPath.'
}
$source = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'mem.s') -Raw
$pattern = '(?m)^\s*\.equ BYTES,\s*\d+\s*$'
if ([regex]::Matches($source, $pattern).Count -ne 1) {
    throw 'Expected one .equ BYTES declaration in mem.s.'
}
$temporarySource = Join-Path $PSScriptRoot ('.mem-' + [guid]::NewGuid().ToString('N') + '.s')
try {
    $results = foreach ($bytes in @(65536, 1048576)) {
        $source -replace $pattern, ".equ BYTES, $bytes" |
            Set-Content -LiteralPath $temporarySource -Encoding Ascii
        $arguments = '--mode cli --src "{0}" -t asm --proc RV32_ISS --timeout 10000' -f $temporarySource
        $process = Start-Process -FilePath $RipesPath -ArgumentList $arguments -WindowStyle Hidden -PassThru
        $null = $process.Handle
        $peak = 0L
        while (-not $process.HasExited) {
            $process.Refresh()
            $peak = [Math]::Max($peak, $process.PeakWorkingSet64)
            Start-Sleep -Milliseconds 50
        }
        if ($process.ExitCode -ne 0 -or $peak -eq 0) {
            throw "Measurement failed for $bytes bytes (exit code $($process.ExitCode))."
        }
        $process.Dispose()
        [pscustomobject]@{ GuestBytes = $bytes; PeakHostBytes = $peak }
    }
} finally {
    if (Test-Path -LiteralPath $temporarySource) {
        Remove-Item -LiteralPath $temporarySource
    }
}
$ratio = ($results[1].PeakHostBytes - $results[0].PeakHostBytes) / (1048576 - 65536)
$report = @(
    'Model: RV32_ISS; host metric: observed PeakWorkingSet64 (bytes)'
    'GuestBytes  PeakHostBytes'
    foreach ($row in $results) { '{0,-11} {1}' -f $row.GuestBytes, $row.PeakHostBytes }
    ('Host bytes per guest byte: {0:F2}' -f $ratio)
    ('Estimated additional host memory for baseline (MiB): {0:F2}' -f ($ratio * 18405414 / 1MB))
)
if ($ratio -le 0) {
    $report += 'The memory difference is not positive; repeat the measurement before drawing conclusions.'
}
$report | Tee-Object -FilePath (Join-Path $PSScriptRoot 'memory-result.txt')
