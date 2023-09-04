#include<stdio.h>
#include<string.h>
#define UCH unsigned char 
#define UINT unsigned int
#define LL unsigned long int
#define MOD32 (LL)0x0000000100000000
 UINT K[4]={ 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
 UINT ONS= 0xFFFFFFFF;

void switchEndian(void *p, int size){ //unchanged
    //printf("%u %u %u %u %u %u %u %u\n", *((UCH *)p), *((UCH *)p+1), *((UCH *)p+2), *((UCH *)p+3), *((UCH *)p+4), *((UCH *)p+5), *((UCH *)p+6), *((UCH *)p+7));
    for(int i=0;i<size/2;i++){
        UCH tmp = *((UCH *)p+i);
        *((UCH *)p+i) = *((UCH *)p+size-1-i);
        *((UCH *)p+size-1-i) = tmp;
    }
    //printf("%u %u %u %u %u %u %u %u\n", *((UCH *)p), *((UCH *)p+1), *((UCH *)p+2), *((UCH *)p+3), *((UCH *)p+4), *((UCH *)p+5), *((UCH *)p+6), *((UCH *)p+7));
}
void pad(UCH *msg, LL N,LL MSize){    // N = 88   bytes unchanged
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
void XOR(UINT *target, UINT *w){ //32 bit words
    *target = (*target)^(*w);
}
void AND(UINT *target, UINT *w){ //32 bit words
    *target = (*target)&(*w);
}
void OR(UINT *target, UINT *w){ //32 bit words
    *target = (*target) | (*w);
}
void ADD(UINT *target, UINT *w){ //32 bit words mod 2^32
    
    LL tt = (LL)(*target);
    LL ww = (LL)(*w);
    
    tt = (tt+ww)%(MOD32);
    memcpy(target,&tt,4);
}
void rotLeft(UINT *w, int n){ //only for 32 bits
    n = n%32;
    *w = ((*w)<<n) | ((*w)>>(32-n));
}
void f1(UINT *tb,UINT *tc,UINT *td){
    AND(tc,tb);     // C <- B & C 
    XOR(tb,&ONS);    // B <- ~B
    AND(tb,td);     // B <- ~B & D
    OR(tc,tb);      // C <- C | B = (B&C) | (~B & D)
}
void f2(UINT *tb,UINT *tc,UINT *td){
    XOR(tc,tb);
    XOR(tc,td);
}
void f3(UINT *tb, UINT *tc, UINT *td){
    UINT tbb=*tb;

    AND(tb,tc);
    AND(&tbb,td);
    AND(tc,td);
    OR(tb,&tbb);
    OR(tc,tb);      //end result in tc
}
void f4(UINT *tb,UINT *tc,UINT *td){
    XOR(tc,tb);
    XOR(tc,td);
}
void messageSchedule(UINT *block, UINT *W){ //W stores the schedule
    //UCH W[320]; //2560 bis = 320 bytes = 80 UINTs
    //UCH block[64] // 512 bits = 16 UINTs
    memcpy(W, block, 64);

    for(int j=0;j<16;j++){
        *(W+j) = *(block+j);
        switchEndian(W+j,4);
    }
    //printf("\n");
    //for(int j=0;j<16;j++) {printf("%08X\n",*(W+j));}
   
    UINT *p = W+16;
    for(int j=16;j<80;j++){
        memcpy(p, (p-16), 4 );
        XOR ( p, (p-14) );
        XOR ( p, (p-8) );
        XOR ( p, (p-3) );          //stupid
        rotLeft(p,1);
        //printf("%08X\n",*(p));
        p++;
    }
}
void fourStages(UINT *a, UINT *b, UINT *c, UINT *d, UINT *e, UINT *W  ){
    
    UINT ta,tb,tc,td,te;
    
    void (*fptr[4])(UINT*, UINT*, UINT*);
    fptr[0] = f1;
    fptr[1] = f2;
    fptr[2] = f3;
    fptr[3] = f4;
    
    UINT *wp=W;
    for(int stage=0;stage<4;stage++){
        
        for(int r=0;r<20;r++){ //20 rounds stage 1
            //copy
            ta = *a;
            tb = *b;
            tc = *c;
            td = *d;
            te = *e;

            memcpy(e,d,4);  //E set, D used
            memcpy(d,c,4);  //D set, C used
            rotLeft(b,30);  
            memcpy(c,b,4);  //C set, B used
            memcpy(b,a,4);  //B set, A used

            (*(fptr+stage))(&tb,&tc,&td);
            ADD(&te,&tc);
            rotLeft(&ta,5);
            ADD(&te,&ta);
            ADD(&te,wp); wp+=1;          //changed to 1for UINT (4 for UCH)
            ADD(&te,K+stage);
            memcpy(a,&te,4); // a SET e USED
           /*
           */

            printf("t=%d: ",r+stage*20);
            printf("%08X",*(a));
            printf(" ");
            printf("%08X",*(b));
            printf(" ");
            printf("%08X",*(c));
            printf(" ");
            printf("%08X",*(d));
            printf(" ");
            printf("%08X",*(e));
            printf("\n");
            
        }
    
    }
}
void hash(UCH *msgg, LL N,LL MSize, UINT *H){
    LL Nb = MSize/64;
    // 1. pad
    // 2. message schedule
    // 3. hash recursive
    /**/
    
    pad(msgg,N,MSize);                 //padding added to the whole message(last block)
    UINT *msg = (UINT *)msgg;
    
    UINT A = 0x67452301;
    UINT B = 0xEFCDAB89;
    UINT C = 0x98BADCFE;
    UINT D = 0x10325476;
    UINT E = 0xC3D2E1F0;

    *(H) = A;
    *(H+1)=B;
    *(H+2)=C;
    *(H+3)=D;
    *(H+4)=E;
    

    UINT W[80];                 //message schedule
    for(int i=0;i<Nb;i++){      //for each block
                                // each block needs 80 rounds
        messageSchedule(msg+i*16,W);
        fourStages( &A,&B,&C,&D,&E, W); //only 20 W's
    
        //XOR with H's
        ADD(H,&A);
        ADD(H+1,&B);
        ADD(H+2,&C);
        ADD(H+3,&D);
        ADD(H+4,&E);

        A=*(H);
        B=*(H+1);
        C=*(H+2);
        D=*(H+3);
        E=*(H+4);
    }
}

int main(){

    LL N = 56;//56;
    LL MSize = N +64 - N%64;
    if(N>55) MSize+=64;
    
    UCH msgg[MSize];
    //strncpy(msgg,"abc",MSize);//defghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789");
    strncpy(msgg,"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",MSize);
    //pad(msg,N,MSize);
    UINT H[5];

    hash(msgg,N,MSize,H);
    //messageSchedule(msgg,W);
    printf("Hash = \n");
    for(int i=0;i<5;i++){
            printf("%08X\n",*(H+i));
    }
}
/*


MESSAGE SCHED
0110 0001 0110 0010 0110 0011 1000 0000
0100 0000 0000 0011 0000 0000 0000 0000 
61626380            
00000000        
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000018    
40030000    
40030000    
40030000    
40030000    
40030000    
40030000    
40030000    
00000000    
00000000    
00010000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
00000000    
FFFB8B17    
00000000    
57F70C09    
00000000    
00000000    
00000000    
B0FA222C    
AE7F0000    
00C01F2C    
AE7F0000    
C0020000    
00000000    
F07C5CFA    
FF7F0000    
9029212C    
AE7F0000    
00000000    
00000000    
E9815CFA    
FF7F0000    
02000000    
00000000    
1C000000    
00000000    
20000000    
= 
*/    