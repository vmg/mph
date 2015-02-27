#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#undef NDEBUG
#include <assert.h>

int	d, n, m, maxlen, minklen, maxklen, minchar, maxchar, loop, alphasz;
int	binary=0, compact=1, safe=0, len=0, insensitive=0;

int
readint(void)
{
	int i;
	if (binary)
		assert(fread(&i, sizeof i, 1, stdin) == 1);
	else
		assert(scanf("%d\n", &i) == 1);
	return i;
}

double
readfloat(void)
{
	double d;
	if (binary)
		assert(fread(&d, sizeof d, 1, stdin) == 1);
	else
		assert(scanf("%lg\n", &d) == 1);
	return d;
}

void
emitconst(void)
{
	assert(scanf("%d\n", &binary) == 1);

	printf("/*\n");
	printf(" * d=%d\n",		d = readint());
	printf(" * n=%d\n",		n = readint());
	printf(" * m=%d\n",		m = readint());
	printf(" * c=%g\n",		readfloat());
	printf(" * maxlen=%d\n",	maxlen = readint());
	printf(" * minklen=%d\n",	minklen = readint());
	printf(" * maxklen=%d\n",	maxklen = readint());
	printf(" * minchar=%d\n",	minchar = readint());
	printf(" * maxchar=%d\n",	maxchar = readint());
	printf(" * loop=%d\n",		loop = readint());
	printf(" * numiter=%d\n",	readint());

	if (readint())
		printf(" * seed=%d\n", readint());
	else
		printf(" * seed=%d\n", readint());

	printf(" */\n");

	alphasz = compact ? maxchar-minchar+1 : 256;
}

void
emitg(void)
{
	int i, j;

	printf("\n");
	printf("static int g[] = {\n");
	for (i=j=0; i<n; i++) {
		if (j == 0)
			printf("\t");
		printf("%d,", readint());
		if (++j == 10) {
			printf("\n");
			j = 0;
		}
		else
			printf(" ");
	}
	if (j)
		printf("\n");
	printf("};\n");
}

void
emitT(int t)
{
	int i, j, k;

	printf("\n");
	printf("static int T%d[] = {\n", t);

	for (i=k=0; i<maxlen; i++) {
		for (j=0; j<256; j++) {
			int v = readint();
			if (compact && (j < minchar || j > maxchar))
				continue;
			if (k == 0)
				printf("\t");
			printf("%d,", v);
			if (++k == 10) {
				printf("\n");
				k = 0;
			}
			else
				printf(" ");
		}
	}
	if (k)
		printf("\n");
	printf("};\n");
}

void
emithash(void)
{
	printf("\n");
	if (insensitive) {
		printf("#ifndef perfhash_tolower\n");
		printf("#define perfhash_tolower(c) tolower(c)\n");
		printf("#endif\n");
	}
	printf("\n");
	printf("static int perfhash(const unsigned char *key%s)\n", len ? ", int len" : "");
	printf("{\n");
	if (maxlen > 1)
		printf("\tint i;\n");
	if (len)
		printf("\tint n;\n");
	printf("\tunsigned ");
	if (d == 2)
		printf("f0, f1;\n");
	else
		printf("f0, f1, f2;\n");
	printf("\tconst unsigned char *kp = key;\n");
	printf("\n");
	if (safe && len) {
		printf("\tif (len < %d || len > %d)\n", minklen, maxklen);
		printf("\t\treturn -1;\n");
		printf("\n");
	}
	printf("\tfor (");
	if (maxlen > 1)
		printf("i=%d, ", compact ? -minchar : 0);
	if (d == 2)
		printf("f0=f1=0");
	else
		printf("f0=f1=f2=0");
	if (len) {
		printf(", n=0; n<len; ++n) {\n");
		printf("\t\tint c = kp[n];\n");
	} else {
		printf("; *kp; ++kp) {\n");
		printf("\t\tint c = *kp;\n");
	}
	if (insensitive)
		printf("\t\tc = perfhash_tolower(c);\n");
	if (safe && compact) {
		printf("\t\tif (c < %d || c > %d)\n", minchar, maxchar);
		printf("\t\t\treturn -1;\n");
	}
	if (safe && !len) {
		printf("\t\tif (kp-key > %d)\n", maxklen-1);
		printf("\t\t\treturn -1;\n");
	}
	if (maxlen > 1) {
		printf("\t\tf0 += T0[i + c];\n");
		printf("\t\tf1 += T1[i + c];\n");
		if (d == 3)
			printf("\t\tf2 += T2[i + c];\n");
		printf("\t\ti += %d;\n", alphasz);
		if (maxlen < maxklen)
			printf("\t\tif (i >= %d)\n\t\t\ti = %d;\n",
				compact ? maxlen*alphasz-minchar : maxlen*256,
				compact ? -minchar : 0);
	}
	else {
		int i = compact ? -minchar : 0;
		printf("\t\tf0 += T0[%d + c];\n", i);
		printf("\t\tf1 += T1[%d + c];\n", i);
		if (d == 3)
			printf("\t\tf2 += T2[%d + c];\n", i);
	}
	printf("\t}\n");
	if (safe && !len) {
		printf("\n");
		printf("\tif (kp-key < %d)\n", minklen);
		printf("\t\treturn -1;\n");
	}
	printf("\n");
	printf("\tf0 %%= %d;\n", n);
	printf("\tf1 %%= %d;\n", n);
	if (d == 3)
		printf("\tf2 %%= %d;\n", n);
	printf("\n");
	if (loop) {
		if (d == 2) {
			printf("\tif (f1 == f0 && ++f1 >= %d)\n", n);
			printf("\t\tf1 = 0;\n");
			printf("\n");
		}
		else {
			printf("\tif (f1 == f0 && ++f1 >= %d)\n", n);
			printf("\t\tf1 = 0;\n");
			printf("\tif (f2 == f0 && ++f2 >= %d)\n", n);
			printf("\t\tf2 = 0;\n");
			printf("\tif (f1 == f2) {\n");
			printf("\t\tif (++f2 >= %d)\n", n);
			printf("\t\t\tf2 = 0;\n");
			printf("\t\tif (f2 == f0 && ++f2 >= %d)\n", n);
			printf("\t\t\tf2 = 0;\n");
			printf("\t}\n");
			printf("\n");
		}
	}
	if (d == 2)
		printf("\treturn (g[f0] + g[f1]) %% %d;\n", m);
	else
		printf("\treturn (g[f0] + g[f1] + g[f2]) %% %d;\n", m);
	printf("}\n");
}

void
usage(void)
{
	fprintf(stderr, "usage: emitc [-n] [-s] [-l]\n");
	exit(1);
}

void
getargs(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "nsli")) != -1)
		switch (opt) {
		case 'n':
			compact = 0;
			break;
		case 's':
			safe = 1;
			break;
		case 'l':
			len = 1;
			break;
		case 'i':
			insensitive = 1;
			break;
		case '?':
			usage();
			break;
		}
}

int
main(int argc, char **argv)
{
	int t;

	getargs(argc, argv);

	emitconst();
	emitg();
	for (t=0; t<d; t++)
		emitT(t);
	emithash();

	return 0;
}
