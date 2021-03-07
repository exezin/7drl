# compiler
CC     =gcc
# ---

# dirs
ODIR   =obj
BDIR   =build
_IDIRS =lib inc lib/physfs lib/MojoAL lib/SDL2 render
IDIRS  =$(patsubst %,-I%/,$(_IDIRS))
BIN    =game
# ---

# flags
CFLAGS =-O3 -lm -ldl -std=c99 -lGL -lSDL2 -I. $(IDIRS)
CFLAGS+=-g -Wall -Wno-unused
# ---

# deps
SRCDIR =*.c */*.c */*/*.c
HDIR   =*.h */*.h */*/*.h
DEPS   =$(filter-out lib/*,$(wildcard $(HDIR)))
LDEPS  =$(wildcard lib/*/.c lib/*/*/.c)

_OBJ   =$(patsubst %.c,%.o,$(wildcard $(SRCDIR)))
OBJ    =$(addprefix $(ODIR)/, $(notdir $(_OBJ)))
# ---

# windows
ifeq ($(OS),Windows_NT)
CC    =x86_64-w64-mingw32-gcc
LD    =x86_64-w64-mingw32-ld
FLAGS   =-Wl,-Bstatic -g -lm -static -static-libgcc -static-libstdc++ -lstdc++ \
-Llib/win -lopengl32 -lmingw32 -Wl,-Bdynamic -lSDL2main -lSDL2 -I. $(IDIRS)
FLAGS  +=-Llib/win/dll
#
CFLAGS  =$(FLAGS)
CFLAGS +=-std=c99 -DAL_LIBTYPE_STATIC
BNAME   =game.exe
endif
# ---

# macos
UNAME  = $(shell uname -s)
ifeq ($(UNAME),Darwin)
CFLAGS =-O3 -g -lm -framework OpenGl -framework Foundation \
-framework IOKit -lSDL2 \
-I. $(IDIRS) -Wno-unused-command-line-argument
CFLAGS+=-std=c99 -Wall -Wno-unused
OBJ   += $(ODIR)/physfs_platform_apple.o
endif
# ---

# rules
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: render/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: input/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: util/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# libs
$(ODIR)/%.o: lib/physfs/%.c $(LDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ifeq ($(UNAME),Darwin)
$(ODIR)/%.o: lib/physfs/%.m $(LDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
endif
# ---

# main
all: files game

game: $(OBJ)
	$(CC) -o $(BDIR)/$(BIN) $^ $(CFLAGS)
ifeq ($(OS),Windows_NT)
	cp lib/win/dll/*.dll $(BDIR)/
endif
# ---

# make files etc
files:
	mkdir -p $(ODIR)
	mkdir -p $(BDIR)
	(zip -ur $(BDIR)/data.dat data || true)
# ---

# util
.PHONY: clean

clean:
	rm -rf $(ODIR)
# ---
