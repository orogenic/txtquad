#include <stdio.h>

void swizzles (FILE * o)
{
	static char const C[] = { 'x', 'y', 'z', 'w' }, n = sizeof C;
	static char S[sizeof C + 1], c, s, i, j, k, l;
	fputs("#pragma once\n#include \"types.h\"\n", o);
	for (i = n; i >= 2; --i) {
		c = -1, s = i - 1, S[i] = '\0';
		for (j = 0; j < i; ++j) S[j] = C[0];
		for (;;) {
			if (c != n - 1) {
				for (S[s] = C[++c]; s < i - 1; S[++s] = C[c = 0]);
				for (l = j = 0; j < i; ++j) {
					for (k = 0; S[j] != C[k]; ++k);
					if (k > l) l = k;
				}
				if (!l) ++l;
				if ((j = l < n - 1)) {
					fprintf(o, "#define v%s(v) _Generic(v,", S);
					for (k = l; ++k <= n;)
						fprintf(o, "v%u:v%sv%u%s", k, S, k, k < n ? "," : ")(v)\n");
				}
				for (k = l; ++k <= n;) {
					fprintf(o, "static inline v%u v%s", i, S);
					if (j) fprintf(o, "v%u", k); else fputs("  ", o);
					fprintf(o, "(v%u v) { return (v%u) { ", k, i);
					for (j = 0; j < i; ++j)
						fprintf(o, "v.%c%s", S[j], j < i - 1 ? ", " : " }; }\n");
				}
			}
			else if (--s < 0) break;
			else while (S[s] != C[c]) --c;
		}
	}
}

int main (int argc, char * argv[])
{
	if (argc != 2) return 1;
	char * out = argv[1];
	FILE * o;
	if (!(o = fopen(out, "wb"))) return 1;
	swizzles(o);
	fclose(o);
	return 0;
}
