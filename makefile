CXX = g++

ifeq ($(PROF), true)
CXXPROF += -pg -no-pie
else
CXXPROF += -s
endif

ifeq ($(BUILD), debug)
CXXFLAGS += -fdiagnostics-color=always -DUNICODE -std=c++20 -Wall -Og -pg -no-pie
else
CXXFLAGS += -DUNICODE -DNDEBUG -std=c++20 -O3 -msse4 -mbmi2 -flto $(CXXPROF) -march=native
endif

SRC = src/chess/*.cpp src/*.cpp

.PHONY: all lys clean makedir

all: lys

lys: makedir
	@$(CXX) $(CXXFLAGS) $(SRC) -o bin/lys.exe

clean: makedir
	@rm -rf bin
	@make makedir

makedir:
	@mkdir -p bin

.DEFAULT_GOAL := lys