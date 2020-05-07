ifndef target
	target=x86
endif
export target

PROJECT = console gui

.PHONY: $(PROJECT)
all: $(PROJECT)

$(PROJECT):
	@make -j2 -s -f $@.make
	@echo.

help:
	@echo Usage:
	@echo   make [targets]
	@echo.
	@echo Targets:
	@echo   all (default)
	@echo   console
	@echo   gui
	@echo   run
	@echo   clean

clean:
	@if exist obj rd /s /q obj
	@if exist ..\bin\Notification rd /s /q ..\bin\Notification

run: console
	@cd ../bin & starte.bat

rung: gui
	@cd ../bin & ArchivariusGui.exe
