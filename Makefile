# Define C compiler.
CC = gcc

# Define optimization flags.
OPTS = -O2

# Define warnings and compile settings.
WFLAGS = -Wall -std=c99

# Binary install location.
PREFIX = /usr/local

# If compiling under Windows, you need an extention.
# EXTENSION=.exe

VERSION = 1.0
DOSVER = 1_0

DISTNAME = zpatchy-$(VERSION)
ZIPNAME = zpatchy-$(DOSVER).zip

BINA = z-differ
BINB = z-patcher

BINAX = $(BINA)$(EXTENSION)
BINBX = $(BINB)$(EXTENSION)

BINS = $(BINA) $(BINB)

CFLAGS = $(OPTS) $(WFLAGS)

all: $(BINS)
.PHONY: all install uninstall clean distclean dist help

BINA: $(BINA).c
	$(CC) $(CFLAGS) -o $(BINAX) $(BINA).c

BINB: $(BINB).c
	$(CC) $(CFLAGS) -o $(BINBX) $(BINB).c

install: $(BINS)
	mkdir -m 755 -p $(PREFIX)/bin
	mkdir -m 755 -p $(PREFIX)/share/doc/$(DISTNAME)
	install -c -m 755 $(BINAX) $(PREFIX)/bin
	install -c -m 755 $(BINBX) $(PREFIX)/bin
	install -c -m 644 zpatchy.txt $(PREFIX)/share/doc/$(DISTNAME)

uninstall:
	rm -f $(PREFIX)/bin/$(BINAX)
	rm -f $(PREFIX)/bin/$(BINBX)
	rm -rf $(PREFIX)/share/doc/$(DISTNAME)

clean:
	rm -f $(BINAX)
	rm -f $(BINBX)

distclean: clean
	rm -f *~

dist: distclean
	cd .. ; rm -f $(ZIPNAME) ; zip -r $(ZIPNAME) $(DISTNAME)

help:
	@echo
	@echo "Targets:"
	@echo "     all"
	@echo "     clean"
	@echo "     install"
	@echo "     uinstall"
	@echo "     dist"
	@echo
