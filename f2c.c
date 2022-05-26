#include <stdio.h>
#include <inttypes.h>

int main (int argc, char * argv[])
{
	if (argc != 4) return 1;

	char * in    = argv[1]
	   , * out   = argv[2]
	   , * ident = argv[3] ;
	FILE * i
	   , * o ;

	if (!(i = fopen( in  , "rb" ))) return 1;
	if (!(o = fopen( out , "wb" ))) return 1;

	fprintf(o, "static unsigned char %s[]={", ident);
	for ( uint8_t b; fread(&b, 1, 1, i); )
		fprintf(o, "%" PRIu8 ",", b);
	fprintf(o, "};\n");

	fclose(i);
	fclose(o);
	return 0;
}
