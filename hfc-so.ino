#include <Arduboy2.h>
Arduboy2 arduboy;
//#include <avr/pgmspace.h>

//sound
BeepPin1 beep;

//font
#include <Tinyfont.h>
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

#define GET_ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))


/*
 * 
 * マクロ定数設定
 * 
 */

#define FRAME_RATE 120  //フレームレート

//画面サイズ
#define SCALE  7
#define SCREEN_X (128<<SCALE)
#define SCREEN_Y (64<<SCALE)

//オート連射系
#define RAPID_INTERVAL 7 //オート連射抑止フレーム数

//ゲームモード
#define GAME_TITLE      0
#define GAME_PLAY       1
#define GAME_OVER       2
#define GAME_HIGH       3
#define GAME_PAUSE      4
#define GAME_START      5
#define GAME_PLAY_MISS  6
#define GAME_RESTART    7
#define DISPLAY_STATUS  8
#define GAME_READY      9

#define ZACO         0
#define HOUDAI       1
#define BATTLESHIP   2
#define IWA          3
#define ITA          4



/*
 * 
 * イメージ
 * 
 */

const byte PROGMEM my_ship[] =
{
// width, height,
7, 7,
0x7f, 0x30, 0x64, 0x44, 0x6c, 0x30, 0x7f, 0x3f, 0x30, 0x64, 0x44, 0x6c, 0x30, 0x3f,
};

const byte zacoImage[] PROGMEM  = {
// width, height,
7, 8,
//zako
0x03, 0x1f, 0x60, 0xdf, 0x7f, 0x1f, 0x03,
};

const byte PROGMEM itaImage[] =
{
// width, height,
15, 5,
0x1c, 0x06, 0x06, 0x0f, 0x02, 0x0f, 0x02, 0x16, 0x1e, 0x0f, 0x02, 0x0f, 0x06, 0x06, 0x1c,
};

const byte PROGMEM battleshipmini[] =
{
// width, height,
12, 44,
0x02, 0xfe, 0x43, 0x9f, 0xbe, 0xc3, 0x9f, 0xbe, 0xc3, 0xdf, 0xfe, 0x02,
0xff, 0x0d, 0x09, 0x7d, 0x80, 0x00, 0xfa, 0x9f, 0xe0, 0xfd, 0xf9, 0xff,
0x1f, 0xf0, 0x03, 0x00, 0xb4, 0x21, 0x01, 0xff, 0xaf, 0xff, 0xff, 0x1f,
0x00, 0xff, 0x00, 0xfe, 0x88, 0x15, 0x17, 0x8f, 0xfe, 0xff, 0xff, 0x00,
0xc0, 0xff, 0x00, 0xff, 0x00, 0x01, 0xc1, 0x00, 0xff, 0xff, 0xff, 0xc0,
0x03, 0x07, 0x08, 0x09, 0x0a, 0x0e, 0x0a, 0x0e, 0x0f, 0x0b, 0x07, 0x03,

};

const byte PROGMEM iwaImage[] =
{
// width, height,
24, 18,
0x80, 0xf8, 0x78, 0x14, 0x9c, 0xf6, 0xee, 0x7e, 0x7a, 0xff, 0x57, 0x7f, 0xff, 0xff, 0x7f, 0xff, 0xfe, 0xfe, 0x7e, 0xbe, 0xfe, 0x9e, 0xf8, 0xe0,
0x0b, 0x60, 0x81, 0x00, 0xc7, 0x00, 0x1c, 0x38, 0x80, 0x18, 0x70, 0x60, 0x79, 0x7f, 0xc7, 0xf1, 0x0f, 0xf9, 0xff, 0x7f, 0x19, 0x5d, 0xff, 0x07,
0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x03, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00,
};

const byte PROGMEM houdaiImage[] =
{
// width, height,
18, 8,
0x16, 0x0a, 0x16, 0x0a, 0x57, 0x22, 0x20, 0x11, 0xd7, 0x3f, 0x2e, 0x27, 0x4a, 0x16, 0x0a, 0x16, 0x0a, 0x1e,
};

//オブジェクトカタログ
#define OBJECT_DATA_LENGTH 6
const int objectCatalog [] PROGMEM = {
//  {HP, size_x, size_y, pattern, score}
    1,(7<<SCALE),(8<<SCALE),ZACO,1,&zacoImage,
    15,(18<<SCALE),(8<<SCALE),HOUDAI,5,&houdaiImage,
    40,(12<<SCALE),(44<<SCALE),BATTLESHIP,100,&battleshipmini,
    255,(24<<SCALE),(18<<SCALE),IWA,0,&iwaImage,
    15,(15<<SCALE),(5<<SCALE),ITA,10,&itaImage,
};

//配列呼び出し用マクロ引数
#define X        1
#define Y        2
#define VX       3
#define VY       4
#define E_TYPE   5
#define E_HP     6
#define E_FRAME  7


/*
 * 
 * 三角関数
 * 
 */
int angle;
//sinの参照テーブル作成
//http://raohu69.hatenablog.jp/entry/2018/04/07/155658
const uint8_t sinTbl[] PROGMEM = {
0x00, 0x06, 0x0c, 0x12, 0x19, 0x1f, 0x25, 0x2b, 0x31, 0x38, 0x3e, 0x44, 0x4a, 0x50, 0x56, 0x5c,
0x61, 0x67, 0x6d, 0x73, 0x78, 0x7e, 0x83, 0x88, 0x8e, 0x93, 0x98, 0x9d, 0xa2, 0xa7, 0xab, 0xb0,
0xb5, 0xb9, 0xbd, 0xc1, 0xc5, 0xc9, 0xcd, 0xd1, 0xd4, 0xd8, 0xdb, 0xde, 0xe1, 0xe4, 0xe7, 0xea,
0xec, 0xee, 0xf1, 0xf3, 0xf4, 0xf6, 0xf8, 0xf9, 0xfb, 0xfc, 0xfd, 0xfe, 0xfe, 0xff, 0xff, 0xff,
};

