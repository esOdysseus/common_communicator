# Installation
CONFIG += create_pc create_prl no_install_prl

DESTDIR=$$_PRO_FILE_PWD_/$$BUILD_MODE/lib
headers.path = $$DESTDIR/../include
headers.files = $$HEADERS

binarys.path = $$DESTDIR/../bin
binarys.commands = $(MKDIR) $$binarys.path ; 

target.path = $$DESTDIR
target.files = &&EXTRA_BINFILES

QMAKE_PKGCONFIG_NAME = $$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = An example that illustrates how to do it right with qmake
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_PREFIX = $$DESTDIR/../bin
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig


INSTALLS += headers target

QMAKE_EXTRA_TARGETS += binarys
POST_TARGETDEPS += binarys
