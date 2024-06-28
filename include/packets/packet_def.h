#include "intellisense.h"

PACKET(TestingPacket,
    INT8(value)
    ARRAY(arr,
        UNSIG INT8(a)
        ARRAY(inner,
            INT16(b)
            FLOAT(c)
        )
        STRING(d)
    )
)