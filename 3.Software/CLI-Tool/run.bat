@echo off
if not defined TAG (
    set TAG=1
    start wt -p "cmd" %0
    exit
)

 
ipython "%~dp0\run_shell.py" -- %*