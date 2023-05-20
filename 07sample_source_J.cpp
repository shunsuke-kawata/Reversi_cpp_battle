#include <iostream> // cout等を使うため
#include <stdlib.h> // randとsrandを使うため
#include <time.h>	// timeを使うため
using namespace std;

#define GAME_TIMES 60000 // 試行回数（コンテスト本番は60000）

#define PLAYER_NUM 6 // プレイヤー人数

#define BOARD_SIZE_ROW 8 // 盤面のサイズ：行
#define BOARD_SIZE_COL 8 // 盤面のサイズ：列

#define Q_TABLE_SIZE_ROW 5 // Qテーブルのサイズ：行
#define Q_TABLE_SIZE_COL 5 // Qテーブルのサイズ：列
// 盤面サイズの半分＋1（+1はエッジ用）

// 盤面：サイズ+2はエッジ表現用
// 空白(0), 黒(-1), 白(1), エッジ(2)
int board[BOARD_SIZE_ROW + 2][BOARD_SIZE_COL + 2] = {};

// 手番：先手(-1 = player_1)，後手(1 = player_2)
int player = -1;

// 対戦回数のカウント
int match[PLAYER_NUM] = {0};

// 勝敗のカウント
int win_b_count = 0;
int win_w_count = 0;

// プレイヤーのスコア
int Score[PLAYER_NUM] = {0};

// q_tableの構造体を定義（メイン関数でプレイヤーの選択を簡単に記述するため）
typedef struct
{
	double value[Q_TABLE_SIZE_ROW][Q_TABLE_SIZE_COL];
} q_table;

/*****************************************************************/
/******************　編集ここから　*******************************/
/*****************************************************************/
// Qテーブル（盤面の左上4×4に射影）
// 一番左上が[1, 1]で, [0, x]や[x, 0]はエッジ（盤外）
// 0以下および1000以上は不可

// Player_AのQテーブル
// 自分
q_table q_player_A = {
	// P_0 川田
	{{0.0, 0.0, 0.0, 0.0, 0.0},
	 {0.0, 999.9999999999999, 0.000000000000000000002, 999.99, 50.0},
	 {0.0, 0.000000000000000000002, 0.000000000000000000000000000000000000000000000000000000000001, 0.01, 1.0},
	 {0.0, 999.99, 0.01, 999, 99.9},
	 {0.0, 50.0, 1.0, 99.9, 0.0000000000000000001}}};

// Player_BのQテーブル
q_table q_player_B = {
	// P_1 山下
	{{0.0, 0.0, 0.0, 0.0, 0.0},
	 {0.0, 999.9999999999999, 0.0000000000000000000000002, 999.9, 800.0},
	 {0.0, 0.0000000000000000000000002, 0.0000000000000000000000001, 10.0, 20.0},
	 {0.0, 999.9, 10.0, 999.99, 100.0},
	 {0.0, 800.0, 20.0, 100.0, 1.0}}};
// Player_CのQテーブル
q_table q_player_C = {
	// P_2 岸本
	{{0.0, 0.0, 0.0, 0.0, 0.0},
	 {0.0, 999.9999999999999, 0.0000000000000000002, 999, 0.9},
	 {0.0, 0.0000000000000000002, 0.0000000000000000001, 0.5, 0.5},
	 {0.0, 999, 0.5, 999.9, 99},
	 {0.0, 0.9, 0.5, 99, 0.0000000000000000001}}};

// Player_DのQテーブル
q_table q_player_D =
	// P_3 川田before
	{{{0.0, 0.0, 0.0, 0.0, 0.0},
	  {0.0, 999.9999999999999, 0.0000000000000000002, 999.9, 100.0},
	  {0.0, 0.0000000000000000002, 0.000000000000000000000000000000000000000000000000000000000001, 1.0, 1.0},
	  {0.0, 999.9, 1.0, 999.9, 99.9},
	  {0.0, 100.0, 1.0, 99.9, 0.0000000000000000001}}};
// Player_EのQテーブル
q_table q_player_E = {
	// P_4 川田コピー
	{{0.0, 0.0, 0.0, 0.0, 0.0},
	 {0.0, 999.9999999999999, 0.000000000000000000002, 999.99, 100.0},
	 {0.0, 0.000000000000000000002, 0.000000000000000000000000000000000000000000000000000000000001, 0.01, 1.0},
	 {0.0, 999.99, 0.01, 999, 99.9},
	 {0.0, 100.0, 1.0, 99.9, 0.0000000000000000001}}};

// Player_FのQテーブル
q_table q_player_F = {
	// P_5 デフォルト
	{{0.0, 0.0, 0.0, 0.0, 0.0},
	 {0.0, 1.0, 1.0, 1.0, 1.0},
	 {0.0, 1.0, 1.0, 1.0, 1.0},
	 {0.0, 1.0, 1.0, 1.0, 1.0},
	 {0.0, 1.0, 1.0, 1.0, 1.0}}};

