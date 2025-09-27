@echo off

:: First check if Docker is running
docker ps >NUL 2>NUL
if %errorlevel% neq 0 (
  echo.
  echo Docker engine is not running! Run Docker Desktop application
  echo.
  exit /b
)

:: Check if os_build_custom container exists and whether one should be created or started
docker container inspect os_build_custom >NUL 2>NUL
if %errorlevel% neq 0 (
  echo Haven't found container named os_build_custom
  echo Creating a new container from image Romanko2024/os_build_custom:0.1. Dir %CD% will be mounted to /src
  docker run -i -d --name os_build_custom -v %CD%:/src Romanko2024/os_build_custom:0.1
) else (
  docker start os_build_custom >NUL
)

:: Building kernel
echo Building kernel in the os_build_custom container
docker exec os_build_custom bash -l -c "cd /src && make"
if %errorlevel% neq 0 (
  echo.
  echo Compilation failed
  echo.
  exit /b
)

:: Check if QEMU is available in PATH
where qemu-system-i386 >NUL 2>NUL
if %errorlevel% == 0 (
    :: Running kernel in QEMU emulator
    qemu-system-i386 -kernel build/kernel.bin
    exit /b 0
)

:: Try default Program Files location for QEMU
SET qemu_path_candidate=C:\Program Files\qemu\qemu-system-i386.exe

if exist "%qemu_path_candidate%" (
    :: Running kernel in QEMU emulator
    "%qemu_path_candidate%" -kernel build/kernel.bin
) else (
    echo .
    echo "Cannot find QEMU. Have you installed it?"
    echo .
    exit /b
)
