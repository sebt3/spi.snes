BLD_DBG := YES
RM      := rm -f
MKDIR   := mkdir -p
LEDITOR := kate


# set build and target directory based on plateform
TARGET    := ../target
BLDD      := build
THEME     := theme
DRAW      := draw
FILTR     := filters
DB        := db
DRAWING   := $(DRAW)
PACKER    := packer
INTERFACE := interface
DBSRC     := data/snes.xml.d
THEMEDIR  := theme_data

PL_LDFLAGS=-lasound
ifneq "$(findstring DPANDORA,$(CFLAGS))" ""
	BLDD   = build.pand
	TARGET = ../target.pand
	DRAWING+=egl
	PL_LDFLAGS+=-lGLESv2 -lEGL -lX11
	CFLAGS+=-Wno-uninitialized
else 
	CFLAGS+=-Wno-unused-but-set-variable
	PL_LDFLAGS+=-lX11 -lGL
endif

ifeq ($(BLD_DBG), YES)
	CFLAGS += -g -DDEBUG
else
	CFLAGS += -O3
endif
CFLAGS += -DUSE_SHADER -DUSE_RENDER_THREAD
THEMEFILE := $(TARGET)/theme.eet
DBSFILE   := $(TARGET)/db_small.eet
DBMFILE   := db_medium.eet

TIME      := time -p
# no timing
TIME      :=


SOURCES   := $(FILTER) $(THEME) $(DRAWING) $(PACKER) $(INTERFACE) $(DB)

LIB_DIR   := $(TARGET)/lib
INC_DIR   := $(TARGET)/include
BIN_DIR   := $(TARGET)/bin

