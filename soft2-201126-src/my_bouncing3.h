// シミュレーション条件を格納する構造体
// 反発係数CORを追加
typedef struct condition
{
  const int width; // 見えている範囲の幅
  const int height; // 見えている範囲の高さ
  const double G; // 重力定数
  const double dt; // シミュレーションの時間幅
  const double cor; // 壁の反発係数
} Condition;

// 個々の物体を表す構造体
typedef struct object
{
  double m;
  double y, x;
  double prev_y, prev_x; // 壁からの反発に使用
  double vy, vx;
} Object;

int my_plot_objects(Object objs[], const size_t numobj, const double t, const Condition cond);
void my_update_velocities(Object objs[], const size_t numobj, const Condition cond);
void my_update_positions(Object objs[], const size_t numobj, const Condition cond);
void my_bounce(Object objs[], const size_t numobj, const Condition cond);

// 座標が画面内にあるかどうか判定する
int in_screen(double y, double x, const Condition cond);

// a, b, cがこの順に単調増加または単調減少であるかどうかを判定する
int is_monotonic(double a, double b, double c);

// オブジェクトファイルを読み込む
void load_objects(size_t numobj, Object objs[], char filename[], const Condition cond);

// 近いオブジェクト同士を融合させる
void fusion_objects(Object objs[], size_t *numobj, const Condition cond);