
#include <stdio.h>

#include "ind_gfx.h"
#include "app/Assertions.h"

int ind_rect_comp(struct ind_rect *rect, int left, int top, int right, int bottom)
{
    int l, r, t, b;

    if (left == rect->left && right == rect->right && top == rect->top && bottom == rect->bottom)
        return RECT_CMP_EQUAL;

    l = left;
    r = right;
    t = top;
    b = bottom;

    if (l < rect->left)
        l = rect->left;
    if (r > rect->right)
        r = rect->right;
    if (t < rect->top)
        t = rect->top;
    if (b > rect->bottom)
        b = rect->bottom;

    if (r - l <= 0 || b - t <= 0)
        return RECT_CMP_BROTHER;

    if (l == left && r == right && t == top && b == bottom)
        return RECT_CMP_CHILD;

    if (l == rect->left && r == rect->right && t == rect->top && b == rect->bottom)
        return RECT_CMP_FATHER;

    return RECT_CMP_ERROR;
}