/*****************************************************************/
/******************　編集ここまで　*******************************/
/*****************************************************************/

// 関数のプロトタイプ宣言
pair<int, int> player_strategy(q_table); // Playerの戦略を記述
bool check_plc(int, int);				 // 指定されたマスに石を置けるか判定
bool can_put_anywhere(void);			 // おける場所があるか判定
void make_board(void);					 // 盤面の初期化
int check_dir(int, int, int, int);		 // 指定されたマスに置くと指定された方向にいくつ挟めるか判定
bool flag_fin(void);					 // 終了の判定
void place_stn(int, int);				 // 石を置き，盤面を更新
int judge_board(void);					 // 勝敗の判定
void Q_table_check(void);				 // Qテーブルの値をチェック

// Player_1の戦略
pair<int, int> player_strategy(q_table q_player)
{
	int row, col;
	int can_put_num = 0;
	double pos_Q[BOARD_SIZE_ROW * BOARD_SIZE_COL - 4];
	int pos_list[BOARD_SIZE_ROW * BOARD_SIZE_COL - 4][2]; // 行(0), 列(1)

	// 盤面のループ
	for (row = 1; row <= BOARD_SIZE_ROW; row++)
	{
		for (col = 1; col <= BOARD_SIZE_COL; col++)
		{
			// 石を置けるマスがあれば選択肢pos_listに記録
			if (check_plc(row, col))
			{
				pos_list[can_put_num][0] = row; // 行
				pos_list[can_put_num][1] = col; // 列
				can_put_num++;
			}
		}
	}
	int projected_row, projected_col;
	for (int i = 0; i < can_put_num; i++)
	{
		// 選択肢の位置(64マス)を左上16マスに射影する
		projected_row = pos_list[i][0];
		if (projected_row > BOARD_SIZE_ROW / 2)
		{
			projected_row = BOARD_SIZE_ROW - projected_row + 1;
		}
		projected_col = pos_list[i][1];
		if (projected_col > BOARD_SIZE_COL / 2)
		{
			projected_col = BOARD_SIZE_COL - projected_col + 1;
		}

		// 各自のQテーブルから, 選択肢にQ値を代入する
		pos_Q[i] = q_player.value[projected_row][projected_col];
	}
	// Q値に従ってルーレット選択する
	double Q_Sum = 0.0;
	double R_Sel, Q_Addition;
	for (int i = 0; i < can_put_num; i++)
	{
		Q_Sum += pos_Q[i];
	}
	R_Sel = ((double)rand() / (RAND_MAX + 1.0));
	R_Sel *= Q_Sum;
	Q_Addition = 0.0;
	for (int i = 0; i < can_put_num; i++)
	{
		Q_Addition += pos_Q[i];
		if (Q_Addition > R_Sel)
		{
			row = pos_list[i][0];
			col = pos_list[i][1];
			// 選択されたrowとcolを pair<int, int> にして返す
			return make_pair(row, col);
		}
	}
}

// 盤面の初期化
void make_board()
{
	// 盤面を全て0で埋める
	for (int row = 1; row <= BOARD_SIZE_ROW; row++)
	{
		for (int col = 1; col <= BOARD_SIZE_COL; col++)
		{
			board[row][col] = 0;
		}
	}
	// エッジにあたる値をを2に設定
	for (int i = 0; i < BOARD_SIZE_ROW + 2; i++)
	{
		board[0][i] = 2;
		board[9][i] = 2;
		board[i][0] = 2;
		board[i][9] = 2;
	}
	// 初期位置を設定
	board[BOARD_SIZE_ROW / 2][BOARD_SIZE_COL / 2] = 1;
	board[BOARD_SIZE_ROW / 2 + 1][BOARD_SIZE_COL / 2 + 1] = 1;
	board[BOARD_SIZE_ROW / 2][BOARD_SIZE_COL / 2 + 1] = -1;
	board[BOARD_SIZE_ROW / 2 + 1][BOARD_SIZE_COL / 2] = -1;
}

// 指定されたマスに置くと指定された方向にいくつ挟めるか判定
int check_dir(int row, int col, int dir_row, int dir_col)
{
	// 指定方向に相手の石がある場合は次のマスを探索する
	int times = 1;
	while (board[row + dir_row * times][col + dir_col * times] == player * -1)
	{
		times++;
	}
	// 指定方向の最後に自分の石がある場合
	if (board[row + dir_row * times][col + dir_col * times] == player)
	{
		// 指定方向に相手の石が何個あるかを返す
		return times - 1;
	}
	// 指定方向の最後に自分の石がなければ0を返す
	return 0;
}

