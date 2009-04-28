#ifndef DEBUG_H__
#define DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifdef DEBUG__
#define debug(X) fprintf(stderr,__FILE__ ": " X "\n");
#else
#define debug(X)
#endif

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* DEBUG_H__ */