#ifndef __CORE_CMINSTR_H
#define __CORE_CMINSTR_H

/*
// #define __USAT(ARG1,ARG2) \
// ({                          \
//   uint32_t __RES, __ARG1 = (ARG1); \
//   __ASM ("usat %0, %1, %2" : "=r" (__RES) :  "I" (ARG2), "r" (__ARG1) ); \
//   __RES; \
//  })
*/

#define __USAT(ARG1,ARG2) \
({                          \
  uint32_t __RES = (ARG1), __ARG1 = (ARG1); \
   __RES; \
 })


#endif /* __CORE_CMINSTR_H */