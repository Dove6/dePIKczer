@echo off
rem USTAW PONI¯EJ ODPOWIEDNIE ŒCIE¯KI
rem PRZESZUKIWANY FOLDER I FOLDER WYJŒCIOWY MO¯ESZ USTAWIÆ TE¯ W KONSOLI, PODAJ¥C JE JAKO ARGUMENTY

set depikczer=bezwzglêdna œcie¿ka do dePIKczer.exe
set przeszukiwany_folder=bezwglêdna œcie¿ka przeszukiwanego folderu
set folder_wyjsciowy=bezwglêdna œcie¿ka folderu wyjœciowego



set opcja_out_dir=/o \"%folder_wyjsciowy%\"
if not [%1]==[] (set przeszukiwany_folder=%1
if not [%2]==[] (set folder_wyjsciowy=%2) else (set folder_wyjsciowy=)
)
if "%folder_wyjsciowy%"=="" (set opcja_out_dir=) else (set opcja_out_dir=/o \"%folder_wyjsciowy%\")
forfiles /p %przeszukiwany_folder% /m *.img /s /c "%depikczer% /s %opcja_out_dir% @path"