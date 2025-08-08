pushd %~dp0
pyinstaller --onefile ProcedureGenerator.py
MOVE .\dist\ProcedureGenerator.exe .\GenProcs.exe
@RD /S /Q .\build
@RD /S /Q .\dist
DEL /S /F /Q .\ProcedureGenerator.spec
PAUSE