#ifndef libfaafo_DGB_H
#define libfaafo_DGB_H

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define NDEBUG

#ifdef NDEBUG
#define log_debug(M, ...)
#else
#define log_debug(M, ...) fprintf(stdout, "DEBUG %s:%d: " M "\n",\
__FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) fprintf(stderr,\
"[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__,\
clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stderr,\
"[WARN] (%s:%d: errno: %s) " M "\n",\
__FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stdout, "[INFO] (%s:%d) " M "\n",\
__FILE__, __LINE__, ##__VA_ARGS__)

// Basic check with custom action
#define check(A, M, ACTION, ...) do { \
if(!(A)) { \
log_err(M, ##__VA_ARGS__); \
errno=0; \
ACTION; \
} \
} while(0)

// Return-based error handling
#define check_return(A, M, RETVAL, ...) do { \
if(!(A)) { \
log_err(M, ##__VA_ARGS__); \
errno=0; \
return RETVAL; \
} \
} while(0)

// Memory check with return value
#define check_mem_return(A, RETVAL) \
check_return((A), "Out of memory.", RETVAL)

#define check_mem(A, ACTION) \
check((A), "Out of memory.", ACTION)

// Debug-only checks
#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__);\
errno=0; return; }

#define try(A, M, ...) check(A, M, goto catch, ##__VA_ARGS__)

#endif
