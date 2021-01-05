#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<immintrin.h>

#define MAXLEN 16384

int *GFLOG;
int *GFILOG;
int K[255];

void generate_Galois_table();
unsigned short Galois_mutipile(int a, int b);
__m256i Galois_mutipile_AVX(__m256i a, int b);
unsigned short Galois_division(int a, int b);
__m256i Galois_division_AVX(__m256i a, int b);
void print_unsigned_char(unsigned char buf[MAXLEN], int len);
int min(int rc[], int n);
void redundancy_N(char *argv[], int n);
void redundancy_AVX_N(char *argv[], int n);
void restore2_N(char *argv[], int n);
void restore2_AVX_N(char *argv[], int n);
void restore1_N(char *argv[], int n);
void restore1_AVX_N(char *argv[], int n);


int main(int argc, char *argv[]){
	generate_Galois_table();
	int i;
	for (i = 0; i < 255; i++)  K[i] = i+1;

	if (strcmp(argv[1], "redundancy") == 0){
		printf("The number of files is %d\n", argc-4);
		printf("Direct calculation:\n");
		redundancy_N(argv, argc);
		printf("Use AVX for acceleration:\n");
		redundancy_AVX_N(argv, argc);
	} else if (strcmp(argv[1], "restore2") == 0) {
		printf("The number of files is %d\n", argc-4);
		printf("Direct calculation:\n");
		restore2_N(argv, argc);
		printf("Use AVX for acceleration:\n");
		restore2_AVX_N(argv, argc);
	} else if (strcmp(argv[1], "restore1") == 0) {
		printf("The number of files is %d\n", argc-4);
		printf("Direct calculation:\n");
		restore1_N(argv, argc);
		printf("Use AVX for acceleration:\n");
		restore1_AVX_N(argv, argc);
	}

	free(GFLOG);
    free(GFILOG);
	
	return 0;
}


void generate_Galois_table(){
    GFLOG = (int *) malloc (sizeof(int) * 256);
    GFILOG = (int *) malloc (sizeof(int) * 256);
    int b = 1, log;
    for (log = 0; log < 255; log++) {
        GFLOG[b] = (int) log;
        GFILOG[log] = (int) b;
        b = b << 1;
        if (b & 256) b = b ^ 285;
    }
}


unsigned short Galois_mutipile(int a, int b){
    if (a == 0 | b == 0)
    	return 0;
    return (unsigned short)GFILOG[(GFLOG[a]+GFLOG[b])%255];
}


