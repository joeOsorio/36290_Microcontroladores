@echo off
setlocal
cd /d "%~dp0.."
set "LOG=%~dp0claude_push_log.txt"
echo ==== %date% %time% > "%LOG%"
git --version >> "%LOG%" 2>&1
echo --- REMOTES --- >> "%LOG%"
git remote -v >> "%LOG%" 2>&1
for /f "delims=" %%b in ('git rev-parse --abbrev-ref HEAD') do set "BR=%%b"
echo BRANCH=%BR% >> "%LOG%"
echo --- STATUS --- >> "%LOG%"
git status --short >> "%LOG%" 2>&1
git config user.email >nul 2>&1 || git config user.email "joshua.osorio@uabc.edu.mx"
git config user.name >nul 2>&1 || git config user.name "Joshua Osorio Osorio"
echo --- ARCHIVOS BORRADOS LOCALMENTE (NO se suben como borrados) --- >> "%LOG%"
git ls-files --deleted >> "%LOG%" 2>&1
git add --ignore-removal -- . >> "%LOG%" 2>&1
echo --- STAGED --- >> "%LOG%"
git diff --cached --name-status >> "%LOG%" 2>&1
git diff --cached --diff-filter=D --quiet
if errorlevel 1 (
  echo ABORT: hay borrados en staging, no se hace commit >> "%LOG%"
  git reset -q >> "%LOG%" 2>&1
  goto end
)
git diff --cached --quiet
if not errorlevel 1 (
  echo NOTHING_TO_COMMIT >> "%LOG%"
  goto push
)
git commit -F "%~dp0claude_msg.txt" >> "%LOG%" 2>&1
if errorlevel 1 (
  echo COMMIT_FAILED >> "%LOG%"
  goto end
)
:push
echo --- PUSH --- >> "%LOG%"
git push -u origin %BR% >> "%LOG%" 2>&1
echo PUSH_EXIT=%errorlevel% >> "%LOG%"
:end
echo --- LOG FINAL --- >> "%LOG%"
git log --oneline -3 >> "%LOG%" 2>&1
git status -sb >> "%LOG%" 2>&1
echo DONE >> "%LOG%"
echo Listo. Puedes cerrar esta ventana.
timeout /t 15
