#include "9cc.h"

// エラーを報告するための関数
// printf と同じ引数を取る
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];

    if (strcmp(user_input, "-test") == 0) {
        runtest();
        return 0;
    }

    // トークナイズした結果を格納するベクタ
    vec = new_vector();

    // トークナイズする
    tokenize();

    // vec のトークンポインタ配列へのショートカット
    // トークナイズの後に設定する必要がある
    tokens = (Token**)(vec->data);
    // AST 作成
    program();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // AST を下りながらコード生成
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        // 式の評価結果としてスタックに一つの値が残っているはず
        // なのでスタックが溢れないようにpopしておく
        printf("  pop rax\n");
    }
    
    // エピローグ
    // 最後の式の結果が rax に残っているのでそれを返り値にする
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}