__m256i Galois_mutipile_AVX(__m256i a, int b){
	__m256i res;
	if (b == 0){
		res = _mm256_setzero_si256();
		return res;
	} else {
		// Extension
		// a -> num5 num7 num6 num8 (8->32)
		__m256i c = _mm256_set_epi8(15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 
			12, 8, 4, 0, 15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 12, 8, 4, 0);
		a = _mm256_shuffle_epi8(a, c);

		__m128i num1 = _mm256_extractf128_si256(a, 0);
		__m128i num2 = _mm256_extractf128_si256(a, 1);
		__m256i num3 = _mm256_cvtepu8_epi16(num1);
		__m256i num4 = _mm256_cvtepu8_epi16(num2);
		num1 = _mm256_extractf128_si256(num3, 0);
		num2 = _mm256_extractf128_si256(num3, 1);
		__m256i num8 = _mm256_cvtepu8_epi16(num1);
		__m256i num7 = _mm256_cvtepu8_epi16(num2);
		num1 = _mm256_extractf128_si256(num4, 0);
		num2 = _mm256_extractf128_si256(num4, 1);
		__m256i num6 = _mm256_cvtepu8_epi16(num1);
		__m256i num5 = _mm256_cvtepu8_epi16(num2);

		__m128i lonum5 = _mm256_extractf128_si256(num5, 0);
		__m128i lonum6 = _mm256_extractf128_si256(num6, 0);
		__m128i hinum7 = _mm256_extractf128_si256(num7, 1);
		__m128i hinum8 = _mm256_extractf128_si256(num8, 1);

		num5 = _mm256_insertf128_si256 (num5, hinum7, 0);
		num6 = _mm256_insertf128_si256 (num6, hinum8, 0);
		num7 = _mm256_insertf128_si256 (num7, lonum5, 1);
		num8 = _mm256_insertf128_si256 (num8, lonum6, 1);

		__m256i num5_ = num5;
		__m256i num6_ = num6;
		__m256i num7_ = num7;
		__m256i num8_ = num8;

		// Take numbers from an array
		// num5 num6 num7 num8 -> num5 num6 num7 num8
		num5 = _mm256_i32gather_epi32(GFLOG, num5, 4);
		num6 = _mm256_i32gather_epi32(GFLOG, num6, 4);
		num7 = _mm256_i32gather_epi32(GFLOG, num7, 4);
		num8 = _mm256_i32gather_epi32(GFLOG, num8, 4);

		// Add  
		// num5 num6 num7 num8 + num9 -> num5 num6 num7 num8
		__m256i num9 = _mm256_set1_epi32(GFLOG[b]);

		num5 = _mm256_add_epi32(num5, num9);
		num6 = _mm256_add_epi32(num6, num9);
		num7 = _mm256_add_epi32(num7, num9);
		num8 = _mm256_add_epi32(num8, num9);

		// Modulo
		// % 255
		__m256i num10 = _mm256_set1_epi32(255);
		__m256i num11 = _mm256_set1_epi32(1);

		__m256i num12 = _mm256_cmpgt_epi32(num10, num5);
		num12 = _mm256_add_epi32(num12, num11);
		num5 = _mm256_add_epi32(num5, num12);
		num5 = _mm256_and_si256(num5, num10);

		num12 = _mm256_cmpgt_epi32(num10, num6);
		num12 = _mm256_add_epi32(num12, num11);
		num6 = _mm256_add_epi32(num6, num12);
		num6 = _mm256_and_si256(num6, num10);

		num12 = _mm256_cmpgt_epi32(num10, num7);
		num12 = _mm256_add_epi32(num12, num11);
		num7 = _mm256_add_epi32(num7, num12);
		num7 = _mm256_and_si256(num7, num10);

		num12 = _mm256_cmpgt_epi32(num10, num8);
		num12 = _mm256_add_epi32(num12, num11);
		num8 = _mm256_add_epi32(num8, num12);
		num8 = _mm256_and_si256(num8, num10);

		// Take numbers from an array
		// num5 num6 num7 num8 -> num5 num6 num7 num8
		num5 = _mm256_i32gather_epi32(GFILOG, num5, 4);
		num6 = _mm256_i32gather_epi32(GFILOG, num6, 4);
		num7 = _mm256_i32gather_epi32(GFILOG, num7, 4);
		num8 = _mm256_i32gather_epi32(GFILOG, num8, 4);

		// if a == 0  res = 0
		__m256i num13 = _mm256_set1_epi32(0);
		__m256i num14;

		num14 = _mm256_cmpeq_epi32 (num5_, num13);
		num5 = _mm256_andnot_si256 (num14, num5);

		num14 = _mm256_cmpeq_epi32 (num6_, num13);
		num6 = _mm256_andnot_si256 (num14, num6);

		num14 = _mm256_cmpeq_epi32 (num7_, num13);
		num7 = _mm256_andnot_si256 (num14, num7);

		num14 = _mm256_cmpeq_epi32 (num8_, num13);
		num8 = _mm256_andnot_si256 (num14, num8);

		// return
		res = _mm256_or_si256(
        _mm256_or_si256(_mm256_slli_epi32 (num5, 8*3), _mm256_slli_epi32 (num7, 8*2)),
        _mm256_or_si256(_mm256_slli_epi32 (num6, 8*1), num8));

		return res;
	}
}


unsigned short Galois_division(int a, int b){
	if (a == 0)
		return 0;
	if (b == 0)
		return -1;
	return (unsigned short)GFILOG[((GFLOG[a]-GFLOG[b])+255)%255];
}


