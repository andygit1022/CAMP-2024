#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_LINE_LENGTH 256
#define MAX_WORD_LENGTH 20



typedef struct {
    char words[4][10];
    int inst_cnt; //  inst_cnt, inst_num 
} Input;

typedef struct {
    char oper[10];// operator
    char op1[10]; // operend1
    char op2[10]; // operend1
    char op3[10]; // operend1
}Instruction;


typedef struct {
    int registers[10];  // r[0] 은 결과값 넣는 레지스터, 나머지는 전부 temporary 레지스터
    int pc;             // pc : program counter
}Processor;


void init_processor(Processor* p) {
    p->pc = 0x0;
    for (int i = 0; i < 10; i++) {
        p->registers[i] = 1;    // init 되지 않은 레지스터 값을 접근했을때, 오류 코드 대신 1로 계산하게 함.
    }
}

// 각 줄의 데이터를 저장할 구조체



/// rd: destination register number, rt : target register number, md : memory address


void sub(Processor* P, int rd, int rt, int rs) {
    int temp1, temp2;
    temp1 = P->registers[rs];
    temp2 = P->registers[rt];
    P->registers[rd] = P->registers[rt] - P->registers[rs];

    printf("sub\tR%d\tR%d\tR%d", rd, rt, rs);
    printf("\t\tR0 : %d = %d - %d\n", P->registers[rd], temp2, temp1);
}



// add instruction
void add(Processor* P, int rd, int rt, int rs) {
    int temp1, temp2;
    temp1 = P->registers[rs];
    temp2 = P->registers[rt];
    P->registers[rd] = P->registers[rt] + P->registers[rs];
    printf("add\tR%d\tR%d\tR%d", rd, rt, rs);
    printf("\t\tR0 : %d = %d + %d\n", P->registers[rd], temp1, temp2);
}

void mult(Processor* P, int rd, int rt, int rs) {
    int temp1, temp2;
    temp1 = P->registers[rs];
    temp2 = P->registers[rt];
    P->registers[rd] = P->registers[rt] * P->registers[rs];
    printf("mult\tR%d\tR%d\tR%d", rd, rt, rs);
    printf("\t\tR0 : %d = %d * %d\n", P->registers[rd], temp1, temp2);
}

// divi instruction
void divi(Processor* P, int rd, int rt, int rs) {

    // 예외처리 5) zero-division error 방지
    if (P->registers[rs] == 0) {
        printf("you can't divid zero\n");
        return;
    }

    int temp1, temp2;
    temp1 = P->registers[rs];
    temp2 = P->registers[rt];
    P->registers[rd] = P->registers[rt] / P->registers[rs];
    printf("div\tR%d\tR%d\tR%d", rd, rt, rs);
    printf("\t\tR0 : %d = %d / %d\n", P->registers[rd], temp1, temp2);
}

// compare instruction
void com(Processor* P, int rt, int rs) {

    printf("com\tR%d\tR%d", rt, rs);

    if (P->registers[rt] > P->registers[rs]) {
        P->registers[0] = 1;
        printf("\t\t\tR0 : 1\t\t( %d > %d )\n", P->registers[rt], P->registers[rs]);
    }
    else if (P->registers[rt] == P->registers[rs]) {
        P->registers[0] = 0;
        printf("\t\t\tR0 : 0\t\t( %d = %d )\n", P->registers[rt], P->registers[rs]);
    }
    else {
        P->registers[0] = -1;
        printf("\t\t\tR0 : -1\t\t( %d < %d )\n", P->registers[rt], P->registers[rs]);
    }

}

// jump instruction
void jump(Processor* P, int md) {
    printf("jump\t0x%x\t", md);
    printf("\t\t\tjump from %d to %d\n", P->pc, md);
    P->pc = md;
    P->pc--;
}

// beq instruction
void beq(Processor* P, int md) {

    int temp = 0;

    if (P->registers[0] == 0) {
        temp = 1;
        P->pc = md;
        P->pc--;
    }
    printf("beq\t0x%x", md);
    if (temp) {
        printf("\t\t\t\tbranch succeed : move to [PC:%d]\n", md);

    }
    else {
        printf("\t\t\t\tbranch fail : goto next inst\n");
    }
}


// move immediate instruction : actually same with li instruction.
void movi(Processor* P, int rt, int imm) {
    P->registers[rt] = imm;
    printf("movi\tR%d\t%x", rt, imm);
    printf("\t\t\tR%d : %d \n", rt, imm);
}

