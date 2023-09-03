#include<stdio.h>
#include<string.h>
#define UCH unsigned char 
#define UINT unsigned int
#define LL unsigned long int
#define MOD32 (LL)0x0000000100000000
 UCH K1[4] = {0x5A,0x82,0x79,0x99};
 UCH K2[4] = {0x6E,0xD9,0xEB,0xA1};
 UCH K3[4] = {0x8F,0x1B,0xBC,0xDC};
 UCH K4[4] = {0xCA,0x62,0xC1,0xD6};
 UCH ONS[4]= {0xFF,0xFF,0xFF,0xFF};

void switchEndian(void *p, int size){ 
    //printf("%u %u %u %u %u %u %u %u\n", *((UCH *)p), *((UCH *)p+1), *((UCH *)p+2), *((UCH *)p+3), *((UCH *)p+4), *((UCH *)p+5), *((UCH *)p+6), *((UCH *)p+7));
    for(int i=0;i<size/2;i++){
        UCH tmp = *((UCH *)p+i);
        *((UCH *)p+i) = *((UCH *)p+size-1-i);
        *((UCH *)p+size-1-i) = tmp;
    }
    //printf("%u %u %u %u %u %u %u %u\n", *((UCH *)p), *((UCH *)p+1), *((UCH *)p+2), *((UCH *)p+3), *((UCH *)p+4), *((UCH *)p+5), *((UCH *)p+6), *((UCH *)p+7));
}

void pad(UCH *msg, LL N,LL MSize){    // N = 88   bytes
    //LL k = 56-N%64;       // k =64-8-24 = 32 bytes = 256 bits
    //No need of setting 0 as strncpy does that auto
    msg[N] = 0x80;          
    //for(LL i=1;i<k;i++) msg[i+N] = 0x00;  

    LL NN =N*8;
    void *p = &NN;
    switchEndian(p,8);  //check if this qorks
    
    memcpy((msg+MSize-8),p,8); 
    //for(int i=0;i<MSize;i++) printf("%02x ",msg[i]);
}
void XOR(UCH *target, UCH *w){ //32 bit words
    for(int i=0;i<4;i++) target[i] = target[i]^w[i];
}
void AND(UCH *target, UCH *w){ //32 bit words
    for(int i=0;i<4;i++) target[i] = target[i] & w[i];
}
void OR(UCH *target, UCH *w){ //32 bit words
    for(int i=0;i<4;i++) target[i] = target[i] | w[i];
}
void ADD(UCH *target, UCH *w){ //32 bit words mod 2^32
    switchEndian(target,4);
    switchEndian(w,4);
    UINT tar = *(UINT *)target;
    UINT wor = *(UINT *)w;
    
    LL tt = (LL)tar;
    LL ww = (LL)wor;
    
    tt = (tt+ww)%(MOD32);
    tar = (UINT)tt;
    switchEndian(&tar,4);
    switchEndian(w,4);
    memcpy(target,&tar,4);
}
void rotLeft(UCH *w, int n){ //only for 32 bits
    /* CHECK
     * ee 34 56 78 
     * on 30 rotation gives
     * 3b 8d 15 9e
     */
    n = n%32;
    UINT word = *(UINT *)w;
    switchEndian(&word, 4);             //problem, endianness
   // printf("word = %0X\n",word);
    
    //printf("%x \n a = %b \n b = %b\n",word,a,b);
    word = (word<<n) | (word>>(32-n));
    //printf("word = %X\n",word);
    switchEndian(&word,4);
    memcpy(w,&word,4);
    //printf("%x %x %x %x\n",*w,*(w+1),*(w+2),*(w+3));
    
}
void f1(UCH *tb,UCH *tc,UCH *td){
    AND(tc,tb);     // C <- B & C 
    XOR(tb,ONS);    // B <- ~B
    AND(tb,td);     // B <- ~B & D
    OR(tc,tb);      // C <- C | B = (B&C) | (~B & D)
}
void f2(UCH *tb,UCH *tc,UCH *td){
    XOR(tc,tb);
    XOR(tc,td);
}
void f3(UCH *tb, UCH *tc, UCH *td){
    UCH tbb[4];
    memcpy(tbb,tb,4);

    AND(tb,tc);
    AND(tbb,td);
    AND(tc,td);
    OR(tb,tbb);
    OR(tc,tb);      //end result in tc
}
void f4(UCH *tb,UCH *tc,UCH *td){
    XOR(tc,tb);
    XOR(tc,td);
}
void messageSchedule(UCH *block, UCH *W){ 
    //UCH W[320]; //2560 bis = 320 bytes
    memcpy(W, block, 64);

   // for(int j=0;j<16;j++) {for(int i=0;i<4;i++)printf("%02X",*(W+j*4+i));        printf("\n");}
   
    UCH *p = W+64;

    for(int j=16;j<80;j++){
        memcpy(p, (p-64), 4 );
        XOR ( p, (p-56) );
        XOR ( p, (p-32) );
        XOR ( p, (p-12) );          //stupid

        rotLeft(p,1);
        p += 4;
      //  for(int i=0;i<4;i++)printf("%02X",*(p+i));
        //printf("\n");
    }
}

