build/debug/c4 --compile tests/$1.c
cat $1.ll
llvm/install/bin/clang -o out $1.ll
llvm/install/bin/llc -o out.s $1.ll
./out
