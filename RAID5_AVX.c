#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<immintrin.h>

#define MAXLEN 16384

void print_unsigned_char(unsigned char buf[MAXLEN], int len);
int min(int rc[], int n);
void redundancy_N(char *argv[], int n);
void redundancy_AVX_N(char *argv[], int n);


int main(int argc, char *argv[]){
	printf("The number of files is %d\n", argc-2);
	printf("Direct calculation:\n");
	redundancy_N(argv, argc);
	printf("Use AVX for acceleration:\n");
	redundancy_AVX_N(argv, argc);

	return 0;
}


void print_unsigned_char(unsigned char buf[MAXLEN], int len){
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


// input  A, B, C, D ..., P
void redundancy_N(char *argv[], int n){
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
	printf("size = %d KB \n", (size-1)*16);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < n-1; i++) {
		fclose(file[i]);
	}
}


// input  A, B, C, D ..., P
void redundancy_AVX_N(char *argv[], int n){
	FILE *file[n-1];
	int i, j, size = 0, flag = 1;
	unsigned char buf[n-1][MAXLEN+1];
	__m256i char_vec[n-1];
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

		int min_ = min(rc, n-2);
		for (i = 0; i < min_; i+=32) {
			char_vec[n-2] = _mm256_setzero_si256();
			for (j = 0; j < n-2; j++) {
				char_vec[j] = _mm256_lddqu_si256((const __m256i*) (buf[j]+i));
				char_vec[n-2] = _mm256_xor_si256(char_vec[n-2], char_vec[j]);
			}

			_mm256_storeu_si256((__m256i*)(buf[n-2]+i), char_vec[n-2]);
			// _mm256_stream_si256((__m256i*)(buf[n-2]+i), char_vec[n-2]);
		}

		// for (i = 0; i < min_; i+=128) {
		// 	__m256i loop_unrolling[8];
		// 	loop_unrolling[4] = _mm256_setzero_si256();
		// 	loop_unrolling[5] = _mm256_setzero_si256();
		// 	loop_unrolling[6] = _mm256_setzero_si256();
		// 	loop_unrolling[7] = _mm256_setzero_si256();
		// 	for (j = 0; j < n-2; j++) {
		// 		loop_unrolling[0] = _mm256_lddqu_si256((const __m256i*) (buf[j]+i));
		// 		loop_unrolling[1] = _mm256_lddqu_si256((const __m256i*) (buf[j]+i+32));
		// 		loop_unrolling[2] = _mm256_lddqu_si256((const __m256i*) (buf[j]+i+64));
		// 		loop_unrolling[3] = _mm256_lddqu_si256((const __m256i*) (buf[j]+i+96));
		// 		loop_unrolling[4] = _mm256_xor_si256(loop_unrolling[4], loop_unrolling[0]);
		// 		loop_unrolling[5] = _mm256_xor_si256(loop_unrolling[5], loop_unrolling[1]);
		// 		loop_unrolling[6] = _mm256_xor_si256(loop_unrolling[6], loop_unrolling[2]);
		// 		loop_unrolling[7] = _mm256_xor_si256(loop_unrolling[7], loop_unrolling[3]);
		// 	}

		// 	_mm256_storeu_si256((__m256i*)(buf[n-2]+i), loop_unrolling[4]);
		// 	_mm256_storeu_si256((__m256i*)(buf[n-2]+i+32), loop_unrolling[5]);
		// 	_mm256_storeu_si256((__m256i*)(buf[n-2]+i+64), loop_unrolling[6]);
		// 	_mm256_storeu_si256((__m256i*)(buf[n-2]+i+96), loop_unrolling[7]);
		// 	// _mm256_stream_si256((__m256i*)(buf[n-2]+i), char_vec[n-2]);
		// }

		size++;
		fwrite(buf[n-2], sizeof(unsigned char), min_, file[n-2]);
	}
	printf("size = %d KB \n", (size-1)*16);

	end = clock();
	printf("time = %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

	for (i = 0; i < n-1; i++) {
		fclose(file[i]);
	}
}