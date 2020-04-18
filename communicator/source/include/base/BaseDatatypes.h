/***
 * BaseDatatypes.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _BASE_DATA_TYPES_H_
#define _BASE_DATA_TYPES_H_

#include <memory>
#include <CAppInternalCaller.h>
#include <CRawMessage.h>

class ICommunicator;

namespace dtype_b
{
    using RawMsgType = std::shared_ptr<CRawMessage>;
    using MsgType = std::shared_ptr<CRawMessage>;
    using SegmentType = CRawMessage;
    using SegmentsType = std::list<std::shared_ptr<SegmentType>>;
    using AppCallerType = std::shared_ptr<CAppInternalCaller>;
}

#endif // _BASE_DATA_TYPES_H_