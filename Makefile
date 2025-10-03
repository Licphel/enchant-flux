CXX := ccache g++
CXXFLAGS := -std=c++20 -Wall -Wextra -g -fdiagnostics-color=always
MAKEFLAGS += -j$(shell nproc)
SRC	:= src
INCLUDE	:= include
LIB := lib
BUILD := build
DLLDIR := dlls

SOURCEDIRS := $(SRC) $(wildcard $(SRC)/*/)
INCLUDEDIRS	:= $(INCLUDE) $(wildcard $(INCLUDE)/*/)

ifeq ($(OS),Windows_NT)
PROGRAMME := enchant.exe
LIBDIRS	:= $(LIB)
FIXPATH = $(subst /,\,$1)
RM := del /q /f
MD := mkdir
else
PROGRAMME := enchant
LIBDIRS := $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -f
MD := mkdir -p
endif

INCLUDEDIRS += C:/msys64/mingw64/include/freetype2
INCLUDEDIRS += C:/msys64/mingw64/include
LIBDIRS += C:/msys64/mingw64/lib

INCLUDES := $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))
LIBS := $(patsubst %,-L%, $(LIBDIRS:%/=%))
SOURCES	:= $(wildcard $(patsubst %,%/*.cpp, $(SOURCEDIRS)))

# Redirect object & dependency files to build dir
OBJECTS := $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

EXTERNAL_LIB := lib
EXTERNAL_LIBS := $(wildcard $(EXTERNAL_LIB)/*.a) $(wildcard $(EXTERNAL_LIB)/*.so)
EXTERNAL_LIB_NAMES := $(patsubst lib%.a,-l%,$(notdir $(filter %.a,$(EXTERNAL_LIBS)))) \
                      $(patsubst lib%.so,-l%,$(notdir $(filter %.so,$(EXTERNAL_LIBS))))
LFLAGS += -L$(EXTERNAL_LIB) $(EXTERNAL_LIB_NAMES)

# dynamic linked libs
LFLAGS += -lopenal -lfreetype -lfmt -lglfw3 -lglew32 -lbrotlienc -lbrotlidec -lbrotlicommon
# boost.asio deps
LFLAGS += -lws2_32 -lwsock32 -lmswsock -ladvapi32
# static linked libs
LFLAGS	+= -lquickjs
# system libs
LFLAGS += -lgdi32 -luser32 -lkernel32  -lopengl32 -pthread

LDFLAGS += -mconsole
LDFLAGS	+= -g -O0

LDFLAGS += -Wl,--disable-dynamicbase -Wl,--image-base,0x140000000
LDFLAGS += -Wl,-rpath,dlls

# ---------------------------- #

OUTPUTPROJ := $(call FIXPATH,$(PROGRAMME))

all: $(BUILD) $(PROGRAMME)

$(OUTPUT):
	$(MD) $(OUTPUT)

$(BUILD):
	$(MD) $(BUILD)

# Link step
$(PROGRAMME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTPROJ) $(OBJECTS) $(LFLAGS) $(LIBS)

-include $(DEPS)

# Compile step: put .o and .d under build/
$(BUILD)/%.o: $(SRC)/%.cpp
ifeq ($(OS),Windows_NT)
	if not exist "$(call FIXPATH,$(dir $@))" mkdir "$(call FIXPATH,$(dir $@))"
else
	$(MD) $(dir $@)
endif
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -MMD $< -o $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTPROJ)
	$(RM) -r $(BUILD)

run: all
	./$(OUTPUTPROJ)