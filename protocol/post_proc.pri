
# delete generated base-code.
QMAKE_PRE_LINK += $$quote(rm -rf $${PROTOCOL_PATH}/include/base$$escape_expand(\n\t))
QMAKE_PRE_LINK += $$quote(rm -rf $${PROTOCOL_PATH}/src/base$$escape_expand(\n\t))