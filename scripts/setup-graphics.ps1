$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
$dependencies = @(
    @{ Name = 'raylib'; Url = 'https://github.com/raysan5/raylib.git'; Tag = '5.5'; Commit = 'c1ab645ca298a2801097931d1079b10ff7eb9df8' },
    @{ Name = 'raygui'; Url = 'https://github.com/raysan5/raygui.git'; Tag = '4.0'; Commit = '25c8c65a6e5f0f4d4b564a0343861898c6f2778b' }
)
foreach ($dependency in $dependencies) {
    $target = Join-Path '.deps' $dependency.Name
    if (-not (Test-Path -LiteralPath $target)) {
        git clone --depth 1 --branch $dependency.Tag $dependency.Url $target
        if ($LASTEXITCODE -ne 0) { throw 'Dependency clone failed' }
    }
    $actual = git -C $target rev-parse HEAD
    if ($LASTEXITCODE -ne 0 -or $actual -ne $dependency.Commit) {
        throw "Unexpected version in $target. Expected $($dependency.Commit)."
    }
}
Write-Host 'raylib 5.5 and raygui 4.0 ready. Run make CC=gcc visualizer.'
