#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define MAXLEN 1024

unsigned short GFLOG[256];
unsigned short GFILOG[256];
int K[255];
 

void generate_Galois_table();
unsigned short Galois_mutipile(int a, int b);
unsigned short Galois_division(int a,int b);
void print_unsigned_char(unsigned char buf[MAXLEN], int len);
int min(int rc[], int n);
void redundancy_(char *argv[]);
void redundancy(char *argv[], int n);
void restore2_(char *argv[]);
void restore2(char *argv[], int n);
void restore1_(char *argv[]);
void restore1(char *argv[], int n);


int main(int argc, char *argv[]){
	generate_Galois_table();
	int i;
	for (i = 0; i < 255; i++)  K[i] = i+1;

	if (strcmp(argv[1], "redundancy") == 0){
		redundancy(argv, argc);
	} else if (strcmp(argv[1], "restore2") == 0) {
		restore2(argv, argc);
	} else if (strcmp(argv[1], "restore1") == 0) {
		restore1(argv, argc);
	}
	
	return 0;
}


void generate_Galois_table(){
    GFILOG[0]=1;
    int i, alog_data;
    for (i = 1; i < 255; i++){
        alog_data = GFILOG[i-1] * 2;
        if (alog_data >= 256) 
        	alog_data ^= 285;
        GFILOG[i] = (unsigned short)alog_data;
        GFLOG[GFILOG[i]] = (unsigned short)i;
    }
}


unsigned short Galois_mutipile(int a, int b){
    if (a == 0 || b == 0) 
    	return 0;
    return GFILOG[(GFLOG[a]+GFLOG[b])%255];
}
 

unsigned short Galois_division(int a,int b){
	if (a == 0)
		return 0;
    if (b == 0)
    	return -1;
    return GFILOG[((GFLOG[a]-GFLOG[b])+255)%255];
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


int min(int rc[], int n) {
	int min = rc[0];
	int i;
	for (i = 1; i < n; i++) {
		min = min < rc[i] ? min : rc[i];
	}

	return min;
}


// input  A, B, C, D, P, Q
void redundancy_(char *argv[]) {
	FILE *outfile1, *outfile2, *infile1, *infile2, *infile3, *infile4;
	infile1 = fopen(argv[2], "rb");
	infile2 = fopen(argv[3], "rb");
	infile3 = fopen(argv[4], "rb");
	infile4 = fopen(argv[5], "rb");
	outfile1 = fopen(argv[6], "wb");
	outfile2 = fopen(argv[7], "wb");

	unsigned char buf1[MAXLEN+1] = "";
	unsigned char buf2[MAXLEN+1] = "";
	unsigned char buf3[MAXLEN+1] = "";
	unsigned char buf4[MAXLEN+1] = "";
	unsigned char P[MAXLEN+1] = "";
	unsigned char Q[MAXLEN+1] = "";

	if (infile1 == NULL || infile2 == NULL  || infile3 == NULL || infile4 == NULL){
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

		int min_ = rc1 > rc2 ? rc2 : rc1;
		min_ = min_ > rc3 ? rc3 : min_;
		min_ = min_ > rc4 ? rc4 : min_;
		for (i = 0; i < min_; i++){
			P[i] = buf1[i] ^ buf2[i] ^ buf3[i] ^ buf4[i];

			// printf("%c  %c  %c  %c  ", buf1[i], buf2[i], buf3[i], buf4[i]);
			// printf("%d  %d  %d  %d  ", (unsigned int)(buf1[i]), (unsigned int)(buf2[i]), (unsigned int)(buf3[i]),
			// 	 (unsigned int)(buf4[i]));
			// printf("%d  %d  ", (unsigned int)P[i] ,(Galois_mutipile((unsigned int)buf1[i], K[0]) ^
			//  Galois_mutipile((unsigned int)buf2[i], K[1]) ^ Galois_mutipile((unsigned int)buf3[i], K[2]) ^ 
			//  Galois_mutipile((unsigned int)buf4[i], K[3])));
			Q[i] = (unsigned char)(Galois_mutipile((unsigned int)buf1[i], K[0]) ^ 
				Galois_mutipile((unsigned int)buf2[i], K[1]) ^ Galois_mutipile((unsigned int)buf3[i], K[2]) ^ 
				Galois_mutipile((unsigned int)buf4[i], K[3]));
			// printf("%c  \n", Q[i]);
		}

		j++;
		// print_unsigned_char(P, strlen((char*)P));
		// print_unsigned_char(Q, strlen((char*)Q));
		fwrite(P, sizeof(unsigned char), min_, outfile1);
		fwrite(Q, sizeof(unsigned char), min_, outfile2);
	} 
	printf("size = %d KB \n", j);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);
	fclose(infile1);
	fclose(infile2);
	fclose(infile3);
	fclose(infile4);
	fclose(outfile1);
	fclose(outfile2);
}


