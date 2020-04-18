/***
 * CException.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _C_EXCEPTION_H_
#define _C_EXCEPTION_H_

#include <stdexcept>

class CException: public std::exception
{
public:
    /** Constructor (C strings).
     *  @param message C-style string error message.
     *                 The string contents are copied upon construction.
     *                 Hence, responsibility for deleting the char* lies
     *                 with the caller. 
     */
    explicit CException(E_ERROR err_num):
      __err_n(err_num)
      {
      }

    /** Destructor.
     * Virtual to allow for subclassing.
     */
    virtual ~CException(void) throw (){}

    /** Returns a pointer to the (constant) error description.
     *  @return A pointer to a const char*. The underlying memory
     *          is in posession of the Exception object. Callers must
     *          not attempt to free the memory.
     */
    virtual const char* what(void) const throw (){
        return exception_switch(__err_n);
    }

protected:
    /** Error message.
     */
    E_ERROR __err_n;
};

#endif // _C_EXCEPTION_H_