REM Test-Batch: Vergleicht automatisiert alle Testbilder

@echo off

cd "C:\Users\Max\OneDrive\HTW\Master\SE Projekt\SE-Projekt Bildersammlung 08.06.2015 small\Dreieck"

REM Form: BildverarbeitungOpenCV [Szene.jpg] [Quelle.jpg]
REM oder: Bildverarbeitung_Release.exe [Szene.jpg]

for /L %%i IN (1 1 114) do start /wait "Search" ""Bildverarbeitung_Release.exe Dreieck_%%i.JPG""
