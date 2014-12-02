
CC=g++
CXX=g++
CFLAGS += -I.. -I. -DTSP104

SOURCEFILES = T380Demo_QAM_A.c  \
		UTIL.c \

OBJSS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SOURCEFILES)))

all: demo

demo: $(OBJSS)
	$(CC) $(EXTRA_CFLAGS) -g -o tvbagent $(OBJSS) -ltsphld0381 -lm -lpthread -ldl

clean:
	rm *.o
