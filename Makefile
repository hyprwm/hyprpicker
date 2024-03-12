PREFIX = /usr/local
CFLAGS ?= -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function -Wno-unused-variable -Wno-unused-result -Wdeclaration-after-statement

CFLAGS += -I. -DWLR_USE_UNSTABLE -std=c99

WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)

PKGS = wlroots wayland-server
CFLAGS += $(foreach p,$(PKGS),$(shell pkg-config --cflags $(p)))
LDLIBS += $(foreach p,$(PKGS),$(shell pkg-config --libs $(p)))

default: all

clear:
	rm -rf build
	rm -f *.o *-protocol.h *-protocol.c

release:
	mkdir -p build && cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -H./ -B./build -G Ninja
	cmake --build ./build --config Release --target all -j 10

debug:
	mkdir -p build && cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Debug -H./ -B./build -G Ninja
	cmake --build ./build --config Debug --target all -j 10

all:
	make clear
	make release

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin ${DESTDIR}${PREFIX}/share/man/man1
	cp doc/hyprpicker.1 ${DESTDIR}${PREFIX}/share/man/man1
	cp build/hyprpicker ${DESTDIR}${PREFIX}/bin
