#include<math.h>
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

void FieldArrayAddDynamicValueConcat(char ***zearray, int *nbfields, int *maxfields, char *zevalue)
{
        if ((*zearray)==NULL) {
                *nbfields=1;
                *maxfields=10;
                (*zearray)=realloc(NULL,(*maxfields)*sizeof(char *));
        } else {
                *nbfields=(*nbfields)+1;
                if (*nbfields>=*maxfields) {
                        *maxfields=(*maxfields)*2;
                        (*zearray)=realloc((*zearray),(*maxfields)*sizeof(char *));
                }
        }
        /* using direct calls because it is more interresting to know which is the caller */
        (*zearray)[(*nbfields)-1]=strdup(zevalue);
        (*zearray)[(*nbfields)]=NULL;
}
char **TxtSplitWithChar(char *in_str, char split_char)
{
        char **tab=NULL;
        char *match_str;
        int redo=1;
        int idx,idmax;

        match_str=in_str;

        while (redo && *in_str) {
                while (*match_str && *match_str!=split_char) match_str++;
                redo=*match_str;
                *match_str=0;
                FieldArrayAddDynamicValueConcat(&tab,&idx,&idmax,in_str);
                in_str=++match_str;
        }

        return tab;
}

struct s_parameter {
	FILE *f;
	char *filename;
	char *crible;

	unsigned int fileOffset,fileLength; // if u dunna want to use all the file
	unsigned int destOffset;
	unsigned int runAt; // offset for the Z80 emulator
	int stopAddress;

	int useMaxNop,useMaxCycle;
	unsigned int maxNop,maxCycle,maxRun;

	int addVec38;
	int genVBL;      // 0:no(default) 1:yes
	int genVBLdelay; // default 312*64 nops
	int genINT;      // 0:no(default) 1:yes
	int genINTdelay; // default 52*64 nops
	int dirtMemory;  // 0:none(default) 1:cpc 2:dirt
	int ioCPC;       // 0:no(default) 1:try to mimic CPC
	int haltStop;
	int eiStop;
	int unknownStop;
	int debug;

