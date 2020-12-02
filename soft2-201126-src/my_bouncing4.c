/*
  太陽系の惑星シミュレーション

  初期位置は2020/06/21 06:44:00(夏至)の惑星の日心座標とした。
  下記のサイトから「日心黄経(度)」「動径(au)」「日変化(度/日)」をファイルに保存し、x,y,vx,vyはこれらの情報から求めた。
  (国立天文台 惑星の日心座標: https://eco.mtk.nao.ac.jp/cgi-bin/koyomi/cande/planet_ecliptic.cgi)

  ただし、地球の日心座標が得られなかったため太陽の地心座標から計算した。
  太陽の地心黄経が90.000220度なので、地球の日心黄経は270.000220度とした(光行差を含むが補正はしていない)。
  地心距離が1.0162988auなので、そのまま動径とした。
  日変化は2020/06/21 07:44:00(1時間後)の地心黄経90.039996との差の24倍とした。
  (国立天文台 太陽の地心座標: https://eco.mtk.nao.ac.jp/cgi-bin/koyomi/cande/sun.cgi)

  惑星の質量と公転周期はWikipediaから得た。

  デフォルトでは0.1auが高さ1マス、幅2マス分になっている。
  (幅が2倍になっているのは、軌道が縦長に見えてしまうのを防ぐため。)

  コマンドライン引数は以下の通り
    ファイル名 (シミュレーション時間[日] 時間刻み幅[日] 縮尺[au/高さ1マス])
  または
    moon (シミュレーション時間[日] 時間刻み幅[日])

  ファイルの形式はmy_bouncing2.cで読み込むものと異なるので注意。
  ファイル名を指定した場合は、ファイルに書かれた全てのオブジェクトを読み込む。
  「moon」を指定した場合は、太陽と地球と月のシミュレーションが開始する。
  太陽と地球の間、地球と月の間の距離のスケールが大きく異なるため、月は地球を中心として別スケールで描画されている。
  (地球周辺が拡大表示されているようなイメージ)

  シミュレーション時間に公転周期を設定すると1周して同じ位置に戻ってくるのが確認できる。
  初期位置を確認できるように、最初に1秒止まるようになっている。

  上の4つの例では地球型惑星のみが表示範囲に含まれる。
  その後の4つの例では縮尺を変更し、木星型惑星が見られるようになってる。(ただし、地球型惑星の動きはほとんどわからなくなる。)

  また、下記のサイトでも惑星の位置をシミュレーションできる。
  (https://stdkmd.net/ssg/)

  実行例:
    水星
    ./a.out data4_solar_system.dat 87.97

    金星
    ./a.out data4_solar_system.dat 224.70

    地球(引数を省略すると地球が一番見やすくなる設定になる)
    ./a.out data4_solar_system.dat

    火星
    ./a.out data4_solar_system.dat 686.98

    木星
    ./a.out data4_solar_system.dat 4329 3 2

    土星
    ./a.out data4_solar_system.dat 10779 5 2

    天王星
    ./a.out data4_solar_system.dat 30752 10 2

    海王星
    ./a.out data4_solar_system.dat 60148 10 2

    太陽と地球と月(実際の公転周期より少し短くなってしまう)
    ./a.out moon 27.3 0.05
    ./a.out moon 365 0.1
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include "my_bouncing4.h"

int main(int argc, char **argv)
{

  if (argc < 2) {
    //ファイル名 (シミュレーション時間[日] 時間刻み幅[日] 縮尺[au/高さ1マス])
    fprintf(stderr, "usage:\t%s <filename> [<days> <dt> <scale>]\n\t%s moon <days> <dt>\n", argv[0], argv[0]);
    return 1;
  }

  const Condition cond = {
		    .width  = 75,
		    .height = 38,
		    .G = 6.67430e-11,
		    .dt = 60*60*24 * (argc >= 4 ? atof(argv[3]) : 1),
        .au = 149597870700,
        .earth_to_moon = 384400000,
        .scale = (argc >= 5 ? atof(argv[4]) : 0.1),
        .moon = (strcmp(argv[1], "moon") == 0 ? 1 : 0)
  };
  
  size_t objnum = 0;
  Object objects[100];

  load_objects(objects, &objnum, argv[1], cond);

  // シミュレーション. ループは整数で回しつつ、実数時間も更新する
  const double stop_time = (argc >= 3 ? atof(argv[2]) : 365) * 60 * 60 * 24;
  double t = 0;
  int line = 0; // 表示した行数
  
  line = my_plot_objects(objects, objnum, t, cond);
  usleep(1000 * 1000); //初期配置が分かるように一時停止

  for (int i = 0 ; t < stop_time ; i++) {

    if (line > 0) printf("\e[%dA", line); // カーソルを表示した分だけ上に戻す
    line = 0;

    t = i * cond.dt;
    my_update_positions(objects, objnum, cond);
    my_update_velocities(objects, objnum, cond);
    
    // 表示の座標系は width/2, height/2 のピクセル位置が原点となるようにする
    // ただし、月は地球を中心として、別スケールで描画する
    line += my_plot_objects(objects, objnum, t, cond);
    
    // シミュレーション時間が長いほどスリープ時間を短くする
    if (cond.moon) {
      usleep(1 * 1000);
    } else {
      usleep(10 * 1000 / (stop_time / (60 * 60 * 24 * 265)));
    }
  }

  return EXIT_SUCCESS;
}

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
  if (cond.moon) {
    int y[3], x[3];

    for (int i=0; i<2; i++) {
      y[i] = objs[i].y / (cond.au * 0.1) + cond.height/2 + 1;
      x[i] = objs[i].x / (cond.au * 0.1/2) + cond.width/2 + 1;
    }

    // 地球を中心としてスケールも変更
    y[2] = (objs[2].y - objs[1].y) / (cond.earth_to_moon * 0.2) + y[1];
    x[2] = (objs[2].x - objs[1].x) / (cond.earth_to_moon * 0.1) + x[1];

    for (int i=0; i<3; i++) {
      if (0 <= y[i] && y[i] < cond.height+2 && 0 <= x[i] && x[i] < cond.width+2) {
        board[y[i]][x[i]] = 'o';
      }
    }

    /*
    // 地球の周りを拡大表示していることを表す円を描画
    for (int i=0; i<cond.height; i++) {
      for (int j=0; j<cond.width; j++) {
        if (is_monotonic(6, sqrt(pow(i-y[1], 2) + pow((j-x[1])/2, 2)), 6.7)) {
          board[i][j] = '.';
        }
      }
    }
    */

  } else {

    for (int i=0; i<numobj; i++) {
      int y = objs[i].y / (cond.au * cond.scale) + cond.height/2 + 1;
      int x = objs[i].x / (cond.au * cond.scale/2) + cond.width/2 + 1;
      if (0 <= y && y < cond.height+2 && 0 <= x && x < cond.width+2) {
        board[y][x] = 'o';
      }
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
  printf("t = %4.1lf days, numobj = %zu \r\n", t / 60 / 60 / 24, numobj);
  line++;
  for (int i=0; i<numobj; i++) {
    printf("%d: .y = %6.2lf .x = %6.2lf [au] .vy = %6.3lf vx = %6.3lf [au/day]\r\n",
      i, objs[i].y / cond.au, objs[i].x / cond.au, objs[i].vy / cond.au * (60 * 60 * 24), objs[i].vx / cond.au * (60 * 60 * 24));
    line++;
  }

  return line;

}


void my_update_velocities(Object objs[], const size_t numobj, const Condition cond) {

  // 速度を更新
  for (int i=0; i<numobj; i++) {
    for (int j=0; j<numobj; j++) {
      if (i == j) continue;

      double dist = distance(objs[i], objs[j], cond);
      objs[i].vy += cond.G * objs[j].m * (objs[j].y - objs[i].y) / pow(dist, 3) * cond.dt;
      objs[i].vx += cond.G * objs[j].m * (objs[j].x - objs[i].x) / pow(dist, 3) * cond.dt;
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

void load_objects(Object objs[], size_t *numobj, char filename[], const Condition cond) {

  if (cond.moon) {

    *numobj = 3;
    objs[0] = (Object) {.m = 1.9891e30, .y = 0, .x = 0, .vy = 0, .vx = 0}; // 太陽
    objs[1] = (Object) {.m = 5.972e24, .y = 1.0162988 * cond.au, .x = 0, .vy = 0, .vx = 29290}; // 地球
    objs[2] = (Object) {.m = 7.347673e22}; // 月

    double dist = 1.0129986 * cond.earth_to_moon;
    double rad = 85.549516 / 360 * 2 * M_PI;
    double vrad = (85.558427 - 85.549516) / 360 * 2 * M_PI / 60; // 1分の変化量から1秒あたりの変化量を求める

    objs[2].x = objs[1].x + dist *  cos(rad);
    objs[2].y = objs[1].y + dist * -sin(rad);
    
    objs[2].vx = objs[1].vx + dist * vrad * -sin(rad);
    objs[2].vy = objs[1].vy + dist * vrad * -cos(rad);

    return;
  }

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't open '%s'\r\n", filename);
    exit(-1);
  }

  int buffer_len = 1000;
  char buffer[buffer_len];
  int i = 0; //objsのインデックス
  while ((fgets(buffer, buffer_len-1, fp)) != NULL) {

    // #で始まる行はコメント
    if (buffer[0] == '#') continue;

    double r, degree, v; // r[au], degree[度], v[度/日]

    sscanf(buffer, "%lf %lf %lf %lf", &objs[i].m, &degree, &r, &v);

    double dist = r * cond.au; // dist[m]
    double rad = degree / 360 * 2 * M_PI; // rad[rad]
    double vrad = v / 360 * 2 * M_PI; // vrad[rad]

    objs[i].x = dist *  cos(rad);
    objs[i].y = dist * -sin(rad); // y軸は下方向が正なので反転

    objs[i].vx = dist * vrad * -sin(rad) / (60 * 60 * 24);
    objs[i].vy = dist * vrad * -cos(rad) / (60 * 60 * 24);

    i++;
  }

  *numobj = i;

  fclose(fp);

}

double distance(Object o1, Object o2, const Condition cond) {
  return sqrt(pow(o1.y - o2.y, 2) + pow(o1.x - o2.x, 2));
}

int in_screen(double y, double x, const Condition cond) {
  return -cond.height/2 <= y && y <= cond.height/2 && -cond.width/2 <= x && x <= cond.width/2;
}

int is_monotonic(double a, double b, double c) {
  return (a <= b && b <= c) || (c <= b && b <= a);
}