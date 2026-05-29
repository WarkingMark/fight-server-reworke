# fightd Makefile
# Supports: lua5.1 (production), lua5.4 (fallback)
# Usage:
#   make              — build with lua5.1 (default, production)
#   make LUA=5.4      — build with lua5.4
#   make test         — build and run unit tests (no Lua needed)
#   make clean

SOURCE_DIR = .
VPATH = $(SOURCE_DIR)

LUA     ?= 5.
LUA_INC ?= /usr/include/lua$(LUA)
LUA_LIB ?= lua$(LUA)
TOLUA   ?= tolua++$(LUA)

INCLUDES = -I$(LUA_INC)

OSRV = vec.o debug.o common.o io.o srv.o srvcmd.o pers.o persmagic.o \
       fight.o luaif.o luaif_tolua.o fightd.o

OCLI = vec.o debug.o common.o io.o fclient.o

BINS = fightd fclient

fightd : COMPILE_FLAGS = -ggdb3 -O0 -w -Wall
fightd : LIBS         = -L/usr/lib -lpthread -lm -l$(LUA_LIB) -l$(TOLUA) -ldl

COMPILE = gcc $(DEFINES) -I. -I$(SOURCE_DIR) $(INCLUDES) $(COMPILE_FLAGS)
LINK    = gcc $(LINK_FLAGS) -o $@

.PHONY: all prepare-build clean test check-lua

all: check-lua prepare-build $(BINS)

check-lua:
	@if [ ! -f "$(LUA_INC)/lua.h" ]; then \
		echo "ERROR: lua.h not found at $(LUA_INC)"; \
		echo "Install: apt install liblua$(LUA)-dev libtolua++$(LUA)-dev"; \
		echo "Or run:  make LUA=5.4"; \
		exit 1; \
	fi
	@echo "Lua $(LUA) found at $(LUA_INC)"

prepare-build:
	@-mkdir -p .deps

clean:
	-rm -f $(BINS) $(OSRV) $(OCLI) fight_emulator

# Standalone emulator — no Lua dependency
test: fight_emulator
	./fight_emulator --seed 42

fight_emulator: fight_emulator.c
	gcc -O2 -lm -o fight_emulator fight_emulator.c
	@echo "Emulator built. Run: ./fight_emulator [--verbose] [--rounds N]"

-include .deps/*.Po

.SUFFIXES: .c .o .pkg

fightd: $(OSRV)
	@-rm -f fightd
	$(LINK) $(OSRV) $(LIBS)

fclient: $(OCLI)
	@-rm -f fclient
	$(LINK) $(OCLI) $(LIBS)

%.o: %.c
	@mkdir -p .deps
	@echo "$(COMPILE) -c -o $@ $<"; \
	$(COMPILE) -MT $@ -MD -MP -MF .deps/$*.Tpo -c -o $@ $< && \
	mv -f .deps/$*.Tpo .deps/$*.Po

%.c: %.pkg
	@echo "tolua++ -S -n $(*F) -o $(@F) $<"; \
	cur_dir=`pwd`; cd $(SOURCE_DIR) && \
	tolua++ -S -n $(*F) -o $$cur_dir/$(@F) $<

.INTERMEDIATE: luaif_tolua.c
