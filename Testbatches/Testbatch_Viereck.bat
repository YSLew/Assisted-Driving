REM Test-Batch: Vergleicht automatisiert alle Testbilder

@echo off

cd "C:\Users\Max\OneDrive\HTW\Master\SE Projekt\SE-Projekt Bildersammlung 08.06.2015 small\Viereck"

REM Form: BildverarbeitungOpenCV [Szene.jpg] [Quelle.jpg]

for /L %%i IN (1 1 122) do start /wait "Search" ""Bildverarbeitung_Release.exe Viereck_%%i.JPG Viereck_Klein.JPG""
