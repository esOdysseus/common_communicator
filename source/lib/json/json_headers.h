#ifndef _C_JSON_HEADERS_SETTING_H_
#define _C_JSON_HEADERS_SETTING_H_

#ifdef JSON_LIB_RAPIDJSON
    // mode-flag for annotation-allow json-format.
    #define RAPIDJSON_PARSE_DEFAULT_FLAGS kParseCommentsFlag

    #include <rapidjson/document.h>

    namespace json_mng
    {
        using JsonManipulator = rapidjson::Document;
        using ValueIterator = rapidjson::Value::ValueIterator;
        using Object_Type = rapidjson::Value::Object;
    }
    
#elif JSON_LIB_HLOHMANN
    // TODO
#else
    #error "Select Json-library: \'JSON_LIB_RAPIDJSON\', \'JSON_LIB_HLOHMANN\'."
#endif

#endif // _C_JSON_HEADERS_SETTING_H_