all:
	@echo Please run one of the following:
	@echo make ruby
	@echo make perl
	@echo make tcl

CFLAGS = -s -Os -DUNICODE -D_UNICODE

script-runner-%.exe: script-runner-%.c script-runner.c script-runner.h
	gcc $(CFLAGS) -o $@ $< script-runner.c

ruby: script-runner-ruby.exe
perl: script-runner-perl.exe
tcl: script-runner-tcl.exe

.PHONY: ruby perl tcl

clean:
	rm -f *.exe
	rm -f *.o