//三角関数呼出？
int16_t GetSin(uint8_t angle)
{
  int16_t result = 256; // 1 << 8
  uint8_t n = angle & 0x7f;
  if (n < 64) {
    result = pgm_read_byte(sinTbl + n);
  } else if (n > 64) {
    result = pgm_read_byte(sinTbl + 128 - n);
  }
  return (angle > 128) ? -result : result;
}

int16_t GetCos(uint8_t angle)
{
  return GetSin(angle + 64);
}



const unsigned char tiles[] PROGMEM  = {
// width, height,
16, 16,

//Grass
0xff, 0x7f, 0xfb, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xf7, 0xff, 0xfd, 0xff, 0xff, 0xf7, 0x7f, 0xff, 
0xdf, 0xff, 0xff, 0xfb, 0x7f, 0xff, 0xff, 0xff, 0xef, 0xfe, 0xff, 0xff, 0xfb, 0xff, 0x7f, 0xff, 

//Water
0x08, 0x10, 0x10, 0x08, 0x10, 0x08, 0x10, 0x10, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x20, 0x40, 0x40, 0x20, 0x00, 0x01, 0x02, 0x02, 0x01, 0x02, 0x02, 0x01, 0x02, 0x21, 0x40, 0x40, 

//Tree
0xff, 0x1f, 0x5b, 0x3f, 0xeb, 0xdd, 0xff, 0xf7, 0xbb, 0xef, 0xfd, 0x7f, 0xe3, 0xcb, 0xe3, 0xff, 
0xff, 0xc7, 0x96, 0xc7, 0xff, 0xff, 0xef, 0xfd, 0xff, 0xe3, 0xcb, 0xe3, 0xff, 0xff, 0x7b, 0xff, 

//Stone
0xff, 0xdf, 0x7b, 0x3f, 0x9f, 0x6f, 0x77, 0xab, 0xdb, 0xd7, 0xcd, 0x5f, 0xbf, 0x77, 0xff, 0xff, 
0xff, 0xc1, 0xdc, 0xd3, 0xaf, 0x9f, 0xae, 0xb0, 0xbb, 0xbd, 0xbd, 0xba, 0xd7, 0xcc, 0x63, 0xff, 
};

#define WORLD_WIDTH    14
#define WORLD_HEIGHT  7
#define GRASS      0
#define WATER      1
#define TREES      2
#define STONE      3

int world[WORLD_HEIGHT][WORLD_WIDTH] = {
  { TREES, GRASS, GRASS, WATER, GRASS, GRASS, GRASS, TREES, GRASS, GRASS, GRASS, GRASS, GRASS, TREES },
  { GRASS, WATER, WATER, WATER, GRASS, WATER, GRASS, GRASS, GRASS, GRASS, GRASS, STONE, GRASS, GRASS },
  { GRASS, GRASS, GRASS, GRASS, GRASS, WATER, STONE, GRASS, GRASS, GRASS, TREES, GRASS, GRASS, GRASS },
  { STONE, GRASS, GRASS, STONE, TREES, WATER, WATER, WATER, GRASS, WATER, WATER, GRASS, TREES, GRASS },
  { GRASS, GRASS, GRASS, GRASS, TREES, GRASS, GRASS, GRASS, TREES, WATER, GRASS, GRASS, STONE, TREES },
  { GRASS, GRASS, GRASS, WATER, STONE, GRASS, GRASS, TREES, TREES, TREES, GRASS, GRASS, WATER, WATER },
  { GRASS, WATER, WATER, TREES, GRASS, WATER, WATER, TREES, TREES, GRASS, GRASS, GRASS, GRASS, STONE }
};












//初期モード設定
static byte gamestate = GAME_TITLE;

static byte gray_show = 0;
static unsigned int scroll  = 0;

static unsigned int score = 0;
static unsigned int hiScore = 0;

//オート連射系
static byte rapid = 0; //オート連射間隔カウント用

//画面揺れ用
static struct
 // {ベクトルの大きさ、効果x,効果y},
  {
    byte frame;
    int x;
    int y;
  } screenShake = { 0,0,0 };
  
//画面揺れ用
static struct
 // {ベクトルの大きさ、効果x,効果y},
  {
    byte frame;
    byte priority;
  } soundEffect = { 0,0 };

//自機関係
#define MY_SIZE_X (7<<SCALE)
#define MY_SIZE_Y (7<<SCALE)
#define MY_GUNPOINT_X (MY_SIZE_X / 2)
#define MY_GUNPOINT_Y (MY_SIZE_Y / 2)
static int my_x = 0;
static int my_y = 0;
static byte shipStock = 0;

//自機ショット
static struct
 // {power&有効/無効,st_x,st_y,ベクトルX,ベクトルY},
 //   {1,SCREEN_X-1,SCREEN_Y-1,0,0}
  {
    byte power;
    int x;
    int y;
    int vx;
    int vy;
  } myShot[10] = { 0,0,0,0,0 };

//自機ショットタイプ
#define MYSHOT_TYPES 5
#define MYSHOT_DATA_LENGTH 3
//const signed int myshotCatalog [] PROGMEM = {
const int16_t myshotCatalog [] PROGMEM = {
 // {ダメージ＆有効/無効=0,vector_x,vector_y},
    3,
    3,0,-4,
    2,-3,-3,//ななめ前方
    2,3,-3,
    2,-3,3,//ななめ後方
    2,3,3
};

//オブジェクト
static struct
 // {状態&有効/無効,st_x,st_y,vector_x,vector_y,objectCatalog,HP,フレーム数},
//    {1,50<<SCALE,0<<SCALE,0,0,0,1,0}
  {
    byte status;
    int x;
    int y;
    int vx;
    int vy;
    byte catalog;
    byte hp;
//    int catalog;
//    int hp;
    int frame;
  } object[20] = { 0,0,0,0,0,0,0,0 };

