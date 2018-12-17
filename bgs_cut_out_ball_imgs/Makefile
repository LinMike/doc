PLATFORM = $(shell uname -s)
MATHINE = $(shell uname -m)

SRC_PATH = src
BUILD_PATH = build

OBJ_PATH = $(BUILD_PATH)


Linux_x86_64_INCLUDE = `pkg-config --cflags opencv`
Linux_x86_64_LDFLAGS = `pkg-config --libs opencv` -lboost_filesystem -lboost_system -lboost_thread

CC = g++
CPPFLAGS = -O3 -g0 -Wall

INCLUDE = $($(PLATFORM)_$(MATHINE)_INCLUDE)
LDFLAGS = $($(PLATFORM)_$(MATHINE)_LDFLAGS)

#$(info $(OBJ_PATH))
#$(info $(SRC_PATH))

SOURCES := $(shell find $(SRC_PATH) -name *.cc)
OBJECTS := $(SOURCES:$(SRC_PATH)/%.cc=$(OBJ_PATH)/%.o)
TARGETS := $(SOURCES:$(SRC_PATH)/%.cc=$(BUILD_PATH)/%)

#$(info $(SOURCES))
#$(info $(OBJECTS))
#$(info $(TARGETS))

#build/fast : build/fast.o
all : $(TARGETS)

$(BUILD_PATH)/%: $(OBJ_PATH)/%.o
	$(CC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

#$(TARGETS): $(OBJECTS)
#	$(CC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) -rf $(BUILD_PATH)/*

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cc
	$(CC) $(CPPFLAGS) $< -o $@ -c $(INCLUDE)


