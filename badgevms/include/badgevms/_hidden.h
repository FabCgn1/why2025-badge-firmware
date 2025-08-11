#pragma once
#include <stddef.h>

extern int _Unwind_Backtrace (void *, void *);
extern size_t _Unwind_GetIP (void *);
extern int __xpg_strerror_r (int, char *, size_t);
