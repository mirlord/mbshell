UNAME=$(shell uname)
DEBUG=1
INSTALL=install
ifndef PREFIX
  PREFIX=/usr
endif
ifndef SYSCONFDIR
  ifeq ($(PREFIX),/usr)
    SYSCONFDIR=/etc
  else
    SYSCONFDIR=$(PREFIX)/etc
  endif
endif

ifeq ($(shell which pkg-config 2>/dev/null 1>/dev/null || echo 1),1)
$(error "pkg-config was not found")
endif

# An easier way to get CFLAGS and LDFLAGS falling back in case there's
# no pkg-config support for certain libraries.
#
# NOTE that you must not use a blank after comma when calling this:
#     $(call ldflags_for_lib name, fallback) # bad
#     $(call ldflags_for_lib name,fallback) # good
# Otherwise, the compiler will get -l foo instead of -lfoo
#
cflags_for_lib = $(shell pkg-config --silence-errors --cflags $(1) 2>/dev/null)
ldflags_for_lib = $(shell pkg-config --exists 2>/dev/null $(1) && pkg-config --libs $(1) 2>/dev/null || echo -l$(2))

CFLAGS += -std=c99
CFLAGS += -pipe
CFLAGS += -Werror
CFLAGS += -Iinclude
CFLAGS += $(call cflags_for_lib, libmodbus)
#CFLAGS += $(call cflags_for_lib, yajl)

LIBS += -lm
# LIBS += $(call ldflags_for_lib, yajl,yajl)
LIBS += $(call ldflags_for_lib, libmodbus,libmodbus)

ifeq ($(DEBUG),1)
# Extended debugging flags, macros shall be available in gcc
CFLAGS += -gdwarf-2
CFLAGS += -g3
CFLAGS += -DDEBUG=1
else
CFLAGS += -O2
CFLAGS += -freorder-blocks-and-partition
endif

# Don’t print command lines which are run
.SILENT:

# Always remake the following targets
.PHONY: install clean

TOPDIR=$(shell pwd)

# Depend on the object files of all source-files in src/*.c and on all header files
#AUTOGENERATED:=src/progparse.tab.c src/progparse.yy.c
#FILES:=$(filter-out $(AUTOGENERATED),$(wildcard src/*.c))
FILES:=$(wildcard src/*.c)
FILES:=$(FILES:.c=.o)
HEADERS:=$(wildcard include/*.h)

# Depend on the specific file (.c for each .o) and on all headers
src/%.o: src/%.c ${HEADERS}
	echo "[mbsh] CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

all: mbsh

mbsh: ${FILES}
	echo "[mbsh] LINK $@"
	$(CC) $(LDFLAGS) -o $@ $(LIBS) ${FILES} -L/usr/lib -liniparser

install: all
	echo "[mbsh] INSTALL"
	$(INSTALL) -d -m 0755 $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 mbsh $(DESTDIR)$(PREFIX)/bin/

clean:
	rm -f src/*.o
	rm -f mbsh

