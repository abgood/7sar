CFLAGS = -Wall -fPIC --shared -g -O2
CC = gcc
LINK = $(CC) $(CFLAGS)

SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.so,$(SOURCES))

all: $(OBJS)

$(OBJS): %.so: %.c
	$(LINK) $< -o $@
clean:
	rm $(OBJS) -f