__m256i Galois_division_AVX(__m256i a, int b){
	__m256i res;
	if (b == 0){
		res = _mm256_set1_epi8(-1);
		return res;
	} else {
		// Extension
		// a -> num5 num7 num6 num8 (8->32)
		__m256i c = _mm256_set_epi8(15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 
			12, 8, 4, 0, 15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 12, 8, 4, 0);
		a = _mm256_shuffle_epi8(a, c);

		__m128i num1 = _mm256_extractf128_si256(a, 0);
		__m128i num2 = _mm256_extractf128_si256(a, 1);
		__m256i num3 = _mm256_cvtepu8_epi16(num1);
		__m256i num4 = _mm256_cvtepu8_epi16(num2);
		num1 = _mm256_extractf128_si256(num3, 0);
		num2 = _mm256_extractf128_si256(num3, 1);
		__m256i num8 = _mm256_cvtepu8_epi16(num1);
		__m256i num7 = _mm256_cvtepu8_epi16(num2);
		num1 = _mm256_extractf128_si256(num4, 0);
		num2 = _mm256_extractf128_si256(num4, 1);
		__m256i num6 = _mm256_cvtepu8_epi16(num1);
		__m256i num5 = _mm256_cvtepu8_epi16(num2);

		__m128i lonum5 = _mm256_extractf128_si256(num5, 0);
		__m128i lonum6 = _mm256_extractf128_si256(num6, 0);
		__m128i hinum7 = _mm256_extractf128_si256(num7, 1);
		__m128i hinum8 = _mm256_extractf128_si256(num8, 1);

		num5 = _mm256_insertf128_si256 (num5, hinum7, 0);
		num6 = _mm256_insertf128_si256 (num6, hinum8, 0);
		num7 = _mm256_insertf128_si256 (num7, lonum5, 1);
		num8 = _mm256_insertf128_si256 (num8, lonum6, 1);

		__m256i num5_ = num5;
		__m256i num6_ = num6;
		__m256i num7_ = num7;
		__m256i num8_ = num8;

		// Take numbers from an array
		// num5 num6 num7 num8 -> num5 num6 num7 num8
		num5 = _mm256_i32gather_epi32(GFLOG, num5, 4);
		num6 = _mm256_i32gather_epi32(GFLOG, num6, 4);
		num7 = _mm256_i32gather_epi32(GFLOG, num7, 4);
		num8 = _mm256_i32gather_epi32(GFLOG, num8, 4);

		// Sub  
		// num5 num6 num7 num8 + num9 -> num5 num6 num7 num8
		__m256i num9 = _mm256_set1_epi32(GFLOG[b]);

		num5 = _mm256_sub_epi32(num5, num9);
		num6 = _mm256_sub_epi32(num6, num9);
		num7 = _mm256_sub_epi32(num7, num9);
		num8 = _mm256_sub_epi32(num8, num9);

		// Add
		num9 = _mm256_set1_epi32(255);

		num5 = _mm256_add_epi32(num5, num9);
		num6 = _mm256_add_epi32(num6, num9);
		num7 = _mm256_add_epi32(num7, num9);
		num8 = _mm256_add_epi32(num8, num9);

		// Modulo
		// % 255
		__m256i num10 = _mm256_set1_epi32(255);
		__m256i num11 = _mm256_set1_epi32(1);

		__m256i num12 = _mm256_cmpgt_epi32(num10, num5);
		num12 = _mm256_add_epi32(num12, num11);
		num5 = _mm256_add_epi32(num5, num12);
		num5 = _mm256_and_si256(num5, num10);

		num12 = _mm256_cmpgt_epi32(num10, num6);
		num12 = _mm256_add_epi32(num12, num11);
		num6 = _mm256_add_epi32(num6, num12);
		num6 = _mm256_and_si256(num6, num10);

		num12 = _mm256_cmpgt_epi32(num10, num7);
		num12 = _mm256_add_epi32(num12, num11);
		num7 = _mm256_add_epi32(num7, num12);
		num7 = _mm256_and_si256(num7, num10);

		num12 = _mm256_cmpgt_epi32(num10, num8);
		num12 = _mm256_add_epi32(num12, num11);
		num8 = _mm256_add_epi32(num8, num12);
		num8 = _mm256_and_si256(num8, num10);

		// Take numbers from an array
		// num5 num6 num7 num8 -> num5 num6 num7 num8
		num5 = _mm256_i32gather_epi32(GFILOG, num5, 4);
		num6 = _mm256_i32gather_epi32(GFILOG, num6, 4);
		num7 = _mm256_i32gather_epi32(GFILOG, num7, 4);
		num8 = _mm256_i32gather_epi32(GFILOG, num8, 4);

		// if a == 0  res = 0
		__m256i num13 = _mm256_set1_epi32(0);
		__m256i num14;

		num14 = _mm256_cmpeq_epi32 (num5_, num13);
		num5 = _mm256_andnot_si256 (num14, num5);

		num14 = _mm256_cmpeq_epi32 (num6_, num13);
		num6 = _mm256_andnot_si256 (num14, num6);

		num14 = _mm256_cmpeq_epi32 (num7_, num13);
		num7 = _mm256_andnot_si256 (num14, num7);

		num14 = _mm256_cmpeq_epi32 (num8_, num13);
		num8 = _mm256_andnot_si256 (num14, num8);

		// return
		res = _mm256_or_si256(
        _mm256_or_si256(_mm256_slli_epi32 (num5, 8*3), _mm256_slli_epi32 (num7, 8*2)),
        _mm256_or_si256(_mm256_slli_epi32 (num6, 8*1), num8));

		return res;
	}
}


