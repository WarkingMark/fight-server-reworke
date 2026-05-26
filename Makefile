# modifed: igorpauk 2017-18

SOURCE_DIR = .

VPATH = $(SOURCE_DIR)

OSRV = vec.o debug.o common.o io.o srv.o srvcmd.o pers.o persmagic.o fight.o luaif.o luaif_tolua.o fightd.o
OCLI = vec.o debug.o common.o io.o fclient.o

BINS = fightd fclient

fightd : COMPILE_FLAGS = -ggdb3 -O0 -w -Wall
fightd : LIBS = -lpthread -lm -llua -ltolua++ -ldl

COMPILE = gcc $(DEFINES) -I. -I$(SOURCE_DIR) $(INCLUDES) $(COMPILE_FLAGS)
LINK    = gcc $(LINK_FLAGS) -o $@

.PHONY: all prepare-build clean

all: prepare-build $(BINS)

prepare-build:
	@-mkdir -p .deps

clean:
	-rm -f $(BINS) $(OSRV) $(OCLI)

-include .deps/*.Po

.SUFFIXES: .c .o .pkg

fightd: $(OSRV)
	@-rm -f fightd
	$(LINK) $(OSRV) $(LIBS)

fclient: $(OCLI)
	@-rm -f fclient
	$(LINK) $(OCLI) $(LIBS)

%.o: %.c
	@echo "$(COMPILE) -c -o $@ $<"; \
	$(COMPILE) -MT $@ -MD -MP -MF .deps/$*.Tpo -c -o $@ $< && \
	mv -f .deps/$*.Tpo .deps/$*.Po

%.c: %.pkg
	@echo "tolua++ -S -n $(*F) -o $(@F) $<"; \
	cur_dir=`pwd`; cd $(SOURCE_DIR) && \
	tolua++ -S -n $(*F) -o $$cur_dir/$(@F) $<

.INTERMEDIATE: luaif_tolua.c
