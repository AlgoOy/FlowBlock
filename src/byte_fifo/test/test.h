// debug.h
//#undef DEBUG_PRINT
//#ifdef DEBUG
//    #define DEBUG_PRINT(msg) printf("DEBUG: %s\n", msg)
//#else
//    #define DEBUG_PRINT(msg) // no-op
//#endif

// debug.h
#ifndef DEBUG_H
#define DEBUG_H

#undef DEBUG_PRINT
#ifdef DEBUG
    #define DEBUG_PRINT(msg) printf("DEBUG: %s\n", msg)
#else
    #define DEBUG_PRINT(msg) // no-op
#endif

#endif // DEBUG_H