//オブジェクトショット
static struct
 // {power&有効/無効,st_x,st_y,ベクトルX,ベクトルY},
 //   {1,SCREEN_X-1,SCREEN_Y-1,0,0}
  {
    byte power;
    int x;
    int y;
    int vx;
    int vy;
  } objectShot[50] = { 0,0,0,0,0 };

//背景星制御
static struct
 // {SPEED&有効/無効,st_x,st_y,ベクトルX,ベクトルY},
  {
    byte speed; 
    char x;
    char y;
  } star[10] = { 0,0,0 };




/*
 * 
 * ステージ構成関係
 * 
 */

//gameMaster
//ゲーム難易度コントロール変数群
    static byte objectShotLimit = 0;
    static byte objectLimit = 0;
    static byte objectShotFrequency = 1;
    static byte objectShotSpeed = 1;
    static byte objectPercent [] {255,0,0,0,0,0}; //乱数発生の比率設定用


//stage構成関係
static byte stage = 0;
static int section = 0;
static int part = 0;
static int termLimit = 0;

//ステージ構成
#define STAGE_DATA_LENGTH 6
const byte PROGMEM stageCatalog[] =
//出現パターン,　オブジェクト（敵）,
{
};

//セクション構成
#define SECTION_DATA_LENGTH 6
const byte PROGMEM sectionCatalog[] =
//出現パターン,　オブジェクト（敵）,
{
};

//パート構成
#define PART_DATA_LENGTH 6
const byte PROGMEM partCatalog[] =
//出現パターン,　オブジェクト（敵）, タームリミット
{
};

//オブジェクト出現セット
#define OBJECTSET_DATA_LENGTH 6
const byte PROGMEM objectSetCatalog[] =
//出現パターン,　オブジェクト（敵）,
{
};

void screenShakeProcess () {
  if (screenShake.frame > 0) {
    screenShake.x = random(0, log(screenShake.frame)) - (log(screenShake.frame/2));
    screenShake.y = random(0, log(screenShake.frame)) - (log(screenShake.frame/2));
    screenShake.frame--;
  } else {
    screenShake.x = 0;
    screenShake.y = 0;
  }
}
void pattern (int i) {
  switch (objectCatalogRead(object[i].catalog, 3)) {
    case ZACO:
      //初期化
      if (object[i].frame == 0) {
        object[i].x = random(1 ,SCREEN_X);                           //初期位置x
        object[i].y = 0 - (objectCatalogRead(object[i].catalog, Y));       //初期位置y
        object[i].vx = (my_x - object[i].x) / 100;
        object[i].vy = 50;
      }
      if (random(0 ,3000) < 2 * objectShotFrequency) {objectShotGenerator (i, objectShotSpeed, 2);}
 //     Sprites::drawSelfMasked((object[i].x)>>SCALE, (object[i].y)>>SCALE, zacoImage, 0);
//      Sprites::drawSelfMasked((object[i].x)>>SCALE, (object[i].y)>>SCALE, objectCatalogRead(object[i].catalog, 5), 0);
      break;

    case BATTLESHIP:
      if (object[i].frame == 0) {
        object[i].x = random(1 ,SCREEN_X);                           //初期位置x
        object[i].y = 0 - (objectCatalogRead(object[i].catalog, Y));       //初期位置y
        object[i].vx = (random(0 ,10) - 5);
        object[i].vy = (8 + (random(0 ,5)));
      }
      else if (object[i].frame > 500) {
        if (random(0 ,200) < 2 * objectShotFrequency) {objectShotGenerator (i, 5*objectShotSpeed, 3);}
      }
//      Sprites::drawSelfMasked((object[i].x)>>SCALE, (object[i].y)>>SCALE, battleshipmini, 0);
      break;

    case ITA:
      if (object[i].frame == 0) {
        object[i].x = random(1 ,SCREEN_X);                           //初期位置x
        object[i].y = 0 - (objectCatalogRead(object[i].catalog, Y));       //初期位置y
        object[i].vy = 100;
      }
      else if (object[i].frame > 30) {
        object[i].vy -=  random(1,3);
        if (random(0 ,500) < 10 * objectShotFrequency) {objectShotGenerator (i, objectShotSpeed, 2);}
      }
//      Sprites::drawSelfMasked((object[i].x)>>SCALE, (object[i].y)>>SCALE, itaImage, 0);
      break;

    case IWA:
      if (object[i].frame == 0) {
        object[i].x = random(1 ,SCREEN_X);                           //初期位置x
        object[i].y = 0 - (objectCatalogRead(object[i].catalog, Y));       //初期位置y
        object[i].vx = (random(0 ,10) - 5);
        object[i].vy = (20 + (random(0 ,10)));
      }
//      Sprites::drawSelfMasked((object[i].x)>>SCALE, (object[i].y)>>SCALE, iwaImage, 0);
      break;

    case HOUDAI:
      if (object[i].frame == 0) {
        object[i].x = random(1 ,SCREEN_X);                           //初期位置x
        object[i].y = 0 - (objectCatalogRead(object[i].catalog, Y));       //初期位置y
        object[i].vy = 80;
      }
      else if (10 <= object[i].frame && object[i].frame < 51) {object[i].vy -= 2;}
      if (object[i].frame > 40) {
        if (random(0 ,500) < 2 * objectShotFrequency) {objectShotGenerator (i, objectShotSpeed, 4);}
      }
//      Sprites::drawSelfMasked((object[i].x)>>SCALE, (object[i].y)>>SCALE, houdaiImage, 0);
      break;

   }
  object[i].x += object[i].vx;
  object[i].y += object[i].vy;
  object[i].frame++;  //フレーム加算
}




/*
 * 
 * ルーチン
 * 
 */
