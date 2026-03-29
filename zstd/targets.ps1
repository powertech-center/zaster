# targets.ps1 - Build target definitions for zstd (and other components via dot-sourcing)

# Architecture tuning flags
$MARCH_X64  = "-march=x86-64-v3"
$MTUNE_X64  = "-mtune=alderlake"
$MCPU_ARM64 = "-mcpu=neoverse-n1+nodotprod+nofp16+norcpc+noras+nossbs"

# Default compiler flags
$CFLAGS_DEFAULT = "-O3 -DNDEBUG -fPIC -fvisibility=hidden"

# Arch-specific compiler flags (extend the default)
$CFLAGS_X64         = "$CFLAGS_DEFAULT $MARCH_X64 $MTUNE_X64"
$CFLAGS_ARM64       = "$CFLAGS_DEFAULT $MCPU_ARM64"
$CFLAGS_ARM64_APPLE = "$CFLAGS_DEFAULT -mcpu=apple-m1"

# Windows targets use clang-cl driver — flags passed via /clang: prefix
$CFLAGS_X64_WINDOWS   = "/clang:-O3 /clang:-DNDEBUG /clang:$MARCH_X64 /clang:$MTUNE_X64"
$CFLAGS_ARM64_WINDOWS = "/clang:-O3 /clang:-DNDEBUG /clang:$MCPU_ARM64"

# Windows GNU targets use plain clang but don't support -fPIC/-fvisibility
$CFLAGS_X64_WINDOWS_GNU   = "-O3 -DNDEBUG $MARCH_X64 $MTUNE_X64"
$CFLAGS_ARM64_WINDOWS_GNU = "-O3 -DNDEBUG $MCPU_ARM64"

# Default Docker image for cross-compilation
$DEFAULT_IMAGE = "ghcr.io/powertech-center/alpine/cross-clang"

class Target {
    [string] $target
    [string] $image
    [string] $cc
    [string] $cflags
    [string] $ld
    [string] $ldflags
    [string] $ar

    Target(
        [string] $target,
        [string] $image,
        [string] $cc,
        [string] $cflags,
        [string] $ld,
        [string] $ldflags,
        [string] $ar
    ) {
        $this.target  = $target
        $this.image   = $image
        $this.cc      = $cc
        $this.cflags  = $cflags
        $this.ld      = $ld
        $this.ldflags = $ldflags
        $this.ar      = $ar
    }
}

function New-Target {
    param(
        [Parameter(Mandatory)] [string] $target,
        [string] $image   = $DEFAULT_IMAGE,
        [string] $cc      = "",
        [string] $cflags  = $CFLAGS_DEFAULT,
        [string] $ld      = "",
        [string] $ldflags = "",
        [string] $ar      = "llvm-ar"
    )

    if ($cc -eq "")  { $cc  = "clang-$target" }
    if ($ld -eq "")  { $ld  = "lld-$target" }

    return [Target]::new($target, $image, $cc, $cflags, $ld, $ldflags, $ar)
}

# All build targets
$AllTargets = @(
    (New-Target "x86_64-linux-musl"   -cflags $CFLAGS_X64),
    (New-Target "x86_64-linux-gnu"    -cflags $CFLAGS_X64),
    (New-Target "aarch64-linux-musl"  -cflags $CFLAGS_ARM64),
    (New-Target "aarch64-linux-gnu"   -cflags $CFLAGS_ARM64),
    (New-Target "x86_64-apple-darwin" -cflags $CFLAGS_X64),
    (New-Target "aarch64-apple-darwin" -cflags $CFLAGS_ARM64_APPLE),
    (New-Target "x86_64-windows-msvc" -cflags $CFLAGS_X64_WINDOWS),
    (New-Target "aarch64-windows-msvc" -cflags $CFLAGS_ARM64_WINDOWS),
    (New-Target "x86_64-windows-gnu"  -cflags $CFLAGS_X64_WINDOWS_GNU),
    (New-Target "aarch64-windows-gnu" -cflags $CFLAGS_ARM64_WINDOWS_GNU)
)

# Returns targets grouped by image, optionally filtered by target name list.
# Usage: Get-TargetGroups         -> all targets, grouped
#        Get-TargetGroups @("x86_64-linux-musl", ...) -> filtered, grouped
function Get-TargetGroups {
    param(
        [string[]] $filter = @()
    )

    $selected = if ($filter.Count -gt 0) {
        $AllTargets | Where-Object { $filter -contains $_.target }
    } else {
        $AllTargets
    }

    $selected | Group-Object -Property image | ForEach-Object {
        @{
            image   = $_.Name
            targets = @($_.Group)
        }
    }
}
