#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    ND_NUM = 256,
};

// ノードの構造体
typedef struct Node {
    int ty;
    struct Node *lhs;
    struct Node *rhs;
    int val;
 } Node;

// トークナイザー
void tokenize(Vector *vec);

// ノード生成関数の関数プロトタイプ
Node *expr(Vector *vec);
Node *equality(Vector *vec);
Node *rational(Vector *vec);
Node *add(Vector *vec);
Node *mul(Vector *vec);
Node *uary(Vector *vec);
Node *term(Vector *vec);
Node *num(Vector *vec);

// コードジェネレータ
void gen(Node *node);

