@echo off
.\out\HW2.exe tree%1.txt >out.txt
code -d out.txt out%1.txt