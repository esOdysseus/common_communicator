#ifndef INTERFACE_PROTOCOL_H_
#define INTERFACE_PROTOCOL_H_

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <cassert>

#include <CPayload.h>


class IProtocolInf : public payload::CPayload {
public:
    IProtocolInf(void);

    IProtocolInf(std::string name);

    ~IProtocolInf(void);

    virtual std::string get_property(const std::string key);

    template <typename T>
    bool set_property(const std::string key, T value);

};


#endif // INTERFACE_PROTOCOL_H_