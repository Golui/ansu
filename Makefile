
INCLUDE := include
SOURCE := src
#OBJECT := $(SOURCE:.c=.o)
BIN := bin

CPP := /usr/local/opt/llvm/bin/clang++

CPP_FLAGS := -MMD -MP -Wall -g -pedantic -std=c++14 -Wno-unknown-pragmas -DNO_VIVADO -I$(INCLUDE)

CFILES := $(shell find $(SOURCE) -name "*.cpp")
HFILES := $(shell find $(INCLUDE) -name "*.hpp")

OFILES := $(patsubst $(SOURCE)/%.cpp,$(BIN)/%.o, $(CFILES))

LD_FLAGS := $(LD_FLAGS)

all: ansu

$(BIN)/%.o: $(SOURCE)/%.cpp
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
