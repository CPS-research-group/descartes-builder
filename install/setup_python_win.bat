@echo off
setlocal

:: ========================================
:: CONFIGURATION
:: ========================================
set PYTHON_VERSION=3.10.8
set INSTALL_DIR=%~dp0\python_win
set REPO_URL=https://github.com/CPS-research-group/kedro-umbrella
set REPO_ZIP=https://github.com/CPS-research-group/kedro-umbrella/archive/refs/heads/main.zip

echo.
echo ========================================
echo Installing Python %PYTHON_VERSION% to: %INSTALL_DIR%
echo ========================================

:: ========================================
:: Check if INSTALL_DIR exists
:: ========================================
if exist "%INSTALL_DIR%" (
    echo.
    echo The directory already exists: %INSTALL_DIR%
    set /p USER_CHOICE=Do you want to delete this directory and continue? (Y/N): 
    if /I "%USER_CHOICE%"=="Y" (
        echo Deleting directory...
        rmdir /S /Q "%INSTALL_DIR%"
    ) else (
        echo Installation cancelled by user.
        exit /b 1
    )
)

:: ========================================
:: Create directory
:: ========================================
echo.
echo Creating Embedded-Python directory at: %INSTALL_DIR%
mkdir %INSTALL_DIR%
cd /d %INSTALL_DIR%

:: ========================================
:: Download and extract embedded Python
:: ========================================
echo.
echo Downloading Embedded Python %PYTHON_VERSION% ...
curl -L -o python.zip https://www.python.org/ftp/python/%PYTHON_VERSION%/python-%PYTHON_VERSION%-embed-amd64.zip

echo Extracting Python...
powershell -Command "Expand-Archive -Path 'python.zip' -DestinationPath '%INSTALL_DIR%'"
del python.zip

:: ========================================
:: Set up pip
:: ========================================
echo.
echo Setting up Python pip...
curl -L -o get-pip.py https://bootstrap.pypa.io/get-pip.py
"%INSTALL_DIR%\python.exe" get-pip.py

:: ========================================
:: Modify ._pth file to enable packages
:: ========================================
for %%f in ("%INSTALL_DIR%\python*._pth") do (
    set "PTH_FILE=%%f"
)
if defined PTH_FILE (
    echo Adding site-packages to Python path...
    echo. >> "%PTH_FILE%"
    echo # Added by setup_python_win.bat >> "%PTH_FILE%"
    echo import site>> "%PTH_FILE%"
    echo Lib>> "%PTH_FILE%"
    echo Lib\site-packages>> "%PTH_FILE%"
) else (
    echo No ._pth file found, please check the Python installation.
    exit /b 1
)

:: ========================================
:: Download and install kedro_umbrella
:: ========================================
echo.
echo Downloading kedro-umbrella repository...
curl -L -o repo.zip %REPO_ZIP%

echo Extracting repository...
powershell -Command "Expand-Archive -Path 'repo.zip' -DestinationPath '%INSTALL_DIR%\repo'"
del repo.zip

set REPO_DIR=%INSTALL_DIR%\repo\kedro-umbrella-main
if not exist "%REPO_DIR%" (
    echo kedro-umbrella directory not found: %REPO_DIR%
    exit /b 1
)

echo.
echo Installing kedro_umbrella into embedded Python...
cd /d %REPO_DIR%
"%INSTALL_DIR%\python.exe" -m pip install .

echo.
echo ========================================
echo Python and kedro_umbrella setup complete!
echo ========================================

endlocal
pause
