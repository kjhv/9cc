#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// エラーを報告するための関数
// printf と同じ引数を取る
void error(char *fmt, ...);

// 入力プログラム。argv[1] の値を代入する。
char *user_input;

// 可変長ベクタ
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

// ベクタ生成関数
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void expect(int line, int expected, int actual);
void runtest();

// トークンの型を表す値
enum {
    TK_NUM = 256,   // 整数トークン
    TK_IDENT,       // 識別子
    TK_EQ,          // ==
    TK_NE,          // !=
    TK_LE,          // <=
    TK_GE,          // >=
    TK_EOF,         // 入力の終わりを表すトークン
};

// トークンの構造体
typedef struct {
    int ty;         // トークンの型
    int val;        // ty が TK_NUM の場合、その数値
    char *input;    // トークン文字列（エラーメッセージ用）
} Token;

// AST Nodeの型
enum {
    ND_NUM = 256,       // 整数ノードの型
    ND_LVAR,            // ローカル変数ノードの型
};

// ノードの構造体
typedef struct Node {
    int ty;             // 演算子、ND_NUM、ND_LVAR
    struct Node *lhs;   // 左辺
    struct Node *rhs;   // 右辺
    int val;            // ty が ND_NUM の場合のみ使う
    int offset;         // ty が ND_LVAR の場合のみ使う
 } Node;

// トークナイザー
void tokenize();

// ノード生成関数の関数プロトタイプ
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *rational();
Node *add();
Node *mul();
Node *uary();
Node *term();
Node *num();

// コードジェネレータ
void gen(Node *node);

// トークナイズした結果を格納するベクタ
Vector *vec;

// vec のトークン配列へのショートカット
Token **tokens;

// ; で区切られた stmt を格納する配列
Node *code[500];