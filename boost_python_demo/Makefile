CXX = g++
CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

#OBJS =	c_call_python.o
SRC = c_call_python.cc

LIBS = -L/usr/lib -lboost_python27 -lpython2.7

INCLUDE = -I/usr/include/python2.7

TARGET = c_call_python

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(SRC) $(LIBS) $(INCLUDE) 

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

