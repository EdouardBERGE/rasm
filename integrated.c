#define INTEGRATED_RASM
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include "rasm.h"



void main() {
	struct s_parameter param={0};
	struct s_rasm_info *debug=NULL;
	unsigned char *opcode=NULL;
	int opcodelen;
	int ret;
	int i;
#define AUTOTEST_EDSK_SIDE_01 " buildsna force,'~/grouik/grouik/grouik' : bankset 0 : nop"
        memset(&param,0,sizeof(struct s_parameter)); //param.edskoverwrite=1;
        ret=RasmAssembleInfoParam(AUTOTEST_EDSK_SIDE_01,strlen(AUTOTEST_EDSK_SIDE_01),&opcode,&opcodelen,&debug,&param);
        if (!ret) {} else {printf("Autotest ERROR\n");
                for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);
                exit(-1);}
	RasmFreeInfoStruct(debug);
        if (opcode) free(opcode);opcode=NULL;
printf("buildsna with wrong path OK\n");

}


