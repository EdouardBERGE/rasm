#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "z80-master/z80.h"

#ifndef stricmp
// Linus Torvald (c) 1991
int stricmp(const char *s1, const char *s2) {
/* Yes, Virginia, it had better be unsigned */
unsigned char c1, c2;
while (1) {
    c1 = *s1++;
    c2 = *s2++;
    if (!c1 || !c2)
        break; 
    if (c1 == c2)
        continue;
    c1 = tolower(c1);
    c2 = tolower(c2);
    if (c1 != c2)
        break;
}
return (int)c1 - (int)c2;
}
#endif

struct s_parameter {
	FILE *f;
	char *filename;

	unsigned int fileOffset,fileLength; // if u dunna want to use all the file
	unsigned int destOffset;
	unsigned int runAt; // offset for the Z80 emulator
	int stopAddress;

	int useMaxNop,useMaxCycle;
	unsigned int maxNop,maxCycle,maxRun;

	int genVBL;      // 0:no(default) 1:yes
	int genVBLdelay; // default 312*64 nops
	int genINT;      // 0:no(default) 1:yes
	int genINTdelay; // default 52*64 nops
	int dirtMemory;  // 0:none(default) 1:cpc 2:dirt
	int ioCPC;       // 0:no(default) 1:try to mimic CPC
	int haltStop;
	int eiStop;
};

unsigned char memory[65536];
unsigned long hasVBL=0;

#define IS_PPI(port)        ((port & 0x0800) == 0x0000)
uint8_t cpc_wrapin(z80* const z, uint8_t port) {
  // CPC port relies on BC value :)
  unsigned char retVal=0xFF;
  unsigned short int bc;

  bc=(z->b<<8)|z->c;
  if (IS_PPI(bc)) {
	  if ((bc&0x300)==0x100) {
		  // PPI port B
		  retVal=0x10; // 50Hz
		  if (z->nop<hasVBL) {
			  retVal|=1;
		  }
	  }
  }

  return retVal;
}
void cpc_wrapout(z80* const z, uint8_t port, uint8_t val) {
	// pouet pouet
}

int EmuZ80(struct s_parameter *param) {
	unsigned int zesize;
	unsigned int INTcpt=0,INTcptMax=64*52;
	unsigned int VBLcpt=0,VBLcptMax=64*312;
	int i;
	z80 z;
	z80_init(&z);

	z.pc=param->runAt;
	z.breakSuccess=0;
	if (param->haltStop) z.breakOnHalt=1;
	if (param->eiStop) z.breakOnEI=1;
	if (param->genINTdelay) INTcptMax=param->genINTdelay;
	if (param->genVBLdelay) VBLcptMax=param->genVBLdelay;
	switch (param->dirtMemory) {
		default:
		case 0:break;
		case 1:for (i=0;i<65536;i++) memory[i]=i&0xFF;
		case 2:for (i=0;i<65536;i++) memory[i]=rand()&0xFF;break;
	}
	if (param->ioCPC) {
		z.port_in=cpc_wrapin;
		z.port_out=cpc_wrapout;
	}

	fseek(param->f,0,SEEK_END);
	zesize=ftell(param->f);
	fseek(param->f,param->fileOffset,SEEK_SET);
	if (param->fileLength) zesize=param->fileLength;
	if (zesize>65536) {
		zesize=65536;
		fprintf(stderr,"WARNING: Truncated dataSize to 65536 bytes\n");
	}
	if (fread(memory,1,zesize,param->f)!=zesize) {
		fprintf(stderr,"ERROR : cannot read %d bytes of [%s] starting at offset %d\n",zesize,param->filename,param->fileOffset);
		exit(2);
	}
	fclose(param->f);
	if (zesize+param->destOffset>65536) {
		fprintf(stderr,"ERROR : cannot insert %d bytes in memory at offset %d\n",zesize,param->destOffset);
		exit(2);
	}
	memcpy(&z.userdata[0],memory+param->destOffset,zesize);

	while (1) {
    		//z80_debug_output(&z);
	    z80_step(&z);
	    z.nbinstructions++;
	    if (z.breakSuccess) break;
	    if (z.pc==param->stopAddress) break;

	    if (param->genINT) {
		    INTcpt++;
		    if (INTcpt>=INTcptMax) {
			    INTcpt=0;
			    z80_gen_int(&z,0xFF);
		    }
	    }
	    if (param->genVBL) {
		    VBLcpt++;
		    if (VBLcpt>=VBLcptMax) {
			    hasVBL=512+z.nop; // approx 512 nops
		    }
	    }
	}
	printf("executed in %ld nop(s) (cycles=%ld) MIPS=%.2lf cycleMIPS=%.2lf (@4MHz)\n",z.nop,z.cyc,(double)z.nbinstructions/(double)z.nop,(double)z.nbinstructions/(double)z.cyc*4.0);
}

