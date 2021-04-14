
INCLUDE := include
SOURCE := src
LIB := ../lib
SCHEMA := schema
BIN := bin

CLIBS := microtar
LIBS := $(CLIBS) CLI11 ansu

LINCLUDES := $(foreach lib, $(LIBS),-I $(LIB)/$(lib)/include)
LSEARCH := $(foreach lib, $(CLIBS),-L $(LIB)/$(lib))
LFLAGS := $(foreach lib, $(CLIBS),-l$(lib))

LINCLUDES += -I ../include
LSEARCH += -L ../out
LFLAGS += -lansu

CPP := g++
C := gcc

ifndef BUILD_PLATFORM
	BUILD_PLATFORM := "unknown"
	ifeq "$(OS)" "Windows_NT"
		BUILD_PLATFORM := win
	else
		UNAME_S := $(shell uname -s)
		# macOS
		ifeq "$(UNAME_S)" "Darwin"
			BUILD_PLATFORM := macOS
		endif
		ifeq "$(UNAME_S)" "Linux"
			BUILD_PLATFORM := linux
		endif
	endif
endif

ifeq "$(BUILD_PLATFORM)" "macOS"
	ifneq ("$(wildcard /usr/local/opt/llvm/.)","")
		CPP := /usr/local/opt/llvm/bin/clang++
		C := /usr/local/opt/llvm/bin/clang
	endif
endif

INCLUDE_SEARCH := -I $(INCLUDE) $(LINCLUDES)

CPP_FLAGS := -MMD -MP -Wall -g -pedantic -std=c++11 -Wno-unknown-pragmas \
			 $(INCLUDE_SEARCH)

ifndef BACKEND
	BACKEND := SOFTWARE
endif

CPP_FLAGS := $(CPP_FLAGS) -D$(BACKEND)

CFILES := $(shell find $(SOURCE) -name "*.cpp")
HFILES := $(shell find $(INCLUDE) -name "*.hpp" -or -name "*.h")

OFILES := $(patsubst $(SOURCE)/%.cpp,$(BIN)/%.o, $(CFILES))

LD_FLAGS := $(LD_FLAGS) $(LSEARCH) $(LFLAGS)

all: ansu_cli

$(BIN)/%.o: $(SOURCE)/%.cpp
	@-mkdir -p $(@D)
	$(CPP) $(CPP_FLAGS) -o $@ -c $<

ansu_cli: $(OFILES)
	
	$(CPP) $(CPP_FLAGS) -o $@ $^ $(LD_FLAGS)

-include $(OFILES:.o=.d)

.PHONY: clean all format
clean:
	rm -rf ./$(BIN)/* ansu_cli

format: $(CFILES)
	clang-format -i $^ $(HFILES)
