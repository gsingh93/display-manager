all: display-manager

display-manager: display-manager.c pam.c
	gcc -Wall -o $@ $^ `pkg-config --cflags --libs gtk+-3.0` -l pam

.PHONY: clean

clean:
	rm -f display-manager
