# 检查 CMake 是否安装并在 PATH 中
Write-Host "正在检查 CMake..."

try {
    # 获取 CMake 版本，若 CMake 未安装或不在 PATH 中，将引发异常
    $cmakeVersion = cmake --version
    if (-not $cmakeVersion) {
        throw "CMake is not installed or not in PATH."
    }
}
catch {
    Write-Host "CMake 未安装或不在 PATH 中。"
    Pause
    exit 1
}

# 获取当前脚本的路径
$scriptDir = Get-Location

# 设置项目路径（假设项目根目录和脚本在同一目录层级）
$projectPath = $scriptDir.Path

# 检查项目路径是否存在 Tools 目录
$toolsDir = Join-Path $projectPath "Tools"
if (-not (Test-Path $toolsDir)) {
    Write-Host "目录 '$toolsDir' 不存在。"
    Pause
    exit 1
}

# 执行 CMake 命令，使用当前工作目录作为项目根目录
Write-Host "开始执行 CMake 命令..."

$cmakeCommand = "cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -S `"$projectPath`" -B `"$projectPath\build_VS2022`" -G `"Visual Studio 17 2022`" -T host=x64 -A x64"

try {
    # 执行 CMake 命令并捕获错误
    Invoke-Expression $cmakeCommand
}
catch {
    Write-Host "CMake 命令执行失败，错误信息: $_"
    Pause
    exit 1
}

Write-Host "CMake 命令执行成功。"
Pause
