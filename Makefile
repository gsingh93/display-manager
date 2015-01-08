all: display-manager

display-manager: display-manager.c pam.c
	gcc `pkg-config --cflags --libs gtk+-3.0` -pthread -l pam -l X11 -Wall -o $@ $^

.PHONY: clean

clean:
	rm -f display-manager