void print_unsigned_char(unsigned char buf[MAXLEN], int len){
	printf("length = %d\n", len);
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


int min(int rc[], int n){
	int min = rc[0];
	int i;
	for (i = 1; i < n; i++) {
		min = min < rc[i] ? min : rc[i];
	}

	return min;
}


// input  A, B, C, D, ..., P, Q
void redundancy_N(char *argv[], int n){
	FILE *file[n-2];
	int i, j, size = 0, flag = 1;
	unsigned char buf[n-2][MAXLEN+1];
	for (i = 0; i < n-2; i++)	memset(buf[i], 0, sizeof(char)*MAXLEN);
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

		fwrite(buf[n-4], sizeof(unsigned char), min_, file[n-4]);
		fwrite(buf[n-3], sizeof(unsigned char), min_, file[n-3]);
	} 
	printf("size = %d KB \n", (size-1)*16);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < n-2; i++) {
		fclose(file[i]);
	}
}


// input  A, B, C, D, ..., P, Q
void redundancy_AVX_N(char *argv[], int n){
	FILE *file[n-2];
	int i, j, size = 0, flag = 1;
	unsigned char buf[n-2][MAXLEN+1];
	for (i = 0; i < n-2; i++)	memset(buf[i], 0, sizeof(unsigned char)*MAXLEN);
	__m256i char_vec[n-2];
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
		for (i = 0; i < min_; i+=32){
			char_vec[n-4] = _mm256_setzero_si256();
			char_vec[n-3] = _mm256_setzero_si256();
			for (j = 0; j < n-4; j++){
				char_vec[j] = _mm256_lddqu_si256((const __m256i*) (buf[j]+i));
				char_vec[n-4] = _mm256_xor_si256(char_vec[n-4], char_vec[j]);

				char_vec[j] = Galois_mutipile_AVX(char_vec[j], K[j]);
				char_vec[n-3] = _mm256_xor_si256(char_vec[n-3], char_vec[j]);
			}
			
			_mm256_storeu_si256((__m256i*)(buf[n-4]+i), char_vec[n-4]);
			_mm256_storeu_si256((__m256i*)(buf[n-3]+i), char_vec[n-3]);
			// _mm256_stream_si256((__m256i*)(buf[n-4]+i), char_vec[n-4]);
			// _mm256_stream_si256((__m256i*)(buf[n-3]+i), char_vec[n-3]);
		}
		
		size++;
		fwrite(buf[n-4], sizeof(unsigned char), min_, file[n-4]);
		fwrite(buf[n-3], sizeof(unsigned char), min_, file[n-3]);
	} 
	printf("size = %d KB \n", (size-1)*16);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < n-2; i++) {
		fclose(file[i]);
	}
}


// lost A, B
// input C, D, ..., P, Q, A_, B_
void restore2_N(char *argv[], int n){
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
	printf("size = %d KB \n", (size-1)*16);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < n-2; i++) {
		fclose(file[i]);
	}
}


