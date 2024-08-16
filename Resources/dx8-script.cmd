@echo off
setlocal enabledelayedexpansion

if "%~1"=="" (
    echo Usage: %~nx0 input_file output_file
    exit /b 1
)

set "input_file=%~1"
set "output_file=%~2"

set "search=D3d8to9"

if not exist "%input_file%" (
    echo Input file not found: %input_file%
    exit /b 1
)

(for /f "tokens=1,* delims=]" %%a in ('find /n /v "" ^< "%input_file%"') do (
    set "line=%%b"
    if defined line (
        set "line=!line: =!"
        if "!line:~0,7!"=="%search%" (
            echo %search%                    = 1
        ) else if not "!line!"=="" (
            echo %%b
        )
    ) else (
        echo.
    )
)) > "%output_file%"

echo Output file created: %output_file%
