all: display-manager

display-manager: display-manager.c pam.c
	gcc `pkg-config --cflags --libs gtk+-3.0` -l pam -Wall -o $@ $^

.PHONY: clean

clean:
	rm -f display-manager
