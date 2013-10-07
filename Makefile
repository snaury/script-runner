.PHONY: all help ruby perl tcl python 

all: ruby perl tcl python

help:
	# Please run one of the following:
	@ls -1 script-runner-*.c | sed -n 's/script-runner-\(.*\).c/#  make \1/p'
	# Or if you want to compile all:
	#  make

CFLAGS = -s -Os -DUNICODE -D_UNICODE
CC := gcc

script-runner.c: script-runner.h

script-runner-%.exe: script-runner-%.c script-runner.c
	$(CC) $(CFLAGS) -o $@ $^

ruby:   script-runner-ruby.exe
perl:   script-runner-perl.exe
tcl:    script-runner-tcl.exe
python: script-runner-python.exe

clean:
	rm -f *.exe
	rm -f *.o
