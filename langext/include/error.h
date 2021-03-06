#pragma once
#include "macros.h"

#define RAGE_OR_ERROR(t) RAGE_EITHER(char const *, t)
#define RAGE_FAIL(et, s) return (et) {.half=RAGE_EITHER_LEFT, .left=s};
#define RAGE_SUCCEED(et, v) return (et) {.half=RAGE_EITHER_RIGHT, .right=v};

#define RAGE_FAILED(e) (e.half == RAGE_EITHER_LEFT)
#define RAGE_FAILURE_VALUE(e) e.left
#define RAGE_SUCCESS_VALUE(e) e.right

#define RAGE_EXTRACT_VALUE(et, e, target) \
    if (RAGE_FAILED(e)) \
        RAGE_FAIL(et, RAGE_FAILURE_VALUE(e)); \
    target = RAGE_SUCCESS_VALUE(e);

typedef RAGE_OR_ERROR(void *) rage_Error;
#define RAGE_OK RAGE_SUCCEED(rage_Error, NULL)
#define RAGE_ERROR(msg) RAGE_FAIL(rage_Error, msg)
