// 構造体の初期化についてまとめました

#include <stdio.h>
#include <stdlib.h>

typedef struct point
{
  double x;
  double y;
} Point;

int main(int argc, char **argv)
{

  const int LEN = 1000;
  const int W = 1000, H = 1000;
  double sum_x = 0, sum_y = 0;
  Point ps[LEN];
  //srand((long)time(NULL));
  int j = 0;
  ps[0] = (Point){.x = j++, .y = j++};
  printf("%f %f", ps[0].x, ps[0].y);
  for (int i=0; i<LEN; i++) {
    ps[i] = (Point){.x = rand()/(double)RAND_MAX*W, .y = rand()/(double)RAND_MAX*H};
    //printf("%f %f\n", ps[i].x, ps[i].y);
    sum_x += ps[i].x;
    sum_y += ps[i].y;
  }

  double gx = sum_x / LEN;
  double gy = sum_y / LEN;

  printf("gx:%f gy:%f\n", gx, gy);

  return 0;
}
