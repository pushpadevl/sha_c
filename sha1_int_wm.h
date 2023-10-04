#ifndef SHA1_INT_WM_H
#define SHA1_INT_WM_H

void XOR(UINT *target, UINT *w);
UINT* _SHA1_compute(UCH *msg, LL msgLength);

#endif