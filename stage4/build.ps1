param(
    [ValidatePattern('^[0-9]{14}$')][string]$State = '21345671111111',
    [ValidateSet('V1', 'V2')][string]$Version = 'V1',
    [switch]$Render
)
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    & gcc -O2 -std=c99 -Wall -Wextra -Wpedantic gen_asm.c ..\stage3\tables.c -o gen_asm.exe
    if ($LASTEXITCODE -ne 0) { throw 'Assembly table exporter compilation failed.' }
    & .\gen_asm.exe | Set-Content -LiteralPath tables.s -Encoding Ascii
    if ($LASTEXITCODE -ne 0) { throw 'Assembly table export failed.' }
    $searchFile = if ($Version -eq 'V2') { 'search_v2.s' } else { 'search.s' }
    $main = Get-Content -LiteralPath main.s -Raw
    if ($Render) {
        $main = $main.Replace('    bnez a0, invalid_input', "    bnez a0, invalid_input`n    jal ra, render_cube")
        $main = $main.Replace('    jal ra, apply_move', "    jal ra, apply_move`n    jal ra, render_cube")
    }
    $parts = @(
        ".data`ndata_begin:`ninput: .asciz `"$State`""
        $main
        (Get-Content -LiteralPath $searchFile -Raw)
    )
    if ($Render) { $parts += ".equ FRAME_DELAY, 1000000`n" + (Get-Content -LiteralPath renderer.s -Raw) }
    $parts += @(
        ".text`ntext_end:"
        (Get-Content -LiteralPath tables.s -Raw)
        ".data`ndata_end: .byte 0"
    )
    $output = if ($Render) { 'gui.s' } else { 'cli.s' }
    $parts -join "`n" | Set-Content -LiteralPath $output -Encoding Ascii
} finally {
    Pop-Location
}
