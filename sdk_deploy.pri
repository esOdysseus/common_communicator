# Installation
CONFIG += create_pc create_prl no_install_prl

message( "[$$TARGET] DESTDIR=$$DESTDIR")

# create variable for pkg-config & installation.
headers.path = $$DESTDIR/../include
headers.files = $$HEADERS

binarys.path = $$DESTDIR/../bin
binarys.commands = $(MKDIR) $$binarys.path ; 

target.path = $$DESTDIR
target.files = &&EXTRA_BINFILES

# set variable for pkg-config of SDK-library.
QMAKE_PKGCONFIG_NAME = $$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = $$PKG_CONFIG_DESCRIPTION
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_PREFIX = $$binarys.path
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

# set variable for installation.
INSTALLS += headers target

QMAKE_EXTRA_TARGETS += binarys
POST_TARGETDEPS += binarys