//オブジェクトステータス
#define ENEMY_EXPLOSION 90     //敵爆発  （自機x、自機弾x、敵x、弾x）
#define CHAIN_EXPLOSION 91     //連鎖爆発（自機x、自機弾x、敵○、弾x）
#define FAIRE_EXPLOSION 92     //爆発ALL （自機○、自機弾x、敵○、弾x）
#define NUCRLEA_EXPLOSION 93   //核爆発  （自機○、自機弾○、敵○、弾○）
#define ENEMY 10               //敵     （自機○、自機弾○、敵x、弾x）
#define ENEMY_SHOT 11          //敵弾   （自機○、自機弾x、敵x、弾x）
#define ENEMY_LASER 12         //敵レーザ（自機○、自機弾x、敵x、弾x）
#define BLOCK 90               //障害物  （自機○、自機弾○、敵x、弾x）
#define FAIRE_BLOCK 91         //障害物  （自機○、自機弾○、敵○、弾x）
#define BACKGROUND 30          //背景   （自機x、自機弾x、敵x、弾x）
#define ALLY 20                //味方   （自機x、自機弾x、敵○、弾○）
#define ALLY_SHOT 21           //味方弾  （自機x、自機弾x、敵○、弾x）
#define ALLY_LASER 22          //味方弾  （自機x、自機弾x、敵○、弾x）
    

//プレイヤー表示
void playerDraw() {
  Sprites::drawOverwrite((my_x - MY_GUNPOINT_X)>>SCALE, (my_y - MY_GUNPOINT_Y)>>SCALE, my_ship, 0);
  //オブジェジェクト当たり判定
  for (int i = 0; i < GET_ARRAY_SIZE(object); i++) {
    if (object[i].status == 1) {
      if (object[i].x <= my_x && my_x <= object[i].x + objectCatalogRead(object[i].catalog, X)) {
        if (object[i].y <= my_y && my_y <= object[i].y + objectCatalogRead(object[i].catalog, Y)) {
          gamestate = GAME_PLAY_MISS;
          rapid = 0;//爆発フレーム計算用
          boomSE(300);
        }
      }
    }
  }
  //オブジェクト弾当たり判定
  for (int i = 0; i < GET_ARRAY_SIZE(objectShot); i++) {
    if (!objectShot[i].power) {continue;}
      if (my_x-256 <= objectShot[i].x && objectShot[i].x <= my_x + 256) {
        if (my_y-256 <= objectShot[i].y && objectShot[i].y <= my_y + 256) {
          gamestate = GAME_PLAY_MISS;
          rapid = 0;//爆発フレーム計算用
      }
    }
  }
}

//プレイヤー弾処理
void playerShotProcess() {
  for (int i = 0; i < GET_ARRAY_SIZE(myShot); i++) {
    if (!myShot[i].power) {continue;}
    myShot[i].x += myShot[i].vx;
    myShot[i].y += myShot[i].vy;
    if (screenOver(myShot[i].x, myShot[i].y, 0, 0)) {myShot[i].power = 0;}
  }
}

//プレイヤー弾表示
void playerShotDraw() {
  for (int i = 0; i < GET_ARRAY_SIZE(myShot); i++) {
    if (!myShot[i].power) {continue;}
    arduboy.fillRect(myShot[i].x>>SCALE, myShot[i].y>>SCALE, 1, 1, 1); //弾丸表示
  }
}

//オブジェクト処理
void objectProcess() {
  for (int i = 0; i < GET_ARRAY_SIZE(object); i++) {
    switch (object[i].status) {
      case 1:
        //object行動処理
        pattern(i);
        if (screenOver(object[i].x, object[i].y, objectCatalogRead(object[i].catalog, X), objectCatalogRead(object[i].catalog, Y))) {
          object[i].status = 0;
        }
        break;
      
      case 2:
        //爆発パターン処理
        object[i].x += object[i].vx;
        object[i].x += object[i].vy;
        if (object[i].catalog <= object[i].frame) {object[i].status = 0;}
        object[i].frame++;
        break;
    }
  }
}

//objectとplayerShotの衝突判定
void playerShotCheck () {
  for (int i = 0; i < GET_ARRAY_SIZE(object); i++) {
    switch (object[i].status) {
      case 1:
        for (int j = 0; j < GET_ARRAY_SIZE(myShot); j++) {
          if (!myShot[j].power) {continue;}
          if (object[i].x <= myShot[j].x && myShot[j].x <= object[i].x + objectCatalogRead(object[i].catalog, 1)) {
            if (object[i].y <= myShot[j].y && myShot[j].y <= object[i].y + objectCatalogRead(object[i].catalog, 2)) {
              if (objectCatalogRead(object[i].catalog, 0) ==  255) {
                arduboy.fillCircle (myShot[j].x>>SCALE, myShot[j].y>>SCALE, 4, WHITE);
              } else if (minusCheck(object[i].hp - myShot[j].power)) {
                object[i].hp = 0;
                myShot[j].power = 0;//弾消滅
                score += objectCatalogRead(object[i].catalog, 4);
                //爆発パターン用パラメータ再設定
                boomSE((objectCatalogRead(object[i].catalog, 1) + objectCatalogRead(object[i].catalog, 2))>>SCALE);
                object[i].status = 2;
                object[i].x = object[i].x + (objectCatalogRead(object[i].catalog, 1) / 2);
                object[i].y = object[i].y + (objectCatalogRead(object[i].catalog, 2) / 2);
                object[i].hp = (((objectCatalogRead(object[i].catalog, 1)) + (objectCatalogRead(object[i].catalog, 2))) / 2)>>SCALE ;//爆発の大きさ
                object[i].catalog = ((objectCatalogRead(object[i].catalog, 1)) + (objectCatalogRead(object[i].catalog, 2)))>>SCALE;//爆発フレーム数
                object[i].frame = 0;//frame初期化
                break;
              } else {
                object[i].hp -= myShot[j].power;
                arduboy.fillCircle (myShot[j].x>>SCALE, myShot[j].y>>SCALE, 4, WHITE);
              }
              myShot[j].power = 0;//弾消滅
            }
          }
        }
        break;
      
    }
  }
}

