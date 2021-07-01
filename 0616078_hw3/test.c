#include "libmini.h"

void printlong(long long x) {
  if (x < 0) {
    write(1, "-", 1);
    x = -x;
  }
  char buf[100];
  int l = 0;
  if (x == 0) {
    write(1, "0\n", 2);
    return;
  }
  while (x) {
    buf[l] = '0' + x % 10;
    x /= 10;
    l++;
  }
  for (int i = 0; i < l / 2; i++) {
    char tmp = buf[i];
    buf[i] = buf[l - i - 1];
    buf[l - i - 1] = tmp;
  }
  buf[l] = '\n';
  write(1, buf, l + 1);
}

typedef void (*proc_t)();
static jmp_buf jb;

#define FUNBODY(m, from)        \
	{                           \
    int a = from * 10;   \
    int b = from * 100;  \
    int c = from * 1000; \
		write(1, m, strlen(m)); \
    printlong(a);    \
    printlong(b);    \
    printlong(c);    \
		longjmp(jb, from);      \
	}

void a() FUNBODY("This is function a().\n", 1);
void b() FUNBODY("This is function b().\n", 2);
void c() FUNBODY("This is function c().\n", 3);
void d() FUNBODY("This is function d().\n", 4);
void e() FUNBODY("This is function e().\n", 5);
void f() FUNBODY("This is function f().\n", 6);
void g() FUNBODY("This is function g().\n", 7);
void h() FUNBODY("This is function h().\n", 8);
void i() FUNBODY("This is function i().\n", 9);
void j() FUNBODY("This is function j().\n", 10);

proc_t funs[] = {a, b, c, d, e, f, g, h, i, j};

int main()
{
  int a = -1;
  int b = -2;
  int c = -3;
  int ret;
	sigset_t s, os;
	ret = sigemptyset(&s);
  write(1, "1 ret: ", 6);
  printlong(ret);
	ret = sigaddset(&s, SIGALRM);
  write(1, "2 ret: ", 6);
  printlong(ret);
	ret = sigprocmask(SIG_BLOCK, &s, &os);
  write(1, "3 ret: ", 6);
  printlong(ret);

	volatile int i = 0;
	if ((ret = setjmp(jb)) != 0)
	{
		i++;
    printlong(a);
    printlong(b);
    printlong(c);
	}

  write(1, "4 ret: ", 6);
  printlong(ret);
  printlong(s);
  printlong(os);

	ret = sigemptyset(&s);
  write(1, "5 ret: ", 6);
  printlong(ret);
  printlong(s);
  printlong(os);
	ret = sigaddset(&s, SIGALRM);
  write(1, "6 ret: ", 6);
  printlong(ret);
  printlong(s);
  printlong(os);
	sigprocmask(SIG_UNBLOCK, &s, &os);
  write(1, "aftproc:\n", 9);
  printlong(s);
  printlong(os);

	if (ret = sigismember(&os, SIGALRM))
	{
    write(1, "7 ret: ", 6);
    printlong(ret);
    printlong(s);
    printlong(os);
		char m[] = "sigalrm was blocked.\n";
		write(1, m, sizeof(m) - 1);
	}
	else
	{
    write(1, "7 ret: ", 6);
    printlong(ret);
    printlong(s);
    printlong(os);
		char m[] = "sigalrm was not blocked.\n";
		write(1, m, sizeof(m) - 1);
	}

	if (i < 10)
		funs[i]();
	return 0;
}
