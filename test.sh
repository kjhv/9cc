#!/bin/bash
# 未定義変数を使うと警告を出す
set -u

try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
      echo "$input => $actual"
    else 
      echo "$expected expected, but got $actual"
      exit 1 
    fi
}

try 0 '0;'
try 19 '19;'
try 42 '42;'
try 21 '5+20-4;'
try 107 '99+28-15+7+35-6-41;'
try 41 " 12 + 34 - 5 ;"
try 49 "65   +     31   - 3 -44  ; "
try 30 "5*7 - 10 / 2;"
try 19 "9 * 4 / 6 + 32 - 19;"
try 90 "5 * (3 + 18) - 15;"
try 34 "3 * (5 + 9) - 88 / (91 - 80) ; "
try 150 "255 - 11 - 41 - 29 -5 * 3 - 4+7 +3 -5 -10    ;    "
try 15 "-3*-5;"
try 76 "-103+179;"
try 1 "6 == 2 * 3;"
try 1 "250 != 5 * (9 + 15);"
try 0 "235 < 93 / 31;"
try 1 "255 / 31 - 4 <= 204;"
try 1 "24 > 215 / 9;"
try 1 "49 >= 199 / 8;"
try 6 "a = 1; a + 5;"
try 174 "v=54;x=3;f = 123; f - x + v;"
try 106 "r = (9 < 37); g = r + 57; w = 69 - 21; g + w;"

echo OK
