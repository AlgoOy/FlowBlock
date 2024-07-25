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

#if __IS_SUPPORTED_ARM_ARCH__
#   undef arm_irq_safe
#   undef arm_exit_irq_safe
#   define arm_irq_safe                                                         \
            arm_using(  uint32_t ARM_2D_SAFE_NAME(temp) =                       \
                        ({uint32_t temp=__get_PRIMASK();__disable_irq();temp;}),\
                        __set_PRIMASK(ARM_2D_SAFE_NAME(temp)))
#   define arm_exit_irq_safe    continue
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef uint8_t byte;

typedef enum fsm_rt_t {
    fsm_rt_err      = -1,
    fsm_rt_on_going = 0,
    fsm_rt_cpl      = 1,
} fsm_rt_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/


#ifdef __cplusplus
}
#endif

#endif