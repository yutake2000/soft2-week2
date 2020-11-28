/*
  画面の上下どちらでもバウンド可能
  画面外から画面内に入ろうとした場合もバウンド可能
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
		    .dt = 1.0,
		    .cor = 0.8
  };
  
  size_t objnum = 2;
  Object objects[objnum];

  // objects[1] は巨大な物体を画面外に... 地球のようなものを想定
  objects[0] = (Object){ .m = 60.0, .y = -19.9*2, .vy = 2.0};
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
    plot_objects(objects, objnum, t, cond);
    
    usleep(200 * 1000); // 200 x 1000us = 200 ms ずつ停止
    printf("\e[%dA", cond.height+3);// 壁とパラメータ表示分で3行
  }
  return EXIT_SUCCESS;
}

// 実習: 以下に my_ で始まる関数を実装する
// 最終的に phisics2.h 内の事前に用意された関数プロトタイプをコメントアウト

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
        objs[i].y -= (objs[i].y - cond.height/2) * 2;
      }

      // 上の壁を通過した場合(上からでも下からでも)
      if (is_monotonic(objs[i].prev_y, -cond.height/2, objs[i].y)) {
        objs[i].y += (-cond.height/2 - objs[i].y) * 2;
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