// 指定されたマスに石を置けるか判定
bool check_plc(int row, int col)
{
	// そのマスが空であるかどうか
	if (board[row][col] == 0)
	{
		// 全方向を探索
		for (int dir_row = -1; dir_row < 2; dir_row++)
		{
			for (int dir_col = -1; dir_col < 2; dir_col++)
			{
				if (check_dir(row, col, dir_row, dir_col))
				{
					// 配置可能であればtrueを返す
					return true;
				}
			}
		}
	}
	return false;
}

// おける場所があるか判定する
bool can_put_anywhere()
{
	// 置ける場所があるか判定
	for (int row = 1; row <= BOARD_SIZE_ROW; row++)
	{
		for (int col = 1; col <= BOARD_SIZE_COL; col++)
		{
			if (check_plc(row, col))
			{
				return true;
			}
		}
	}
	return false;
}

// 終了の判定
bool flag_fin()
{
	if (can_put_anywhere())
	{
		return true;
	}
	else
	{
		player *= -1;
		return can_put_anywhere();
	}
	return true;
}

// 石を置き，盤面を更新
void place_stn(int row, int col)
{
	// 方向ごとに走査
	for (int dir_row = -1; dir_row < 2; dir_row++)
	{
		for (int dir_col = -1; dir_col < 2; dir_col++)
		{
			// 挟んだ石の数
			int change_num = check_dir(row, col, dir_row, dir_col);
			// 挟んだ石の数だけ裏返す
			for (int k = 1; k < change_num + 1; k++)
			{
				board[row + dir_row * k][col + dir_col * k] = player;
			}
		}
	}
	// マスに石を置く
	board[row][col] = player;
}

// 勝敗の判定(先手が勝利＝1, 後手が勝利=2, 引き分け=0を返す)
int judge_board()
{
	int count_b = 0; // 黒石の数
	int count_w = 0; // 白石の数
	for (int row = 1; row <= BOARD_SIZE_ROW; row++)
	{
		for (int col = 1; col <= BOARD_SIZE_COL; col++)
		{
			if (board[row][col] == -1)
			{
				count_b++;
			}
			else if (board[row][col] == 1)
			{
				count_w++;
			}
		}
	}
	// 勝敗判定
	//  先手の勝利
	if (count_b > count_w)
	{
		return 1;
	}
	// 後手の勝利
	else if (count_w > count_b)
	{
		return 2;
	}
	// 引き分け
	else
	{
		return 0;
	}
}

// プレイヤー番号を引数に，その番号のプレイヤーのQテーブルを返す
q_table select_player(int player_num)
{
	switch (player_num)
	{
	case 0:
		return q_player_A;
		break;
	case 1:
		return q_player_B;
		break;
	case 2:
		return q_player_C;
		break;
	case 3:
		return q_player_D;
		break;
	case 4:
		return q_player_E;
		break;
	case 5:
		return q_player_F;
		break;
	}
}

int max_index(int arr[])
{
	int max_value, max_index;
	max_value = -GAME_TIMES;
	max_index = 0;

	for (int i = 0; i < PLAYER_NUM; i++)
	{
		if (arr[i] > max_value)
		{
			max_value = arr[i];
			max_index = i;
		}
	}
	return max_index;
}

void Q_table_check(void)
{ // Qテーブルの値をチェック
	for (int p_num = 0; p_num < PLAYER_NUM; p_num++)
	{
		q_table Q_value_check = select_player(p_num);
		for (int row = 1; row < Q_TABLE_SIZE_ROW; row++)
		{
			for (int col = 1; col < Q_TABLE_SIZE_COL; col++)
			{
				if (Q_value_check.value[row][col] == 0 ||
					Q_value_check.value[row][col] >= 1000)
				{
					cout << "Q値エラー：" << Q_value_check.value[row][col] << endl;			// Q値エラー
					cout << "対象者:" << p_num << "(0:A, 1:B, 2:C, 3:D, 4:E, 5:F)" << endl; // 対象者
					cout << "終了時は何かを入力";											// 終了時は何かを入力
					int x;
					cin >> x;
					exit;
				}
			}
		}
	}
}