// input  A, B, C, D, ..., P, Q
void redundancy(char *argv[], int n) {
	FILE *file[n-2];
	int i, j, size = 0, flag = 1;
	unsigned char buf[n-2][MAXLEN+1];
	int rc[n-4];
	clock_t start, end;
	
	for (i = 2; i < n-2; i++) {
		file[i-2] = fopen(argv[i], "rb");
	}
	file[n-4] = fopen(argv[n-2], "wb");
	file[n-3] = fopen(argv[n-1], "wb");
	
	for (i = 0; i < n-2; i++){
		if (file[i] == NULL){
			printf("%s file not exit\n", argv[i+2]);
			exit(1);
		}
	}

	start = clock();

	while(flag){
		for (i = 0; i < n-4; i++) {
			rc[i] = fread(buf[i], sizeof(unsigned char), MAXLEN, file[i]);
			if (rc[i] == 0) {
				flag = 0;
			}
		}

		// for (i = 0; i < n-4; i++) {
		// 	printf("%d  ", rc[i]);
		// }
		// printf("\n");
		// for (i = 0; i < n-4; i++) {
		// 	print_unsigned_char(buf[i], strlen((char*)buf[i]));
		// }

		int min_ = min(rc, n-4);
		memset(buf[n-4], 0, sizeof(char)*MAXLEN);
		for (i = 0; i < min_; i++){
			unsigned short tem = 0;
			for (j = 0; j < n-4; j++){
				buf[n-4][i] ^= buf[j][i];

				tem ^= Galois_mutipile((unsigned short)buf[j][i], K[j]);
			}
			buf[n-3][i] = (unsigned char)tem;
		}
		
		size++;

		// print_unsigned_char(buf[n-4], strlen((char*)buf[n-4]));
		// print_unsigned_char(buf[n-3], strlen((char*)buf[n-3]));
		fwrite(buf[n-4], sizeof(unsigned char), min_, file[n-4]);
		fwrite(buf[n-3], sizeof(unsigned char), min_, file[n-3]);
	} 
	printf("size = %d KB \n", size-1);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < n-2; i++) {
		fclose(file[i]);
	}
}


// lost A, B
// input C, D, P, Q, A_, B_
void restore2_(char *argv[]) {
	FILE *outfile1, *outfile2, *infile1, *infile2, *infile3, *infile4;
	infile1 = fopen(argv[2], "rb");
	infile2 = fopen(argv[3], "rb");
	infile3 = fopen(argv[4], "rb");
	infile4 = fopen(argv[5], "rb");
	outfile1 = fopen(argv[6], "wb");
	outfile2 = fopen(argv[7], "wb");

	unsigned char buf1[MAXLEN+1] = "";
	unsigned char buf2[MAXLEN+1] = "";
	unsigned char buf3[MAXLEN+1] = "";
	unsigned char buf4[MAXLEN+1] = "";
	unsigned char data1[MAXLEN+1] = "";
	unsigned char data2[MAXLEN+1] = "";

	if (infile1 == NULL || infile2 == NULL  || infile3 == NULL || infile4 == NULL){
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

		int min_ = rc1 > rc2 ? rc2 : rc1;
		min_ = min_ > rc3 ? rc3 : min_;
		min_ = min_ > rc4 ? rc4 : min_;
		for (i = 0; i < min_; i++){
			// B = (P*K1 ^ C*K1 ^ D*K1 ^ C*K3 ^ D*K4 ^ Q)/(K1 ^ K2)
			// A = P ^ B ^ C ^ D

			unsigned int tem = Galois_mutipile((unsigned int)buf3[i], K[0]) ^ Galois_mutipile((unsigned int)buf1[i], K[0]) ^
				Galois_mutipile((unsigned int)buf2[i], K[0]) ^ Galois_mutipile((unsigned int)buf1[i], K[2]) ^
				Galois_mutipile((unsigned int)buf2[i], K[3]) ^ (unsigned int)buf4[i];
			data2[i] = (unsigned char)Galois_division(tem, K[0] ^ K[1]);
			data1[i] = buf1[i] ^ buf2[i] ^ buf3[i] ^ data2[i];


			// printf("%c  %c  %c  %c   ", buf1[i], buf2[i], buf3[i], buf4[i]);
			// printf("%d  %d  %d  %d   ", (unsigned int)(buf1[i]), (unsigned int)(buf2[i]), (unsigned int)(buf3[i]),
			// 	 (unsigned int)(buf4[i]));
			// printf("%d  %d   ", Galois_division(tem, K[0] ^ K[1]), (unsigned int)data1[i]);
			// printf("data2 = %c   ", data2[i]);
			// char str[10];
			// itoa((unsigned int)data2[i], str, 2);
			// printf("%s  \n", str);
		}

		j++;
		// print_unsigned_char(data1, strlen((char*)data1));
		// print_unsigned_char(data2, strlen((char*)data1));
		fwrite(data1, sizeof(unsigned char), min_, outfile1);
		fwrite(data2, sizeof(unsigned char), min_, outfile2);
	} 
	printf("size = %d KB \n", j);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);
	fclose(infile1);
	fclose(infile2);
	fclose(infile3);
	fclose(infile4);
	fclose(outfile1);
	fclose(outfile2);
}


