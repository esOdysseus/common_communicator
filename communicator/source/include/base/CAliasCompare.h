#ifndef _C_ALIAS_COMPARE_H_
#define _C_ALIAS_COMPARE_H_

typedef struct _func_compare_sockaddr_in_ {
    bool operator()(const struct sockaddr_in &left, const struct sockaddr_in &right) const;
} CALIAS_CMPFUNC_for_sockaddr_in;

#endif // _C_ALIAS_COMPARE_H_