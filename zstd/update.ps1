# update.ps1 - Downloads and vendors zstd lib/ sources into this directory

# Detect if launched by double-click (Windows Explorer as parent process)
$launchedByExplorer = $false
if ($IsWindows) {
    try {
        $parentPid = (Get-CimInstance Win32_Process -Filter "ProcessId=$PID").ParentProcessId
        $parentName = (Get-CimInstance Win32_Process -Filter "ProcessId=$parentPid").Name
        if ($parentName -eq "explorer.exe") {
            $launchedByExplorer = $true
        }
    } catch {
        # Ignore errors, assume terminal launch
    }
}

function Wait-IfNeeded {
    if ($launchedByExplorer) {
        Write-Host ""
        Write-Host "Press any key to exit..." -NoNewline
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    }
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$versionFile = Join-Path $scriptDir "VERSION"
$libDir = Join-Path $scriptDir "lib"

# Fetch latest stable zstd version from GitHub
Write-Host "Fetching latest zstd version..."
try {
    $response = Invoke-RestMethod -Uri "https://api.github.com/repos/facebook/zstd/releases/latest" -Headers @{ "User-Agent" = "update.ps1" }
    $latestVersion = $response.tag_name -replace '^v', ''
    Write-Host "Latest stable zstd version: $latestVersion"
} catch {
    Write-Error "Failed to fetch latest version: $_"
    Wait-IfNeeded
    exit 1
}

# Check current version
if (Test-Path $versionFile) {
    $currentVersion = (Get-Content $versionFile -Raw).Trim()
    if ($currentVersion -eq $latestVersion) {
        Write-Host "Already up to date (version $currentVersion)."
        Wait-IfNeeded
        exit 0
    }
    Write-Host "Updating from $currentVersion to $latestVersion..."
} else {
    Write-Host "No VERSION file found, installing version $latestVersion..."
}

# Remove existing lib directory
if (Test-Path $libDir) {
    Write-Host "Removing existing lib directory..."
    try {
        Remove-Item -Recurse -Force $libDir
    } catch {
        Write-Error "Failed to remove lib directory: $_"
        Wait-IfNeeded
        exit 1
    }
    if (Test-Path $libDir) {
        Write-Error "lib directory still exists after removal attempt."
        Wait-IfNeeded
        exit 1
    }
}

# Download release archive
$archiveUrl = "https://github.com/facebook/zstd/archive/refs/tags/v$latestVersion.zip"
$tmpZip = Join-Path ([System.IO.Path]::GetTempPath()) "zstd-$latestVersion.zip"
$tmpDir = Join-Path ([System.IO.Path]::GetTempPath()) "zstd-$latestVersion-extract"

Write-Host "Downloading $archiveUrl..."
try {
    Invoke-WebRequest -Uri $archiveUrl -OutFile $tmpZip -UseBasicParsing
} catch {
    Write-Error "Failed to download archive: $_"
    Wait-IfNeeded
    exit 1
}

# Extract archive
Write-Host "Extracting archive..."
try {
    if (Test-Path $tmpDir) { Remove-Item -Recurse -Force $tmpDir }
    Expand-Archive -Path $tmpZip -DestinationPath $tmpDir -Force
} catch {
    Write-Error "Failed to extract archive: $_"
    Wait-IfNeeded
    exit 1
}

# Copy lib/ from extracted archive
$extractedLib = Join-Path $tmpDir "zstd-$latestVersion" "lib"
if (-not (Test-Path $extractedLib)) {
    Write-Error "lib directory not found in extracted archive (expected: $extractedLib)."
    Wait-IfNeeded
    exit 1
}

Write-Host "Copying lib directory..."
try {
    Copy-Item -Recurse -Force $extractedLib $libDir
} catch {
    Write-Error "Failed to copy lib directory: $_"
    Wait-IfNeeded
    exit 1
}

# Cleanup temp files
Remove-Item -Force $tmpZip -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force $tmpDir -ErrorAction SilentlyContinue

# Update VERSION file
Write-Host "Updating VERSION file..."
try {
    Set-Content -Path $versionFile -Value $latestVersion -NoNewline
} catch {
    Write-Error "Failed to write VERSION file: $_"
    Wait-IfNeeded
    exit 1
}

Write-Host "Done. zstd version $latestVersion is ready in lib/."
Wait-IfNeeded
