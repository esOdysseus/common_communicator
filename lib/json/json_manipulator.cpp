/***
 * json_manipulator.cpp
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#include <cassert>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <logger.h>
#include <json_manipulator.h>
#include <CRawMessage.h>

namespace json_mng
{

template CMjson::CList<std::string> CMjson::get_array<std::string>(std::string &key);
template CMjson::CList<CMjson> CMjson::get_array<CMjson>(std::string &key);

template std::string CMjson::get_second<std::string>(MemberIterator itor);
template int CMjson::get_second<int>(MemberIterator itor);
template short CMjson::get_second<short>(MemberIterator itor);
template int8_t CMjson::get_second<int8_t>(MemberIterator itor);
template uint32_t CMjson::get_second<uint32_t>(MemberIterator itor);
template uint16_t CMjson::get_second<uint16_t>(MemberIterator itor);
template uint8_t CMjson::get_second<uint8_t>(MemberIterator itor);
template long CMjson::get_second<long>(MemberIterator itor);
template bool CMjson::get_second<bool>(MemberIterator itor);
template double CMjson::get_second<double>(MemberIterator itor);
template float CMjson::get_second<float>(MemberIterator itor);

template CMjson::CList<int> CMjson::get_second_array<int>(MemberIterator itor);
template CMjson::CList<short> CMjson::get_second_array<short>(MemberIterator itor);
template CMjson::CList<int8_t> CMjson::get_second_array<int8_t>(MemberIterator itor);
template CMjson::CList<uint32_t> CMjson::get_second_array<uint32_t>(MemberIterator itor);
template CMjson::CList<uint16_t> CMjson::get_second_array<uint16_t>(MemberIterator itor);
template CMjson::CList<uint8_t> CMjson::get_second_array<uint8_t>(MemberIterator itor);
template CMjson::CList<long> CMjson::get_second_array<long>(MemberIterator itor);
template CMjson::CList<bool> CMjson::get_second_array<bool>(MemberIterator itor);
template CMjson::CList<double> CMjson::get_second_array<double>(MemberIterator itor);
template CMjson::CList<float> CMjson::get_second_array<float>(MemberIterator itor);
template CMjson::CList<std::string> CMjson::get_second_array<std::string>(MemberIterator itor);
template CMjson::CList<CMjson> CMjson::get_second_array<CMjson>(MemberIterator itor);

template std::shared_ptr<int> CMjson::get<int>(ValueIterator itr);
template std::shared_ptr<short> CMjson::get<short>(ValueIterator itr);
template std::shared_ptr<int8_t> CMjson::get<int8_t>(ValueIterator itr);
template std::shared_ptr<uint32_t> CMjson::get<uint32_t>(ValueIterator itr);
template std::shared_ptr<uint16_t> CMjson::get<uint16_t>(ValueIterator itr);
template std::shared_ptr<uint8_t> CMjson::get<uint8_t>(ValueIterator itr);
template std::shared_ptr<long> CMjson::get<long>(ValueIterator itr);
template std::shared_ptr<bool> CMjson::get<bool>(ValueIterator itr);
template std::shared_ptr<double> CMjson::get<double>(ValueIterator itr);
template std::shared_ptr<float> CMjson::get<float>(ValueIterator itr);

template std::string CMjson::get<std::string>(std::string &key);
template int CMjson::get<int>(std::string &key);
template short CMjson::get<short>(std::string &key);
template int8_t CMjson::get<int8_t>(std::string &key);
template uint32_t CMjson::get<uint32_t>(std::string &key);
template uint16_t CMjson::get<uint16_t>(std::string &key);
template uint8_t CMjson::get<uint8_t>(std::string &key);
template long CMjson::get<long>(std::string &key);
template bool CMjson::get<bool>(std::string &key);
template double CMjson::get<double>(std::string &key);
template float CMjson::get<float>(std::string &key);
template std::shared_ptr<CMjson> CMjson::get< std::shared_ptr<CMjson> >(std::string &key);

template bool CMjson::update<int>(std::string &key, int value);
template bool CMjson::update<short>(std::string &key, short value);
template bool CMjson::update<int8_t>(std::string &key, int8_t value);
template bool CMjson::update<uint32_t>(std::string &key, uint32_t value);
template bool CMjson::update<uint16_t>(std::string &key, uint16_t value);
template bool CMjson::update<uint8_t>(std::string &key, uint8_t value);
template bool CMjson::update<long>(std::string &key, long value);
template bool CMjson::update<bool>(std::string &key, bool value);
template bool CMjson::update<double>(std::string &key, double value);
template bool CMjson::update<float>(std::string &key, float value);


static const char* exception_switch(E_ERROR err_num) {
    switch(err_num) {
    case E_ERROR::E_NO_ERROR:
        return "E_NO_ERROR in json_mng pkg.";
    case E_ERROR::E_HAS_NOT_MEMBER:
        return "E_HAS_NOT_MEMBER in json_mng pkg.";
    case E_ERROR::E_HAS_NOT_STRING_MEMBER:
        return "E_HAS_NOT_STRING_MEMBER in json_mng pkg.";
    case E_ERROR::E_ITS_NOT_ARRAY:
        return "E_ITS_NOT_ARRAY in json_mng pkg.";
    case E_ERROR::E_ITS_NOT_SUPPORTED_TYPE:
        return "E_ITS_NOT_SUPPORTED_TYPE in json_mng pkg.";
    case E_ERROR::E_INVALID_VALUE:
        return "E_INVALID_VALUE in json_mng pkg.";
    case E_ERROR::E_NOT_ALLOCATED_MEMORY:
        return "E_NOT_ALLOCATED_MEMORY in json_mng pkg.";
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
    object_buf.Clear();
    object = std::make_shared<Value_Type>(ValueDef_Flag);
}

CMjson::CMjson(Object_Type value) : is_parsed(false) {
    object.reset();
    object_buf.Clear();
    object = std::make_shared<Value_Type>(value);
    is_parsed=true;
}

CMjson::~CMjson(void) {
    is_parsed = false;
    object.reset();
    object_buf.Clear();
}

bool CMjson::is_there(void) {
    return is_parsed;
}

bool CMjson::parse(std::string input_data, const E_PARSE arg_type) {
    try {
        switch(arg_type) {
        case E_PARSE::E_PARSE_FILE:
            {
                std::shared_ptr<CRawMessage> msg = file_read(input_data);
                is_parsed = parse(msg);
            }
            break;
        case E_PARSE::E_PARSE_MESSAGE:
            {
                std::shared_ptr<CRawMessage> msg = std::make_shared<CRawMessage>();
                msg->set_new_msg(input_data.c_str(), input_data.length());
                is_parsed = parse(msg);
            }
        default :
            throw CException(E_ERROR::E_ITS_NOT_SUPPORTED_TYPE);
        }
    }
    catch( const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
    return is_parsed;
}

bool CMjson::parse(const char* input_data, const ssize_t input_size) {
    assert(input_data != NULL);
    assert(input_size > 0);
    assert(strlen(input_data) == input_size);

    try {
        is_parsed = parse(input_data);
    }
    catch( const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
    return is_parsed;
}

const char* CMjson::print_buf(void) {
	const char* data = NULL;

    try{
    	object_buf.Clear();
        rapidjson::Writer<rapidjson::StringBuffer> writer(object_buf);

        object->Accept(writer);
        data = object_buf.GetString();
        LOGD("data = %s", data);
        return data;
    }
    catch( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
    return NULL;
}

MemberIterator CMjson::begin(void) {
    return get_begin_member();
}

MemberIterator CMjson::end(void) {
    return get_end_member();
}

std::string CMjson::get_first(MemberIterator itor) {
    return get_first_member(itor);
}

bool CMjson::has_member(std::string &key) {
    assert(is_there() == true);
    return object->HasMember(key.c_str());
}

bool CMjson::is_array(std::string &key) {
    assert(is_there() == true);
    return (*object.get())[key.c_str()].IsArray();
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
    if (fd <= 0) {
        LOGERR("Can not open file.(%s)", json_file_path.c_str());
    }

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
inline std::string CMjson::get_data<std::string>(const char* data) {
    assert(data != NULL);
    return std::string(data);
}

template<>
inline int CMjson::get_data<int>(const char* data) {
    assert(data != NULL);
    return atoi(data);
}

template<>
inline short CMjson::get_data<short>(const char* data) {
    assert(data != NULL);
    return (short)(atoi(data));
}

template<>
inline int8_t CMjson::get_data<int8_t>(const char* data) {
    assert(data != NULL);
    return (int8_t)(atoi(data));
}

template<>
inline uint32_t CMjson::get_data<uint32_t>(const char* data) {
    assert(data != NULL);
    return static_cast<uint32_t>(std::stoul(std::string(data), nullptr, 10));
}

template<>
inline uint16_t CMjson::get_data<uint16_t>(const char* data) {
    assert(data != NULL);
    return (uint16_t)(atoi(data));
}

template<>
inline uint8_t CMjson::get_data<uint8_t>(const char* data) {
    assert(data != NULL);
    return (uint8_t)(atoi(data));
}

template<>
inline long CMjson::get_data<long>(const char* data) {
    assert(data != NULL);
    return atol(data);
}

template<>
inline bool CMjson::get_data<bool>(const char* data) {
    assert(data != NULL);
    return (bool)(atoi(data));
}

template<>
inline double CMjson::get_data<double>(const char* data) {
    assert(data != NULL);
    return std::stod( std::string(data), nullptr );
}

template<>
inline float CMjson::get_data<float>(const char* data) {
    assert(data != NULL);
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

std::shared_ptr<Value_Type> CMjson::get_object(void) {
    return object;
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
    object = std::make_shared<Value_Type>(manipulator.GetObject());

    return true;
}

bool CMjson::parse(const char* msg_const) {
    assert(msg_const != NULL);

    if( manipulator.Parse(msg_const).HasParseError() ) {
        return false;
    }
    assert(manipulator.IsObject());
    object.reset();
    object = std::make_shared<Value_Type>(manipulator.GetObject());

    return true;
}

// Definition of Get Functions.
template <typename T>
CMjson::CList<T> CMjson::get_array(std::string &key) {
    assert(is_there() == true);
    is_array_check(key);

    if ( std::is_same<T, std::string>::value == false && 
         std::is_same<T, CMjson>::value == false ) {
        throw CException(E_ERROR::E_ITS_NOT_SUPPORTED_TYPE);
    }

    CList<T> ret = std::make_shared<std::list<std::shared_ptr<T>>>();
    Value_Type &target = (*object.get())[key.c_str()];
    ValueIterator itr = target.Begin();
    for(; itr != target.End(); itr++) {
        ret->push_back( CMjson::get<T>(itr) );
    }
    return ret;
}

template <typename T>
std::shared_ptr<T> CMjson::get(ValueIterator itr) {
    try {
        auto ret = std::make_shared<T>();
        if( ret.get() == NULL ) {
            throw CException(E_ERROR::E_NOT_ALLOCATED_MEMORY);
        }

        if( itr->IsString() == false ) {
            throw CException(E_ERROR::E_HAS_NOT_STRING_MEMBER);
        }
        *ret = get_data<T>( itr->GetString() );
        return ret;
    }
    catch( const std::exception &e) {
        LOGERR("%s", e.what());
        throw e;
    }
}

template <>
std::shared_ptr<std::string> CMjson::get<std::string>(ValueIterator itr) {
    if( itr->IsString() == false ) {
        throw CException(E_ERROR::E_HAS_NOT_STRING_MEMBER);
    }
    return std::make_shared<std::string>(itr->GetString());
}

template <>
std::shared_ptr<CMjson> CMjson::get<CMjson>(ValueIterator itr) {
    if( itr->IsObject() == false ) {
        throw CException(E_ERROR::E_HAS_NOT_MEMBER);
    }
    return std::make_shared<CMjson>(itr->GetObject());
}

template<typename T>
CMjson::CList<T> CMjson::get_second_array(MemberIterator itor) {
    auto ret = std::make_shared< std::list<std::shared_ptr<T>> >();
    if( ret.get() == NULL ) {
        throw CException(E_ERROR::E_NOT_ALLOCATED_MEMORY);
    }

    if( itor->value.IsArray() == false ) {
        throw CException(E_ERROR::E_ITS_NOT_ARRAY);
    }

    ValueIterator itr = itor->value.Begin();
    for(; itr != itor->value.End(); itr++) {
        ret->push_back( CMjson::get<T>(itr) );
    }
    return ret;
}

template <typename T>
T CMjson::get_second(MemberIterator itor) {
    const char* value = NULL;
    T ret;

    if ( itor->value.IsString() == true ) {
        value = itor->value.GetString();
        ret = get_data<T>(value);
    }
    else {
        throw CException(E_ERROR::E_ITS_NOT_SUPPORTED_TYPE);
    }
    return ret;
}

template <>
std::shared_ptr<CMjson> CMjson::get_second< std::shared_ptr<CMjson> >(MemberIterator itor) {
    return std::make_shared<CMjson>(itor->value.GetObject());
}

template <typename T>
T CMjson::get(std::string &key) {
    assert(is_there() == true);
    MemberIterator target = object->FindMember(key.c_str());

    if ( target == object->MemberEnd() ) {
        throw CException(E_ERROR::E_INVALID_VALUE);
    }
    
    return CMjson::get_second<T>(target);
}


inline MemberIterator CMjson::get_begin_member(void) {
    assert(is_there() == true);
    return object.get()->MemberBegin();
}

inline MemberIterator CMjson::get_end_member(void) {
    assert(is_there() == true);
    return object.get()->MemberEnd();
}

inline std::string CMjson::get_first_member(MemberIterator itor) {
    return itor->name.GetString();
}

// Definition of Set Functions.
std::shared_ptr<CMjson> CMjson::update(std::string &key, CMjson *value) {
    MemberIterator target;
    // Value_Type key_name(rapidjson::StringRef(key.c_str()), manipulator.GetAllocator());

    try {
        if ( value == NULL ) {
            Value_Type temp(rapidjson::kObjectType);
            assert( update_value(key, temp) == true );
        }
        else {
            assert( update_value(key, *(value->get_object().get())) == true );
        }

        target = object->FindMember(key.c_str());
        assert( target->value.IsObject() == true );

        return std::make_shared<CMjson>();
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return std::make_shared<CMjson>();
}

template <>
bool CMjson::update<const char *>(std::string &key, const char *value) {
    Value_Type temp;

    try {
        temp.Set(value);
        return update_value(key, temp);
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}

template <>
bool CMjson::update<std::string>(std::string &key, std::string value) {
    Value_Type temp;

    try {
        temp.Set(value.c_str());
        return update_value(key, temp);
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}

template <>
bool CMjson::update<CMjson::CList<std::string>>(std::string& key, CList<std::string> value) {
    Value_Type temp;

    try {
        auto& val = temp.SetArray();
        for( auto itr=value->begin(); itr != value->end(); itr++ ) {
            Value_Type ele((*itr)->data(), manipulator.GetAllocator());
            val.PushBack( ele, manipulator.GetAllocator() );
        }
        return update_value(key, val);
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
    return false;
}

template <>
bool CMjson::update<CMjson::CList<CMjson>>(std::string& key, CList<CMjson> value) {
    Value_Type temp;

    try {
        auto& val = temp.SetArray();
        for( auto itr=value->begin(); itr != value->end(); itr++ ) {
            val.PushBack( *((*itr)->get_object()), manipulator.GetAllocator() );
        }
        return update_value(key, val);
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }
    return false;
}

template <typename T>
bool CMjson::update(std::string &key, T value) {
    Value_Type temp;

    try {
        temp.Set(std::to_string(value).c_str());
        return update_value(key, temp);
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}

bool CMjson::update_value(std::string &key, Value_Type &value) {
    MemberIterator target;
    Value_Type key_name(rapidjson::StringRef(key.c_str()), manipulator.GetAllocator());

    try {
        target = object->FindMember(key.c_str());

        // If already exist 'key', then swap it.
        if ( target != object->MemberEnd() ) {
            target->value.CopyFrom(value, manipulator.GetAllocator(), true);
        }
        else {  // Otherwise, insert new 'key' with value.
            std::shared_ptr<Value_Type> Value = std::make_shared<Value_Type>(value, manipulator.GetAllocator(), true);
            object->AddMember(key_name.Move(), 
                              *(Value.get()),                           // move data from 'value' to 'object'.
                              manipulator.GetAllocator());
        }
        return true;
    }
    catch ( const std::exception &e ) {
        LOGERR("%s", e.what());
        throw e;
    }

    return false;
}


#elif JSON_LIB_HLOHMANN
    // TODO
#endif // JSON_LIB_RAPIDJSON or JSON_LIB_HLOHMANN

}   // namespace json_mng
