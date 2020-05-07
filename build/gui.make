# makefile archivarius gui version #

ifndef target
	target=x86
endif

CC = cl.exe
LINKER = link.exe

CCFLAGS = \
	/I../code/include \
	/DTAA_ALLOCATOR \
	/DTAA_ARCHIVARIUS_COMPRESS \
	/DTAA_ARCHIVARIUS_THREAD \
	/DTAA_ARCHIVARIUS_GUI_NATIVE \
	/DTAA_NDEBUG \
	/D_WIN32_WINNT=0x0502 \
	/DUNICODE /D_UNICODE \
	/nologo /MT /O2 /w

LIBFLAGS = /nologo

OBJ := $(filter-out Main.obj, $(patsubst %.cpp,%.obj, $(notdir $(shell dir /b /s ..\code\source\*.cpp))))

CPPATH = \
	../code/source/Main \
	../code/source/Common \
	../code/source/Compress \
	../code/source/FileManager \
	../code/source/Widget

HPPATH = \
	../code/source/Common \
	../code/source/Compress \
	../code/include/Common/container

OBJP_BASE := obj\gui

BIN = ..\bin
BIN_NAME = ArchivariusGui.exe
OBJP := $(OBJP_BASE)\$(target)
OBJC = $(addprefix $(OBJP)/,$(OBJ)) 
MD = @if not exist $(OBJP) md $(OBJP)
MDB = @if not exist $(BIN) md $(BIN)
LINK := Psapi.lib Gdi32.lib Comdlg32.lib User32.lib Comctl32.lib Shell32.lib Advapi32.lib
RES = gui.res
RESC = $(OBJP)\$(RES)

vpath %.cpp $(CPPATH)
vpath %.asm $(CPPATH)
vpath %.hpp $(HPPATH)
vpath %.obj $(OBJP)
vpath %.exe $(BIN)
vpath %.res $(OBJP)
vpath %.rc res

all: $(BIN_NAME)

$(BIN_NAME): $(OBJ) $(RES)
	@echo.
	$(MDB)
	@echo Linking $@ ($(target))
	$(LINKER) $(LIBFLAGS) /out:$(BIN)/$(BIN_NAME) $(OBJC) $(LINK) $(RESC)

$(OBJ): %.obj: %.cpp
	$(MD)
	$(CC) $(CCFLAGS) /Fo$(OBJP)/$@ /c $<

$(RES): %.res: %.rc
	rc.exe /nologo /Fo$(OBJP)/$@ $<

LzreCoder.obj: LzreThread.hpp
Allocator.obj: MemoryPage.hpp

clean:
	@echo.
	@echo Cleaning Archivarius Gui $(target)
	@if exist $(OBJP_BASE) rd /s /q $(OBJP_BASE)