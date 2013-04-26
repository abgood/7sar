CC = gcc
CFLAGS = -g -O2 -Wall
INCLUDE_DIR = -I../include
TARGET = tsar
LIBDL = -ldl
DYNAM = -rdynamic
LIBMYSQL=`mysql_config --libs`

DIRS = modules

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o:%.cpp
	$(XX) $(CFLAGS) -c $< -o $@

# 把所有以.c结尾的文件放到列表里
SOURCES = $(wildcard *.c)
# 匹配sources列表里所有.c文件替换成.o文件放到列表里
OBJS = $(patsubst %.c,%.o,$(SOURCES))

$(TARGET):$(OBJS)
	make -C $(DIRS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBDL) $(DYNAM) $(LIBMYSQL)
	chmod u+x $(TARGET)

clean:
	rm $(OBJS) $(TARGET) -f
	cd $(DIRS); make clean; cd ..