//オブジェクト表示
void objectDraw() {
  for (int i = 0; i < GET_ARRAY_SIZE(object); i++) {
    switch (object[i].status) {
      case 1:
        Sprites::drawSelfMasked(((object[i].x)>>SCALE) + screenShake.x, ((object[i].y)>>SCALE) + screenShake.y, objectCatalogRead(object[i].catalog, 5), 0);
        break;
      case 2:
        //爆発パターン表示
        arduboy.fillCircle(object[i].x>>SCALE, object[i].y>>SCALE, (object[i].hp) * ((object[i].catalog) - (object[i].frame)) / (object[i].catalog), WHITE);
    }
  }
}

//オブジェクト弾処理
void objectShotProcess() {
  for (int i = 0; i < GET_ARRAY_SIZE(objectShot); i++) {
    if (!objectShot[i].power) {continue;}
    if (objectShot[i].power <= 2) {
      objectShot[i].x += objectShot[i].vx;
      objectShot[i].y += objectShot[i].vy;
    } else {
      objectShot[i].x += objectShot[i].vx;
      objectShot[i].y += objectShot[i].vy;
    }
    if (screenOver(objectShot[i].x, objectShot[i].y, 0, 0)) {objectShot[i].power = 0;}
  }
}

//オブジェクト弾表示
void objectShotDraw () {
  for (int i = 0; i < GET_ARRAY_SIZE(objectShot); i++) {
    if (!objectShot[i].power) {continue;}
    if (objectShot[i].power <= 2) {
      arduboy.fillRect(objectShot[i].x>>SCALE, objectShot[i].y>>SCALE, objectShot[i].power, objectShot[i].power, WHITE); //弾丸表示
    } else {
      if (gray_show == 0) {//グレー表示処理（０）
        arduboy.fillCircle ((objectShot[i].x - objectShot[i].vx*4)>>SCALE, (objectShot[i].y - objectShot[i].vy* 4)>>SCALE, objectShot[i].power-1, WHITE);
      }
      arduboy.fillCircle(objectShot[i].x>>SCALE, objectShot[i].y>>SCALE, objectShot[i].power,  WHITE); //弾丸表示
    }
  }
}

//オブジェクトジェネレータ
void objectGenerator () {
  //object generator
  for (int i=0; i < objectLimit; i++) {
    if (object[i].status == 0) {
      int objectTypeSeed = random(1 ,254);
      //objectPercent
      for (int j=0; j < GET_ARRAY_SIZE(objectPercent); j++) {
        if (objectTypeSeed <= objectPercent[j]) {
          object[i].status = 1;
          object[i].catalog = j;  //objectCatalog
          object[i].x = 0;         //初期位置x
          object[i].y = 0;         //初期位置y
          object[i].vx = 0;                                            //vector_x
          object[i].vy = 0;                                            //vector_y
          object[i].hp = objectCatalogRead(object[i].catalog, 0);      //HP
          object[i].frame = 0;                                        //frame初期化
          break;
        }
      }
      break;
    }
  }
}

//オブジェクト弾ジェネレータ
void objectShotGenerator (int i, int speed, int power) {
  //敵弾生成
  switch (power) {
    case 4: //４方向ショット
      for (int k=random(0, 2)* 32; k<=255; k+=64) {
        for (int j=0; j < objectShotLimit; j++) {
          if (objectShot[j].power == 0) {
            objectShot[j].power = 2;
            objectShot[j].x = object[i].x + (objectCatalogRead(object[i].catalog, 1) / 2);
            objectShot[j].y = object[i].y + (objectCatalogRead(object[i].catalog, 2) / 2);
            objectShot[j].vx = (GetCos(k) * speed)>>8;
            objectShot[j].vy = (GetSin(k) * speed)>>8;
            break;
          }
        }
      }
      break;

    case 8://８方向ショット
      for (int k=random(0, 32); k<=255; k+=32) {
        for (int j=0; j < objectShotLimit; j++) {
          if (objectShot[j].power == 0) {
            objectShot[j].power = 2;
            objectShot[j].x = object[i].x + (objectCatalogRead(object[i].catalog, 1) / 2);
            objectShot[j].y = object[i].y + (objectCatalogRead(object[i].catalog, 2) / 2);
            objectShot[j].vx = (GetCos(k) * speed)>>8;
            objectShot[j].vy = (GetSin(k) * speed)>>8;
            break;
          }
        }
      }
      break;

    default ://自機狙いショット
      for (int j=0; j < objectShotLimit; j++) {
        if (objectShot[j].power == 0) {
          objectShot[j].x = object[i].x + (objectCatalogRead(object[i].catalog, 1) / 2);
          objectShot[j].y = object[i].y + (objectCatalogRead(object[i].catalog, 2) / 2);
          objectShot[j].power = power;
          objectShot[j].vx = ((my_x - objectShot[j].x)>>SCALE)*speed / ((abs(my_y - objectShot[j].y) + abs(my_x - objectShot[j].x))>>SCALE);
          objectShot[j].vy = ((my_y - objectShot[j].y)>>SCALE)*speed / ((abs(my_y - objectShot[j].y) + abs(my_x - objectShot[j].x))>>SCALE);
          break;
        }
      }
  }
}

//背景星制御
void starProcess () {
  //star generator
  if (gray_show == 1) {
    for (int i=0; i < GET_ARRAY_SIZE(star); i++) {
      star[i].y += star[i].speed;
      if (star[i].y >= SCREEN_Y>>SCALE || star[i].speed == 0) {
        star[i].speed = random(3 ,25);              //speed
        star[i].x = random(1 ,SCREEN_X>>SCALE); //x
        star[i].y -= SCREEN_Y>>SCALE;           //y
      }
      arduboy.fillRect(star[i].x, star[i].y, 1, 1, 1); //星表示
    }
  }
}


/*
 * ゲーム展開設定処理
 */
