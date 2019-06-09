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

try 0 0
try 19 19
try 42 42
try 31 31
try 21 '5+20-4'
try 107 '99+28-15+7+35-6-41'
try 41 " 12 + 34 - 5 "
try 49 "65   +     31   - 3 -44   "
try 30 "5*7 - 10 / 2"
try 19 "9 * 4 / 6 + 32 - 19"
try 90 "5 * (3 + 18) - 15"
try 34 "3 * (5 + 9) - 88 / (91 - 80)"
try 47 "5+6*7"
try 15 "-3*-5"
try 76 "-103+179"

echo OK
