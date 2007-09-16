call setvals.bat
net stop %NAME%
bin\Release\ServiceInstaller.exe UNINSTALL %NAME%
pause