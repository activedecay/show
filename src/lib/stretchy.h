//
// Created by justin on 1/26/19.
//

#ifndef STRETCHY_BUFFER_H_INCLUDED
#define STRETCHY_BUFFER_H_INCLUDED

#define stretch_free(a)                                                    \
        ((a) ? free(stretch_raw(a)),0 : 0)
#define push(a, v)                                                         \
        (stretch_maybegrow(a,1), (a)[stretch_n(a)++] = (v))
#define count(a)                                                           \
        ((a) ? stretch_n(a) : 0)
#define add_all(a, n)                                                      \
        (stretch_maybegrow(a,n), stretch_n(a)+=(n), &(a)[stretch_n(a)-(n)])
#define last(a)                                                            \
        ((a)[stretch_n(a)-1])

#define stretch_raw(a) ((int *) (a) - 2)
#define stretch_m(a)   stretch_raw(a)[0]
#define stretch_n(a)   stretch_raw(a)[1]

#define stretch_needgrow(a, n)  \
        ((a)==0 || stretch_n(a)+(n) >= stretch_m(a))
#define stretch_maybegrow(a, n) \
        (stretch_needgrow(a,(n)) ? stretch_grow(a,n) : 0)
#define stretch_grow(a, n)      \
        (*((void **)&(a)) = stretch_fun((a), (n), sizeof(*(a))))

#include <stdlib.h>

static void *stretch_fun(void *arr, int increment, int itemsize) {
  int dbl_cur = arr ? 2 * stretch_m(arr) : 0;
  int min_needed = count(arr) + increment;
  int m = dbl_cur > min_needed ? dbl_cur : min_needed;
  int *p = (int *) realloc(arr ? stretch_raw(arr)
                               : 0, itemsize * m + sizeof(int) * 2);
  if (p) {
    if (!arr)
      p[1] = 0;
    p[0] = m;
    return p + 2;
  } else {
#ifdef STRETCHY_BUFFER_OUT_OF_MEMORY
    STRETCHY_BUFFER_OUT_OF_MEMORY;
#endif
    // try to force a NULL pointer exception later
    return (void *) (2 * sizeof(int));
  }
}

#endif // STRETCHY_BUFFER_H_INCLUDED

