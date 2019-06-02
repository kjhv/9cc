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

echo OK
