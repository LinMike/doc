
include common.mk

DEBUG=-O0 -g

SRCDIR=. input output converter utils com
ALLSRC=$(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.c $(dir)/*.cpp))
COMMONSRC=$(filter-out $(wildcard ./main*.cpp), $(ALLSRC))

#FLAGS=-I/home/yuanf/src/kernel/linux-3.0.35/include -I./input -I./output -I./converter -I./utils -I com $(DEBUG) -lpthread
FLAGS= -I/usr/include -I./include -I./input -I./output -I./converter -I./utils -I com $(DEBUG) -lpthread

SRC := $(COMMONSRC) main.cpp
EXAMPLE_SRC := $(COMMONSRC) main_example.cpp

target:$(SRC)
	$(eval TARGET = capture_manager)
	$(CC) $(FLAGS) -o $(TARGET) $(SRC)
	$(postbuild)
example:$(EXAMPLE_SRC)
	$(eval TARGET = example)
	$(CC) $(FLAGS) -o $(TARGET) $(EXAMPLE_SRC)
	$(postbuild)
clean:
	@rm ./capture_manager
	@rm ./example
	$(postclean)
all: target example


