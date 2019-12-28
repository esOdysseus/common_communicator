#include <cassert>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>

#include <logger.h>
#include <json_manipulator.h>

namespace json_mng
{

template std::shared_ptr<std::list<std::shared_ptr<std::string>>> CMjson::get_array<std::string>(std::string &key);
template std::shared_ptr<std::list<std::shared_ptr<CMjson>>> CMjson::get_array<CMjson>(std::string &key);

template std::shared_ptr<std::string> CMjson::get<std::string>(std::string &key);
template std::shared_ptr<int> CMjson::get<int>(std::string &key);
template std::shared_ptr<long> CMjson::get<long>(std::string &key);
template std::shared_ptr<bool> CMjson::get<bool>(std::string &key);
template std::shared_ptr<double> CMjson::get<double>(std::string &key);
template std::shared_ptr<float> CMjson::get<float>(std::string &key);

static const char* exception_switch(E_ERROR err_num) {
    switch(err_num) {
    case E_ERROR::E_NO_ERROR:
        return "E_NO_ERROR in json_mng pkg.";
    case E_ERROR::E_HAS_NOT_MEMBER:
        return "E_HAS_NOT_MEMBER in json_mng pkg.";
    case E_ERROR::E_ITS_NOT_ARRAY:
        return "E_ITS_NOT_ARRAY in json_mng pkg.";
    case E_ERROR::E_ITS_NOT_SUPPORTED_TYPE:
        return "E_ITS_NOT_SUPPORTED_TYPE in json_mng pkg.";
    case E_ERROR::E_INVALID_VALUE:
        return "E_INVALID_VALUE in json_mng pkg.";
    default:
        return "\'not support error_type\' in json_mng pkg.";
    }
}

#include <CException.h>

/*******************************
 * Public Function Definiction.
 */
CMjson::CMjson(void) : is_parsed(false) {
    object.reset();
}

CMjson::CMjson(Object_Type value) : is_parsed(false) {
    object.reset();
    object = std::make_shared<Object_Type>(value);
    is_parsed=true;
}

CMjson::~CMjson(void) {
    is_parsed = false;
    object.reset();
}

bool CMjson::is_there(void) {
    return is_parsed;
}

bool CMjson::parse(std::string json_file_path) {
    std::shared_ptr<CRawMessage> msg = file_read(json_file_path);
    is_parsed = parse(msg);
    return is_parsed;
}

/*******************************
 * Private Function Definiction.
 */
std::shared_ptr<CRawMessage> CMjson::file_read(std::string &json_file_path) {
    int fd = 0;
    size_t msg_size = read_bufsize;
    std::shared_ptr<CRawMessage> msg = std::make_shared<CRawMessage>();
    
    // open file
    fd = open(json_file_path.c_str(), O_RDONLY);
    assert(fd > 0);

    // read message
    while(msg_size == read_bufsize) {
        // >>>> Return value description
        // -1 : Error.
        // >= 0 : The number of received message.
        msg_size = read(fd, (char *)read_buf, read_bufsize); // Blocking Function.
        assert( msg_size >= 0 && msg_size <= read_bufsize);

        if( msg_size > 0 ){
            assert(msg->append_msg(read_buf, msg_size) == true);
        }
    }

    // close file
    close(fd);
    return msg;
}

template<>
std::string CMjson::get_data<std::string>(const char* data) {
    return std::string(data);
}

template<>
int CMjson::get_data<int>(const char* data) {
    return atoi(data);
}

template<>
long CMjson::get_data<long>(const char* data) {
    return atol(data);
}

template<>
bool CMjson::get_data<bool>(const char* data) {
    return atoi(data);
}

template<>
double CMjson::get_data<double>(const char* data) {
    return atof(data);
}

template<>
float CMjson::get_data<float>(const char* data) {
    return atof(data);
}

void CMjson::validation_check(std::string &key) {
    assert(key.empty() == false);
    assert(key.length() > 0);
    assert(is_there() == true);

    if( has_member(key) == false ) {
        throw CException(E_ERROR::E_HAS_NOT_MEMBER);
    }
}

void CMjson::is_array_check(std::string &key) {
    if( is_array(key) == false) {
        throw CException(E_ERROR::E_ITS_NOT_ARRAY);
    }
}

/***
 * Third-party library dependency function.
 */
#ifdef JSON_LIB_RAPIDJSON
bool CMjson::parse(std::shared_ptr<CRawMessage>& msg) {
    assert( msg.get() != NULL );
    const char* msg_const = (const char*)msg->get_msg_read_only();

    if( manipulator.Parse(msg_const).HasParseError() ) {
        return false;
    }
    assert(manipulator.IsObject());
    object.reset();
    object = std::make_shared<Object_Type>(manipulator.GetObject());

    return true;
}

bool CMjson::has_member(std::string &key) {
    assert(is_there() == true);
    return object->HasMember(key.c_str());
}

bool CMjson::is_array(std::string &key) {
    assert(is_there() == true);
    return (*object.get())[key.c_str()].IsArray();
}

template <typename T>
std::shared_ptr<std::list<std::shared_ptr<T>>> CMjson::get_array(std::string &key) {
    assert(is_there() == true);
    is_array_check(key);

    if ( std::is_same<T, std::string>::value == false && 
         std::is_same<T, CMjson>::value == false ) {
        throw CException(E_ERROR::E_ITS_NOT_SUPPORTED_TYPE);
    }

    std::shared_ptr<std::list<std::shared_ptr<T>>> ret = std::make_shared<std::list<std::shared_ptr<T>>>();
    rapidjson::Value &target = (*object.get())[key.c_str()];
    ValueIterator itr = target.Begin();
    for(; itr != target.End(); itr++) {
        ret->push_back(get<T>(itr));
    }

    return ret;
}

template <>
std::shared_ptr<std::string> CMjson::get<std::string>(ValueIterator itr) {
    return std::make_shared<std::string>(itr->GetString());
}

template <>
std::shared_ptr<CMjson> CMjson::get<CMjson>(ValueIterator itr) {
    return std::make_shared<CMjson>(itr->GetObject());
}

template <typename T>
std::shared_ptr<T> CMjson::get(std::string &key) {
    assert(is_there() == true);
    const char* value = NULL;
    std::shared_ptr<T> ret = std::make_shared<T>();
    rapidjson::Value::MemberIterator target = object->FindMember(key.c_str());

    if ( target == object->MemberEnd() ) {
        throw CException(E_ERROR::E_INVALID_VALUE);
    }
    
    if ( target->value.IsString() == true ) {
        value = target->value.GetString();
        assert(value != NULL);
        *(ret.get()) = get_data<T>(value);
    }
    else {
        throw CException(E_ERROR::E_ITS_NOT_SUPPORTED_TYPE);
    }

    return ret;
}

template <>
std::shared_ptr<CMjson> CMjson::get<CMjson>(std::string &key) {
    assert(is_there() == true);
    rapidjson::Value::MemberIterator target = object->FindMember(key.c_str());

    if ( target == object->MemberEnd() ) {
        throw CException(E_ERROR::E_INVALID_VALUE);
    }
    
    return std::make_shared<CMjson>(target->value.GetObject());
}
#elif JSON_LIB_HLOHMANN
    // TODO
#endif // JSON_LIB_RAPIDJSON or JSON_LIB_HLOHMANN

}