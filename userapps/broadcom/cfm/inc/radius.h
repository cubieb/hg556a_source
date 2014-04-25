#ifndef __RADIUS_H__
#define __RADIUS_H__
#include "ifcdefs.h"
#include "syscall.h"
/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif



 
int RadiusInit (char *name , char *password);
 
#if defined(__cplusplus)
}
#endif

#endif /* __RADIUS_H__ */

