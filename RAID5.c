#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define MAXLEN 1024

void print_unsigned_char(unsigned char buf[MAXLEN], int len);
int max(int rc[], int n);
int min(int rc[], int n);
void redundancy(char *argv[], int n);
void redundancy_(char *argv[]);

int main(int argc, char *argv[]){
	redundancy(argv, argc);
	// redundancy_(argv);
	
	return 0;
}


void print_unsigned_char(unsigned char buf[MAXLEN], int len) {
	printf("buf = %s\n", buf);
	int i, j;
 	for (i = 0; i < len; i++){
		for (j = 7; j >= 0; j--){
			unsigned char tmpp = buf[i];
			tmpp = buf[i] & (1 << j);
			printf("%d", tmpp >> j);
		}
		printf(" ");
	}
	printf("\n");
}

int max(int rc[], int n) {
	int max = rc[0];
	int i;
	for (i = 1; i < n; i++) {
		max = max > rc[i] ? max : rc[i];
	}
	
	return max;
}

int min(int rc[], int n) {
	int min = rc[0];
	int i;
	for (i = 1; i < n; i++) {
		min = min < rc[i] ? min : rc[i];
	}

	return min;
}

// input  A, B, C, D ..., P
void redundancy(char *argv[], int n) {
	FILE *file[n-1];
	int i, j, size = 0, flag = 1;
	unsigned char buf[n-1][MAXLEN+1];
	int rc[n-2];
	clock_t start, end;

	for (i = 1; i < n-1; i++) {
		file[i-1] = fopen(argv[i], "rb");
	}
	file[n-2] = fopen(argv[n-1], "wb");

	for (i = 0; i < n-1; i++){
		if (file[i] == NULL){
			printf("%s file not exit\n", argv[i+1]);
			exit(1);
		}
	}	

	start = clock();
	while(flag){
		for (i = 0; i < n-2; i++) {
			rc[i] = fread(buf[i], sizeof(unsigned char), MAXLEN, file[i]);
			if (rc[i] == 0) {
				flag = 0;
			}
		}

		// for (int i = 0; i < n-2; i++) {
		// 	print_unsigned_char(buf[i], strlen((char*)buf[i]));
		// }

		int min_ = min(rc, n-2);
		memset(buf[n-2], 0, sizeof(char)*MAXLEN);
		for (i = 0; i < min_; i++){
			for (j = 0; j < n-2; j++){
				buf[n-2][i] ^= buf[j][i];
			}
		}

		size++;
		fwrite(buf[n-2], sizeof(unsigned char), min_, file[n-2]);
	} 
	printf("size = %d KB \n", size-1);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < n-1; i++) {
		fclose(file[i]);
	}
}

// input  A, B, C, D, P
void redundancy_(char *argv[]) {
	FILE *outfile, *infile1, *infile2, *infile3, *infile4;
	infile1 = fopen(argv[1], "rb");
	infile2 = fopen(argv[2], "rb");
	infile3 = fopen(argv[3], "rb");
	infile4 = fopen(argv[4], "rb");
	outfile = fopen(argv[5], "wb");

	unsigned char buf1[MAXLEN+1] = "";
	unsigned char buf2[MAXLEN+1] = "";
	unsigned char buf3[MAXLEN+1] = "";
	unsigned char buf4[MAXLEN+1] = "";
	unsigned char buf5[MAXLEN+1] = "";

	if (outfile == NULL || infile1 == NULL || infile2 == NULL  || infile3 == NULL || infile4 == NULL){
		printf("file not exit\n");
		exit(1);
	}   

	int rc1, rc2, rc3, rc4;
	int i, j = 0;

	clock_t start, end; 
	start = clock();

	while((rc1 = fread(buf1, sizeof(unsigned char), MAXLEN, infile1)) != 0 &&
		(rc2 = fread(buf2, sizeof(unsigned char), MAXLEN, infile2)) != 0 && 
		(rc3 = fread(buf3, sizeof(unsigned char), MAXLEN, infile3)) != 0 &&
		(rc4 = fread(buf4, sizeof(unsigned char), MAXLEN, infile4)) != 0){

		// printf("rc = %d %d %d %d\n", rc1, rc2, rc3, rc4);
		// print_unsigned_char(buf1, strlen((char*)buf1));
		// print_unsigned_char(buf2, strlen((char*)buf2));
		// print_unsigned_char(buf3, strlen((char*)buf3));
		// print_unsigned_char(buf4, strlen((char*)buf4));

		int min_ = rc1;
		min_ = min_ < rc2 ? min_ : rc2;
		min_ = min_ < rc3 ? min_ : rc3;
		min_ = min_ < rc4 ? min_ : rc4;
		for (i = 0; i < min_; i++){
			buf5[i] = buf1[i] ^ buf2[i] ^ buf3[i] ^ buf4[i];
		}

		j++;
		// print_unsigned_char(buf5, strlen((char*)buf5));
		fwrite(buf5, sizeof(unsigned char), min_, outfile);
	} 
	printf("size = %d KB \n", j);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	fclose(infile1);
	fclose(infile2);
	fclose(infile3);
	fclose(infile4);
	fclose(outfile);
}