void Usage(int errcode) {
	printf("===========   Z80 profiler   ===========\n");
	printf("Emulation based on Nicolas Allemand work\n");
	printf("\n");
	printf("syntax : emuZ80 <filename> <options>\n");
	printf("\n");
	printf("load options :\n");
	printf("-fileOffset <value>         to skip first bytes\n");
	printf("-fileLength <value>         to truncate reading\n");
	printf("-destOffset <value>         read at memory address\n");
	printf("\n");
	printf("emulation options :\n");
	printf("-VBL                        generate VBL signal\n");
	printf("-VBLdelay <value>           delay between each VBL\n");
	printf("-INT                        generate INT signal\n");
	printf("-INTdelay <value>[,<value>] delay(s) between each INT\n");
	printf("-dirt 0|1|2                 0: use clean memory\n");
	printf("                            1: use CPC like memory\n");
	printf("                            2: use shuffled memory\n");
	printf("-IOCPC                      mimic CPC input/output\n");
	printf("\n");
	printf("emulation init options :\n");
	printf("-pc <value>                 set PC value when starting\n");
	printf("                            ex: -pc 0x100\n");
	printf("\n");
	printf("run options :\n");
	printf("-maxnop <value>             RUN maximum duration in nops\n");
	printf("-maxcycle <value>           RUN maximum duration in cycles\n");
	printf("-maxrun <value>             RUN multiple times\n");
	printf("-haltStop                   RUN until HALT\n");
	printf("-stopAddress <value>        RUN until reaching address\n");
	exit(errcode);
}

int ParseOptions(char **argv,int argc, struct s_parameter *param)
{
	#undef FUNC
	#define FUNC "ParseOptions"
	
	char *sep;
	int i=0;

	if (argv[0][0]=='-') {
		if (stricmp(argv[i],"-h")==0) {
			Usage(0);
		} else if (stricmp(argv[i],"-fileOffset")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->fileOffset=strtol(argv[1],NULL,16); else
				param->fileOffset=atoi(argv[1]);
				printf("fileOffset setting #%04X (%d)\n",param->fileOffset,param->fileOffset);
			} else {
				fprintf(stderr,"ERROR : Missing argument for -fileOffset option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-fileLength")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->fileLength=strtol(argv[1],NULL,16); else
				param->fileLength=atoi(argv[1]);
				printf("fileLength setting #%04X (%d)\n",param->fileLength,param->fileLength);
			} else {
				fprintf(stderr,"ERROR : Missing argument for -fileLength option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-destOffset")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->destOffset=strtol(argv[1],NULL,16); else
				param->destOffset=atoi(argv[1]);
				printf("destOffset setting #%04X (%d)\n",param->destOffset,param->destOffset);
			} else {
				fprintf(stderr,"ERROR : Missing argument for -destOffset option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-stopAddress")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->stopAddress=strtol(argv[1],NULL,16); else
				param->stopAddress=atoi(argv[1]);
				printf("stopAddress setting #%04X (%d)\n",param->stopAddress,param->stopAddress);
			} else {
				fprintf(stderr,"ERROR : Missing argument for -stopAddress option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-INTdelay")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->genINTdelay=strtol(argv[1],NULL,16); else
				param->genINTdelay=atoi(argv[1]);
				printf("INT delay setting #%04X (%d)\n",param->genINTdelay,param->genINTdelay);
			} else {
				fprintf(stderr,"ERROR : Missing argument for -VBLdelay option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-VBLdelay")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->genVBLdelay=strtol(argv[1],NULL,16); else
				param->genVBLdelay=atoi(argv[1]);
				printf("VBL delay setting #%04X (%d)\n",param->genVBLdelay,param->genVBLdelay);
			} else {
				fprintf(stderr,"ERROR : Missing argument for -VBLdelay option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-INT")==0) {
			param->genINT=1;
		} else if (stricmp(argv[i],"-VBL")==0) {
			param->genVBL=1;
		} else if (stricmp(argv[i],"-IOCPC")==0) {
			param->ioCPC=1;
		} else if (stricmp(argv[i],"-haltStop")==0) {
			param->haltStop=1;
		} else if (stricmp(argv[i],"-dirt")==0) {
			if (i+1<argc) {
				i++;
				if (strcmp(argv[0],"0")==0) {}
				else if (strcmp(argv[0],"1")==0) param->dirtMemory=1;
				else if (strcmp(argv[0],"2")==0) param->dirtMemory=2;
				else {
					fprintf(stderr,"ERROR : Invalid argument for -dirt option\nRun -h for help\n");
					exit(1);
				}
			} else {
				fprintf(stderr,"ERROR : Missing argument for -dirt option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-pc")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->runAt=strtol(argv[1],NULL,16); else
				param->runAt=atoi(argv[1]);
				printf("will RUN at #%04X (%d)\n",param->runAt,param->runAt);
				i++;
			} else {
				fprintf(stderr,"ERROR : Missing argument for -pc option\nRun -h for help\n");
				exit(1);
			}
		} else {
			fprintf(stderr,"ERROR : Invalid option [%s]\nRun -h for help\n",argv[0]);
			exit(1);
		}
	} else if (!param->filename) {
		// open file
		param->f=fopen(argv[0],"rb");
		if (param->f) {
			param->filename=argv[0];
			printf("opening [%s] as binary data\n",param->filename);
		} else {
			fprintf(stderr,"ERROR : Cannot open [%s] for reading\n",argv[0]);
			exit(1);
		}
	} else {
		fprintf(stderr,"ERROR : Already have a filename. What is [%s] parameter supposed to do?\nRun -h for help\n",argv[0]);
		exit(1);
	}

	return i;
}

void GetParametersFromCommandLine(int argc, char **argv, struct s_parameter *param) {
	int i;

	if (argc<2) {
		printf("=== Z80 profiler, try -h option to display help\n");
		exit(0);
	}
	for (i=1;i<argc;i++)
		i+=ParseOptions(&argv[i],argc-i,param);

	if (!param->filename) {
		fprintf(stderr,"ERROR : You need at least a <filename> to inject data in Z80 emulation\n");
		Usage(1);
	}
}

void main(int argc, char **argv) {
	struct s_parameter param={0};
	GetParametersFromCommandLine(argc,argv,&param);
	exit(EmuZ80(&param));
}