	int a,b,c,d,e,regF,h,l,xl,xh,yl,yh,af,bc,de,hl,ix,iy,afp,bcp,dep,hlp,ap,fp,bp,cp,dp,ep,hp,lp,sp;
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

unsigned char crible[256];
unsigned int ncrible;
unsigned int icrible;
unsigned int mcrible;
int rcrible[32]={0};

void initialize_crible(int maxrun) {
	int i=0,j,k;
	int vmax,imax,vmin,imin;

	crible[i++]=0;
	crible[i++]=255;
	crible[i++]=127;
	crible[i++]=1;
	crible[i++]=2;
	crible[i++]=3;
	for (;i<256;i++) {
		vmax=0;
		for (j=0;j<256;j++) {
			for (k=0;k<i;k++) {
				if (crible[k]==j) break; // already assigned
			}
			if (k<i) continue; // skip
					   //
			// find minimum
			vmin=256;
			for (k=0;k<i;k++) {
				if (abs(crible[k]-j)<vmin) {
					vmin=abs(crible[k]-j);
					imin=j;
				}
			}
			// keep max :)
			if (vmin>vmax) {
				imax=imin;
				vmax=vmin;
			}
		}
		// value far away from previous values
		crible[i]=imax;
	}
	// crible distribution ok
	//for (i=0;i<256;i++) printf("#%02X ",crible[i]);
	//printf("\n");

	imax=2;
	while (pow(mcrible,imax)<maxrun) imax++;
	imax--;
	if (imax<2) imax=2;
	printf("crible will target %d different values per register (according to maxrun value)\n",imax);
	ncrible=imax;
	icrible=0;
}

unsigned char crible8() {
	unsigned char ret;
	unsigned int tcrible;

	ret=crible[rcrible[icrible]];
	rcrible[icrible]=(rcrible[icrible]+1)%mcrible;
	tcrible=icrible;
	while (!rcrible[tcrible]) {
		tcrible++; // increment next register until not zero
		rcrible[tcrible]=(rcrible[tcrible]+1)%mcrible;
	}
	icrible=(icrible+1)%mcrible; // next register for next call
	
	return ret;
}

unsigned short int crible16() {
	return (crible8()<<8)|crible8();
}

int EmuZ80(struct s_parameter *param) {
	unsigned int zesize;
	unsigned int INTcpt=0,INTcptMax=64*52;
	unsigned int VBLcpt=0,VBLcptMax=64*312;

	unsigned long long totalNop=0;
	unsigned long long totalRun=0;
	unsigned long minRun,maxRun;
	char **myreg=NULL;
	int ireg,maxreg=0;
	int i;
	z80 z;
	if (param->haltStop) z.breakOnHalt=1;
	if (param->eiStop) z.breakOnEI=1;
	if (param->unknownStop) z.breakOnUnknownInstruction=1;
	if (param->genINTdelay) INTcptMax=param->genINTdelay;
	if (param->genVBLdelay) VBLcptMax=param->genVBLdelay;
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

	if (param->crible) {
		myreg=TxtSplitWithChar(param->crible,',');
		for (maxreg=mcrible=0;myreg[maxreg];maxreg++,mcrible++) { // count registers
			if (stricmp(myreg[ireg],"af")==0) mcrible++; else
			if (stricmp(myreg[ireg],"bc")==0) mcrible++; else
			if (stricmp(myreg[ireg],"de")==0) mcrible++; else
			if (stricmp(myreg[ireg],"hl")==0) mcrible++; else
			if (stricmp(myreg[ireg],"ix")==0) mcrible++; else
			if (stricmp(myreg[ireg],"iy")==0) mcrible++; else
			if (stricmp(myreg[ireg],"afp")==0) mcrible++; else
			if (stricmp(myreg[ireg],"bcp")==0) mcrible++; else
			if (stricmp(myreg[ireg],"dep")==0) mcrible++; else
			if (stricmp(myreg[ireg],"hlp")==0) mcrible++;
		}
		initialize_crible(param->maxRun);
	}

	// reset emulator + memory preparation
	z80_init(&z);
	if (param->ioCPC) {
		z.port_in=cpc_wrapin;
		z.port_out=cpc_wrapout;
	}
	// initialise memory with dirt (or not)
	switch (param->dirtMemory) {
		default:
		case 0:break;
		case 1:for (i=0;i<65536;i++) memory[i]=(i&0x08)?0xFF:0x00;
		case 2:for (i=0;i<65536;i++) memory[i]=rand()&0xFF;break;
	}
	if (param->addVec38) {
		memory[0x38]=0xFB;
		memory[0x39]=0xC9;
	}
	// copy initial programm
	memcpy(&z.userdata[0],memory+param->destOffset,zesize);

	if (!param->maxRun) param->maxRun=1; // execute at least once
					     //
	for (i=0;i<param->maxRun;i++) {
		z.pc=param->runAt;
		z.breakSuccess=0;

		if (param->a!=-1)    z.a=param->a;
		if (param->regF!=-1) z80_set_f(&z,param->regF);
		if (param->b!=-1)    z.b=param->b;
		if (param->c!=-1)    z.c=param->c;
		if (param->d!=-1)    z.d=param->d;
		if (param->e!=-1)    z.e=param->e;
		if (param->h!=-1)    z.h=param->h;
		if (param->l!=-1)    z.l=param->l;
		if (param->xh!=-1)   z.ix|=param->xh<<8;
		if (param->xl!=-1)   z.ix|=param->xl&0xFF;
		if (param->yh!=-1)   z.iy=param->yh<<8;
		if (param->yl!=-1)   z.iy=param->yl&0xFF;
		if (param->sp!=-1)   z.sp=param->sp;
		if (param->af!=-1)   {unsigned char oldf=z.f_;z.a=param->af>>8;z.f_=param->af&0xFF;z80_set_f(&z,z.f_);z.f_=oldf;}
		if (param->bc!=-1)   {z.b=param->bc>>8;z.c=param->bc&0xFF;}
		if (param->de!=-1)   {z.d=param->de>>8;z.e=param->de&0xFF;}
		if (param->hl!=-1)   {z.h=param->hl>>8;z.l=param->hl&0xFF;}
		if (param->ix!=-1)   z.ix=param->ix;
		if (param->iy!=-1)   z.iy=param->iy;
		// EXX
		if (param->ap!=-1)  z.a_=param->ap;
		if (param->fp!=-1)  z.f_=param->fp;
		if (param->bp!=-1)  z.b_=param->bp;
		if (param->cp!=-1)  z.c_=param->cp;
		if (param->dp!=-1)  z.d_=param->dp;
		if (param->ep!=-1)  z.e_=param->ep;
		if (param->hp!=-1)  z.h_=param->hp;
		if (param->lp!=-1)  z.l_=param->lp;
		if (param->afp!=-1) {z.a_=param->afp>>8;z.f_=param->afp&0xFF;}
		if (param->bcp!=-1) {z.b=param->bcp>>8;z.c=param->bcp&0xFF;}
		if (param->dep!=-1) {z.d=param->dep>>8;z.e=param->dep&0xFF;}
		if (param->hlp!=-1) {z.h=param->hlp>>8;z.l=param->hlp&0xFF;}

		if (param->crible) {
			for (ireg=0;myreg[ireg];ireg++) {
				if (stricmp(myreg[ireg],"a")==0) z.a=crible8(); else
				if (stricmp(myreg[ireg],"f")==0) z80_set_f(&z,crible8()); else
				if (stricmp(myreg[ireg],"b")==0) z.b=crible8(); else
				if (stricmp(myreg[ireg],"c")==0) z.c=crible8(); else
				if (stricmp(myreg[ireg],"d")==0) z.d=crible8(); else
				if (stricmp(myreg[ireg],"e")==0) z.e=crible8(); else
				if (stricmp(myreg[ireg],"h")==0) z.h=crible8(); else
				if (stricmp(myreg[ireg],"l")==0) z.l=crible8(); else
				if (stricmp(myreg[ireg],"xh")==0) z.ix=(z.ix&0xFF)|(crible8()<<8); else
				if (stricmp(myreg[ireg],"xl")==0) z.ix=(z.ix&0xFF00)|crible8(); else
				if (stricmp(myreg[ireg],"yh")==0) z.iy=(z.iy&0xFF)|(crible8()<<8); else
				if (stricmp(myreg[ireg],"yl")==0) z.iy=(z.iy&0xFF00)|crible8(); else
				if (stricmp(myreg[ireg],"af")==0) {z.a=crible8();z80_set_f(&z,crible8());} else
				if (stricmp(myreg[ireg],"bc")==0) {z.b=crible8();z.c=crible8();} else
				if (stricmp(myreg[ireg],"de")==0) {z.d=crible8();z.e=crible8();} else
				if (stricmp(myreg[ireg],"hl")==0) {z.h=crible8();z.l=crible8();} else
				if (stricmp(myreg[ireg],"ix")==0) z.ix=crible16(); else
				if (stricmp(myreg[ireg],"iy")==0) z.iy=crible16(); else
				if (stricmp(myreg[ireg],"ap")==0) z.a_=crible8(); else
				if (stricmp(myreg[ireg],"fp")==0) z.f_=crible8(); else
				if (stricmp(myreg[ireg],"bp")==0) z.b_=crible8(); else
				if (stricmp(myreg[ireg],"cp")==0) z.c_=crible8(); else
				if (stricmp(myreg[ireg],"dp")==0) z.d_=crible8(); else
				if (stricmp(myreg[ireg],"ep")==0) z.e_=crible8(); else
				if (stricmp(myreg[ireg],"hp")==0) z.h_=crible8(); else
				if (stricmp(myreg[ireg],"lp")==0) z.l_=crible8(); else
				if (stricmp(myreg[ireg],"afp")==0) {z.a_=crible8();z.f_=crible8();} else
				if (stricmp(myreg[ireg],"bcp")==0) {z.b_=crible8();z.c_=crible8();} else
				if (stricmp(myreg[ireg],"dep")==0) {z.d_=crible8();z.e_=crible8();} else
				if (stricmp(myreg[ireg],"hlp")==0) {z.h_=crible8();z.l_=crible8();} else
				{
					fprintf(stderr,"unknown register to crible [%s]\n",myreg[ireg]);
					exit(2);
				}
			}
		}

		while (1) {
		    if (param->debug) z80_debug_output(&z);
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

		if (!(i%(param->maxRun/10))) printf("Executed in %ld nop(s) (cycles=%ld) MIPS=%.2lf cycleMIPS=%.2lf (@4MHz)\n",z.nop,z.cyc,(double)z.nbinstructions/(double)z.nop,(double)z.nbinstructions/(double)z.cyc*4.0);

		totalNop+=z.nop;
		totalRun++;
		if (!i) minRun=maxRun=z.nop; else {
			if (minRun>z.nop) minRun=z.nop;
			if (maxRun<z.nop) maxRun=z.nop;
		}
	}
	if (totalRun>1) {
		printf("Executed %lld times\n",totalRun);
		printf("Average RUN : %lld\n",totalNop/totalRun);
		printf("Minimum RUN : %ld\n",minRun);
		printf("Maximum RUN : %ld\n",maxRun);
	}
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
	printf("-<reg> <value>              set register value\n");
	printf("                            ex: -bc 0x100\n");
	printf("     reg may be : a,f,b,c,d,e,h,l,xl,xh,yl,yh\n");
	printf("                  af,bc,de,hl,ix,iy\n");
	printf("                  afp,bcp,dep,hlp,ap,fp,bp,cp,dp,ep,hp,lp\n");
	printf("-crible <reg>,<reg>,...     crible registers in the list\n");
	printf("-addVec38                   add EI:RET in #38\n");
	printf("\n");
	printf("run options :\n");
	printf("-maxnop <value>             RUN maximum duration in nops\n");
	printf("-maxCycle <value>           RUN maximum duration in cycles\n");
	printf("-maxRun <value>             RUN multiple times\n");
	printf("-stopAddress <value>        RUN until reaching address\n");
	printf("-eiStop                     RUN until EI\n");
	printf("-haltStop                   RUN until HALT\n");
	printf("-unknownStop                RUN until unknown instruction\n");
	printf("-debug                      ultra verbose information (for debug purpose)\n");
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
		} else if (stricmp(argv[i],"-crible")==0) {
			if (i+1<argc) {
				param->crible=argv[1];
				printf("crible setting [%s]\n",param->crible);
				i++;
			} else {
				fprintf(stderr,"ERROR : Missing argument for -crible option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-fileOffset")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->fileOffset=strtol(argv[1],NULL,16); else
				param->fileOffset=atoi(argv[1]);
				printf("fileOffset setting #%04X (%d)\n",param->fileOffset,param->fileOffset);
				i++;
			} else {
				fprintf(stderr,"ERROR : Missing argument for -fileOffset option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-fileLength")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->fileLength=strtol(argv[1],NULL,16); else
				param->fileLength=atoi(argv[1]);
				printf("fileLength setting #%04X (%d)\n",param->fileLength,param->fileLength);
				i++;
			} else {
				fprintf(stderr,"ERROR : Missing argument for -fileLength option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-destOffset")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->destOffset=strtol(argv[1],NULL,16); else
				param->destOffset=atoi(argv[1]);
				printf("destOffset setting #%04X (%d)\n",param->destOffset,param->destOffset);
				i++;
			} else {
				fprintf(stderr,"ERROR : Missing argument for -destOffset option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-stopAddress")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->stopAddress=strtol(argv[1],NULL,16); else
				param->stopAddress=atoi(argv[1]);
				printf("stopAddress setting #%04X (%d)\n",param->stopAddress,param->stopAddress);
				i++;
			} else {
				fprintf(stderr,"ERROR : Missing argument for -stopAddress option\nRun -h for help\n");
				exit(1);
			}
		} else if (stricmp(argv[i],"-maxrun")==0) {
			if (i+1<argc) {
				if (argv[1][0]=='0' && argv[1][1]=='x') param->maxRun=strtol(argv[1],NULL,16); else
				param->maxRun=atoi(argv[1]);
				printf("maxRun setting #%04X (%d)\n",param->maxRun,param->maxRun);
				i++;
			} else {
				fprintf(stderr,"ERROR : Missing argument for -maxRun option\nRun -h for help\n");
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
		} else if (stricmp(argv[i],"-debug")==0) {
			param->debug=1;
		} else if (stricmp(argv[i],"-addVec38")==0) {
			param->addVec38=1;
		} else if (stricmp(argv[i],"-IOCPC")==0) {
			param->ioCPC=1;
		} else if (stricmp(argv[i],"-unknownStop")==0) {
			param->unknownStop=1;
		} else if (stricmp(argv[i],"-eiStop")==0) {
			param->eiStop=1;
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
	int a,b,c,d,e,regF,h,l,xl,xh,yl,yh,af,bc,de,hl,ix,iy,afp,bcp,dep,hlp,ap,fp,bp,cp,dp,ep,hp,lp;
		} else if (stricmp(argv[i],"-a")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->a=strtol(argv[1],NULL,16); else param->a=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -a option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-b")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->b=strtol(argv[1],NULL,16); else param->b=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -b option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-c")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->c=strtol(argv[1],NULL,16); else param->c=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -c option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-d")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->d=strtol(argv[1],NULL,16); else param->d=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -d option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-e")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->e=strtol(argv[1],NULL,16); else param->e=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -e option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-f")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->regF=strtol(argv[1],NULL,16); else param->regF=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -f option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-h")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->h=strtol(argv[1],NULL,16); else param->h=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -h option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-l")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->l=strtol(argv[1],NULL,16); else param->l=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -l option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-xl")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->xl=strtol(argv[1],NULL,16); else param->xl=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -xl option\nRun -h for help\n"); exit(1); } 
		} else if (stricmp(argv[i],"-xh")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->xh=strtol(argv[1],NULL,16); else param->xh=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -xh option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-yl")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->yl=strtol(argv[1],NULL,16); else param->yl=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -yl option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-yh")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->yh=strtol(argv[1],NULL,16); else param->yh=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -yh option\nRun -h for help\n"); exit(1); }
		} else if (stricmp(argv[i],"-ix")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->ix=strtol(argv[1],NULL,16); else param->ix=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -ix option\nRun -h for help\n"); exit(1); }
		} else if (stricmp(argv[i],"-iy")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->iy=strtol(argv[1],NULL,16); else param->iy=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -iy option\nRun -h for help\n"); exit(1); }

		} else if (stricmp(argv[i],"-af")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->af=strtol(argv[1],NULL,16); else param->af=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -af option\nRun -h for help\n"); exit(1); }
		} else if (stricmp(argv[i],"-bc")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->bc=strtol(argv[1],NULL,16); else param->bc=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -bc option\nRun -h for help\n"); exit(1); }
		} else if (stricmp(argv[i],"-de")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->de=strtol(argv[1],NULL,16); else param->de=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -de option\nRun -h for help\n"); exit(1); }
		} else if (stricmp(argv[i],"-hl")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->hl=strtol(argv[1],NULL,16); else param->hl=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -hl option\nRun -h for help\n"); exit(1); }
		} else if (stricmp(argv[i],"-sp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->sp=strtol(argv[1],NULL,16); else param->sp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -sp option\nRun -h for help\n"); exit(1); }
		// extended registers
		} else if (stricmp(argv[i],"-ap")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->ap=strtol(argv[1],NULL,16); else param->ap=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -ap option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-bp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->bp=strtol(argv[1],NULL,16); else param->bp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -bp option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-cp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->cp=strtol(argv[1],NULL,16); else param->cp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -cp option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-dp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->dp=strtol(argv[1],NULL,16); else param->dp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -dp option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-ep")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->ep=strtol(argv[1],NULL,16); else param->ep=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -ep option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-fp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->fp=strtol(argv[1],NULL,16); else param->fp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -fp option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-hp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->hp=strtol(argv[1],NULL,16); else param->hp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -hp option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-lp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->lp=strtol(argv[1],NULL,16); else param->lp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -lp option\nRun -h for help\n"); exit(1); }  
		} else if (stricmp(argv[i],"-afp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->afp=strtol(argv[1],NULL,16); else param->afp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -afp option\nRun -h for help\n"); exit(1); }
		} else if (stricmp(argv[i],"-bcp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->bcp=strtol(argv[1],NULL,16); else param->bcp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -bcp option\nRun -h for help\n"); exit(1); }
		} else if (stricmp(argv[i],"-dep")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->dep=strtol(argv[1],NULL,16); else param->dep=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -dep option\nRun -h for help\n"); exit(1); }
		} else if (stricmp(argv[i],"-hlp")==0) {
			if (i+1<argc) { if (argv[1][0]=='0' && argv[1][1]=='x') param->hlp=strtol(argv[1],NULL,16); else param->hlp=atoi(argv[1]); i++; } else {
			fprintf(stderr,"ERROR : Missing argument for -hlp option\nRun -h for help\n"); exit(1); }

		} else { fprintf(stderr,"ERROR : Invalid option [%s]\nRun -h for help\n",argv[0]); exit(1); }
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
	param.stopAddress=0x12345;
	param.a=-1; param.regF=-1; param.b=-1; param.c=-1; param.d=-1; param.e=-1;
	param.h=-1; param.l=-1; param.xh=-1; param.xl=-1; param.yh=-1; param.yl=-1;
	param.sp=-1; param.af=-1; param.bc=-1; param.de=-1; param.hl=-1; param.ix=-1;
	param.iy=-1; param.ap=-1; param.fp=-1; param.bp=-1; param.cp=-1; param.dp=-1;
	param.ep=-1; param.hp=-1; param.lp=-1; param.afp=-1; param.bcp=-1; param.dep=-1;
	param.hlp=-1;

	GetParametersFromCommandLine(argc,argv,&param);
	exit(EmuZ80(&param));
}