// lost A, B
// input C, D, ..., P, Q, A_, B_
void restore2(char *argv[], int n) {
	FILE *file[n-2];
	int i, j, size = 0, flag = 1;
	unsigned char buf[n-2][MAXLEN+1];
	int rc[n-4];
	clock_t start, end;
	
	for (i = 2; i < n-2; i++) {
		file[i-2] = fopen(argv[i], "rb");
	}
	file[n-4] = fopen(argv[n-2], "wb");
	file[n-3] = fopen(argv[n-1], "wb");
	
	for (i = 0; i < n-2; i++){
		if (file[i] == NULL){
			printf("%s file not exit\n", argv[i+2]);
			exit(1);
		}
	}

	start = clock();

	while(flag){
		for (i = 0; i < n-4; i++) {
			rc[i] = fread(buf[i], sizeof(unsigned char), MAXLEN, file[i]);
			if (rc[i] == 0) {
				flag = 0;
			}
		}

		// for (i = 0; i < n-4; i++) {
		// 	printf("%d  ", rc[i]);
		// }
		// printf("\n");
		// for (i = 0; i < n-4; i++) {
		// 	print_unsigned_char(buf[i], strlen((char*)buf[i]));
		// }

		int min_ = min(rc, n-4);
		
		// B = (P*K1 ^ C*K1 ^ D*K1 ^ C*K3 ^ D*K4 ^ Q)/(K1 ^ K2)
		// A = P ^ B ^ C ^ D
		for (i = 0; i < min_; i++){
			unsigned short tem = (unsigned short)buf[n-5][i];
			for (j = 0; j < n-4-1; j++){
				tem ^= Galois_mutipile((unsigned short)buf[j][i], K[0]);
			}
			for (j = 0; j < n-4-2; j++){
				tem ^= Galois_mutipile((unsigned short)buf[j][i], K[j+2]);
			}
			tem = Galois_division(tem, K[0] ^ K[1]);
			buf[n-3][i] = (unsigned char)tem;

			for (j = 0; j < n-4-1; j++){
				tem ^= (unsigned short)buf[j][i];
			}
			buf[n-4][i] = (unsigned char)tem;
		}
		
		size++;
	
		fwrite(buf[n-4], sizeof(unsigned char), min_, file[n-4]);
		fwrite(buf[n-3], sizeof(unsigned char), min_, file[n-3]);
	} 
	printf("size = %d KB \n", size-1);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < n-2; i++) {
		fclose(file[i]);
	}
}