int main()
{
	// Qテーブルの値をチェック
	Q_table_check();

	// 乱数の種を変更
	srand((unsigned int)time(NULL)); // 乱数を現在時刻で初期化

	// 対戦者の選択に必要な変数
	int player_1_num, player_2_num;
	q_table Q_Player_1, Q_Player_2;

	// 対戦ループ GAME_TIMES回繰り返す
	for (int t = 0; t < GAME_TIMES; t++)
	{
		// 盤面の初期化
		make_board();

		// 対戦回数最少者を選択
		player_1_num = 0;
		for (int p_num = 1; p_num < PLAYER_NUM; p_num++)
		{
			if (match[player_1_num] > match[p_num])
			{
				player_1_num = p_num;
			}
		}
		Q_Player_1 = select_player(player_1_num);
		match[player_1_num]++;

		// 対戦相手をランダムに選択
		do
		{
			player_2_num = (int)((double)PLAYER_NUM * rand() / ((double)RAND_MAX + 1.0));
		} while (player_1_num == player_2_num);

		Q_Player_2 = select_player(player_2_num);
		match[player_2_num]++;

		// 勝者の番号を保持(Player_1(1), Player_2(2))
		int winner = 0; // 0で初期化
		// 終了までループ
		while (flag_fin())
		{
			// Player_1の手番
			if (player == -1)
			{
				int row, col;
				pair<int, int> pos = player_strategy(Q_Player_1);
				row = pos.first;
				col = pos.second;
				// 置けない場所に置いた際は負ける
				if (!check_plc(row, col))
				{
					winner = 2;
					cout << "反則負け:" << player_1_num << "(0:A, 1:B, 2:C, 3:D, 4:E, 5:F)" << endl; // 反則負け
					break;
				}
				// 石を配置する
				place_stn(row, col);
				// 手番を入れ替える
				player *= -1;
			}
			// Player_2の手番
			else
			{
				int row, col;
				pair<int, int> pos = player_strategy(Q_Player_2);
				row = pos.first;
				col = pos.second;
				// 置けない場所に置いた際は負ける
				if (!check_plc(row, col))
				{
					winner = 1;
					cout << "反則負け:" << player_2_num << "(0:A, 1:B, 2:C, 3:D, 4:E, 5:F)" << endl; // 反則負け
					break;
				}
				// 石を配置する
				place_stn(row, col);
				// 手番を入れ替える
				player *= -1;
			}
		}
		// 反則がない場合
		if (winner == 0)
		{
			// 勝利判定
			winner = judge_board();
		}
		// 先手の勝利
		if (winner == 1)
		{
			Score[player_1_num]++;
			Score[player_2_num]--;
		}
		// 後手の勝利
		else if (winner == 2)
		{
			Score[player_2_num]++;
			Score[player_1_num]--;
		}
		// 　引き分け
		else if (winner == 0)
		{
		}
		// winnerが0, 1, 2以外の数字になった際は，何かしらのエラーが発生している
		else
		{
			cout << "An Error Occured";
		}

		if (t != 0 && t % (GAME_TIMES / 10) == 0)
		{
			cout << "対戦回数: " << t << endl; // 対戦回数
			for (int P_Num = 0; P_Num < PLAYER_NUM; P_Num++)
			{
				cout << Score[P_Num] << "，";
			}
			cout << endl
				 << endl;
		}
	}

	// 最終結果を表示する
	cout << "============< 最終結果 >============" << endl; // 最終結果
	cout << "対戦回数: " << GAME_TIMES << endl;				// 対戦回数
	for (int P_Num = 0; P_Num < PLAYER_NUM; P_Num++)
	{
		cout << Score[P_Num] << "，";
	}
	cout << endl
		 << endl;
	cout << "      番号   勝ち越し数" << endl; // 番号   勝ち越し数
	for (int P_Num = 0; P_Num < PLAYER_NUM; P_Num++)
	{
		int index = max_index(Score);
		cout << P_Num + 1 << "位:  P_" << index << "\t" << Score[index] << endl;
		Score[index] = -GAME_TIMES;
	}
	cout << endl
		 << "(0:A, 1:B, 2:C, 3:D, 4:E, 5:F)" << endl;
	cout << "====================================" << endl;

	cout << "終了時は何かを入力"; // 終了時は何かを入力
	int x;
	cin >> x;
	return 0;
}

/****************************************************************/
/******************* 画面表示用プログラム ***********************/
/****************************************************************/
/*
void show_board(void);				// 盤面の表示
void show_player(void);				// 手番の表示

// 盤面の表示
void show_board() {
	//番兵も含めて表示
	for (int row = 0; row < BOARD_SIZE_ROW + 2; row++) {
		for (int col = 0; col < BOARD_SIZE_COL + 2; col++) {
			switch (board[row][col]) {
			case -1:
				cout << "b";
				break;
			case 1:
				cout << "w";
				break;
			case 0:
				cout << "-";
				break;
			case 2:
				cout << "~";
				break;
			default:
				break;
			}
		}
		cout << endl;
	}
}

// 手番の表示
void show_player() {
	switch (player) {
	case -1:
		cout << "先手(黒)の手番です" << endl;	//先手(黒)の手番
		break;
	case 1:
		cout << "後手(白)の手番です" << endl;	//後手(白)の手番
		break;
	default:
		break;
	}
}
*/
/****************************************************************/
/****************************************************************/
/****************************************************************/
