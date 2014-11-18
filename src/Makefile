APP = $(PWD)/..

BIN = $(APP)/bin

AR = $(APP)/lib/*.a

LIB_PATH = -L /usr/local/lib/

LIBS = -lpthread -lrt -ldl

INCLUDE = -I $(APP)/third-party/ \
	  -I $(APP)/common/ \
	  -I $(APP)/util/ \
	  -I $(APP)/include/ \
	  -I /usr/local/include/

LDFLAGS = -shared -g -Wall -fPIC

CXXFLAGS = -std=c++11 -fPIC -Wall -O3 -fopenmp
#CXXFLAGS = -std=c++11 -fPIC -g -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-label -Wno-parentheses -gdwarf-2 -gstrict-dwarf

CC = /usr/local/gcc-4.8.1/bin/g++

SERVICE = $(BIN)/service
BUILDER = $(BIN)/builder

SERVICE_OBJS = service.o \
	       mg_thread_handler.o \
	       query_parser.o \
	       request_handler.o \
	       mongoose.o \
	       index.o \
	       pv_bid_bitmap.o \
	       uv_bid_bitmap.o \
	       uv_merge_bitmap.o \
	       target_map.o \
	       param.o

BUILDER_OBJS = builder.o \
	       index.o \
	       pv_bid_bitmap.o \
	       uv_bid_bitmap.o \
	       uv_merge_bitmap.o \
	       target_map.o

.PHONY : all clean lint
all: $(BUILDER) $(SERVICE)

lint:
	python cpplint.py src/*.cc include/*.h
	
.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)

$(SERVICE): $(SERVICE_OBJS)
	$(CC) $(CXXFLAGS) $^ -o $@ $(INCLUDE) $(AR) $(LIB_PATH) $(LIBS)

$(BUILDER): $(BUILDER_OBJS)
	$(CC) $(CXXFLAGS) $^ -o $@ $(INCLUDE) $(AR) $(LIB_PATH) $(LIBS)

clean:
	rm $(SERVICE_OBJS) $(BUILDER_OBJS) $(SERVICE) $(BUILDER)
