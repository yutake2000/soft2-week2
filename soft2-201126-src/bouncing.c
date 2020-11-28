/*
  画面の上下どちらでもバウンド可能
  画面外から画面内に入ろうとした場合もバウンド可能
  反発係数を0.01などの小さな値にしたときに自然な挙動をするように、速さだけでなく跳ね上がる距離にも反発係数をかけた
  コマンドライン引数に反発係数を渡すことも可能
  実行例: ./a.out 0.01
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "physics2.h"

int main(int argc, char **argv)
{
  const Condition cond = {
		    .width  = 75,
		    .height = 40,
		    .G = 1.0,
		    .dt = 0.1,
		    .cor = (argc == 2 ? atof(argv[1]) : 0.9)
  };
  
  size_t objnum = 2;
  Object objects[objnum];

  // objects[1] は巨大な物体を画面外に... 地球のようなものを想定
  objects[0] = (Object){ .m = 60.0, .y = -19.9, .vy = 5.0};
  objects[1] = (Object){ .m = 100000.0, .y =  1000.0, .vy = 0.0};

  // シミュレーション. ループは整数で回しつつ、実数時間も更新する
  const double stop_time = 400;
  double t = 0;
  printf("\n");
  for (int i = 0 ; t <= stop_time ; i++){
    t = i * cond.dt;
    my_update_velocities(objects, objnum, cond);
    my_update_positions(objects, objnum, cond);
    my_bounce(objects, objnum, cond);

    //デバッグ出力 2> out.txt
    //fprintf(stderr, "%lf\n", objects[0].y);
    
    // 表示の座標系は width/2, height/2 のピクセル位置が原点となるようにする
    my_plot_objects(objects, objnum, t, cond);
    
    // 200 x 1000us = 200 ms ずつ停止
    // ただし、時間の刻み幅が小さいときはそれに合わせて時間を短くする
    usleep(200 * 1000 * cond.dt);
    printf("\e[%dA", cond.height+3);// 壁とパラメータ表示分で3行
  }
  return EXIT_SUCCESS;
}

// 実習: 以下に my_ で始まる関数を実装する
// 最終的に phisics2.h 内の事前に用意された関数プロトタイプをコメントアウト

void my_plot_objects(Object objs[], const size_t numobj, const double t, const Condition cond) {

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
    int x = cond.width/2 + 1;
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

  //情報を表示
  //printf("y:%lf \r\n", objs[0].y);
  //printf("t =  33.0, objs[0].y =  -15.43, objs[1].y =  999.96", )
  printf("t = %4.1lf, objs[0].y = %6.2lf, objs[1].y = %6.2lf, cor = %.2lf\r\n", t, objs[0].y, objs[1].y, cond.cor);

}


void my_update_velocities(Object objs[], const size_t numobj, const Condition cond) {

  // それぞれの物体同士の距離を計算
  double dist[numobj][numobj];
  for (int i=0; i<numobj; i++) {
    for (int j=0; j<numobj; j++) {
      dist[i][j] = fabs(objs[i].y - objs[j].y);
    }
  }

  // 速度を更新
  for (int i=0; i<numobj; i++) {
    for (int j=0; j<numobj; j++) {
      if (i == j) continue;
      objs[i].vy += cond.G * objs[j].m * (objs[j].y - objs[i].y) / pow(dist[i][j], 3);
    }
  }
}


void my_update_positions(Object objs[], const size_t numobj, const Condition cond) {

  // 現在の位置をprev_yに保存してから更新する
  for (int i=0; i<numobj; i++) {
    objs[i].prev_y = objs[i].y;
    objs[i].y += objs[i].vy * cond.dt;
  }

}

void my_bounce(Object objs[], const size_t numobj, const Condition cond) {

  for (int i=0; i<numobj; i++) {

    // 画面端を横切った場合
    if (in_screen(objs[i].prev_y, cond) != in_screen(objs[i].y, cond)) {
      objs[i].vy *= -cond.cor;

      // 下の壁の座標をprev_yとyで挟んでいる場合
      // つまり、下の壁を通過した場合(上からでも下からでも)
      if (is_monotonic(objs[i].prev_y, cond.height/2, objs[i].y)) {
        // 画面内から画面外なら (objs[i].y - cond.height/2) > 0
        objs[i].y = cond.height/2 - (objs[i].y - cond.height/2) * cond.cor;
      }

      // 上の壁を通過した場合(上からでも下からでも)
      if (is_monotonic(objs[i].prev_y, -cond.height/2, objs[i].y)) {
        objs[i].y = -cond.height/2 + (-cond.height/2 - objs[i].y) * cond.cor;
      }
    }

  }

}

int in_screen(double y, const Condition cond) {
  return -cond.height/2 <= y && y <= cond.height/2;
}

int is_monotonic(double a, double b, double c) {
  return (a <= b && b <= c) || (c <= b && b <= a);
}