// lost A, P
// input B, C, D, Q, A_, P_
void restore1_(char *argv[]) {
	FILE *outfile1, *outfile2, *infile1, *infile2, *infile3, *infile4;
	infile1 = fopen(argv[2], "rb");
	infile2 = fopen(argv[3], "rb");
	infile3 = fopen(argv[4], "rb");
	infile4 = fopen(argv[5], "rb");
	outfile1 = fopen(argv[6], "wb");
	outfile2 = fopen(argv[7], "wb");

	unsigned char buf1[MAXLEN] = "";
	unsigned char buf2[MAXLEN] = "";
	unsigned char buf3[MAXLEN] = "";
	unsigned char buf4[MAXLEN] = "";
	unsigned char data1[MAXLEN] = "";
	unsigned char data2[MAXLEN] = "";

	if (infile1 == NULL || infile2 == NULL  || infile3 == NULL || infile4 == NULL){
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

		int min_ = rc1 > rc2 ? rc2 : rc1;
		min_ = min_ > rc3 ? rc3 : min_;
		min_ = min_ > rc4 ? rc4 : min_;
		for (i = 0; i < min_; i++){
			// A = (B*K2 ^ C*K3 ^ D*K4 ^ Q) / K1
			// P = A ^ B ^ C ^ D

			// printf("%c  %c  %c  %c   ", buf1[i], buf2[i], buf3[i], buf4[i]);
			// printf("%d  %d  %d  %d   ", (unsigned int)(buf1[i]), (unsigned int)(buf2[i]), (unsigned int)(buf3[i]),
			// 	 (unsigned int)(buf4[i]));
			// printf("%d  %d  %d  ", Galois_mutipile((unsigned int)buf1[i], K[1]), Galois_mutipile((unsigned int)buf2[i], K[2])
			// 	, Galois_mutipile((unsigned int)buf3[i], K[3]) );

			unsigned int tem = (unsigned int)buf4[i] ^ Galois_mutipile((unsigned int)buf1[i], K[1]) ^ 
				Galois_mutipile((unsigned int)buf2[i], K[2]) ^ Galois_mutipile((unsigned int)buf3[i], K[3]);
			data1[i] = (unsigned char)Galois_division(tem , K[0]);

			// printf("%d  ", Galois_division(tem , K[0]));
			// printf("%c  \n", (unsigned char)Galois_division(tem , K[0]));

			data2[i] = buf1[i] ^ buf2[i] ^ buf3[i] ^ data1[i];
		}

		j++;
		// print_unsigned_char(data1, strlen((char*)data1));
		// print_unsigned_char(data2, strlen((char*)data1));
		fwrite(data1, sizeof(unsigned char), min_, outfile1);
		fwrite(data2, sizeof(unsigned char), min_, outfile2);
	} 
	printf("size = %d KB \n", j);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);
	fclose(infile1);
	fclose(infile2);
	fclose(infile3);
	fclose(infile4);
	fclose(outfile1);
	fclose(outfile2);
}


// lost A, P
// input B, C, D, ..., Q, A_, P_
void restore1(char *argv[], int n) {
	FILE *file[n-2];
	int i, j, size = 0, flag = 1;
	unsigned char buf[n-2][MAXLEN];
	int rc[n-4];
	clock_t start, end;
	
	for (i = 2; i < n-2; i++) {
		file[i-2] = fopen(argv[i], "rb");
	}
	file[n-4] = fopen(argv[n-2], "wb");
	file[n-3] = fopen(argv[n-1], "wb");
	
	for (i = 0; i < n-2; i++){
		if (file[i] == NULL){
			printf("%s file not exit\n", argv[i+2]);
			exit(1);
		}
	}

	start = clock();

	while(flag){
		for (i = 0; i < n-4; i++) {
			rc[i] = fread(buf[i], sizeof(unsigned char), MAXLEN, file[i]);
			if (rc[i] == 0) {
				flag = 0;
			}
		}

		int min_ = min(rc, n-4);

		// A = (B*K2 ^ C*K3 ^ D*K4 ^ Q) / K1
		// P = A ^ B ^ C ^ D
		for (i = 0; i < min_; i++){
			unsigned short tem = (unsigned short)buf[n-5][i];
			for (j = 0; j < n-5; j++){
				tem ^= Galois_mutipile((unsigned short)buf[j][i], K[j+1]);
			}
			tem = Galois_division(tem , K[0]);
			buf[n-4][i] = (unsigned char)tem;

			for (j = 0; j < n-5; j++){
				tem ^= (unsigned short)buf[j][i];
			}
			buf[n-3][i] = (unsigned char)tem;
		}
		
		size++;
	
		fwrite(buf[n-4], sizeof(unsigned char), min_, file[n-4]);
		fwrite(buf[n-3], sizeof(unsigned char), min_, file[n-3]);
	} 
	printf("size = %d KB \n", size-1);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);
	
	for (i = 0; i < n-2; i++) {
		fclose(file[i]);
	}
}