void gameMaster () {
  if (scroll == 0) {
    objectLimit = 0;
    objectShotLimit = 0;
    objectShotFrequency = 1;
    objectShotSpeed = 20;   
    objectPercent[ZACO] = 0;
    objectPercent[HOUDAI] = 0;
    objectPercent[BATTLESHIP] = 0;
    objectPercent[IWA] = 0;
    objectPercent[ITA] = 0;

  } else if (scroll == 100) {
     //ファーストコンタクト
    objectLimit = GET_ARRAY_SIZE(object) - 15;
    objectShotLimit = GET_ARRAY_SIZE(objectShot) -25;
    objectShotFrequency = 1;
    objectShotSpeed = 20;   
    objectPercent[ZACO] = 100;
    objectPercent[HOUDAI] = 0;
    objectPercent[BATTLESHIP] = 0;
    objectPercent[IWA] = 0;
    objectPercent[ITA] = 0;

  } else if (scroll == 600) {
    //戦艦登場
//    objectLimit = GET_ARRAY_SIZE(object) - 15;
//    objectShotLimit = GET_ARRAY_SIZE(objectShot) - 25;
//    objectShotFrequency = 1;
//    objectShotSpeed = 1;
    objectPercent[ZACO] = 230;
    objectPercent[HOUDAI] = 250;
    objectPercent[BATTLESHIP] = 255;
    objectPercent[IWA] = 0;
    objectPercent[ITA] = 0;

  } else if (scroll == 3000) {
    //岩登場
    objectLimit = GET_ARRAY_SIZE(object) -10;
//    objectShotLimit = GET_ARRAY_SIZE(objectShot) - 25;
//    objectShotFrequency = 2;
//    objectShotSpeed = 1;
    objectPercent[ZACO] = 230;
    objectPercent[HOUDAI] = 240;
    objectPercent[BATTLESHIP] = 250;
    objectPercent[IWA] = 255;
    objectPercent[ITA] = 0;

  } else if (scroll == 5000) {
    //岩地帯
//    objectLimit = GET_ARRAY_SIZE(object) - 10;
    objectShotLimit = GET_ARRAY_SIZE(objectShot) - 15;
//    objectShotFrequency = 5;
//    objectShotSpeed = 1;
    objectPercent[ZACO] = 190;
    objectPercent[HOUDAI] = 200;
    objectPercent[BATTLESHIP] = 220;
    objectPercent[IWA] = 250;
    objectPercent[ITA] = 255;

  } else if (scroll == 9000) {
    //戦艦ラッシュ
//    objectLimit = GET_ARRAY_SIZE(object) - 10;
//    objectShotLimit = GET_ARRAY_SIZE(objectShot) - 15;
//    objectShotFrequency = 3;
//    objectShotSpeed = 2;
    objectPercent[ZACO] = 200;
    objectPercent[HOUDAI] = 220;
    objectPercent[BATTLESHIP] = 240;
    objectPercent[IWA] = 250;
    objectPercent[ITA] = 255;

  } else if (scroll == 12000) {
    //板ラッシュ
//    objectLimit = GET_ARRAY_SIZE(object) - 10;
//    objectShotLimit = GET_ARRAY_SIZE(objectShot) - 15;
//    objectShotFrequency = 3;
//    objectShotSpeed = 2;
    objectPercent[ZACO] = 150;
    objectPercent[HOUDAI] = 170;
    objectPercent[BATTLESHIP] = 190;
    objectPercent[IWA] = 200;
    objectPercent[ITA] = 255;

  } else if (scroll == 18000) {
    //擲弾リミット解除
    objectLimit = GET_ARRAY_SIZE(object) - 5;
    objectShotLimit = GET_ARRAY_SIZE(objectShot) ;
//    objectShotFrequency = 3;
//    objectShotSpeed = 2;
    objectPercent[ZACO] = 150;
    objectPercent[HOUDAI] = 170;
    objectPercent[BATTLESHIP] = 190;
    objectPercent[IWA] = 220;
    objectPercent[ITA] = 255;

  } else if (scroll == 25000) {
    //敵数リミット解除＆敵弾頻度上昇
    objectLimit = GET_ARRAY_SIZE(object) ;
//    objectShotLimit = GET_ARRAY_SIZE(objectShot);
    objectShotFrequency = 2;
//    objectShotSpeed = 2;
    objectPercent[ZACO] = 150;
    objectPercent[HOUDAI] = 170;
    objectPercent[BATTLESHIP] = 190;
    objectPercent[IWA] = 220;
    objectPercent[ITA] = 255;

  } else if (scroll == 35000) {
 //敵弾速度上昇
//    objectLimit = GET_ARRAY_SIZE(object);
//    objectShotLimit = GET_ARRAY_SIZE(objectShot);
//    objectShotFrequency = 2;
    objectShotSpeed = 2;
    objectPercent[ZACO] = 150;
    objectPercent[HOUDAI] = 170;
    objectPercent[BATTLESHIP] = 190;
    objectPercent[IWA] = 220;
    objectPercent[ITA] = 255;

  }
}
#define TILE_SIZE  16
int mapx = 50;
int mapy = 0;
void drawworld() {

  const int tileswide = WIDTH / TILE_SIZE + 1;
  const int tilestall = HEIGHT / TILE_SIZE + 1;

  for (int y = 0; y < tilestall; y++) {
    for (int x = 0; x < tileswide; x++) {
      const int tilex = x - mapx / TILE_SIZE;
      const int tiley = y - mapy / TILE_SIZE;
      if (tilex >= 0 && tiley >= 0 && tilex < WORLD_WIDTH && tiley < WORLD_HEIGHT) {
        Sprites::drawOverwrite(x * TILE_SIZE + mapx % TILE_SIZE, y * TILE_SIZE + mapy % TILE_SIZE, tiles, world[tiley][tilex]);
      }
    }
  }

  arduboy.fillRect(0, 0, 48, 8, BLACK);
  arduboy.setCursor(0, 0);
  arduboy.print(mapx);
  arduboy.print(",");
  arduboy.print(mapy);


}
void topInformation () {
  tinyfont.setCursor(0 + screenShake.x, 0 + screenShake.y);
  tinyfont.print("H ");
  tinyfont.print(shipStock);
  tinyfont.setCursor(60 + screenShake.x, 0 + screenShake.y);
  tinyfont.print(score);
  tinyfont.print("00");
  tinyfont.setCursor(60 + screenShake.x, 55 + screenShake.y);
  tinyfont.print((scroll/(120*60))/2);
  tinyfont.print(":");
  tinyfont.print((scroll/(120))/2);
  
}