// mov instrcution : move register to register
void mov(Processor* P, int rt, int rs) {
    P->registers[rt] = P->registers[rs];
    printf("mov\tR%d\t%x", rt, rs);
    printf("\t\t\tR%d to R%d : %d \n", rs, rt, P->registers[rt]);
}


// save_char : save to the Instrcution structure
void save_char(Instruction* i, char* oper, char* op1, char* op2, char* op3, int* a) {

    strcpy(i->oper, oper);
    strcpy(i->op1, op1);
    strcpy(i->op2, op2);
    strcpy(i->op3, op3);
    (*a)++;
}




// 예외처리 3) is_valid 함수들을 통해 input이 잘못됬으면 instruction으로 저장되지 않게

// 유효한 연산자인지 확인하는 함수
int is_oper_valid(const char* op) {
    const char* valid_oper[] = { "+", "-", "*", "/", "M", "MI", "BEQ", "COM", "J" };
    for (int i = 0; i < sizeof(valid_oper) / sizeof(valid_oper[0]); ++i) {
        if (strcmp(op, valid_oper[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// 유효한 피연산자인지 확인하는 함수
int is_operand_valid(const char* operand) {
    if (operand[0] == 'R' && isdigit(operand[1]) && operand[2] == '\0') {
        return 1;
    }
    else if (operand[0] == '0' && operand[1] == 'x') {
        for (int i = 2; operand[i] != '\0'; ++i) {


            if (!isxdigit(operand[i])) {
                return 0; // '0x'로 시작하고, 그 뒤에 16진수 숫자가 오지 않으면 0을 리턴
            }
        }
        return 1; // '0x'로 시작하고, 그 뒤에 16진수 숫자가 오지 않으면 1을 리턴
    }
    return 0;
}






/// 받은 input을 instruction 형태로 변환하여 넣어주는 함수

int input2inst(Input* input, Instruction* inst, int ip_cnt) {   // ip_cnt : file io로 받은 입력 개수


    int inst_cnt = 0;
    printf("[Input to Instruction]\n");

    for (int j = 0; j < ip_cnt; j++) {


        if (!is_oper_valid(input[j].words[0])) {
            printf("%d: Invalid operator '%s'\n", j, input[j].words[0]);
            continue; // 유효하지 않은 연산자면 다음 입력으로 넘어감
        }

        int valid_operands = 1; // 피연산자 유효성 판별
        for (int k = 1; k < input[j].inst_cnt; k++) {
            if (!is_operand_valid(input[j].words[k])) { // 피연산자 유효성 검사
                printf("%d: Invalid operand '%s'\n", j, input[j].words[k]);
                valid_operands = 0; // 유효하지 않은 피연산자 발견
                break; // 유효하지 않은 피연산자가 있으면 for문 중단
            }
        }
        if (!valid_operands) continue; // 유효하지 않은 피연산자가 있으면 다음 입력으로 넘어감

        int logic1, logic2, logic3, logic4, logic_temp1, logic_temp2;

        // 예외처리 4) R또는 0x가 연산자와 적합한지 확인하는 숫자들 ex) MI R0 R2 -> MI는 두번째 연산자에 16진수가 들어가야함.
        logic1 = (input[j].words[1][0] == '0') && (input[j].words[1][1] == 'x');
        logic2 = input[j].words[1][0] == 'R';
        logic3 = (input[j].words[2][0] == '0') && (input[j].words[2][1] == 'x');
        logic4 = input[j].words[2][0] == 'R';
        logic_temp1 = (input[j].inst_cnt == 2);
        logic_temp2 = (input[j].inst_cnt == 3);


        // op1만 사용하는 instruction (두 instruction 모두 흐름 관련 명령어)

        if (input[j].words[0][0] == 'J' && logic_temp1) {
            save_char(&inst[inst_cnt], "jump\0", input[j].words[1], "\0", "\0", &inst_cnt);
            continue;
        }
        else if ((strcmp(input[j].words[0], "BEQ") == 0) && logic_temp1) {
            save_char(&inst[inst_cnt], "beq\0", input[j].words[1], "\0", "\0", &inst_cnt);
            continue;

        }


        // op1과 op2 모두 사용하는 instruction
        if (!((logic3 || logic4))) {     // input이 0x나 R이 아닌 경우를 걸러내기 위해
            printf("%d : you got wrong input(0x or R)\n", j);
            continue;
        }

        // 흐름 관련 명령어 input -> inst 변환      
        if ((strcmp(input[j].words[0], "MI") == 0) && logic_temp2) {
            if (!(logic2 && logic3)) {
                printf("%d : you got wrong input(MI)\n", j);
                continue;
            }
            save_char(&inst[inst_cnt], "movi\0", input[j].words[1], input[j].words[2], "\0", &inst_cnt);
            continue;
        }
        else if (input[j].words[0][0] == 'M' && logic_temp2) {
            if (!(logic2 && logic4)) {
                printf("%d : you got wrong input(M)\n", j);
                continue;
            }
            save_char(&inst[inst_cnt], "mov\0", input[j].words[1], input[j].words[2], "\0", &inst_cnt);
            continue;
        }
        else if ((strcmp(input[j].words[0], "COM") == 0) && logic_temp2) {
            if (!(logic2 && logic4)) {
                printf("%d : you got wrong input(M)\n", j);
                continue;
            }
            save_char(&inst[inst_cnt], "com\0", input[j].words[1], input[j].words[2], "\0", &inst_cnt);
            continue;
        }


        // 사칙연산 관련 명령어 input -> inst 변환


        if ((input[j].words[0][0] == '+') && logic_temp2) {

            if ((logic1 && logic3)) {                         // oper 0x 0x 형태
                save_char(&inst[inst_cnt], "movi\0", "R1\0", input[j].words[1], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "movi\0", "R2\0", input[j].words[2], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "add\0", "R0\0", "R1\0", "R2\0", &inst_cnt);
            }
            else if ((logic1 && logic4)) {                     // oper 0x R 형태
                save_char(&inst[inst_cnt], "movi\0", "R1\0", input[j].words[1], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "add\0", "R0\0", "R1\0", input[j].words[2], &inst_cnt);
            }
            else if ((logic2 && logic3)) {                     // oper R 0x 형태
                save_char(&inst[inst_cnt], "movi\0", "R2\0", input[j].words[2], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "add\0", "R0\0", input[j].words[1], "R2\0", &inst_cnt);
            }
            else if ((logic2 && logic4)) {                     // oper R R 형태
                save_char(&inst[inst_cnt], "add\0", "R0\0", input[j].words[1], input[j].words[2], &inst_cnt);
            }
            else {
                printf("%d : you got wrong input(+)\n", j);
            }
        }
        else if ((input[j].words[0][0] == '-') && logic_temp2) {

            if ((logic1 && logic3)) {                         // oper 0x 0x 형태
                save_char(&inst[inst_cnt], "movi\0", "R1\0", input[j].words[1], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "movi\0", "R2\0", input[j].words[2], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "sub\0", "R0\0", "R1\0", "R2\0", &inst_cnt);
            }
            else if ((logic1 && logic4)) {                     // oper 0x R 형태
                save_char(&inst[inst_cnt], "movi\0", "R1\0", input[j].words[1], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "sub\0", "R0\0", "R1\0", input[j].words[2], &inst_cnt);
            }
            else if ((logic2 && logic3)) {                     // oper R 0x 형태
                save_char(&inst[inst_cnt], "movi\0", "R2\0", input[j].words[2], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "sub\0", "R0\0", input[j].words[1], "R2\0", &inst_cnt);
            }
            else if ((logic2 && logic4)) {                     // oper R R 형태
                save_char(&inst[inst_cnt], "sub\0", "R0\0", input[j].words[1], input[j].words[2], &inst_cnt);
            }
            else {
                printf("%d : you got wrong input(-)\n", j);

            }



        }
        else if ((input[j].words[0][0] == '*') && logic_temp2) {

            if ((logic1 && logic3)) {                         // oper 0x 0x 형태
                save_char(&inst[inst_cnt], "movi\0", "R1\0", input[j].words[1], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "movi\0", "R2\0", input[j].words[2], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "mult\0", "R0\0", "R1\0", "R2\0", &inst_cnt);
            }
            else if ((logic1 && logic4)) {                     // oper 0x R 형태
                save_char(&inst[inst_cnt], "movi\0", "R1\0", input[j].words[1], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "mult\0", "R0\0", "R1\0", input[j].words[2], &inst_cnt);
            }
            else if ((logic2 && logic3)) {                     // oper R 0x 형태
                save_char(&inst[inst_cnt], "movi\0", "R2\0", input[j].words[2], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "mult\0", "R0\0", input[j].words[1], "R2\0", &inst_cnt);
            }
            else if ((logic2 && logic4)) {                     // oper R R 형태
                save_char(&inst[inst_cnt], "mult\0", "R0\0", input[j].words[1], input[j].words[2], &inst_cnt);
            }
            else {
                printf("%d : you got wrong input(*)\n", j);
            }
        }
        else if ((input[j].words[0][0] == '/') && logic_temp2) {
            if ((logic1 && logic3)) {                         // oper 0x 0x 형태
                save_char(&inst[inst_cnt], "movi\0", "R1\0", input[j].words[1], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "movi\0", "R2\0", input[j].words[2], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "divi\0", "R0\0", "R1\0", "R2\0", &inst_cnt);
            }
            else if ((logic1 && logic4)) {                     // oper 0x R 형태
                save_char(&inst[inst_cnt], "movi\0", "R1\0", input[j].words[1], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "divi\0", "R0\0", "R1\0", input[j].words[2], &inst_cnt);
            }
            else if ((logic2 && logic3)) {                     // oper R 0x 형태
                save_char(&inst[inst_cnt], "movi\0", "R2\0", input[j].words[2], "\0", &inst_cnt);
                save_char(&inst[inst_cnt], "divi\0", "R0\0", input[j].words[1], "R2\0", &inst_cnt);
            }
            else if ((logic2 && logic4)) {                     // oper R R 형태
                save_char(&inst[inst_cnt], "divi\0", "R0\0", input[j].words[1], input[j].words[2], &inst_cnt);
            }
            else {
                printf("%d : you got wrong input(/)\n", j);
            }
        }
        else {
            printf("%d : you got wrong input(none of these)\n", j);
            continue;
        }
    }
    return inst_cnt;
}



/// parse_clsfy는 문자열을 R와 0x를 구분해서 정수화 해준 후 종료

int parse(char* a) {

    int temp_int = -1;
    char temp[10];
    int i = 1, j = 0;

    if (a[0] == 'R') {
        while (a[i] != '\0') {
            temp[i - 1] = a[i];
            i++;
        }
        temp_int = atoi(temp);
    }
    else if ((a[0] == '0') && (a[1] == 'x')) {
        temp_int = strtol(a, NULL, 16);
    }
    else {
        printf("got wrong input");
    }
    return temp_int;
}



void execute(Instruction* i, Processor* p, int inst_cnt) {

    int op1, op2, op3;
    p->pc = 0;
    printf("\n\n\n[Execution]\ninst count : %d\n", inst_cnt);

    while (p->pc != inst_cnt) {
        printf("PC : %d  \t", p->pc);
        if (strcmp(i[p->pc].oper, "mov") == 0) {
            op1 = parse(i[p->pc].op1);
            op2 = parse(i[p->pc].op2);
            mov(p, op1, op2);
        }
        else if (strcmp(i[p->pc].oper, "movi") == 0) {
            op1 = parse(i[p->pc].op1);
            op2 = parse(i[p->pc].op2);
            movi(p, op1, op2);
        }
        else if (strcmp(i[p->pc].oper, "beq") == 0) {
            op1 = parse(i[p->pc].op1);
            beq(p, op1);
        }
        else if (strcmp(i[p->pc].oper, "jump") == 0) {
            op1 = parse(i[p->pc].op1);
            jump(p, op1);
        }
        else if (strcmp(i[p->pc].oper, "com") == 0) {
            op1 = parse(i[p->pc].op1);
            op2 = parse(i[p->pc].op2);
            com(p, op1, op2);
        }
        else if (strcmp(i[p->pc].oper, "divi") == 0) {
            op1 = parse(i[p->pc].op1);
            op2 = parse(i[p->pc].op2);
            op3 = parse(i[p->pc].op3);

            divi(p, op1, op2, op3);

        }
        else if (strcmp(i[p->pc].oper, "add") == 0) {
            op1 = parse(i[p->pc].op1);
            op2 = parse(i[p->pc].op2);
            op3 = parse(i[p->pc].op3);

            add(p, op1, op2, op3);

        }
        else if (strcmp(i[p->pc].oper, "mult") == 0) {
            op1 = parse(i[p->pc].op1);
            op2 = parse(i[p->pc].op2);
            op3 = parse(i[p->pc].op3);

            mult(p, op1, op2, op3);

        }
        else if (strcmp(i[p->pc].oper, "sub") == 0) {
            op1 = parse(i[p->pc].op1);
            op2 = parse(i[p->pc].op2);
            op3 = parse(i[p->pc].op3);
            sub(p, op1, op2, op3);
        }
        else {
            printf("\n\n\n\tWARNING : YOU GOT SOMETHING WRONG!!\n\n\n");
        }
        p->pc++;
    }

}