// lost A, B
// input C, D, ..., P, Q, A_, B_
void restore2_AVX_N(char *argv[], int n){
	FILE *file[n-2];
	int i, j, size = 0, flag = 1;
	__m256i char_vec[n-2];
	unsigned char buf[n-2][MAXLEN+1];
	for (i = 0; i < n-2; i++)	memset(buf[i], 0, sizeof(unsigned char)*MAXLEN);
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
		
		// B = (P*K1 ^ C*K1 ^ D*K1 ^ C*K3 ^ D*K4 ^ Q)/(K1 ^ K2)
		// A = P ^ B ^ C ^ D
		for (i = 0; i < min_; i+=32){
			char_vec[n-4] = _mm256_setzero_si256();
			char_vec[n-3] = _mm256_lddqu_si256((const __m256i*)(buf[n-5]+i));
			__m256i mul;
			for (j = 0; j < n-4-1; j++){
				char_vec[j] = _mm256_lddqu_si256((const __m256i*) (buf[j]+i));
				mul = Galois_mutipile_AVX(char_vec[j], K[0]);
				char_vec[n-3] = _mm256_xor_si256(char_vec[n-3], mul);
			}
			for (j = 0; j < n-4-2; j++){
				mul = Galois_mutipile_AVX(char_vec[j], K[j+2]);
				char_vec[n-3] = _mm256_xor_si256(char_vec[n-3], mul);
			}
			char_vec[n-3] = Galois_division_AVX(char_vec[n-3], K[0] ^ K[1]);
			_mm256_storeu_si256((__m256i*)(buf[n-3]+i), char_vec[n-3]);

			for (j = 0; j < n-4-1; j++){
				char_vec[n-3] = _mm256_xor_si256(char_vec[n-3], char_vec[j]);
			}
			_mm256_storeu_si256((__m256i*)(buf[n-4]+i), char_vec[n-3]);

		}
		
		size++;
	
		fwrite(buf[n-4], sizeof(unsigned char), min_, file[n-4]);
		fwrite(buf[n-3], sizeof(unsigned char), min_, file[n-3]);
	} 
	printf("size = %d KB \n", (size-1)*16);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < n-2; i++) {
		fclose(file[i]);
	}
}


// lost A, P
// input B, C, D, ..., Q, A_, P_
void restore1_N(char *argv[], int n){
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
	printf("size = %d KB \n", (size-1)*16);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);
	
	for (i = 0; i < n-2; i++) {
		fclose(file[i]);
	}
}


// lost A, P
// input B, C, D, ..., Q, A_, P_
void restore1_AVX_N(char *argv[], int n){
	FILE *file[n-2];
	int i, j, size = 0, flag = 1;
	unsigned char buf[n-2][MAXLEN+1];
	for (i = 0; i < n-2; i++)	memset(buf[i], 0, sizeof(unsigned char)*MAXLEN);
	__m256i char_vec[n-2];
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
		for (i = 0; i < min_; i+=32){
			char_vec[n-4] = _mm256_lddqu_si256((const __m256i*)(buf[n-5]+i));
			__m256i mul;
			for (j = 0; j < n-5; j++){
				char_vec[j] = _mm256_lddqu_si256((const __m256i*)(buf[j]+i));
				mul = Galois_mutipile_AVX(char_vec[j], K[j+1]);
				char_vec[n-4] = _mm256_xor_si256(char_vec[n-4], mul);
			}
			char_vec[n-4] = Galois_division_AVX(char_vec[n-4], K[0]);
			_mm256_storeu_si256((__m256i*)(buf[n-4]+i), char_vec[n-4]);

			for (j = 0; j < n-5; j++){
				char_vec[n-4] = _mm256_xor_si256(char_vec[n-4], char_vec[j]);
			}
			_mm256_storeu_si256((__m256i*)(buf[n-3]+i), char_vec[n-4]);
		}
		
		size++;
	
		fwrite(buf[n-4], sizeof(unsigned char), min_, file[n-4]);
		fwrite(buf[n-3], sizeof(unsigned char), min_, file[n-3]);
	} 
	printf("size = %d KB \n", (size-1)*16);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);
	
	for (i = 0; i < n-2; i++) {
		fclose(file[i]);
	}
}