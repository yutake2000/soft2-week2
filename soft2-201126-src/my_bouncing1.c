/*
  画面の上下どちらでもバウンド可能
  画面外から画面内に入ろうとした場合もバウンド可能
  反発係数を0.01などの小さな値にしたときに自然な挙動をするように、速さだけでなく跳ね上がる距離にも反発係数をかけた
  コマンドライン引数に反発係数を渡すことも可能
    実行例: ./a.out 0.01
  dtを小さくして、その分スリープ時間を短くした(シミュレーションの1秒が現実時間の200msになるように)
  初速度を大きくして上の壁とも衝突するようにした
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "my_bouncing1.h"

int main(int argc, char **argv)
{
  const Condition cond = {
		    .width  = 75,
		    .height = 40,
		    .G = 1.0,
		    .dt = 0.1,
		    .cor = (argc == 2 ? atof(argv[1]) : 0.9)
  };
  
  size_t objnum = 3;
  Object objects[objnum];

  // objects[2] は巨大な物体を画面外に... 地球のようなものを想定
  objects[0] = (Object){ .m = 60.0, .y = -19.9, .x = -30, .vy = 7.0, .vx = 12};
  objects[1] = (Object){ .m = 60.0, .y = -19.9, .x = 30, .vy = 4.0, .vx = 6.0};
  objects[2] = (Object){ .m = 100000.0, .y =  1000.0, .x = 0, .vy = 0.0, .vx = 0.0};

  // シミュレーション. ループは整数で回しつつ、実数時間も更新する
  const double stop_time = 400;
  double t = 0;
  int line = 0;
  printf("\n");
  for (int i = 0 ; t <= stop_time ; i++){
    t = i * cond.dt;
    my_update_velocities(objects, objnum, cond);
    my_update_positions(objects, objnum, cond);
    my_bounce(objects, objnum, cond);

    //デバッグ出力 2> out.txt
    //fprintf(stderr, "%lf\n", objects[0].y);
    
    // 表示の座標系は width/2, height/2 のピクセル位置が原点となるようにする
    line += my_plot_objects(objects, objnum, t, cond);
    
    // 200 x 1000us = 200 ms ずつ停止
    // ただし、時間の刻み幅が小さいときはそれに合わせて時間を短くする
    usleep(200 * 1000 * cond.dt);
    printf("\e[%dA", line);// 壁とパラメータ表示分で3行
    line = 0;
  }
  return EXIT_SUCCESS;
}

// 実習: 以下に my_ で始まる関数を実装する
// 最終的に phisics2.h 内の事前に用意された関数プロトタイプをコメントアウト

int my_plot_objects(Object objs[], const size_t numobj, const double t, const Condition cond) {

  int line = 0;

  char board[cond.height+2][cond.width+2];

  // 全て空白で埋める
  for (int y=0; y<cond.height+2; y++) {
    for (int x=0; x<cond.width+2; x++) {
      board[y][x] = ' ';
    }
  }

  // 物体
  for (int i=0; i<numobj; i++) {
    int y = objs[i].y + cond.height/2 + 1;
    int x = objs[i].x + cond.width/2 + 1;
    if (0 <= y && y < cond.height+2 && 0 <= x && x < cond.width+2) {
      board[y][x] = 'o';
    }
  }

  // 四隅の +
  board[0][0] = board[0][cond.width+1] = board[cond.height+1][0] = board[cond.height+1][cond.width+1] = '+';

  // 上下の -
  for (int x=1; x<cond.width+1; x++) {
    board[0][x] = '-';
    board[cond.height+1][x] = '-';
  }

  // 左右の |
  for (int y=1; y<cond.height+1; y++) {
    board[y][0] = '|';
    board[y][cond.width+1] = '|';
  }

  // boardを表示
  for (int y=0; y<cond.height+2; y++) {
    for (int x=0; x<cond.width+2; x++) {
      printf("%c", board[y][x]);
    }
    printf("\r\n");
  }

  line += cond.height + 2;

  //情報を表示
  printf("t = %4.1lf, cor = %0.2lf \r\n", t, cond.cor);
  for (int i=0; i<numobj; i++) {
    printf("obj[%d].y = %6.2lf, objs[%d].x = %6.2lf \r\n", i, objs[i].y, i, objs[i].x);
  }

  line += numobj + 1;

  return line;

}


void my_update_velocities(Object objs[], const size_t numobj, const Condition cond) {

  // 速度を更新
  for (int i=0; i<numobj; i++) {
    for (int j=0; j<numobj; j++) {
      if (i == j) continue;

      double dist = sqrt(pow(objs[i].y - objs[j].y, 2) + pow(objs[i].x - objs[j].x, 2));
      objs[i].vy += cond.G * objs[j].m * (objs[j].y - objs[i].y) / pow(dist, 3);
      objs[i].vx += cond.G * objs[j].m * (objs[j].x - objs[i].x) / pow(dist, 3);
    }
  }
}


void my_update_positions(Object objs[], const size_t numobj, const Condition cond) {

  // 現在の位置をprev_yに保存してから更新する
  for (int i=0; i<numobj; i++) {
    objs[i].prev_y = objs[i].y;
    objs[i].y += objs[i].vy * cond.dt;
    objs[i].prev_x = objs[i].x;
    objs[i].x += objs[i].vx * cond.dt;
  }

}

void my_bounce(Object objs[], const size_t numobj, const Condition cond) {

  for (int i=0; i<numobj; i++) {

    // 画面端を横切った場合
    if (in_screen(objs[i].prev_y, objs[i].prev_x, cond) != in_screen(objs[i].y, objs[i].x, cond)) {

      // 下の壁の座標をprev_yとyで挟んでいる場合
      // つまり、下の壁を通過した場合(上からでも下からでも)
      if (is_monotonic(objs[i].prev_y, cond.height/2, objs[i].y)) {
        // 画面内から画面外なら (objs[i].y - cond.height/2) > 0
        objs[i].y = cond.height/2 - (objs[i].y - cond.height/2) * cond.cor;
        objs[i].vy *= -cond.cor;
      }

      // 上の壁を通過した場合(上からでも下からでも)
      if (is_monotonic(objs[i].prev_y, -cond.height/2, objs[i].y)) {
        objs[i].y = -cond.height/2 + (-cond.height/2 - objs[i].y) * cond.cor;
        objs[i].vy *= -cond.cor;
      }

      // 右の壁
      if (is_monotonic(objs[i].prev_x, cond.width/2, objs[i].x)) {
        objs[i].x = cond.width/2 - (objs[i].x - cond.width/2) * cond.cor;
        objs[i].vx *= -cond.cor;
      }

      // 左の壁
      if (is_monotonic(objs[i].prev_x, -cond.width/2, objs[i].x)) {
        objs[i].x = -cond.width/2 + (-cond.width/2 - objs[i].x) * cond.cor;
        objs[i].vx *= -cond.cor;
      }
    }

  }

}

int in_screen(double y, double x, const Condition cond) {
  return -cond.height/2 <= y && y <= cond.height/2 && -cond.width/2 <= x && x <= cond.width/2;
}

int is_monotonic(double a, double b, double c) {
  return (a <= b && b <= c) || (c <= b && b <= a);
}
