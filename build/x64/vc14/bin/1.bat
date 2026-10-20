mkdir borland
for %%F in (*.dll) do implib -a borland\%%~nF.lib %%F
pause