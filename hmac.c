#include"sha1_int_wm.c"
UINT IPAD[BLOCK_SIZE] = {0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636, 0X36363636}; 
UINT OPAD[BLOCK_SIZE] = {0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C, 0X5C5C5C5C};
#include "hmac.h"

UINT* _HMAC_compute(UCH *msg, LL N, UINT* key){
    //using calloc for zero init of XpandedKey
    UINT *XKey = (UINT*)calloc(BLOCK_SIZE,1); //BLOCK_Size = 64 bytes
    memcpy(XKey+BLOCK_SIZE-16,key,16);
    XOR(XKey,IPAD);

    N+=BLOCK_SIZE;
    UCH *msg2 = (UCH*)malloc(N);
    memcpy(msg2,XKey,BLOCK_SIZE);
    memcpy(msg2+BLOCK_SIZE,msg,N-BLOCK_SIZE);

    UINT *msgDigest = _SHA1_compute(msg2,N); // 160 bits = 20 B
    free(msg2);

    XOR(XKey,IPAD); //taking back to Original
    XOR(XKey,OPAD);

    N=BLOCK_SIZE+20;
    UCH *msg3 = (UCH*)malloc(N);
    memcpy(msg3,XKey,BLOCK_SIZE);
    free(XKey);
    memcpy(msg3+BLOCK_SIZE,msgDigest,20);
    free(msgDigest);
    UINT *msgDigest2 = _SHA1_compute(msg3,N);

    return msgDigest2;

}
int main(){
    LL N=56;
    UCH *msg=(UCH*)malloc(56);
    strncpy(msg,"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",N);
    
    UCH *kkey=(UCH*)malloc(16); // key = 16 bytes = 128 bits 
    //BLOCK_SZIE = 64 bytes, so key be expanded
    strncpy(kkey,"abcdefghijklmnop",16);
    //UINT *key = (UINT*)kkey;
    //for(UINT i=0;i<16; i++) key[i] = i;

    UINT *hmac = _HMAC_compute(msg,N,(UINT*)kkey);
    for(int i=0;i<5;i++) 
        printf("%08X\n",*(hmac+i));
    return 0;
}

//