void gameplay() {
  gameMaster ();
  playerinput();
/*
  if (gray_show == 1) {
    drawworld();
  }
*/
  playerDraw();
  playerShotProcess();
  playerShotCheck();
  playerShotDraw();
  objectProcess();
  objectDraw();
  objectShotProcess();
  objectShotDraw();
  objectGenerator();
  topInformation ();
  starProcess();
  screenShakeProcess();
  rapid++;
  scroll++;
}

void gameplay_miss() {
  playerShotProcess();
  playerShotDraw();
  objectProcess();
  objectDraw();
  objectShotProcess();
  objectShotDraw();
  if (rapid > 250) {
    if (shipStock <= 0) {
      gamestate = GAME_OVER;
    } else {
      shipStock--;
      gamestate = GAME_RESTART;
    }
  } else if (rapid < 50) {
    arduboy.fillCircle(my_x>>SCALE, my_y>>SCALE, 10*(50-rapid)/50, WHITE);
  }
  topInformation ();
  starProcess();
  screenShakeProcess();
  rapid++;
}

void gameplay_pause() {
  playerDraw();
  playerShotDraw();
  objectDraw();
  objectShotDraw();
  arduboy.setCursor(50, 30);
  arduboy.print("PAUSE");
  topInformation ();
  starProcess();
  if (arduboy.justReleased(B_BUTTON)) {
    gamestate = GAME_PLAY;
  }
}

void game_ready() {
  playerinput();
  playerDraw();
  playerShotProcess();
  playerShotCheck();
  playerShotDraw();
  objectDraw();
  objectShotDraw();
  arduboy.setCursor(50, 30);
  arduboy.print("ready");
  topInformation ();
  starProcess();

  if (object[0].x <= 0) {
    gamestate = GAME_PLAY;
  }
   object[0].x--;
}

void titlescreen() {
  starProcess();
/*
  if (gray_show == 0) {
    drawworld();
  }
*/


arduboy.setTextSize(2);
  arduboy.setCursor(0, 0);
  arduboy.print("HFC-SO");
  tinyfont.setCursor(0, 15);
//  char str [] = {'S','t','a','r',' ','S','o','l','d','u','e','r',0};
  angle += 2;
//  int str = GetSin(angle);
//  int str = GetCos(angle);
  double aaa = (((10)*GetSin(angle))>>8);
  tinyfont.print(GetSin(angle)/255);
  tinyfont.print(":");
  tinyfont.print(GetCos(angle)/255);
   tinyfont.setCursor(aaa+60, ((10*GetCos(angle))>>8)+20);
  tinyfont.print("O");
 
  arduboy.setTextSize(1);
  arduboy.setCursor(0, 25);
  arduboy.print("Highscore\n");
  if (hiScore > 0) {
    arduboy.print(hiScore);
    arduboy.print("00");
  } else {
    arduboy.print("0\n");
  }
  arduboy.setCursor(0, HEIGHT-10);
  arduboy.print("PUSH A START\n");
  if (arduboy.justPressed(A_BUTTON)) {
    gamestate = GAME_START;
  }
}


void gameoverscreen() {
  objectProcess();
  objectDraw();
  objectShotProcess();
  objectShotDraw();
  starProcess();
  arduboy.setTextWrap(true);
  arduboy.setCursor(40, 30);
  arduboy.print("Game Over");
  if (arduboy.justPressed(A_BUTTON)) {
    gamestate = GAME_HIGH;
  }
  topInformation ();
}

void highscorescreen() {
  starProcess();
  arduboy.setCursor(0, 10);
  arduboy.print("Highscore\n");
  if (hiScore > 0) {
    arduboy.print(hiScore);
    arduboy.print("00");
  } else {
    arduboy.print("0");
  }
  arduboy.setCursor(0, 35);
  arduboy.print("Score\n");
  if (score > 0) {
    arduboy.print(score);
    arduboy.print("00");
  } else {
    arduboy.print("0");
  }
  if (arduboy.justPressed(A_BUTTON)) {
    if (hiScore < score) {hiScore = score;}
    gamestate = GAME_TITLE;
  }
}

//テスト用ステータス表示
void testDisplay () {
  for (int i=0; i<GET_ARRAY_SIZE(objectShot); i++) {
    tinyfont.setCursor(0, 5 + (i * 5));
    tinyfont.print(i);
    tinyfont.print(':');
    tinyfont.print(objectShot[i].vx);
    tinyfont.setCursor(30, 5 + (i * 5));
    tinyfont.print(objectShot[i].vy);
  }
/*
  for (int i=0; i<GET_ARRAY_SIZE(myShot); i++) {
    tinyfont.setCursor(30, 5 + (i * 5));
    tinyfont.print(i);
    tinyfont.print(':');
    tinyfont.print(myShot[i].power);
  }
*/
  if (arduboy.justPressed(B_BUTTON)) {
    gamestate = GAME_PLAY;
  }

}