void fourStages(UCH *a, UCH *b, UCH *c, UCH *d, UCH *e, UCH *W  ){
    UCH K[4][4];
    memcpy(K[0], K1,4);
    memcpy(K[1], K2,4);
    memcpy(K[2], K3,4);
    memcpy(K[3], K4,4);
    
    
    UCH ta[4],tb[4],tc[4],td[4],te[4];
    
    void (*fptr[4])(UCH*, UCH*, UCH*);
    fptr[0] = f1;
    fptr[1] = f2;
    fptr[2] = f3;
    fptr[3] = f4;
    
    UCH *wp=W;
    for(int stage=0;stage<4;stage++){
        
        for(int r=0;r<20;r++){ //20 rounds stage 1
            //copy
            memcpy(ta,a,4);
            memcpy(tb,b,4);
            memcpy(tc,c,4);
            memcpy(td,d,4);
            memcpy(te,e,4);

            memcpy(e,d,4);  //E set, D used
            memcpy(d,c,4);  //D set, C used
            rotLeft(b,30);  
            memcpy(c,b,4);  //C set, B used
            memcpy(b,a,4);  //B set, A used

            // f1 function c stores the res, so give XOR,AND,OR first arg as c
            /*
            printf("\n1tb: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tb+j));
            printf("\n2tc: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tc+j));
            printf("\n3tc: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tc+j));
            printf("\n\n4tb: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tb+j));
            printf("\n4tc: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tc+j));
            printf("\n4tc: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tc+j));
            
            printf("\n\n4tb: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tb+j));
            printf("\n4ON: ");
            for(int j=0;j<4;j++)printf("%02x ",*(ONS+j));
            for(int j=0;j<4;j++)printf("%02x ",*(tb+j));
            
            printf("\n\n4tb: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tb+j));
            printf("\n4td: ");
            for(int j=0;j<4;j++)printf("%02x ",*(td+j));
            printf("\n4tb: ");
            printf("\n5tb: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tb+j));

            printf("\n\n5tc: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tc+j));
            printf("\n5tb: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tb+j));
            printf("\n6tc: ");
            for(int j=0;j<4;j++)printf("%02x ",*(tc+j));
            */
           /**/
            fptr[stage](tb,tc,td);
            ADD(te,tc);
            rotLeft(ta,5);
            ADD(te,ta);
            ADD(te,wp); wp+=4;
            ADD(te,K[stage]);
            memcpy(a,te,4); // a SET e USED

            printf("t=%d: ",r+stage*20);
            for(int j=0;j<4;j++)printf("%02X",*(a+j));
            printf(" ");
            for(int j=0;j<4;j++)printf("%02X",*(b+j));
            printf(" ");
            for(int j=0;j<4;j++)printf("%02X",*(c+j));
            printf(" ");
            for(int j=0;j<4;j++)printf("%02X",*(d+j));
            printf(" ");
            for(int j=0;j<4;j++)printf("%02X",*(e+j));
            printf("\n");
            
        }
    
    }
}

