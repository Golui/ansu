
INCLUDE := include
SOURCE := src
#OBJECT := $(SOURCE:.c=.o)
BIN := bin

CPP := gcc

CPP_FLAGS := -MMD -MP -Wall -g -pedantic -I$(INCLUDE)

CFILES := $(shell find $(SOURCE) -name "*.c")
HFILES := $(shell find $(INCLUDE) -name "*.h")

OFILES := $(patsubst $(SOURCE)/%.c,$(BIN)/%.o, $(CFILES))

LD_FLAGS := $(LD_FLAGS)

all: ansu

$(BIN)/%.o: $(SOURCE)/%.c
	@-mkdir -p $(@D)
	$(CPP) $(CPP_FLAGS) -o $@ -c $<

ansu: $(OFILES)
	$(CPP) $(CPP_FLAGS) -o $@ $^ $(LD_FLAGS)

-include $(OFILES:.o=.d)

.PHONY: clean all format
clean:
	rm -rf ./$(BIN)/* ansu

format: $(CFILES)
	clang-format -i $^ $(HFILES)
