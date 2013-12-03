LANGUAGES:=$(patsubst script-runner-%.c,%,$(wildcard script-runner-*.c))

.PHONY: all help $(LANGUAGES)

all: $(LANGUAGES)

help:
	# Please run one of the following:
	$(foreach lang,$(LANGUAGES),#	make $(lang) $(\n))
	# Or if you want to compile all:
	#	make

CFLAGS = -s -Os -DUNICODE -D_UNICODE
CC := gcc

script-runner.c: script-runner.h

script-runner-%.exe: script-runner-%.c script-runner.c
	$(CC) $(CFLAGS) -o $@ $^

$(LANGUAGES):
	make script-runner-$@.exe

clean:
	rm -f *.exe
	rm -f *.o

define \n


endef
