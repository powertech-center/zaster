# build.ps1 - Builds the zaster CLI binary for specified targets using Docker

param(
    [string[]] $Targets = @()
)

. "$PSScriptRoot/../zstd/targets.ps1"

# CLI target mapping: friendly name -> full target triple
$CliTargetMap = [ordered]@{
    "linux-x64"     = "x86_64-linux-musl"
    "linux-arm64"   = "aarch64-linux-musl"
    "macos-x64"     = "x86_64-apple-darwin"
    "macos-arm64"   = "aarch64-apple-darwin"
    "windows-x64"   = "x86_64-windows-msvc"
    "windows-arm64" = "aarch64-windows-msvc"
}

# Resolve CLI target names to full target triples
$resolvedTargets = @()
if ($Targets.Count -gt 0) {
    foreach ($t in $Targets) {
        if ($CliTargetMap.Contains($t)) {
            $resolvedTargets += $CliTargetMap[$t]
        } elseif ($AllTargets | Where-Object { $_.target -eq $t }) {
            # Allow passing full target names directly
            $resolvedTargets += $t
        } else {
            $validNames = @($CliTargetMap.Keys) + @($AllTargets | ForEach-Object { $_.target })
            Write-Error "Unknown target: '$t'. Valid targets are:`n$($validNames -join "`n")"
            exit 1
        }
    }
} else {
    $resolvedTargets = @($CliTargetMap.Values)
}

# Validate all resolved targets exist in AllTargets
$validNames = $AllTargets | ForEach-Object { $_.target }
foreach ($t in $resolvedTargets) {
    if ($validNames -notcontains $t) {
        Write-Error "Target '$t' is not defined in targets.ps1"
        exit 1
    }
}

# Reverse map: full target -> CLI name (for output directory)
$ReverseMap = @{}
foreach ($key in $CliTargetMap.Keys) {
    $ReverseMap[$CliTargetMap[$key]] = $key
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

# Get target groups (filtered by resolved targets)
$groups = Get-TargetGroups $resolvedTargets

foreach ($group in $groups) {
    $image = $group.image

    # Generate build.sh for this group (LF line endings)
    $lines = @()
    $lines += "#!/bin/sh"
    $lines += "set -e"
    $lines += ""

    foreach ($t in $group.targets) {
        $cliName = $ReverseMap[$t.target]
        if (-not $cliName) { continue } # skip targets not in CLI map

        $objDir = "/repo/cli/.tmp/obj/$($t.target)"
        $outDir = "/repo/dist/$cliName"
        $srcDir = "/repo/cli"
        $libDir = "/repo/libs/$($t.target)"

        # Determine output binary name
        $isWin = $t.target -match "windows"
        $binName = if ($isWin) { "zaster.exe" } else { "zaster" }

        $lines += "# --- $cliName ($($t.target)) ---"
        $lines += "mkdir -p $objDir $outDir"

        # Compile .c files
        $lines += "for f in $srcDir/*.c; do"
        $ccLine = "  $($t.cc) -c $($t.cflags) " + '$f -o ' + "$objDir/" + '$(basename $f .c).o'
        $lines += $ccLine
        $lines += "done"

        # Link with libzaster.a and libzstd.a
        $ldLine = "$($t.cc) -o $outDir/$binName $objDir/*.o $libDir/libzaster.a $libDir/libzstd.a"
        if (-not $isWin) {
            $ldLine += " -lpthread"
        }
        if ($t.target -match "linux") {
            $ldLine += " -static"
        }
        $lines += $ldLine
        $lines += "echo 'Built: $outDir/$binName'"
        $lines += ""
    }

    $content = $lines -join "`n"
    [System.IO.File]::WriteAllBytes($buildScript, [System.Text.Encoding]::UTF8.GetBytes($content + "`n"))

    Write-Host "Running Docker image: $image"
    Write-Host "Targets: $($group.targets | ForEach-Object { $_.target })"

    docker run --rm `
        -v "${mountPath}:/repo" `
        $image `
        sh /repo/cli/.tmp/build.sh

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Docker build failed for image: $image"
        Write-Host "Leaving .tmp for inspection."
        exit 1
    }
}

Remove-Item -Recurse -Force $tmpDir
Write-Host "Done."
