
INCLUDE := include
SOURCE := src
LIB := lib
#OBJECT := $(SOURCE:.c=.o)
BIN := bin

CPP := /usr/local/opt/llvm/bin/clang++

CPP_FLAGS := -MMD -MP -Wall -g -pedantic -std=c++11 -Wno-unknown-pragmas \
			 -I$(INCLUDE) -I$(LIB)/CLI11/include

ifndef BACKEND
	BACKEND := SOFTWARE
endif

CPP_FLAGS := $(CPP_FLAGS) -D$(BACKEND)

CFILES := $(shell find $(SOURCE) -name "*.cpp" ! -path "$(SOURCE)/backend/*")
HFILES := $(shell find $(INCLUDE) -name "*.hpp" ! -path "$(INCLUDE)/backend/*")

ifeq "$(BACKEND)" "SOFTWARE"
	override CFILES += $(shell find $(SOURCE)/backend/software -name "*.cpp")
	override HFILES += $(shell find $(INCLUDE)/backend/software -name "*.hpp")
endif

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
