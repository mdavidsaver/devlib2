#ifndef EPICSMMIOGNU_H
#define EPICSMMIOGNU_H

#ifndef __GNUC__
# error GNU/GCC specific
#endif

#include <epicsTypes.h>
#include <epicsEndian.h>
#include <epicsVersion.h>
#include <shareLib.h>

#ifndef VERSION_INT
#  define VERSION_INT(V,R,M,P) ( ((V)<<24) | ((R)<<16) | ((M)<<8) | (P))
#  define EPICS_VERSION_INT VERSION_INT(EPICS_VERSION, EPICS_REVISION, EPICS_MODIFICATION, EPICS_PATCH_LEVEL)
#endif

#if EPICS_VERSION_INT>=VERSION_INT(3,15,0,2)
#  define HAVE_MMIO64
#endif

#ifdef __cplusplus
#  ifndef INLINE
#    define INLINE inline
#  endif
#endif
#ifndef EPICS_ALWAYS_INLINE
#  if __GNUC__ > 2
#    define EPICS_ALWAYS_INLINE __inline__ __attribute__((always_inline))
#  else
#    define EPICS_ALWAYS_INLINE __inline__
#  endif
#endif

#ifdef __PPC__
#  define MMIO_SYNC_INST "eieio"
#else
#  define MMIO_SYNC_INST ""
#endif

#define BUILD_MMIO(PREFIX,N) \
static EPICS_ALWAYS_INLINE epicsUInt ## N PREFIX ## read ## N(volatile void* addr) {\
    epicsUInt##N ret;\
    __asm__  __volatile__ ("" ::: "memory");\
    ret = *(volatile epicsUInt##N *)(addr);\
    __asm__  __volatile__ (MMIO_SYNC_INST ::: "memory");\
    return ret;\
}\
static EPICS_ALWAYS_INLINE void PREFIX ## write##N(volatile void* addr, epicsUInt##N val) {\
    __asm__  __volatile__ ("" ::: "memory");\
    *(volatile epicsUInt##N*)(addr) = val;\
    __asm__  __volatile__ (MMIO_SYNC_INST ::: "memory");\
}

BUILD_MMIO(io,8)
BUILD_MMIO(nat_io, 16)
BUILD_MMIO(nat_io, 32)
#ifdef HAVE_MMIO64
BUILD_MMIO(nat_io, 64)
#endif

#undef BUILD_MMIO
#undef MMIO_SYNC_INST

#if defined(__clang__)

#if __has_builtin(__builtin_bswap16)
#define _MMIO_swap16(X) __builtin_bswap16(X)
#endif
#if __has_builtin(__builtin_bswap32)
#define _MMIO_swap32(X) __builtin_bswap32(X)
#endif
#if __has_builtin(__builtin_bswap64)
#define _MMIO_swap64(X) __builtin_bswap64(X)
#endif

#elif (__GNUC__>4) || (__GNUC__==4 && __GNUC_MINOR__>=3)

#if (__GNUC__>4) || (__GNUC__==4 && __GNUC_MINOR__>=8)
#define _MMIO_swap16(X) __builtin_bswap16(X)
#endif

#define _MMIO_swap32(X) __builtin_bswap32(X)
#define _MMIO_swap64(X) __builtin_bswap64(X)

#endif

static EPICS_ALWAYS_INLINE
epicsUInt16
bswap16(epicsUInt16 value)
{
#ifdef _MMIO_swap16
    return _MMIO_swap16(value);
#else
    return (((epicsUInt16)(value) & 0x00ff) << 8)    |
           (((epicsUInt16)(value) & 0xff00) >> 8);
#endif
}

static EPICS_ALWAYS_INLINE
epicsUInt32
bswap32(epicsUInt32 value)
{
#ifdef _MMIO_swap32
    return _MMIO_swap32(value);
#else
    return (((epicsUInt32)(value) & 0x000000ff) << 24)   |
           (((epicsUInt32)(value) & 0x0000ff00) << 8)    |
           (((epicsUInt32)(value) & 0x00ff0000) >> 8)    |
           (((epicsUInt32)(value) & 0xff000000) >> 24);
#endif
}

#ifdef HAVE_MMIO64
static EPICS_ALWAYS_INLINE
epicsUInt64
bswap64(epicsUInt64 value)
{
#ifdef _MMIO_swap64
    return _MMIO_swap64(value);
#else
    epicsUInt64 ret = bswap32(value);
    ret <<= 32;
    ret |= bswap32(value>>32u);
    return ret;
#endif
}
#endif /* HAVE_MMIO64 */

#if EPICS_BYTE_ORDER == EPICS_ENDIAN_BIG
#  define be_ioread16(A)    nat_ioread16(A)
#  define be_ioread32(A)    nat_ioread32(A)
#  define be_iowrite16(A,D) nat_iowrite16(A,D)
#  define be_iowrite32(A,D) nat_iowrite32(A,D)

#  define le_ioread16(A)    bswap16(nat_ioread16(A))
#  define le_ioread32(A)    bswap32(nat_ioread32(A))
#  define le_iowrite16(A,D) nat_iowrite16(A,bswap16(D))
#  define le_iowrite32(A,D) nat_iowrite32(A,bswap32(D))

#ifdef HAVE_MMIO64
#  define be_ioread64(A)    nat_ioread64(A)
#  define be_iowrite64(A,D) nat_iowrite64(A,D)

#  define le_ioread64(A)    bswap64(nat_ioread64(A))
#  define le_iowrite64(A,D) nat_iowrite64(A,bswap64(D))
#endif

/** @} */

#elif EPICS_BYTE_ORDER == EPICS_ENDIAN_LITTLE
#  define be_ioread16(A)    bswap16(nat_ioread16(A))
#  define be_ioread32(A)    bswap32(nat_ioread32(A))
#  define be_iowrite16(A,D) nat_iowrite16(A,bswap16(D))
#  define be_iowrite32(A,D) nat_iowrite32(A,bswap32(D))

#  define le_ioread16(A)    nat_ioread16(A)
#  define le_ioread32(A)    nat_ioread32(A)
#  define le_iowrite16(A,D) nat_iowrite16(A,D)
#  define le_iowrite32(A,D) nat_iowrite32(A,D)

#ifdef HAVE_MMIO64
#  define be_ioread64(A)    bswap64(nat_ioread64(A))
#  define be_iowrite64(A,D) nat_iowrite64(A,bswap64(D))

#  define le_ioread64(A)    nat_ioread64(A)
#  define le_iowrite64(A,D) nat_iowrite64(A,D)
#endif

#else
#  error Unable to determine native byte order
#endif

#endif // EPICSMMIOGNU_H
