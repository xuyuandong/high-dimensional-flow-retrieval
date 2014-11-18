CC = /usr/local/gcc-4.8.1/bin/g++

INCLUDE = -I ../common/ \
		  -I ../third-party/ \
		  -I ../util/ \
		  -I ../include/

LDFLAGS = -Wall -fPIC -g -fopenmp -std=c++11

LIB = ../lib

LIB_PATH = -L ../lib/

LIBS = -lpthread -lrt -lgtest 

.PHONY : all clean

TARGET = bid_bitmap_unittest \
	 bitmap_unittest \
	 tostring_ext_unittest \
	 string_ext_unittest \
	 random_ext_unittest \
	 pv_bid_bitmap_unittest \
	 uv_bid_bitmap_unittest \
	 uv_merge_bitmap_unittest \
	 index_unittest \
	 param_unittest

OBJS = bitmap_unittest.o \
       tostring_ext_unittest.o \
       string_ext_unittest.o \
       bid_bitmap_unittest.o \
       random_ext_unittest.o \
       pv_bid_bitmap_unittest.o \
       uv_bid_bitmap_unittest.o \
       uv_merge_bitmap_unittest.o \
       index_unittest.o \
       param_unittest.o \


all: $(TARGET)

bid_bitmap_unittest: bid_bitmap_unittest.o $(LIB)/libgtest.a
	$(CC) -o $@ $(LIBS) $(LDFLAGS) -pthread $< $(LIB_PATH)

bid_bitmap_unittest.o: bid_bitmap_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

bitmap_unittest: bitmap_unittest.o $(LIB)/libgtest.a
	$(CC) -o $@ $(LIBS) $(LDFLAGS) -pthread $< $(LIB_PATH)

bitmap_unittest.o: bitmap_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

tostring_ext_unittest: tostring_ext_unittest.o $(LIB)/libgtest.a
	$(CC) -o $@ $(LIBS) $(LDFLAGS) -pthread $< $(LIB_PATH)

tostring_ext_unittest.o: tostring_ext_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

string_ext_unittest: string_ext_unittest.o $(LIB)/libgtest.a
	$(CC) -o $@ $(LIBS) $(LDFLAGS) -pthread $< $(LIB_PATH)

string_ext_unittest.o: string_ext_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

random_ext_unittest: random_ext_unittest.o $(LIB)/libgtest.a
	$(CC)  -o $@ $(LIBS) $(LDFLAGS) -pthread $< $(LIB_PATH)

random_ext_unittest.o: random_ext_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

pv_bid_bitmap_unittest: pv_bid_bitmap_unittest.o ../src/pv_bid_bitmap.o $(LIB)/libgtest.a 
	$(CC) -o $@ $(LIBS) $(LDFLAGS) -pthread $^ $(LIB_PATH)

pv_bid_bitmap_unittest.o: pv_bid_bitmap_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

../src/pv_bid_bitmap.o: ../src/pv_bid_bitmap.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

uv_bid_bitmap_unittest: uv_bid_bitmap_unittest.o ../src/uv_bid_bitmap.o $(LIB)/libgtest.a 
	$(CC) -o $@ $(LIBS) $(LDFLAGS) -pthread $^ $(LIB_PATH)

uv_bid_bitmap_unittest.o: uv_bid_bitmap_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

../src/uv_bid_bitmap.o: ../src/uv_bid_bitmap.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

uv_merge_bitmap_unittest: uv_merge_bitmap_unittest.o ../src/uv_merge_bitmap.o $(LIB)/libgtest.a 
	$(CC) -o $@ $(LIBS) $(LDFLAGS) -pthread $^ $(LIB_PATH)

uv_merge_bitmap_unittest.o: uv_merge_bitmap_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

../src/uv_merge_bitmap.o: ../src/uv_merge_bitmap.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

index_unittest: index_unittest.o ../src/index.o ../src/*.o $(LIB)/*.a
	$(CC) -o $@ $(LIBS) $(LDFLAGS) -pthread $^ $(LIB_PATH)

index_unittest.o: index_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

../src/index.o: ../src/index.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

param_unittest: param_unittest.o ../src/param.o ../src/*.o $(LIB)/*.a
	$(CC) -o $@ $(LIBS) $(LDFLAGS) -pthread $^ $(LIB_PATH)

param_unittest.o: param_unittest.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

../src/param.o: ../src/param.cc
	$(CC) -c -D_GNU_SOURCE $(LDFLAGS) $^ -o $@ $(INCLUDE)

clean:
	rm -rf *.o $(TARGET) 
