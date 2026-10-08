@echo off
cd /d "%~dp0"
echo ========================================
echo        VINOTH MART - STARTING
echo ========================================
where java >nul 2>nul
if errorlevel 1 (
  echo Java was not found. Install JDK 17 and add it to PATH.
  pause
  exit /b 1
)
where mvn >nul 2>nul
if errorlevel 1 (
  echo Maven was not found. Install Maven 3.6.3+ and add it to PATH.
  pause
  exit /b 1
)
call mvn spring-boot:run
pause
