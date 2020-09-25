# rawdrawandroid Docker image Windows helper script

[CmdletBinding(PositionalBinding = $false)]

param
(
    # Switch: build image
    [switch]$Build,

    # Switch: remove rawdrawandroid Docker images
    [switch]$Clean,

    # Switch: execute command in running container
    [switch]$Exec,

    # Switch: run container and execute command
    [switch]$Run,

    # Switch: start container and leave it running in background
    [switch]$Start,

    # Switch: stop container running in background
    [switch]$Stop,

    # Image name
    [ValidateNotNullOrEmpty()]
    [string]$ImageName = 'rawdrawandroid',

    # Image tag
    [ValidateNotNullOrEmpty()]
    [string]$ImageTag = '0.0.1',

    # Remaining arguments
    [string[]]
    [Parameter(ValueFromRemainingArguments)]
    $Passthrough
)

$ErrorActionPreference = 'Stop'

# Path to adb executable
# !! EDIT THIS according to your system adb location !!
$CmdAdb = 'c:\Users\YOUR_USERNAME\AppData\Local\Android\Sdk\platform-tools\adb'

# Path to Docker executable on your system
$CmdDocker = 'docker'

$Cwd = Get-Location

function Build() {
    # Build Docker image
    $argDocker = 
    'build',
    '--pull',
    '-t', "${ImageName}:${ImageTag}",
    '-f', "./Dockerfile",
    '.'
    & $CmdDocker $argDocker
}

function Run() {
    # Run Docker container
    # - make sure host ADB server is running
    & $CmdAdb start-server
    # - run container
    $argDocker = 
    'run',
    '-it',
    '--rm',
    '--name', "${ImageName}",
    '--network', 'host',
    '--volume', "${Cwd}:/src",
    '--workdir', '/src',
    "${ImageName}:${ImageTag}"
    & $CmdDocker $argDocker @Passthrough
}

function StartBg() {
    # Start container and leave it running in background
    $argDocker = 
    'run',
    '-d',
    '-t',
    '--rm',
    '--name', "${ImageName}",
    '--network', 'host',
    '--volume', "${Cwd}:/src",
    '--workdir', '/src',
    '--entrypoint', '/bg.sh',
    "${imageName}:${imageTag}"
    & $CmdDocker $argDocker
}

function StopBg() {
    # Stop container running in background
    $argDocker = 
    'stop',
    "${ImageName}"
    & $CmdDocker $argDocker
}

function Exec() {
    # Run a command in a running container
    # - make sure host ADB server is running
    & $CmdAdb start-server
    # - execute command
    $argDocker = 
    'exec',
    '-t',
    "${ImageName}"
    & $CmdDocker $argDocker @Passthrough
}

function Clean() {
    # Remove rawdrawandroid Docker images
    $ImageNameRegEx = '\b$ImageName\b'
    docker images | Select-String -Pattern $ImageNameRegEx |
    ForEach-Object {
        $iName = $_.Line.split(" ", [System.StringSplitOptions]::RemoveEmptyEntries)[0];
        $iTag = $_.Line.split(" ", [System.StringSplitOptions]::RemoveEmptyEntries)[1];
        Write-Host 'Removing image ${iName}:${iTag}';
        docker rmi ${iName}:${iTag} --force
    }
}

if ($Build) {
    Build
}

if ($Clean) {
    Clean
}

if ($Exec) {
    Exec
}

if ($Start) {
    StartBg
}

if ($Stop) {
    StopBg
}

if ($Run) {
    Run
}

