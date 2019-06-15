#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// 新しいしいトークンを生成する関数
Token *new_token(int ty, int val, char *input) {
    Token *tok = malloc(sizeof(Token));
    tok->ty = ty;
    tok->val = val;
    tok->input = input;
    return tok;
}

// 可変長ベクタ
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector *new_vector() {
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *) * 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

// 入力プログラム
char *user_input;

// トークナイズした結果のトークン列はこの配列に保存する
// 100個以下とする
//Token tokens[100];

// エラーを報告するための関数
// printf と同じ引数を取る
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告するための関数
void error_at(char *loc, char *msg) {
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");        // pos個の空白出力
    fprintf(stderr, "^ %s\n", msg);
    exit(1);
}

// user_inputが指している文字列を
// トークンに分割して vec に保存する
void tokenize(Vector *vec) {
    char *p = user_input;

    int i = 0;
    while (*p) {
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (!strncmp(p, "==", 2)) {
            vec_push(vec, new_token(TK_EQ, 0, p));
            p += 2;
            continue;
        }

        if (!strncmp(p, "!=", 2)) {
            vec_push(vec, new_token(TK_NE, 0, p));
            p += 2;
            continue;
        }

        if (!strncmp(p, "<=", 2)) {
            vec_push(vec, new_token(TK_LE, 0, p));
            p += 2;
            continue;
        }

        if (!strncmp(p, ">=", 2)) {
            vec_push(vec, new_token(TK_GE, 0, p));
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-' ||
            *p == '*' || *p == '/' ||
            *p == '(' || *p == ')' ||
            *p == '<' || *p == '>') {
            vec_push(vec, new_token(*p, 0, p));
            p++;
            continue;
        }
        
        if (isdigit(*p)) {
            vec_push(vec, new_token(TK_NUM, strtol(p, &p, 10), p));
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    vec_push(vec, new_token(TK_EOF, 0, p));
}

// AST Nodeの型
enum {
    ND_NUM = 256,
};

typedef struct Node {
    int ty;
    struct Node *lhs;
    struct Node *rhs;
    int val;
 } Node;

// 新しいノードを作成する関数
Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 新しい数値のノードを生成する関数
Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

// トークンのインデックス。初期値ゼロ
int pos = 0;

// トークンを先読みして期待した型ならば１トークン読み進めて
// 真を返す関数
int consume(Vector *vec, int ty) {
    if (((Token*)vec->data[pos])->ty != ty)
        return 0;
    pos++;
    return 1;
}

// ノード生成関数の関数プロトタイプ
Node *expr(Vector *vec);
Node *equality(Vector *vec);
Node *rational(Vector *vec);
Node *add(Vector *vec);
Node *mul(Vector *vec);
Node *uary(Vector *vec);
Node *term(Vector *vec);
Node *num(Vector *vec);

Node *expr(Vector *vec) {
    Node *node = equality(vec);
}

Node *equality(Vector *vec) {
    Node *node = rational(vec);

    for (;;) {
        if (consume(vec, TK_EQ))
            node = new_node(TK_EQ, node, rational(vec));
        else if (consume(vec, TK_NE))
            node = new_node(TK_NE, node, rational(vec));
        else 
            return node;
    }
}

Node *rational(Vector *vec) {
    Node *node = add(vec);

    for(;;) {
        if (consume(vec, '<'))
            node = new_node('<', node, add(vec));
        else if (consume(vec, TK_LE))
            node = new_node(TK_LE, node, add(vec));
        else if (consume(vec, '>'))
            node = new_node('<', add(vec), node);
        else if (consume(vec, TK_GE))
            node = new_node(TK_LE, add(vec), node);
        else 
            return node;
    }
}

Node *add(Vector *vec) {
    Node *node = mul(vec);

    for(;;) {
        if (consume(vec, '+'))
            node = new_node('+', node, mul(vec));
        else if (consume(vec, '-'))
            node = new_node('-', node, mul(vec));
        else 
            return node;
    }
}

Node *mul(Vector *vec) {
    Node *node = uary(vec);

    for(;;) {
        if (consume(vec, '*'))
            node = new_node('*', node, uary(vec));
        else if (consume(vec, '/'))
            node = new_node('/', node, uary(vec));
        else
            return node;
    }
}

Node *uary(Vector *vec) {
    if (consume(vec, '+'))
        return term(vec);
    if (consume(vec, '-'))
        return new_node('-', new_node_num(0), term(vec));
    return term(vec);
}

Node *term(Vector *vec) {
    // 次のトークンが '(' なら "(" expr ")" のはず
    if (consume(vec, '(')) {
        Node *node = expr(vec);
        if (!consume(vec, ')'))
            error_at(((Token*)vec->data[pos])->input, "開きカッコに対応する閉じカッコがありません");

        return node;
    }

    // そうでなければ数値
    return num(vec);
}

// 数値ノードを返す関数
Node *num(Vector *vec) {
    if (((Token*)vec->data[pos])->ty == TK_NUM)
        return new_node_num(((Token*)vec->data[pos++])->val);

    error_at(((Token*)vec->data[pos])->input, "数値ではないトークンです");
}

void gen(Node *node) {
    if (node->ty == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->ty) {
    case '+':
        printf("  add rax, rdi\n");
        break;
    case '-':
        printf("  sub rax, rdi\n");
        break;
    case '*':
        printf("  imul rdi\n");
        break;
    case '/':
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case TK_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case TK_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case '<':
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case TK_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
    }

    printf("  push rax\n");

}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    // トークナイズした結果を格納するベクタ
    Vector *vec = new_vector();
    // トークナイズする
    tokenize(vec);
    // AST 作成
    Node *node = expr(vec);

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // AST を下りながらコード生成
    gen(node);

    // スタックトップに式全体の値が残っているはずなので
    // それを RAX にロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
