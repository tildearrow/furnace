#ifdef _MSC_VER
#if _MSC_VER >= 1200
#pragma comment(linker, "/OPT:NOWIN98")
#endif
#endif

#include <stdio.h>
#include <string.h>

static void bin2c(char *path)
{
	int c, p = 0;
	FILE *fp;
	fp = fopen(path, "rb");
	if (!fp) return;
	while (!feof(fp))
	{
		c = fgetc(fp);
		if (c == EOF) break;
		if ((p & 7) == 0) printf("\t");
		printf("0x%02X, ", c);
		if ((p++ & 7) == 7) printf("\n");
	}
	if (p & 7) printf("\n");
	fclose(fp);
}

int main(int argc, char **argv)
{
	int argp = 1;
	while (argc - argp >= 1)
	{
		bin2c(argv[argp++]);
	}
	if (argp == 1)
	{
		fprintf(stderr, "Usage: %s [INPUT SYMBOL]...\n",argv[0]);
	}
	return 0;
}
