#include "9cc.h"

// トークンのインデックス。初期値ゼロ
int pos = 0;

// 新しいしいトークンを生成する関数
Token *new_token(int ty, int val, char *input) {
    Token *tok = malloc(sizeof(Token));
    tok->ty = ty;
    tok->val = val;
    tok->input = input;
    return tok;
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

// 新しいノードを作成する関数
Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 新しい数値ノードを生成する関数
Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

// トークンを先読みして期待した型ならば１トークン読み進めて
// 真を返す関数
int consume(Vector *vec, int ty) {
    if (((Token*)vec->data[pos])->ty != ty)
        return 0;
    pos++;
    return 1;
}

Node *expr(Vector *vec) {
    return equality(vec);
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
    if (((Token*)vec->data[pos])->ty != TK_NUM)
        error_at(((Token*)vec->data[pos])->input, "数値ではないトークンです");

    return new_node_num(((Token*)vec->data[pos++])->val);
}