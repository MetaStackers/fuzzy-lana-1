CC = gcc
CFLAGS = -Wall -Werror -std=gnu11

all : harness

harness : harness.o
	$(CC) $(CFLAGS) -o $@ $<

harness.o : harness.c signed.h bare.h
	$(CC) $(CFLAGS) -c -o $@ $<


signed.h : vmlinuz-3.11.10-301.fc20.x86_64.signed
	xxd -i $< $@

bare.h : vmlinuz-3.11.10-301.fc20.x86_64.bare
	xxd -i $< $@

clean :
	@rm -vf *.h *.o harness

.PHONY : all clean
