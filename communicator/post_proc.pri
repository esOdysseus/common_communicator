
# copy configuration files to release folder.
CONFIG_FILES += \
    $$files($$_PRO_FILE_PWD_/config/*.json)
RELEASE_CONFIG_PATH=$${DESTDIR}/../config

QMAKE_POST_LINK += $$quote($(MKDIR) -p $${RELEASE_CONFIG_PATH}$$escape_expand(\n\t))
for(FILE, CONFIG_FILES){
    QMAKE_POST_LINK += $$quote(cp $${FILE} $${RELEASE_CONFIG_PATH}$$escape_expand(\n\t))
}

