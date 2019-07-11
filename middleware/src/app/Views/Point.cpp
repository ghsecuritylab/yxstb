
#include "Point.h"


namespace Hippo {

void Point::rotateCW(Point* dst) const {
    //SkASSERT(dst);

    // use a tmp in case this == dst
    int32_t tmp = fX;
    dst->fX = -fY;
    dst->fY = tmp;
}

void Point::rotateCCW(Point* dst) const {
    //SkASSERT(dst);

    // use a tmp in case this == dst
    int32_t tmp = fX;
    dst->fX = fY;
    dst->fY = -tmp;
}

} // namespace Hippo
