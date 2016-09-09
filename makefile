# Project: Project 1 Operating Systems
# Makefile created by Dev-C++ 4.9.9.2
# Modified by Matthew Williams

CPP  = g++
RES  = 
OBJ  = Project1-MW.o $(RES)
LINKOBJ  = Project1-MW.o $(RES)
BIN  = Project1-MW
CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS)  
RM = rm -f


all: Project1-MW

clean: 
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o Project1-MW $(LIBS)

Project1-MW.o: Project1-MW.cpp
	$(CPP) -c Project1-MW.cpp -o Project1-MW.o $(CXXFLAGS)