//ゲーム状態による分岐
void gameloop() {

  switch(gamestate) {

    case GAME_TITLE:
      titlescreen();
      break;

    case GAME_START:
      score = 0;
      shipStock = 1;
      scroll = 0;

    case GAME_RESTART: //死んだ後
      rapid = 0;
      my_x = SCREEN_X / 2;
      my_y = SCREEN_Y * 0.8;
      for (int i = 0; i < GET_ARRAY_SIZE(objectShot); i++) {
        objectShot[i].power = 0;
      }
      for (int i = 0; i < GET_ARRAY_SIZE(object); i++) {
        object[i].status = 0;
      }
      object[0].x = 200;  //ゲーム開始タイミングの長さ
      gamestate = GAME_READY;
      break;

    case DISPLAY_STATUS:
      testDisplay();

    case GAME_PLAY:
      gameplay();
      break;

    case GAME_PAUSE:
      gameplay_pause();
      break;

    case GAME_READY:
      game_ready();
      break;

    case GAME_PLAY_MISS:
      gameplay_miss();
      break;

    case GAME_OVER:
      gameoverscreen();
      break;

    case GAME_HIGH:
      highscorescreen();
      break;
  }
}


/*
 * メインルーチン
 */

void setup() {
  //初期セットアップ
  arduboy.begin();
  arduboy.bootLogo();
  arduboy.setFrameRate(FRAME_RATE);
  arduboy.display();
//  arduboy.initRandomSeed();
  beep.begin();
  arduboy.clear();
}

void loop() {
  //ゲーム内loop
  // Handle the timing and stopping of tones
 // playToneTimed(1046, 80);
  if(!(arduboy.nextFrame())) {
    return;
  }
  beep.timer();
  if (gray_show == 1) {gray_show = 0;} else {gray_show = 1;}
  arduboy.pollButtons();
  arduboy.clear();
  gameloop();
  arduboy.display();
  if (soundEffect.frame > 0) {
    soundEffect.frame--;
  }
}



//_/_/_/_/_/_/_/_/_/_/_/_/
//  INPUT
//_/_/_/_/_/_/_/_/_/_/_/_/
void playerinput() {
  if (arduboy.pressed(UP_BUTTON)) {
    my_y -= 1<<SCALE;
    if (my_y < 0) {
      my_y = 0;
    }
  }
  if (arduboy.pressed(DOWN_BUTTON)) {
    my_y += 1<<SCALE;
    if (SCREEN_Y <= my_y) {
      my_y = SCREEN_Y;
    }
  }
  if (arduboy.pressed(LEFT_BUTTON)) {
    my_x -= 1<<SCALE;
    if (my_x <= 0) {
      my_x = 0;
    }
  }
  if (arduboy.pressed(RIGHT_BUTTON)) {
    my_x += 1<<SCALE;
    if (SCREEN_X <= my_x) {
      my_x = SCREEN_X;
    }
  }
  if (arduboy.justReleased(B_BUTTON)) {
    gamestate = GAME_PAUSE;
  }
  if (arduboy.pressed(A_BUTTON)) {
    if (rapid > RAPID_INTERVAL) {
      rapid = 0;
      //myShot generator
      for (int j=0; j<5; j++) {
        for (int i=0; i<GET_ARRAY_SIZE(myShot); i++) {
          if (myShot[i].power == 0) {
            myShot[i].power = myshotCatalogRead (j, 0); //myShotType[j].power;
            myShot[i].x = my_x;
            myShot[i].y = my_y;
            myShot[i].vx = myshotCatalogRead (j, 1)<<SCALE; ////myShotType[j].x<<SCALE;
            myShot[i].vy = myshotCatalogRead (j, 2)<<SCALE; //myShotType[j].y<<SCALE;
            break;
          }
        }
      }
    }
  }
  if (arduboy.pressed(LEFT_BUTTON) && arduboy.pressed(RIGHT_BUTTON)) {
    gamestate = DISPLAY_STATUS;
  }
}


/*
 * 
 * Liblary
 * 
 */

//爆発エフェクトSE
//int boomSE (int size, int priority) {
int boomSE (int size) {
  if (
      (soundEffect.frame > 0 && soundEffect.priority < 5) ||
      (soundEffect.frame == 0)
     )
  {
    if (30 < size) {
      playTone(2, 450);
      screenShake.frame = 120;
      soundEffect.frame = 450/4;
      soundEffect.priority = 10;
    } else if (20 < size) {
      playTone(40, 150);
      soundEffect.frame = 150/4;
      soundEffect.priority = 15;
    } else {
      playTone(350, 20);
      soundEffect.frame = 20/4;
      soundEffect.priority = 1;
    }
  }
}
// Play a tone at the specified frequency for the specified duration.
void playTone(unsigned int frequency, unsigned int duration)
{
  beep.tone(beep.freq(frequency), duration / (1000 / FRAME_RATE));
}

// Play a tone at the specified frequency for the specified duration using
// a delay to time the tone.
// Used when beep.timer() isn't being called.
void playToneTimed(unsigned int frequency, unsigned int duration)
{
  beep.tone(beep.freq(frequency));
  arduboy.delayShort(duration);
  beep.noTone();
}



//画面外判定
bool screenOver (int x, int y, int size_x, int size_y){
  if ((x+size_x < 0 || SCREEN_X < x) || (y+size_y < 0 || SCREEN_Y < y)) {
    return true;
  }
  return false;
}


//オーバーフロー判定
int overFlowLowerCheck (int before, int after) {
  return (before >= after)?true:false;
}

//マイナスオーバーフロー判定
int minusCheck (int source) {
    return (source < 0)?true:false;
}

//プログラムエリアデータ呼び出し関数

//オブジェクトの種類データ
int objectCatalogRead (int i, int j) {
  return pgm_read_word(&objectCatalog [i * OBJECT_DATA_LENGTH + j]);
//  int aa = pgm_read_word(asd);
}

//自機ショットの種類データ
int myshotCatalogRead (int i, int j) {
//  return pgm_read_word(objectCatalog + (i * OBJECT_DATA_LENGTH) + j);
//  return pgm_read_word(myshotCatalog + (i * pgm_read_word(myshotCatalog)) + j + 1);
//  int temp = pgm_read_byte(&myshotCatalog[i * 3 + j + 1]);

  return (int)pgm_read_word(&myshotCatalog[i * pgm_read_word(&myshotCatalog) + j + 1]);
}
