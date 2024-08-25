#include "simple_calc_func.h"

// main함수의 스택 메모리가 부족하므로 힙영역으로 옮겨야 되서 전역 구조체로 선언

Input input[0x400];                  // 최대 1000줄을 저장할 수 있음
Instruction inst_reg[0x800];        // instruction 2024개
Processor prc;


int main() {
    FILE* fp;

    char line[MAX_LINE_LENGTH];
    int ipcnt = 0;              // input.txt 파일에 있는 명령어 줄 개수
    int incnt;                  // instruction의 개수


    // file io
    // "input.txt" 파일을 읽기 : 실패시 종료
    fp = fopen("input.txt", "r");
    if (fp == NULL) {
        printf("input.txt 파일을 열 수 없습니다.\n");
        return 1;
    }

    // 파일의 끝에 도달할 때까지 한 줄씩 읽고 line에 저장,   
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL && (ipcnt < 0x400)) {   // 예외처리 1) input 구조체 배열 인덱스를 초과하여 접근못하게

        if (line[strspn(line, " \t\n")] == '\0') {  // 빈 줄이면 건너뛰기
            continue;
        }


        char* temp = strtok(line, " \n");
        int w_idx = 0;

        // 현재 줄에서 단어를 분리하여 저장
        while (temp != NULL && w_idx < 4) {     // 예외처리 2) w_idx가 4를 넘어가면(input 파일의 한줄에 들어있는 단어개수가 5개 이상이면) out.
            strcpy(input[ipcnt].words[w_idx], temp);
            temp = strtok(NULL, " \n");
            w_idx++;
        }

        input[ipcnt].inst_cnt = w_idx; // 한줄에 들어간 단어 개수 저장
        ipcnt++;
    }

    // 예외처리 1)1024개 이상의 input이 들어오면 1024개 이후는 추가 되지 않고 다음 프로세스로 넘어가는 경고문
    if (ipcnt >= 0x3ff) {
        printf("you cannot put input more than 1024\nonly 1024 inputs can be used\n");
    }


    fclose(fp);
    // file io finish

    // input -> instruction 으로 변환
    incnt = input2inst(input, inst_reg, ipcnt);
    // cpu execute
    init_processor(&prc);
    execute(inst_reg, &prc,incnt);

    return 0;
}
