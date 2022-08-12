// run this in build : <dir to clang14>/clang -cc1 -load ./libHelloWorld.so -plugin hello-world ../test.cpp

class Foo {
  int i;
};

struct Bar {
  int j;
};

union Bez {
  int k;
  float l;
};

int a(int a) {
  return a;
}

int b(int b) {
  return b;
}

/*
(found function) Name : a  Loc : 14:5
(found function) Name : b  Loc : 18:5
(clang-tutor)  file: ../test.cpp
(clang-tutor)  count: 5
*/