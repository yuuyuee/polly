// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_RING_H_
#define OAK_COMMON_RING_H_

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef void* Ring[2];

#define OAK_RING_DATA(ptr, type, field)               \
  ((type *) (((char *) (ptr)) - offsetof(type, field)))   // NOLINT

#define OAK_RING_NEXT(n) (*(Ring**) &((*(n))[0]))   // NOLINT
#define OAK_RING_PREV(n) (*(Ring**) &((*(n))[1]))   // NOLINT

#define OAK_RING_HEAD(n) (OAK_RING_NEXT(n))

#define OAK_RING_INIT(n) do {                         \
  OAK_RING_NEXT(n) = (n);                             \
  OAK_RING_PREV(n) = (n);                             \
} while (0)

#define OAK_RING_EMPTY(h)                             \
  ((const Ring *)(h) == (const Ring *)OAK_RING_NEXT(h))

#define OAK_RING_PREV_NEXT(n) (OAK_RING_NEXT(OAK_RING_PREV(n)))
#define OAK_RING_NEXT_PREV(n) (OAK_RING_PREV(OAK_RING_NEXT(n)))

#define OAK_RING_FOREACH(n, h)                        \
  for ((n) = OAK_RING_NEXT(h); (n) != (h); (n) = OAK_RING_NEXT(n))

#define OAK_RING_INSERT_HEAD(h, n) do {               \
  OAK_RING_NEXT(n) = OAK_RING_NEXT(h);                \
  OAK_RING_PREV(n) = (h);                             \
  OAK_RING_NEXT_PREV(n) = (n);                        \
  OAK_RING_NEXT(h) = (n);                             \
} while (0)

#define OAK_RING_INSERT_TAIL(h, n) do {               \
  OAK_RING_NEXT(n) = (h);                             \
  OAK_RING_PREV(n) = OAK_RING_PREV(h);                \
  OAK_RING_PREV_NEXT(n) = (n);                        \
  OAK_RING_PREV(h) = (n);                             \
} while (0)

#define OAK_RING_REMOVE(n) do {                       \
  OAK_RING_PREV_NEXT(n) = OAK_RING_NEXT(n);           \
  OAK_RING_NEXT_PREV(n) = OAK_RING_PREV(n);           \
} while (0)

#define OAK_RING_ADD(h, h2) do {                      \
  OAK_RING_PREV_NEXT(h) = OAK_RING_NEXT(h2);          \
  OAK_RING_NEXT_PREV(h2) = OAK_RING_PREV(h2);         \
  OAK_RING_PREV(h) = OAK_RING_PREV(h2);               \
  OAK_RING_PREV_NEXT(h) = (h);                        \
} while (0)

#define OAK_RING_SPLIT(h, n, h2) do {                 \
  OAK_RING_PREV(h2) = OAK_RING_PREV(h);               \
  OAK_RING_PREV_NEXT(h2) = (h2);                      \
  OAK_RING_NEXT(h2) = (n);                            \
  OAK_RING_PREV(h) = OAK_RING_PREV(n);                \
  OAK_RING_PREV_NEXT(h) = (h);                        \
  OAK_RING_PREV(n) = (h2);                            \
} while (0)

#define OAK_RING_MOVE(h, h2) do {                     \
  if (OAK_RING_EMPTY(h)) {                            \
    OAK_RING_INIT(h2);                                \
  } else {                                            \
    Ring* n = OAK_RING_HEAD(h);                       \
    OAK_RING_SPLIT(h, n, h2);                         \
  }                                                   \
} while (0)

#if defined(__cplusplus)
}
#endif

#endif  // OAK_COMMON_RING_H_
