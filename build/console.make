# makefile archivarius console version #

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
	/DTAA_NDEBUG \
	/DUNICODE /D_UNICODE \
	/nologo /MT /w /O2

LIBFLAGS = /nologo

OBJ := Main.obj
OBJ += $(filter-out Registry.obj Vector2i.obj, $(patsubst %.cpp,%.obj, $(notdir $(wildcard ../code/source/Common/*.cpp))))
OBJ += $(patsubst %.cpp,%.obj, $(notdir $(wildcard ../code/source/Compress/*.cpp)))

CPPATH = \
	../code/source/Main \
	../code/source/Common \
	../code/source/Compress

HPPATH = \
	../code/source/Common \
	../code/source/Compress \
	../code/include/Common/container

OBJP_BASE := obj\console

BIN = ..\bin
BIN_NAME = Archivarius.exe
OBJP := $(OBJP_BASE)\$(target)
OBJC = $(addprefix $(OBJP)/,$(OBJ))
MD = @if not exist $(OBJP) md $(OBJP)
MDB = @if not exist $(BIN) md $(BIN)
LINK := Psapi.lib Gdi32.lib Comdlg32.lib User32.lib Shell32.lib
RES = console.res
RESC = $(OBJP)\$(RES)

vpath %.cpp $(CPPATH)
vpath %.hpp $(HPPATH)
vpath %.h $(HPPATH)
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

LzreCoder.obj: LzreThread.hpp AvlTree.h RingBuffer.h
Allocator.obj: MemoryPage.hpp

clean:
	@echo.
	@echo Cleaning Archivarius $(target)
	@if exist $(OBJP_BASE) rd /s /q $(OBJP_BASE)