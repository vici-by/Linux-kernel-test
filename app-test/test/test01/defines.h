#ifndef __DEFINES_H
#define __DEFINES_H

#include <inttypes.h>
#include <endian.h>
#include <stdio.h>
#include <string.h>





typedef int64_t       cell;
typedef uint64_t     ucell;
typedef __int128_t   dcell;
typedef __uint128_t ducell;

typedef int64_t         prom_arg_t;
typedef uint64_t        prom_uarg_t;

#define bitspercell			(sizeof(cell)<<3)
#define bitsperdcell		(sizeof(dcell)<<3)



// 宏定义区域
#define CONFIG_LITTLE_ENDIAN 1



/* bit width handling */


#define PRId32 "d"
#define PRIu32 "u"
#define PRIx32 "x"
#define PRIX32 "X"
#define PRId64 "lld"
#define PRIu64 "llu"
#define PRIx64 "llx"
#define PRIX64 "llX"

#define FMT_CELL_x PRIx64
#define FMT_CELL_d PRId64


#endif//__DEFINES_H
