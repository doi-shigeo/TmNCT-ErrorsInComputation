#include <stdio.h>
#include <stdlib.h>
#include <math.h> // gcc では -lm オプションをつけてコンパイル

// 共用体，構造体と同じように使えるがメモリ領域は共有する
// long, double ともに 8byte であることを仮定(IEEE754倍精度)
// Windows の gcc だと sizeof(long) != sizeof(double) なので注意

#define OFFSET 0.3
#define TIME 100
#define BITWIDTH 64
#define MASK_SIGN      0x8000000000000000
#define MASK_EXPONENT  0x7FF0000000000000
#define MASK_MANTISSA  0x000FFFFFFFFFFFFF

// 共用体として扱う
typedef union {
  double d;
  long p;
} DL;

long IEEE754_sign(long l) { // 符号部を返す
  return (l & MASK_SIGN) >> (BITWIDTH - 1); // extract MSB
}
long IEEE754_exp(long l) { // 指数部を返す．バイアス(1023)が含まれる
  return (l & MASK_EXPONENT) >> (BITWIDTH - 1 - 11); // 1は符号部ビット数, 11は指数部ビット数
}
long IEEE754_man(long l) { // 仮数部を返す
  return (l & MASK_MANTISSA); 
}
double decode_IEEE754_to_double(long l) {
  double d = 0;
  
  long b, e;

  e = IEEE754_exp(l) - 1023; // 指数部はバイアスを引く
  b = IEEE754_man(l); // IEEE754の仮数部は "1.(bのビット列)"の形式
  
  if (b != 0 || e!= -1023) { // 仮数部のバイナリをdoubleに変換する
    d = 1;
    for (int i = (BITWIDTH - 11 - 1); i >=0; i--) { // 仮数部を先頭から1ビットずつ抽出
      long w = (b >> (i-1)) & 0x0000000000000001;
      d += pow(2, i - (BITWIDTH -11 - 1) - 1) * w;
    }
  }
  else
    d = 0;
  d *= pow(2, e);
  d *= (IEEE754_sign(l) ? -1 : 1);
  return d;
}


int main(void) {

  // 共用体の型の変数を初期化し，0とする
  DL u;
  u.d = 0.0;

  // Windows
  if (sizeof(double) != sizeof(long) || sizeof(double) != 8) {
    fprintf(stderr, "Size of double doesn't match that of long or isn't equal to 8\n");
    exit(-1);
  }
  printf("size(double)=%lu\n", sizeof(double));

  // バイナリとして変数の変化を表示
  printf("ループ回数,16進表現,符号,指数部,仮数部,手デコード,doubleで表現\n");
  for (int i = 0; i < TIME; i++) {
    printf("%3d,%16lx,%1ld,%4ld,%13lx,%18.15lf, %18.15lf\n", i, u.p, IEEE754_sign(u.p), IEEE754_exp(u.p), IEEE754_man(u.p), decode_IEEE754_to_double(u.p), u.d); // double型のビットを16進数で表示
    //printf("%3d,%lx,%lf\n", i, u.p, u.d); // double型のビットを16進数で表示
    u.d += OFFSET;
  }

  // 最終的に理論的結果と一致するかを確認
  if (u.d == TIME * OFFSET) {
    printf("%d * %lf = %lf\n", TIME, OFFSET, u.d);
  } else {
    printf("%d * %lf != %lf\n", TIME, OFFSET, u.d);
  }

  return 0;
}
