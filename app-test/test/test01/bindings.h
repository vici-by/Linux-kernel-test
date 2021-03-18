#ifndef _H_BINDINGS
#define _H_BINDINGS

#include "stack.h"

#define fword(w)	({ static xt_t cache_xt = 0; _fword(w, &cache_xt); })
#define eword(w, nargs)	({ static xt_t cache_xt = 0; _eword(w, &cache_xt, nargs); })
#define selfword(w)	({ static xt_t cache_xt = 0; _selfword(w, &cache_xt); })
#define parword(w)	({ static xt_t cache_xt = 0; _parword(w, &cache_xt); })



#endif // _H_BINDINGS
