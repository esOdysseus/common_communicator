#ifndef _C_JSON_MANIPULATOR_H_
#define _C_JSON_MANIPULATOR_H_

#include <list>
#include <cassert>
#include <memory>
#include <string>

#include <CRawMessage.h>
#include <json_headers.h>

namespace json_mng
{
    typedef enum E_ERROR {
        E_NO_ERROR = 0,
        E_HAS_NOT_MEMBER = 1,
        E_ITS_NOT_ARRAY = 2,
        E_ITS_NOT_SUPPORTED_TYPE = 3,
        E_INVALID_VALUE = 4
    }E_ERROR;


    class CMjson {
    public:
        CMjson(void);

        CMjson(Object_Type value);

        ~CMjson(void);

        bool is_there(void);

        bool parse(std::string json_file_path);

        template <typename T=std::string>
        std::shared_ptr<T> get_member(std::string key) {
            validation_check(key);
            return get<T>(key);
        }

        template <typename T=std::string>
        std::shared_ptr<std::list<std::shared_ptr<T>>> get_array_member(std::string key) {
            validation_check(key);
            return get_array<T>(key);
        }

    private:
        std::shared_ptr<CRawMessage> file_read(std::string &json_file_path);

        void validation_check(std::string& key);

        bool parse(std::shared_ptr<CRawMessage>& msg);

        bool has_member(std::string &key);

        bool is_array(std::string &key);

        void is_array_check(std::string &key);

        template <typename T=std::string>
        std::shared_ptr<std::list<std::shared_ptr<T>>> get_array(std::string &key);

        template <typename T>
        std::shared_ptr<T> get(ValueIterator itr);

        template <typename T>
        std::shared_ptr<T> get(std::string &key);

        template <typename T>
        T get_data(const char* data);

    private:
        bool is_parsed;

        static const unsigned int read_bufsize = 1024;

        char read_buf[read_bufsize];

        JsonManipulator manipulator;

        std::shared_ptr<Object_Type> object;

    };
}

using Json_DataType = std::shared_ptr<json_mng::CMjson>;

#endif // _C_JSON_MANIPULATOR_H_