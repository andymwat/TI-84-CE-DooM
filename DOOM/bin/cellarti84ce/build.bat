for /F %%a in ('wmic os get LocalDateTime /value ^| findstr Local') do set "DATETIME=%%a"
set "DATETIME=%DATETIME:~14,17%"
md "old\%DATETIME%"
copy *.c "old\%DATETIME%"
copy *.asm "old\%DATETIME%"
copy Makefile "old\%DATETIME%"
copy *.bat "old\%DATETIME%"
copy *.py "old\%DATETIME%"
copy *.png "old\%DATETIME%"
copy *.txt "old\%DATETIME%"

headers.py
%cedev%\bin\make
del cellar-ti84ce.zip
7za a -bd -tzip cellar-ti84ce.zip *.8xp *.c *.bat *.asm *.png *.py cellar.txt Makefile > nul