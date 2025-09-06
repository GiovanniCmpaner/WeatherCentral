cd "%USERPROFILE%\.platformio\packages\tool-esptoolpy"
espefuse.py -p COM6 set_flash_voltage 3.3V
pause