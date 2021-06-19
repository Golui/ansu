
INCLUDE := include lib/cereal/include
SOURCE := src
CLI_MODULE := cli
SOURCE_VIVADO := vivado
LIB := lib
SCHEMA := schema
BIN := bin

CLIBS := microtar
LIBS := $(CLIBS)

LINCLUDES := $(foreach lib, $(LIBS),-I $(LIB)/$(lib)/include)
LSEARCH := $(foreach lib, $(CLIBS),-L $(LIB)/$(lib))
LFLAGS := $(foreach lib, $(CLIBS),-l$(lib))

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

INCLUDE_SEARCH := $(foreach INCL, $(INCLUDE), -I $(INCL)) $(LINCLUDES)

CPP_FLAGS := -MMD -MP -Wall -g -pedantic -std=c++11 -Wno-unknown-pragmas -fmax-errors=8\
			 $(INCLUDE_SEARCH)

ifndef BACKEND
	BACKEND := SOFTWARE
endif

CPP_FLAGS := $(CPP_FLAGS) -D$(BACKEND)

CFILES := $(shell find $(SOURCE) -name "*.cpp" ! -path "$(SOURCE)/backend/*")
HFILES := $(shell find $(INCLUDE) -name "*.hpp" -or -name "*.h" ! -path "$(INCLUDE)/ansu/backend/*")

ifeq "$(BACKEND)" "SOFTWARE"
	override CFILES += $(shell find $(SOURCE)/backend/software -name "*.cpp")
	override HFILES += $(shell find $(INCLUDE)/ansu/backend/software -name "*.hpp")
endif

OFILES := $(patsubst $(SOURCE)/%.cpp,$(BIN)/%.o, $(CFILES))

LD_FLAGS := $(LD_FLAGS) $(LSEARCH) $(LFLAGS)

all: libs libansu ansu_cli

$(CLIBS):
	cd $(LIB)/$@ && $(MAKE) C=$(C) CPP=$(CPP)

libs: $(CLIBS)

$(BIN)/%.o: $(SOURCE)/%.cpp
	@-mkdir -p $(@D)
	$(CPP) $(CPP_FLAGS) -o $@ -c $<

libansu: $(OFILES)
	echo "Libbing ansu"
	ar rcs libansu.a $^

ansu_cli: libansu
	$(MAKE) -C $(CLI_MODULE)
	cp $(CLI_MODULE)/ansu_cli ansu_cli

-include $(OFILES:.o=.d)

.PHONY: clean all format
clean:
	$(MAKE) -C $(CLI_MODULE) clean
	rm -rf ./$(BIN)/* ansu_cli

format: $(CFILES)
	clang-format -i $^ $(HFILES)
