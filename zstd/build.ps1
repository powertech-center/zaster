# build.ps1 - Builds libzstd.a for specified targets using Docker

param(
    [string[]] $Targets = @()
)

. "$PSScriptRoot/targets.ps1"

# Validate requested targets
$validNames = $AllTargets | ForEach-Object { $_.target }
foreach ($t in $Targets) {
    if ($validNames -notcontains $t) {
        Write-Error "Unknown target: '$t'. Valid targets are:`n$($validNames -join "`n")"
        exit 1
    }
}

$tmpDir      = "$PSScriptRoot/.tmp"
$buildScript = "$tmpDir/build.sh"

# Prepare .tmp directory: clear if exists, create fresh
if (Test-Path $tmpDir) {
    try {
        Remove-Item -Recurse -Force $tmpDir
    } catch {
        Write-Error "Failed to clear .tmp directory: $_"
        exit 1
    }
}
try {
    New-Item -ItemType Directory -Path $tmpDir | Out-Null
} catch {
    Write-Error "Failed to create .tmp directory: $_"
    exit 1
}

# Compute Docker mount path from repo root (handle Windows drive letters)
$repoRoot  = (Resolve-Path "$PSScriptRoot/..").Path
$mountPath = $repoRoot -replace '\\', '/'
if ($mountPath -match '^([A-Za-z]):(.*)') {
    $mountPath = "/$(($matches[1]).ToLower())$($matches[2])"
}

# Get target groups (filtered or all)
$groups = Get-TargetGroups $Targets

foreach ($group in $groups) {
    $image = $group.image

    # Generate build.sh for this group (LF line endings)
    $lines = @()
    $lines += "#!/bin/sh"
    $lines += "set -e"
    $lines += ""

    foreach ($t in $group.targets) {
        $objDir  = "/repo/zstd/.tmp/obj/$($t.target)"
        $outDir  = "/repo/prebuilt/$($t.target)"
        $outLib  = "$outDir/libzstd.a"
        $srcDirs = @("common", "compress", "decompress") | ForEach-Object { "/repo/zstd/lib/$_" }

        $lines += "# --- $($t.target) ---"
        $lines += "mkdir -p $objDir $outDir"

        foreach ($srcDir in $srcDirs) {
            $lines += "for f in $srcDir/*.c; do"
            $ccLine = "  $($t.cc) -c $($t.cflags) -I/repo/zstd/lib " + '$f -o ' + "$objDir/" + '$(basename $f .c).o'
            $lines += $ccLine
            $lines += "done"
        }

        # Compile .S assembly files in decompress if present
        $lines += 'for f in /repo/zstd/lib/decompress/*.S; do'
        $lines += '  [ -f "$f" ] || continue'
        $ccLineS = "  $($t.cc) -c $($t.cflags) " + '$f -o ' + "$objDir/" + '$(basename $f .S).o'
        $lines += $ccLineS
        $lines += 'done'

        $arLine = "$($t.ar) rcs $outLib $objDir/*.o"
        $lines += $arLine
        $lines += "echo 'Built: $outLib'"
        $lines += ""
    }

    $content = $lines -join "`n"
    [System.IO.File]::WriteAllBytes($buildScript, [System.Text.Encoding]::UTF8.GetBytes($content + "`n"))

    Write-Host "Running Docker image: $image"
    Write-Host "Targets: $($group.targets | ForEach-Object { $_.target })"

    docker run --rm `
        -v "${mountPath}:/repo" `
        $image `
        sh /repo/zstd/.tmp/build.sh

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Docker build failed for image: $image"
        Write-Host "Leaving .tmp for inspection."
        exit 1
    }
}

Remove-Item -Recurse -Force $tmpDir
Write-Host "Done."
