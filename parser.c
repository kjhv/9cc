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

// 英数文字あるいはアンダースコアかを判定する関数
int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
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
void tokenize() {
    char *p = user_input;

    while (*p) {
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            vec_push(vec, new_token(TK_RETURN, 0, p));
            p += 6;
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
            *p == '<' || *p == '>' ||
            *p == ';' || *p == '=') {
            vec_push(vec, new_token(*p, 0, p));
            p++;
            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            vec_push(vec, new_token(TK_IDENT, 0, p));
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
int consume(int ty) {
    if (tokens[pos]->ty != ty)
        return 0;
    pos++;
    return 1;
}

void program() {
    int i = 0;
    while(tokens[pos]->ty != TK_EOF)
        code[i++] = stmt();

    code[i] = NULL;
}

Node *stmt() {
    Node *node;

    if(consume(TK_RETURN)) {
        node = malloc(sizeof(Node));
        node->ty = ND_RETURN;
        node->lhs = expr();
    } else {
        node = expr();
    }

    if (!consume(';'))
        error_at(tokens[pos]->input, "';'ではないトークンです");
    
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if (consume('='))
        node = new_node('=', node, assign());

    return node;
}

Node *equality() {
    Node *node = rational();

    for (;;) {
        if (consume(TK_EQ))
            node = new_node(TK_EQ, node, rational());
        else if (consume(TK_NE))
            node = new_node(TK_NE, node, rational());
        else 
            return node;
    }
}

Node *rational() {
    Node *node = add();

    for(;;) {
        if (consume('<'))
            node = new_node('<', node, add());
        else if (consume(TK_LE))
            node = new_node(TK_LE, node, add());
        else if (consume('>'))
            node = new_node('<', add(), node);
        else if (consume(TK_GE))
            node = new_node(TK_LE, add(), node);
        else 
            return node;
    }
}

Node *add() {
    Node *node = mul();

    for(;;) {
        if (consume('+'))
            node = new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else 
            return node;
    }
}

Node *mul() {
    Node *node = uary();

    for(;;) {
        if (consume('*'))
            node = new_node('*', node, uary());
        else if (consume('/'))
            node = new_node('/', node, uary());
        else
            return node;
    }
}

Node *uary() {
    if (consume('+'))
        return term();
    if (consume('-'))
        return new_node('-', new_node_num(0), term());
    return term();
}

Node *term() {
    // 次のトークンが '(' なら "(" expr ")" のはず
    if (consume('(')) {
        Node *node = expr();
        if (!consume(')'))
            error_at(tokens[pos]->input, "開きカッコに対応する閉じカッコがありません");

        return node;
    }

    if (tokens[pos]->ty == TK_IDENT) {
        char varname = tokens[pos++]->input[0];

        Node *node = malloc(sizeof(Node));
        node->ty = ND_LVAR;
        node->offset = (varname - 'a' + 1) * 8;
        return node;
    }

    // そうでなければ数値
    return num();
}

// 数値ノードを返す関数
Node *num() {
    if (tokens[pos]->ty != TK_NUM)
        error_at(tokens[pos]->input, "数値ではないトークンです");

    return new_node_num(tokens[pos++]->val);
}