EXE_INTF  := $(BIN_DIR)/$(INTERFACE)
EXE_PCK   := $(BIN_DIR)/$(INTERFACE).pack
BLDDFILTR := $(BLDD)/$(FILTR)
BLDDTHEME := $(BLDD)/$(THEME)
BLDDPCK   := $(BLDD)/$(PACKER)
BLDDRAW   := $(BLDD)/$(DRAW)
BLDDB     := $(BLDD)/$(DB)
BLDDINT   := $(BLDD)/$(INTERFACE)
ifneq "$(findstring DPANDORA,$(CFLAGS))" ""
	OBJFILTR := $(foreach dir, $(FILTR),   $(patsubst $(dir)/%.S,  $(BLDDFILTR)/%.o, $(wildcard $(dir)/*.S))) 
else 
	OBJFILTR :=
endif

COUNTER   := $(BLDD)/.cnt.sh
OBJTHEME  := $(foreach dir, $(THEME),   $(patsubst $(dir)/%.c,  $(BLDDTHEME)/%.o,$(wildcard $(dir)/*.c))) 
OBJFILTR  += $(foreach dir, $(FILTR),   $(patsubst $(dir)/%.c,  $(BLDDFILTR)/%.o,$(wildcard $(dir)/*.c))) 
OBJPCK    := $(foreach dir, $(PACKER),  $(patsubst $(dir)/%.c,  $(BLDDPCK)/%.o,  $(wildcard $(dir)/*.c))) 
OBJDRAW   := $(foreach dir, $(DRAWING), $(patsubst $(dir)/%.c,  $(BLDDRAW)/%.o,  $(wildcard $(dir)/*.c))) 
OBJDB     := $(foreach dir, $(DB),      $(patsubst $(dir)/%.c,  $(BLDDB)/%.o,    $(wildcard $(dir)/*.c))) 
OBJINT    := $(foreach dir, $(INTERFACE), $(patsubst $(dir)/%.c,  $(BLDDINT)/%.o,$(wildcard $(dir)/*.c))) $(foreach dir, $(INTERFACE), $(patsubst $(dir)/%.cpp,  $(BLDDINT)/%.o,$(wildcard $(dir)/*.cpp)))
SRCFILES  := $(foreach dir, $(THEMEDIR), $(wildcard $(dir)/*.txt) $(wildcard $(dir)/*.frm) $(wildcard $(dir)/*.frms)) $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c) $(wildcard $(dir)/*.cpp) $(wildcard $(dir)/*.h))
OBJS      := $(OBJTHEME) $(OBJDRAW) $(OBJFILTR) $(OBJINT) $(OBJPCK) $(OBJDB)

JUNK    := $(shell find . -name '*~')

LDFLAGS += $(foreach dir, $(LIB_DIR), -L$(wildcard $(dir))) -lm  $(shell sdl-config --libs) -lSDL_ttf -lSDL_image $(shell pkg-config --libs eet eina) -ldl
CFLAGS  += $(shell pkg-config --cflags eet eina) $(shell sdl-config --cflags) -Wall
CFLAGS  += $(foreach dir, $(INC_DIR), -I$(wildcard $(dir)))
IDRAW   += $(foreach dir, $(DRAWING), -I$(wildcard $(dir)))
SHOW_BLD_FILTR = printf "[%3d%%] \033[32mBuilding filter file \033[1m%s\033[0m\n"  `$(COUNTER)`
SHOW_BLD_THEME = printf "[%3d%%] \033[32mBuilding theme  file \033[1m%s\033[0m\n"  `$(COUNTER)`
SHOW_BLD_DRAW = printf "[%3d%%] \033[32mBuilding  draw  file \033[1m%s\033[0m\n"  `$(COUNTER)`
SHOW_BLD_DB  = printf "[%3d%%] \033[32mBuilding   db   file \033[1m%s\033[0m\n"  `$(COUNTER)`
SHOW_BLD_PCK = printf "[%3d%%] \033[32mBuilding packer file \033[1m%s\033[0m\n"  `$(COUNTER)`
SHOW_BLD_INT = printf "[%3d%%] \033[32mBuilding  apps  file \033[1m%s\033[0m\n"  `$(COUNTER)`
SHOW_BLD_EMU = printf "[%3d%%] \033[32mBuilding snes9x file \033[1m%s\033[0m\n"  `$(COUNTER)`
SHOW_LNK     = printf "[%3d%%] \033[34mLinking binary \033[34;1m%s\033[0m\n"  `$(COUNTER)`
SHOW_SUCESS  = printf "[%3d%%] \033[33;1mSuccessfully build target \033[0;33m%s\033[0m\n"  `$(COUNTER)`
SHOW_STARTED = printf "[%3s%%] \033[33;1mEditor started\033[0m\n"  "---"
SHOW_GAMESTT = printf "[%3s%%] \033[33;1mGame starting\033[0m\n"  "---"
SHOW_PACKING = printf "[%3s%%] \033[33;1mPacking the data \033[0;33m%s\033[0m\n"  "---"


### Emulator specific
EMU       := snes9x
EMUDIR    := snes9x snes9x/apu snes9x/jma snes9x/unzip 
BLDDEMU   := $(BLDD)/$(EMU)
OBJEMU    := $(BLDDEMU)/apu.o $(BLDDEMU)/SNES_SPC.o $(BLDDEMU)/SNES_SPC_misc.o $(BLDDEMU)/SNES_SPC_state.o $(BLDDEMU)/SPC_DSP.o $(BLDDEMU)/SPC_Filter.o $(BLDDEMU)/bsx.o $(BLDDEMU)/c4.o $(BLDDEMU)/c4emu.o $(BLDDEMU)/cheats.o $(BLDDEMU)/cheats2.o $(BLDDEMU)/clip.o $(BLDDEMU)/conffile.o $(BLDDEMU)/controls.o $(BLDDEMU)/cpu.o $(BLDDEMU)/cpuexec.o $(BLDDEMU)/cpuops.o $(BLDDEMU)/crosshairs.o $(BLDDEMU)/dma.o $(BLDDEMU)/dsp.o $(BLDDEMU)/dsp1.o $(BLDDEMU)/dsp2.o $(BLDDEMU)/dsp3.o $(BLDDEMU)/dsp4.o $(BLDDEMU)/fxinst.o $(BLDDEMU)/fxemu.o $(BLDDEMU)/gfx.o $(BLDDEMU)/globals.o $(BLDDEMU)/logger.o $(BLDDEMU)/memmap.o $(BLDDEMU)/movie.o $(BLDDEMU)/obc1.o $(BLDDEMU)/ppu.o $(BLDDEMU)/reader.o $(BLDDEMU)/sa1.o $(BLDDEMU)/sa1cpu.o $(BLDDEMU)/screenshot.o $(BLDDEMU)/sdd1.o $(BLDDEMU)/sdd1emu.o $(BLDDEMU)/seta.o $(BLDDEMU)/seta010.o $(BLDDEMU)/seta011.o $(BLDDEMU)/seta018.o $(BLDDEMU)/snapshot.o $(BLDDEMU)/snes9x.o $(BLDDEMU)/spc7110.o $(BLDDEMU)/srtc.o $(BLDDEMU)/tile.o
OBJEMU    += $(BLDDEMU)/loadzip.o $(BLDDEMU)/ioapi.o $(BLDDEMU)/unzip.o
OBJEMU    += $(BLDDEMU)/7zlzma.o $(BLDDEMU)/crc32.o $(BLDDEMU)/iiostrm.o $(BLDDEMU)/inbyte.o $(BLDDEMU)/jma.o $(BLDDEMU)/lzma.o $(BLDDEMU)/lzmadec.o $(BLDDEMU)/s9x-jma.o $(BLDDEMU)/winout.o
EMUFLAGS  := -O3 -fomit-frame-pointer -fno-exceptions -fno-rtti -Wall -W -Wno-unused-parameter -DZLIB -DUNZIP_SUPPORT -DJMA_SUPPORT -DHAVE_LIBPNG -DHAVE_MKSTEMP -DHAVE_STRINGS_H -DHAVE_SYS_IOCTL_H -DHAVE_STDINT_H -DRIGHTSHIFT_IS_SAR
EMUFLAGS  += $(foreach dir, $(EMUDIR), -I$(wildcard $(dir)))
EMULDFLAGS:= -lz -lpng
#ifneq "$(findstring DPANDORA,$(CFLAGS))" ""
#	EMUFLAGS+= -DPANDORA -DHAVE_GLES 
#endif


### global
bin: directories $(OBJS) $(BLDDEMU) $(OBJEMU) $(EXE_INTF) $(EXE_PCK)
	@$(SHOW_SUCESS) bin

all: bin $(THEMEFILE) $(DBSFILE) $(DBMFILE)
	@$(SHOW_SUCESS) all

directories: $(INC_DIR) $(BIN_DIR) $(LIB_DIR) $(BLDD) $(COUNTER) $(BLDDB) $(BLDDTHEME) $(BLDDFILTR) $(BLDDRAW) $(BLDDPCK) $(BLDDINT) 

runtest: bin $(THEMEFILE)
	@$(EXE_INTF)

edit:
	@$(LEDITOR) Makefile $(SRCFILES) >/dev/null 2>&1 &
	@$(SHOW_STARTED)

### Emulator specific
$(BLDDEMU)/%.o: $(EMU)/%.cpp
	@$(SHOW_BLD_EMU) $<
	@$(CXX) $(EMUFLAGS) $(CFLAGS) -c $< -o $@

$(BLDDEMU)/%.o: $(EMU)/%.c
	@$(SHOW_BLD_EMU) $<
	@$(CC) $(EMUFLAGS) $(CFLAGS) -c $< -o $@

$(BLDDEMU)/%.o: $(EMU)/unzip/%.c
	@$(SHOW_BLD_EMU) $<
	@$(CC) $(EMUFLAGS) $(CFLAGS) -c $< -o $@

$(BLDDEMU)/%.o: $(EMU)/filter/%.cpp
	@$(SHOW_BLD_EMU) $<
	@$(CXX) $(EMUFLAGS) $(CFLAGS) -c $< -o $@

$(BLDDEMU)/%.o: $(EMU)/filter/%.c
	@$(SHOW_BLD_EMU) $<
	@$(CC) $(EMUFLAGS) $(CFLAGS) -c $< -o $@

$(BLDDEMU)/%.o: $(EMU)/jma/%.cpp
	@$(SHOW_BLD_EMU) $<
	@$(CXX) $(EMUFLAGS) $(CFLAGS) -fexceptions -c $< -o $@

$(BLDDEMU)/%.o: $(EMU)/apu/%.cpp
	@$(SHOW_BLD_EMU) $<
	@$(CXX) $(EMUFLAGS) $(CFLAGS) -c $< -o $@


### Interface
$(EXE_PCK):$(OBJTHEME) $(OBJDB) $(OBJPCK)
	@$(SHOW_LNK) $@
	@$(CC) $(OBJTHEME) $(OBJPCK) $(OBJDB) -o $(EXE_PCK) $(LDFLAGS) $(PL_LDFLAGS)

$(BLDDPCK)/%.o: $(PACKER)/%.c
	@$(SHOW_BLD_PCK) $<
	@$(CC) -I$(DB) -I$(THEME) -I$(PACKER) $(CFLAGS) -c $< -o $@

$(EXE_INTF): $(OBJTHEME) $(OBJFILTR) $(OBJDRAW) $(OBJDB) $(OBJINT) $(OBJEMU)
	@$(SHOW_LNK) $@
	@$(CXX) $(OBJTHEME) $(OBJFILTR) $(OBJDRAW) $(OBJINT) $(OBJDB)  $(OBJEMU) -o $(EXE_INTF) $(LDFLAGS) $(PL_LDFLAGS) $(EMULDFLAGS)

$(BLDDINT)/%.o: $(INTERFACE)/%.c $(INTERFACE)/*.h $(DRAW)/*.h  $(THEME)/*.h $(FILTR)/*.h
	@$(SHOW_BLD_INT) $<
	@$(CC) -I$(DB) -I$(THEME) -I$(FILTR) $(IDRAW) -I$(INTERFACE) $(CFLAGS) -c $< -o $@

$(BLDDINT)/%.o: $(INTERFACE)/%.cpp $(INTERFACE)/*.h
	@$(SHOW_BLD_INT) $<
	@$(CXX) $(EMUFLAGS) -I$(DB) -I$(THEME) $(IDRAW) -I$(INTERFACE) $(CFLAGS) -c $< -o $@

$(BLDDRAW)/%.o: $(DRAW)/%.c $(THEME)/*.h $(DRAW)/*.h $(FILTR)/*.h
	@$(SHOW_BLD_DRAW) $<
	@$(CC) $(CFLAGS) -I$(THEME) -I$(FILTR) $(IDRAW) -c $< -o $@

$(BLDDB)/%.o: $(DB)/%.c $(DB)/*.h
	@$(SHOW_BLD_DB) $<
	@$(CC) $(CFLAGS) -I$(DB) -I$(THEME) -c $< -o $@

$(BLDDRAW)/%.o: egl/%.c egl/*.h
	@$(SHOW_BLD_DRAW) $<
	@$(CC) $(CFLAGS) -DUSE_GLES2 -DUSE_EGL_SDL -c $< -o $@

$(BLDDTHEME)/%.o: $(THEME)/%.c $(THEME)/*.h
	@$(SHOW_BLD_THEME) $<
	@$(CC) $(CFLAGS) -I$(THEME) -c $< -o $@

$(BLDDFILTR)/%.o: $(FILTR)/%.c $(FILTR)/*.h
	@$(SHOW_BLD_FILTR) $<
	@$(CC) $(CFLAGS) -I$(FILTR) -c $< -o $@

$(BLDDFILTR)/%.o: $(FILTR)/%.S $(FILTR)/*.h
	@$(SHOW_BLD_FILTR) $<
	@$(CC) $(CFLAGS) -I$(FILTR) -c $< -o $@

$(THEMEFILE): $(THEMEDIR)/* $(EXE_PCK)
	@$(SHOW_PACKING) $@
	@rm -f $(THEMEFILE)
	@$(TIME) $(EXE_PCK) -t $(THEMEFILE) $(THEMEDIR)

$(DBSFILE): $(DBSRC)/* $(EXE_PCK)
	@$(SHOW_PACKING) $@
	@rm -f $(DBSFILE)
	@$(TIME) $(EXE_PCK) -s $(DBSFILE) $(DBSRC)

$(DBMFILE): $(DBSRC)/* $(EXE_PCK)
	@$(SHOW_PACKING) $@
	@rm -f $(DBMFILE)
	@$(TIME) $(EXE_PCK) -m $(DBMFILE) $(DBSRC)


$(COUNTER): Makefile $(SRCFILES) $(BLDD)
	@echo "#!/bin/sh">$@
	@echo 'expr 100 "*" `ls -1d $(BLDDEMU)/*.o $(BLDDTHEME)/*.o $(BLDDFILTR)/*.o $(BLDDPCK)/*.o $(BLDDB)/*.o $(BLDDINT)/*.o $(BLDDRAW)/*.o $(EXE_PCK) $(EXE_INTF) 2>/dev/null|wc -l` "/" `echo $(OBJS) $(OBJEMU) $(EXE_PCK) $(EXE_INTF)|wc -w`' >> $@
	@chmod 700 $@

$(INC_DIR):
	@$(MKDIR) $(INC_DIR)
$(BIN_DIR):
	@$(MKDIR) $(BIN_DIR)
$(LIB_DIR):
	@$(MKDIR) $(LIB_DIR)
$(BLDD):
	@$(MKDIR) $(BLDD)
$(BLDDEMU): $(BLDD)
	@$(MKDIR) $(BLDDEMU)
$(BLDDB): $(BLDD)
	@$(MKDIR) $(BLDDB)
$(BLDDRAW): $(BLDD)
	@$(MKDIR) $(BLDDRAW)
$(BLDDPCK): $(BLDD)
	@$(MKDIR) $(BLDDPCK)
$(BLDDINT): $(BLDD)
	@$(MKDIR) $(BLDDINT)
$(BLDDTHEME): $(BLDD)
	@$(MKDIR) $(BLDDTHEME)
$(BLDDFILTR): $(BLDD)
	@$(MKDIR) $(BLDDFILTR)

clean:
	@$(RM) $(OBJS) $(EXE_ED) $(EXE_INTF) $(JUNK) $(BLDD)/.cnt.sh

fullclean: clean
	@$(RM) -r $(BLDD)

.PHONY:clean all runtest edit bin directories fullclean
.PRECIOUS: $(OBJS)
.SUFFIXES: .o .c .h .S .cpp .eet
