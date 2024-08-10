#ifndef __BLOCK_UTILS_H__
#define __BLOCK_UTILS_H__

/*============================ INCLUDES ======================================*/

#include <stdint.h>

#undef __IS_SUPPORTED_ARM_ARCH__
#if (   (defined(__ARM_ARCH) && __ARM_ARCH)                                     \
    ||  defined(__TARGET_ARCH_ARM)) && !defined(__APPLE__)
#   define __IS_SUPPORTED_ARM_ARCH__        1
#else
#   define __IS_SUPPORTED_ARM_ARCH__        0
#endif

#if __IS_SUPPORTED_ARM_ARCH__

#ifdef   __cplusplus
extern "C" {
#endif

#   include "cmsis_compiler.h"

#ifdef   __cplusplus
}
#endif

#else
#   include "user_arch_port.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// for IAR
#undef __IS_COMPILER_IAR__
#if defined(__IAR_SYSTEMS_ICC__)
#   define __IS_COMPILER_IAR__                  1
#endif

// for arm compiler 5
#undef __IS_COMPILER_ARM_COMPILER_5__
#if ((__ARMCC_VERSION >= 5000000) && (__ARMCC_VERSION < 6000000))
#   define __IS_COMPILER_ARM_COMPILER_5__       1
#endif


//for arm compiler 6

#undef __IS_COMPILER_ARM_COMPILER_6__
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#   define __IS_COMPILER_ARM_COMPILER_6__       1
#endif
#undef __IS_COMPILER_ARM_COMPILER__
#if defined(__IS_COMPILER_ARM_COMPILER_5__) && __IS_COMPILER_ARM_COMPILER_5__   \
||  defined(__IS_COMPILER_ARM_COMPILER_6__) && __IS_COMPILER_ARM_COMPILER_6__
#   define __IS_COMPILER_ARM_COMPILER__         1
#endif

// for clang
#undef  __IS_COMPILER_LLVM__
#if defined(__clang__) && !__IS_COMPILER_ARM_COMPILER_6__
#   define __IS_COMPILER_LLVM__                 1
#else

// for gcc
#   undef __IS_COMPILER_GCC__
#   if defined(__GNUC__) && !(  defined(__IS_COMPILER_ARM_COMPILER__)           \
                            ||  defined(__IS_COMPILER_LLVM__)                   \
                            ||  defined(__IS_COMPILER_IAR__))
#       define __IS_COMPILER_GCC__              1
#   endif

#endif

/*============================ MACROS ========================================*/

#ifndef ARM_NONNULL
#   if  defined(__IS_COMPILER_ARM_COMPILER_5__) ||\
        defined(__IS_COMPILER_ARM_COMPILER_6__) ||\
        defined(__IS_COMPILER_GCC__)            ||\
        defined(__IS_COMPILER_LLVM__)
#       define ARM_NONNULL(...)     __attribute__((nonnull(__VA_ARGS__)))
#   else
#       define ARM_NONNULL(...)
#   endif
#endif

#ifndef __VA_NUM_ARGS_IMPL
#define __VA_NUM_ARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11,    \
                            _12, _13, _14, _15, _16, _N, ...) _N
#endif

#ifndef __VAL_NUM_ARGS
#define __VAL_NUM_ARGS(...) \
            __VA_NUM_ARGS_IMPL(0, ##__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10,    \
                                9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#endif

#undef __CONNECT2
#undef __CONNECT3

#define __CONNECT2(__A, __B)                        __A##__B
#define __CONNECT3(__A, __B, __C)                   __A##__B##__C

#undef CONNECT2
#undef CONNECT3

#define CONNECT2(__A, __B)                  __CONNECT2(__A, __B)
#define CONNECT3(__A, __B, __C)             __CONNECT3(__A, __B, __C)

#define SAFE_NAME(__NAME) CONNECT3(__, __NAME, __LINE__)

#undef __USING1
#define __USING1(__declare)                                                     \
            for (__declare, *CONNECT3(__using_, __LINE__,_ptr) = NULL;          \
                 CONNECT3(__using_, __LINE__,_ptr)++ == NULL;                   \
                )
                
#undef __USING2
#define __USING2(__declare, __on_leave_expr)                                    \
            for (__declare, *CONNECT3(__using_, __LINE__,_ptr) = NULL;          \
                 CONNECT3(__using_, __LINE__,_ptr)++ == NULL;                   \
                 (__on_leave_expr)                                              \
                )

#undef __USING
#define __USING(...)                                                            \
            CONNECT2(__USING, __VAL_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#ifndef __IRQ_SAFE
#define __IRQ_SAFE                                                              \
            __USING(global_interrupt_status_t SAFE_NAME(temp) =                 \
                        global_interrupt_disable(),                             \
                    global_interrupt_resume(SAFE_NAME(temp)))
#undef __IRQ_SAFE_EXIT
#define __IRQ_SAFE_EXIT continue;
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef uint32_t global_interrupt_status_t;

typedef enum fsm_rt_t {
    fsm_rt_err      = -1,
    fsm_rt_on_going = 0,
    fsm_rt_cpl      = 1,
} fsm_rt_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

#if __IS_SUPPORTED_ARM_ARCH__

__STATIC_INLINE
global_interrupt_status_t global_interrupt_disable(void)
{
    global_interrupt_status_t tStatus = __get_PRIMASK();
    __disable_irq();
    
    return tStatus;
}

__STATIC_INLINE
void global_interrupt_resume(global_interrupt_status_t tStatus)
{
    __set_PRIMASK(tStatus);
}

#endif

#ifdef __cplusplus
}
#endif

#endif