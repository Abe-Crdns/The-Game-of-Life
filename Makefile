EXECBIN   := gol
CC 	  := g++
CCFLAGS   := -I/opt/X11/include 
LD 	  := g++
LDFLAGS   := -L/opt/X11/lib -lX11

MODULES   := lpc_lib/private lpc_lib game_of_life/private game_of_life main					 
SRC_DIR   := $(addprefix src/, $(MODULES))
BUILD_DIR := $(addprefix build/, $(MODULES))

SRC       := $(foreach sdir, $(SRC_DIR), $(wildcard $(sdir)/*.cpp))
OBJECTS   := $(patsubst src/%.cpp, build/%.o, $(SRC))
INCLUDES  := $(addprefix -I, $(SRC_DIR))

vpath %.cpp $(SRC_DIR)

define make-goal
$1/%.o: %.cpp
	$(CC) $(CCFLAGS) $(INCLUDES) -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs build/$(EXECBIN)

build/$(EXECBIN): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@


checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_DIR)

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))
