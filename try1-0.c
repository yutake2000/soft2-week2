#include <stdio.h>
#include <stdlib.h>

typedef struct point
{
  double x;
  double y;
} Point;

int main(int argc, char **argv)
{

  Point ps[2];
  int j = 0;
  ps[0] = (Point){.x = j++, .y = j++};
  j = 0;
  ps[1] = (Point){.y = j++, .x = j++};

  printf("ps[0] (x, y)=(%f, %f)\n", ps[0].x, ps[0].y);
  printf("ps[1] (x, y)=(%f, %f)\n", ps[1].x, ps[1].y);

  return 0;
}