void hash(UCH *msg, LL N,LL MSize, UCH *H){
    LL Nb = MSize/64;
    // 1. pad
    // 2. message schedule
    // 3. hash recursive
    /**/
    UCH A[4] = {0x67,0x45,0x23,0x01};
    UCH B[4] = {0xEF,0xCD,0xAB,0x89};
    UCH C[4] = {0x98,0xBA,0xDC,0xFE};
    UCH D[4] = {0x10,0x32,0x54,0x76};
    UCH E[4] = {0xC3,0xD2,0xE1,0xF0};

    memcpy(H,&A,4);
    memcpy(H+4,&B,4);
    memcpy(H+8,&C,4);
    memcpy(H+12,&D,4);
    memcpy(H+16,&E,4);
    
    pad(msg,N,MSize);                 //padding added to the whole message(last block)

    UCH W[320];                 //message schedule
    for(int i=0;i<Nb;i++){      //for each block
                                // each block needs 80 rounds
        messageSchedule(msg+i*64,W);
        fourStages( A,B,C,D,E, W); //only 20 W's
    
        //XOR with H's
        ADD(H,A);
        ADD(H+4,B);
        ADD(H+8,C);
        ADD(H+12,D);
        ADD(H+16,E);

        memcpy(A,H,4);
        memcpy(B,H+4,4);
        memcpy(C,H+8,4);
        memcpy(D,H+12,4);
        memcpy(E,H+16,4);
    }
}

int main(){

    LL N = 56;//56;
    LL MSize = N +64 - N%64;
    if(N>55) MSize+=64;
    
    UCH msg[MSize];
    //strncpy(msg,"abc",MSize);//defghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789");
    strncpy(msg,"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",MSize);
    //pad(msg,N,MSize);
    UCH H[20];
    hash(msg,N,MSize,H);
    printf("Hash = \n");
    for(int i=0;i<5;i++){
        for(int j=0;j<4;j++){
            printf("%02X",*(H+j+i*4));
        }
        printf("\n");
    }
   /*
    UCH vector1[4] = {0x01, 0x00, 0x00, 0x00}; // 0x00000001
    UCH vector2[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // 0xFFFFFFFF
    UCH vector3[4] = {0x12, 0x34, 0x56, 0x78}; // 0x12345678

    // Apply the ADD function to the vectors
    ADD(vector1, vector2);
    ADD(vector2, vector3);
    //ADD(vector3, vector1);

    // Print the modified vectors
    printf("Vector 1: %02x %02x %02x %02x\n", vector1[0], vector1[1], vector1[2], vector1[3]);
    printf("Vector 2: %02x %02x %02x %02x\n", vector2[0], vector2[1], vector2[2], vector2[3]);
    printf("Vector 3: %02x %02x %02x %02x\n", vector3[0], vector3[1], vector3[2], vector3[3]);

   */
    //UCH ww[4] = {0xee,0x34 ,0x56, 0x78};
    //hash(msg,N,H);
    //printf("%x %x %x %x\n",*ww,*(ww+1),*(ww+2),*(ww+3));
    //rotLeft(ww,30);

    //printf("%x %x %x %x\n",*ww,*(ww+1),*(ww+2),*(ww+3));
    
    
}

/*
ADD
v1 = 01 00 00 00
     0000 0001 0000 0000 0000 0000 0000 0000
v2 = ff ff ff ff
     1111 1111 1111 1111 1111 1111 1111 1111

res=00000 0000 1111 1111 1111 1111 1111 1111

v3 = 12 34 56 78
     0001 0010 0011 0100 0101 0110 0111 1000
     1111 1111 1111 1111 1111 1111 1111 1111
add=10001 0010 0011 0100 0101 0110 0111 0111
    12 34 56 78


ee 34 56 78
1110 1110 0011 0100 0101 0110 0111 1000
1101 1100 0110 1000 1010 1100 1111 0001
dc 68 ac f1
1101 1100 0110 1000 1010 1100 1111 0001
1011 1000 1101 0001 0101 1001 1110 0011
b8 d1 59 e3

ee 34 56 78
1110 1110 0011 0100 0101 0110 0111 1000
30 rotate
0011 1011 1000 1101 0001 0101 1001 1110
3b 8d 15 9e
78 56 34 ee (in little endian)
0111 1000 0101 0110 0011 0100 1110 1110
1111 0000 1010 1100 0110 1001 1101 1100
f0 ac 59 dc
*/