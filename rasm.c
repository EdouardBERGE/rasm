#define PROGRAM_NAME      "RASM"
#define PROGRAM_VERSION   "1.4 nightly"
#define PROGRAM_DATE      "xx/11/2020"
#define PROGRAM_COPYRIGHT "© 2017 BERGE Edouard / roudoudou from Resistance"

#define RASM_VERSION PROGRAM_NAME" v"PROGRAM_VERSION" (build "PROGRAM_DATE")"
#define RASM_SNAP_VERSION PROGRAM_NAME" v"PROGRAM_VERSION

#define TRACE_GENERALE 0
#define TRACE_PREPRO 0
#define TRACE_ASSEMBLE 0
#define TRACE_COMPUTE_EXPRESSION 0
#define TRACE_HEXBIN 0
#define TRACE_MAKEAMSDOSREAL 0
#define TRACE_STRUCT 0
#define TRACE_EDSK 0
#define TRACE_LABEL 0



/***
Rasm (roudoudou assembler) Z80 assembler

doc & latest official release at: https://github.com/EdouardBERGE/rasm

You may send requests/bugs in the same topic

-----------------------------------------------------------------------------------------------------
This software is using MIT "expat" license

« Copyright © BERGE Edouard (roudoudou)

Permission  is  hereby  granted,  free  of charge,to any person obtaining a copy  of  this  software
and  associated  documentation/source   files   of RASM, to deal in the Software without restriction,
including without limitation the  rights  to  use, copy,   modify,   merge,   publish,    distribute,
sublicense,  and/or  sell  copies of the Software, and  to  permit  persons  to  whom the Software is
furnished  to  do  so,  subject  to  the following conditions:

The above copyright  notice  and  this  permission notice   shall   be  included  in  all  copies  or
substantial portions of the Software.
The   Software   is   provided  "as is",   without warranty   of   any   kind,  express  or  implied,
including  but  not  limited  to the warranties of merchantability,   fitness   for   a    particular
purpose  and  noninfringement.  In  no event shall the  authors  or  copyright  holders be liable for
any  claim, damages  or other  liability,  whether in  an  action  of  contract, tort  or  otherwise,
arising from,  out of  or in connection  with  the software  or  the  use  or  other  dealings in the
Software. »
-----------------------------------------------------------------------------------------------------
Linux compilation with GCC or Clang:
cc rasm.c -O2 -lm -lrt -march=native -o rasm
strip rasm

Windows compilation with Visual studio:
cl.exe rasm.c -O2 -Ob3

pure MS-DOS 32 bits compilation with Watcom without native support of AP-Ultra:
wcl386 rasm.c -6r -6s -fp6 -d0 -k4000000 -ox /bt=DOS /l=dos4g -DOS_WIN=1 -DNOAPLIB=1

MorphOS compilation (ixemul):
gcc -noixemul -O2 -c -o rasm rasm.c
strip rasm

MacOS compilation:
cc rasm.c -O2 -lm -march=native -o rasm

*/

#ifdef __WATCOMC__
#define OS_WIN 1
#endif

#ifdef _WIN32
#define OS_WIN 1
#endif

#ifdef _WIN64
#define OS_WIN 1
#endif

#ifndef RDD
	/* public lib */
	#include"minilib.h"
#else
	/* private dev lib wont be published */
	#include"../tools/library.h"
	#define TxtSplitWithChar _internal_TxtSplitWithChar
#endif

#ifndef NO_3RD_PARTIES
#define __FILENAME__ "3rd parties"
/* 3rd parties compression */
#include"zx7.h"
#include"lz4.h"
#include"exomizer.h"
#endif

#ifdef __MORPHOS__
/* Add standard version string to executable */
const char __attribute__((section(".text"))) ver_version[]={ "\0$VER: "PROGRAM_NAME" "PROGRAM_VERSION" ("PROGRAM_DATE") "PROGRAM_COPYRIGHT"" };
/* Expand the default stack to match rasm requirements (about 64KiB) */
unsigned long __stack = 128 * 1024;
#endif

#undef __FILENAME__
#define __FILENAME__ "rasm.c"

#ifndef OS_WIN
#define KNORMAL  "\x1B[0m"
#define KERROR   "\x1B[31m"
#define KAYGREEN "\x1B[32m"
#define KWARNING "\x1B[33m"
#define KBLUE    "\x1B[34m"
#define KVERBOSE "\x1B[36m"
#define KIO      "\x1B[97m"
#else
#define KNORMAL  ""
#define KERROR   "Error: "
#define KAYGREEN ""
#define KWARNING "Warning: "
#define KBLUE    ""
#define KVERBOSE ""
#define KIO      ""
#endif

/*******************************************************************
         c o m m a n d    l i n e    p a r a m e t e r s 
*******************************************************************/
enum e_dependencies_type {
E_DEPENDENCIES_NO=0,
E_DEPENDENCIES_LIST,
E_DEPENDENCIES_MAKE
};

struct s_parameter {
	char **labelfilename;
	char *filename;
	char *outputfilename;
	int automatic_radix;
	int export_local;
	int export_var;
	int export_equ;
	int export_sym;
	int export_multisym;
	int export_tape;
	char *flexible_export;
	int export_sna;
	int export_snabrk;
	int export_brk;
	int nowarning;
	int erronwarn;
	int checkmode;
	int dependencies;
	int maxerr;
	int macrovoid;
	int extended_error;
	int display_stats;
	int edskoverwrite;
	int xpr;
	float rough;
	int as80,dams,pasmo;
	int v2;
	int warn_unused;
	char *symbol_name;
	char *binary_name;
	char *cartridge_name;
	char *snapshot_name;
	char *rom_name;
	char *tape_name;
	char *breakpoint_name;
	char **symboldef;
	int nsymb,msymb;
	char **pathdef;
	int npath,mpath;
	int noampersand;
	char module_separator;
};



/*******************************************************************
 c o m p u t e   o p e r a t i o n s   f o r   c a l c u l a t o r
*******************************************************************/

enum e_compute_operation_type {
E_COMPUTE_OPERATION_PUSH_DATASTC=0,
E_COMPUTE_OPERATION_OPEN=1,
E_COMPUTE_OPERATION_CLOSE=2,
E_COMPUTE_OPERATION_ADD=3,
E_COMPUTE_OPERATION_SUB=4,
E_COMPUTE_OPERATION_DIV=5,
E_COMPUTE_OPERATION_MUL=6,
E_COMPUTE_OPERATION_AND=7,
E_COMPUTE_OPERATION_OR=8,
E_COMPUTE_OPERATION_MOD=9,
E_COMPUTE_OPERATION_XOR=10,
E_COMPUTE_OPERATION_NOT=11,
E_COMPUTE_OPERATION_SHL=12,
E_COMPUTE_OPERATION_SHR=13,
E_COMPUTE_OPERATION_BAND=14,
E_COMPUTE_OPERATION_BOR=15,
E_COMPUTE_OPERATION_LOWER=16,
E_COMPUTE_OPERATION_GREATER=17,
E_COMPUTE_OPERATION_EQUAL=18,
E_COMPUTE_OPERATION_NOTEQUAL=19,
E_COMPUTE_OPERATION_LOWEREQ=20,
E_COMPUTE_OPERATION_GREATEREQ=21,
/* math functions */
E_COMPUTE_OPERATION_SIN=22,
E_COMPUTE_OPERATION_COS=23,
E_COMPUTE_OPERATION_INT=24,
E_COMPUTE_OPERATION_FLOOR=25,
E_COMPUTE_OPERATION_ABS=26,
E_COMPUTE_OPERATION_LN=27,
E_COMPUTE_OPERATION_LOG10=28,
E_COMPUTE_OPERATION_SQRT=29,
E_COMPUTE_OPERATION_ASIN=30,
E_COMPUTE_OPERATION_ACOS=31,
E_COMPUTE_OPERATION_ATAN=32,
E_COMPUTE_OPERATION_EXP=33,
E_COMPUTE_OPERATION_LOW=34,
E_COMPUTE_OPERATION_HIGH=35,
E_COMPUTE_OPERATION_PSG=36,
E_COMPUTE_OPERATION_RND=37,
E_COMPUTE_OPERATION_FRAC=38,
E_COMPUTE_OPERATION_CEIL=39,
E_COMPUTE_OPERATION_GET_R=40,
E_COMPUTE_OPERATION_GET_V=41,
E_COMPUTE_OPERATION_GET_B=42,
E_COMPUTE_OPERATION_SET_R=43,
E_COMPUTE_OPERATION_SET_V=44,
E_COMPUTE_OPERATION_SET_B=45,
E_COMPUTE_OPERATION_SOFT2HARD=46,
E_COMPUTE_OPERATION_HARD2SOFT=47,
/* string functions */
E_COMPUTE_OPERATION_GETNOP=48,
E_COMPUTE_OPERATION_GETTICK=49,
E_COMPUTE_OPERATION_DURATION=50,
E_COMPUTE_OPERATION_END=51
};

struct s_compute_element {
enum e_compute_operation_type operator;
double value;
int priority;
char *string;
};

struct s_compute_core_data {
	/* evaluator v3 may be recursive */
	char *varbuffer;
	int maxivar;
	struct s_compute_element *tokenstack;
	int maxtokenstack;
	struct s_compute_element *operatorstack;
	int maxoperatorstack;
};

/***********************************************************
  w a v   h e a d e r    f o r    a u d i o    i m p o r t
***********************************************************/
struct s_wav_header {
char ChunkID[4];
unsigned char ChunkSize[4];
char Format[4];
char SubChunk1ID[4];
unsigned char SubChunk1Size[4];
unsigned char AudioFormat[2];
unsigned char NumChannels[2];
unsigned char SampleRate[4];
unsigned char ByteRate[4];
unsigned char BlockAlign[2];
unsigned char BitsPerSample[2];
unsigned char SubChunk2ID[4];
unsigned char SubChunk2Size[4];
};

enum e_audio_sample_type {
AUDIOSAMPLE_SMP,
AUDIOSAMPLE_SM2,
AUDIOSAMPLE_SM4,
AUDIOSAMPLE_DMAA,
AUDIOSAMPLE_DMAB,
AUDIOSAMPLE_DMAC,
AUDIOSAMPLE_END
};

/***********************************************************************
  e x p r e s s i o n   t y p e s   f o r   d e l a y e d   w r i t e
***********************************************************************/
enum e_expression {
	E_EXPRESSION_J8,     /* relative 8bits jump */
	E_EXPRESSION_0V8,    /* 8 bits value to current adress */
	E_EXPRESSION_V8,     /* 8 bits value to current adress+1 */
	E_EXPRESSION_V16,    /* 16 bits value to current adress+1 */
	E_EXPRESSION_V16C,   /* 16 bits value to current adress+1 */
	E_EXPRESSION_0V16,   /* 16 bits value to current adress */
	E_EXPRESSION_0V32,   /* 32 bits value to current adress */
	E_EXPRESSION_0VR,    /* AMSDOS real value (5 bytes) to current adress */
	E_EXPRESSION_0VRMike,/* Microsoft IEEE-754 real value (5 bytes) to current adress */
	E_EXPRESSION_IV8,    /* 8 bits value to current adress+2 */
	E_EXPRESSION_IV81,   /* 8 bits value+1 to current adress+2 */
	E_EXPRESSION_3V8,    /* 8 bits value to current adress+3 used with LD (IX+n),n */
	E_EXPRESSION_IV16,   /* 16 bits value to current adress+2 */
	E_EXPRESSION_RST,    /* the offset of RST is translated to the opcode */
	E_EXPRESSION_IM,     /* the interrupt mode is translated to the opcode */
	E_EXPRESSION_RUN,    /* delayed RUN value */
	E_EXPRESSION_ZXRUN,  /* delayed RUN value for ZX snapshot */
	E_EXPRESSION_ZXSTACK,/* delayed STACK value for ZX snapshot */
	E_EXPRESSION_BRS     /* delayed shifting for BIT, RES, SET */
};

struct s_expression {	
	char *reference;          /* backup when used inside loop (or macro?) */
	int iw;                   /* word index in the main wordlist */
	int o;                    /* offset de depart 0, 1 ou 3 selon l'opcode */
	int ptr;                  /* offset courant pour calculs relatifs */
	int wptr;                 /* where to write the result  */
	enum e_expression zetype; /* type of delayed write */
	int lz;                   /* lz zone */
	int ibank;                /* ibank of expression */
	int iorgzone;             /* org of expression */
	char *module;
};

struct s_expr_dico {
	char *name;
	int crc;
	int autorise_export;
	double v;
	int used;
	int iw;
};

struct s_label {
	char *name;   /* is alloced for local repeat or struct OR generated global -> in this case iw=-1 */
	int iw;       /* index of the word of label name */
	int crc;      /* crc of the label name */
	int ptr;      /* "physical" adress */
	int lz;       /* is the label in a crunched section (or after)? */
	int iorgzone; /* org of label */
	int ibank;    /* current CPR bank / always zero in classic mode */
	int local;
	/* errmsg */
	int fileidx;
	int fileline;
	int autorise_export,backidx;
	int used;
};

struct s_alias {
	char *alias;
	char *translation;
	int crc,len,autorise_export;
	int iw;
	int used;
};

struct s_ticker {
	char *varname;
	int crc;
	long nopstart;
	long tickerstart;
};

/***********************************************************************
   m e r k e l    t r e e s    f o r    l a b e l,  v a r,  a l i a s
***********************************************************************/
struct s_crclabel_tree {
	struct s_crclabel_tree *radix[256];
	struct s_label *label;
	int nlabel,mlabel;
};
struct s_crcdico_tree {
	struct s_crcdico_tree *radix[256];
	struct s_expr_dico *dico;
	int ndico,mdico;
};
struct s_crcused_tree {
	struct s_crcused_tree *radix[256];
	char **used;
	int nused,mused;
};
struct s_crcstring_tree {
	struct s_crcstring_tree *radix[256];
	char **text;
	int ntext,mtext;
	char **replace;
	int nreplace,mreplace;
};
/*************************************************
          m e m o r y    s e c t i o n
*************************************************/
struct s_lz_section {
	int iw;
	int memstart,memend;
	int lzversion; /* 0 -> NO CRUNCH but must be delayed / 4 -> LZ4 / 7 -> ZX7 / 48 -> LZ48 / 49 -> LZ49 / 8 -> Exomizer */
	int version,minmatch; /* LZSA */
	int iorgzone;
	int ibank;
	/* idx backup */
	int iexpr;
	int ilabel;
};

struct s_orgzone {
	int ibank,protect;
	int memstart,memend;
	int ifile,iline;
	int nocode;
	int inplace;
};

/**************************************************
         i n c b i n     s t o r a g e
**************************************************/
struct s_hexbin {
	unsigned char *data;
	int datalen,rawlen;
	char *filename;
	int crunch;
	int version,minmatch;
};

/**************************************************
          e d s k    m a n a g e m e n t        
**************************************************/
struct s_edsk_sector_global_struct {
unsigned char track;
unsigned char side;
unsigned char id;
unsigned char size;
unsigned char st1;
unsigned char st2;
unsigned short int length;
unsigned char *data;
};

struct s_edsk_track_global_struct  {
int sectornumber;
/* information purpose */
int sectorsize;
int gap3length;
int fillerbyte;
int datarate;
int recordingmode;
struct s_edsk_sector_global_struct *sector;
};

struct s_edsk_global_struct {
int tracknumber;
int sidenumber;
int tracksize; /* DSK legacy */
struct s_edsk_track_global_struct *track;
};

struct s_edsk_wrapper_entry {
unsigned char user;
unsigned char filename[11];
unsigned char subcpt;
unsigned char extendcounter;
unsigned char reserved;
unsigned char rc;
unsigned char blocks[16];
};

struct s_edsk_wrapper {
char *edsk_filename;
struct s_edsk_wrapper_entry entry[64];
int nbentry;
unsigned char blocks[178][1024]; /* DATA format */
int face;
};

struct s_save {
	int ibank;
	int ioffset;
	int isize;
	int iw,irun;
	int amsdos,hobeta;
	int tape,dsk,face,iwdskname;
};


/********************
      L O O P S
********************/

enum e_loop_style {
E_LOOPSTYLE_REPEATN,
E_LOOPSTYLE_REPEATUNTIL,
E_LOOPSTYLE_WHILE
};

struct s_repeat {
	int start;
	int cpt;
	int value;
	int maxim;
	int repeat_counter;
	char *repeatvar;
	int repeatcrc;
};

struct s_whilewend {
	int start;
	int cpt;
	int value;
	int maxim;
	int while_counter;
};

struct s_switchcase {
	int refval;
	int execute;
	int casematch;
};

struct s_repeat_index {
	int ifile;
	int ol,oidx;
	int cl,cidx;
};


enum e_ifthen_type {
E_IFTHEN_TYPE_IF=0,
E_IFTHEN_TYPE_IFNOT=1,
E_IFTHEN_TYPE_IFDEF=2,
E_IFTHEN_TYPE_IFNDEF=3,
E_IFTHEN_TYPE_ELSE=4,
E_IFTHEN_TYPE_ELSEIF=5,
E_IFTHEN_TYPE_IFUSED=6,
E_IFTHEN_TYPE_IFNUSED=7,
E_IFTHEN_TYPE_END
};

struct s_ifthen {
	char *filename;
	int line,v;
	enum e_ifthen_type type;
};

/**************************************************
          w o r d    p r o c e s s i n g
**************************************************/
struct s_wordlist {
	char *w;
	int l,t,e; /* e=1 si egalite dans le mot */
	int ifile;
};

struct s_macro {
	char *mnemo;
	int crc;
	/* une macro concatene des chaines et des parametres */
	struct s_wordlist *wc;
	int nbword,maxword;
	/**/
	char **param;
	int nbparam;
};

struct s_macro_position {
	int start,end,value,level,pushed;
	//char *lastlocal;
	//int lastlocalen,lastlocalalloc;
};

/* preprocessing only */
struct s_macro_fast {
	char *mnemo;
	int crc;
};

struct s_math_keyword {
	char *mnemo;
	int crc;
	enum e_compute_operation_type operation;
};

struct s_expr_word {
	char *w;
	int aw;
	int op;
	int comma;
	int fct;
	double v;
};

struct s_listing {
	char *listing;
	int ifile;
	int iline;
};

enum e_tagtranslateoption {
E_TAGOPTION_NONE=0,
E_TAGOPTION_REMOVESPACE=1,
E_TAGOPTION_PRESERVE=2
};

#ifdef RASM_THREAD
struct s_rasm_thread {
	pthread_t thread;
	int lz;
	unsigned char *datain;
	int datalen;
	unsigned char *dataout;
	int lenout;
	int status;
};
#endif


/*********************************************************
            S N A P S H O T     E X P O R T
*********************************************************/
/* extension 4Mo = 256 slots + 4 slots 64K de RAM par défaut => 260 */

#define BANK_MAX_NUMBER 260

struct s_snapshot_symbol {
	unsigned char size;
	unsigned char name[256];
	unsigned char reserved[6];
	unsigned char bigendian_adress[2];
};


struct s_zxsnapshot {
	
	unsigned int run;
	unsigned int stack;
};

struct s_snapshot {
	char idmark[8];
	char unused1[8];
	unsigned char version; /* 3 */
	struct {
		struct {
			unsigned char F;
			unsigned char A;
			unsigned char C;
			unsigned char B;
			unsigned char E;
			unsigned char D;
			unsigned char L;
			unsigned char H;
		}general;
		unsigned char R;
		unsigned char regI; /* I incompatible with tgmath.h */
		unsigned char IFF0;
		unsigned char IFF1;
		unsigned char LX;
		unsigned char HX;
		unsigned char LY;
		unsigned char HY;
		unsigned char LSP;
		unsigned char HSP;
		unsigned char LPC;
		unsigned char HPC;
		unsigned char IM; /* 0,1,2 */
		struct {
			unsigned char F;
			unsigned char A;
			unsigned char C;
			unsigned char B;
			unsigned char E;
			unsigned char D;
			unsigned char L;
			unsigned char H;
		}alternate;
	}registers;
		
	struct {
		unsigned char selectedpen;
		unsigned char palette[17];
		unsigned char multiconfiguration;
	}gatearray;
	unsigned char ramconfiguration;
	struct {
		unsigned char selectedregister;
		unsigned char registervalue[18];
	}crtc;
	unsigned char romselect;
	struct {
		unsigned char portA;
		unsigned char portB;
		unsigned char portC;
		unsigned char control;
	}ppi;
	struct {
		unsigned char selectedregister;
		unsigned char registervalue[16];
	}psg;
	unsigned char dumpsize[2]; /* 64 then use extended memory chunks */
	
	unsigned char CPCType; /* 0=464 / 1=664 / 2=6128 / 4=6128+ / 5=464+ / 6=GX4000 */
	unsigned char interruptnumber;
	unsigned char multimodebytes[6];
	unsigned char unused2[0x9C-0x75];
	
	/* offset #9C */
	struct {
		unsigned char motorstate;
		unsigned char physicaltrack;
	}fdd;
	unsigned char unused3[3];
	unsigned char printerstrobe;
	unsigned char unused4[2];
	struct {
		unsigned char model; /* 0->4 */
		unsigned char unused5[4];
		unsigned char HCC;
		unsigned char unused;
		unsigned char CLC;
		unsigned char RLC;
		unsigned char VTC;
		unsigned char HSC;
		unsigned char VSC;
		unsigned short int flags;
	}crtcstate;
	unsigned char vsyncdelay;
	unsigned char interruptscanlinecounter;
	unsigned char interruptrequestflag;
	unsigned char unused6[0xFF-0xB5+1];
};

struct s_snapshot_chunks {
	unsigned char chunkname[4]; /* MEM1 -> MEM8 */
	unsigned int chunksize;
};

struct s_breakpoint {
	int address;
	int bank;
};


/*********************************
        S T R U C T U R E S
*********************************/
enum e_rasmstructfieldtype {
E_RASMSTRUCTFIELD_BYTE,
E_RASMSTRUCTFIELD_WORD,
E_RASMSTRUCTFIELD_LONG,
E_RASMSTRUCTFIELD_REAL,
E_RASMSTRUCTFIELD_END
};
struct s_rasmstructfield {
	char *fullname;
	char *name;
	int offset;
	int size;
	int crc;
	/* filler */
	unsigned char *data;
	int idata,mdata;
	enum e_rasmstructfieldtype zetype;
};

struct s_rasmstruct {
	char *name;
	int crc;
	int size;
	int ptr;
	int nbelem;
	/* fields */
	struct s_rasmstructfield *rasmstructfield;
	int irasmstructfield,mrasmstructfield;
};

/*********************************
           D E B U G        
*********************************/
struct s_debug_error {
        char *filename;
        int line;
        char *msg;
        int lenmsg,lenfilename;
};
struct s_debug_symbol {
        char *name;
        int v;
};
struct s_rasm_info {
        struct s_debug_error *error;
        int nberror,maxerror;
        struct s_debug_symbol *symbol;
        int nbsymbol,maxsymbol;
};

/*******************************************
              P O K E R               
*******************************************/
enum e_poker {
E_POKER_XOR8=0,
E_POKER_SUM8=1,
E_POKER_END
};

struct s_poker {
	enum e_poker method;
	int istart,iend;
	int outputadr;
	int ibank;
};

/*******************************************
        G L O B A L     S T R U C T
*******************************************/
struct s_assenv {
	/* current memory */
	int maxptr;
	/* CPR memory */
	unsigned char **mem;
	int iwnamebank[BANK_MAX_NUMBER];
	int nbbank,maxbank;
	int forcetape,forcezx,forcecpr,forceROM,bankmode,activebank,amsdos,forcesnapshot,packedbank,extendedCPR,xpr;
	struct s_snapshot snapshot;
	struct s_zxsnapshot zxsnapshot;
	int bankset[BANK_MAX_NUMBER>>2];   /* 64K selected flag */
	int bankused[BANK_MAX_NUMBER]; /* 16K selected flag */
	int bankgate[BANK_MAX_NUMBER+1];
	int setgate[BANK_MAX_NUMBER+1];
	int rundefined;
	/* parsing */
	struct s_wordlist *wl;
	int nbword;
	int idx,stage;
	char *label_filename;
	int label_line;
	char **filename;
	int ifile,maxfile;
	int nberr,flux;
	int fastmatch[256];
	unsigned char charset[256];
	int maxerr,extended_error,nowarning,erronwarn;
	/* ORG tracking */
	int codeadr,outputadr,nocode;
	int codeadrbackup,outputadrbackup;
	int minadr,maxadr;
	struct s_orgzone *orgzone;
	int io,mo;
	/* Struct */
	struct s_rasmstruct *rasmstruct;
	int irasmstruct,mrasmstruct;
	int getstruct;
	int backup_outputadr,backup_codeadr;
	char *backup_filename;
	int backup_line;
	struct s_rasmstruct *rasmstructalias;
	int irasmstructalias,mrasmstructalias;
	/* expressions */
	struct s_expression *expression;
	int ie,me;
	int maxam,as80,dams,pasmo;
	float rough;
	struct s_compute_core_data *computectx,ctx1,ctx2;
	struct s_crcstring_tree stringtree;
	/* label */
	struct s_label *label;
	int il,ml;
	struct s_crclabel_tree labeltree; /* fast label access */
	char *module;
	int modulen;
	char module_separator[2];
	struct s_breakpoint *breakpoint;
	int ibreakpoint,maxbreakpoint;
	char *lastgloballabel;
	//char *lastsuperglobal;
	int lastgloballabellen, lastglobalalloc;
	char **globalstack; /* retrieve back global from previous scope */
	int igs,mgs;
	/* repeat */
	struct s_repeat *repeat;
	int ir,mr;
	/* while/wend */
	struct s_whilewend *whilewend;
	int iw,mw;
	/* if/then/else */
	//int *ifthen;
	struct s_ifthen *ifthen;
	int ii,mi;
	/* switch/case */
	struct s_switchcase *switchcase;
	int isw,msw;
	/* expression dictionnary */
	struct s_expr_dico *dico;
	int idic,mdic;
	struct s_crcdico_tree dicotree; /* fast dico access */
	struct s_crcused_tree usedtree; /* fast used access */
	/* ticker */
	struct s_ticker *ticker;
	int iticker,mticker;
	long tick,nop;
	/* crunch section flag */
	struct s_lz_section *lzsection;
	int ilz,mlz;
	int lz,curlz;
	/* poker */
	struct s_poker *poker;
	int nbpoker,maxpoker;
	/* macro */
	struct s_macro *macro;
	int imacro,mmacro;
	int macrovoid;
	/* labels locaux */
	int repeatcounter,whilecounter,macrocounter;
	struct s_macro_position *macropos;
	int imacropos,mmacropos;
	/* alias */
	struct s_alias *alias;
	int ialias,malias;
	/* hexbin */
	struct s_rasm_thread **rasm_thread;
	int irt,mrt;
	struct s_hexbin *hexbin;
	int ih,mh;
	char **includepath;
	int ipath,mpath;
	/* automates */
	char AutomateExpressionValidCharExtended[256];
	char AutomateExpressionValidCharFirst[256];
	char AutomateExpressionValidChar[256];
	char AutomateExpressionDecision[256];
	char AutomateValidLabelFirst[256];
	char AutomateValidLabel[256];
	char AutomateDigit[256];
	char AutomateHexa[256];
	struct s_compute_element AutomateElement[256];
	unsigned char psgtab[256];
	unsigned char psgfine[256];
	int noampersand;
	/* output */
	char *outputfilename;
	int export_sym,export_local,export_multisym;
	int export_var,export_equ;
	int export_sna,export_snabrk;
	int export_brk,export_tape;
	int autorise_export;
	char *flexible_export;
	char *breakpoint_name;
	char *symbol_name;
	char *binary_name;
	char *cartridge_name;
	char *snapshot_name;
	char *tape_name;
	char *rom_name;
	struct s_save *save;
	int nbsave,maxsave;
	int current_run_idx;
	struct s_edsk_wrapper *edsk_wrapper;
	int nbedskwrapper,maxedskwrapper;
	int edskoverwrite;
	int checkmode,dependencies;
	int stop;
	int warn_unused;
	int display_stats;
	/* debug */
	struct s_rasm_info debug;
	struct s_rasm_info **retdebug;
	int debug_total_len;
};

/*************************************
         D I R E C T I V E S
*************************************/
struct s_asm_keyword {
	char *mnemo;
	int crc;
	void (*makemnemo)(struct s_assenv *ae);
};

struct s_math_keyword math_keyword[]={
{"SIN",0,E_COMPUTE_OPERATION_SIN},
{"COS",0,E_COMPUTE_OPERATION_COS},
{"INT",0,E_COMPUTE_OPERATION_INT},
{"ABS",0,E_COMPUTE_OPERATION_ABS},
{"LN",0,E_COMPUTE_OPERATION_LN},
{"LOG10",0,E_COMPUTE_OPERATION_LOG10},
{"SQRT",0,E_COMPUTE_OPERATION_SQRT},
{"FLOOR",0,E_COMPUTE_OPERATION_FLOOR},
{"ASIN",0,E_COMPUTE_OPERATION_ASIN},
{"ACOS",0,E_COMPUTE_OPERATION_ACOS},
{"ATAN",0,E_COMPUTE_OPERATION_ATAN},
{"EXP",0,E_COMPUTE_OPERATION_EXP},
{"LO",0,E_COMPUTE_OPERATION_LOW},
{"HI",0,E_COMPUTE_OPERATION_HIGH},
{"PSGVALUE",0,E_COMPUTE_OPERATION_PSG},
{"RND",0,E_COMPUTE_OPERATION_RND},
{"FRAC",0,E_COMPUTE_OPERATION_FRAC},
{"CEIL",0,E_COMPUTE_OPERATION_CEIL},
{"GETR",0,E_COMPUTE_OPERATION_GET_R},
{"GETV",0,E_COMPUTE_OPERATION_GET_V},
{"GETG",0,E_COMPUTE_OPERATION_GET_V},
{"GETB",0,E_COMPUTE_OPERATION_GET_B},
{"SETR",0,E_COMPUTE_OPERATION_SET_R},
{"SETV",0,E_COMPUTE_OPERATION_SET_V},
{"SETG",0,E_COMPUTE_OPERATION_SET_V},
{"SETB",0,E_COMPUTE_OPERATION_SET_B},
{"SOFT2HARD_INK",0,E_COMPUTE_OPERATION_SOFT2HARD},
{"S2H_INK",0,E_COMPUTE_OPERATION_SOFT2HARD},
{"HARD2SOFT_INK",0,E_COMPUTE_OPERATION_HARD2SOFT},
{"H2S_INK",0,E_COMPUTE_OPERATION_HARD2SOFT},
{"GETNOP",0,E_COMPUTE_OPERATION_GETNOP},
{"GETTICK",0,E_COMPUTE_OPERATION_GETTICK},
{"DURATION",0,E_COMPUTE_OPERATION_DURATION},
{"",0,-1}
};

#define CRC_SWITCH    0x01AEDE4A
#define CRC_CASE      0x0826B794
#define CRC_DEFAULT   0x9A0DAC7D
#define CRC_BREAK     0xCD364DDD
#define CRC_ENDSWITCH 0x18E9FB21

#define CRC_ELSEIF 0xE175E230
#define CRC_ELSE   0x3FF177A1
#define CRC_ENDIF  0xCD5265DE
#define CRC_IF     0x4BD52507
#define CRC_IFDEF  0x4CB29DD6
#define CRC_UNDEF  0xCCD2FDEA
#define CRC_IFNDEF 0xD9AD0824
#define CRC_IFNOT  0x4CCAC9F8
#define CRC_WHILE  0xBC268FF1
#define CRC_UNTIL  0xCC12A604
#define CRC_MEND   0xFFFD899C
#define CRC_ENDM   0x3FF9559C
#define CRC_MACRO  0x64AA85EA
#define CRC_IFUSED 0x91752638
#define CRC_IFNUSED 0x1B39A886

#define CRC_SIN 0xE1B71962
#define CRC_COS 0xE077C55D

#define CRC_0    0x7A98A6A8
#define CRC_1    0x7A98A6A9
#define CRC_2    0x7A98A6AA


#define CRC_NC   0x4BD52B09
#define CRC_Z    0x7A98A6D2
#define CRC_NZ   0x4BD52B20
#define CRC_P    0x7A98A6C8
#define CRC_PO   0x4BD53717
#define CRC_PE   0x4BD5370D
#define CRC_M    0x7A98A6C5

/* cut registers */
#define CRC_HL_LOW	0xF9FDE22C
#define CRC_HL_HIGH	0x2261E25A
#define CRC_DE_LOW	0x3A3CE221
#define CRC_DE_HIGH	0x23D0E04F
#define CRC_BC_LOW	0xFDFF1E1D
#define CRC_BC_HIGH	0x222BE44B
#define CRC_IX_LOW	0xB9FD0439
#define CRC_IX_HIGH	0xA3FD0667
#define CRC_IY_LOW	0xD9ED6C3A
#define CRC_IY_HIGH	0x23DD5068
#define CRC_AF_LOW	0xDDCF141F
#define CRC_AF_HIGH	0x223FEA4D

/* 8 bits registers */
#define CRC_F    0x7A98A6BE
#define CRC_I    0x7A98A6C1
#define CRC_R    0x7A98A6CA
#define CRC_A    0x7A98A6B9
#define CRC_B    0x7A98A6BA
#define CRC_C    0x7A98A6BB
#define CRC_D    0x7A98A6BC
#define CRC_E    0x7A98A6BD
#define CRC_H    0x7A98A6C0
#define CRC_L    0x7A98A6C4
/* dual naming */
#define CRC_XH   0x4BD50718
#define CRC_XL   0x4BD5071C
#define CRC_YH   0x4BD50519
#define CRC_YL   0x4BD5051D
#define CRC_HX   0x4BD52718
#define CRC_LX   0x4BD52F1C
#define CRC_HY   0x4BD52719
#define CRC_LY   0x4BD52F1D
#define CRC_IXL  0xE19F1765
#define CRC_IXH  0xE19F1761
#define CRC_IYL  0xE19F1166
#define CRC_IYH  0xE19F1162

/* 16 bits registers */
#define CRC_BC   0x4BD5D2FD
#define CRC_DE   0x4BD5DF01
#define CRC_HL   0x4BD5270C
#define CRC_IX   0x4BD52519
#define CRC_IY   0x4BD5251A
#define CRC_SP   0x4BD5311B
#define CRC_AF   0x4BD5D4FF
/* memory convention */
#define CRC_MHL  0xD0765F5D
#define CRC_MDE  0xD0467D52
#define CRC_MBC  0xD05E694E
#define CRC_MIX  0xD072B76A
#define CRC_MIY  0xD072B16B
#define CRC_MSP  0xD01A876C
#define CRC_MC   0xE018210C
/* struct parsing */
#define CRC_DEFB	0x37D15389
#define CRC_DB		0x4BD5DEFE
#define CRC_DEFW	0x37D1539E
#define CRC_DW		0x4BD5DF13
#define CRC_DEFI	0x37D15390
#define CRC_DEFS	0x37D1539A
#define CRC_DS		0x4BD5DF0F
#define CRC_DEFR	0x37D15399
#define CRC_DR		0x4BD5DF0E
#define CRC_DEFF	0x37D1538D
#define CRC_DF	        0x4BD5DF02

/* struct declaration use special instructions for defines */
int ICRC_DEFB,ICRC_DEFW,ICRC_DEFI,ICRC_DEFR,ICRC_DEFF,ICRC_DF,ICRC_DEFS,ICRC_DB,ICRC_DW,ICRC_DR,ICRC_DS;
/* need to pre-declare var */
extern struct s_asm_keyword instruction[];

/*
# base=16
% base=2
0-9 base=10
A-Z variable ou fonction (cos, sin, tan, sqr, pow, mod, and, xor, mod, ...)
+*-/&^m| operateur
*/

#define AutomateExpressionValidCharExtendedDefinition "0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZ_{}@+-*/~^$#%<=>|&" /* § */
#define AutomateExpressionValidCharFirstDefinition "#%0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZ_@${"
#define AutomateExpressionValidCharDefinition "0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZ_{}@$"
#define AutomateValidLabelFirstDefinition ".ABCDEFGHIJKLMNOPQRSTUVWXYZ_@"
#define AutomateValidLabelDefinition "0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZ_@{}"
#define AutomateDigitDefinition ".0123456789"
#define AutomateHexaDefinition "0123456789ABCDEF"

#ifndef NO_3RD_PARTIES
unsigned char *LZ4_crunch(unsigned char *data, int zelen, int *retlen){
	unsigned char *lzdest=NULL;
	lzdest=MemMalloc(65536);
	*retlen=LZ4_compress_HC((char*)data,(char*)lzdest,zelen,65536,9);
	return lzdest;
}
#ifndef NOAPULTRA
size_t apultra_compress(const unsigned char *pInputData, unsigned char *pOutBuffer, size_t nInputSize, size_t nMaxOutBufferSize,
      const unsigned int nFlags, size_t nMaxWindowSize, size_t nDictionarySize, void(*progress)(long long nOriginalSize, long long nCompressedSize), void *pStats);
size_t apultra_get_max_compressed_size(size_t nInputSize);

int do_apultra(unsigned char *datain, int lenin, unsigned char **dataout, int *lenout) {
   size_t nCompressedSize = 0L, nMaxCompressedSize;
   int nFlags = 0;
   //apultra_stats stats;
   unsigned char *pCompressedData;

   /* Allocate max compressed size */

   nMaxCompressedSize = apultra_get_max_compressed_size(lenin);
   pCompressedData = (unsigned char*)MemMalloc(nMaxCompressedSize);
   memset(pCompressedData, 0, nMaxCompressedSize);

   nCompressedSize = apultra_compress(datain, pCompressedData, lenin, nMaxCompressedSize, nFlags, 65536, 0 /* dico */, NULL /*compression_progress*/, NULL /*&stats*/);

   if (nCompressedSize == -1) {
      fprintf(stderr, "APULTRA compression error\n");
      *lenout=0;
      *dataout=NULL;
      return 100;
   }

   *lenout=nCompressedSize;
   *dataout=pCompressedData;
   return 0;
}
int APULTRA_crunch(unsigned char *data, int len, unsigned char **dataout, int *lenout) {
   return do_apultra(data, len, dataout, lenout);
}


size_t lzsa_compress_inmem(unsigned char *pInputData, unsigned char *pOutBuffer, size_t nInputSize, size_t nMaxOutBufferSize,
                             const unsigned int nFlags, const int nMinMatchSize, const int nFormatVersion);

int LZSA_crunch(unsigned char *datain, int lenin, unsigned char **dataout, int *lenout, int version, int matchsize) {
   size_t nCompressedSize = 0L, nMaxCompressedSize;
   int nFlags = 0;
   unsigned char *pCompressedData;

pCompressedData=(unsigned char *)MemMalloc(65536);
nMaxCompressedSize=65536;

/* RAW */
nFlags=1<<1; // nFlags=LZSA_FLAG_RAW_BLOCK;
/* par défaut du LZSA1-Fast */
if (version<1 || version>2) {
	version=1;
}
if (matchsize<2 || matchsize>5) {
	switch (version) {
		case 1:matchsize=5;break;
		case 2:matchsize=2;break;
		default:break;
	}
}

nCompressedSize=lzsa_compress_inmem(datain, pCompressedData, lenin, nMaxCompressedSize, nFlags, matchsize, version);

   if (nCompressedSize == -1) {
      fprintf(stderr, "LZSA compression error\n");
      *lenout=0;
      *dataout=NULL;
      return 100;
   }

   *lenout=nCompressedSize;
   *dataout=pCompressedData;
   return 0;
}

#endif
#endif

unsigned char *LZ48_encode_legacy(unsigned char *data, int length, int *retlength);
#define LZ48_crunch LZ48_encode_legacy
unsigned char *LZ49_encode_legacy(unsigned char *data, int length, int *retlength);
#define LZ49_crunch LZ49_encode_legacy


/*
 * optimised reading of text file in one shot
 */
unsigned char *_internal_readbinaryfile(char *filename, int *filelength)
{
        #undef FUNC
        #define FUNC "_internal_readbinaryfile"

        unsigned char *binary_data=NULL;

        *filelength=FileGetSize(filename);
        binary_data=MemMalloc((*filelength)+1);
        /* we try to read one byte more to close the file just after the read func */
        if (FileReadBinary(filename,(char*)binary_data,(*filelength)+1)!=*filelength) {
                logerr("Cannot fully read %s",filename);
                exit(INTERNAL_ERROR);
        }
        return binary_data;
}
char **_internal_readtextfile(char *filename, char replacechar)
{
        #undef FUNC
        #define FUNC "_internal_readtextfile"

        char **lines_buffer=NULL;
        unsigned char *bigbuffer;
        int nb_lines=0,max_lines=0,i=0,e=0;
        int file_size;

        bigbuffer=_internal_readbinaryfile(filename,&file_size);

        while (i<file_size) {
                while (e<file_size && bigbuffer[e]!=0x0A) {
                        /* Windows de meeeeeeeerrrdde... */
                        if (bigbuffer[e]==0x0D) bigbuffer[e]=replacechar;
                        e++;
                }
                if (e<file_size) e++;
                if (nb_lines>=max_lines) {
                        max_lines=max_lines*2+10;
                        lines_buffer=MemRealloc(lines_buffer,(max_lines+1)*sizeof(char **));
                }
                lines_buffer[nb_lines]=MemMalloc(e-i+1);
                memcpy(lines_buffer[nb_lines],bigbuffer+i,e-i);
                lines_buffer[nb_lines][e-i]=0;
                if (0)
                {
                        int yy;
                        for (yy=0;lines_buffer[nb_lines][yy];yy++) {
                                if (lines_buffer[nb_lines][yy]>31) printf("%c",lines_buffer[nb_lines][yy]); else printf("(0x%X)",lines_buffer[nb_lines][yy]);
                        }
                        printf("\n");
                }
                nb_lines++;
                i=e;
        }
        if (!max_lines) {
                lines_buffer=MemMalloc(sizeof(char**));
                lines_buffer[0]=NULL;
        } else {
                lines_buffer[nb_lines]=NULL;
        }
        MemFree(bigbuffer);
        return lines_buffer;
}

#define FileReadLines(filename) _internal_readtextfile(filename,':')
#define FileReadLinesRAW(filename) _internal_readtextfile(filename,0x0D)
#define FileReadContent(filename,filesize) _internal_readbinaryfile(filename,filesize)


/***
	TxtReplace
	
	input:
	in_str:     string where replace will occur
	in_substr:  substring to look for
	out_substr: replace substring
	recurse:    loop until no in_substr is found
	
	note: in_str MUST BE previously mallocated if out_substr is bigger than in_substr
*/
#ifndef RDD
char *TxtReplace(char *in_str, char *in_substr, char *out_substr, int recurse)
{
	#undef FUNC
	#define FUNC "TxtReplace"
	
	char *str_look,*m1,*m2;
	char *out_str;
	int sl,l1,l2,dif,cpt;

	if (in_str==NULL)
		return NULL;
		
	sl=strlen(in_str);
	l1=strlen(in_substr);
	/* empty string, nothing to do except return empty string */
	if (!sl || !l1)
		return in_str;
		
	l2=strlen(out_substr);
	dif=l2-l1;
		
	/* replace string is small or equal in size, we dont realloc */
	if (dif<=0)
	{
		/* we loop while there is a replace to do */
		str_look=strstr(in_str,in_substr);
		while (str_look!=NULL)
		{
			/* we copy the new string if his len is not null */
			if (l2)
				memcpy(str_look,out_substr,l2);
			/* only if len are different */
			if (l1!=l2)
			{
				/* we move the end of the string byte per byte
				   because memory locations overlap. This is
				   faster than memmove */
				m1=str_look+l1;
				m2=str_look+l2;
				while (*m1!=0)
				{
					*m2=*m1;
					m1++;m2++;
				}
				/* we must copy the EOL */
				*m2=*m1;
			}
			/* look for next replace */
			if (!recurse)
				str_look=strstr(str_look+l2,in_substr);
			else
				str_look=strstr(in_str,in_substr);
		}
		out_str=in_str;
	}
	else
	{
		/* we need to count each replace */
		cpt=0;
		str_look=strstr(in_str,in_substr);
		while (str_look!=NULL)
		{
			cpt++;
			str_look=strstr(str_look+l1,in_substr);
		}
		/* is there anything to do? */
		if (cpt)
		{
			/* we realloc to a size that will fit all replaces */
			out_str=MemRealloc(in_str,sl+1+dif*cpt);
			str_look=strstr(out_str,in_substr);
			while (str_look!=NULL && cpt)
			{
				/* as the replace string is bigger we
				   have to move memory first from the end */
				m1=out_str+sl;
				m2=m1+dif;
				sl+=dif;
				while (m1!=str_look+l1-dif)
				{
					*m2=*m1;
					m1--;m2--;
				}
				/* then we copy the replace string (can't be NULL in this case) */
				memcpy(str_look,out_substr,l2);
				
				/* look for next replace */
				if (!recurse)
					str_look=strstr(str_look+l2,in_substr);
				else
					str_look=strstr(in_str,in_substr);
					
				/* to prevent from naughty overlap */
				cpt--;
			}
			if (str_look!=NULL)
			{
				printf("INTERNAL ERROR - overlapping replace string (%s/%s), you can't use this one!\n",in_substr,out_substr);
				exit(ABORT_ERROR);
			}
		}
		else
			out_str=in_str;
	}
	return out_str;
}
#endif

#ifndef min
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif

/* Levenshtein implementation by TheRayTracer https://gist.github.com/TheRayTracer/2644387 */
int _internal_LevenshteinDistance(char *s,  char *t)
{
	int i,j,n,m,*d;
	int im,jn;
	int r;
	
   n=strlen(s)+1;
   m=strlen(t)+1;
   d=malloc(n*m*sizeof(int));
   memset(d, 0, sizeof(int) * n * m);

   for (i = 1, im = 0; i < m; i++, im++)
   {
      for (j = 1, jn = 0; j < n; j++, jn++)
      {
         if (s[jn] == t[im])
         {
            d[(i * n) + j] = d[((i - 1) * n) + (j - 1)];
         }
         else
         {
            d[(i * n) + j] = min(d[(i - 1) * n + j] + 1, /* A deletion. */
                                 min(d[i * n + (j - 1)] + 1, /* An insertion. */
                                     d[(i - 1) * n + (j - 1)] + 1)); /* A substitution. */
         }
      }
   }
   r = d[n * m - 1];
   free(d);
   return r;
}

unsigned int FastRand()
{
        #undef FUNC
        #define FUNC "FastRand"
	static unsigned int zeseed=0x12345678;
        zeseed=214013*zeseed+2531011;
        return (zeseed>>16)&0x7FFF;
}


#ifdef RASM_THREAD
/*
 threads used for crunching
*/
void _internal_ExecuteThreads(struct s_assenv *ae,struct s_rasm_thread *rasm_thread, void *(*fct)(void *))
{
	#undef FUNC
	#define FUNC "_internal_ExecuteThreads"

	pthread_attr_t attr;
	void *status;
	int rc;
	/* launch threads */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setstacksize(&attr,65536);

	if ((rc=pthread_create(&image_threads[i].thread,&attr,fct,(void *)rasm_thread))) {
		rasm_printf(ae,"FATAL ERROR - Cannot create thread!\n");
		exit(INTERNAL_ERROR);
	}
}
void _internal_WaitForThreads(struct s_assenv *ae,struct s_rasm_thread *rasm_thread)
{
	#undef FUNC
	#define FUNC "_internal_WaitForThreads"
	int rc;
	
	if ((rc=pthread_join(rasm_thread->thread,&status))) {
		rasm_printf(ae,"FATAL ERROR - Cannot wait for thread\n");
		exit(INTERNAL_ERROR);
	}
}
void PushCrunchedFile(struct s_assenv *ae, unsigned char *datain, int datalen, int lz)
{
	#undef FUNC
	#define FUNC "PushCrunchedFile"
	
	struct s_rasm_thread *rasm_thread;
	
	rasm_thread=MemMalloc(sizeof(struct s_rasm_thread));
	memset(rasm_thread,0,sizeof(struct s_rasm_thread));
	rasm_thread->datain=datain;
	rasm_thread->datalen=datalen;
	rasm_thread->lz=lz;
	_internal_ExecuteThreads(ae,rasm_thread, void *(*fct)(void *));
	ObjectArrayAddDynamicValueConcat((void**)&ae->rasm_thread,&ae->irt,&ae->mrt,&rasm_thread,sizeof(struct s_rasm_thread *));
}
void PopAllCrunchedFiles(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "PopAllCrunchedFiles"
	
	int i;
	for (i=0;i<ae->irt;i++) {
		_internal_WaitForThreads(ae,ae->rasm_thread[i]);
	}
}
#endif

void MaxError(struct s_assenv *ae);

void rasm_printf(struct s_assenv *ae, ...) {
	#undef FUNC
	#define FUNC "(internal) rasm_printf"
	
	char *format;
	va_list argptr;

	if (!ae->flux && !ae->dependencies) {
		va_start(argptr,ae);
		format=va_arg(argptr,char *);
		vfprintf(stdout,format,argptr);	
		va_end(argptr);
		fprintf(stdout,KNORMAL);
	}
}
/***
	build the string of current line for error messages
*/
char *rasm_getline(struct s_assenv *ae, int offset) {
	#undef FUNC
	#define FUNC "rasm_getline"
	
	static char myline[40]={0};
	int idx=0,icopy,first=1;

	while (!ae->wl[ae->idx+offset].t && idx<32) {
		for (icopy=0;idx<32 && ae->wl[ae->idx+offset].w[icopy];icopy++) {
			myline[idx++]=ae->wl[ae->idx+offset].w[icopy];
		}
		if (!first) myline[idx++]=','; else first=0;
		offset++;
	}
	if (idx>=32) {
		strcpy(myline+29,"...");
	} else {
		myline[idx++]=0;
	}
	
	return myline;
}

char *SimplifyPath(char *filename) {
	#undef FUNC
	#define FUNC "SimplifyPath"

	return filename;
#if 0	
	char *pos,*repos;
	int i,len;

	char *rpath;

	rpath=realpath(filename,NULL);
	if (!rpath) {
		printf("rpath error!\n");
		switch (errno) {
			case EACCES:printf("read permission failure\n");break;
			case EINVAL:printf("wrong argument\n");break;
			case EIO:printf("I/O error\n");break;
			case ELOOP:printf("too many symbolic links\n");break;
			case ENAMETOOLONG:printf("names too long\n");break;
			case ENOENT:printf("names does not exists\n");break;
			case ENOMEM:printf("out of memory\n");break;
			case ENOTDIR:printf("a component of the path is not a directory\n");break;
			default:printf("unknown error\n");break;
		}
		exit(1);
	}
	if (strlen(rpath)<strlen(filename)) {
		strcpy(filename,rpath);
	}
	free(rpath);
	return filename;
	
#ifdef OS_WIN
	while ((pos=strstr(filename,"\\..\\"))!=NULL) {
		repos=pos-1;
		/* sequence found, looking back for '\' */
		while (repos>=filename) {
			if (*repos=='\\') {
				break;
			}
			repos--;
		}
		repos++;
		if (repos>=filename && repos!=pos) {
			len=strlen(pos)-4+1;
			pos+=4;
			for (i=0;i<len;i++) {
				*repos=*pos;
				repos++;
				pos++;
			}
		}
		if (strncmp(filename,".\\..\\",5)==0) {
			repos=filename;
			pos=repos+2;
			for (;*repos;pos++,repos++) {
				*repos=*pos;
			}
			*repos=0;
		}
	}
#else
printf("*************\nfilename=[%s]\n",filename);
	while ((pos=strstr(filename,"/../"))!=NULL) {
		repos=pos-1;
		while (repos>=filename) {
			if (*repos=='/') {
				break;
			}
			repos--;
		}
		repos++;
		if (repos>=filename && repos!=pos) {
			len=strlen(pos)-4+1;
			pos+=4;
			for (i=0;i<len;i++) {
				*repos=*pos;
				repos++;
				pos++;
			}
		}
printf("filename=[%s]\n",filename);
		if (strncmp(filename,"./../",5)==0) {
			repos=filename;
			pos=repos+2;
			for (;*repos;pos++,repos++) {
				*repos=*pos;
			}
			*repos=0;
		}
printf("filename=[%s]\n",filename);
	}
#endif
	return NULL;
#endif

}

char *rasm_GetPath(char *filename) {
	#undef FUNC
	#define FUNC "rasm_GetPath"

	static char curpath[PATH_MAX];
	int zelen,idx;

	zelen=strlen(filename);

#ifdef OS_WIN
	#define CURRENT_DIR ".\\"

	TxtReplace(filename,"/","\\",1);
	idx=zelen-1;
	while (idx>=0 && filename[idx]!='\\') idx--;
	if (idx<0) {
		/* pas de chemin */
		strcpy(curpath,".\\");
	} else {
		/* chemin trouve */
		strcpy(curpath,filename);
		curpath[idx+1]=0;
	}
#else
#ifdef __MORPHOS__
	#define CURRENT_DIR ""
#else
	#define CURRENT_DIR "./"
#endif
	idx=zelen-1;
	while (idx>=0 && filename[idx]!='/') idx--;
	if (idx<0) {
		/* pas de chemin */
		strcpy(curpath,CURRENT_DIR);
	} else {
		/* chemin trouve */
		strcpy(curpath,filename);
		curpath[idx+1]=0;
	}
#endif

	return curpath;
}
char *MergePath(struct s_assenv *ae,char *dadfilename, char *filename) {
	#undef FUNC
	#define FUNC "MergePath"

	static char curpath[PATH_MAX];


#ifdef OS_WIN
	TxtReplace(filename,"/","\\",1);

	if (filename[0] && filename[1]==':' && filename[2]=='\\') {
		/* chemin absolu */
		strcpy(curpath,filename);
	} else if (filename[0] && filename[1]==':') {
		rasm_printf(ae,KERROR"unsupported path style [%s]\n",filename);
		exit(-111);
	} else {
		if (filename[0]=='.' && filename[1]=='\\') {
			strcpy(curpath,rasm_GetPath(dadfilename));
			strcat(curpath,filename+2);
		} else {
			strcpy(curpath,rasm_GetPath(dadfilename));
			strcat(curpath,filename);
		}
	}
#else
	if (filename[0]=='/') {
		/* chemin absolu */
		strcpy(curpath,filename);
	} else if (filename[0]=='.' && filename[1]=='/') {
		strcpy(curpath,rasm_GetPath(dadfilename));
		strcat(curpath,filename+2);
	} else {
		strcpy(curpath,rasm_GetPath(dadfilename));
		strcat(curpath,filename);
	}
#endif

	return curpath;
}


void InitAutomate(char *autotab, const unsigned char *def)
{
	#undef FUNC
	#define FUNC "InitAutomate"

	int i;

	memset(autotab,0,256);
	for (i=0;def[i];i++) {
		autotab[(unsigned int)def[i]]=1;
	}
}
void StateMachineResizeBuffer(char **ABuf, int idx, int *ASize) {
	#undef FUNC
	#define FUNC "StateMachineResizeBuffer"

	if (idx>=*ASize) {
		if (*ASize<16384) {
			*ASize=(*ASize)*2;
		} else {
			*ASize=(*ASize)+16384;
		}
		*ABuf=MemRealloc(*ABuf,(*ASize)+2);
	}
}

int GetCRC(char *label)
{
	#undef FUNC
	#define FUNC "GetCRC"
	int crc=0x12345678;
	int i=0;

	while (label[i]!=0) {
		crc=(crc<<9)^(crc+label[i++]);
	}
	return crc;
}

int IsDirective(char *expr);

int IsRegister(char *zeexpression)
{
	#undef FUNC
	#define FUNC "IsRegister"

	switch (GetCRC(zeexpression)) {
		case CRC_F:if (strcmp(zeexpression,"F")==0) return 1; else return 0;
		case CRC_I:if (strcmp(zeexpression,"I")==0) return 1; else return 0;
		case CRC_R:if (strcmp(zeexpression,"R")==0) return 1; else return 0;
		case CRC_A:if (strcmp(zeexpression,"A")==0) return 1; else return 0;
		case CRC_B:if (strcmp(zeexpression,"B")==0) return 1; else return 0;
		case CRC_C:if (strcmp(zeexpression,"C")==0) return 1; else return 0;
		case CRC_D:if (strcmp(zeexpression,"D")==0) return 1; else return 0;
		case CRC_E:if (strcmp(zeexpression,"E")==0) return 1; else return 0;
		case CRC_H:if (strcmp(zeexpression,"H")==0) return 1; else return 0;
		case CRC_L:if (strcmp(zeexpression,"L")==0) return 1; else return 0;
		case CRC_BC:if (strcmp(zeexpression,"BC")==0) return 1; else return 0;
		case CRC_DE:if (strcmp(zeexpression,"DE")==0) return 1; else return 0;
		case CRC_HL:if (strcmp(zeexpression,"HL")==0) return 1; else return 0;
		case CRC_IX:if (strcmp(zeexpression,"IX")==0) return 1; else return 0;
		case CRC_IY:if (strcmp(zeexpression,"IY")==0) return 1; else return 0;
		case CRC_SP:if (strcmp(zeexpression,"SP")==0) return 1; else return 0;
		case CRC_AF:if (strcmp(zeexpression,"AF")==0) return 1; else return 0;
		case CRC_XH:if (strcmp(zeexpression,"XH")==0) return 1; else return 0;
		case CRC_XL:if (strcmp(zeexpression,"XL")==0) return 1; else return 0;
		case CRC_YH:if (strcmp(zeexpression,"YH")==0) return 1; else return 0;
		case CRC_YL:if (strcmp(zeexpression,"YL")==0) return 1; else return 0;
		case CRC_HX:if (strcmp(zeexpression,"HX")==0) return 1; else return 0;
		case CRC_LX:if (strcmp(zeexpression,"LX")==0) return 1; else return 0;
		case CRC_HY:if (strcmp(zeexpression,"HY")==0) return 1; else return 0;
		case CRC_LY:if (strcmp(zeexpression,"LY")==0) return 1; else return 0;
		case CRC_IXL:if (strcmp(zeexpression,"IXL")==0) return 1; else return 0;
		case CRC_IXH:if (strcmp(zeexpression,"IXH")==0) return 1; else return 0;
		case CRC_IYL:if (strcmp(zeexpression,"IYL")==0) return 1; else return 0;
		case CRC_IYH:if (strcmp(zeexpression,"IYH")==0) return 1; else return 0;
		default:break;
	}
	return 0;
}

int StringIsMem(char *w)
{
	#undef FUNC
	#define FUNC "StringIsMem"

	int p=1,idx=1;

	if (w[0]=='(') {
		while (w[idx]) {
			switch (w[idx]) {
				case '\\':if (w[idx+1]) idx++;
					break;
				case '\'':if (w[idx+1] && w[idx+1]!='\\') idx++;
					break;
				case '(':p++;break;
				case ')':p--;
					/* si on sort de la première parenthèse */
					if (!p && w[idx+1]) return 0;
					break;
				default:break;
			}
			idx++;
		}
		/* si on ne termine pas par une parenthèse */
		if (w[idx-1]!=')') return 0;
	} else {
		return 0;
	}
	return 1;

}


int StringIsQuote(char *w)
{
	#undef FUNC
	#define FUNC "StringIsQuote"

	int i,tquote,lens;

	if (w[0]=='\'' || w[0]=='"') {
		tquote=w[0];
		lens=strlen(w);
		
		/* est-ce bien une chaine et uniquement une chaine? */
		i=1;
		while (w[i] && w[i]!=tquote) {
			if (w[i]=='\\') i++;
			i++;
		}
		if (i==lens-1) {
			return tquote;
		}
	}
	return 0;
}
char *StringLooksLikeDicoRecurse(struct s_crcdico_tree *lt, int *score, char *str)
{
	#undef FUNC
	#define FUNC "StringLooksLikeDicoRecurse"

	char *retstr=NULL,*tmpstr;
	int i,curs;

	for (i=0;i<256;i++) {
		if (lt->radix[i]) {
			tmpstr=StringLooksLikeDicoRecurse(lt->radix[i],score,str);
			if (tmpstr!=NULL) retstr=tmpstr;
		}
	}
	if (lt->mdico) {
		for (i=0;i<lt->ndico;i++) {
			if (strlen(lt->dico[i].name)>4) {
				curs=_internal_LevenshteinDistance(str,lt->dico[i].name);
				if (curs<*score) {
					*score=curs;
					retstr=lt->dico[i].name;
				}
			}
		}
	}
	return retstr;
}
char *StringLooksLikeDico(struct s_assenv *ae, int *score, char *str)
{
	#undef FUNC
	#define FUNC "StringLooksLikeDico"

	char *retstr=NULL,*tmpstr;
	int i;

	for (i=0;i<256;i++) {
		if (ae->dicotree.radix[i]) {
			tmpstr=StringLooksLikeDicoRecurse(ae->dicotree.radix[i],score,str);
			if (tmpstr!=NULL) retstr=tmpstr;
		}
	}
	return retstr;
}
char *StringLooksLikeMacro(struct s_assenv *ae, char *str, int *retscore)
{
	#undef FUNC
	#define FUNC "StringLooksLikeMacro"
	
	char *ret=NULL;
	int i,curs,score=3;
	/* search in macros */
	for (i=0;i<ae->imacro;i++) {
		curs=_internal_LevenshteinDistance(ae->macro[i].mnemo,str);
		if (curs<score) {
			score=curs;
			ret=ae->macro[i].mnemo;
		}
	}
	if (retscore) *retscore=score;
	return ret;
}	

char *StringLooksLike(struct s_assenv *ae, char *str)
{
	#undef FUNC
	#define FUNC "StringLooksLike"

	char *ret=NULL,*tmpret;
	int i,curs,score=4;

	/* search in variables */
	ret=StringLooksLikeDico(ae,&score,str);

	/* search in labels */
	for (i=0;i<ae->il;i++) {
		if (!ae->label[i].name && strlen(ae->wl[ae->label[i].iw].w)>4) {
			curs=_internal_LevenshteinDistance(ae->wl[ae->label[i].iw].w,str);
			if (curs<score) {
				score=curs;
				ret=ae->wl[ae->label[i].iw].w;
			}
		}
	}
	
	/* search in alias */
	for (i=0;i<ae->ialias;i++) {
		if (strlen(ae->alias[i].alias)>4) {
			curs=_internal_LevenshteinDistance(ae->alias[i].alias,str);
			if (curs<score) {
				score=curs;
				ret=ae->alias[i].alias;
			}
		}
	}
	
	tmpret=StringLooksLikeMacro(ae,str,&curs);
	if (curs<score) {
		score=curs;
		ret=tmpret;
	}
	return ret;
}

int RoundComputeExpression(struct s_assenv *ae,char *expr, int ptr, int didx, int expression_expected);
int RoundComputeExpressionCore(struct s_assenv *ae,char *zeexpression,int ptr,int didx);
double ComputeExpressionCore(struct s_assenv *ae,char *original_zeexpression,int ptr, int didx);
char *GetExpFile(struct s_assenv *ae,int didx);
void __STOP(struct s_assenv *ae);


void MakeError(struct s_assenv *ae, char *filename, int line, char *format, ...)
{
	#undef FUNC
	#define FUNC "MakeError"

	va_list argptr;

	MaxError(ae);
	if (ae->flux) {
		/* in embedded Rasm all errors are stored in a debug struct */
		struct s_debug_error curerror;
		char toosmalltotakeitall[2]={0};
		int myalloc;
		char *errstr;
		
		va_start(argptr,format);
		myalloc=vsnprintf(toosmalltotakeitall,1,format,argptr);
		va_end(argptr);

		#if defined(_MSC_VER) && _MSC_VER < 1900
		/* visual studio before 2015 does not fully support C99 */
		if (myalloc<1 && strlen(format)) {
			va_start(argptr,format);
			myalloc=_vscprintf(format,argptr);
			va_end(argptr);
		}
		#endif
		if (myalloc<1) {
			/* does not crash */
			return;
		}

		va_start(argptr,format);
		errstr=MemMalloc(myalloc+1);
		vsnprintf(errstr,myalloc,format,argptr);
		curerror.msg=errstr;
		curerror.lenmsg=myalloc;
		curerror.line=line;
		if (filename) curerror.filename=TxtStrDupLen(filename,&curerror.lenfilename); else curerror.filename=TxtStrDupLen("<internal>",&curerror.lenfilename);
		ObjectArrayAddDynamicValueConcat((void **)&ae->debug.error,&ae->debug.nberror,&ae->debug.maxerror,&curerror,sizeof(struct s_debug_error));
		va_end(argptr);
	} else {
		fprintf(stdout,KERROR);
		if (filename && line) {
			printf("[%s:%d] ",filename,line);
		} else if (filename) {
			printf("[%s] ",filename);
		}
		va_start(argptr,format);
		vfprintf(stdout,format,argptr);	
		va_end(argptr);
		fprintf(stdout,KNORMAL);
	}
}

/* convert v double value to Microsoft REAL 
 *
 * https://en.wikipedia.org/wiki/Microsoft_Binary_Format
 *
 * exponent:8
 * sign:1
 * mantiss:23
 *
 * */
unsigned char *__internal_MakeRosoftREAL(struct s_assenv *ae, double v, int iexpression)
{
	#undef FUNC
	#define FUNC "__internal_MakeRosoftREAL"
	
	static unsigned char orc[5]={0};
	unsigned char rc[5]={0};
	
	int j,ib,ibb,exp=0;
	unsigned int deci;
	int fracmax=0;
	double frac;
	int mesbits[32];
	int ibit=0;
	unsigned int mask;

	memset(rc,0,sizeof(rc));

	deci=fabs(floor(v));
	frac=fabs(v)-deci;

	if (deci) {
		mask=0x80000000;
		while (!(deci & mask)) mask=mask>>1;
		while (mask) {
			mesbits[ibit]=!!(deci & mask);
#if TRACE_MAKEAMSDOSREAL
printf("%d",mesbits[ibit]);
#endif
			ibit++;
			mask=mask/2;
		}
#if TRACE_MAKEAMSDOSREAL
printf("\nexposant positif: %d\n",ibit);
#endif
		exp=ibit;
#if TRACE_MAKEAMSDOSREAL
printf(".");
#endif
		while (ibit<32 && frac!=0) {
			frac=frac*2.0;
			if (frac>=1.0) {
				mesbits[ibit++]=1;
#if TRACE_MAKEAMSDOSREAL
printf("1");
#endif
				frac-=1.0;
			} else {
				mesbits[ibit++]=0;
#if TRACE_MAKEAMSDOSREAL
printf("0");
#endif
			}
			fracmax++;
		}
	} else {
#if TRACE_MAKEAMSDOSREAL
printf("\nexposant negatif a definir:\n");
printf("x.");
#endif
		
		/* handling zero */
		if (frac==0.0) {
#if TRACE_MAKEAMSDOSREAL
printf("\ncas special ZERO:\n");
#endif
			exp=0;
			ibit=0;
		} else {
			/* looking for first significant bit */
			while (1) {
				frac=frac*2.0;
				if (frac>=1.0) {
					mesbits[ibit++]=1;
#if TRACE_MAKEAMSDOSREAL
printf("1");
#endif
					frac-=1.0;
					break; /* first significant bit found, now looking for limit */
				} else {
#if TRACE_MAKEAMSDOSREAL
printf("o");
#endif
				}
				fracmax++;
				exp--;
			}
			while (ibit<32 && frac!=0) {
				frac=frac*2.0;
				if (frac>=1.0) {
					mesbits[ibit++]=1;
#if TRACE_MAKEAMSDOSREAL
printf("1");
#endif
					frac-=1.0;
				} else {
					mesbits[ibit++]=0;
#if TRACE_MAKEAMSDOSREAL
printf("0");
#endif
				}
				fracmax++;
			}
		}
	}

#if TRACE_MAKEAMSDOSREAL
printf("\n%d bits utilises en mantisse\n",ibit);
#endif
	/* pack bits */
	ib=3;ibb=0x80;
	for (j=0;j<ibit;j++) {
		if (mesbits[j])	rc[ib]|=ibb;
		ibb>>=1;
		if (ibb==0) {
			ibb=0x80;
			ib--;
		}
	}
	/* exponent */
	if (exp==0 && ibit==0) {
		/* special zero */
	} else {
		exp+=128;
		if (exp<0 || exp>255) {
			if (iexpression) MakeError(ae,GetExpFile(ae,iexpression),ae->wl[ae->expression[iexpression].iw].l,"Exponent overflow\n");
			else MakeError(ae,GetExpFile(ae,0),ae->wl[ae->idx].l,"Exponent overflow\n");
			exp=128;
		}
	}
	rc[4]=exp;

	/* Microsoft REAL sign */
	if (v>=0) {
		rc[3]&=0x7F;
	} else {
		rc[3]|=0x80;
	}

#if TRACE_MAKEAMSDOSREAL
	for (j=0;j<5;j++) printf("%02X ",rc[j]);
	printf("\n");
#endif

	/* switch byte order */
	orc[0]=rc[4];
	orc[1]=rc[3];
	orc[2]=rc[2];
	orc[3]=rc[1];
	orc[4]=rc[0];

	return orc;
}


/* convert v double value to Amstrad REAL 
 *
 * http://www.cpcwiki.eu/index.php?title=Technical_information_about_Locomotive_BASIC&mobileaction=toggle_view_desktop#Floating_Point_data_definition
 *
 * exponent:8
 * sign:1
 * mantiss:23
 *
 * */
unsigned char *__internal_MakeAmsdosREAL(struct s_assenv *ae, double v, int iexpression)
{
	#undef FUNC
	#define FUNC "__internal_MakeAmsdosREAL"
	
	static unsigned char rc[5];

	
	int j,ib,ibb,exp=0;
	unsigned int deci;
	int fracmax=0;
	double frac;
	int mesbits[32];
	int ibit=0;
	unsigned int mask;

	memset(rc,0,sizeof(rc));

	deci=fabs(floor(v));
	frac=fabs(v)-deci;

	if (deci) {
		mask=0x80000000;
		while (!(deci & mask)) mask=mask/2;
		while (mask) {
			mesbits[ibit]=!!(deci & mask);
#if TRACE_MAKEAMSDOSREAL
printf("%d",mesbits[ibit]);
#endif
			ibit++;
			mask=mask/2;
		}
#if TRACE_MAKEAMSDOSREAL
printf("\nexposant positif: %d\n",ibit);
#endif
		exp=ibit;
#if TRACE_MAKEAMSDOSREAL
printf(".");
#endif
		while (ibit<32 && frac!=0) {
			frac=frac*2;
			if (frac>=1.0) {
				mesbits[ibit++]=1;
#if TRACE_MAKEAMSDOSREAL
printf("1");
#endif
				frac-=1.0;
			} else {
				mesbits[ibit++]=0;
#if TRACE_MAKEAMSDOSREAL
printf("0");
#endif
			}
			fracmax++;
		}
	} else {
#if TRACE_MAKEAMSDOSREAL
printf("\nexposant negatif a definir:\n");
printf("x.");
#endif
		
		/* handling zero */
		if (frac==0.0) {
			exp=0;
			ibit=0;
		} else {
			/* looking for first significant bit */
			while (1) {
				frac=frac*2;
				if (frac>=1.0) {
					mesbits[ibit++]=1;
#if TRACE_MAKEAMSDOSREAL
printf("1");
#endif
					frac-=1.0;
					break; /* first significant bit found, now looking for limit */
				} else {
#if TRACE_MAKEAMSDOSREAL
printf("o");
#endif
				}
				fracmax++;
				exp--;
			}
			while (ibit<32 && frac!=0) {
				frac=frac*2;
				if (frac>=1.0) {
					mesbits[ibit++]=1;
#if TRACE_MAKEAMSDOSREAL
printf("1");
#endif
					frac-=1.0;
				} else {
					mesbits[ibit++]=0;
#if TRACE_MAKEAMSDOSREAL
printf("0");
#endif
				}
				fracmax++;
			}
		}
	}

#if TRACE_MAKEAMSDOSREAL
printf("\n%d bits utilises en mantisse\n",ibit);
#endif
	/* pack bits */
	ib=3;ibb=0x80;
	for (j=0;j<ibit;j++) {
		if (mesbits[j])	rc[ib]|=ibb;
		ibb/=2;
		if (ibb==0) {
			ibb=0x80;
			ib--;
		}
	}
	/* exponent */
	exp+=128;
	if (exp<0 || exp>255) {
		if (iexpression) MakeError(ae,GetExpFile(ae,iexpression),ae->wl[ae->expression[iexpression].iw].l,"Exponent overflow\n");
		else MakeError(ae,GetExpFile(ae,0),ae->wl[ae->idx].l,"Exponent overflow\n");
		exp=128;
	}
	rc[4]=exp;

	/* REAL sign */
	if (v>=0) {
		rc[3]&=0x7F;
	} else {
		rc[3]|=0x80;
	}

#if TRACE_MAKEAMSDOSREAL
	for (j=0;j<5;j++) printf("%02X ",rc[j]);
	printf("\n");
#endif

	return rc;
}




struct s_label *SearchLabel(struct s_assenv *ae, char *label, int crc);
char *GetExpFile(struct s_assenv *ae,int didx){
	#undef FUNC
	#define FUNC "GetExpFile"
	
	if (ae->label_filename) {
		return ae->label_filename;
	}
	if (didx<0) {
		return ae->filename[ae->wl[-didx].ifile];
	} else if (!didx) {
		return ae->filename[ae->wl[ae->idx].ifile];
	} else if (ae->expression && didx<ae->ie) {
			return ae->filename[ae->wl[ae->expression[didx].iw].ifile];
	} else {
		//return ae->filename[ae->wl[ae->idx].ifile];
		return 0;
	}
}

int GetExpLine(struct s_assenv *ae,int didx){
	#undef FUNC
	#define FUNC "GetExpLine"

	if (ae->label_line) return ae->label_line;

	if (didx<0) {
		return ae->wl[-didx].l;
	} else if (!didx) {
		return ae->wl[ae->idx].l;
	} else if (didx<ae->ie) {
		return ae->wl[ae->expression[didx].iw].l;
	} else return 0;
}

char *GetCurrentFile(struct s_assenv *ae)
{
	return GetExpFile(ae,0);
}


/*******************************************************************************************
			    M E M O R Y       C L E A N U P 
*******************************************************************************************/
void FreeLabelTree(struct s_assenv *ae);
void FreeDicoTree(struct s_assenv *ae);
void FreeUsedTree(struct s_assenv *ae);
void ExpressionFastTranslate(struct s_assenv *ae, char **ptr_expr, int fullreplace);
char *TradExpression(char *zexp);


void _internal_RasmFreeInfoStruct(struct s_rasm_info *debug)
{
	#undef FUNC
	#define FUNC "RasmFreeInfoStruct"

	int i;
	if (debug->maxerror) {
		for (i=0;i<debug->nberror;i++) {
			MemFree(debug->error[i].filename);
			MemFree(debug->error[i].msg);
		}
		MemFree(debug->error);
	}
	if (debug->maxsymbol) {
		for (i=0;i<debug->nbsymbol;i++) {
			MemFree(debug->symbol[i].name);
		}
		MemFree(debug->symbol);
	}
}

void RasmFreeInfoStruct(struct s_rasm_info *debug)
{
	_internal_RasmFreeInfoStruct(debug);
	MemFree(debug);
}

void FreeAssenv(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "FreeAssenv"
	int i,j;

#ifndef RDD
	/* let the system free the memory in command line except when debug/dev */
	#ifndef __MORPHOS__
	/* MorphOS does not like when memory is not freed before exit */
	if (!ae->flux) return;
	#endif
#endif
	/*** debug info ***/	
	if (!ae->retdebug) {
		_internal_RasmFreeInfoStruct(&ae->debug);
	} else {
		/* symbols */
		struct s_debug_symbol debug_symbol={0};

		for (i=0;i<ae->il;i++) {
			/* on exporte tout */
			if (!ae->label[i].name) {
				/* les labels entiers */
				debug_symbol.name=TxtStrDup(ae->wl[ae->label[i].iw].w);
				debug_symbol.v=ae->label[i].ptr;
				ObjectArrayAddDynamicValueConcat((void**)&ae->debug.symbol,&ae->debug.nbsymbol,&ae->debug.maxsymbol,&debug_symbol,sizeof(struct s_debug_symbol));
			} else {
				/* les labels locaux et générés */
				debug_symbol.name=TxtStrDup(ae->label[i].name);
				debug_symbol.v=ae->label[i].ptr;
				ObjectArrayAddDynamicValueConcat((void**)&ae->debug.symbol,&ae->debug.nbsymbol,&ae->debug.maxsymbol,&debug_symbol,sizeof(struct s_debug_symbol));
			}
		}
		for (i=0;i<ae->ialias;i++) {
			if (strcmp(ae->alias[i].alias,"IX") && strcmp(ae->alias[i].alias,"IY")) {
				debug_symbol.name=TxtStrDup(ae->alias[i].alias);
				debug_symbol.v=RoundComputeExpression(ae,ae->alias[i].translation,0,0,0);
				ObjectArrayAddDynamicValueConcat((void**)&ae->debug.symbol,&ae->debug.nbsymbol,&ae->debug.maxsymbol,&debug_symbol,sizeof(struct s_debug_symbol));
			}
		}

		/* export struct */
		*ae->retdebug=MemMalloc(sizeof(struct s_rasm_info));
		memcpy(*ae->retdebug,&ae->debug,sizeof(struct s_rasm_info));
	}
	/*** end debug ***/

	for (i=0;i<ae->nbbank;i++) {
		MemFree(ae->mem[i]);
	}
	MemFree(ae->mem);
	
	/* expression core buffer free */
	ComputeExpressionCore(NULL,NULL,0,0);
	ExpressionFastTranslate(NULL,NULL,0);
	/* free labels, expression, orgzone, repeat, ... */
	if (ae->mo) MemFree(ae->orgzone);
	if (ae->me) {
		for (i=0;i<ae->ie;i++) {
			if (ae->expression[i].reference) MemFree(ae->expression[i].reference);
			if (ae->expression[i].module) MemFree(ae->expression[i].module);
		}
		MemFree(ae->expression);
	}
	if (ae->mh) {
		for (i=0;i<ae->ih;i++) {
			//MemFree(ae->hexbin[i].data);
			MemFree(ae->hexbin[i].filename);
		}
		MemFree(ae->hexbin);
	}
	for (i=0;i<ae->il;i++) {
		if (ae->label[i].name && ae->label[i].iw==-1) MemFree(ae->label[i].name);
	}
	/* structures */
	for (i=0;i<ae->irasmstructalias;i++) {
		MemFree(ae->rasmstructalias[i].name);
	}
	if (ae->mrasmstructalias) MemFree(ae->rasmstructalias);
	
	for (i=0;i<ae->irasmstruct;i++) {
		for (j=0;j<ae->rasmstruct[i].irasmstructfield;j++) {
			MemFree(ae->rasmstruct[i].rasmstructfield[j].fullname);
			MemFree(ae->rasmstruct[i].rasmstructfield[j].name);
			if (ae->rasmstruct[i].rasmstructfield[j].mdata) MemFree(ae->rasmstruct[i].rasmstructfield[j].data);
		}
		if (ae->rasmstruct[i].mrasmstructfield) MemFree(ae->rasmstruct[i].rasmstructfield);
		MemFree(ae->rasmstruct[i].name);
	}
	if (ae->mrasmstruct) MemFree(ae->rasmstruct);
	
	/* other */
	if (ae->maxbreakpoint) MemFree(ae->breakpoint);
	if (ae->ml) MemFree(ae->label);
	if (ae->mr) MemFree(ae->repeat);
	if (ae->mi) MemFree(ae->ifthen);
	if (ae->msw) MemFree(ae->switchcase);
	if (ae->mw) MemFree(ae->whilewend);
	if (ae->modulen || ae->module) {
		MemFree(ae->module);
	}
	/* deprecated
	for (i=0;i<ae->idic;i++) {
		MemFree(ae->dico[i].name);
	}
	if (ae->mdic) MemFree(ae->dico);
	*/
	if (ae->mlz) MemFree(ae->lzsection);

	for (i=0;i<ae->ifile;i++) {
		MemFree(ae->filename[i]);
	}
	MemFree(ae->filename);

	for (i=0;i<ae->imacro;i++) {
		if (ae->macro[i].maxword) MemFree(ae->macro[i].wc);
		for (j=0;j<ae->macro[i].nbparam;j++) MemFree(ae->macro[i].param[j]);
		if (ae->macro[i].nbparam) MemFree(ae->macro[i].param);
	}

	
	if (ae->mmacro) MemFree(ae->macro);

	for (i=0;i<ae->igs;i++) {
		if (ae->globalstack[i]) MemFree(ae->globalstack[i]);
	}
	if (ae->mgs) MemFree(ae->globalstack);
	if (ae->lastglobalalloc) {
		MemFree(ae->lastgloballabel);
		ae->lastglobalalloc=0;
		ae->lastgloballabel=NULL;
	}

	for (i=0;i<ae->ialias;i++) {
		MemFree(ae->alias[i].alias);
		MemFree(ae->alias[i].translation);
	}
	if (ae->malias) MemFree(ae->alias);

	for (i=0;ae->wl[i].t!=2;i++) {
		MemFree(ae->wl[i].w);
	}
	MemFree(ae->wl);

	if (ae->ctx1.varbuffer) {
		MemFree(ae->ctx1.varbuffer);
	}
	if (ae->ctx1.maxtokenstack) {
		MemFree(ae->ctx1.tokenstack);
	}
	if (ae->ctx1.maxoperatorstack) {
		MemFree(ae->ctx1.operatorstack);
	}
	if (ae->ctx2.varbuffer) {
		MemFree(ae->ctx2.varbuffer);
	}
	if (ae->ctx2.maxtokenstack) {
		MemFree(ae->ctx2.tokenstack);
	}
	if (ae->ctx2.maxoperatorstack) {
		MemFree(ae->ctx2.operatorstack);
	}

	for (i=0;i<ae->iticker;i++) {
		MemFree(ae->ticker[i].varname);
	}
	if (ae->mticker) MemFree(ae->ticker);

	MemFree(ae->outputfilename);
	FreeLabelTree(ae);
	FreeDicoTree(ae);
	FreeUsedTree(ae);
	if (ae->mmacropos) MemFree(ae->macropos);
	TradExpression(NULL);
	MemFree(ae);
}



void MaxError(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "MaxError"

	char **source_lines=NULL;
	int zeline;


	/* extended error is useful with generated code we do not want to edit */
	if (ae->extended_error && ae->wl) {
		/* super dupper slow but anyway, there is an error... */
		if (ae->wl[ae->idx].l) {
			source_lines=FileReadLinesRAW(GetCurrentFile(ae));
			zeline=0;
			while (zeline<ae->wl[ae->idx].l-1 && source_lines[zeline]) zeline++;
			if (zeline==ae->wl[ae->idx].l-1 && source_lines[zeline]) {
				rasm_printf(ae,KAYGREEN"-> %s",source_lines[zeline]);
			} else {
				rasm_printf(ae,KERROR"cannot read line %d of file [%s]\n",ae->wl[ae->idx].l,GetCurrentFile(ae));
			}
			FreeArrayDynamicValue(&source_lines);
		}
	}

	ae->nberr++;
	if (ae->nberr==ae->maxerr) {
		rasm_printf(ae,KERROR"Too many errors!\n");
		FreeAssenv(ae);
		exit(ae->nberr);
	}
}

void (*___output)(struct s_assenv *ae, unsigned char v);

void ___internal_output_disabled(struct s_assenv *ae,unsigned char v)
{
	#undef FUNC
	#define FUNC "fake ___output"
}
void ___internal_output(struct s_assenv *ae,unsigned char v)
{
	#undef FUNC
	#define FUNC "___output"
	
	if (ae->outputadr<ae->maxptr) {
		ae->mem[ae->activebank][ae->outputadr++]=v;
		ae->codeadr++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"output exceed limit %d\n",ae->maxptr);
		ae->stop=1;
		___output=___internal_output_disabled;
	}
}
void ___internal_output_nocode(struct s_assenv *ae,unsigned char v)
{
	#undef FUNC
	#define FUNC "___output (nocode)"
	
	if (ae->outputadr<ae->maxptr) {
		/* struct definition always in NOCODE */
		if (ae->getstruct) {
			int irs,irsf;
			irs=ae->irasmstruct-1;
			irsf=ae->rasmstruct[irs].irasmstructfield-1;
			
			/* ajouter les data du flux au champ de la structure */			
			ObjectArrayAddDynamicValueConcat((void**)&ae->rasmstruct[irs].rasmstructfield[irsf].data,
				&ae->rasmstruct[irs].rasmstructfield[irsf].idata,
				&ae->rasmstruct[irs].rasmstructfield[irsf].mdata,
				&v,sizeof(unsigned char));
		}
		
		ae->outputadr++;
		ae->codeadr++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"output exceed limit %d\n",ae->maxptr);
		ae->stop=1;
		___output=___internal_output_disabled;
	}
}


void ___output_set_limit(struct s_assenv *ae,int zelimit)
{
	#undef FUNC
	#define FUNC "___output_set_limit"

	int limit=65536;
	
	if (zelimit<=limit) {
		/* apply limit */
		limit=zelimit;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"limit exceed hardware limitation!");
		ae->stop=1;
	}
	if (ae->outputadr>=0 && ae->outputadr>=limit) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"limit too high for current output!");
		ae->stop=1;
	}
	ae->maxptr=limit;
}

unsigned char *MakeAMSDOSHeader(int run, int minmem, int maxmem, char *amsdos_name) {
	#undef FUNC
	#define FUNC "MakeAMSDOSHeader"
	
	static unsigned char AmsdosHeader[128];
	int checksum,i=0;
	/***  cpcwiki			
	Byte 00: User number
	Byte 01 to 08: filename
	Byte 09 bis 11: Extension
	Byte 18: type-byte
	Byte 21 and 22: loading address
	Byte 24 and 25: file length
	Byte 26 and 27: execution address for machine code programs
	Byte 64 and 65: (file length)
	Byte 67 and 68: checksum for byte 00 to byte 66
	To calculate the checksum, just add byte 00 to byte 66 to each other.
	*/
	memset(AmsdosHeader,0,sizeof(AmsdosHeader));
	AmsdosHeader[0]=0;
	memcpy(AmsdosHeader+1,amsdos_name,11);

	AmsdosHeader[18]=2; /* 0 basic 1 basic protege 2 binaire */
	AmsdosHeader[19]=(maxmem-minmem)&0xFF;
	AmsdosHeader[20]=(maxmem-minmem)>>8;
	AmsdosHeader[21]=minmem&0xFF;
	AmsdosHeader[22]=minmem>>8;
	AmsdosHeader[24]=AmsdosHeader[19];
	AmsdosHeader[25]=AmsdosHeader[20];
	AmsdosHeader[26]=run&0xFF;
	AmsdosHeader[27]=run>>8;
	AmsdosHeader[64]=AmsdosHeader[19];
	AmsdosHeader[65]=AmsdosHeader[20];
	AmsdosHeader[66]=0;
	
	for (i=checksum=0;i<=66;i++) {
		checksum+=AmsdosHeader[i];
	}
	AmsdosHeader[67]=checksum&0xFF;
	AmsdosHeader[68]=checksum>>8;

	/* garbage / shadow values from sector buffer? */
	memcpy(AmsdosHeader+0x47,amsdos_name,8);
	AmsdosHeader[0x4F]=0x24;
	AmsdosHeader[0x50]=0x24;
	AmsdosHeader[0x51]=0x24;
	AmsdosHeader[0x52]=0xFF;
	AmsdosHeader[0x54]=0xFF;
	AmsdosHeader[0x57]=0x02;
	AmsdosHeader[0x5A]=AmsdosHeader[21];
	AmsdosHeader[0x5B]=AmsdosHeader[22];
	AmsdosHeader[0x5D]=AmsdosHeader[24];
	AmsdosHeader[0x5E]=AmsdosHeader[25];

	sprintf((char *)AmsdosHeader+0x47+17," created by %-9.9s ",RASM_SNAP_VERSION);

	return AmsdosHeader;
}

unsigned char *MakeHobetaHeader(int minmem, int maxmem, char *trdos_name) {
	#undef FUNC
	#define FUNC "MakeHobetaHeader"
	
	static unsigned char HobetaHeader[17];
	int i,checksum=0;
	/***  http://rk.nvg.ntnu.no/sinclair/faq/fileform.html#HOBETA			
   0x00     FileName     0x08      TR-DOS file name
   0x08     FileType     0x01      TR-DOS file type
   0x09     StartAdr     0x02      start address of file
   0x0A     FlLength     0x02      length of file (in bytes)  -> /!\ wrong offset!!!
   0x0C     FileSize     0x02      size of file (in sectors) 
   0x0E     HdrCRC16     0x02      Control checksum of the 15 byte
                                   header (not sector data!)
   */
	memset(HobetaHeader,0,sizeof(HobetaHeader));

	strncpy((char*)&HobetaHeader[0],trdos_name,8);
	HobetaHeader[8]='C';
	HobetaHeader[0x9]=(maxmem-minmem)&0xFF;
	HobetaHeader[0xA]=(maxmem-minmem)>>8;
	
	HobetaHeader[0xB]=(maxmem-minmem)&0xFF;
	HobetaHeader[0xC]=(maxmem-minmem)>>8;
	
	HobetaHeader[0xD]=((maxmem-minmem)+255)>>8;
	HobetaHeader[0xE]=0;
	
	for (i=0;i<0xF;i++) checksum+=HobetaHeader[i]*257+i;
	
	HobetaHeader[0xF]=checksum&0xFF;
	HobetaHeader[0x10]=(checksum>>8)&0xFF;

	return HobetaHeader;
}


int cmpAmsdosentry(const void * a, const void * b)
{
	return memcmp(a,b,32);
}

int cmpmacros(const void * a, const void * b)
{
	struct s_macro *sa,*sb;
	sa=(struct s_macro *)a;
	sb=(struct s_macro *)b;
	if (sa->crc<sb->crc) return -1; else return 1;
}
int SearchAlias(struct s_assenv *ae, int crc, char *zemot)
{
    int dw,dm,du,i;
//printf("SearchAlias [%s] ",zemot);
	/* inutile de tourner autour du pot pour un si petit nombre */
	if (ae->ialias<5) {
		for (i=0;i<ae->ialias;i++) {
			if (ae->alias[i].crc==crc && strcmp(ae->alias[i].alias,zemot)==0) {
				ae->alias[i].used=1;
//printf("found\n");
				return i;
			}
		}
//printf("not found\n");
		return -1;
	}
	
	dw=0;
	du=ae->ialias-1;
	while (dw<=du) {
		dm=(dw+du)/2;
		if (ae->alias[dm].crc==crc) {
			/* chercher le premier de la liste */
			while (dm>0 && ae->alias[dm-1].crc==crc) dm--;
			/* controle sur le texte entier */
			while (ae->alias[dm].crc==crc && strcmp(ae->alias[dm].alias,zemot)) dm++;
			if (ae->alias[dm].crc==crc && strcmp(ae->alias[dm].alias,zemot)==0) {
				ae->alias[dm].used=1;
//printf("[%s] found => [%s]\n",zemot,ae->alias[dm].translation);
				return dm;
			} else return -1;
		} else if (ae->alias[dm].crc>crc) {
			du=dm-1;
		} else if (ae->alias[dm].crc<crc) {
			dw=dm+1;
		}
	}
//printf("not found\n");
	return -1;
}
int SearchMacro(struct s_assenv *ae, int crc, char *zemot)
{
	int dw,dm,du,i;

	/* inutile de tourner autour du pot pour un si petit nombre */
	if (ae->imacro<5) {
			for (i=0;i<ae->imacro;i++) {
					if (ae->macro[i].crc==crc && strcmp(ae->macro[i].mnemo,zemot)==0) {
							return i;
					}
			}
			return -1;
	}
	
	dw=0;
	du=ae->imacro-1;
	while (dw<=du) {
		dm=(dw+du)/2;
		if (ae->macro[dm].crc==crc) {
			/* chercher le premier de la liste */
			while (dm>0 && ae->macro[dm-1].crc==crc) dm--;
			/* controle sur le texte entier */
			while (ae->macro[dm].crc==crc && strcmp(ae->macro[dm].mnemo,zemot)) dm++;
			if (ae->macro[dm].crc==crc && strcmp(ae->macro[dm].mnemo,zemot)==0) return dm; else return -1;
		} else if (ae->macro[dm].crc>crc) {
			du=dm-1;
		} else if (ae->macro[dm].crc<crc) {
			dw=dm+1;
		}
	}
	return -1;
}

void CheckAndSortAliases(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "CheckAndSortAliases"

	struct s_alias tmpalias;
	int i,dw,dm=0,du,crc;
	for (i=0;i<ae->ialias-1;i++) {
		/* is there previous aliases in the new alias? */
		if (strstr(ae->alias[ae->ialias-1].translation,ae->alias[i].alias)) {
			/* there is a match, apply alias translation */
			ExpressionFastTranslate(ae,&ae->alias[ae->ialias-1].translation,2);
			/* need to compute again len */
			ae->alias[ae->ialias-1].len=strlen(ae->alias[ae->ialias-1].translation);
			break;
		}
	}
	
	/* cas particuliers pour insertion en début ou fin de liste */
	if (ae->ialias-1) {
		if (ae->alias[ae->ialias-1].crc>ae->alias[ae->ialias-2].crc) {
			/* pas de tri il est déjà au bon endroit */
		} else if (ae->alias[ae->ialias-1].crc<ae->alias[0].crc) {
			/* insertion tout en bas de liste */
			tmpalias=ae->alias[ae->ialias-1];
			MemMove(&ae->alias[1],&ae->alias[0],sizeof(struct s_alias)*(ae->ialias-1));
			ae->alias[0]=tmpalias;
		} else {
			/* on cherche ou inserer */
			crc=ae->alias[ae->ialias-1].crc;
			dw=0;
			du=ae->ialias-1;
			while (dw<=du) {
				dm=(dw+du)/2;
				if (ae->alias[dm].crc==crc) {
					break;
				} else if (ae->alias[dm].crc>crc) {
					du=dm-1;
				} else if (ae->alias[dm].crc<crc) {
					dw=dm+1;
				}
			}
			/* ajustement */
			if (ae->alias[dm].crc<crc) dm++;
			/* insertion */
			tmpalias=ae->alias[ae->ialias-1];
			MemMove(&ae->alias[dm+1],&ae->alias[dm],sizeof(struct s_alias)*(ae->ialias-1-dm));
			ae->alias[dm]=tmpalias;
		}
	} else {
		/* one alias need no sort */
	}
}

void InsertDicoToTree(struct s_assenv *ae, struct s_expr_dico *dico)
{
	#undef FUNC
	#define FUNC "InsertDicoToTree"

	struct s_crcdico_tree *curdicotree;
	int radix,dek=32;

	curdicotree=&ae->dicotree;
	while (dek) {
		dek=dek-8;
		radix=(dico->crc>>dek)&0xFF;
		if (curdicotree->radix[radix]) {
			curdicotree=curdicotree->radix[radix];
		} else {
			curdicotree->radix[radix]=MemMalloc(sizeof(struct s_crcdico_tree));
			curdicotree=curdicotree->radix[radix];
			memset(curdicotree,0,sizeof(struct s_crcdico_tree));
		}
	}
	ObjectArrayAddDynamicValueConcat((void**)&curdicotree->dico,&curdicotree->ndico,&curdicotree->mdico,dico,sizeof(struct s_expr_dico));
}

unsigned char *SnapshotDicoInsert(char *symbol_name, int ptr, int *retidx)
{
	static unsigned char *subchunk=NULL;
	static int subchunksize=0;
	static int idx=0;
	int symbol_len;
	
	if (retidx) {
		if (symbol_name && strcmp(symbol_name,"FREE")==0) {
			subchunksize=0;
			idx=0;
			MemFree(subchunk);
			subchunk=NULL;
		}
		*retidx=idx;
		return subchunk;
	}
	
	if (idx+65536>subchunksize) {
		subchunksize=subchunksize+65536;
		subchunk=MemRealloc(subchunk,subchunksize);
	}
	
	symbol_len=strlen(symbol_name);
	if (symbol_len>255) symbol_len=255;
	subchunk[idx++]=symbol_len;
	memcpy(subchunk+idx,symbol_name,symbol_len);
	idx+=symbol_len;
	memset(subchunk+idx,0,6);
	idx+=6;
	subchunk[idx++]=(ptr&0xFF00)/256;
	subchunk[idx++]=ptr&0xFF;
	return NULL;
}

void SnapshotDicoTreeRecurse(struct s_crcdico_tree *lt)
{
	#undef FUNC
	#define FUNC "SnapshottDicoTreeRecurse"

	int i;

	for (i=0;i<256;i++) {
		if (lt->radix[i]) {
			SnapshotDicoTreeRecurse(lt->radix[i]);
		}
	}
	if (lt->mdico) {
		for (i=0;i<lt->ndico;i++) {
			if (strcmp(lt->dico[i].name,"IX") && strcmp(lt->dico[i].name,"IY") && strcmp(lt->dico[i].name,"PI") && strcmp(lt->dico[i].name,"ASSEMBLER_RASM")) {
				SnapshotDicoInsert(lt->dico[i].name,(int)floor(lt->dico[i].v+0.5),NULL);
			}
		}
	}
}
unsigned char *SnapshotDicoTree(struct s_assenv *ae, int *retidx)
{
	#undef FUNC
	#define FUNC "SnapshotDicoTree"

	unsigned char *sc;
	int idx;
	int i;

	for (i=0;i<256;i++) {
		if (ae->dicotree.radix[i]) {
			SnapshotDicoTreeRecurse(ae->dicotree.radix[i]);
		}
	}
	
	sc=SnapshotDicoInsert(NULL,0,&idx);
	*retidx=idx;
	return sc;
}

void WarnLabelTreeRecurse(struct s_assenv *ae, struct s_crclabel_tree *lt)
{
	#undef FUNC
	#define FUNC "WarnLabelTreeRecurse"

	int i;

	for (i=0;i<256;i++) {
		if (lt->radix[i]) {
			WarnLabelTreeRecurse(ae,lt->radix[i]);
		}
	}
	for (i=0;i<lt->nlabel;i++) {
		if (!lt->label[i].used) {
			if (!lt->label[i].name) {
				rasm_printf(ae,KWARNING"[%s:%d] Warning: label %s declared but not used\n",ae->filename[lt->label[i].fileidx],lt->label[i].fileline,ae->wl[lt->label[i].iw].w);
				if (ae->erronwarn) MaxError(ae);
			} else {
				rasm_printf(ae,KWARNING"[%s:%d] Warning: label %s declared but not used\n",ae->filename[lt->label[i].fileidx],lt->label[i].fileline,lt->label[i].name);
				if (ae->erronwarn) MaxError(ae);
			}
		}
	}
}
void WarnLabelTree(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "WarnLabelTree"

	int i;

	for (i=0;i<256;i++) {
		if (ae->labeltree.radix[i]) {
			WarnLabelTreeRecurse(ae,ae->labeltree.radix[i]);
		}
	}
}
void WarnDicoTreeRecurse(struct s_assenv *ae, struct s_crcdico_tree *lt)
{
	#undef FUNC
	#define FUNC "WarnDicoTreeRecurse"

	int i;


	for (i=0;i<256;i++) {
		if (lt->radix[i]) {
			WarnDicoTreeRecurse(ae,lt->radix[i]);
		}
	}
	for (i=0;i<lt->ndico;i++) {
		if (strcmp(lt->dico[i].name,"IX") && strcmp(lt->dico[i].name,"IY") && strcmp(lt->dico[i].name,"PI") && strcmp(lt->dico[i].name,"ASSEMBLER_RASM") && lt->dico[i].autorise_export) {
			rasm_printf(ae,KWARNING"[%s:%d] Warning: variable %s declared but not used\n",ae->filename[ae->wl[lt->dico[i].iw].ifile],ae->wl[lt->dico[i].iw].l,lt->dico[i].name);
				if (ae->erronwarn) MaxError(ae);
		}
	}
}
void WarnDicoTree(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "ExportDicoTree"

	int i;

	for (i=0;i<256;i++) {
		if (ae->dicotree.radix[i]) {
			WarnDicoTreeRecurse(ae,ae->dicotree.radix[i]);
		}
	}
}
void ExportDicoTreeRecurse(struct s_crcdico_tree *lt, char *zefile, char *zeformat)
{
	#undef FUNC
	#define FUNC "ExportDicoTreeRecurse"

	char symbol_line[1024];
	int i;

	for (i=0;i<256;i++) {
		if (lt->radix[i]) {
			ExportDicoTreeRecurse(lt->radix[i],zefile,zeformat);
		}
	}
	if (lt->mdico) {
		for (i=0;i<lt->ndico;i++) {
			if (strcmp(lt->dico[i].name,"IX") && strcmp(lt->dico[i].name,"IY") && strcmp(lt->dico[i].name,"PI") && strcmp(lt->dico[i].name,"ASSEMBLER_RASM") && lt->dico[i].autorise_export) {
				snprintf(symbol_line,sizeof(symbol_line)-1,zeformat,lt->dico[i].name,(int)floor(lt->dico[i].v+0.5));
				symbol_line[sizeof(symbol_line)-1]=0xD;
				FileWriteLine(zefile,symbol_line);
			}
		}
	}
}
void ExportDicoTree(struct s_assenv *ae, char *zefile, char *zeformat)
{
	#undef FUNC
	#define FUNC "ExportDicoTree"

	int i;

	for (i=0;i<256;i++) {
		if (ae->dicotree.radix[i]) {
			ExportDicoTreeRecurse(ae->dicotree.radix[i],zefile,zeformat);
		}
	}
}
void FreeDicoTreeRecurse(struct s_crcdico_tree *lt)
{
	#undef FUNC
	#define FUNC "FreeDicoTreeRecurse"

	int i;

	for (i=0;i<256;i++) {
		if (lt->radix[i]) {
			FreeDicoTreeRecurse(lt->radix[i]);
		}
	}
	if (lt->mdico) {
		for (i=0;i<lt->ndico;i++) {
			MemFree(lt->dico[i].name);
		}
		MemFree(lt->dico);
	}
	MemFree(lt);
}
void FreeDicoTree(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "FreeDicoTree"

	int i;

	for (i=0;i<256;i++) {
		if (ae->dicotree.radix[i]) {
			FreeDicoTreeRecurse(ae->dicotree.radix[i]);
		}
	}
	if (ae->dicotree.mdico) {
		for (i=0;i<ae->dicotree.ndico;i++) MemFree(ae->dicotree.dico[i].name);
		MemFree(ae->dicotree.dico);
	}
}
struct s_expr_dico *SearchDico(struct s_assenv *ae, char *dico, int crc)
{
	#undef FUNC
	#define FUNC "SearchDico"

	struct s_crcdico_tree *curdicotree;
	int i,radix,dek=32;


	curdicotree=&ae->dicotree;

	while (dek) {
		dek=dek-8;
		radix=(crc>>dek)&0xFF;
		if (curdicotree->radix[radix]) {
			curdicotree=curdicotree->radix[radix];
		} else {
			/* radix not found, dico is not in index */
			return NULL;
		}
	}
	for (i=0;i<curdicotree->ndico;i++) {
		if (strcmp(curdicotree->dico[i].name,dico)==0) {
			curdicotree->dico[i].used=1;
			return &curdicotree->dico[i];
		}
	}
	return NULL;
}
int DelDico(struct s_assenv *ae, char *dico, int crc)
{
	#undef FUNC
	#define FUNC "DelDico"

	struct s_crcdico_tree *curdicotree;
	int i,radix,dek=32;

	curdicotree=&ae->dicotree;

	while (dek) {
		dek=dek-8;
		radix=(crc>>dek)&0xFF;
		if (curdicotree->radix[radix]) {
			curdicotree=curdicotree->radix[radix];
		} else {
			/* radix not found, dico is not in index */
			return 0;
		}
	}
	for (i=0;i<curdicotree->ndico;i++) {
		if (strcmp(curdicotree->dico[i].name,dico)==0) {
			/* must free memory */
			MemFree(curdicotree->dico[i].name);
			if (i<curdicotree->ndico-1) {
				MemMove(&curdicotree->dico[i],&curdicotree->dico[i+1],(curdicotree->ndico-i-1)*sizeof(struct s_expr_dico));
			}
			curdicotree->ndico--;
			return 1;
		}
	}
	return 0;
}


void InsertUsedToTree(struct s_assenv *ae, char *used, int crc)
{
	#undef FUNC
	#define FUNC "InsertUsedToTree"

	struct s_crcused_tree *curusedtree;
	int radix,dek=32,i;
	
	curusedtree=&ae->usedtree;
	while (dek) {
		dek=dek-8;
		radix=(crc>>dek)&0xFF;
		if (curusedtree->radix[radix]) {
			curusedtree=curusedtree->radix[radix];
		} else {
			curusedtree->radix[radix]=MemMalloc(sizeof(struct s_crcused_tree));
			curusedtree=curusedtree->radix[radix];
			memset(curusedtree,0,sizeof(struct s_crcused_tree));
		}
	}
	for (i=0;i<curusedtree->nused;i++) if (strcmp(used,curusedtree->used[i])==0) break;
	/* no double */
	if (i==curusedtree->nused) {
		FieldArrayAddDynamicValueConcat(&curusedtree->used,&curusedtree->nused,&curusedtree->mused,used);
	}
}

void FreeUsedTreeRecurse(struct s_crcused_tree *lt)
{
	#undef FUNC
	#define FUNC "FreeUsedTreeRecurse"

	int i;

	for (i=0;i<256;i++) {
		if (lt->radix[i]) {
			FreeUsedTreeRecurse(lt->radix[i]);
		}
	}
	if (lt->mused) {
		for (i=0;i<lt->nused;i++) MemFree(lt->used[i]);
		MemFree(lt->used);
	}
	MemFree(lt);
}
void FreeUsedTree(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "FreeUsedTree"

	int i;

	for (i=0;i<256;i++) {
		if (ae->usedtree.radix[i]) {
			FreeUsedTreeRecurse(ae->usedtree.radix[i]);
		}
	}
}
int SearchUsed(struct s_assenv *ae, char *used, int crc)
{
	#undef FUNC
	#define FUNC "SearchUsed"

	struct s_crcused_tree *curusedtree;
	int i,radix,dek=32;

	curusedtree=&ae->usedtree;
	while (dek) {
		dek=dek-8;
		radix=(crc>>dek)&0xFF;
		if (curusedtree->radix[radix]) {
			curusedtree=curusedtree->radix[radix];
		} else {
			/* radix not found, used is not in index */
			return 0;
		}
	}
	for (i=0;i<curusedtree->nused;i++) {
		if (strcmp(curusedtree->used[i],used)==0) {
			return 1;
		}
	}
	return 0;
}



void InsertTextToTree(struct s_assenv *ae, char *text, char *replace, int crc)
{
	#undef FUNC
	#define FUNC "InsertTextToTree"

	struct s_crcstring_tree *curstringtree;
	int radix,dek=32,i;
	
	curstringtree=&ae->stringtree;
	while (dek) {
		dek=dek-8;
		radix=(crc>>dek)&0xFF;
		if (curstringtree->radix[radix]) {
			curstringtree=curstringtree->radix[radix];
		} else {
			curstringtree->radix[radix]=MemMalloc(sizeof(struct s_crcused_tree));
			curstringtree=curstringtree->radix[radix];
			memset(curstringtree,0,sizeof(struct s_crcused_tree));
		}
	}
	for (i=0;i<curstringtree->ntext;i++) if (strcmp(text,curstringtree->text[i])==0) break;
	/* no double */
	if (i==curstringtree->ntext) {
		text=TxtStrDup(text);
		replace=TxtStrDup(replace);
		FieldArrayAddDynamicValueConcat(&curstringtree->text,&curstringtree->ntext,&curstringtree->mtext,text);
		FieldArrayAddDynamicValueConcat(&curstringtree->replace,&curstringtree->nreplace,&curstringtree->mreplace,replace);
	}
}

void FreeTextTreeRecurse(struct s_crcstring_tree *lt)
{
	#undef FUNC
	#define FUNC "FreeTextTreeRecurse"

	int i;

	for (i=0;i<256;i++) {
		if (lt->radix[i]) {
			FreeTextTreeRecurse(lt->radix[i]);
		}
	}
	if (lt->mtext) {
		for (i=0;i<lt->ntext;i++) MemFree(lt->text[i]);
		MemFree(lt->text);
	}
	MemFree(lt);
}
void FreeTextTree(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "FreeTextTree"

	int i;

	for (i=0;i<256;i++) {
		if (ae->stringtree.radix[i]) {
			FreeTextTreeRecurse(ae->stringtree.radix[i]);
		}
	}
	if (ae->stringtree.mtext) MemFree(ae->stringtree.text);
}
int SearchText(struct s_assenv *ae, char *text, int crc)
{
	#undef FUNC
	#define FUNC "SearchText"

	struct s_crcstring_tree *curstringtree;
	int i,radix,dek=32;

	curstringtree=&ae->stringtree;
	while (dek) {
		dek=dek-8;
		radix=(crc>>dek)&0xFF;
		if (curstringtree->radix[radix]) {
			curstringtree=curstringtree->radix[radix];
		} else {
			/* radix not found, used is not in index */
			return 0;
		}
	}
	for (i=0;i<curstringtree->ntext;i++) {
		if (strcmp(curstringtree->text[i],text)==0) {
			return 1;
		}
	}
	return 0;
}







/*
struct s_crclabel_tree {





struct s_crclabel_tree {
	struct s_crclabel_tree *radix[256];
	struct s_label *label;
	int nlabel,mlabel;
};
*/
void FreeLabelTreeRecurse(struct s_crclabel_tree *lt)
{
	#undef FUNC
	#define FUNC "FreeLabelTreeRecurse"

	int i;

	for (i=0;i<256;i++) {
		if (lt->radix[i]) {
			FreeLabelTreeRecurse(lt->radix[i]);
		}
	}
	/* label.name already freed elsewhere as this one is a copy */
	if (lt->mlabel) MemFree(lt->label);
	MemFree(lt);
}
void FreeLabelTree(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "FreeLabelTree"

	int i;

	for (i=0;i<256;i++) {
		if (ae->labeltree.radix[i]) {
			FreeLabelTreeRecurse(ae->labeltree.radix[i]);
		}
	}
	if (ae->labeltree.mlabel) MemFree(ae->labeltree.label);
}

struct s_label *SearchLabel(struct s_assenv *ae, char *label, int crc)
{
	#undef FUNC
	#define FUNC "SearchLabel"

	struct s_crclabel_tree *curlabeltree;
	int i,radix,dek=32;

//printf("searchLabel [%s]",label);
	curlabeltree=&ae->labeltree;
	while (dek) {
		dek=dek-8;
		radix=(crc>>dek)&0xFF;
		if (curlabeltree->radix[radix]) {
			curlabeltree=curlabeltree->radix[radix];
		} else {
			/* radix not found, label is not in index */
//printf(" not found\n");
			return NULL;
		}
	}
	for (i=0;i<curlabeltree->nlabel;i++) {
		if (!curlabeltree->label[i].name && strcmp(ae->wl[curlabeltree->label[i].iw].w,label)==0) {
			curlabeltree->label[i].used=1;
//printf(" found (global)\n");
			return &curlabeltree->label[i];
		} else if (curlabeltree->label[i].name && strcmp(curlabeltree->label[i].name,label)==0) {
			curlabeltree->label[i].used=1;
//printf(" found (local or proximity)\n");
			return &curlabeltree->label[i];
		}
	}
	return NULL;
}

char *MakeLocalLabel(struct s_assenv *ae,char *varbuffer, int *retdek)
{
	#undef FUNC
	#define FUNC "MakeLocalLabel"
	
	char *locallabel;
	char hexdigit[32];
	int lenbuf=0,dek,i,im;
	char *zepoint;

	lenbuf=strlen(varbuffer);
	
	/* not so local labels */
	if (varbuffer[0]=='.') {
		/* create reference */
		if (ae->lastgloballabel) {
			locallabel=MemMalloc(strlen(varbuffer)+1+ae->lastgloballabellen);
			sprintf(locallabel,"%s%s",ae->lastgloballabel,varbuffer);
			if (retdek) *retdek=0;
			return locallabel;
		} else {
			if (retdek) *retdek=0;
			return TxtStrDup(varbuffer);
		}
	}

	/***************************************************
	without retdek -> build a local label
	with    retdek -> build the hash string
	***************************************************/	
	if (!retdek) {
		locallabel=MemMalloc(lenbuf+(ae->ir+ae->iw+3)*8+8);
		zepoint=strchr(varbuffer,'.');
		if (zepoint) {
			*zepoint=0;
		}
		strcpy(locallabel,varbuffer);
	} else {
		locallabel=MemMalloc((ae->ir+ae->iw+3)*8+4);
		locallabel[0]=0;
	}	
//printf("locallabel=[%s] (draft)\n",locallabel);

	dek=0;
	dek+=strappend(locallabel,"R");
	for (i=0;i<ae->ir;i++) {
		sprintf(hexdigit,"%04X",ae->repeat[i].cpt);
		dek+=strappend(locallabel,hexdigit);
	}
	if (ae->ir) {
		sprintf(hexdigit,"%04X",ae->repeat[ae->ir-1].value);
		dek+=strappend(locallabel+dek,hexdigit);
	}
	
	dek+=strappend(locallabel,"W");
	for (i=0;i<ae->iw;i++) {
		sprintf(hexdigit,"%04X",ae->whilewend[i].cpt);
		dek+=strappend(locallabel+dek,hexdigit);
	}
	if (ae->iw) {
		sprintf(hexdigit,"%04X",ae->whilewend[ae->iw-1].value);
		dek+=strappend(locallabel+dek,hexdigit);
	}
	/* where are we? */
	if (ae->imacropos) {
		for (im=ae->imacropos-1;im>=0;im--) {
			if (ae->idx>=ae->macropos[im].start && ae->idx<ae->macropos[im].end) break;
		}
		if (im>=0) {
			/* si on n'est pas dans une macro, on n'indique rien */
			sprintf(hexdigit,"M%04X",ae->macropos[im].value);
			dek+=strappend(locallabel+dek,hexdigit);
		}
	}
	if (!retdek) {
		if (zepoint) {
			*zepoint='.';
			strcat(locallabel+dek,zepoint);
		}
	} else {
		*retdek=dek;
	}
//printf("locallabel=[%s] (end)\n",locallabel);
	return locallabel;
}

char *TradExpression(char *zexp)
{
	#undef FUNC
	#define FUNC "TradExpression"
	
	static char *last_expression=NULL;
	char *wstr;
	
	if (last_expression) {MemFree(last_expression);last_expression=NULL;}
	if (!zexp) return NULL;
	
	wstr=TxtStrDup(zexp);
	wstr=TxtReplace(wstr,"[","<<",0);
	wstr=TxtReplace(wstr,"]",">>",0);
	wstr=TxtReplace(wstr,"m","%",0);

	last_expression=wstr;
	return wstr;
}

int TrimFloatingPointString(char *fps) {
	int i=0,pflag,zflag=0;
	
	while (fps[i]) {
		if (fps[i]=='.') {
			pflag=i;
			zflag=1;
		} else if (fps[i]!='0') {
			zflag=0;
		}
		i++;
	}
	/* truncate floating fract */
	if (zflag) {
		fps[pflag]=0;
	} else {
		pflag=i;
	}
	return pflag;
}



/*
	translate tag or formula between curly brackets
	used in label declaration
	used in print directive
*/
char *TranslateTag(struct s_assenv *ae, char *varbuffer, int *touched, int enablefast, int tagoption) {
	/*******************************************************
	       v a r i a b l e s     i n    s t r i n g s
	*******************************************************/
	char *starttag,*endtag,*tagcheck,*expr;
	int newlen,lenw,taglen,tagidx,tagcount,validx;
	char curvalstr[256]={0};


//printf("TranslateTag [%s]\n",varbuffer);

	if (tagoption & E_TAGOPTION_PRESERVE) {
		if (ae->iw || ae->ir) {
			/* inside a loop we must care about variables */
//printf("TranslateTag [%s] with PRESERVE inside a loop!\n",varbuffer);
			return varbuffer;
		}
	}

	*touched=0;
	while ((starttag=strchr(varbuffer+1,'{'))!=NULL) {
		if ((endtag=strchr(starttag,'}'))==NULL) {
			MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"invalid tag in string [%s]\n",varbuffer);
			return NULL;
		}
		/* allow inception */
		tagcount=1;
		tagcheck=starttag+1;
		while (*tagcheck && tagcount) {
			if (*tagcheck=='}') tagcount--; else if (*tagcheck=='{') tagcount++;
			tagcheck++;			
		}
		if (tagcount) {
			MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"invalid brackets combination in string [%s]\n",varbuffer);
			return NULL;
		} else {
			endtag=tagcheck-1;
		}
		*touched=1;
		taglen=endtag-starttag+1;
		tagidx=starttag-varbuffer;
		lenw=strlen(varbuffer); // before the EOF write
		*endtag=0;
		/*** c o m p u t e    e x p r e s s i o n ***/
		expr=TxtStrDup(starttag+1);
		if (tagoption & E_TAGOPTION_REMOVESPACE) expr=TxtReplace(expr," ","",0);
		if (enablefast) ExpressionFastTranslate(ae,&expr,0);
		validx=(int)RoundComputeExpressionCore(ae,expr,ae->codeadr,0);
		if (validx<0) {
			strcpy(curvalstr,"");
			MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"indexed tag must NOT be a negative value [%s]\n",varbuffer);
			MemFree(expr);
			return NULL;
		} else {
			#ifdef OS_WIN
			snprintf(curvalstr,sizeof(curvalstr)-1,"%d",validx);
			newlen=strlen(curvalstr);
			#else
			newlen=snprintf(curvalstr,sizeof(curvalstr)-1,"%d",validx);
			#endif
		}
		MemFree(expr);
		if (newlen>taglen) {
			/* realloc */
			varbuffer=MemRealloc(varbuffer,lenw+newlen-taglen+1);
		}
		if (newlen!=taglen ) {
			MemMove(varbuffer+tagidx+newlen,varbuffer+tagidx+taglen,lenw-taglen-tagidx+1);
		}
		strncpy(varbuffer+tagidx,curvalstr,newlen); /* copy without zero terminator */
	}

	return varbuffer;
}

#define CRC_NOP		0xE1830165
#define CRC_LDI		0xE18B3F51
#define CRC_LDD		0xE18B3F4C
#define CRC_DEC		0xE06BDD44
#define CRC_INC		0xE19F3B52
#define CRC_CPI		0xE077C754
#define CRC_CPD		0xE077C74F
#define CRC_BIT		0xE073D557
#define CRC_RES		0xE1B32D62
#define CRC_SET		0xE1B71164
#define CRC_CCF		0xE0742D44
#define CRC_IND		0xE19F3B53
#define CRC_INI		0xE19F3B58
#define CRC_DAA		0xE068253E
#define CRC_CPL		0xE077C757
#define CRC_EI		0x4BD5DD06
#define CRC_DI		0x4BD5DF05
#define CRC_IM		0x4BD5250E
#define CRC_SCF		0xE1B72D54
#define CRC_NEG		0xE1833D52
#define CRC_OUTI	0xEFA5F1B9
#define CRC_OUTD	0xEFA5F1B4
#define CRC_OUT		0xE1871170
#define CRC_IN		0x4BD5250F

#define CRC_RLA		0xE1B31F57
#define CRC_RLCA	0x878DAD9A
#define CRC_RRCA	0x87A5B5A0
#define CRC_RRA		0xE1B30B5D
#define CRC_RLD		0xE1B31F5A
#define CRC_RRD		0xE1B30B60
#define CRC_RST		0xE1B30971
#define CRC_RR		0x4BD5331C
#define CRC_RL		0x4BD53316
#define CRC_RRC		0xE1B30B5F
#define CRC_RLC		0xE1B31F59
#define CRC_SLA		0xE1B71F58
#define CRC_SLL		0xE1B71F63
#define CRC_SRA		0xE1B70B5E
#define CRC_SRL		0xE1B70B69

#define CRC_ADD		0xE07C2F41
#define CRC_ADC		0xE07C2F40
#define CRC_SBC		0xE1B72B50
#define CRC_SUB		0xE1B77162
#define CRC_XOR		0xE1DB3971
#define CRC_AND		0xE07FDB4B
#define CRC_OR		0x4BD52919
#define CRC_CP		0x4BD5D10B

#define CRC_PUSH	0x97A1EDB8
#define CRC_POP		0xE1BB1967

#define CRC_JR		0x4BD52314
#define CRC_JP		0x4BD52312
#define CRC_DJNZ	0x37CD7BAE
#define CRC_RET		0xE1B32D63
#define CRC_RETN	0x87E9EBB1
#define CRC_RETI	0x87E9EBAC

#define CRC_LD		0x4BD52F08

#define CRC_EX		0x4BD5DD15
#define CRC_EXX		0xE06FF76D
#define CRC_LDIR	0xF7F59DA3
#define CRC_LDDR	0xF7F5A79E
#define CRC_INIR	0xDFE98BAA
#define CRC_INDR	0xDFE99DA5
#define CRC_OTIR	0xEFB9D7B6
#define CRC_OTDR	0xEFB9A1B1
#define CRC_CPIR	0xFF96FA6
#define CRC_CPDR	0xFF959A1



int __GETNOP(struct s_assenv *ae,char *oplist, int didx)
{
	#undef FUNC
	#define FUNC "__GETNOP"

	int idx=0,crc,tick=0;
	char **opcode=NULL;
	char *opref;

	/* upper case */
	while (oplist[idx]) {
		oplist[idx]=toupper(oplist[idx]);
		idx++;
	}
	/* duplicata */
	opref=TxtStrDup(oplist);
	/* clean-up */
	TxtReplace(opref,"\t"," ",0);
	TxtReplace(opref,"  "," ",1);
	TxtReplace(opref,": ",":",1);
	/* simplify extended registers to XL or IX */
	TxtReplace(opref,"IY","IX",0);
	TxtReplace(opref,"IXL","XL",0);
	TxtReplace(opref,"IXH","XL",0);
	TxtReplace(opref,"LX","XL",0);
	TxtReplace(opref,"HX","XL",0);
	TxtReplace(opref,"LY","XL",0);
	TxtReplace(opref,"HY","XL",0);
	TxtReplace(opref,"YL","XL",0);
	TxtReplace(opref,"XH","XL",0);
	TxtReplace(opref,"YH","XL",0);

	/* count opcodes */
	opcode=TxtSplitWithChar(opref,':');

	idx=0;
	while (opcode[idx]) {
		char *zeopcode,*terminator,*zearg=NULL;
		char **listarg;

		zeopcode=opcode[idx];
		/* trim */
		while (*zeopcode==' ') zeopcode++;
		terminator=zeopcode;
		while (*terminator!=0 && *terminator!=' ') terminator++;
		if (*terminator) {
			zearg=terminator+1;
			*terminator=0;
			/* no space in args */
			TxtReplace(zearg," ","",1);
		}
		if (!zeopcode[0]) {idx++;continue;}
		crc=GetCRC(zeopcode);

		/*************************************
		* very simple and simplified parsing *
		*************************************/
		switch (crc) {
			case CRC_RLA:
			case CRC_RLCA:
			case CRC_RRCA:
			case CRC_RRA:
			case CRC_NOP:
			case CRC_CCF:
			case CRC_DAA:
			case CRC_SCF:
			case CRC_CPL:
			case CRC_EXX:
			case CRC_EI:
			case CRC_DI:tick+=1;break;

			case CRC_IM:
			case CRC_NEG:tick+=2;break;

			case CRC_RST:
			case CRC_RETN:
			case CRC_RETI:
			case CRC_CPDR:
			case CRC_CPIR:
			case CRC_CPD:
			case CRC_CPI:tick+=4;break;

			case CRC_RLD:
			case CRC_RRD:
			case CRC_LDD:
			case CRC_LDI:
			case CRC_OUTI:
			case CRC_OUTD:
			case CRC_LDIR:
			case CRC_LDDR:
			case CRC_INIR:
			case CRC_INDR:
			case CRC_OTIR:
			case CRC_OTDR:
			case CRC_IND:
			case CRC_INI:tick+=5;break;

			case CRC_EX:
				if (zearg) {
					if (strstr(zearg,"AF") || strstr(zearg,"DE")) tick+=1; else
					if (strstr(zearg,"(SP)") && strstr(zearg,"HL")) tick+=6; else
					if (strstr(zearg,"(SP)") && strstr(zearg,"IX")) tick+=7;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETNOP, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_PUSH:
				if (zearg) {
					if (strcmp(zearg,"IX")==0) tick+=5; else tick+=4;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETNOP, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_POP:
				if (zearg) {
					if (strcmp(zearg,"IX")==0) tick+=4; else tick+=3;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETNOP, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_SLA:
			case CRC_SLL:
			case CRC_SRA:
			case CRC_SRL:
			case CRC_RL:
			case CRC_RLC:
			case CRC_RR:
			case CRC_RRC:
				if (zearg) {
					if (strstr(zearg,"(HL)")) tick+=4; else
					if (strstr(zearg,"(IX")) tick+=7; else
						tick+=2;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETNOP, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_OUT:
			case CRC_IN:
				if (zearg) {
					if (strstr(zearg,"(C)")) tick+=4; else tick+=3;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETNOP, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_ADD:
			     if (zearg) {
					/* simplify deprecated notation */
					TxtReplace(zearg,"A,","",0);
					if (strcmp(zearg,"IX,BC")==0 || strcmp(zearg,"IX,DE")==0 || strcmp(zearg,"IX,IX")==0 || strcmp(zearg,"IX,SP")==0) tick+=4; else
					if (strcmp(zearg,"HL,BC")==0 || strcmp(zearg,"HL,DE")==0 || strcmp(zearg,"HL,HL")==0 || strcmp(zearg,"HL,SP")==0) tick+=3; else
					if (strstr(zearg,"(HL)") || strcmp(zearg,"XL")==0) tick+=2; else
					if (strstr(zearg,"(IX")) tick+=5; else
					if ((*zearg>='A' && *zearg<='E') || *zearg=='H' || *zearg=='L') tick+=1; else tick+=2;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETNOP, see documentation about this directive\n",opcode[idx]);
				}
				break;

			/* ADC/SBC/SUB/XOR/AND/OR */
			case CRC_ADC:
				if (zearg) {
					/* simplify deprecated notation */
					TxtReplace(zearg,"A,","",0);
					if (strcmp(zearg,"IX,BC")==0 || strcmp(zearg,"IX,DE")==0 ||strcmp(zearg,"IX,IX")==0 ||strcmp(zearg,"IX,SP")==0) {tick+=5;break;}
				}
			case CRC_SBC:
				if (zearg) {
					/* simplify deprecated notation */
					TxtReplace(zearg,"A,","",0);
					if (strcmp(zearg,"HL,BC")==0 || strcmp(zearg,"HL,DE")==0 ||strcmp(zearg,"HL,HL")==0 ||strcmp(zearg,"HL,SP")==0) {tick+=4;break;}
				}
			case CRC_SUB:
				/* simplify deprecated notation */
				TxtReplace(zearg,"A,","",0);
			case CRC_XOR:
			case CRC_AND:
			case CRC_OR:
			case CRC_CP:
			     if (zearg) {
					if (strstr(zearg,"(HL)") || strcmp(zearg,"XL")==0) tick+=2; else
					if (strstr(zearg,"(IX")) tick+=5; else
					if ((*zearg>='A' && *zearg<='E') || *zearg=='H' || *zearg=='L') tick+=1; else tick+=2;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETNOP, see documentation about this directive\n",opcode[idx]);
				}
				break;

			/* BIT/RES/SET */
			case CRC_BIT:
				if (strstr(zearg,"(HL)")) tick+=3; else
				if (strstr(zearg,"(IX")) tick+=6; else tick+=2;
				break;
			case CRC_RES:
			case CRC_SET:
				if (strstr(zearg,"(HL)")) tick+=4; else
				if (strstr(zearg,"(IX")) tick+=7; else tick+=2;
				break;
			case CRC_DEC:
			case CRC_INC:
				if (strcmp(zearg,"XL")==0 || strcmp(zearg,"SP")==0 || strcmp(zearg,"BC")==0
				     || strcmp(zearg,"DE")==0 || strcmp(zearg,"HL")==0)
					     tick+=2;
				else if (strcmp(zearg,"IX")==0 || strcmp(zearg,"(HL)")==0)
						tick+=3;
				else if (strncmp(zearg,"(IX",3)==0)
						tick+=6;
				else tick++;
				break;
			case CRC_JP:
				// JP is supposed to loop!
				if (zearg) {
					if (strcmp(zearg,"(IX)")==0)
						tick+=2;
					else if (strcmp(zearg,"(HL)")==0)
						tick+=1;
					else tick+=3;
				} else tick+=3;
				break;
			case CRC_DJNZ:
				// DJNZ is supposed to loop!
			case CRC_JR:
				// JR is supposed to loop!
				tick+=3;
				break;
			case CRC_RET:
				// conditionnal RET shorter because it's supposed to be the exit!
				if (!zearg) tick+=3; else tick+=2;
				break;

			case CRC_LD:
				/* big cake! */
				if (zearg && strchr(zearg,',')) {
					int crc1,crc2;

					/* split args */
					listarg=TxtSplitWithChar(zearg,',');
					crc1=GetCRC(listarg[0]);
					crc2=GetCRC(listarg[1]);

					switch (crc1) {
						case CRC_I:
						case CRC_R:
							switch (crc2) {
								case CRC_A:
									tick+=3;
									break;
								default:
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETNOP, see documentation\n",listarg[0],listarg[1]);
							}
							break;
						case CRC_A:
						case CRC_B:
						case CRC_C:
						case CRC_D:
						case CRC_E:
						case CRC_H:
						case CRC_L:
							switch (crc2) {
								case CRC_A:
								case CRC_B:
								case CRC_C:
								case CRC_D:
								case CRC_E:
								case CRC_H:
								case CRC_L:
									tick++;
									break;
								case CRC_I:
								case CRC_R:
									if (crc1==CRC_A) tick+=3; else
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETNOP, see documentation\n",listarg[0],listarg[1]);
									break;
								case CRC_MBC:
								case CRC_MDE:
									if (crc1!=CRC_A) {
										MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETNOP, see documentation\n",listarg[0],listarg[1]);
										break;
									}
								case CRC_XL:
								case CRC_MHL:
									tick+=2;
									break;
								default:
									/* MIX + memory + value */
									if (strncmp(listarg[1],"(IX",3)==0) {
										tick+=5;
									} else if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') {
										/* memory */
										if (crc1==CRC_A) {
										tick+=4;
										} else {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETNOP, see documentation\n",listarg[0],listarg[1]);
										}
									} else {
										/* numeric value as default */
										tick+=2;
									}
							}
							break;

						case CRC_XL:
							switch (crc2) {
								case CRC_A:
								case CRC_B:
								case CRC_C:
								case CRC_D:
								case CRC_E:
								case CRC_H:
								case CRC_L:
									tick+=2;
									break;
								case CRC_XL:
									tick+=2;
									break;
								default:
									/* value */
									tick+=3;
							}
							break;

						case CRC_BC:
						case CRC_DE:
							/* memory / value */
							if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') tick+=6; else tick+=3;
							break;
						case CRC_HL:
							/* memory / value */
							if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') tick+=5; else tick+=3;
							break;
						case CRC_SP:
							if (crc2==CRC_HL) {
								tick+=2;
							} else if (crc2==CRC_IX) {
								/* IX */
								tick+=3;
							} else if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') {
								/* memory */
								tick+=6;
							} else tick+=3;
							break;
						case CRC_IX:
							/* memory / value */
							if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') tick+=6; else tick+=4;
							break;

						case CRC_MBC:
						case CRC_MDE:
							if (crc2==CRC_A) {
								tick+=2;
							} else {
								MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETNOP, see documentation\n",listarg[0],listarg[1]);
							}
							break;
						case CRC_MHL:
							switch (crc2) {
								case CRC_A:
								case CRC_B:
								case CRC_C:
								case CRC_D:
								case CRC_E:
								case CRC_H:
								case CRC_L:
									tick+=2;
									break;
								default:
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETNOP, see documentation\n",listarg[0],listarg[1]);
							}
							break;
						default:
							if (strncmp(listarg[0],"(IX",3)==0) {
								/* MIX */
								switch (crc2) {
									case CRC_A:
									case CRC_B:
									case CRC_C:
									case CRC_D:
									case CRC_E:
									case CRC_H:
									case CRC_L:tick+=5;break;
									default:tick+=6;
								}
							} else if (listarg[0][0]=='(' && listarg[0][strlen(listarg[0])-1]==')') {
								/* memory */
								switch (crc2) {
									case CRC_A:tick+=4;break;
									case CRC_HL:tick+=5;break;
									case CRC_BC:
									case CRC_DE:
									case CRC_SP:
									case CRC_IX:tick+=6;break;
									default:
										MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETNOP, see documentation\n",listarg[0],listarg[1]);
								}
							} else {
								MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETNOP, see documentation\n",listarg[0],listarg[1]);
							}
					}
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode LD for GETNOP, need 2 arguments [%s]\n",zearg);
				}
				break;

			default: 
				MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETNOP, see documentation about this directive\n",opcode[idx]);
		}
		idx++;
	}
	MemFree(opref);
	if (opcode) MemFree(opcode);
	return tick;
}
int __GETTICK(struct s_assenv *ae,char *oplist, int didx)
{
	#undef FUNC
	#define FUNC "__GETTICK"

	int idx=0,crc,tick=0;
	char **opcode=NULL;
	char *opref;

	/* upper case */
	while (oplist[idx]) {
		oplist[idx]=toupper(oplist[idx]);
		idx++;
	}
	/* duplicata */
	opref=TxtStrDup(oplist);
	/* clean-up */
	TxtReplace(opref,"\t"," ",0);
	TxtReplace(opref,"  "," ",1);
	TxtReplace(opref,": ",":",1);
	/* simplify extended registers to XL or IX */
	TxtReplace(opref,"IY","IX",0);
	TxtReplace(opref,"IXL","XL",0);
	TxtReplace(opref,"IXH","XL",0);
	TxtReplace(opref,"LX","XL",0);
	TxtReplace(opref,"HX","XL",0);
	TxtReplace(opref,"LY","XL",0);
	TxtReplace(opref,"HY","XL",0);
	TxtReplace(opref,"YL","XL",0);
	TxtReplace(opref,"XH","XL",0);
	TxtReplace(opref,"YH","XL",0);

	/* count opcodes */
	opcode=TxtSplitWithChar(opref,':');

	idx=0;
	while (opcode[idx]) {
		char *zeopcode,*terminator,*zearg=NULL;
		char **listarg;

		zeopcode=opcode[idx];
		/* trim */
		while (*zeopcode==' ') zeopcode++;
		terminator=zeopcode;
		while (*terminator!=0 && *terminator!=' ') terminator++;
		if (*terminator) {
			zearg=terminator+1;
			*terminator=0;
			/* no space in args */
			TxtReplace(zearg," ","",1);
		}
		if (!zeopcode[0]) {idx++;continue;}
		crc=GetCRC(zeopcode);

		/*************************************
		* very simple and simplified parsing *
		*************************************/
		switch (crc) {
			case CRC_RLA:
			case CRC_RLCA:
			case CRC_RRCA:
			case CRC_RRA:
			case CRC_NOP:
			case CRC_CCF:
			case CRC_DAA:
			case CRC_SCF:
			case CRC_CPL:
			case CRC_EXX:
			case CRC_EI:
			case CRC_DI:tick+=4;break;

			case CRC_IM:
			case CRC_NEG:tick+=8;break;

			case CRC_RST:tick+=11;break;

			case CRC_RETN:
			case CRC_RETI:tick+=14;break;

			case CRC_CPIR:
			case CRC_CPDR:
			case CRC_CPD:
			case CRC_CPI:
			case CRC_OUTI:
			case CRC_OUTD:
			case CRC_LDD:
			case CRC_LDI:
			case CRC_LDIR:
			case CRC_LDDR:
			case CRC_INIR:
			case CRC_INDR:
			case CRC_OTIR:
			case CRC_OTDR:
			case CRC_IND:
			case CRC_INI:tick+=16;break;

			case CRC_RLD:
			case CRC_RRD:tick+=18;break;

			case CRC_EX:
				if (zearg) {
					if (strstr(zearg,"AF") || strstr(zearg,"DE")) tick+=4; else
					if (strstr(zearg,"(SP)") && strstr(zearg,"HL")) tick+=19; else
					if (strstr(zearg,"(SP)") && strstr(zearg,"IX")) tick+=23;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETTICK, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_PUSH:
				if (zearg) {
					if (strcmp(zearg,"IX")==0) tick+=15; else tick+=11;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETTICK, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_POP:
				if (zearg) {
					if (strcmp(zearg,"IX")==0) tick+=14; else tick+=10;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETTICK, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_SLA:
			case CRC_SLL:
			case CRC_SRA:
			case CRC_SRL:
			case CRC_RL:
			case CRC_RLC:
			case CRC_RR:
			case CRC_RRC:
				if (zearg) {
					if (strstr(zearg,"(HL)")) tick+=15; else
					if (strstr(zearg,"(IX")) tick+=23; else
						tick+=8;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETTICK, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_OUT:
				if (zearg) {
					if (strstr(zearg,"(C),")) tick+=12; else tick+=11;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETTICK, see documentation about this directive\n",opcode[idx]);
				}
				break;
			case CRC_IN:
				if (zearg) {
					if (strstr(zearg,"(C)")) tick+=12; else tick+=11;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETTICK, see documentation about this directive\n",opcode[idx]);
				}
				break;

			case CRC_ADD:
			     if (zearg) {
					/* simplify deprecated notation */
					TxtReplace(zearg,"A,","",0);
					if (strcmp(zearg,"IX,BC")==0 || strcmp(zearg,"IX,DE")==0 || strcmp(zearg,"IX,IX")==0 || strcmp(zearg,"IX,SP")==0) tick+=15; else
					if (strcmp(zearg,"HL,BC")==0 || strcmp(zearg,"HL,DE")==0 || strcmp(zearg,"HL,HL")==0 || strcmp(zearg,"HL,SP")==0) tick+=11; else
					if (strstr(zearg,"(HL)")) tick+=7; else
					if (strstr(zearg,"(IX")) tick+=19; else
					if (strstr(zearg,"XL")) tick+=8; else
					if ((*zearg>='A' && *zearg<='E') || *zearg=='H' || *zearg=='L') tick+=4; else tick+=7;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETTICK, see documentation about this directive\n",opcode[idx]);
				}
				break;

			/* ADC/SBC/SUB/XOR/AND/OR */
			case CRC_ADC:
			case CRC_SBC:
				if (zearg) {
					/* simplify deprecated notation */
					TxtReplace(zearg,"A,","",0);
					if (strcmp(zearg,"HL,BC")==0 || strcmp(zearg,"HL,DE")==0 ||strcmp(zearg,"HL,HL")==0 ||strcmp(zearg,"HL,SP")==0) {tick+=15;break;}
				}
			case CRC_SUB:
			     if (zearg) {
					/* simplify deprecated notation */
					TxtReplace(zearg,"A,","",0);
				}
			case CRC_XOR:
			case CRC_AND:
			case CRC_OR:
			case CRC_CP:
			     if (zearg) {
					if (strstr(zearg,"(HL)")) tick+=7; else
					if (strstr(zearg,"(IX")) tick+=19; else
					if (strstr(zearg,"XL")) tick+=8; else
					if ((*zearg>='A' && *zearg<='E') || *zearg=='H' || *zearg=='L') tick+=4; else tick+=7;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETTICK, see documentation about this directive\n",opcode[idx]);
				}
				break;

			/* BIT/RES/SET */
			case CRC_BIT:
				if (strstr(zearg,"(HL)")) tick+=12; else
				if (strstr(zearg,"(IX")) tick+=20; else tick+=8;
				break;
			case CRC_RES:
			case CRC_SET:
				if (strstr(zearg,"(HL)")) tick+=15; else
				if (strstr(zearg,"(IX")) tick+=23; else tick+=8;
				break;
			case CRC_DEC:
			case CRC_INC:
				if (strcmp(zearg,"XL")==0) tick+=8;
				else if (strcmp(zearg,"SP")==0 || strcmp(zearg,"BC")==0 || strcmp(zearg,"DE")==0 || strcmp(zearg,"HL")==0) tick+=6;
				else if (strcmp(zearg,"IX")==0) tick+=10;
				else if (strcmp(zearg,"(HL)")==0) tick+=11;
				else if (strncmp(zearg,"(IX",3)==0) tick+=23;
				else tick+=4;
				break;
			case CRC_JP:
				// JP is supposed to loop!
				if (zearg) {
					if (strcmp(zearg,"(IX)")==0)
						tick+=8;
					else if (strcmp(zearg,"(HL)")==0)
						tick+=4;
					else tick+=10;
				} else tick+=10;
				break;
			case CRC_DJNZ:
				// DJNZ is supposed to loop!
				tick+=13;
				break;
			case CRC_JR:
				// JR is supposed to loop!
				tick+=12;
				break;
			case CRC_RET:
				// conditionnal RET shorter because it's supposed to be the exit!
				if (!zearg) tick+=10; else tick+=5;
				break;

			case CRC_LD:
				/* big cake! */
				if (zearg && strchr(zearg,',')) {
					int crc1,crc2;

					/* split args */
					listarg=TxtSplitWithChar(zearg,',');
					crc1=GetCRC(listarg[0]);
					crc2=GetCRC(listarg[1]);

					switch (crc1) {
						case CRC_I:
						case CRC_R:
							switch (crc2) {
								case CRC_A:
									tick+=9;
									break;
								default:
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETTICK, see documentation\n",listarg[0],listarg[1]);
							}
							break;
						case CRC_A:
						case CRC_B:
						case CRC_C:
						case CRC_D:
						case CRC_E:
						case CRC_H:
						case CRC_L:
							switch (crc2) {
								case CRC_A:
								case CRC_B:
								case CRC_C:
								case CRC_D:
								case CRC_E:
								case CRC_H:
								case CRC_L:
									tick+=4;
									break;
								case CRC_I:
								case CRC_R:
									if (crc1==CRC_A) tick+=9; else
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETTICK, see documentation\n",listarg[0],listarg[1]);
									break;
								case CRC_MBC:
								case CRC_MDE:
									if (crc1!=CRC_A) {
										MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETTICK, see documentation\n",listarg[0],listarg[1]);
										break;
									}
								case CRC_MHL:
									tick+=7;
									break;
								case CRC_XL:
									tick+=8;
									break;
								default:
									/* MIX + memory + value */
									if (strncmp(listarg[1],"(IX",3)==0) {
										tick+=19;
									} else if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') {
										/* memory */
										if (crc1==CRC_A) {
										tick+=13;
										} else {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETTICK, see documentation\n",listarg[0],listarg[1]);
										}
									} else {
										/* numeric value as default */
										tick+=7;
									}
							}
							break;

						case CRC_XL:
							switch (crc2) {
								case CRC_A:
								case CRC_B:
								case CRC_C:
								case CRC_D:
								case CRC_E:
								case CRC_H:
								case CRC_L:
								case CRC_XL:
									tick+=8;
									break;
								default:
									/* value */
									tick+=11;
							}
							break;

						case CRC_BC:
						case CRC_DE:
							/* memory / value */
							if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') tick+=20; else tick+=10;
							break;
						case CRC_HL:
							/* memory / value */
							if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') tick+=16; else tick+=10;
							break;
						case CRC_SP:
							if (crc2==CRC_HL) {
								tick+=6;
							} else if (crc2==CRC_IX) {
								/* IX */
								tick+=10;
							} else if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') {
								/* memory */
								tick+=20;
							} else tick+=10;
							break;
						case CRC_IX:
							/* memory / value */
							if (listarg[1][0]=='(' && listarg[1][strlen(listarg[1])-1]==')') tick+=20; else tick+=14;
							break;

						case CRC_MBC:
						case CRC_MDE:
							if (crc2==CRC_A) {
								tick+=7;
							} else {
								MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETTICK, see documentation\n",listarg[0],listarg[1]);
							}
							break;
						case CRC_MHL:
							switch (crc2) {
								case CRC_A:
								case CRC_B:
								case CRC_C:
								case CRC_D:
								case CRC_E:
								case CRC_H:
								case CRC_L:
									tick+=7;
									break;
								default:
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETTICK, see documentation\n",listarg[0],listarg[1]);
							}
							break;
						default:
							if (strncmp(listarg[0],"(IX",3)==0) {
								/* MIX */
								switch (crc2) {
									case CRC_A:
									case CRC_B:
									case CRC_C:
									case CRC_D:
									case CRC_E:
									case CRC_H:
									case CRC_L:tick+=19;break;
									default:tick+=23;
								}
							} else if (listarg[0][0]=='(' && listarg[0][strlen(listarg[0])-1]==')') {
								/* memory */
								switch (crc2) {
									case CRC_A:tick+=13;break;
									case CRC_HL:tick+=16;break;
									case CRC_BC:
									case CRC_DE:
									case CRC_SP:
									case CRC_IX:tick+=20;break;
									default:
										MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETTICK, see documentation\n",listarg[0],listarg[1]);
								}
							} else {
								MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported LD %s,%s for GETTICK, see documentation\n",listarg[0],listarg[1]);
							}
					}
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode LD for GETTICK, need 2 arguments [%s]\n",zearg);
				}
				break;

			default: 
				MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"unsupported opcode [%s] for GETTICK, see documentation about this directive\n",opcode[idx]);
		}
		idx++;
	}
	MemFree(opref);
	if (opcode) MemFree(opcode);
	return tick;
}
/*
	default returned value of Duration is NOP
	but BUILDZX usage change this to ticks!
*/
int __DURATION(struct s_assenv *ae,char *opcode, int didx)
{
	#undef FUNC
	#define FUNC "__DURATION"

	if (!ae->forcezx) return __GETNOP(ae,opcode,didx);
	return __GETTICK(ae,opcode,didx);
}

int __Soft2HardInk(struct s_assenv *ae,int soft, int didx) {
	switch (soft) {
		case 0:return 64+20;break;
		case 1:return 64+4 ;break;
		case 2:return 64+21 ;break;
		case 3:return 64+28 ;break;
		case 4:return 64+24 ;break;
		case 5:return 64+29 ;break;
		case 6:return 64+12 ;break;
		case 7:return 64+5 ;break;
		case 8:return 64+13 ;break;
		case 9:return 64+22 ;break;
		case 10:return 64+6 ;break;
		case 11:return 64+23 ;break;
		case 12:return 64+30 ;break;
		case 13:return 64+0 ;break;
		case 14:return 64+31 ;break;
		case 15:return 64+14 ;break;
		case 16:return 64+7 ;break;
		case 17:return 64+15 ;break;
		case 18:return 64+18 ;break;
		case 19:return 64+2 ;break;
		case 20:return 64+19 ;break;
		case 21:return 64+26 ;break;
		case 22:return 64+25 ;break;
		case 23:return 64+27 ;break;
		case 24:return 64+10 ;break;
		case 25:return 64+3 ;break;
		case 26:return 64+11 ;break;
		default:
			MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"SOFT2HARD_INK needs 0-26 color index");
	}
	return 0;
}
int __Hard2SoftInk(struct s_assenv *ae,int hard, int didx) {
	hard&=31;
	switch (hard) {
		case 0:return 13;break;
		case 1:return 13;break;
		case 2:return 19;break;
		case 3:return 25;break;
		case 4:return 1;break;
		case 5:return 7;break;
		case 6:return 10;break;
		case 7:return 16;break;
		case 8:return 7;break;
		case 9:return 25;break;
		case 10:return 24;break;
		case 11:return 26;break;
		case 12:return 6;break;
		case 13:return 8;break;
		case 14:return 15;break;
		case 15:return 17;break;
		case 16:return 1;break;
		case 17:return 19;break;
		case 18:return 18;break;
		case 19:return 20;break;
		case 20:return 0;break;
		case 21:return 2;break;
		case 22:return 9;break;
		case 23:return 11;break;
		case 24:return 4;break;
		case 25:return 22;break;
		case 26:return 21;break;
		case 27:return 23;break;
		case 28:return 3;break;
		case 29:return 5;break;
		case 30:return 12;break;
		case 31:return 14;break;
		default:/*warning remover*/
			MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"SOFT2HARD_INK warning remover");
	}
	return 0;
}

double ComputeExpressionCore(struct s_assenv *ae,char *original_zeexpression,int ptr, int didx)
{
	#undef FUNC
	#define FUNC "ComputeExpressionCore"

	/* static execution buffers */
	static double *accu=NULL;
	static int maccu=0;
	static struct s_compute_element *computestack=NULL;
	static int maxcomputestack=0;
	int i,j,paccu=0;
	int nbtokenstack=0;
	int nbcomputestack=0;
	int nboperatorstack=0;

	struct s_compute_element stackelement;
	int o2,okclose,itoken;
	
	int idx=0,crc,icheck,is_binary,ivar=0;
	char asciivalue[11];
	unsigned char c;
	int accu_err=0;
	/* backup alias replace */
	char *zeexpression,*expr;
	int original=1;
	int ialias,startvar=0;
	int newlen,lenw;
	/* dictionnary */
	struct s_expr_dico *curdic;
	struct s_label *curlabel;
	int minusptr,imkey,bank,page;
	double curval;
	int is_string=0;
	/* negative value */
	int allow_minus_as_sign=0;
	/* extended replace in labels */
	int curly=0,curlyflag=0;
	char *Automate;
	double dummint;

	/* memory cleanup */
	if (!ae) {
		if (maccu) MemFree(accu);
		accu=NULL;maccu=0;
		if (maxcomputestack) MemFree(computestack);
		computestack=NULL;maxcomputestack=0;
		return 0.0;
	}

	/* be sure to have at least some bytes allocated */
	StateMachineResizeBuffer(&ae->computectx->varbuffer,128,&ae->computectx->maxivar);


#if TRACE_COMPUTE_EXPRESSION
	printf("expression=[%s]\n",zeexpression);
#endif
	zeexpression=original_zeexpression;
	if (!zeexpression[0]) {
		return 0;
	}
	/* double hack if the first value is negative */
	if (zeexpression[0]=='-') {
		if (ae->AutomateExpressionValidCharFirst[(int)zeexpression[1]&0xFF]) {
			allow_minus_as_sign=1;
		} else {
			memset(&stackelement,0,sizeof(stackelement));
			ObjectArrayAddDynamicValueConcat((void **)&ae->computectx->tokenstack,&nbtokenstack,&ae->computectx->maxtokenstack,&stackelement,sizeof(stackelement));
		}
	}

	/* is there ascii char? */
	while ((c=zeexpression[idx])!=0) {
		if (c=='\'' || c=='"') {
			/* echappement */
			if (zeexpression[idx+1]=='\\') {
				if (zeexpression[idx+2] && zeexpression[idx+3]==c) {
					sprintf(asciivalue,"#%03X",zeexpression[idx+2]);
					memcpy(zeexpression+idx,asciivalue,4);
					idx+=3;
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"Only single escaped char may be quoted [%s]\n",TradExpression(zeexpression));
					zeexpression[0]=0;
					return 0;
				}
			} else if (zeexpression[idx+1] && zeexpression[idx+2]==c) {
					sprintf(asciivalue,"#%02X",zeexpression[idx+1]);
					memcpy(zeexpression+idx,asciivalue,3);
					idx+=2;
			} else {
				//printf("Expression with => moar than one char in quotes\n");
				//MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"Only single char may be quoted [%s]\n",TradExpression(zeexpression));
				//zeexpression[0]=0;
				//return 0;
				idx++;
				while (zeexpression[idx] && zeexpression[idx]!=c) idx++; // no escape code management

			}
		}
		
		idx++;
	}
#if TRACE_COMPUTE_EXPRESSION
	printf("apres conversion des chars [%s]\n",zeexpression);
#endif
	/***********************************************************
	    P A T C H    F O R    P O S I T I V E     V A L U E    
	***********************************************************/
	if (zeexpression[0]=='+') idx=1; else idx=0;
	/***********************************************************
	  C O M P U T E   E X P R E S S I O N   M A I N    L O O P
	***********************************************************/
	while ((c=zeexpression[idx])!=0) {
		switch (c) {
			case '"':
			case '\'':
				//printf("COMPUTE => string detected!\n");
				ivar=0;
				idx++;
				while (zeexpression[idx] && zeexpression[idx]!=c) {
					ae->computectx->varbuffer[ivar++]=zeexpression[idx];
					StateMachineResizeBuffer(&ae->computectx->varbuffer,ivar,&ae->computectx->maxivar);
					idx++;
				}
				if (zeexpression[idx]) idx++; else MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"ComputeExpression [%s] quote bug!\n",TradExpression(zeexpression));
				ae->computectx->varbuffer[ivar]=0;
				is_string=1; // donc on ira jamais utiliser startvar derriere
				break;

			/* parenthesis */
			case ')':
				/* next to a closing parenthesis, a minus is an operator */
				allow_minus_as_sign=0;
				break;
			case '(':
			/* operator detection */
			case '*':
			case '/':
			case '^':
			case '[':
			case 'm':
			case '+':
			case ']':
				allow_minus_as_sign=1;
				break;
			case '&':
				allow_minus_as_sign=1;
				if (c=='&' && zeexpression[idx+1]=='&') {
					idx++;
					c='a'; // boolean AND
				}
				break;
			case '|':
				allow_minus_as_sign=1;
				if (c=='|' && zeexpression[idx+1]=='|') {
					idx++;
					c='o'; // boolean OR
				}
				break;
			/* testing */
			case '<':
				allow_minus_as_sign=1;
				if (zeexpression[idx+1]=='=') {
					idx++;
					c='k'; // boolean LOWEREQ
				} else if (zeexpression[idx+1]=='>') {
					idx++;
					c='n'; // boolean NOTEQUAL
				} else {
					c='l';
				}
				break;
			case '>':
				allow_minus_as_sign=1;
				if (zeexpression[idx+1]=='=') {
					idx++;
					c='h'; // boolean GREATEREQ
				} else {
					c='g';
				}
				break;
			case '!':
				allow_minus_as_sign=1;
				if (zeexpression[idx+1]=='=') {
					idx++;
					c='n'; // boolean NOTEQUAL
				} else {
					c='b';
				}
				break;
			case '=':
				allow_minus_as_sign=1;
				/* expecting == */
				if (zeexpression[idx+1]=='=') {
					idx++;
					c='e'; // boolean EQUAL
				/* except in maxam mode with a single = */
				} else if (ae->maxam) {
					c='e'; // boolean EQUAL
				/* cannot affect data inside an expression */
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] cannot set variable inside an expression\n",TradExpression(zeexpression));
					return 0;
				}
				break;
			case '-':
				if (allow_minus_as_sign) {
					/* previous char was an opening parenthesis or an operator */
					ivar=0;
					ae->computectx->varbuffer[ivar++]='-';
					StateMachineResizeBuffer(&ae->computectx->varbuffer,ivar,&ae->computectx->maxivar);
					c=zeexpression[++idx];
					if (ae->AutomateExpressionValidCharFirst[(int)c&0xFF]) {
						ae->computectx->varbuffer[ivar++]=c;
						StateMachineResizeBuffer(&ae->computectx->varbuffer,ivar,&ae->computectx->maxivar);
						c=zeexpression[++idx];
						while (ae->AutomateExpressionValidChar[(int)c&0xFF]) {
							ae->computectx->varbuffer[ivar++]=c;
							StateMachineResizeBuffer(&ae->computectx->varbuffer,ivar,&ae->computectx->maxivar);
							c=zeexpression[++idx];
						}
					}
					ae->computectx->varbuffer[ivar]=0;
					if (ivar<2) {
						MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] invalid minus sign\n",TradExpression(zeexpression));
						if (!original) {
							MemFree(zeexpression);
						}
						return 0;
					}
					break;
				}
				allow_minus_as_sign=1;
				break;
				
			/* operator OR binary value */
			case '%':
				/* % symbol may be a modulo or a binary literal value */
				is_binary=0;
				for (icheck=1;zeexpression[idx+icheck];icheck++) {
					switch (zeexpression[idx+icheck]) {
						case '1':
						case '0':/* still binary */
							is_binary=1;
							break;
						case '+':
						case '-':
						case '/':
						case '*':
						case '|':
						case 'm':
						case '%':
						case '^':
						case '&':
						case '(':
						case ')':
						case '=':
						case '<':
						case '>':
						case '!':
						case '[':
						case ']':
							if (is_binary) is_binary=2; else is_binary=-1;
							break;
						default:
							is_binary=-1;
					}
					if (is_binary==2) {
						break;
					}
					if (is_binary==-1) {
						is_binary=0;
						break;
					}
				}
				if (!is_binary) {
					allow_minus_as_sign=1;
					c='m';
					break;
				}
			default:
				allow_minus_as_sign=0;
				/* semantic analysis */
				startvar=idx;
				ivar=0;
				/* first char does not allow same chars as next chars */
				if (ae->AutomateExpressionValidCharFirst[((int)c)&0xFF]) {
					ae->computectx->varbuffer[ivar++]=c;
					if (c=='{') {
						/* not a formula but only a prefix tag */
						curly++;
					}
					StateMachineResizeBuffer(&ae->computectx->varbuffer,ivar,&ae->computectx->maxivar);
					idx++;
					c=zeexpression[idx];
					Automate=ae->AutomateExpressionValidChar;
					while (Automate[((int)c)&0xFF]) {
						if (c=='{') {
							curly++;
							curlyflag=1;
							Automate=ae->AutomateExpressionValidCharExtended;
						} else if (c=='}') {
							curly--;
							if (!curly) {
								Automate=ae->AutomateExpressionValidChar;
							}
						}
						ae->computectx->varbuffer[ivar++]=c;
						StateMachineResizeBuffer(&ae->computectx->varbuffer,ivar,&ae->computectx->maxivar);
						idx++;
						c=zeexpression[idx];
					}
				}
				ae->computectx->varbuffer[ivar]=0;
				if (!ivar) {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"ComputeExpression invalid char (%d=%c) expression [%s]\n",c,c>31?c:' ',TradExpression(zeexpression));
					if (!original) {
						MemFree(zeexpression);
					}
					return 0;
				} else if (curly) {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"ComputeExpression wrong curly brackets in expression [%s]\n",TradExpression(zeexpression));
					if (!original) {
						MemFree(zeexpression);
					}
					return 0;
				}
		}
		if (c && !ivar) idx++;
	
		/************************************
		   S T A C K   D I S P A T C H E R
		************************************/
		/* push operator or stack value */
		if (!ivar) {
			/************************************
			          O P E R A T O R 
			************************************/
			stackelement=ae->AutomateElement[c];
			if (stackelement.operator>E_COMPUTE_OPERATION_GREATEREQ) {
				MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] has unknown operator %c (%d)\n",TradExpression(zeexpression),c>31?c:'.',c);
			}
			/* stackelement.value isn't used */
			stackelement.string=NULL;
		} else if (is_string) {
			stackelement.operator=E_COMPUTE_OPERATION_PUSH_DATASTC;
			/* priority & value isn't used */
			stackelement.string=TxtStrDup(ae->computectx->varbuffer);
			allow_minus_as_sign=0;
			ivar=is_string=0;
		} else {
			/************************************
			              V A L U E
			************************************/
#if TRACE_COMPUTE_EXPRESSION
	printf("value [%s]\n",ae->computectx->varbuffer);
#endif
			if (ae->computectx->varbuffer[0]=='-') minusptr=1; else minusptr=0;
			/* constantes ou variables/labels */
			switch (ae->computectx->varbuffer[minusptr]) {
				case '0':
					/* 0x hexa value hack */
					if (ae->computectx->varbuffer[minusptr+1]=='X' && ae->AutomateHexa[(int)ae->computectx->varbuffer[minusptr+2]&0xFF]) {
						for (icheck=minusptr+3;ae->computectx->varbuffer[icheck];icheck++) {
							if (ae->AutomateHexa[(int)ae->computectx->varbuffer[icheck]&0xFF]) continue;
							MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid hex number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
							break;
						}
						curval=strtol(ae->computectx->varbuffer+minusptr+2,NULL,16);
						break;
					} else
					/* 0b binary value hack */
					if (ae->computectx->varbuffer[minusptr+1]=='B' && (ae->computectx->varbuffer[minusptr+2]>='0' && ae->computectx->varbuffer[minusptr+2]<='1')) {
						for (icheck=minusptr+3;ae->computectx->varbuffer[icheck];icheck++) {
							if (ae->computectx->varbuffer[icheck]>='0' && ae->computectx->varbuffer[icheck]<='1') continue;
							MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid binary number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
							break;
						}
						curval=strtol(ae->computectx->varbuffer+minusptr+2,NULL,2);
						break;
					}
					/* 0o octal value hack */
					if (ae->computectx->varbuffer[minusptr+1]=='O' && (ae->computectx->varbuffer[minusptr+2]>='0' && ae->computectx->varbuffer[minusptr+2]<='5')) {
						for (icheck=minusptr+3;ae->computectx->varbuffer[icheck];icheck++) {
							if (ae->computectx->varbuffer[icheck]>='0' && ae->computectx->varbuffer[icheck]<='5') continue;
							MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid octal number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
							break;
						}
						curval=strtol(ae->computectx->varbuffer+minusptr+2,NULL,2);
						break;
					}
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					/* check number */
					for (icheck=minusptr;ae->computectx->varbuffer[icheck];icheck++) {
						if (ae->AutomateDigit[(int)ae->computectx->varbuffer[icheck]&0xFF]) continue;
						/* Intel hexa & binary style */
						switch (ae->computectx->varbuffer[strlen(ae->computectx->varbuffer)-1]) {
							case 'H':
								for (icheck=minusptr;ae->computectx->varbuffer[icheck+1];icheck++) {
									if (ae->AutomateHexa[(int)ae->computectx->varbuffer[icheck]&0xFF]) continue;
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid hex number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
								}
								curval=strtol(ae->computectx->varbuffer+minusptr,NULL,16);
								break;
							case 'B':
								for (icheck=minusptr;ae->computectx->varbuffer[icheck+1];icheck++) {
									if (ae->computectx->varbuffer[icheck]=='0' || ae->computectx->varbuffer[icheck]=='1') continue;
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid binary number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
								}
								curval=strtol(ae->computectx->varbuffer+minusptr,NULL,2);
								break;
							default:
								MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
						}
						icheck=0;
						break;
					}
					if (!ae->computectx->varbuffer[icheck]) curval=atof(ae->computectx->varbuffer+minusptr);
					break;
				case '%':
					/* check number */
					if (!ae->computectx->varbuffer[minusptr+1]) {
						MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is an empty binary number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
					}
					for (icheck=minusptr+1;ae->computectx->varbuffer[icheck];icheck++) {
						if (ae->computectx->varbuffer[icheck]=='0' || ae->computectx->varbuffer[icheck]=='1') continue;
						MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid binary number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
						break;
					}
					curval=strtol(ae->computectx->varbuffer+minusptr+1,NULL,2);
					break;
				case '#':
					/* check number */
					if (!ae->computectx->varbuffer[minusptr+1]) {
						MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is an empty hex number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
					}
					for (icheck=minusptr+1;ae->computectx->varbuffer[icheck];icheck++) {
						if (ae->AutomateHexa[(int)ae->computectx->varbuffer[icheck]&0xFF]) continue;
						MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid hex number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
						break;
					}
					curval=strtol(ae->computectx->varbuffer+minusptr+1,NULL,16);
					break;
				default:
					if (1 || !curlyflag) {
						/* $ hex value hack */
						if (ae->computectx->varbuffer[minusptr+0]=='$' && ae->AutomateHexa[(int)ae->computectx->varbuffer[minusptr+1]&0xFF]) {
							for (icheck=minusptr+2;ae->computectx->varbuffer[icheck];icheck++) {
								if (ae->AutomateHexa[(int)ae->computectx->varbuffer[icheck]&0xFF]) continue;
								MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid hex number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
								break;
							}
							curval=strtol(ae->computectx->varbuffer+minusptr+1,NULL,16);
							break;
						}
						/* @ octal value hack */
						if (ae->computectx->varbuffer[minusptr+0]=='@' &&  ((ae->computectx->varbuffer[minusptr+1]>='0' && ae->computectx->varbuffer[minusptr+1]<='7'))) {
							for (icheck=minusptr+2;ae->computectx->varbuffer[icheck];icheck++) {
								if (ae->computectx->varbuffer[icheck]>='0' && ae->computectx->varbuffer[icheck]<='7') continue;
								MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is not a valid octal number\n",TradExpression(zeexpression),ae->computectx->varbuffer);
								break;
							}
							curval=strtol(ae->computectx->varbuffer+minusptr+1,NULL,8);
							break;
						}
						/* Intel hexa value hack */
						if (ae->AutomateHexa[(int)ae->computectx->varbuffer[minusptr+0]&0xFF]) {
							if (ae->computectx->varbuffer[strlen(ae->computectx->varbuffer)-1]=='H') {
								for (icheck=minusptr;ae->computectx->varbuffer[icheck+1];icheck++) {
									if (!ae->AutomateHexa[(int)ae->computectx->varbuffer[icheck]&0xFF]) break;
								}
								if (!ae->computectx->varbuffer[icheck+1]) {
									curval=strtol(ae->computectx->varbuffer+minusptr,NULL,16);
									break;
								}
							}
						}
					}
					
					
					if (curlyflag) {
						char *minivarbuffer;
						int touched;

						/* besoin d'un sous-contexte */
						minivarbuffer=TxtStrDup(ae->computectx->varbuffer+minusptr);
						ae->computectx=&ae->ctx2;
#if TRACE_COMPUTE_EXPRESSION
	printf("curly [%s]\n",minivarbuffer);
#endif
						minivarbuffer=TranslateTag(ae,minivarbuffer, &touched,0,E_TAGOPTION_NONE);
#if TRACE_COMPUTE_EXPRESSION
	printf("après curly [%s]\n",minivarbuffer);
#endif
						ae->computectx=&ae->ctx1;
						if (!touched) {
							strcpy(ae->computectx->varbuffer+minusptr,minivarbuffer);
						} else {
							StateMachineResizeBuffer(&ae->computectx->varbuffer,strlen(minivarbuffer)+2,&ae->computectx->maxivar);
							strcpy(ae->computectx->varbuffer+minusptr,minivarbuffer);
						}
						MemFree(minivarbuffer);
						curlyflag=0;
					}

					crc=GetCRC(ae->computectx->varbuffer+minusptr);
					/***************************************************
					     L O O K I N G   F O R   A   F U N C T I O N
					***************************************************/
					for (imkey=0;math_keyword[imkey].mnemo[0];imkey++) {
						if (crc==math_keyword[imkey].crc && strcmp(ae->computectx->varbuffer+minusptr,math_keyword[imkey].mnemo)==0) {
							if (c=='(') {
								/* push function as operator! */
								stackelement.operator=math_keyword[imkey].operation;
								stackelement.string=NULL;
								/************************************************
								      C R E A T E    E X T R A     T O K E N
								************************************************/
								ObjectArrayAddDynamicValueConcat((void **)&ae->computectx->tokenstack,&nbtokenstack,&ae->computectx->maxtokenstack,&stackelement,sizeof(stackelement));
								stackelement.operator=E_COMPUTE_OPERATION_OPEN;
								ObjectArrayAddDynamicValueConcat((void **)&ae->computectx->tokenstack,&nbtokenstack,&ae->computectx->maxtokenstack,&stackelement,sizeof(stackelement));
								allow_minus_as_sign=1;
								idx++;
							} else {
								MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is a reserved keyword!\n",TradExpression(zeexpression),math_keyword[imkey].mnemo);
								curval=0;
								idx++;
							}
							ivar=0;
							break;
						}
					}
					if (math_keyword[imkey].mnemo[0]) continue;
					
					if (ae->computectx->varbuffer[minusptr+0]=='$' && ae->computectx->varbuffer[minusptr+1]==0) {
						curval=ptr;
					} else {
#if TRACE_COMPUTE_EXPRESSION
	printf("search dico [%s]\n",ae->computectx->varbuffer+minusptr);
#endif
						curdic=SearchDico(ae,ae->computectx->varbuffer+minusptr,crc);
						if (curdic) {
#if TRACE_COMPUTE_EXPRESSION
	printf("trouvé valeur=%.2lf\n",curdic->v);
#endif
							curval=curdic->v;
							break;
						} else {
							/* getbank hack */
							if (ae->computectx->varbuffer[minusptr]!='{') {
								bank=0;
								page=0;
							} else if (strncmp(ae->computectx->varbuffer+minusptr,"{BANK}",6)==0) {
								bank=6;
								page=0;
								/* obligé de recalculer le CRC */
								crc=GetCRC(ae->computectx->varbuffer+minusptr+bank);
							} else if (strncmp(ae->computectx->varbuffer+minusptr,"{PAGE}",6)==0) {
								bank=6;
								page=1;
								/* obligé de recalculer le CRC */
								crc=GetCRC(ae->computectx->varbuffer+minusptr+bank);
							} else if (strncmp(ae->computectx->varbuffer+minusptr,"{PAGESET}",9)==0) {
								bank=9;
								page=2;
								/* obligé de recalculer le CRC */
								crc=GetCRC(ae->computectx->varbuffer+minusptr+bank);
							} else if (strncmp(ae->computectx->varbuffer+minusptr,"{SIZEOF}",8)==0) {
								bank=8;
								page=3;
								/* obligé de recalculer le CRC */
								crc=GetCRC(ae->computectx->varbuffer+minusptr+bank);
								/* search in structures prototypes and subfields */
								for (i=0;i<ae->irasmstruct;i++) {
									if (ae->rasmstruct[i].crc==crc && strcmp(ae->rasmstruct[i].name,ae->computectx->varbuffer+minusptr+bank)==0) {
										curval=ae->rasmstruct[i].size;
										break;
									}

									for (j=0;j<ae->rasmstruct[i].irasmstructfield;j++) {
										if (ae->rasmstruct[i].rasmstructfield[j].crc==crc && strcmp(ae->rasmstruct[i].rasmstructfield[j].fullname,ae->computectx->varbuffer+minusptr+bank)==0) {
											curval=ae->rasmstruct[i].rasmstructfield[j].size;
											i=ae->irasmstruct+1;
											break;
										}
									}
								}

								if (i==ae->irasmstruct) {
									/* search in structures aliases */
									for (i=0;i<ae->irasmstructalias;i++) {
										if (ae->rasmstructalias[i].crc==crc && strcmp(ae->rasmstructalias[i].name,ae->computectx->varbuffer+minusptr+bank)==0) {
											curval=ae->rasmstructalias[i].size+ae->rasmstructalias[i].ptr;
											break;
										}
									}
									if (i==ae->irasmstructalias) {
										MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"cannot SIZEOF unknown structure [%s]!\n",ae->computectx->varbuffer+minusptr+bank);
										curval=0;
									}
								}
							} else {
								MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] - %s is an unknown prefix!\n",TradExpression(zeexpression),ae->computectx->varbuffer);
								bank=0; // on pourrait sauter le tag pour eviter la merde a suivre
								page=0;
							}
							/* limited label translation while processing crunched blocks
							   ae->curlz == current crunched block processed
							   expression->crunch_block=0 -> oui
							   expression->crunch_block=1 -> oui si même block
							   expression->crunch_block=2 -> non car sera relogée
							*/
							if (page!=3) {



if (didx>0 && didx<ae->ie) {
	if (ae->expression[didx].module) {
		char *dblvarbuffer;
#if TRACE_LABEL || TRACE_COMPUTE_EXPRESSION
		printf("search label [%s] in an expression / module=[%s]\n",ae->computectx->varbuffer+minusptr+bank,ae->expression[didx].module);
#endif
		dblvarbuffer=MemMalloc(strlen(ae->computectx->varbuffer)+strlen(ae->expression[didx].module)+2);

		strcpy(dblvarbuffer,ae->expression[didx].module);
		strcat(dblvarbuffer,ae->module_separator);
		strcat(dblvarbuffer,ae->computectx->varbuffer+minusptr+bank);

		/* on essaie toujours de trouver le label du module courant */	
		curlabel=SearchLabel(ae,dblvarbuffer,GetCRC(dblvarbuffer));
		MemFree(dblvarbuffer);
	} else {
#if TRACE_LABEL || TRACE_COMPUTE_EXPRESSION
		printf("search label [%s] in an expression without module\n",ae->computectx->varbuffer+minusptr+bank);
#endif
		curlabel=NULL;
	}

	/* pas trouvé on cherche LEGACY */
	if (!curlabel) curlabel=SearchLabel(ae,ae->computectx->varbuffer+minusptr+bank,crc);
#if TRACE_LABEL || TRACE_COMPUTE_EXPRESSION
	else printf("label trouve via ajout du MODULE\n");
#endif

} else {
#if TRACE_LABEL || TRACE_COMPUTE_EXPRESSION
	printf("search label [%s] outside an expression taking current module!\n",ae->computectx->varbuffer+minusptr+bank);
#endif
	if (ae->module) {
		char *dblvarbuffer;
		dblvarbuffer=MemMalloc(strlen(ae->computectx->varbuffer)+strlen(ae->module)+2);
		strcpy(dblvarbuffer,ae->module);
		strcat(dblvarbuffer,ae->module_separator);
		strcat(dblvarbuffer,ae->computectx->varbuffer+minusptr+bank);

		/* on essaie toujours de trouver le label du module courant */	
		curlabel=SearchLabel(ae,dblvarbuffer,GetCRC(dblvarbuffer));
		/* pas trouvé on cherche LEGACY */
		if (!curlabel) curlabel=SearchLabel(ae,ae->computectx->varbuffer+minusptr+bank,crc);
#if TRACE_LABEL || TRACE_COMPUTE_EXPRESSION
		else printf("label trouve via ajout du MODULE\n");
#endif

		MemFree(dblvarbuffer);
	} else {
		curlabel=SearchLabel(ae,ae->computectx->varbuffer+minusptr+bank,crc);
	}
}



								if (curlabel) {
									if (ae->stage<2) {
										if (curlabel->lz==-1) {
											if (!bank) {
												curval=curlabel->ptr;
											} else {
#if TRACE_COMPUTE_EXPRESSION
printf("page=%d | ptr=%X ibank=%d\n",page,curlabel->ptr,curlabel->ibank);
#endif
												switch (page) {
													case 2: /* PAGESET */
														if (curlabel->ibank<BANK_MAX_NUMBER) {
															curval=ae->setgate[curlabel->ibank];
														} else {
															MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] cannot use PAGESET - label [%s] is in a temporary space!\n",TradExpression(zeexpression),ae->computectx->varbuffer);
															curval=curlabel->ibank;
														}
														break;
													case 1:/* PAGE */
														if (curlabel->ibank<BANK_MAX_NUMBER) {
															/* 4M expansion compliant */
															if (ae->bankset[curlabel->ibank>>2]) {
																curval=ae->bankgate[(curlabel->ibank&0x1FC)+(curlabel->ptr>>14)];
															} else {
																curval=ae->bankgate[curlabel->ibank];
															}
														} else {
															MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] cannot use PAGE - label [%s] is in a temporary space!\n",TradExpression(zeexpression),ae->computectx->varbuffer);
															curval=curlabel->ibank;
														}
														break;
													case 0:
														curval=curlabel->ibank;
														break;
													default:MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INTERNAL ERROR (unknown paging)\n",GetExpFile(ae,didx),GetExpLine(ae,didx));FreeAssenv(ae);exit(-664);
												}
											}
										} else {
											/* label MUST be intermediate OR in the crunched block */
											if (ae->lzsection[curlabel->lz].lzversion==0 || (curlabel->iorgzone==ae->expression[didx].iorgzone && curlabel->ibank==ae->expression[didx].ibank && curlabel->lz<=ae->expression[didx].lz)) {
												if (!bank) {
													curval=curlabel->ptr;
												} else {
													if (page) {
														switch (page) {
															case 2:  /* PAGESET */
																if (curlabel->ibank<BANK_MAX_NUMBER) {
																	/* 4M expansion compliant */
																	curval=ae->setgate[curlabel->ibank];
																} else {
																	MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] cannot use PAGESET - label [%s] is in a temporary space!\n",TradExpression(zeexpression),ae->computectx->varbuffer);
																	curval=curlabel->ibank;
																}
																break;
															case 1: /* PAGE */
																if (curlabel->ibank<BANK_MAX_NUMBER) {
																	/* 4M expansion compliant */
																	if (ae->bankset[curlabel->ibank>>2]) {
																		curval=ae->bankgate[(curlabel->ibank&0x1FC)+(curlabel->ptr>>14)];
																	} else {																		
																		curval=ae->bankgate[curlabel->ibank];
																	}
																} else {
																	MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] cannot use PAGE - label [%s] is in a temporary space!\n",TradExpression(zeexpression),ae->computectx->varbuffer);
																	curval=curlabel->ibank;
																}
																break;
															case 0:curval=curlabel->ibank;break;
															default:MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INTERNAL ERROR (unknown paging)\n");FreeAssenv(ae);exit(-664);
														}
													}
												}
											} else {
												MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"Label [%s](%d) cannot be computed because it is located after the crunched zone %d\n",ae->computectx->varbuffer,curlabel->lz,ae->expression[didx].lz);
												curval=0;
											}
										}
									} else {
#if TRACE_COMPUTE_EXPRESSION
printf("stage 2 | page=%d | ptr=%X ibank=%d\n",page,curlabel->ptr,curlabel->ibank);
#endif
										if (bank) {
											//curval=curlabel->ibank;
											switch (page) {
												case 2: /* PAGESET */
													if (curlabel->ibank<BANK_MAX_NUMBER) {
														curval=ae->setgate[curlabel->ibank];
													} else {
														MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] cannot use PAGESET - label [%s] is in a temporary space!\n",TradExpression(zeexpression),ae->computectx->varbuffer);
														curval=curlabel->ibank;
													}
													break;
												case 1:/* PAGE */
													if (curlabel->ibank<BANK_MAX_NUMBER) {
														/* 4M expansion compliant */
														if (ae->bankset[curlabel->ibank>>2]) {
															curval=ae->bankgate[(curlabel->ibank&0x1FC)+(curlabel->ptr>>14)];
														} else {
															curval=ae->bankgate[curlabel->ibank];
														}
													} else {
														MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] cannot use PAGE - label [%s] is in a temporary space!\n",TradExpression(zeexpression),ae->computectx->varbuffer);
														curval=curlabel->ibank;
													}
													break;
												case 0:
													curval=curlabel->ibank;
													break;
												default:MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INTERNAL ERROR (unknown paging)\n",GetExpFile(ae,didx),GetExpLine(ae,didx));FreeAssenv(ae);exit(-664);
											}
										} else {
											curval=curlabel->ptr;
										}
									}
								} else {
									/***********
										to allow aliases declared after use
									***********/
									if ((ialias=SearchAlias(ae,crc,ae->computectx->varbuffer+minusptr))>=0) {
										newlen=ae->alias[ialias].len;
										lenw=strlen(zeexpression);
										if (newlen>ivar) {
											/* realloc bigger */
											if (original) {
												expr=MemMalloc(lenw+newlen-ivar+1);
												memcpy(expr,zeexpression,lenw+1);
												zeexpression=expr;
												original=0;
											} else {
												zeexpression=MemRealloc(zeexpression,lenw+newlen-ivar+1);
											}
										}
										/* startvar? */
										if (newlen!=ivar) {
											MemMove(zeexpression+startvar+newlen,zeexpression+startvar+ivar,lenw-startvar-ivar+1);
										}
										strncpy(zeexpression+startvar,ae->alias[ialias].translation,newlen); /* copy without zero terminator */
										idx=startvar;
										ivar=0;
										continue;
									} else {
										/* index possible sur une struct? */
										int reverse_idx,validx=-1;
										char *structlabel;

										reverse_idx=strlen(ae->computectx->varbuffer)-1;
										if (ae->computectx->varbuffer[reverse_idx]>='0' && ae->computectx->varbuffer[reverse_idx]<='9') {
											/* vu que ça ne PEUT PAS être une valeur litérale, on ne fait pas de test de débordement */
											reverse_idx--;
											while (ae->computectx->varbuffer[reverse_idx]>='0' && ae->computectx->varbuffer[reverse_idx]<='9') {
												reverse_idx--;
											}
											reverse_idx++;
											validx=atoi(ae->computectx->varbuffer+reverse_idx);
											structlabel=TxtStrDup(ae->computectx->varbuffer+minusptr);
											structlabel[reverse_idx-minusptr]=0;
#if TRACE_STRUCT
			printf("EVOL 119 -> looking for struct %s IDX=%d\n",structlabel,validx);
#endif
											/* unoptimized search in structures aliases */
											crc=GetCRC(structlabel);
											for (i=0;i<ae->irasmstructalias;i++) {
												if (ae->rasmstructalias[i].crc==crc && strcmp(ae->rasmstructalias[i].name,structlabel)==0) {
#if TRACE_STRUCT
							printf("EVOL 119 -> found! ptr=%d size=%d\n",ae->rasmstructalias[i].ptr,ae->rasmstructalias[i].size);
#endif
													curval=ae->rasmstructalias[i].size*validx+ae->rasmstructalias[i].ptr;
													if (validx>=ae->rasmstructalias[i].nbelem) {
														if (!ae->nowarning) {
															rasm_printf(ae,KWARNING"[%s:%d] Warning: index out of array size!\n",GetExpFile(ae,didx),GetExpLine(ae,didx));
															if (ae->erronwarn) MaxError(ae);
														}
													}
													break;
												}
											}
											if (i==ae->irasmstructalias) {
												/* not found */
												validx=-1;
											}
											MemFree(structlabel);
										}
										if (validx<0) {
											/* last chance to get a keyword */
											if (strcmp(ae->computectx->varbuffer+minusptr,"REPEAT_COUNTER")==0) {
												if (ae->ir) {
													curval=ae->repeat[ae->ir-1].repeat_counter;
												} else {
													MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"cannot use REPEAT_COUNTER keyword outside a repeat loop\n");
													curval=0;
												}
											} else if (strcmp(ae->computectx->varbuffer+minusptr,"WHILE_COUNTER")==0) {
												if (ae->iw) {
													curval=ae->whilewend[ae->iw-1].while_counter;
												} else {
													MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"cannot use WHILE_COUNTER keyword outside a while loop\n");
													curval=0;
												}
											} else {
												/* in case the expression is a register */
												if (IsRegister(ae->computectx->varbuffer+minusptr)) {
													MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"cannot use register %s in this context\n",TradExpression(zeexpression));
												} else {
													if (IsDirective(ae->computectx->varbuffer+minusptr)) {
														MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"cannot use directive %s in this context\n",TradExpression(zeexpression));
													} else {

														MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"expression [%s] keyword [%s] not found in variables, labels or aliases\n",TradExpression(zeexpression),ae->computectx->varbuffer+minusptr);
														if (ae->extended_error) {
															char *lookstr;
															lookstr=StringLooksLike(ae,ae->computectx->varbuffer+minusptr);
															if (lookstr) {
																rasm_printf(ae,KERROR" did you mean [%s] ?\n",lookstr);
															}
														}
													}
												}
												
												curval=0;
											}
										}
									}
								}
							}
						}
					}
			}
			if (minusptr) curval=-curval;
			stackelement.operator=E_COMPUTE_OPERATION_PUSH_DATASTC;
			stackelement.value=curval;
			/* priority isn't used */
			stackelement.string=NULL;
			
			allow_minus_as_sign=0;
			ivar=0;
		}
		/************************************
		      C R E A T E    T O K E N
		************************************/
		ObjectArrayAddDynamicValueConcat((void **)&ae->computectx->tokenstack,&nbtokenstack,&ae->computectx->maxtokenstack,&stackelement,sizeof(stackelement));
	}
	/*******************************************************
	      C R E A T E    E X E C U T I O N    S T A C K
	*******************************************************/
#define DEBUG_STACK 0
#if DEBUG_STACK
	for (itoken=0;itoken<nbtokenstack;itoken++) {
		switch (ae->computectx->tokenstack[itoken].operator) {
			case E_COMPUTE_OPERATION_PUSH_DATASTC:printf("%lf %s",ae->computectx->tokenstack[itoken].value,ae->computectx->tokenstack[itoken].string?ae->computectx->tokenstack[itoken].string:"(null)");break;
			case E_COMPUTE_OPERATION_OPEN:printf("(");break;
			case E_COMPUTE_OPERATION_CLOSE:printf(")");break;
			case E_COMPUTE_OPERATION_ADD:printf("+ ");break;
			case E_COMPUTE_OPERATION_SUB:printf("- ");break;
			case E_COMPUTE_OPERATION_DIV:printf("/ ");break;
			case E_COMPUTE_OPERATION_MUL:printf("* ");break;
			case E_COMPUTE_OPERATION_AND:printf("and ");break;
			case E_COMPUTE_OPERATION_OR:printf("or ");break;
			case E_COMPUTE_OPERATION_MOD:printf("mod ");break;
			case E_COMPUTE_OPERATION_XOR:printf("xor ");break;
			case E_COMPUTE_OPERATION_NOT:printf("! ");break;
			case E_COMPUTE_OPERATION_SHL:printf("<< ");break;
			case E_COMPUTE_OPERATION_SHR:printf(">> ");break;
			case E_COMPUTE_OPERATION_BAND:printf("&& ");break;
			case E_COMPUTE_OPERATION_BOR:printf("|| ");break;
			case E_COMPUTE_OPERATION_LOWER:printf("< ");break;
			case E_COMPUTE_OPERATION_GREATER:printf("> ");break;
			case E_COMPUTE_OPERATION_EQUAL:printf("== ");break;
			case E_COMPUTE_OPERATION_NOTEQUAL:printf("!= ");break;
			case E_COMPUTE_OPERATION_LOWEREQ:printf("<= ");break;
			case E_COMPUTE_OPERATION_GREATEREQ:printf(">= ");break;
			case E_COMPUTE_OPERATION_SIN:printf("sin ");break;
			case E_COMPUTE_OPERATION_COS:printf("cos ");break;
			case E_COMPUTE_OPERATION_INT:printf("int ");break;
			case E_COMPUTE_OPERATION_FLOOR:printf("floor ");break;
			case E_COMPUTE_OPERATION_ABS:printf("abs ");break;
			case E_COMPUTE_OPERATION_LN:printf("ln ");break;
			case E_COMPUTE_OPERATION_LOG10:printf("log10 ");break;
			case E_COMPUTE_OPERATION_SQRT:printf("sqrt ");break;
			case E_COMPUTE_OPERATION_ASIN:printf("asin ");break;
			case E_COMPUTE_OPERATION_ACOS:printf("acos ");break;
			case E_COMPUTE_OPERATION_ATAN:printf("atan ");break;
			case E_COMPUTE_OPERATION_EXP:printf("exp ");break;
			case E_COMPUTE_OPERATION_LOW:printf("low ");break;
			case E_COMPUTE_OPERATION_HIGH:printf("high ");break;
			case E_COMPUTE_OPERATION_PSG:printf("psg ");break;
			case E_COMPUTE_OPERATION_RND:printf("rnd ");break;
			case E_COMPUTE_OPERATION_FRAC:printf("frac ");break;
			case E_COMPUTE_OPERATION_CEIL:printf("ceil ");break;
			case E_COMPUTE_OPERATION_GET_R:printf("get_r ");break;
			case E_COMPUTE_OPERATION_GET_V:printf("get_v ");break;
			case E_COMPUTE_OPERATION_GET_B:printf("get_b ");break;
			case E_COMPUTE_OPERATION_SET_R:printf("set_r ");break;
			case E_COMPUTE_OPERATION_SET_V:printf("set_v ");break;
			case E_COMPUTE_OPERATION_SET_B:printf("set_b ");break;
			case E_COMPUTE_OPERATION_SOFT2HARD:printf("soft2hard ");break;
			case E_COMPUTE_OPERATION_HARD2SOFT:printf("hard2soft ");break;
			case E_COMPUTE_OPERATION_GETNOP:printf("getnop ");break;
			case E_COMPUTE_OPERATION_GETTICK:printf("gettick ");break;
			case E_COMPUTE_OPERATION_DURATION:printf("duration ");break;
			default:printf("bug\n");break;
		}
		
	}
	printf("\n");
#endif

	for (itoken=0;itoken<nbtokenstack;itoken++) {
		switch (ae->computectx->tokenstack[itoken].operator) {
			case E_COMPUTE_OPERATION_PUSH_DATASTC:
#if DEBUG_STACK
printf("data string=%X\n",ae->computectx->tokenstack[itoken].string);
#endif
				ObjectArrayAddDynamicValueConcat((void **)&computestack,&nbcomputestack,&maxcomputestack,&ae->computectx->tokenstack[itoken],sizeof(stackelement));
				break;
			case E_COMPUTE_OPERATION_OPEN:
				ObjectArrayAddDynamicValueConcat((void **)&ae->computectx->operatorstack,&nboperatorstack,&ae->computectx->maxoperatorstack,&ae->computectx->tokenstack[itoken],sizeof(stackelement));
#if DEBUG_STACK
printf("ajout ( string=%X\n",ae->computectx->tokenstack[itoken].string);
#endif
				break;
			case E_COMPUTE_OPERATION_CLOSE:
#if DEBUG_STACK
printf("close\n");
#endif
				/* pop out token until the opened parenthesis is reached */
				o2=nboperatorstack-1;
				okclose=0;
				while (o2>=0) {
					if (ae->computectx->operatorstack[o2].operator!=E_COMPUTE_OPERATION_OPEN) {
						ObjectArrayAddDynamicValueConcat((void **)&computestack,&nbcomputestack,&maxcomputestack,&ae->computectx->operatorstack[o2],sizeof(stackelement));
						nboperatorstack--;
#if DEBUG_STACK
printf("op-- string=%X\n",ae->computectx->operatorstack[o2].string);
#endif
						o2--;
					} else {
						/* discard opening parenthesis as operator */
#if DEBUG_STACK
printf("discard )\n");
#endif
						nboperatorstack--;
						okclose=1;
						o2--;
						break;
					}
				}
				if (!okclose) {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"missing parenthesis [%s]\n",TradExpression(zeexpression));
					if (!original) {
						MemFree(zeexpression);
					}
					return 0;
				}
				/* if upper token is a function then pop from the stack */
				if (o2>=0 && ae->computectx->operatorstack[o2].operator>=E_COMPUTE_OPERATION_SIN) {
					ObjectArrayAddDynamicValueConcat((void **)&computestack,&nbcomputestack,&maxcomputestack,&ae->computectx->operatorstack[o2],sizeof(stackelement));
					nboperatorstack--;
#if DEBUG_STACK
printf("pop function string=%X\n",ae->computectx->operatorstack[o2].string);
#endif
				}
				break;
			case E_COMPUTE_OPERATION_ADD:
			case E_COMPUTE_OPERATION_SUB:
			case E_COMPUTE_OPERATION_DIV:
			case E_COMPUTE_OPERATION_MUL:
			case E_COMPUTE_OPERATION_AND:
			case E_COMPUTE_OPERATION_OR:
			case E_COMPUTE_OPERATION_MOD:
			case E_COMPUTE_OPERATION_XOR:
			case E_COMPUTE_OPERATION_NOT:
			case E_COMPUTE_OPERATION_SHL:
			case E_COMPUTE_OPERATION_SHR:
			case E_COMPUTE_OPERATION_BAND:
			case E_COMPUTE_OPERATION_BOR:
			case E_COMPUTE_OPERATION_LOWER:
			case E_COMPUTE_OPERATION_GREATER:
			case E_COMPUTE_OPERATION_EQUAL:
			case E_COMPUTE_OPERATION_NOTEQUAL:
			case E_COMPUTE_OPERATION_LOWEREQ:
			case E_COMPUTE_OPERATION_GREATEREQ:
				o2=nboperatorstack-1;
				while (o2>=0 && ae->computectx->operatorstack[o2].operator!=E_COMPUTE_OPERATION_OPEN) {
					if (ae->computectx->tokenstack[itoken].priority>=ae->computectx->operatorstack[o2].priority || ae->computectx->operatorstack[o2].operator>=E_COMPUTE_OPERATION_SIN) {
						ObjectArrayAddDynamicValueConcat((void **)&computestack,&nbcomputestack,&maxcomputestack,&ae->computectx->operatorstack[o2],sizeof(stackelement));
#if DEBUG_STACK
printf("operator string=%X\n",ae->computectx->operatorstack[o2].string);
#endif
						nboperatorstack--;
						o2--;
					} else {
						break;
					}
				}
				ObjectArrayAddDynamicValueConcat((void **)&ae->computectx->operatorstack,&nboperatorstack,&ae->computectx->maxoperatorstack,&ae->computectx->tokenstack[itoken],sizeof(stackelement));
				break;
			case E_COMPUTE_OPERATION_SIN:
			case E_COMPUTE_OPERATION_COS:
			case E_COMPUTE_OPERATION_INT:
			case E_COMPUTE_OPERATION_FLOOR:
			case E_COMPUTE_OPERATION_ABS:
			case E_COMPUTE_OPERATION_LN:
			case E_COMPUTE_OPERATION_LOG10:
			case E_COMPUTE_OPERATION_SQRT:
			case E_COMPUTE_OPERATION_ASIN:
			case E_COMPUTE_OPERATION_ACOS:
			case E_COMPUTE_OPERATION_ATAN:
			case E_COMPUTE_OPERATION_EXP:
			case E_COMPUTE_OPERATION_LOW:
			case E_COMPUTE_OPERATION_HIGH:
			case E_COMPUTE_OPERATION_PSG:
			case E_COMPUTE_OPERATION_RND:
			case E_COMPUTE_OPERATION_FRAC:
			case E_COMPUTE_OPERATION_CEIL:
			case E_COMPUTE_OPERATION_GET_R:
			case E_COMPUTE_OPERATION_GET_V:
			case E_COMPUTE_OPERATION_GET_B:
			case E_COMPUTE_OPERATION_SET_R:
			case E_COMPUTE_OPERATION_SET_V:
			case E_COMPUTE_OPERATION_SET_B:
			case E_COMPUTE_OPERATION_SOFT2HARD:
			case E_COMPUTE_OPERATION_HARD2SOFT:
			case E_COMPUTE_OPERATION_GETNOP:
			case E_COMPUTE_OPERATION_GETTICK:
			case E_COMPUTE_OPERATION_DURATION:
#if DEBUG_STACK
printf("ajout de la fonction\n");
#endif
				ObjectArrayAddDynamicValueConcat((void **)&ae->computectx->operatorstack,&nboperatorstack,&ae->computectx->maxoperatorstack,&ae->computectx->tokenstack[itoken],sizeof(stackelement));
				break;
			default:break;
		}
	}
	/* pop remaining operators */
	while (nboperatorstack>0) {
		ObjectArrayAddDynamicValueConcat((void **)&computestack,&nbcomputestack,&maxcomputestack,&ae->computectx->operatorstack[--nboperatorstack],sizeof(stackelement));
#if DEBUG_STACK
printf("final POP string=%X\n",ae->computectx->operatorstack[nboperatorstack+1].string);
#endif
	}
	
	/********************************************
	        E X E C U T E        S T A C K
	********************************************/
	if (ae->maxam || ae->as80) {
		int workinterval;
		if (ae->as80) workinterval=0xFFFFFFFF; else workinterval=0xFFFF;
		for (i=0;i<nbcomputestack;i++) {
			switch (computestack[i].operator) {
				/************************************************
				  c a s e s   s h o u l d    b e    s o r t e d
				************************************************/
				case E_COMPUTE_OPERATION_PUSH_DATASTC:
					if (maccu<=paccu) {
						maccu=16+paccu;
						accu=MemRealloc(accu,sizeof(double)*maccu);
					}
					if (computestack[i].string) {
						/* string hack */
						accu[paccu]=i+0.1;
					} else {
						accu[paccu]=computestack[i].value;
					}
					paccu++;
					break;
				case E_COMPUTE_OPERATION_OPEN:
				case E_COMPUTE_OPERATION_CLOSE:/* cannot happend */ break;
				case E_COMPUTE_OPERATION_ADD:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]+(int)accu[paccu-1])&workinterval;paccu--;break;
				case E_COMPUTE_OPERATION_SUB:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]-(int)accu[paccu-1])&workinterval;paccu--;break;
				case E_COMPUTE_OPERATION_MUL:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]*(int)accu[paccu-1])&workinterval;paccu--;break;
				case E_COMPUTE_OPERATION_DIV:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]/(int)accu[paccu-1])&workinterval;paccu--;break;
				case E_COMPUTE_OPERATION_AND:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]&(int)accu[paccu-1])&workinterval;paccu--;break;
				case E_COMPUTE_OPERATION_OR:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]|(int)accu[paccu-1])&workinterval;paccu--;break;
				case E_COMPUTE_OPERATION_XOR:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]^(int)accu[paccu-1])&workinterval;paccu--;break;
				case E_COMPUTE_OPERATION_MOD:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]%(int)accu[paccu-1])&workinterval;paccu--;break;
				case E_COMPUTE_OPERATION_SHL:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2])<<((int)accu[paccu-1]);
								if (((int)accu[paccu-1])>31 || ((int)accu[paccu-1])<-31) {
									if (!ae->nowarning) {
										rasm_printf(ae,KWARNING"Warning - shifting %d is architecture dependant, result forced to ZERO\n",(int)accu[paccu-1]);
															if (ae->erronwarn) MaxError(ae);
									}
									accu[paccu-2]=0;
								}
								paccu--;break;
				case E_COMPUTE_OPERATION_SHR:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2])>>((int)accu[paccu-1]);
								if (((int)accu[paccu-1])>31 || ((int)accu[paccu-1])<-31) {
									if (!ae->nowarning) {
										rasm_printf(ae,KWARNING"Warning - shifting %d is architecture dependant, result forced to ZERO\n",(int)accu[paccu-1]);
															if (ae->erronwarn) MaxError(ae);
									}
									accu[paccu-2]=0;
								}
								paccu--;break;
				case E_COMPUTE_OPERATION_BAND:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]&&(int)accu[paccu-1])&workinterval;paccu--;break;
				case E_COMPUTE_OPERATION_BOR:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]||(int)accu[paccu-1])&workinterval;paccu--;break;
				/* comparison */
				case E_COMPUTE_OPERATION_LOWER:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]&workinterval)<((int)accu[paccu-1]&workinterval);paccu--;break;
				case E_COMPUTE_OPERATION_LOWEREQ:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]&workinterval)<=((int)accu[paccu-1]&workinterval);paccu--;break;
				case E_COMPUTE_OPERATION_EQUAL:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]&workinterval)==((int)accu[paccu-1]&workinterval);paccu--;break;
				case E_COMPUTE_OPERATION_NOTEQUAL:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]&workinterval)!=((int)accu[paccu-1]&workinterval);paccu--;break;
				case E_COMPUTE_OPERATION_GREATER:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]&workinterval)>((int)accu[paccu-1]&workinterval);paccu--;break;
				case E_COMPUTE_OPERATION_GREATEREQ:if (paccu>1) accu[paccu-2]=((int)accu[paccu-2]&workinterval)>=((int)accu[paccu-1]&workinterval);paccu--;break;
				/* functions */
				case E_COMPUTE_OPERATION_SIN:if (paccu>0) accu[paccu-1]=(int)sin(accu[paccu-1]*3.1415926545/180.0);break;
				case E_COMPUTE_OPERATION_COS:if (paccu>0) accu[paccu-1]=(int)cos(accu[paccu-1]*3.1415926545/180.0);break;
				case E_COMPUTE_OPERATION_ASIN:if (paccu>0) accu[paccu-1]=(int)asin(accu[paccu-1])*180.0/3.1415926545;break;
				case E_COMPUTE_OPERATION_ACOS:if (paccu>0) accu[paccu-1]=(int)acos(accu[paccu-1])*180.0/3.1415926545;break;
				case E_COMPUTE_OPERATION_ATAN:if (paccu>0) accu[paccu-1]=(int)atan(accu[paccu-1])*180.0/3.1415926545;break;
				case E_COMPUTE_OPERATION_INT:break;
				case E_COMPUTE_OPERATION_FLOOR:if (paccu>0) accu[paccu-1]=(int)floor(accu[paccu-1])&workinterval;break;
				case E_COMPUTE_OPERATION_ABS:if (paccu>0) accu[paccu-1]=(int)fabs(accu[paccu-1])&workinterval;break;
				case E_COMPUTE_OPERATION_EXP:if (paccu>0) accu[paccu-1]=(int)exp(accu[paccu-1])&workinterval;break;
				case E_COMPUTE_OPERATION_LN:if (paccu>0) accu[paccu-1]=(int)log(accu[paccu-1])&workinterval;break;
				case E_COMPUTE_OPERATION_LOG10:if (paccu>0) accu[paccu-1]=(int)log10(accu[paccu-1])&workinterval;break;
				case E_COMPUTE_OPERATION_SQRT:if (paccu>0) accu[paccu-1]=(int)sqrt(accu[paccu-1])&workinterval;break;
				case E_COMPUTE_OPERATION_LOW:if (paccu>0) accu[paccu-1]=((int)accu[paccu-1])&0xFF;break;
				case E_COMPUTE_OPERATION_HIGH:if (paccu>0) accu[paccu-1]=(((int)accu[paccu-1])&0xFF00)>>8;break;
				case E_COMPUTE_OPERATION_PSG:if (paccu>0) accu[paccu-1]=ae->psgfine[((int)accu[paccu-1])&0xFF];break;
				case E_COMPUTE_OPERATION_RND:if (paccu>0) {
								     int zemod;
								     zemod=(int)floor(accu[paccu-1]+0.5);
								     if (zemod>0) {
									     accu[paccu-1]=FastRand()%zemod;
								     } else {
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"RND function needs a value greater than zero to perform a random value\n");
								        accu[paccu-1]=0;
								     }
							     }
							     break;
				case E_COMPUTE_OPERATION_FRAC:if (paccu>0) accu[paccu-1]=((int)(accu[paccu-1]-(int)accu[paccu-1]));break;
				case E_COMPUTE_OPERATION_CEIL:if (paccu>0) accu[paccu-1]=(int)ceil(accu[paccu-1])&workinterval;break;
				case E_COMPUTE_OPERATION_GET_R:if (paccu>0) accu[paccu-1]=((((int)accu[paccu-1])&0xF0)>>4);break;
				case E_COMPUTE_OPERATION_GET_V:if (paccu>0) accu[paccu-1]=((((int)accu[paccu-1])&0xF00)>>8);break;
				case E_COMPUTE_OPERATION_GET_B:if (paccu>0) accu[paccu-1]=(((int)accu[paccu-1])&0xF);break;
				case E_COMPUTE_OPERATION_SET_R:if (paccu>0) accu[paccu-1]=MinMaxInt(accu[paccu-1],0,15)<<4;break;
				case E_COMPUTE_OPERATION_SET_V:if (paccu>0) accu[paccu-1]=MinMaxInt(accu[paccu-1],0,15)<<8;break;
				case E_COMPUTE_OPERATION_SET_B:if (paccu>0) accu[paccu-1]=MinMaxInt(accu[paccu-1],0,15);break;
				case E_COMPUTE_OPERATION_SOFT2HARD:if (paccu>0) accu[paccu-1]=__Soft2HardInk(ae,accu[paccu-1],didx);break;
				case E_COMPUTE_OPERATION_HARD2SOFT:if (paccu>0) accu[paccu-1]=__Hard2SoftInk(ae,accu[paccu-1],didx);break;
				/* functions with strings */
				case E_COMPUTE_OPERATION_GETNOP:if (paccu>0) {
								      int integeridx;
								      integeridx=floor(accu[paccu-1]);

								      if (integeridx>=0 && integeridx<nbcomputestack && computestack[integeridx].string) {
									      accu[paccu-1]=__GETNOP(ae,computestack[integeridx].string,didx);
									      MemFree(computestack[integeridx].string);
									      computestack[integeridx].string=NULL;
								      } else {
									      if (integeridx>=0 && integeridx<nbcomputestack) {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETNOP function needs a proper string\n");
										} else {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETNOP internal error (wrong string index)\n");
										}
									}
								} else {
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETNOP is empty\n");
								}
							       break;
							       /* CC GETNOP */
				case E_COMPUTE_OPERATION_GETTICK:if (paccu>0) {
								      int integeridx;
								      integeridx=floor(accu[paccu-1]);

								      if (integeridx>=0 && integeridx<nbcomputestack && computestack[integeridx].string) {
									      accu[paccu-1]=__GETTICK(ae,computestack[integeridx].string,didx);
									      MemFree(computestack[integeridx].string);
									      computestack[integeridx].string=NULL;
								      } else {
									      if (integeridx>=0 && integeridx<nbcomputestack) {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETTICK function needs a proper string\n");
										} else {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETTICK internal error (wrong string index)\n");
										}
									}
								} else {
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETTICK is empty\n");
								}
							       break;
							       /* CC GETNOP */
				case E_COMPUTE_OPERATION_DURATION:if (paccu>0) {
								      int integeridx;
								      integeridx=floor(accu[paccu-1]);

								      if (integeridx>=0 && integeridx<nbcomputestack && computestack[integeridx].string) {
									      accu[paccu-1]=__DURATION(ae,computestack[integeridx].string,didx);
									      MemFree(computestack[integeridx].string);
									      computestack[integeridx].string=NULL;
								      } else {
									      if (integeridx>=0 && integeridx<nbcomputestack) {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"DURATION function needs a proper string\n");
										} else {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"DURATION internal error (wrong string index)\n");
										}
									}
								} else {
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"DURATION is empty\n");
								}
							       break;
				default:MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"invalid computing state! (%d)\n",computestack[i].operator);paccu=0;
			}
			if (!paccu) {
				if (zeexpression[0]=='&') {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"Missing operand for calculation [%s] Did you use & for an hexadecimal value?\n",TradExpression(zeexpression));
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"Missing operand for calculation [%s]\n",TradExpression(zeexpression));
				}
				accu_err=1;
				break;
			}
		}
	} else {
		for (i=0;i<nbcomputestack;i++) {
#if 0
			int kk;
			for (kk=0;kk<paccu;kk++) printf("stack[%d]=%lf\n",kk,accu[kk]);
			if (computestack[i].operator==E_COMPUTE_OPERATION_PUSH_DATASTC) {
				printf("pacc=%d push %.1lf or %s\n",paccu,computestack[i].value,computestack[i].string?computestack[i].string:"null");
			} else {
				printf("pacc=%d operation %s p=%d\n",paccu,computestack[i].operator==E_COMPUTE_OPERATION_MUL?"*":
								computestack[i].operator==E_COMPUTE_OPERATION_ADD?"+":
								computestack[i].operator==E_COMPUTE_OPERATION_DIV?"/":
								computestack[i].operator==E_COMPUTE_OPERATION_SUB?"-":
								computestack[i].operator==E_COMPUTE_OPERATION_BAND?"&&":
								computestack[i].operator==E_COMPUTE_OPERATION_BOR?"||":
								computestack[i].operator==E_COMPUTE_OPERATION_SHL?"<<":
								computestack[i].operator==E_COMPUTE_OPERATION_SHR?">>":
								computestack[i].operator==E_COMPUTE_OPERATION_LOWER?"<":
								computestack[i].operator==E_COMPUTE_OPERATION_GREATER?">":
								computestack[i].operator==E_COMPUTE_OPERATION_EQUAL?"==":
								computestack[i].operator==E_COMPUTE_OPERATION_INT?"INT":
								computestack[i].operator==E_COMPUTE_OPERATION_LOWEREQ?"<=":
								computestack[i].operator==E_COMPUTE_OPERATION_GREATEREQ?">=":
								computestack[i].operator==E_COMPUTE_OPERATION_OPEN?"(":
								computestack[i].operator==E_COMPUTE_OPERATION_CLOSE?")":
								computestack[i].operator==E_COMPUTE_OPERATION_GETNOP?"getnpo":
								"<autre>",computestack[i].priority);
			}
#endif
			switch (computestack[i].operator) {
				case E_COMPUTE_OPERATION_PUSH_DATASTC:
					if (maccu<=paccu) {
						maccu=16+paccu;
						accu=MemRealloc(accu,sizeof(double)*maccu);
					}
					if (computestack[i].string) {
						/* string hack */
						accu[paccu]=i+0.1;
					} else {
						accu[paccu]=computestack[i].value;
					}
					paccu++;
					break;
				case E_COMPUTE_OPERATION_OPEN:
				case E_COMPUTE_OPERATION_CLOSE: /* cannot happend */ break;
				case E_COMPUTE_OPERATION_ADD:if (paccu>1) accu[paccu-2]+=accu[paccu-1];paccu--;break;
				case E_COMPUTE_OPERATION_SUB:if (paccu>1) accu[paccu-2]-=accu[paccu-1];paccu--;break;
				case E_COMPUTE_OPERATION_MUL:if (paccu>1) accu[paccu-2]*=accu[paccu-1];paccu--;break;
				case E_COMPUTE_OPERATION_DIV:if (paccu>1) accu[paccu-2]/=accu[paccu-1];paccu--;break;
				case E_COMPUTE_OPERATION_AND:if (paccu>1) accu[paccu-2]=((int)floor(accu[paccu-2]+0.5))&((int)floor(accu[paccu-1]+0.5));paccu--;break;
				case E_COMPUTE_OPERATION_OR:if (paccu>1) accu[paccu-2]=((int)floor(accu[paccu-2]+0.5))|((int)floor(accu[paccu-1]+0.5));paccu--;break;
				case E_COMPUTE_OPERATION_XOR:if (paccu>1) accu[paccu-2]=((int)floor(accu[paccu-2]+0.5))^((int)floor(accu[paccu-1]+0.5));paccu--;break;
				case E_COMPUTE_OPERATION_NOT:/* half operator, half function */ if (paccu>0) accu[paccu-1]=!((int)floor(accu[paccu-1]+0.5));break;
				case E_COMPUTE_OPERATION_MOD:if (paccu>1) accu[paccu-2]=((int)floor(accu[paccu-2]+0.5))%((int)floor(accu[paccu-1]+0.5));paccu--;break;
				case E_COMPUTE_OPERATION_SHL:if (paccu>1) accu[paccu-2]=((int)floor(accu[paccu-2]+0.5))<<((int)floor(accu[paccu-1]+0.5));
								if (((int)accu[paccu-1])>31 || ((int)accu[paccu-1])<-31) {
									if (!ae->nowarning) {
										rasm_printf(ae,KWARNING"Warning - shifting %d is architecture dependant, result forced to ZERO\n",(int)accu[paccu-1]);
															if (ae->erronwarn) MaxError(ae);
									}
									accu[paccu-2]=0;
								}
								paccu--;break;
				case E_COMPUTE_OPERATION_SHR:if (paccu>1) accu[paccu-2]=((int)floor(accu[paccu-2]+0.5))>>((int)floor(accu[paccu-1]+0.5));
								if (((int)accu[paccu-1])>31 || ((int)accu[paccu-1])<-31) {
									if (!ae->nowarning) {
										rasm_printf(ae,KWARNING"Warning - shifting %d is architecture dependant, result forced to ZERO\n",(int)accu[paccu-1]);
															if (ae->erronwarn) MaxError(ae);
									}
									accu[paccu-2]=0;
								}
								paccu--;break;
				case E_COMPUTE_OPERATION_BAND:if (paccu>1) accu[paccu-2]=((int)floor(accu[paccu-2]+0.5))&&((int)floor(accu[paccu-1]+0.5));paccu--;break;
				case E_COMPUTE_OPERATION_BOR:if (paccu>1) accu[paccu-2]=((int)floor(accu[paccu-2]+0.5))||((int)floor(accu[paccu-1]+0.5));paccu--;break;
				/* comparison */
				case E_COMPUTE_OPERATION_LOWER:if (paccu>1) accu[paccu-2]=accu[paccu-2]<accu[paccu-1];paccu--;break;
				case E_COMPUTE_OPERATION_LOWEREQ:if (paccu>1) accu[paccu-2]=accu[paccu-2]<=accu[paccu-1];paccu--;break;
				case E_COMPUTE_OPERATION_EQUAL:if (paccu>1) accu[paccu-2]=fabs(accu[paccu-2]-accu[paccu-1])<0.000001;paccu--;break;
				case E_COMPUTE_OPERATION_NOTEQUAL:if (paccu>1) accu[paccu-2]=accu[paccu-2]!=accu[paccu-1];paccu--;break;
				case E_COMPUTE_OPERATION_GREATER:if (paccu>1) accu[paccu-2]=accu[paccu-2]>accu[paccu-1];paccu--;break;
				case E_COMPUTE_OPERATION_GREATEREQ:if (paccu>1) accu[paccu-2]=accu[paccu-2]>=accu[paccu-1];paccu--;break;
				/* functions */
				case E_COMPUTE_OPERATION_SIN:if (paccu>0) accu[paccu-1]=sin(accu[paccu-1]*3.1415926545/180.0);break;
				case E_COMPUTE_OPERATION_COS:if (paccu>0) accu[paccu-1]=cos(accu[paccu-1]*3.1415926545/180.0);break;
				case E_COMPUTE_OPERATION_ASIN:if (paccu>0) accu[paccu-1]=asin(accu[paccu-1])*180.0/3.1415926545;break;
				case E_COMPUTE_OPERATION_ACOS:if (paccu>0) accu[paccu-1]=acos(accu[paccu-1])*180.0/3.1415926545;break;
				case E_COMPUTE_OPERATION_ATAN:if (paccu>0) accu[paccu-1]=atan(accu[paccu-1])*180.0/3.1415926545;break;
				case E_COMPUTE_OPERATION_INT:if (paccu>0) accu[paccu-1]=floor(accu[paccu-1]+0.5);break;
				case E_COMPUTE_OPERATION_FLOOR:if (paccu>0) accu[paccu-1]=floor(accu[paccu-1]);break;
				case E_COMPUTE_OPERATION_ABS:if (paccu>0) accu[paccu-1]=fabs(accu[paccu-1]);break;
				case E_COMPUTE_OPERATION_EXP:if (paccu>0) accu[paccu-1]=exp(accu[paccu-1]);break;
				case E_COMPUTE_OPERATION_LN:if (paccu>0) accu[paccu-1]=log(accu[paccu-1]);break;
				case E_COMPUTE_OPERATION_LOG10:if (paccu>0) accu[paccu-1]=log10(accu[paccu-1]);break;
				case E_COMPUTE_OPERATION_SQRT:if (paccu>0) accu[paccu-1]=sqrt(accu[paccu-1]);break;
				case E_COMPUTE_OPERATION_LOW:if (paccu>0) accu[paccu-1]=((int)floor(accu[paccu-1]+0.5))&0xFF;break;
				case E_COMPUTE_OPERATION_HIGH:if (paccu>0) accu[paccu-1]=(((int)floor(accu[paccu-1]+0.5))&0xFF00)>>8;break;
				case E_COMPUTE_OPERATION_PSG:if (paccu>0) accu[paccu-1]=ae->psgfine[((int)floor(accu[paccu-1]+0.5))&0xFF];break;
				case E_COMPUTE_OPERATION_RND:if (paccu>0) {
								     int zemod;
								     zemod=(int)floor(accu[paccu-1]+0.5);
								     if (zemod>0) {
									     accu[paccu-1]=FastRand()%zemod;
								     } else {
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"RND function needs a value greater than zero to perform a random value\n");
								        accu[paccu-1]=0;
								     }
							     }
							     break;
				case E_COMPUTE_OPERATION_FRAC:if (paccu>0) accu[paccu-1]=modf(accu[paccu-1],&dummint);break;
				case E_COMPUTE_OPERATION_CEIL:if (paccu>0) accu[paccu-1]=ceil(accu[paccu-1]);break;
				case E_COMPUTE_OPERATION_GET_R:if (paccu>0) accu[paccu-1]=((((int)accu[paccu-1])&0xF0)>>4);break;
				case E_COMPUTE_OPERATION_GET_V:if (paccu>0) accu[paccu-1]=((((int)accu[paccu-1])&0xF00)>>8);break;
				case E_COMPUTE_OPERATION_GET_B:if (paccu>0) accu[paccu-1]=(((int)accu[paccu-1])&0xF);break;
				case E_COMPUTE_OPERATION_SET_R:if (paccu>0) accu[paccu-1]=MinMaxInt(accu[paccu-1],0,15)<<4;break;
				case E_COMPUTE_OPERATION_SET_V:if (paccu>0) accu[paccu-1]=MinMaxInt(accu[paccu-1],0,15)<<8;break;
				case E_COMPUTE_OPERATION_SET_B:if (paccu>0) accu[paccu-1]=MinMaxInt(accu[paccu-1],0,15);break;
				case E_COMPUTE_OPERATION_SOFT2HARD:if (paccu>0) accu[paccu-1]=__Soft2HardInk(ae,accu[paccu-1],didx);break;
				case E_COMPUTE_OPERATION_HARD2SOFT:if (paccu>0) accu[paccu-1]=__Hard2SoftInk(ae,accu[paccu-1],didx);break;
				/* functions with strings */
				case E_COMPUTE_OPERATION_GETNOP:if (paccu>0) {
								      int integeridx;
								      integeridx=floor(accu[paccu-1]);

								      if (integeridx>=0 && integeridx<nbcomputestack && computestack[integeridx].string) {
									      accu[paccu-1]=__GETNOP(ae,computestack[integeridx].string,didx);
									      MemFree(computestack[integeridx].string);
									      computestack[integeridx].string=NULL;
								      } else {
									      if (integeridx>=0 && integeridx<nbcomputestack) {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETNOP function needs a proper string\n");
										} else {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETNOP internal error (wrong string index)\n");
										}
									}
								} else {
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETNOP is empty\n");
								}
							       break;
							       /* CC GETNOP */
				case E_COMPUTE_OPERATION_GETTICK:if (paccu>0) {
								      int integeridx;
								      integeridx=floor(accu[paccu-1]);

								      if (integeridx>=0 && integeridx<nbcomputestack && computestack[integeridx].string) {
									      accu[paccu-1]=__GETTICK(ae,computestack[integeridx].string,didx);
									      MemFree(computestack[integeridx].string);
									      computestack[integeridx].string=NULL;
								      } else {
									      if (integeridx>=0 && integeridx<nbcomputestack) {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETTICK function needs a proper string\n");
										} else {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETTICK internal error (wrong string index)\n");
										}
									}
								} else {
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"GETTICK is empty\n");
								}
							       break;
							       /* CC GETNOP */
				case E_COMPUTE_OPERATION_DURATION:if (paccu>0) {
								      int integeridx;
								      integeridx=floor(accu[paccu-1]);

								      if (integeridx>=0 && integeridx<nbcomputestack && computestack[integeridx].string) {
									      accu[paccu-1]=__DURATION(ae,computestack[integeridx].string,didx);
									      MemFree(computestack[integeridx].string);
									      computestack[integeridx].string=NULL;
								      } else {
									      if (integeridx>=0 && integeridx<nbcomputestack) {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"DURATION function needs a proper string\n");
										} else {
											MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"DURATION internal error (wrong string index)\n");
										}
									}
								} else {
									MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"DURATION is empty\n");
								}
							       break;

				default:MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"invalid computing state! (%d)\n",computestack[i].operator);paccu=0;
			}
			if (!paccu) {
				if (zeexpression[0]=='&') {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"Missing operand for calculation [%s] Did you use & for an hexadecimal value?\n",TradExpression(zeexpression));
				} else {
					MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"Missing operand for calculation [%s]\n",TradExpression(zeexpression));
				}
				accu_err=1;
				break;
			}
		}
	}
	if (!original) {
		MemFree(zeexpression);
	}
	if (paccu==1) {
		return accu[0];
	} else if (!accu_err) {
		if (paccu) {
			MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"Missing operator\n");
		} else {
			MakeError(ae,GetExpFile(ae,didx),GetExpLine(ae,didx),"Missing operand for calculation\n");
		}
		return 0;
	} else {
		return 0;
	}
}
int RoundComputeExpressionCore(struct s_assenv *ae,char *zeexpression,int ptr,int didx) {
	return floor(ComputeExpressionCore(ae,zeexpression,ptr,didx)+ae->rough);
}

void ExpressionSetDicoVar(struct s_assenv *ae,char *name, double v)
{
	#undef FUNC
	#define FUNC "ExpressionSetDicoVar"

	struct s_expr_dico curdic;
	curdic.name=TxtStrDup(name);
	curdic.crc=GetCRC(name);
	curdic.v=v;
	curdic.iw=ae->idx;
	curdic.autorise_export=ae->autorise_export;
	//ObjectArrayAddDynamicValueConcat((void**)&ae->dico,&ae->idic,&ae->mdic,&curdic,sizeof(curdic));
	if (SearchLabel(ae,curdic.name,curdic.crc)) {
		MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"cannot create variable [%s] as there is already a label with the same name\n",name);
		MemFree(curdic.name);
		return;
	}
	InsertDicoToTree(ae,&curdic);
}

double ComputeExpression(struct s_assenv *ae,char *expr, int ptr, int didx, int expected_eval)
{
	#undef FUNC
	#define FUNC "ComputeExpression"

	char *ptr_exp,*ptr_exp2;
	int crc,idx=0,ialias,touched;
	double v;
	struct s_alias curalias;
	struct s_expr_dico *curdic;


	while (!ae->AutomateExpressionDecision[((int)expr[idx])&0xFF]) idx++;

	switch (ae->AutomateExpressionDecision[((int)expr[idx])&0xFF]) {
		/*****************************************
		          M A K E     A L I A S
		*****************************************/
		case '~':
			memset(&curalias,0,sizeof(curalias));
			ptr_exp=expr+idx;
			*ptr_exp=0; // on scinde l'alias de son texte
			ptr_exp2=ptr_exp+1;
#if TRACE_COMPUTE_EXPRESSION
printf("MakeAlias (1) EXPR=[%s EQU %s]\n",expr,ptr_exp2);
#endif
			
			/* alias locaux ou de proximité */
			if (strchr("@.",expr[0])) {
#if TRACE_COMPUTE_EXPRESSION
printf("WARNING! alias is local! [%s]\n",expr);
#endif
				/* local label creation does not handle formula in tags */
				curalias.alias=TranslateTag(ae,TxtStrDup(expr),&touched,0,E_TAGOPTION_NONE);
				curalias.alias=MakeLocalLabel(ae,curalias.alias,NULL);
			} else if (strchr(expr,'{')) {
#if TRACE_COMPUTE_EXPRESSION
printf("WARNING! alias has tag! [%s]\n",expr);
#endif
				/* alias name contains formula */
				curalias.alias=TranslateTag(ae,TxtStrDup(expr),&touched,0,E_TAGOPTION_NONE);
#if TRACE_COMPUTE_EXPRESSION
printf("MakeAlias (2) EXPR=[%s EQU %s]\n",expr,ptr_exp2);
#endif
			} else {
				curalias.alias=TxtStrDup(expr);
			}
			curalias.crc=GetCRC(curalias.alias);
			if ((ialias=SearchAlias(ae,curalias.crc,curalias.alias))>=0) {
				MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"Duplicate alias [%s]\n",expr);
				MemFree(curalias.alias);
			} else if (SearchDico(ae,curalias.alias,curalias.crc)) {
				MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"Alias cannot override existing variable [%s]\n",expr);
				MemFree(curalias.alias);
			} else {
				curalias.translation=MemMalloc(strlen(ptr_exp2)+1+2);
				sprintf(curalias.translation,"(%s)",ptr_exp2);
#if TRACE_COMPUTE_EXPRESSION
printf("MakeAlias (3) EXPR=[%s EQU %s]\n",expr,ptr_exp2);
printf("alias translation [%s] -> ",curalias.translation);fflush(stdout);
#endif
				ExpressionFastTranslate(ae,&curalias.translation,2); // FAST type 2
#if TRACE_COMPUTE_EXPRESSION
printf("%s\n",curalias.translation);
#endif
				curalias.len=strlen(curalias.translation);
				curalias.autorise_export=ae->autorise_export;
				curalias.iw=ae->idx;
				ObjectArrayAddDynamicValueConcat((void**)&ae->alias,&ae->ialias,&ae->malias,&curalias,sizeof(curalias));
				CheckAndSortAliases(ae);
			}
			*ptr_exp='~'; // on remet l'alias en place
#if TRACE_COMPUTE_EXPRESSION
printf("MakeAlias end with alias=[%s]=[%s]\n",curalias.alias,curalias.translation);
printf("***********\n");
#endif
			return 0;
		/*****************************************
		               S E T     V A R
		*****************************************/
		case '=':
			/* patch NOT 
			 this is a variable assign if there is no other comparison operator after '='
			 BUT we may have ! which stand for NOT but is also a comparison operator...
			*/
			if (ae->AutomateExpressionDecision[((int)expr[idx+1])&0xFF]==0 || expr[idx+1]=='!') {
				if (expected_eval) {
					if (ae->maxam) {
						/* maxam mode AND expected a value -> force comparison */
					} else {
						/* use of a single '=' but expected a comparison anyway */
						MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"meaningless use of an expression [%s]\n",expr);
						return 0;
					}
				} else {
					/* ASSIGN */
					if ((expr[0]<'A' || expr[0]>'Z') && expr[0]!='_') {
						MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"variable name must begin by a letter or '_' [%s]\n",expr);
						return 0;
					} else {
						char operatorassignment;

						ptr_exp=expr+idx;
						v=ComputeExpressionCore(ae,ptr_exp+1,ptr,didx);
						*ptr_exp=0;
						/* patch operator+assign value */
						switch (ptr_exp[-1]) {
							case '+':
							case '-':
							case '*':
							case '/':
							case '^':
							case '&':
							case '|':
							case '%':
							case ']':
							case '[':
								operatorassignment=ptr_exp[-1];ptr_exp[-1]=0;break;
							default:operatorassignment=0;break;
						}

						crc=GetCRC(expr);
						if ((ialias=SearchAlias(ae,crc,expr))>=0) {
							MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"Variable cannot override existing alias [%s]\n",expr);
							return 0;
						}
#if TRACE_ASSEMBLE
	printf("try to set [%s] with %lf operatorassignment=%c\n",expr,v,operatorassignment);
#endif
						curdic=SearchDico(ae,expr,crc);
						if (curdic) {
							switch (operatorassignment) {
								default:printf("warning remover\n");break;
								case 0:curdic->v=v;break;
								case '+':curdic->v+=v;ptr_exp[-1]='+';break;
								case '-':curdic->v-=v;ptr_exp[-1]='-';break;
								case '*':curdic->v*=v;ptr_exp[-1]='*';break;
								case '/':curdic->v/=v;ptr_exp[-1]='/';break;
								/* bit operations */
								case '|':curdic->v=((int)curdic->v)|((int)v);ptr_exp[-1]='|';break;
								case '&':curdic->v=((int)curdic->v)&((int)v);ptr_exp[-1]='&';break;
								case '^':curdic->v=((int)curdic->v)^((int)v);ptr_exp[-1]='^';break;
								case '%':curdic->v=((int)curdic->v)%((int)v);ptr_exp[-1]='%';break;
								case ']':curdic->v=((int)curdic->v)>>((int)v);ptr_exp[-1]=']';
									 if (v>31 || v<-31) {
										if (!ae->nowarning) {
                                                                			rasm_printf(ae,KWARNING"Warning - shifting %d is architecture dependant, result forced to ZERO\n",(int)v);
															if (ae->erronwarn) MaxError(ae);
										}
										curdic->v=0;
									 }
									 break;
								case '[':curdic->v=((int)curdic->v)<<((int)v);ptr_exp[-1]='[';
									 if (v>31 || v<-31) {
										if (!ae->nowarning) {
                                                                			rasm_printf(ae,KWARNING"Warning - shifting %d is architecture dependant, result forced to ZERO\n",(int)v);
															if (ae->erronwarn) MaxError(ae);
										}
										curdic->v=0;
									 }
									 break;
							}
						} else {
							switch (operatorassignment) {
								default: /* cannot do operator on non existing variable */
									MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"Cannot do an operator assignment on non existing variable [%s]\n",expr);
									return 0;
								case 0: /* assign a new variable */
									ExpressionSetDicoVar(ae,expr,v);
									break;
							}
						}
						*ptr_exp='=';
						return v;
					}
				}
			}
			break;
		/*****************************************
		     P U R E    E X P R E S S I O N
		*****************************************/
		default:break;
	}
#if TRACE_ASSEMBLE
printf("pure expression to compute [%s]\n",expr);
#endif
	return ComputeExpressionCore(ae,expr,ptr,didx);
}
int RoundComputeExpression(struct s_assenv *ae,char *expr, int ptr, int didx, int expression_expected) {
	return floor(ComputeExpression(ae,expr,ptr,didx,expression_expected)+ae->rough);
}

/*
	ExpressionFastTranslate
	
	purpose: translate all known symbols in an expression (especially variables acting like counters)

0:
1:
2: (equ declaration)
*/
void ExpressionFastTranslate(struct s_assenv *ae, char **ptr_expr, int fullreplace)
{
	#undef FUNC
	#define FUNC "ExpressionFastTranslate"

	struct s_label *curlabel;
	struct s_expr_dico *curdic;
	static char *varbuffer=NULL;
	static int ivar,maxivar=1;
	char curval[256]={0};
	int c,lenw=0,idx=0,crc,startvar=0,newlen,ialias,found_replace,yves,dek,reidx,lenbuf,rlen,tagoffset;
	double v;
	char tmpuchar[16];
	char *expr,*locallabel;
	int curly=0,curlyflag=0;
	char *Automate;
	int recurse=-1,recursecount=0;
	
	if (!ae || !ptr_expr) {
		if (varbuffer) MemFree(varbuffer);
		varbuffer=NULL;
		maxivar=1;
		ivar=0;
		return;
	}
	/* be sure to have at least some bytes allocated */
	StateMachineResizeBuffer(&varbuffer,128,&maxivar);
	expr=*ptr_expr;

	ivar=0;

//printf("fast [%s]\n",expr);

	while (!ae->AutomateExpressionDecision[((int)expr[idx])&0xFF]) idx++;

	switch (ae->maxam) {
		default:
		case 0: /* full check */
			if (expr[idx]=='~' || (expr[idx]=='=' && expr[idx+1]!='=')) {reidx=idx+1;break;}
			reidx=0;
			break;
		case 1: /* partial check with maxam */
			if (expr[idx]=='~') {reidx=idx+1;break;}
			reidx=0;
			break;
	}

	idx=0;
	/* is there ascii char? */
	while ((c=expr[idx])!=0) {
		if (c=='\'' || c=='"') {
			/* one char escape code */
			if (expr[idx+1]=='\\') {
				if (expr[idx+2] && expr[idx+3]==c) {
					/* no charset conversion for escaped chars */
					c=expr[idx+2];
					switch (c) {
						case 'b':c='\b';break;
						case 'v':c='\v';break;
						case 'f':c='\f';break;
						case '0':c='\0';break;
						case 'r':c='\r';break;
						case 'n':c='\n';break;
						case 't':c='\t';break;
						default:break;
					}
					sprintf(tmpuchar,"#%03X",c);
					memcpy(expr+idx,tmpuchar,4);
					idx+=3;
				} else {
					MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"expression [%s] - Only single escaped char may be quoted\n",expr);
					expr[0]=0;
					return;
				}
			} else if (expr[idx+1] && expr[idx+2]==c) {
					sprintf(tmpuchar,"#%02X",ae->charset[(int)expr[idx+1]]);
					memcpy(expr+idx,tmpuchar,3);
					idx+=2;
			} else {
				//printf("FAST => moar than one quoted char\n");
				//MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"expression [%s] - Only single char may be quoted\n",expr);
				//expr[0]=0;
				//return;
				idx++;
				while (expr[idx] && expr[idx]!=c) idx++;
			}
		}
		idx++;
	}
	
	idx=reidx;
	while ((c=expr[idx])!=0) {
		switch (c) {
			/* string in expression */
			case '"':
			case '\'':
				//printf("FAST => skip string [%s]\n",expr);
				idx++;
				while (expr[idx] && expr[idx]!=c) idx++;
				if (expr[idx]) idx++;
				ivar=0;
				break;
			/* operator / parenthesis */
			case '!':
			case '=':
			case '>':
			case '<':
			case '(':
			case ')':
			case ']':
			case '[':
			case '*':
			case '/':
			case '+':
			case '~':
			case '-':
			case '^':
			case 'm':
			case '|':
			case '&':
				idx++;
				break;
			default:
				startvar=idx;
				if (ae->AutomateExpressionValidCharFirst[((int)c)&0xFF]) {
					varbuffer[ivar++]=c;
					if (c=='{') {
						/* this is only tag and not a formula */
						curly++;
					}
					StateMachineResizeBuffer(&varbuffer,ivar,&maxivar);
					idx++;
					c=expr[idx];

					Automate=ae->AutomateExpressionValidChar;
					while (Automate[((int)c)&0xFF]) {
						if (c=='{') {
							curly++;
							curlyflag=1;					
							Automate=ae->AutomateExpressionValidCharExtended;
						} else if (c=='}') {
							curly--;
							if (!curly) {
								Automate=ae->AutomateExpressionValidChar;
							}
						}
						varbuffer[ivar++]=c;
						StateMachineResizeBuffer(&varbuffer,ivar,&maxivar);
						idx++;
						c=expr[idx];
					}
				}
				varbuffer[ivar]=0;
				if (!ivar) {
					MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"invalid expression [%s] c=[%c] idx=%d\n",expr,c,idx);
					return;
				} else if (curly) {
					MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"wrong curly brackets in expression [%s]\n",expr);
					return;
				}
		}
		if (ivar && (varbuffer[0]<'0' || varbuffer[0]>'9')) {
			/* numbering var or label */
			if (curlyflag) {
				char *minivarbuffer;
				int touched;
				minivarbuffer=TranslateTag(ae,TxtStrDup(varbuffer), &touched,0,E_TAGOPTION_NONE|(fullreplace?0:E_TAGOPTION_PRESERVE));
				StateMachineResizeBuffer(&varbuffer,strlen(minivarbuffer)+1,&maxivar);
				strcpy(varbuffer,minivarbuffer);
				newlen=strlen(varbuffer);
				lenw=strlen(expr);
				/* must update source */
				if (newlen>ivar) {
					/* realloc bigger */
					expr=*ptr_expr=MemRealloc(expr,lenw+newlen-ivar+1);
				}
				if (newlen!=ivar ) {
					lenw=strlen(expr);
					MemMove(expr+startvar+newlen,expr+startvar+ivar,lenw-startvar-ivar+1);
				}
				strncpy(expr+startvar,minivarbuffer,newlen); /* copy without zero terminator */
				idx=startvar+newlen;
				/***/
				MemFree(minivarbuffer);
				curlyflag=0;
				/******* ivar must be updated in case of label or alias following ***********/
				ivar=newlen;
			}

			/* recherche dans dictionnaire et remplacement */
			crc=GetCRC(varbuffer);
			found_replace=0;
			/* pour les affectations ou les tests conditionnels on ne remplace pas le dico (pour le Push oui par contre!) */
			if (fullreplace) {
//printf("ExpressionFastTranslate (full) => varbuffer=[%s] lz=%d\n",varbuffer,ae->lz);
				if (varbuffer[0]=='$' && !varbuffer[1]) {
					if (ae->lz==-1) {
						#ifdef OS_WIN
						snprintf(curval,sizeof(curval)-1,"%d",ae->codeadr);
						newlen=strlen(curval);
						#else
						newlen=snprintf(curval,sizeof(curval)-1,"%d",ae->codeadr);
						#endif
						lenw=strlen(expr);
						if (newlen>ivar) {
							/* realloc bigger */
							expr=*ptr_expr=MemRealloc(expr,lenw+newlen-ivar+1);
						}
						if (newlen!=ivar ) {
							MemMove(expr+startvar+newlen,expr+startvar+ivar,lenw-startvar-ivar+1);
							found_replace=1;
						}
						strncpy(expr+startvar,curval,newlen); /* copy without zero terminator */
						idx=startvar+newlen;
						ivar=0;
					}
					/* qu'on le remplace ou pas on passe a la suite */
					found_replace=1;
				} else {
					curdic=SearchDico(ae,varbuffer,crc);
					if (curdic) {
						v=curdic->v;
//printf("ExpressionFastTranslate (full) -> replace var (%s=%0.1lf)\n",varbuffer,v);

						#ifdef OS_WIN
						snprintf(curval,sizeof(curval)-1,"%lf",v);
						newlen=TrimFloatingPointString(curval);
						#else
						snprintf(curval,sizeof(curval)-1,"%lf",v);
						newlen=TrimFloatingPointString(curval);
						#endif
						lenw=strlen(expr);
						if (newlen>ivar) {
							/* realloc bigger */
							expr=*ptr_expr=MemRealloc(expr,lenw+newlen-ivar+1);
						}
						if (newlen!=ivar ) {
							MemMove(expr+startvar+newlen,expr+startvar+ivar,lenw-startvar-ivar+1);
						}
						strncpy(expr+startvar,curval,newlen); /* copy without zero terminator */
						idx=startvar+newlen;
						ivar=0;
						found_replace=1;
					}
				}
			}
			/* on cherche aussi dans les labels existants => priorité aux modules!!! */   // modulmodif => pas utile?
			if (!found_replace) {
				curlabel=SearchLabel(ae,varbuffer,crc);
				if (curlabel) {
					if (!curlabel->lz || ae->stage>1) {
						yves=curlabel->ptr;

						#ifdef OS_WIN
						snprintf(curval,sizeof(curval)-1,"%d",yves);
						newlen=strlen(curval);
						#else
						newlen=snprintf(curval,sizeof(curval)-1,"%d",yves);
						#endif
						lenw=strlen(expr);
						if (newlen>ivar) {
							/* realloc bigger */
							expr=*ptr_expr=MemRealloc(expr,lenw+newlen-ivar+1);
						}
						if (newlen!=ivar ) {
							MemMove(expr+startvar+newlen,expr+startvar+ivar,lenw-startvar-ivar+1);
						}
						strncpy(expr+startvar,curval,newlen); /* copy without zero terminator */
						found_replace=1;
						idx=startvar+newlen;
						ivar=0;
					}
				}		
			}
			/* non trouve on cherche dans les alias */
			if (!found_replace) {
				if ((ialias=SearchAlias(ae,crc,varbuffer))>=0) {
					newlen=ae->alias[ialias].len;
					lenw=strlen(expr);
					/* infinite replacement check */
					if (recurse<=startvar) {
						/* recurse maximum count is a mix of alias len and alias number */
						if (recursecount>ae->ialias+ae->alias[ialias].len) {
							if (strchr(expr,'~')!=NULL) *strchr(expr,'~')=0;
							MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"alias definition of %s has infinite recursivity\n",expr);
							expr[0]=0; /* avoid some errors due to shitty definition */
							return;
						} else {
							recursecount++;
						}
					}
					if (newlen>ivar) {
						/* realloc bigger */
						expr=*ptr_expr=MemRealloc(expr,lenw+newlen-ivar+1);
					}
					if (newlen!=ivar) {
						MemMove(expr+startvar+newlen,expr+startvar+ivar,lenw-startvar-ivar+1);
					}
					strncpy(expr+startvar,ae->alias[ialias].translation,newlen); /* copy without zero terminator */
					found_replace=1;
					/* need to parse again alias because of delayed declarations */
					recurse=startvar;
					idx=startvar;
					ivar=0;
				} else {
				}
			}
			if (!found_replace) {
	//printf("fasttranslate test local label\n");
				/* non trouve c'est peut-etre un label local - mais pas de l'octal */
				if (varbuffer[0]=='@' && (varbuffer[1]<'0' || varbuffer[1]>'9')) {
					char *zepoint;
					lenbuf=strlen(varbuffer);
//printf("MakeLocalLabel(ae,varbuffer,&dek); (1)\n");
					locallabel=MakeLocalLabel(ae,varbuffer,&dek);
//printf("exprin =[%s]   rlen=%d dek-lenbuf=%d\n",expr,rlen,dek-lenbuf);
					/*** le grand remplacement ***/
					/* local to macro or loop */
					rlen=strlen(expr+startvar+lenbuf)+1;
					expr=*ptr_expr=MemRealloc(expr,strlen(expr)+dek+1);
					/* move end of expression in order to insert local ID */
					zepoint=strchr(varbuffer,'.');
					if (zepoint) {
						/* far proximity access */
						int suffixlen,dotpos;
						dotpos=(zepoint-varbuffer);
						suffixlen=lenbuf-dotpos;

						MemMove(expr+startvar+dotpos+dek,expr+startvar+dotpos,rlen+suffixlen);
						strncpy(expr+startvar+dotpos,locallabel,dek);
					} else {
						/* legacy */
						MemMove(expr+startvar+lenbuf+dek,expr+startvar+lenbuf,rlen);
						strncpy(expr+startvar+lenbuf,locallabel,dek);
					}
					idx+=dek;
					MemFree(locallabel);
					found_replace=1;
//printf("exprout=[%s]\n",expr);
				} else if (varbuffer[0]=='.' && (varbuffer[1]<'0' || varbuffer[1]>'9')) {
					/* proximity label */
					lenbuf=strlen(varbuffer);
//printf("MakeLocalLabel(ae,varbuffer,&dek); (2)\n");
					locallabel=MakeLocalLabel(ae,varbuffer,&dek);
					/*** le grand remplacement ***/
					rlen=strlen(expr+startvar+lenbuf)+1;
					dek=strlen(locallabel);
//printf("exprin =[%s]   rlen=%d dek-lenbuf=%d\n",expr,rlen,dek-lenbuf);
					expr=*ptr_expr=MemRealloc(expr,strlen(expr)+dek-lenbuf+1);
					MemMove(expr+startvar+dek,expr+startvar+lenbuf,rlen);
					strncpy(expr+startvar,locallabel,dek);
					idx+=dek-lenbuf;
					MemFree(locallabel);
//printf("exprout=[%s]\n",expr);

//@@TODO ajouter une recherche d'alias?

				} else if (varbuffer[0]=='{') {
					if (strncmp(varbuffer,"{BANK}",6)==0 || strncmp(varbuffer,"{PAGE}",6)==0) tagoffset=6; else
					if (strncmp(varbuffer,"{PAGESET}",9)==0) tagoffset=9; else
					if (strncmp(varbuffer,"{SIZEOF}",8)==0) tagoffset=8; else
					{
						tagoffset=0;
						MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"Unknown prefix tag\n");
					}
				
					if (varbuffer[tagoffset]=='.') {
						int lenadd;
						startvar+=tagoffset;
						lenbuf=strlen(varbuffer+tagoffset);
						locallabel=MakeLocalLabel(ae,varbuffer+tagoffset,NULL);
						/*** le grand remplacement meriterait une modif pour DEK dans MakeLocalLabel ***/
						rlen=strlen(expr+startvar+lenbuf)+1;
						lenadd=strlen(locallabel)-strlen(varbuffer+tagoffset);
						expr=*ptr_expr=MemRealloc(expr,strlen(expr)+lenadd+1);

//printf("expr[%s] move to %d from %d len=%d\n",expr,startvar+lenadd,startvar,rlen+lenbuf);
						MemMove(expr+startvar+lenadd,expr+startvar,rlen+lenbuf);
						strncpy(expr+startvar,locallabel,lenadd);

						MemFree(locallabel);
						found_replace=1;
						idx+=lenadd;
					} else	if (varbuffer[tagoffset]=='@') {
						char *zepoint;
						startvar+=tagoffset;
						lenbuf=strlen(varbuffer+tagoffset);
//printf("MakeLocalLabel(ae,varbuffer,&dek); (3)\n");
						locallabel=MakeLocalLabel(ae,varbuffer+tagoffset,&dek);
//printf("local [%s] =>",locallabel);
						/*** le grand remplacement ***/
						rlen=strlen(expr+startvar+lenbuf)+1;
						expr=*ptr_expr=MemRealloc(expr,strlen(expr)+dek+1);
						/* move end of expression in order to insert local ID */
						zepoint=strchr(varbuffer+tagoffset,'.'); // +tagoffset
						if (zepoint) {
							/* far proximity access */
							int suffixlen,dotpos;
							dotpos=(zepoint-varbuffer);
							suffixlen=lenbuf-dotpos;
		//printf("prox [%s] => ",expr);
							MemMove(expr+startvar+dotpos+dek,expr+startvar+dotpos,rlen+suffixlen);
							strncpy(expr+startvar+dotpos,locallabel,dek);
						} else {
							/* legacy */
		//printf("legacy [%s] => ",expr);
							MemMove(expr+startvar+lenbuf+dek,expr+startvar+lenbuf,rlen);
							strncpy(expr+startvar+lenbuf,locallabel,dek);
						}
						idx+=dek;
						MemFree(locallabel);
						found_replace=1;
	//printf("exprout=[%s]\n",expr);
					} else if (varbuffer[tagoffset]=='$') {
						int tagvalue=-1;
						/*
						 * There is no {SLOT}$ support...
						 */
						if (strcmp(varbuffer,"{BANK}$")==0) {
							if (ae->forcecpr) {
								if (ae->activebank<32) {
									tagvalue=ae->activebank;
								} else {
									MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"expression [%s] cannot use BANK $ in a temporary space!\n",TradExpression(expr));
									tagvalue=0;
								}
							} else if (ae->forcesnapshot) {
								if (ae->activebank<BANK_MAX_NUMBER) {
									/* on autorise le préfixe BANK en snapshot avec une subtilité */
								if (ae->bankset[ae->activebank>>2]) {
									tagvalue=ae->activebank+(ae->codeadr>>14); /* dans un bankset on tient compte de l'adresse */
								} else {
									tagvalue=ae->activebank;
								}
									
								} else {
									MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"expression [%s] cannot use BANK $ in a temporary space!\n",TradExpression(expr));
									tagvalue=0;
								}
							}
						} else if (strcmp(varbuffer,"{PAGE}$")==0) {
							if (ae->activebank<BANK_MAX_NUMBER) {
								if (ae->bankset[ae->activebank>>2]) {
									tagvalue=ae->bankgate[(ae->activebank&0x1FC)+(ae->codeadr>>14)];
								} else {
									tagvalue=ae->bankgate[ae->activebank];
								}
							} else {
								MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"expression [%s] cannot use PAGE $ in a temporary space!\n",TradExpression(expr));
								tagvalue=ae->activebank;
							}
						} else if (strcmp(varbuffer,"{PAGESET}$")==0) {
							if (ae->activebank<BANK_MAX_NUMBER) {
								tagvalue=ae->setgate[ae->activebank];
								//if (ae->activebank>3) tagvalue=((ae->activebank>>2)-1)*8+0xC2; else tagvalue=0xC0;
							} else {
								MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"expression [%s] cannot use PAGESET $ in a temporary space!\n",TradExpression(expr));
								tagvalue=ae->activebank;
							}
						}
						/* replace */
						#ifdef OS_WIN
						snprintf(curval,sizeof(curval)-1,"%d",tagvalue);
						newlen=strlen(curval);
						#else
						newlen=snprintf(curval,sizeof(curval)-1,"%d",tagvalue);
						#endif
						lenw=strlen(expr);
						if (newlen>ivar) {
							/* realloc bigger */
							expr=*ptr_expr=MemRealloc(expr,lenw+newlen-ivar+1);
						}
						if (newlen!=ivar ) {
							MemMove(expr+startvar+newlen,expr+startvar+ivar,lenw-startvar-ivar+1);
							found_replace=1;
						}
						strncpy(expr+startvar,curval,newlen); /* copy without zero terminator */
						idx=startvar+newlen;
						ivar=0;
						found_replace=1;
					}
				}
			}
			
			
			
			
			
			
			if (!found_replace && strcmp(varbuffer,"REPEAT_COUNTER")==0) {
				if (ae->ir) {
					yves=ae->repeat[ae->ir-1].repeat_counter;
					#ifdef OS_WIN
					snprintf(curval,sizeof(curval)-1,"%d",yves);
					newlen=strlen(curval);
					#else
					newlen=snprintf(curval,sizeof(curval)-1,"%d",yves);
					#endif
					lenw=strlen(expr);
					if (newlen>ivar) {
						/* realloc bigger */
						expr=*ptr_expr=MemRealloc(expr,lenw+newlen-ivar+1);
					}
					if (newlen!=ivar ) {
						MemMove(expr+startvar+newlen,expr+startvar+ivar,lenw-startvar-ivar+1);
						found_replace=1;
					}
					strncpy(expr+startvar,curval,newlen); /* copy without zero terminator */
					found_replace=1;
					idx=startvar+newlen;
					ivar=0;
				} else {
					MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"cannot use REPEAT_COUNTER outside repeat loop\n");
				}
			}
			if (!found_replace && strcmp(varbuffer,"WHILE_COUNTER")==0) {
				if (ae->iw) {
					yves=ae->whilewend[ae->iw-1].while_counter;
					#ifdef OS_WIN
					snprintf(curval,sizeof(curval)-1,"%d",yves);
					newlen=strlen(curval);
					#else
					newlen=snprintf(curval,sizeof(curval)-1,"%d",yves);
					#endif
					lenw=strlen(expr);
					if (newlen>ivar) {
						/* realloc bigger */
						expr=*ptr_expr=MemRealloc(expr,lenw+newlen-ivar+1);
					}
					if (newlen!=ivar ) {
						MemMove(expr+startvar+newlen,expr+startvar+ivar,lenw-startvar-ivar+1);
						found_replace=1;
					}
					strncpy(expr+startvar,curval,newlen); /* copy without zero terminator */
					found_replace=1;
					idx=startvar+newlen;
					ivar=0;
				} else {
					MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"cannot use WHILE_COUNTER outside repeat loop\n");
				}
			}
			/* unknown symbol -> add to used symbol pool */
			if (!found_replace) {
				InsertUsedToTree(ae,varbuffer,crc);
			}
		}
		ivar=0;
	}
}

void PushExpression(struct s_assenv *ae,int iw,enum e_expression zetype)
{
	#undef FUNC
	#define FUNC "PushExpression"
	
	struct s_expression curexp={0};
	int startptr=0;

	if (!ae->nocode) {
		curexp.iw=iw;
		curexp.wptr=ae->outputadr;
		curexp.zetype=zetype;
		curexp.ibank=ae->activebank;
		curexp.iorgzone=ae->io-1;
		curexp.lz=ae->lz;
		/* need the module to know where we are */
		if (ae->module) curexp.module=TxtStrDup(ae->module); else curexp.module=NULL;
		/* on traduit de suite les variables du dictionnaire pour les boucles et increments
			SAUF si c'est une affectation 
		*/
		if (!ae->wl[iw].e) {
			switch (zetype) {
				case E_EXPRESSION_V16C:
 					/* check non register usage */
					switch (GetCRC(ae->wl[iw].w)) {
						case CRC_IX:
						case CRC_IY:
						case CRC_MIX:
						case CRC_MIY:
							MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"invalid register usage\n",ae->maxptr);
						default:break;
					}
				case E_EXPRESSION_J8:
				case E_EXPRESSION_V8:
				case E_EXPRESSION_V16:
				case E_EXPRESSION_IM:startptr=-1;
							break;
				case E_EXPRESSION_IV8:
				case E_EXPRESSION_IV81:
				case E_EXPRESSION_IV16:startptr=-2;
							break;
				case E_EXPRESSION_3V8:startptr=-3;
							break;
				case E_EXPRESSION_RUN:
				case E_EXPRESSION_ZXRUN:
				case E_EXPRESSION_ZXSTACK:
				case E_EXPRESSION_BRS:break;
				default:break;
			}
			/* hack pourri pour gérer le $ */
			ae->codeadr+=startptr;
			/* ok mais les labels locaux des macros? */
			if (ae->ir || ae->iw || ae->imacro) {
				curexp.reference=TxtStrDup(ae->wl[iw].w);
				ExpressionFastTranslate(ae,&curexp.reference,1);
			} else {
				ExpressionFastTranslate(ae,&ae->wl[iw].w,1);
			}
			ae->codeadr-=startptr;
		}
		/* calcul adresse de reference et post-incrementation pour sauter les data */
		switch (zetype) {
			case E_EXPRESSION_J8:curexp.ptr=ae->codeadr-1;ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_0V8:curexp.ptr=ae->codeadr;ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_V8:curexp.ptr=ae->codeadr-1;ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_0V16:curexp.ptr=ae->codeadr;ae->outputadr+=2;ae->codeadr+=2;break;
			case E_EXPRESSION_0V32:curexp.ptr=ae->codeadr;ae->outputadr+=4;ae->codeadr+=4;break;
			case E_EXPRESSION_0VR:curexp.ptr=ae->codeadr;ae->outputadr+=5;ae->codeadr+=5;break;
			case E_EXPRESSION_0VRMike:curexp.ptr=ae->codeadr;ae->outputadr+=5;ae->codeadr+=5;break;
			case E_EXPRESSION_V16C:
			case E_EXPRESSION_V16:curexp.ptr=ae->codeadr-1;ae->outputadr+=2;ae->codeadr+=2;break;
			case E_EXPRESSION_IV81:curexp.ptr=ae->codeadr-2;ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_IV8:curexp.ptr=ae->codeadr-2;ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_3V8:curexp.ptr=ae->codeadr-3;ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_IV16:curexp.ptr=ae->codeadr-2;ae->outputadr+=2;ae->codeadr+=2;break;
			case E_EXPRESSION_RST:curexp.ptr=ae->codeadr;ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_IM:curexp.ptr=ae->codeadr-1;ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_RUN:break;
			case E_EXPRESSION_ZXRUN:break;
			case E_EXPRESSION_ZXSTACK:break;
			case E_EXPRESSION_BRS:curexp.ptr=ae->codeadr;break; // minimum syndical
			default:break;
		}
		/* le contrôle n'est pas bon avec les DEFB, DEFW, ...  -> @@TODO */
		if (ae->outputadr<=ae->maxptr) {
			ObjectArrayAddDynamicValueConcat((void **)&ae->expression,&ae->ie,&ae->me,&curexp,sizeof(curexp));
		} else {
			/* to avoid double error message */
			if (!ae->stop) MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"output exceed limit %d\n",ae->maxptr); else MaxError(ae);
			ae->stop=1;
			return;
		}
	} else {
		switch (zetype) {
			case E_EXPRESSION_J8:ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_0V8:ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_V8:ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_0V16:ae->outputadr+=2;ae->codeadr+=2;break;
			case E_EXPRESSION_0V32:ae->outputadr+=4;ae->codeadr+=4;break;
			case E_EXPRESSION_0VR:ae->outputadr+=5;ae->codeadr+=5;break;
			case E_EXPRESSION_0VRMike:ae->outputadr+=5;ae->codeadr+=5;break;
			case E_EXPRESSION_V16C:
			case E_EXPRESSION_V16:ae->outputadr+=2;ae->codeadr+=2;break;
			case E_EXPRESSION_IV81:ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_IV8:ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_3V8:ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_IV16:ae->outputadr+=2;ae->codeadr+=2;break;
			case E_EXPRESSION_RST:ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_IM:ae->outputadr++;ae->codeadr++;break;
			case E_EXPRESSION_RUN:break;
			case E_EXPRESSION_ZXRUN:break;
			case E_EXPRESSION_ZXSTACK:break;
			case E_EXPRESSION_BRS:break;
		}
		if (ae->outputadr<=ae->maxptr) {
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"NOCODE output exceed limit %d\n",ae->maxptr);
			FreeAssenv(ae);exit(3);
		}
	}
}

/*
The CP/M 2.2 directory has only one type of entry:

UU F1 F2 F3 F4 F5 F6 F7 F8 T1 T2 T3 EX S1 S2 RC   .FILENAMETYP....
AL AL AL AL AL AL AL AL AL AL AL AL AL AL AL AL   ................

UU = User number. 0-15 (on some systems, 0-31). The user number allows multiple
    files of the same name to coexist on the disc. 
     User number = 0E5h => File deleted
Fn - filename
Tn - filetype. The characters used for these are 7-bit ASCII.
       The top bit of T1 (often referred to as T1') is set if the file is 
     read-only.
       T2' is set if the file is a system file (this corresponds to "hidden" on 
     other systems). 
EX = Extent counter, low byte - takes values from 0-31
S2 = Extent counter, high byte.

      An extent is the portion of a file controlled by one directory entry.
    If a file takes up more blocks than can be listed in one directory entry,
    it is given multiple entries, distinguished by their EX and S2 bytes. The
    formula is: Entry number = ((32*S2)+EX) / (exm+1) where exm is the 
    extent mask value from the Disc Parameter Block.

S1 - reserved, set to 0.
RC - Number of records (1 record=128 bytes) used in this extent, low byte.
    The total number of records used in this extent is

    (EX & exm) * 128 + RC

    If RC is 80h, this extent is full and there may be another one on the disc.
    File lengths are only saved to the nearest 128 bytes.

AL - Allocation. Each AL is the number of a block on the disc. If an AL
    number is zero, that section of the file has no storage allocated to it
    (ie it does not exist). For example, a 3k file might have allocation 
    5,6,8,0,0.... - the first 1k is in block 5, the second in block 6, the 
    third in block 8.
     AL numbers can either be 8-bit (if there are fewer than 256 blocks on the
    disc) or 16-bit (stored low byte first). 
*/
int EDSK_getblockid(int *fb) {
	#undef FUNC
	#define FUNC "EDSK_getblockid"
	
	int i;
	for (i=0;i<180;i++) {
		if (fb[i]) {
			return i;
		}
	}
	return -1;
}
int EDSK_getdirid(struct s_edsk_wrapper *curwrap) {
	#undef FUNC
	#define FUNC "EDSK_getdirid"
	
	int ie;
	for (ie=0;ie<64;ie++) {
		if (curwrap->entry[ie].user==0xE5) {
#if TRACE_EDSK
	printf("getdirid returns %d\n",ie);
#endif
			return ie;
		}
	}
	return -1;
}
char *MakeAMSDOS_name(struct s_assenv *ae, char *reference_filename)
{
	#undef FUNC
	#define FUNC "MakeAMSDOS_name"

	static char amsdos_name[12];
	char *filename,*jo;
	int i,ia;
	char *pp;

	/* remove path */
	filename=reference_filename;
	while ((jo=strchr(filename,'/'))!=NULL) filename=jo+1;
	while ((jo=strchr(filename,'\\'))!=NULL) filename=jo+1;

	/* warning */
	if (strlen(filename)>12) {
		if (!ae->nowarning) {
			rasm_printf(ae,KWARNING"Warning - filename [%s] too long for AMSDOS, will be truncated\n",filename);
			if (ae->erronwarn) MaxError(ae);
		}
	} else if ((pp=strchr(filename,'.'))!=NULL) {
		if (pp-filename>8) {
			if (!ae->nowarning) {
				rasm_printf(ae,KWARNING"Warning - filename [%s] too long for AMSDOS, will be truncated\n",filename);
			if (ae->erronwarn) MaxError(ae);
			}
		}
	}
	/* copy filename */
	for (i=0;filename[i]!=0 && filename[i]!='.' && i<8;i++) {
		amsdos_name[i]=toupper(filename[i]);
	}
	/* fill with spaces */
	for (ia=i;ia<8;ia++) {
		amsdos_name[ia]=0x20;
	}
	/* looking for extension */
	for (;filename[i]!=0 && filename[i]!='.';i++);
	/* then copy it if any */
	if (filename[i]=='.') {
		i++;
		for (ia=0;filename[i]!=0 && ia<3;ia++) {
			amsdos_name[8+ia]=toupper(filename[i++]);
		}
	}
	amsdos_name[11]=0;
#if TRACE_EDSK
	printf("MakeAMSDOS_name [%s] -> [%s]\n",filename,amsdos_name);
#endif
	return amsdos_name;
}


void EDSK_load(struct s_assenv *ae,struct s_edsk_wrapper *curwrap, char *edskfilename, int face)
{
	#undef FUNC
	#define FUNC "EDSK_load"

	unsigned char header[256];
	unsigned char *data;
	int tracknumber,sidenumber,tracksize,disksize;
	int i,b,s,f,t,curtrack,sectornumber,sectorsize,sectorid,reallength;
	int currenttrackposition=0,currentsectorposition,tmpcurrentsectorposition;
	unsigned char checksectorid[9];
	int curblock=0,curoffset=0;
#if TRACE_EDSK
	printf("EDSK_Load('%s',%d);",edskfilename,face);
#endif
	if (FileReadBinary(edskfilename,(char*)&header,0x100)!=0x100) {
		rasm_printf(ae,KERROR"Cannot read EDSK header of [%s]!\n",edskfilename);
		FreeAssenv(ae);exit(ABORT_ERROR);
	}
	if (strncmp((char *)header,"MV - CPC",8)==0) {
		rasm_printf(ae,KAYGREEN"updating DSK to EDSK [%s] / creator: %s",edskfilename,header+34);
		
		tracknumber=header[34+14];
		sidenumber=header[34+14+1];
		tracksize=header[34+14+1+1]+header[34+14+1+1+1]*256;
		rasm_printf(ae,"tracks: %d  sides:%d   track size:%d",tracknumber,sidenumber,tracksize);
		if (tracknumber>40 || sidenumber>2) {
			rasm_printf(ae,KERROR"[%s] DSK format is not supported in update mode (ntrack=%d nside=%d)\n",edskfilename,tracknumber,sidenumber);
			FreeAssenv(ae);exit(ABORT_ERROR);
		}
		if (face>=sidenumber) {
			rasm_printf(ae,KWARNING"[%s] Warning - DSK has no face %d - DSK updated\n",edskfilename,face);
			if (ae->erronwarn) MaxError(ae);
			return;
		}

		data=MemMalloc(tracksize*tracknumber*sidenumber);
		memset(data,0,tracksize*tracknumber*sidenumber);
		if (FileReadBinary(edskfilename,(char *)data,tracksize*tracknumber*sidenumber)!=tracksize*tracknumber*sidenumber) {
			rasm_printf(ae,"Cannot read DSK tracks!");
			FreeAssenv(ae);exit(ABORT_ERROR);
		}
		//loginfo("track data read (%dkb)",tracksize*tracknumber*sidenumber/1024);
		f=face;
		for (t=0;t<tracknumber;t++) {
			curtrack=t*sidenumber+f;

			i=(t*sidenumber+f)*tracksize;
			if (strncmp((char *)data+i,"Track-Info\r\n",12)) {
				rasm_printf(ae,"Invalid track information block side %d track %d",f,t);
				FreeAssenv(ae);exit(ABORT_ERROR);
			}
			sectornumber=data[i+21];
			sectorsize=data[i+20];
			if (sectornumber!=9 || sectorsize!=2) {
				rasm_printf(ae,"Cannot read [%s] Invalid DATA format",edskfilename);
				FreeAssenv(ae);exit(ABORT_ERROR);
			}
			memset(checksectorid,0,sizeof(checksectorid));			
			/* we want DATA format */
			for (s=0;s<sectornumber;s++) {
				if (t!=data[i+24+8*s]) {
					rasm_printf(ae,"Invalid track number in sector %02X track %d",data[i+24+8*s+2],t);
					FreeAssenv(ae);exit(ABORT_ERROR);
				}
				if (f!=data[i+24+8*s+1]) {
					rasm_printf(ae,"Invalid side number in sector %02X track %d",data[i+24+8*s+2],t);
					FreeAssenv(ae);exit(ABORT_ERROR);
				}
				if (data[i+24+8*s+2]<0xC1 || data[i+24+8*s+2]>0xC9) {
					rasm_printf(ae,"Invalid sector ID in sector %02X track %d",data[i+24+8*s+2],t);
					FreeAssenv(ae);exit(ABORT_ERROR);
				} else {
					checksectorid[data[i+24+8*s+2]-0xC1]=1;
				}				
				if (data[i+24+8*s+3]!=2) {
					rasm_printf(ae,"Invalid sector size in sector %02X track %d",data[i+24+8*s+2],t);
					FreeAssenv(ae);exit(ABORT_ERROR);
				}
			}
			for (s=0;s<sectornumber;s++) {
				if (!checksectorid[s]) {
					rasm_printf(ae,"Missing sector %02X track %d",s+0xC1,t);
					FreeAssenv(ae);exit(ABORT_ERROR);
				}
			}
			/* piste à piste on lit les blocs DANS L'ORDRE LOGIQUE!!! */
			for (b=0xC1;b<=0xC9;b++)
			for (s=0;s<sectornumber;s++) {
				if (data[i+24+8*s+2]==b) {
					memcpy(&curwrap->blocks[curblock][curoffset],&data[i+0x100+s*512],512);
					curoffset+=512;
					if (curoffset>=1024) {
						curoffset=0;
						curblock++;
					}
				}
			}
		}
	} else if (strncmp((char *)header,"EXTENDED",8)==0) {
		rasm_printf(ae,KAYGREEN"updating EDSK [%s] / creator: %-14.14s\n",edskfilename,header+34);
		tracknumber=header[34+14];
		sidenumber=header[34+14+1];
		// not in EDSK tracksize=header[34+14+1+1]+header[34+14+1+1+1]*256;
#if TRACE_EDSK
		loginfo("tracks: %d  sides:%d",tracknumber,sidenumber);
#endif

		if (sidenumber>2) {
			rasm_printf(ae,KERROR"[%s] EDSK format is not supported in update mode (ntrack=%d nside=%d)\n",edskfilename,tracknumber,sidenumber);
			FreeAssenv(ae);exit(ABORT_ERROR);
		}
		if (face>=sidenumber) {
			rasm_printf(ae,KWARNING"[%s] EDSK has no face %d - DSK updated\n",edskfilename,face);
			if (ae->erronwarn) MaxError(ae);
			return;
		}

		for (i=disksize=0;i<tracknumber*sidenumber;i++) disksize+=header[0x34+i]*256;
#if TRACE_EDSK
	loginfo("total track size: %dkb",disksize/1024);
#endif

		data=MemMalloc(disksize);
		memset(data,0,disksize);
		if (FileReadBinary(edskfilename,(char *)data,disksize)!=disksize) {
			rasm_printf(ae,KERROR"Cannot read DSK tracks!\n");
			FreeAssenv(ae);exit(ABORT_ERROR);
		}

		f=face;
		for (t=0;t<tracknumber && t<40;t++) {
			int track_sectorsize;

			curtrack=t*sidenumber+f;
			i=currenttrackposition;
			currentsectorposition=i+0x100;

			if (!header[0x34+curtrack] && t<40) {
				rasm_printf(ae,KERROR"Unexpected unformated track Side %d Track %02d\n",f,t);
			} else {
				currenttrackposition+=header[0x34+curtrack]*256;

				if (strncmp((char *)data+i,"Track-Info\r\n",12)) {
					rasm_printf(ae,KERROR"Invalid track information block side %d track %d\n",f,t);
					FreeAssenv(ae);exit(ABORT_ERROR);
				}
				sectornumber=data[i+21];
				track_sectorsize=data[i+20];
				if (sectornumber!=9) {
					rasm_printf(ae,KERROR"Unsupported track %d (sectornumber=%d sectorsize=%d)\n",t,sectornumber,sectorsize);
					FreeAssenv(ae);exit(ABORT_ERROR);
				}
				memset(checksectorid,0,sizeof(checksectorid));			
				/* we want DATA format */
				for (s=0;s<sectornumber;s++) {
					sectorid=data[i+24+8*s+2];
					if (sectorid>=0xC1 && sectorid<=0xC9) checksectorid[sectorid-0xC1]=1; else {
						rasm_printf(ae,KERROR"invalid sector id %02X for DATA track %d\n",sectorid,t);
						return;
					}
					sectorsize=data[i+24+8*s+3];
					if (sectorsize!=2) {
						rasm_printf(ae,KERROR"invalid sector size track %d\n",t);
						return;
					}
					reallength=data[i+24+8*s+6]+data[i+24+8*s+7]*256; /* real length stored */
					if (reallength!=512) {
						rasm_printf(ae,KERROR"invalid sector length %d for track %d\n",reallength,t);
						return;
					}
#if TRACE_EDSK
	printf("%02X ",sectorid);
#endif
				}
				if (track_sectorsize!=2) {
					rasm_printf(ae,KWARNING"track %02d has invalid sector size but sectors are OK\n",t);
			if (ae->erronwarn) MaxError(ae);
				}
#if TRACE_EDSK
	printf("\n");
#endif

				/* piste à piste on lit les blocs DANS L'ORDRE LOGIQUE!!! */
				for (b=0xC1;b<=0xC9;b++) {
					tmpcurrentsectorposition=currentsectorposition;
					for (s=0;s<sectornumber;s++) {
						if (b==data[i+24+8*s+2]) {
							memcpy(&curwrap->blocks[curblock][curoffset],&data[tmpcurrentsectorposition],512);
							curoffset+=512;
							if (curoffset>=1024) {
								curoffset=0;
								curblock++;
							}
						}
						reallength=data[i+24+8*s+6]+data[i+24+8*s+7]*256;
						tmpcurrentsectorposition+=reallength;
					}
				}
			}
		}
		
		
	} else {
		rasm_printf(ae,KERROR"file [%s] is not a valid (E)DSK floppy image\n",edskfilename);
		FreeAssenv(ae);exit(-923);
	}
	FileReadBinaryClose(edskfilename);
	
	/* Rasm management of (e)DSK files is AMSDOS compatible, just need to copy CATalog blocks but sort them... */
	memcpy(&curwrap->entry[0],curwrap->blocks[0],1024);
	memcpy(&curwrap->entry[32],curwrap->blocks[1],1024);
	/* tri des entrées selon le user */
	qsort(curwrap->entry,64,sizeof(struct s_edsk_wrapper_entry),cmpAmsdosentry);
	curwrap->nbentry=64;
	for (i=0;i<64;i++) {
		if (curwrap->entry[i].user==0xE5) {
			curwrap->nbentry=i;
			break;
		}
	}
#if TRACE_EDSK
	printf("%d entr%s found\n",curwrap->nbentry,curwrap->nbentry>1?"ies":"y");
	for (i=0;i<curwrap->nbentry;i++) {
		printf("[%02d] - ",i);
		if (curwrap->entry[i].user<16) {
			printf("U%02d [%-8.8s.%c%c%c] %c%c subcpt=#%02X rc=#%02X blocks=",curwrap->entry[i].user,curwrap->entry[i].filename,
			curwrap->entry[i].filename[8]&0x7F,curwrap->entry[i].filename[9]&0x7F,curwrap->entry[i].filename[10],
			curwrap->entry[i].filename[8]&0x80?'P':'-',curwrap->entry[i].filename[9]&0x80?'H':'-',
			curwrap->entry[i].subcpt,curwrap->entry[i].rc);
			for (b=0;b<16;b++) if (curwrap->entry[i].blocks[b]) printf("%s%02X",b>0?" ":"",curwrap->entry[i].blocks[b]); else printf("%s  ",b>0?" ":"");
			if (i&1) printf("\n"); else printf(" | ");
		} else {
			printf("free entry                  =    rc=    blocks=                                               ");
			if (i&1) printf("\n"); else printf(" | ");
		}
	}
	if (i&1) printf("\n");
#endif
}

struct s_edsk_wrapper *EDSK_select(struct s_assenv *ae,char *edskfilename, int facenumber)
{
	#undef FUNC
	#define FUNC "EDSK_select"
	
	struct s_edsk_wrapper newwrap={0},*curwrap=NULL;
	int i;
#if TRACE_EDSK
	printf("EDSK_select('%s',%d);\n",edskfilename,facenumber);
#endif
	/* check if there is a DSK in memory */
	for (i=0;i<ae->nbedskwrapper;i++) {
		if (!strcmp(ae->edsk_wrapper[i].edsk_filename,edskfilename)) {
#if TRACE_EDSK
	printf("Found! return %d\n",i);
#endif
			return &ae->edsk_wrapper[i];
		}
	}
	/* not in memory, create an empty struct */
	newwrap.edsk_filename=TxtStrDup(edskfilename);
	memset(newwrap.entry,0xE5,sizeof(struct s_edsk_wrapper_entry)*64);
	memset(newwrap.blocks[0],0xE5,1024);
	memset(newwrap.blocks[1],0xE5,1024);
#if TRACE_EDSK
	printf("Not found! create empty struct\n");
#endif
	newwrap.face=facenumber;
	ObjectArrayAddDynamicValueConcat((void**)&ae->edsk_wrapper,&ae->nbedskwrapper,&ae->maxedskwrapper,&newwrap,sizeof(struct s_edsk_wrapper));
	/* and load files if the DSK exists on disk */
	curwrap=&ae->edsk_wrapper[ae->nbedskwrapper-1];
	if (FileExists(edskfilename)) {
		EDSK_load(ae,curwrap,edskfilename,facenumber);
	}
	return curwrap;
}

int EDSK_addfile(struct s_assenv *ae,char *edskfilename,int facenumber, char *filename,unsigned char *indata,int insize, int offset, int run)
{
	#undef FUNC
	#define FUNC "EDSK_addfile"

	struct s_edsk_wrapper *curwrap=NULL;
	char amsdos_name[12]={0};
	int j,i,ia,ib,ie,filesize,idxdata;
	int fb[180],rc,idxb;
	unsigned char *data=NULL;
	int size=0;
	int firstblock;

	curwrap=EDSK_select(ae,edskfilename,facenumber);
	/* update struct */
	size=insize+128;
	data=MemMalloc(size);
	strcpy(amsdos_name,MakeAMSDOS_name(ae,filename));
	memcpy(data,MakeAMSDOSHeader(run,offset,offset+insize,amsdos_name),128);
	memcpy(data+128,indata,insize);
	/* overwrite check */
#if TRACE_EDSK
	printf("EDSK_addfile will checks %d entr%s for [%s]\n",curwrap->nbentry,curwrap->nbentry>1?"ies":"y",amsdos_name);
#endif
	for (i=0;i<curwrap->nbentry;i++) {
		if (!strncmp((char *)curwrap->entry[i].filename,amsdos_name,11)) {
			if (!ae->edskoverwrite) {
				MakeError(ae,NULL,0,"Error - Cannot save [%s] in edsk [%s] with overwrite disabled as the file already exists\n",amsdos_name,edskfilename);
				MemFree(data);
				return 0;
			} else {
				/* overwriting previous file */
#if TRACE_EDSK
	printf(" -> reset previous entry %d with 0xE5\n",i);
#endif
				memset(&curwrap->entry[i],0xE5,sizeof(struct s_edsk_wrapper_entry));
			}
		}
	}
	/* find free blocks */
#if TRACE_EDSK
	printf("EDSK_addfile find free blocks\n");
#endif
	fb[0]=fb[1]=0;
	for (i=2;i<180;i++) fb[i]=1;
	for (i=0;i<curwrap->nbentry;i++) {
		if (curwrap->entry[i].rc!=0xE5 && curwrap->entry[i].rc!=0) {
			/* entry found, compute number of blocks to read */
			rc=curwrap->entry[i].rc/8;
			if (curwrap->entry[i].rc%8) rc++; /* adjust value */
			/* mark as used */
			for (j=0;j<rc;j++) {
				fb[curwrap->entry[i].blocks[j]]=0;
			}
		}
	}
	/* set directory, blocks and data in blocks */
	firstblock=-1;
	filesize=size;
	idxdata=0;
	ia=0;

#if TRACE_EDSK
	printf("Writing [%s] size=%d\n",amsdos_name,size);
#endif

	while (filesize>0) {
		if (filesize>16384) {
			/* extended entry */
#if TRACE_EDSK
	printf("extended entry for file (filesize=%d)\nblocklist: ",filesize);
#endif
			if ((ie=EDSK_getdirid(curwrap))==-1)  {
				MakeError(ae,NULL,0,"Error - edsk [%s] DIRECTORY FULL\n",edskfilename);
				MemFree(data);
				return 0;
			}
			if (curwrap->nbentry<=ie) curwrap->nbentry=ie+1;
			idxb=0;
			for (i=0;i<16;i++) {
				if ((ib=EDSK_getblockid(fb))==-1) {
					MakeError(ae,NULL,0,"Error - edsk [%s] DISK FULL\n",edskfilename);
					MemFree(data);
					return 0;
				} else {
					if (firstblock==-1) firstblock=ib;

#if TRACE_EDSK
	printf("%02X ",ib);
#endif
					memcpy(curwrap->blocks[ib],data+idxdata,1024);
					idxdata+=1024;
					filesize-=1024;
					fb[ib]=0;
					curwrap->entry[ie].blocks[idxb++]=ib;
				}
			}
#if TRACE_EDSK
	printf("\n");
#endif
			memcpy(curwrap->entry[ie].filename,amsdos_name,11);
			curwrap->entry[ie].subcpt=ia;
			curwrap->entry[ie].rc=0x80;
			curwrap->entry[ie].user=0;
			ia++;
			idxb=0;
		} else {
			/* last entry */
#if TRACE_EDSK
	printf("last entry for file (filesize=%d)\nblocklist: ",filesize);
#endif
			if ((ie=EDSK_getdirid(curwrap))==-1)  {
				MakeError(ae,NULL,0,"Error - edsk [%s] DIRECTORY FULL\n",edskfilename);
				MemFree(data);
				return 0;
			}
			if (curwrap->nbentry<=ie) curwrap->nbentry=ie+1;
			/* calcul du nombre de sous blocs de 128 octets */
			curwrap->entry[ie].rc=filesize/128;
			if (filesize%128) {
				curwrap->entry[ie].rc+=1;
			}
			idxb=0;
			for (i=0;i<16 && filesize>0;i++) {
				if ((ib=EDSK_getblockid(fb))==-1) {
					MakeError(ae,NULL,0,"Error - edsk [%s] DISK FULL\n",edskfilename);
					MemFree(data);
					return 0;
				} else {
					if (firstblock==-1) firstblock=ib;
#if TRACE_EDSK
	printf("%02X ",ib);
#endif

					memcpy(curwrap->blocks[ib],&data[idxdata],filesize>1024?1024:filesize);
					idxdata+=1024;
					filesize-=1024;
					fb[ib]=0;
					curwrap->entry[ie].blocks[idxb++]=ib;
				}
			}
#if TRACE_EDSK
	printf("\n");
#endif
			filesize=0;
			memcpy(curwrap->entry[ie].filename,amsdos_name,11);
			curwrap->entry[ie].subcpt=ia;
			curwrap->entry[ie].user=0;
		}
	}

	MemFree(data);
	return 1;
}

void EDSK_build_amsdos_directory(struct s_edsk_wrapper *face)
{
	#undef FUNC
	#define FUNC "EDSK_build_amsdos_directory"
	
	unsigned char amsdosdir[2048]={0};
	int i,idx=0,b;

	if (!face) return;
	
#if TRACE_EDSK	
printf("build amsdos dir with %d entries\n",face->nbentry);	
#endif
	for (i=0;i<face->nbentry;i++) {
		if (face->entry[i].rc && face->entry[i].rc!=0xE5) {
			amsdosdir[idx]=face->entry[i].user;
			memcpy(amsdosdir+idx+1,face->entry[i].filename,11);
			amsdosdir[idx+12]=face->entry[i].subcpt;
			amsdosdir[idx+13]=0;
			amsdosdir[idx+14]=0;
			amsdosdir[idx+15]=face->entry[i].rc;
#if TRACE_EDSK	
printf("%-11.11s [%02X.%02X] blocks:",amsdosdir+idx+1,amsdosdir[idx+12],amsdosdir[idx+15]);
#endif
			for (b=0;b<16;b++) {
				if (face->entry[i].blocks[b]!=0xE5) {
					amsdosdir[idx+16+b]=face->entry[i].blocks[b];
#if TRACE_EDSK	
					printf("%s%02X",b>0?".":"",amsdosdir[idx+16+b]);
#endif
				} else {
					amsdosdir[idx+16+b]=0;
				}
			}
#if TRACE_EDSK	
printf("\n");
#endif
		}
		idx+=32;
	}
#if TRACE_EDSK	
printf("filling amsdos remaining entries (%d) with #E5\n",64-face->nbentry);
#endif
	memset(amsdosdir+idx,0xE5,32*(64-face->nbentry));

	/* AMSDOS directory copy to blocks! */
	memcpy(face->blocks[0],amsdosdir,1024);
	memcpy(face->blocks[1],amsdosdir+1024,1024);
}
void EDSK_write_file(struct s_assenv *ae,struct s_edsk_wrapper *faceA,struct s_edsk_wrapper *faceB)
{
	#undef FUNC
	#define FUNC "EDSK_write_file"

	struct s_edsk_wrapper emptyface={0};
	unsigned char header[256]={0};
	unsigned char trackblock[256]={0};
	unsigned char headertag[25];
	int idblock,blockoffset;
	int i,t;
	
	if (!faceA && !faceB) return;
	
	/* création des deux blocs du directory par face */
	EDSK_build_amsdos_directory(faceA);
	EDSK_build_amsdos_directory(faceB);
	/* écriture header */
	strcpy((char *)header,"EXTENDED CPC DSK File\r\nDisk-Info\r\n");
	sprintf(headertag,"%-9.9s",RASM_SNAP_VERSION);
	strcpy((char *)header+0x22,headertag);
	header[0x30]=40;
	if (!faceA) {
		faceA=&emptyface;
		faceA->edsk_filename=TxtStrDup(faceB->edsk_filename);
	}
#if TRACE_EDSK
	printf("deleting [%s]\n",faceA->edsk_filename);
#endif
	FileRemoveIfExists(faceA->edsk_filename);

	if (faceB!=NULL) header[0x31]=2; else header[0x31]=1;
	for (i=0;i<header[0x30]*header[0x31];i++) header[0x34+i]=19; /* tracksize=(9*512+256)/256 */
#if TRACE_EDSK
	printf("writing EDSK header (256b)\n");
#endif
	FileWriteBinary(faceA->edsk_filename,(char *)header,256);
	
	/* écriture des pistes */
	for (t=0;t<40;t++) {
		strcpy((char *)trackblock,"Track-Info\r\n");
		trackblock[0x10]=t;
		trackblock[0x11]=0;
		trackblock[0x14]=2;
		trackblock[0x15]=9;
		trackblock[0x16]=0x4E;
		trackblock[0x17]=0xE5;
		i=0;
		while (1) {
			trackblock[0x18+i*8+0]=trackblock[0x10];
			trackblock[0x18+i*8+1]=trackblock[0x11];
			trackblock[0x18+i*8+2]=(i>>1)+0xC1;
#if TRACE_EDSK
	if (t<3) printf("%02X ",trackblock[0x18+i*8+2]);
#endif
			trackblock[0x18+i*8+3]=2;
			trackblock[0x18+i*8+4]=0;
			trackblock[0x18+i*8+5]=0;
			trackblock[0x18+i*8+6]=0;
			trackblock[0x18+i*8+7]=2;
			i++;
			if (i==9) break;
			/* interleave */
			trackblock[0x18+i*8+0]=trackblock[0x10];
			trackblock[0x18+i*8+1]=trackblock[0x11];
			trackblock[0x18+i*8+2]=(i>>1)+0xC6; /* start at C6 */
#if TRACE_EDSK
	if (t<3) printf("%02X ",trackblock[0x18+i*8+2]);
#endif
			trackblock[0x18+i*8+3]=2;
			trackblock[0x18+i*8+4]=0;
			trackblock[0x18+i*8+5]=0;
			trackblock[0x18+i*8+6]=0;
			trackblock[0x18+i*8+7]=2;
			i++;
		}
#if TRACE_EDSK
	if (t<3) printf("\n"); else if (t==3) printf("...\n");
#endif
		/* écriture du track info */
		FileWriteBinary(faceA->edsk_filename,(char *)trackblock,256);


		/* il faut convertir les blocs logiques en secteurs physiques ET entrelacés */
		idblock=t*9/2;
		blockoffset=((t*9)%2)*512;

		/* le premier secteur de la piste est à cheval sur le bloc logique une fois sur deux */
		FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock][0]+blockoffset,512); /* C1 */
		if (!blockoffset) {
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+2][0]+512,512); /* C6 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+0][0]+512,512); /* C2 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+3][0]+0,512);   /* C7 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+1][0]+0,512);   /* C3 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+3][0]+512,512); /* C8 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+1][0]+512,512); /* C4 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+4][0]+0,512);   /* C9 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+2][0]+0,512);   /* C5 */
		} else {
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+3][0]+0,512);   /* C6 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+1][0]+0,512);   /* C2 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+3][0]+512,512); /* C7 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+1][0]+512,512); /* C3 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+4][0]+0,512);   /* C8 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+2][0]+0,512);   /* C4 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+4][0]+512,512); /* C9 */
			FileWriteBinary(faceA->edsk_filename,(char *)&faceA->blocks[idblock+2][0]+512,512); /* C5 */
		}

		/* @@TODO ça semble un peu foireux comme procédé */	
		if (faceB) {
#if TRACE_EDSK
	printf("writing EDSK face B /!\\  probably NOT WORKING !!!\n");
#endif
			trackblock[0x11]=1;
			for (i=0;i<9;i++) {
				trackblock[0x18+i*8+0]=trackblock[0x10];
				trackblock[0x18+i*8+1]=trackblock[0x11];
			}
			/* écriture du track info */
			FileWriteBinary(faceB->edsk_filename,(char *)trackblock,256);
			/* écriture des secteurs */
			idblock=t*9/2;
			blockoffset=((t*9)%2)*512;
			FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock][0]+blockoffset,512);
			if (!blockoffset) {
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+2][0]+512,512); /* C6 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+0][0]+512,512); /* C2 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+3][0]+0,512);   /* C7 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+1][0]+0,512);   /* C3 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+3][0]+512,512); /* C8 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+1][0]+512,512); /* C4 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+4][0]+0,512);   /* C9 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+2][0]+0,512);   /* C5 */
			} else {
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+3][0]+0,512);   /* C6 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+1][0]+0,512);   /* C2 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+3][0]+512,512); /* C7 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+1][0]+512,512); /* C3 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+4][0]+0,512);   /* C8 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+2][0]+0,512);   /* C4 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+4][0]+512,512); /* C9 */
				FileWriteBinary(faceB->edsk_filename,(char *)&faceB->blocks[idblock+2][0]+512,512); /* C5 */
			}
		}
	}
	FileWriteBinaryClose(faceA->edsk_filename);
	rasm_printf(ae,KIO"Write edsk file %s\n",faceA->edsk_filename);
}
void EDSK_write(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "EDSK_write"

	struct s_edsk_wrapper *faceA,*faceB;
	int i,j;

	
	/* on passe en revue toutes les structs */
	for (i=0;i<ae->nbedskwrapper;i++) {
		/* already done */
		if (ae->edsk_wrapper[i].face==-1) continue;
		
		switch (ae->edsk_wrapper[i].face) {
			default:
			case 0:faceA=&ae->edsk_wrapper[i];faceB=NULL;break;
			case 1:faceA=NULL;faceB=&ae->edsk_wrapper[i];break;
		}
		/* doit-on fusionner avec une autre face? */
		for (j=i+1;j<ae->nbedskwrapper;j++) {
			if (!strcmp(ae->edsk_wrapper[i].edsk_filename,ae->edsk_wrapper[j].edsk_filename)) {
				/* found another face for the floppy */
				switch (ae->edsk_wrapper[j].face) {
					default:
					case 0:faceA=&ae->edsk_wrapper[j];break;
					case 1:faceB=&ae->edsk_wrapper[j];break;
				}
			}
		}
		EDSK_write_file(ae,faceA,faceB);
	}
}

/* CDT output code / courtesy of CNG */
void update11(unsigned char *head,int n,int is1st,int islast,int l, int fileload)
{
        head[0x10]=n;
        head[0x11]=islast?-1:0;
        head[0x13]=l;
        head[0x14]=l>>8;
        head[0x15]=fileload;
        head[0x16]=fileload>>8;
        head[0x17]=is1st?-1:0;
}
#define fputcc(x,y) { fputc((x),y); fputc((x)>>8,y); }
#define fputccc(x,y) { fputc((x),y); fputc((x)>>8,y); fputc((x)>>16,y); }
void record11(char *filename,unsigned char *t,int first,int l,int p, int flag_bb, int flag_b)
{
	FILE *fo;
	#ifdef OS_WIN
	fo=FileOpen(filename,"w");
	#else
	fo=FileOpen(filename,"a+");
	#endif

	/* almost legacy */
        fputc(0x11,fo);
        fputcc(flag_bb,fo);
        fputcc(flag_b,fo);
        fputcc(flag_b,fo);
        fputcc(flag_b,fo);
        fputcc(flag_bb,fo);
        //fputcc(flag_o,fo);
        fputcc(4096,fo); // 4K block
        fputc(8,fo);
        fputcc(p,fo);
        p=1+(((l+255)/256)*258)+4; //flag_z;
        fputccc(p,fo);
        fputc(first,fo);
        p=0;
        while (l>0)
        {
		int crc16=0xFFFF;
                fwrite(t+p,1,256,fo);
                first=256;
		while (first--) {
			// early CRC-16-CCITT as used by Amstrad
                        int xor8=(t[p++]<<8)+1;
                        while (xor8&0xFF)
                        {
                                if ((xor8^crc16)&0x8000)
                                        crc16=((crc16^0x0810)<<1)+1;
                                else
                                        crc16<<=1;
                                xor8<<=1;
                        }
                }
                crc16=~crc16;
                fputc(crc16>>8,fo); // HI FIRST,
                fputc(crc16,fo); // AND LO NEXT!
                l-=256;
        }
        l=4; //flag_z;
        while (l--)
                fputc(255,fo);
}

void __output_CDT(struct s_assenv *ae, char *tapefilename,char *filename,char *mydata,int size, int offset, int run)
{
	unsigned char *AmsdosHeader;
	unsigned char head[256];
	char TZX_header[14];
	int wrksize,fileload,nbblock=0;
	unsigned char body[65536+128];
	int flag_h=2560, flag_p=10240, flag_bb, flag_b=1000,i,j,k;

	FileRemoveIfExists(tapefilename); // pas de append pour le moment

	memcpy(TZX_header,"ZXTape!\032\001\000\040\000\012",13);
	FileWriteBinary(tapefilename,(char *)TZX_header,13);

	AmsdosHeader=MakeAMSDOSHeader(run,offset,offset+size,MakeAMSDOS_name(ae,filename));
	memcpy(body,AmsdosHeader,128);
	wrksize=size;

	memset(head,0,16);
	strcpy(head,MakeAMSDOS_name(ae,filename));
	head[0x12]=body[0x12];
	head[0x18]=body[0x40];
	head[0x19]=body[0x41];
	head[0x1A]=body[0x1A];
	head[0x1B]=body[0x1B];
	fileload=body[0x15]+body[0x16]*256;
	flag_b=(3500000/3+flag_b/2)/flag_b;
	flag_bb=flag_b*2;
	memcpy(body,mydata,size);

	if (wrksize>0x800) {
		update11(head,j=1,1,0,0x800,fileload); // FIRST BLOCK
		record11(tapefilename,head,44,28,16,flag_bb,flag_b);
		record11(tapefilename,body,22,0x800,flag_h,flag_bb,flag_b);
		k=wrksize-0x800;
		i=0x800;
		nbblock=1;
		while (k>0x800) {
			fileload+=0x800;
			update11(head,++j,0,0,0x800,fileload); // MID BLOCK
			record11(tapefilename,head,44,28,16,flag_bb,flag_b);
			record11(tapefilename,body+i,22,0x800,flag_h,flag_bb,flag_b);
			k-=0x800;
			i+=0x800;
			nbblock++;
		}
		nbblock++;
		fileload+=0x800;
		update11(head,++j,0,1,k,fileload); // LAST BLOCK
		record11(tapefilename,head,44,28,16,flag_bb,flag_b);
		record11(tapefilename,body+i,22,k,flag_p,flag_bb,flag_b);
	} else {
		update11(head,1,1,1,wrksize,fileload); // SINGLE BLOCK
		record11(tapefilename,head,44,28,16,flag_bb,flag_b);
		record11(tapefilename,body,22,wrksize,flag_p,flag_bb,flag_b);
		nbblock=1;
	}
	FileWriteBinaryClose(tapefilename);
	rasm_printf(ae,KIO"Write tape file %s (%d block%s) run=#%04X\n",tapefilename,nbblock,nbblock>1?"s":"",run);
}



void PopAllSave(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "PopAllSave"
	
	unsigned char *AmsdosHeader;
	char *dskfilename;
	char *filename;
	int offset,size,run;
	int i,is,erreur=0,touched;
	
	for (is=0;is<ae->nbsave;is++) {
		/* avoid quotes */
		filename=ae->wl[ae->save[is].iw].w;
		filename[strlen(filename)-1]=0;
		filename=TxtStrDup(filename+1);
		/* translate tags! */
		filename=TranslateTag(ae,filename,&touched,1,E_TAGOPTION_REMOVESPACE);

#if TRACE_EDSK
	printf("woff=[%s](%d) wsize=[%s](%d)\n",ae->wl[ae->save[is].ioffset].w,ae->save[is].ioffset,ae->wl[ae->save[is].isize].w,ae->save[is].isize);
#endif

		ae->idx=ae->save[is].ioffset; /* exp hack */
		ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
		offset=RoundComputeExpression(ae,ae->wl[ae->idx].w,0,0,0);

		ae->idx=ae->save[is].isize; /* exp hack */
		ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
		size=RoundComputeExpression(ae,ae->wl[ae->idx].w,0,0,0);

		ae->idx=ae->save[is].irun; /* exp hack */
		if (ae->idx) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
			run=RoundComputeExpression(ae,ae->wl[ae->idx].w,0,0,0);
		} else {
			run=offset;
		}

		if (size<1 || size>65536) {
			MakeError(ae,NULL,0,"cannot save [%s] as the size is invalid!\n",filename);
			MemFree(filename);
			continue;
		}
		if (offset<0 || offset>65535) {
			MakeError(ae,NULL,0,"cannot save [%s] as the offset is invalid!\n",filename);
			MemFree(filename);
			continue;
		}
		if (offset+size>65536) {
			MakeError(ae,NULL,0,"cannot save [%s] as the offset+size will be out of bounds!\n",filename);
			MemFree(filename);
			continue;
		}
		/* DSK management */
		if (ae->save[is].dsk) {
			if (ae->save[is].iwdskname!=-1) {
				/* obligé de dupliquer à cause du reuse */
				dskfilename=TxtStrDup(ae->wl[ae->save[is].iwdskname].w);
				dskfilename[strlen(dskfilename)-1]=0;
				if (!EDSK_addfile(ae,dskfilename+1,ae->save[is].face,filename,ae->mem[ae->save[is].ibank]+offset,size,offset,run)) {
					erreur++;
					//break;
				}
				MemFree(dskfilename);
			}
		} else if (ae->save[is].tape) {
			char *tapefilename;

			if (ae->save[is].iwdskname>0) {
				tapefilename=ae->wl[ae->save[is].iwdskname].w;
				tapefilename[strlen(tapefilename)-1]=0;
				tapefilename=TxtStrDup(tapefilename+1);
			} else {
				tapefilename=TxtStrDup("rasmoutput.cdt");
			}

			__output_CDT(ae,tapefilename,filename,(char*)ae->mem[ae->save[is].ibank]+offset,size,offset,run);
		} else {
			/* output file on filesystem */
			rasm_printf(ae,KIO"Write binary file %s (%d byte%s)\n",filename,size,size>1?"s":"");
			FileRemoveIfExists(filename);
			if (ae->save[is].amsdos) {
				AmsdosHeader=MakeAMSDOSHeader(run,offset,offset+size,MakeAMSDOS_name(ae,filename));
				FileWriteBinary(filename,(char *)AmsdosHeader,128);
			}		
			FileWriteBinary(filename,(char*)ae->mem[ae->save[is].ibank]+offset,size);
			FileWriteBinaryClose(filename);
		}
		MemFree(filename);
	}
	if (!erreur) EDSK_write(ae);
	
	for (i=0;i<ae->nbedskwrapper;i++) {
		MemFree(ae->edsk_wrapper[i].edsk_filename);
	}
	if (ae->maxedskwrapper) MemFree(ae->edsk_wrapper);

	if (ae->nbsave) {
		MemFree(ae->save);
	}
}

void PopAllExpression(struct s_assenv *ae, int crunched_zone)
{
	#undef FUNC
	#define FUNC "PopAllExpression"
	
	static int first=1;
	double v;
	long r;
	int i;
	unsigned char *mem;
	char *expr;
	
	/* pop all expressions BUT thoses who where already computed (in crunched blocks) */

	/* calcul des labels et expressions en zone crunch (et locale?)
	   les labels doivent pointer:
	   - une valeur absolue (numerique ou variable calculee) -> completement transparent
	   - un label dans la meme zone de crunch -> label->lz=1 && verif de la zone crunch
	   - un label hors zone crunch MAIS avant toute zone de crunch de la bank destination (!label->lz)

	   idealement on doit tolerer les adresses situees apres le crunch dans une autre ORG zone!

	   on utilise ae->stage pour créer un état intermédiaire dans le ComputeExpressionCore
	*/
	if (crunched_zone>=0) {
		ae->stage=1;
	} else {
		/* on rescanne tout pour combler les trous */
		ae->stage=2;
		first=1;
	}
	
	for (i=first;i<ae->ie;i++) {
		/* first compute only crunched expression (0,1,2,3,...) then intermediates and (-1) at the end */
		if (crunched_zone>=0) {
			/* jump over previous crunched or non-crunched zones */
			if (ae->expression[i].lz<crunched_zone) continue;
			/* OPTIM: keep index and stop when we are after the current crunched zone */
			if (ae->expression[i].lz>crunched_zone) {
				first=i;
				break;
			}
		} else {
			if (ae->expression[i].lz>=0) continue;
		}

		mem=ae->mem[ae->expression[i].ibank];
		
		if (ae->expression[i].reference) {
			expr=ae->expression[i].reference;
		} else {
			expr=ae->wl[ae->expression[i].iw].w;
		}
		v=ComputeExpressionCore(ae,expr,ae->expression[i].ptr,i);
		r=(long)floor(v+ae->rough);
		switch (ae->expression[i].zetype) {
			case E_EXPRESSION_J8:
				r=r-ae->expression[i].ptr-2;
				if (r<-128 || r>127) {
					MakeError(ae,GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l,"relative offset %d too far [%s]\n",r,ae->wl[ae->expression[i].iw].w);
				}
				mem[ae->expression[i].wptr]=(unsigned char)r;
				break;
			case E_EXPRESSION_IV81:
				/* for enhanced 16bits instructions */
				r++;
			case E_EXPRESSION_0V8:
			case E_EXPRESSION_IV8:
			case E_EXPRESSION_3V8:
			case E_EXPRESSION_V8:
				if (r>255 || r<-128) {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: truncating value #%X to #%X\n",GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l,r,r&0xFF);
						if (ae->erronwarn) MaxError(ae);
					}
				}
				mem[ae->expression[i].wptr]=(unsigned char)r;
				break;
			case E_EXPRESSION_IV16:
			case E_EXPRESSION_V16:
			case E_EXPRESSION_V16C:
			case E_EXPRESSION_0V16:
				if (r>65535 || r<-32768) {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: truncating value #%X to #%X\n",GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l,r,r&0xFFFF);
						if (ae->erronwarn) MaxError(ae);
					}
				}
				mem[ae->expression[i].wptr]=(unsigned char)r&0xFF;
				mem[ae->expression[i].wptr+1]=(unsigned char)((r&0xFF00)>>8);
				break;
			case E_EXPRESSION_0V32:
				/* meaningless in 32 bits architecture... */
				if (v>4294967295 || v<-2147483648) {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: truncating value\n",GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l);
						if (ae->erronwarn) MaxError(ae);
					}
				}
				mem[ae->expression[i].wptr]=(unsigned char)r&0xFF;
				mem[ae->expression[i].wptr+1]=(unsigned char)((r>>8)&0xFF);
				mem[ae->expression[i].wptr+2]=(unsigned char)((r>>16)&0xFF);
				mem[ae->expression[i].wptr+3]=(unsigned char)((r>>24)&0xFF);
				break;
			case E_EXPRESSION_0VR:
				/* convert v double value to Amstrad REAL */
				memcpy(&mem[ae->expression[i].wptr],__internal_MakeAmsdosREAL(ae,v,i),5);
				break;
			case E_EXPRESSION_0VRMike:
				/* convert v double value to Microsoft 40bits REAL */
				memcpy(&mem[ae->expression[i].wptr],__internal_MakeRosoftREAL(ae,v,i),5);
				break;
			case E_EXPRESSION_IM:
				switch (r) {
					case 0x00:mem[ae->expression[i].wptr]=0x46;break;
					case 0x01:mem[ae->expression[i].wptr]=0x56;break;
					case 0x02:mem[ae->expression[i].wptr]=0x5E;break;
					default:
						MakeError(ae,GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l,"IM 0,1 or 2 only\n");
						mem[ae->expression[i].wptr]=0;
				}
				break;
			case E_EXPRESSION_RST:
				switch (r) {
					case 0x00:mem[ae->expression[i].wptr]=0xC7;break;
					case 0x08:mem[ae->expression[i].wptr]=0xCF;break;
					case 0x10:mem[ae->expression[i].wptr]=0xD7;break;
					case 0x18:mem[ae->expression[i].wptr]=0xDF;break;
					case 0x20:mem[ae->expression[i].wptr]=0xE7;break;
					case 0x28:mem[ae->expression[i].wptr]=0xEF;break;
					case 0x30:mem[ae->expression[i].wptr]=0xF7;break;
					case 0x38:mem[ae->expression[i].wptr]=0xFF;break;
					default:
						MakeError(ae,GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l,"RST #0,#8,#10,#18,#20,#28,#30,#38 only\n");
						mem[ae->expression[i].wptr]=0;
				}
				break;
			case E_EXPRESSION_RUN:
				if (r<0 || r>65535) {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: run adress truncated from %X to %X\n",GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l,r,r&0xFFFF);
						if (ae->erronwarn) MaxError(ae);
					}
				}
				ae->snapshot.registers.LPC=r&0xFF;
				ae->snapshot.registers.HPC=(r>>8)&0xFF;
				break;			
			case E_EXPRESSION_ZXRUN:
				if (r<0 || r>65535) {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: run adress truncated from %X to %X\n",GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l,r,r&0xFFFF);
						if (ae->erronwarn) MaxError(ae);
					}
				}
				ae->zxsnapshot.run=r&0xFFFF;
				break;			
			case E_EXPRESSION_ZXSTACK:
				if (r<0 || r>65535) {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: stack adress truncated from %X to %X\n",GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l,r,r&0xFFFF);
						if (ae->erronwarn) MaxError(ae);
					}
				}
				ae->zxsnapshot.stack=r&0xFFFF;
				break;
			case E_EXPRESSION_BRS:
				if (r>=0 && r<8) {
					mem[ae->expression[i].wptr]+=r*8;
				} else {
					MakeError(ae,GetExpFile(ae,i),ae->wl[ae->expression[i].iw].l,"SET,RES,BIT shift value from 0 to 7 only\n");
				}
				break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"FATAL - unknown expression type\n");
				FreeAssenv(ae);exit(-8);
		}	
	}
}

void InsertLabelToTree(struct s_assenv *ae, struct s_label *label)
{
	#undef FUNC
	#define FUNC "InsertLabelToTree"

	struct s_crclabel_tree *curlabeltree;
	int radix,dek=32;

	curlabeltree=&ae->labeltree;
	while (dek) {
		dek=dek-8;
		radix=(label->crc>>dek)&0xFF;
		if (curlabeltree->radix[radix]) {
			curlabeltree=curlabeltree->radix[radix];
		} else {
			curlabeltree->radix[radix]=MemMalloc(sizeof(struct s_crclabel_tree));
			curlabeltree=curlabeltree->radix[radix];
			memset(curlabeltree,0,sizeof(struct s_crclabel_tree));
		}
	}
	ObjectArrayAddDynamicValueConcat((void**)&curlabeltree->label,&curlabeltree->nlabel,&curlabeltree->mlabel,&label[0],sizeof(struct s_label));
}

/* use by structure mechanism and label import to add fake labels */
void PushLabelLight(struct s_assenv *ae, struct s_label *curlabel) {
	#undef FUNC
	#define FUNC "PushLabelLight"
	
	struct s_label *searched_label;
	
	/* PushLabel light */
	if ((searched_label=SearchLabel(ae,curlabel->name,curlabel->crc))!=NULL) {
		MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"%s caused duplicate label [%s]\n",ae->idx?"Structure insertion":"Label import",curlabel->name);
		MemFree(curlabel->name);
	} else {
		curlabel->backidx=ae->il;
		curlabel->autorise_export=ae->autorise_export&(!ae->getstruct);
		ObjectArrayAddDynamicValueConcat((void **)&ae->label,&ae->il,&ae->ml,curlabel,sizeof(struct s_label));
		InsertLabelToTree(ae,curlabel);
	}				
}
void PushLabel(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "PushLabel"
	
	struct s_label curlabel={0},*searched_label;
	int i;
	/* label with counters */
	char *varbuffer;
	int tagcount=0;
	int touched;

#if TRACE_LABEL
	printf("check label [%s]\n",ae->wl[ae->idx].w);
#endif
	if (ae->AutomateValidLabelFirst[(int)ae->wl[ae->idx].w[0]&0xFF]) {
		for (i=1;ae->wl[ae->idx].w[i];i++) {
			if (ae->wl[ae->idx].w[i]=='{') tagcount++; else if (ae->wl[ae->idx].w[i]=='}') tagcount--;
			if (!tagcount) {
				if (!ae->AutomateValidLabel[(int)ae->wl[ae->idx].w[i]&0xFF]) {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid char in label declaration (%c)\n",ae->wl[ae->idx].w[i]);
					return;
				}
			}
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid first char in label declaration (%c)\n",ae->wl[ae->idx].w[0]);
		return;
	}
	
	switch (i) {
		case 1:
			switch (ae->wl[ae->idx].w[0]) {
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
				case 'H':
				case 'L':
				case 'I':
				case 'R':
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use reserved word [%s] for label\n",ae->wl[ae->idx].w);
					return;
				default:break;
			}
			break;
		case 2:
			if (strcmp(ae->wl[ae->idx].w,"AF")==0 || strcmp(ae->wl[ae->idx].w,"BC")==0 || strcmp(ae->wl[ae->idx].w,"DE")==0 || strcmp(ae->wl[ae->idx].w,"HL")==0 || 
				strcmp(ae->wl[ae->idx].w,"IX")==0 || strcmp(ae->wl[ae->idx].w,"IY")==0 || strcmp(ae->wl[ae->idx].w,"SP")==0 ||
				strcmp(ae->wl[ae->idx].w,"LX")==0 || strcmp(ae->wl[ae->idx].w,"HX")==0 || strcmp(ae->wl[ae->idx].w,"XL")==0 || strcmp(ae->wl[ae->idx].w,"XH")==0 ||
				strcmp(ae->wl[ae->idx].w,"LY")==0 || strcmp(ae->wl[ae->idx].w,"HY")==0 || strcmp(ae->wl[ae->idx].w,"YL")==0 || strcmp(ae->wl[ae->idx].w,"YH")==0) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use reserved word [%s] for label\n",ae->wl[ae->idx].w);
				return;
			}
			break;
		case 3:
			if (strcmp(ae->wl[ae->idx].w,"IXL")==0 || strcmp(ae->wl[ae->idx].w,"IYL")==0 || strcmp(ae->wl[ae->idx].w,"IXH")==0 || strcmp(ae->wl[ae->idx].w,"IYH")==0) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use reserved word [%s] for label\n",ae->wl[ae->idx].w);
				return;
			}			
			break;
		case 4:
			if (strcmp(ae->wl[ae->idx].w,"VOID")==0) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use reserved word [%s] for label\n",ae->wl[ae->idx].w);
				return;
			}
		default:break;
	}

	/*******************************************************
	   v a r i a b l e s     i n    l a b e l    n a m e

	           -- varbuffer is always allocated --
	*******************************************************/
	varbuffer=TranslateTag(ae,TxtStrDup(ae->wl[ae->idx].w),&touched,1,E_TAGOPTION_NONE); // on se moque du touched ici => varbuffer toujours "new"
#if TRACE_LABEL
	printf("label after translation [%s]\n",varbuffer);
#endif
	/**************************************************
	   s t r u c t u r e     d e c l a r a t i o n
	**************************************************/
	if (ae->getstruct) {
		struct s_rasmstructfield rasmstructfield={0};
#if TRACE_LABEL
	printf("label used for structs! [%s]\n",varbuffer);
#endif
		if (varbuffer[0]=='@') {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Please no local label in a struct [%s]\n",ae->wl[ae->idx].w);
			MemFree(varbuffer);
			return;
		}
		/* copy label+offset in the structure */
		rasmstructfield.name=varbuffer;
		rasmstructfield.offset=ae->codeadr;
		ObjectArrayAddDynamicValueConcat((void **)&ae->rasmstruct[ae->irasmstruct-1].rasmstructfield,
				&ae->rasmstruct[ae->irasmstruct-1].irasmstructfield,&ae->rasmstruct[ae->irasmstruct-1].mrasmstructfield,
				&rasmstructfield,sizeof(rasmstructfield));
		/* label is structname+field */
		curlabel.name=MemMalloc(strlen(ae->rasmstruct[ae->irasmstruct-1].name)+strlen(varbuffer)+2);
		sprintf(curlabel.name,"%s.%s",ae->rasmstruct[ae->irasmstruct-1].name,varbuffer);
		curlabel.iw=-1;
		/* legacy */
		curlabel.crc=GetCRC(curlabel.name);
		curlabel.ptr=ae->codeadr;
#if TRACE_STRUCT
	printf("pushLabel (struct) [%X] [%s]\n",curlabel.ptr,curlabel.name);
#endif
	} else {
		/**************************************************
		   l a b e l s
		**************************************************/
		/* labels locaux */
		if (varbuffer[0]=='@' && (ae->ir || ae->iw || ae->imacro)) {
#if TRACE_LABEL
	printf("PUSH LOCAL\n");
#endif
			curlabel.iw=-1;
			curlabel.local=1;
			curlabel.name=MakeLocalLabel(ae,varbuffer,NULL);  MemFree(varbuffer);
			curlabel.crc=GetCRC(curlabel.name);

			/* local labels ALSO set new reference */
			if (ae->lastglobalalloc) {
//printf("push LOCAL is freeing lastgloballabel\n");
				MemFree(ae->lastgloballabel);
			}
			ae->lastgloballabel=TxtStrDup(curlabel.name);
			ae->lastgloballabellen=strlen(ae->lastgloballabel);
			ae->lastglobalalloc=1;
//printf("push LOCAL as reference [%d] for proximity label -> [%s]\n",im, ae->lastgloballabel);

		} else {
#if TRACE_LABEL
	printf("PUSH GLOBAL or PROXIMITY\n");
#endif
			switch (varbuffer[0]) {
				case '.':
					if (ae->dams) {
						/* old Dams style declaration (remove the dot) */
						i=0;
						do {
							varbuffer[i]=varbuffer[i+1];
							i++;
						} while (varbuffer[i]!=0);

						curlabel.iw=-1;
						curlabel.name=varbuffer;
						curlabel.crc=GetCRC(curlabel.name);
					} else {
						/* proximity labels */
						if (ae->lastgloballabel) {
							curlabel.name=MemMalloc(strlen(varbuffer)+1+ae->lastgloballabellen);
							sprintf(curlabel.name,"%s%s",ae->lastgloballabel,varbuffer);
							MemFree(varbuffer);
							curlabel.iw=-1;
							curlabel.crc=GetCRC(curlabel.name);
#if TRACE_LABEL
printf("PUSH PROXIMITY label that may be exported [%s]->[%s]\n",ae->wl[ae->idx].w,curlabel.name);
#endif
						} else {
#if TRACE_LABEL
printf("PUSH Orphan PROXIMITY label that cannot be exported [%s]->[%s]\n",ae->wl[ae->idx].w,curlabel.name);
#endif

							curlabel.iw=-1;
							curlabel.name=varbuffer;
							curlabel.crc=GetCRC(varbuffer);
						}
					}
					break;
				default:
#if TRACE_LABEL
	printf("PUSH => GLOBAL [%s]\n",varbuffer);
#endif
					curlabel.iw=-1;
					curlabel.name=varbuffer; 
					curlabel.crc=GetCRC(varbuffer);

					/* global labels set new reference */
					if (ae->lastglobalalloc) MemFree(ae->lastgloballabel);
					ae->lastgloballabel=TxtStrDup(curlabel.name);
					ae->lastgloballabellen=strlen(curlabel.name);
					ae->lastglobalalloc=1;
					break;
			}


			/* this stage varbuffer maybe already freed or used */
			if (curlabel.name[0]!='@' && ae->module && ae->modulen) {
				char *newlabelname;

				newlabelname=MemMalloc(strlen(curlabel.name)+ae->modulen+2);
				strcpy(newlabelname,ae->module);
				strcat(newlabelname,ae->module_separator);
				strcat(newlabelname,curlabel.name);
				MemFree(curlabel.name);
				curlabel.name=newlabelname;
				curlabel.crc=GetCRC(curlabel.name);
				//curlabel.iw=-1; => deja mis depuis longtemps
			}
#if TRACE_LABEL
	if (curlabel.name[0]!='@') printf("PUSH => ADD MODULE [%s] => [%s]\n",ae->module?ae->module:"(null)",curlabel.name);
	else printf("PUSH => NO MODULE for local label\n");
#endif

			/* contrôle dico uniquement avec des labels non locaux */
			if (SearchDico(ae,curlabel.name,curlabel.crc)) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"cannot create label [%s] as there is already a variable with the same name\n",curlabel.name);
				return;
			}
			if(SearchAlias(ae,curlabel.crc,curlabel.name)!=-1) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"cannot create label [%s] as there is already an alias with the same name\n",curlabel.name);
				return;
			}
		}
		curlabel.ptr=ae->codeadr;
		curlabel.ibank=ae->activebank;
		curlabel.iorgzone=ae->io-1;
		curlabel.lz=ae->lz;
	}

	if ((searched_label=SearchLabel(ae,curlabel.name,curlabel.crc))!=NULL) {
		MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"Duplicate label [%s] - previously defined in [%s:%d]\n",curlabel.name,ae->filename[searched_label->fileidx],searched_label->fileline);
		MemFree(curlabel.name);
	} else {
//printf("PushLabel(%s) name=%s crc=%X lz=%d\n",curlabel.name,curlabel.name?curlabel.name:"null",curlabel.crc,curlabel.lz);
		curlabel.fileidx=ae->wl[ae->idx].ifile;
		curlabel.fileline=ae->wl[ae->idx].l;
		curlabel.autorise_export=ae->autorise_export&(!ae->getstruct);
		curlabel.backidx=ae->il;
		ObjectArrayAddDynamicValueConcat((void **)&ae->label,&ae->il,&ae->ml,&curlabel,sizeof(curlabel));
		InsertLabelToTree(ae,&curlabel);
	}

}


unsigned char *EncodeSnapshotRLE(unsigned char *memin, int *lenout) {
	#undef FUNC
	#define FUNC "EncodeSnapshotRLE"
	
	int i,cpt,idx=0;
	unsigned char *memout;
	
	memout=MemMalloc(65536*2);
	
	for (i=0;i<65536;) {

		for (cpt=1;cpt<255 && i+cpt<65536;cpt++) if (memin[i]!=memin[i+cpt]) break;

		if (cpt>=3 || memin[i]==0xE5) {
			memout[idx++]=0xE5;
			memout[idx++]=cpt;
			memout[idx++]=memin[i];
			i+=cpt;
		} else {
			memout[idx++]=memin[i++];
		}
	}
	if (lenout) *lenout=idx;
	if (idx<65536) return memout;
	
	MemFree(memout);
	return NULL;
}



#undef FUNC
#define FUNC "Instruction CORE"

void _IN(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		if (strcmp(ae->wl[ae->idx+2].w,"(C)")==0) {
			switch (GetCRC(ae->wl[ae->idx+1].w)) {
				case CRC_0:
				case CRC_F:___output(ae,0xED);___output(ae,0x70);ae->nop+=4;ae->tick+=12;break;
				case CRC_A:___output(ae,0xED);___output(ae,0x78);ae->nop+=4;ae->tick+=12;break;
				case CRC_B:___output(ae,0xED);___output(ae,0x40);ae->nop+=4;ae->tick+=12;break;
				case CRC_C:___output(ae,0xED);___output(ae,0x48);ae->nop+=4;ae->tick+=12;break;
				case CRC_D:___output(ae,0xED);___output(ae,0x50);ae->nop+=4;ae->tick+=12;break;
				case CRC_E:___output(ae,0xED);___output(ae,0x58);ae->nop+=4;ae->tick+=12;break;
				case CRC_H:___output(ae,0xED);___output(ae,0x60);ae->nop+=4;ae->tick+=12;break;
				case CRC_L:___output(ae,0xED);___output(ae,0x68);ae->nop+=4;ae->tick+=12;break;
				default:
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is IN [0,F,A,B,C,D,E,H,L],(C)\n");
			}
		} else if (strcmp(ae->wl[ae->idx+1].w,"A")==0 && StringIsMem(ae->wl[ae->idx+2].w)) {
			___output(ae,0xDB);
			PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
			ae->nop+=3;
			ae->tick+=11;
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IN [0,F,A,B,C,D,E,H,L],(C) or IN A,(n) only\n");
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IN [0,F,A,B,C,D,E,H,L],(C) or IN A,(n) only\n");
	}
}

void _OUT(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		if (strcmp(ae->wl[ae->idx+1].w,"(C)")==0) {
			switch (GetCRC(ae->wl[ae->idx+2].w)) {
				case CRC_0:___output(ae,0xED);___output(ae,0x71);ae->nop+=4;ae->tick+=12;break;
				case CRC_A:___output(ae,0xED);___output(ae,0x79);ae->nop+=4;ae->tick+=12;break;
				case CRC_B:___output(ae,0xED);___output(ae,0x41);ae->nop+=4;ae->tick+=12;break;
				case CRC_C:___output(ae,0xED);___output(ae,0x49);ae->nop+=4;ae->tick+=12;break;
				case CRC_D:___output(ae,0xED);___output(ae,0x51);ae->nop+=4;ae->tick+=12;break;
				case CRC_E:___output(ae,0xED);___output(ae,0x59);ae->nop+=4;ae->tick+=12;break;
				case CRC_H:___output(ae,0xED);___output(ae,0x61);ae->nop+=4;ae->tick+=12;break;
				case CRC_L:___output(ae,0xED);___output(ae,0x69);ae->nop+=4;ae->tick+=12;break;
				default:
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is OUT (C),[0,A,B,C,D,E,H,L]\n");
			}
		} else if (strcmp(ae->wl[ae->idx+2].w,"A")==0 && StringIsMem(ae->wl[ae->idx+1].w)) {
			___output(ae,0xD3);
			PushExpression(ae,ae->idx+1,E_EXPRESSION_V8);
			ae->nop+=3;
			ae->tick+=11;
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"OUT (C),[0,A,B,C,D,E,H,L] or OUT (n),A only\n");
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"OUT (C),[0,A,B,C,D,E,H,L] or OUT (n),A only\n");
	}
}

void _EX(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_HL:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_DE:___output(ae,0xEB);ae->nop+=1;ae->tick+=4;break;
					case CRC_MSP:___output(ae,0xE3);ae->nop+=6;ae->tick+=19;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is EX HL,[(SP),DE]\n");
				}
				break;
			case CRC_AF:
				if (strcmp(ae->wl[ae->idx+2].w,"AF'")==0) {
					___output(ae,0x08);ae->nop+=1;ae->tick+=4;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is EX AF,AF'\n");
				}
				break;
			case CRC_MSP:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_HL:___output(ae,0xE3);ae->nop+=6;ae->tick+=19;break;
					case CRC_IX:___output(ae,0xDD);___output(ae,0xE3);ae->nop+=7;ae->tick+=23;break;
					case CRC_IY:___output(ae,0xFD);___output(ae,0xE3);ae->nop+=7;ae->tick+=23;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is EX (SP),[HL,IX,IY]\n");
				}
				break;
			case CRC_DE:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_HL:___output(ae,0xEB);ae->nop+=1;ae->tick+=4;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is EX DE,HL\n");
				}
				break;
			case CRC_IX:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_MSP:___output(ae,0xDD);___output(ae,0xE3);ae->nop+=7;ae->tick+=23;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is EX IX,(SP)\n");
				}
				break;
			case CRC_IY:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_MSP:___output(ae,0xFD);___output(ae,0xE3);ae->nop+=7;ae->tick+=23;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is EX IY,(SP)\n");
				}
				break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is EX [AF,DE,HL,(SP),IX,IY],reg16\n");
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use EX reg16,[DE|(SP)]\n");
	}
}

void _SBC(struct s_assenv *ae) {
	if ((!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) || ((!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) && strcmp(ae->wl[ae->idx+1].w,"A")==0)) {
		if (!ae->wl[ae->idx+1].t) ae->idx++;
		/* do implicit A */
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_A:___output(ae,0x9F);ae->nop+=1;ae->tick+=4;break;
			case CRC_MHL:___output(ae,0x9E);ae->nop+=2;ae->tick+=7;break;
			case CRC_B:___output(ae,0x98);ae->nop+=1;ae->tick+=4;break;
			case CRC_C:___output(ae,0x99);ae->nop+=1;ae->tick+=4;break;
			case CRC_D:___output(ae,0x9A);ae->nop+=1;ae->tick+=4;break;
			case CRC_E:___output(ae,0x9B);ae->nop+=1;ae->tick+=4;break;
			case CRC_H:___output(ae,0x9C);ae->nop+=1;ae->tick+=4;break;
			case CRC_L:___output(ae,0x9D);ae->nop+=1;ae->tick+=4;break;
			case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x9C);ae->nop+=2;ae->tick+=8;break;
			case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x9D);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x9C);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x9D);ae->nop+=2;ae->tick+=8;break;
			case CRC_IX:case CRC_IY:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use SBC with A,B,C,D,E,H,L,XH,XL,YH,YL,(HL),(IX),(IY)\n");
				ae->idx++;
				return;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0x9E);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0x9E);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else {
					___output(ae,0xDE);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_V8);
					ae->nop+=2;ae->tick+=7;
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_HL:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_BC:___output(ae,0xED);___output(ae,0x42);ae->nop+=4;ae->tick+=15;break;
					case CRC_DE:___output(ae,0xED);___output(ae,0x52);ae->nop+=4;ae->tick+=15;break;
					case CRC_HL:___output(ae,0xED);___output(ae,0x62);ae->nop+=4;ae->tick+=15;break;
					case CRC_SP:___output(ae,0xED);___output(ae,0x72);ae->nop+=4;ae->tick+=15;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SBC HL,[BC,DE,HL,SP]\n");
				}
				break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SBC HL,[BC,DE,HL,SP]\n");
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid syntax for SBC\n");
	}
}

void _ADC(struct s_assenv *ae) {
	if ((!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) || ((!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) && strcmp(ae->wl[ae->idx+1].w,"A")==0)) {
		if (!ae->wl[ae->idx+1].t) ae->idx++;
		/* also implicit A */
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_A:___output(ae,0x8F);ae->nop+=1;ae->tick+=4;break;
			case CRC_MHL:___output(ae,0x8E);ae->nop+=2;ae->tick+=7;break;
			case CRC_B:___output(ae,0x88);ae->nop+=1;ae->tick+=4;break;
			case CRC_C:___output(ae,0x89);ae->nop+=1;ae->tick+=4;break;
			case CRC_D:___output(ae,0x8A);ae->nop+=1;ae->tick+=4;break;
			case CRC_E:___output(ae,0x8B);ae->nop+=1;ae->tick+=4;break;
			case CRC_H:___output(ae,0x8C);ae->nop+=1;ae->tick+=4;break;
			case CRC_L:___output(ae,0x8D);ae->nop+=1;ae->tick+=4;break;
			case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x8C);ae->nop+=2;ae->tick+=8;break;
			case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x8D);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x8C);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x8D);ae->nop+=2;ae->tick+=8;break;
			case CRC_IX:case CRC_IY:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use ADC with A,B,C,D,E,H,L,XH,XL,YH,YL,(HL),(IX),(IY)\n");
				ae->idx++;
				return;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0x8E);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0x8E);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else {
					___output(ae,0xCE);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_V8);
					ae->nop+=2;ae->tick+=7;
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_HL:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_BC:___output(ae,0xED);___output(ae,0x4A);ae->nop+=4;ae->tick+=15;break;
					case CRC_DE:___output(ae,0xED);___output(ae,0x5A);ae->nop+=4;ae->tick+=15;break;
					case CRC_HL:___output(ae,0xED);___output(ae,0x6A);ae->nop+=4;ae->tick+=15;break;
					case CRC_SP:___output(ae,0xED);___output(ae,0x7A);ae->nop+=4;ae->tick+=15;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is ADC HL,[BC,DE,HL,SP]\n");
				}
				break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is ADC HL,[BC,DE,HL,SP]\n");
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid syntax for ADC\n");
	}
}

void _ADD(struct s_assenv *ae) {
	if ((!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) || ((!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) && strcmp(ae->wl[ae->idx+1].w,"A")==0)) {
		if (!ae->wl[ae->idx+1].t) ae->idx++;
		/* also implicit A */
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_A:___output(ae,0x87);ae->nop+=1;ae->tick+=4;break;
			case CRC_MHL:___output(ae,0x86);ae->nop+=2;ae->tick+=7;break;
			case CRC_B:___output(ae,0x80);ae->nop+=1;ae->tick+=4;break;
			case CRC_C:___output(ae,0x81);ae->nop+=1;ae->tick+=4;break;
			case CRC_D:___output(ae,0x82);ae->nop+=1;ae->tick+=4;break;
			case CRC_E:___output(ae,0x83);ae->nop+=1;ae->tick+=4;break;
			case CRC_H:___output(ae,0x84);ae->nop+=1;ae->tick+=4;break;
			case CRC_L:___output(ae,0x85);ae->nop+=1;ae->tick+=4;break;
			case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x84);ae->nop+=2;ae->tick+=8;break;
			case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x85);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x84);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x85);ae->nop+=2;ae->tick+=8;break;
			case CRC_IX:case CRC_IY:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use ADD with A,B,C,D,E,H,L,XH,XL,YH,YL,(HL),(IX),(IY)\n");
				ae->idx++;
				return;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0x86);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0x86);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else {
					___output(ae,0xC6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_V8);
					ae->nop+=2;ae->tick+=7;
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_HL:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_BC:___output(ae,0x09);ae->nop+=3;ae->tick+=11;break;
					case CRC_DE:___output(ae,0x19);ae->nop+=3;ae->tick+=11;break;
					case CRC_HL:___output(ae,0x29);ae->nop+=3;ae->tick+=11;break;
					case CRC_SP:___output(ae,0x39);ae->nop+=3;ae->tick+=11;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is ADD HL,[BC,DE,HL,SP]\n");
				}
				break;
			case CRC_IX:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_BC:___output(ae,0xDD);___output(ae,0x09);ae->nop+=4;ae->tick+=15;break;
					case CRC_DE:___output(ae,0xDD);___output(ae,0x19);ae->nop+=4;ae->tick+=15;break;
					case CRC_IX:___output(ae,0xDD);___output(ae,0x29);ae->nop+=4;ae->tick+=15;break;
					case CRC_SP:___output(ae,0xDD);___output(ae,0x39);ae->nop+=4;ae->tick+=15;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is ADD IX,[BC,DE,IX,SP]\n");
				}
				break;
			case CRC_IY:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_BC:___output(ae,0xFD);___output(ae,0x09);ae->nop+=4;ae->tick+=15;break;
					case CRC_DE:___output(ae,0xFD);___output(ae,0x19);ae->nop+=4;ae->tick+=15;break;
					case CRC_IY:___output(ae,0xFD);___output(ae,0x29);ae->nop+=4;ae->tick+=15;break;
					case CRC_SP:___output(ae,0xFD);___output(ae,0x39);ae->nop+=4;ae->tick+=15;break;
					default:
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is ADD IY,[BC,DE,IY,SP]\n");
				}
				break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is ADD [HL,IX,IY],reg16\n");
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid syntax for ADD\n");
	}
}

void _CP(struct s_assenv *ae) {
	if ((!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) || ((!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) && strcmp(ae->wl[ae->idx+1].w,"A")==0)) {
		if (!ae->wl[ae->idx+1].t) ae->idx++;
		/* also implicit A */
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_A:___output(ae,0xBF);ae->nop+=1;ae->tick+=4;break;
			case CRC_MHL:___output(ae,0xBE);ae->nop+=2;ae->tick+=7;break;
			case CRC_B:___output(ae,0xB8);ae->nop+=1;ae->tick+=4;break;
			case CRC_C:___output(ae,0xB9);ae->nop+=1;ae->tick+=4;break;
			case CRC_D:___output(ae,0xBA);ae->nop+=1;ae->tick+=4;break;
			case CRC_E:___output(ae,0xBB);ae->nop+=1;ae->tick+=4;break;
			case CRC_H:___output(ae,0xBC);ae->nop+=1;ae->tick+=4;break;
			case CRC_L:___output(ae,0xBD);ae->nop+=1;ae->tick+=4;break;
			case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0xBC);ae->nop+=2;ae->tick+=8;break;
			case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0xBD);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0xBC);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0xBD);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0xBE);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0xBE);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else {
					___output(ae,0xFE);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_V8);
					ae->nop+=2;ae->tick+=7;
				}
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Syntax is CP reg8/(reg16)\n");
	}
}

void _RET(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_NZ:___output(ae,0xC0);ae->nop+=2;ae->tick+=5;break;
			case CRC_Z:___output(ae,0xC8);ae->nop+=2;ae->tick+=5;break;
			case CRC_C:___output(ae,0xD8);ae->nop+=2;ae->tick+=5;break;
			case CRC_NC:___output(ae,0xD0);ae->nop+=2;ae->tick+=5;break;
			case CRC_PE:___output(ae,0xE8);ae->nop+=2;ae->tick+=5;break;
			case CRC_PO:___output(ae,0xE0);ae->nop+=2;ae->tick+=5;break;
			case CRC_P:___output(ae,0xF0);ae->nop+=2;ae->tick+=5;break;
			case CRC_M:___output(ae,0xF8);ae->nop+=2;ae->tick+=5;break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Available flags for RET are C,NC,Z,NZ,PE,PO,P,M\n");
		}
		ae->idx++;
	} else if (ae->wl[ae->idx].t==1) {
		___output(ae,0xC9);
		ae->nop+=3;ae->tick+=10;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid RET syntax\n");
	}
}

void _CALL(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==0 && ae->wl[ae->idx+2].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_C:___output(ae,0xDC);ae->nop+=3;ae->tick+=10;break;
			case CRC_Z:___output(ae,0xCC);ae->nop+=3;ae->tick+=10;break;
			case CRC_NZ:___output(ae,0xC4);ae->nop+=3;ae->tick+=10;break;
			case CRC_NC:___output(ae,0xD4);ae->nop+=3;ae->tick+=10;break;
			case CRC_PE:___output(ae,0xEC);ae->nop+=3;ae->tick+=10;break;
			case CRC_PO:___output(ae,0xE4);ae->nop+=3;ae->tick+=10;break;
			case CRC_P:___output(ae,0xF4);ae->nop+=3;ae->tick+=10;break;
			case CRC_M:___output(ae,0xFC);ae->nop+=3;ae->tick+=10;break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Available flags for CALL are C,NC,Z,NZ,PE,PO,P,M\n");
		}
		PushExpression(ae,ae->idx+2,E_EXPRESSION_V16C);
		ae->idx+=2;
	} else if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		___output(ae,0xCD);
		PushExpression(ae,ae->idx+1,E_EXPRESSION_V16C);
		ae->idx++;
		ae->nop+=5;ae->tick+=17;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid CALL syntax\n");
	}
}

void _JR(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==0 && ae->wl[ae->idx+2].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_NZ:___output(ae,0x20);ae->nop+=2;ae->tick+=7;break;
			case CRC_C:___output(ae,0x38);ae->nop+=2;ae->tick+=7;break;
			case CRC_Z:___output(ae,0x28);ae->nop+=2;ae->tick+=7;break;
			case CRC_NC:___output(ae,0x30);ae->nop+=2;ae->tick+=7;break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Available flags for JR are C,NC,Z,NZ\n");
		}
		PushExpression(ae,ae->idx+2,E_EXPRESSION_J8);
		ae->idx+=2;
	} else if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		___output(ae,0x18);
		PushExpression(ae,ae->idx+1,E_EXPRESSION_J8);
		ae->idx++;
		ae->nop+=3;ae->tick+=12;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid JR syntax\n");
	}
}

void _JP(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==0 && ae->wl[ae->idx+2].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_C:___output(ae,0xDA);ae->nop+=3;ae->tick+=10;break;
			case CRC_Z:___output(ae,0xCA);ae->nop+=3;ae->tick+=10;break;
			case CRC_NZ:___output(ae,0xC2);ae->nop+=3;ae->tick+=10;break;
			case CRC_NC:___output(ae,0xD2);ae->nop+=3;ae->tick+=10;break;
			case CRC_PE:___output(ae,0xEA);ae->nop+=3;ae->tick+=10;break;
			case CRC_PO:___output(ae,0xE2);ae->nop+=3;ae->tick+=10;break;
			case CRC_P:___output(ae,0xF2);ae->nop+=3;ae->tick+=10;break;
			case CRC_M:___output(ae,0xFA);ae->nop+=3;ae->tick+=10;break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Available flags for JP are C,NC,Z,NZ,PE,PO,P,M\n");
		}
		if (!strcmp(ae->wl[ae->idx+2].w,"(IX)") || !strcmp(ae->wl[ae->idx+2].w,"(IY)")) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"conditionnal JP cannot use register adressing\n");
		} else {
			PushExpression(ae,ae->idx+2,E_EXPRESSION_V16);
		}
		ae->idx+=2;
	} else if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_HL:case CRC_MHL:___output(ae,0xE9);ae->nop+=1;ae->tick+=4;break;
			case CRC_IX:case CRC_MIX:___output(ae,0xDD);___output(ae,0xE9);ae->nop+=2;ae->tick+=8;break;
			case CRC_IY:case CRC_MIY:___output(ae,0xFD);___output(ae,0xE9);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0 || strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"JP (IX) or JP (IY) only\n");
				} else {
					___output(ae,0xC3);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_V16);
					ae->tick+=10;
					ae->nop+=3;
				}
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid JP syntax\n");
	}
}


void _DEC(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		do {
			switch (GetCRC(ae->wl[ae->idx+1].w)) {
				case CRC_A:___output(ae,0x3D);ae->nop+=1;ae->tick+=4;break;
				case CRC_B:___output(ae,0x05);ae->nop+=1;ae->tick+=4;break;
				case CRC_C:___output(ae,0x0D);ae->nop+=1;ae->tick+=4;break;
				case CRC_D:___output(ae,0x15);ae->nop+=1;ae->tick+=4;break;
				case CRC_E:___output(ae,0x1D);ae->nop+=1;ae->tick+=4;break;
				case CRC_H:___output(ae,0x25);ae->nop+=1;ae->tick+=4;break;
				case CRC_L:___output(ae,0x2D);ae->nop+=1;ae->tick+=4;break;
				case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x25);ae->nop+=2;ae->tick+=8;break;
				case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x2D);ae->nop+=2;ae->tick+=8;break;
				case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x25);ae->nop+=2;ae->tick+=8;break;
				case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x2D);ae->nop+=2;ae->tick+=8;break;
				case CRC_BC:___output(ae,0x0B);ae->nop+=2;ae->tick+=6;break;
				case CRC_DE:___output(ae,0x1B);ae->nop+=2;ae->tick+=6;break;
				case CRC_HL:___output(ae,0x2B);ae->nop+=2;ae->tick+=6;break;
				case CRC_IX:___output(ae,0xDD);___output(ae,0x2B);ae->nop+=3;ae->tick+=10;break;
				case CRC_IY:___output(ae,0xFD);___output(ae,0x2B);ae->nop+=3;ae->tick+=10;break;
				case CRC_SP:___output(ae,0x3B);ae->nop+=2;ae->tick+=6;break;
				case CRC_MHL:___output(ae,0x35);ae->nop+=3;ae->tick+=11;break;
				default:
					if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0x35);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
						ae->nop+=6;ae->tick+=23;
					} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0x35);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
						ae->nop+=6;ae->tick+=23;
					} else {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use DEC with A,B,C,D,E,H,L,XH,XL,YH,YL,BC,DE,HL,SP,(HL),(IX),(IY)\n");
					}
			}
			ae->idx++;
		} while (ae->wl[ae->idx].t==0);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use DEC with A,B,C,D,E,H,L,XH,XL,YH,YL,BC,DE,HL,SP,(HL),(IX),(IY)\n");
	}
}
void _INC(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		do {
			switch (GetCRC(ae->wl[ae->idx+1].w)) {
				case CRC_A:___output(ae,0x3C);ae->nop+=1;ae->tick+=4;break;
				case CRC_B:___output(ae,0x04);ae->nop+=1;ae->tick+=4;break;
				case CRC_C:___output(ae,0x0C);ae->nop+=1;ae->tick+=4;break;
				case CRC_D:___output(ae,0x14);ae->nop+=1;ae->tick+=4;break;
				case CRC_E:___output(ae,0x1C);ae->nop+=1;ae->tick+=4;break;
				case CRC_H:___output(ae,0x24);ae->nop+=1;ae->tick+=4;break;
				case CRC_L:___output(ae,0x2C);ae->nop+=1;ae->tick+=4;break;
				case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x24);ae->nop+=2;ae->tick+=8;break;
				case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x2C);ae->nop+=2;ae->tick+=8;break;
				case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x24);ae->nop+=2;ae->tick+=8;break;
				case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x2C);ae->nop+=2;ae->tick+=8;break;
				case CRC_BC:___output(ae,0x03);ae->nop+=2;ae->tick+=6;break;
				case CRC_DE:___output(ae,0x13);ae->nop+=2;ae->tick+=6;break;
				case CRC_HL:___output(ae,0x23);ae->nop+=2;ae->tick+=6;break;
				case CRC_IX:___output(ae,0xDD);___output(ae,0x23);ae->nop+=3;ae->tick+=10;break;
				case CRC_IY:___output(ae,0xFD);___output(ae,0x23);ae->nop+=3;ae->tick+=10;break;
				case CRC_SP:___output(ae,0x33);ae->nop+=2;ae->tick+=6;break;
				case CRC_MHL:___output(ae,0x34);ae->nop+=3;ae->tick+=11;break;
				default:
					if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0x34);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
						ae->nop+=6;ae->tick+=23;
					} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0x34);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
						ae->nop+=6;ae->tick+=23;
					} else {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use INC with A,B,C,D,E,H,L,XH,XL,YH,YL,BC,DE,HL,SP,(HL),(IX),(IY)\n");
					}
			}
			ae->idx++;
		} while (ae->wl[ae->idx].t==0);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use INC with A,B,C,D,E,H,L,XH,XL,YH,YL,BC,DE,HL,SP,(HL),(IX),(IY)\n");
	}
}

void _SUB(struct s_assenv *ae) {
	#ifdef OPCODE
	#undef OPCODE
	#endif
	#define OPCODE 0x90
	
	if ((!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1)  || ((!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) && strcmp(ae->wl[ae->idx+1].w,"A")==0)) {
		if (!ae->wl[ae->idx+1].t) ae->idx++;
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_A:___output(ae,OPCODE+7);ae->nop+=1;ae->tick+=4;break;
			case CRC_MHL:___output(ae,OPCODE+6);ae->nop+=2;ae->tick+=7;break;
			case CRC_B:___output(ae,OPCODE);ae->nop+=1;ae->tick+=4;break;
			case CRC_C:___output(ae,OPCODE+1);ae->nop+=1;ae->tick+=4;break;
			case CRC_D:___output(ae,OPCODE+2);ae->nop+=1;ae->tick+=4;break;
			case CRC_E:___output(ae,OPCODE+3);ae->nop+=1;ae->tick+=4;break;
			case CRC_H:___output(ae,OPCODE+4);ae->nop+=1;ae->tick+=4;break;
			case CRC_L:___output(ae,OPCODE+5);ae->nop+=1;ae->tick+=4;break;
			case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,OPCODE+4);ae->nop+=2;ae->tick+=8;break;
			case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,OPCODE+5);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,OPCODE+4);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,OPCODE+5);ae->nop+=2;ae->tick+=8;break;
			case CRC_IX:case CRC_IY:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use SUB with A,B,C,D,E,H,L,XH,XL,YH,YL,(HL),(IX),(IY)\n");
				ae->idx++;
				return;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,OPCODE+6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,OPCODE+6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else {
					___output(ae,0xD6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_V8);
					ae->nop+=2;ae->tick+=7;
				}
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use SUB with A,B,C,D,E,H,L,XH,XL,YH,YL,(HL),(IX),(IY)\n");
	}
}
void _AND(struct s_assenv *ae) {
	#ifdef OPCODE
	#undef OPCODE
	#endif
	#define OPCODE 0xA0
	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_A:___output(ae,OPCODE+7);ae->nop+=1;ae->tick+=4;break;
			case CRC_MHL:___output(ae,OPCODE+6);ae->nop+=2;ae->tick+=7;break;
			case CRC_B:___output(ae,OPCODE);ae->nop+=1;ae->tick+=4;break;
			case CRC_C:___output(ae,OPCODE+1);ae->nop+=1;ae->tick+=4;break;
			case CRC_D:___output(ae,OPCODE+2);ae->nop+=1;ae->tick+=4;break;
			case CRC_E:___output(ae,OPCODE+3);ae->nop+=1;ae->tick+=4;break;
			case CRC_H:___output(ae,OPCODE+4);ae->nop+=1;ae->tick+=4;break;
			case CRC_L:___output(ae,OPCODE+5);ae->nop+=1;ae->tick+=4;break;
			case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,OPCODE+4);ae->nop+=2;ae->tick+=8;break;
			case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,OPCODE+5);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,OPCODE+4);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,OPCODE+5);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,OPCODE+6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,OPCODE+6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else {
					___output(ae,0xE6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_V8);
					ae->nop+=2;ae->tick+=7;
				}
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use AND with A,B,C,D,E,H,L,XH,XL,YH,YL,(HL),(IX),(IY)\n");
	}
}
void _OR(struct s_assenv *ae) {
	#ifdef OPCODE
	#undef OPCODE
	#endif
	#define OPCODE 0xB0
	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_A:___output(ae,OPCODE+7);ae->nop+=1;ae->tick+=4;break;
			case CRC_MHL:___output(ae,OPCODE+6);ae->nop+=2;ae->tick+=7;break;
			case CRC_B:___output(ae,OPCODE);ae->nop+=1;ae->tick+=4;break;
			case CRC_C:___output(ae,OPCODE+1);ae->nop+=1;ae->tick+=4;break;
			case CRC_D:___output(ae,OPCODE+2);ae->nop+=1;ae->tick+=4;break;
			case CRC_E:___output(ae,OPCODE+3);ae->nop+=1;ae->tick+=4;break;
			case CRC_H:___output(ae,OPCODE+4);ae->nop+=1;ae->tick+=4;break;
			case CRC_L:___output(ae,OPCODE+5);ae->nop+=1;ae->tick+=4;break;
			case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,OPCODE+4);ae->nop+=2;ae->tick+=8;break;
			case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,OPCODE+5);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,OPCODE+4);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,OPCODE+5);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,OPCODE+6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,OPCODE+6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else {
					___output(ae,0xF6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_V8);
					ae->nop+=2;ae->tick+=7;
				}
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use OR with A,B,C,D,E,H,L,XH,XL,YH,YL,(HL),(IX),(IY)\n");
	}
}
void _XOR(struct s_assenv *ae) {
	#ifdef OPCODE
	#undef OPCODE
	#endif
	#define OPCODE 0xA8
	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_A:___output(ae,OPCODE+7);ae->nop+=1;ae->tick+=4;break;
			case CRC_MHL:___output(ae,OPCODE+6);ae->nop+=2;ae->tick+=7;break;
			case CRC_B:___output(ae,OPCODE);ae->nop+=1;ae->tick+=4;break;
			case CRC_C:___output(ae,OPCODE+1);ae->nop+=1;ae->tick+=4;break;
			case CRC_D:___output(ae,OPCODE+2);ae->nop+=1;ae->tick+=4;break;
			case CRC_E:___output(ae,OPCODE+3);ae->nop+=1;ae->tick+=4;break;
			case CRC_H:___output(ae,OPCODE+4);ae->nop+=1;ae->tick+=4;break;
			case CRC_L:___output(ae,OPCODE+5);ae->nop+=1;ae->tick+=4;break;
			case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,OPCODE+4);ae->nop+=2;ae->tick+=8;break;
			case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,OPCODE+5);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,OPCODE+4);ae->nop+=2;ae->tick+=8;break;
			case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,OPCODE+5);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,OPCODE+6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,OPCODE+6);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					ae->nop+=5;ae->tick+=19;
				} else {
					___output(ae,0xEE);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_V8);
					ae->nop+=2;ae->tick+=7;
				}
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use XOR with A,B,C,D,E,H,L,XH,XL,YH,YL,(HL),(IX),(IY)\n");
	}
}


void _POP(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			switch (GetCRC(ae->wl[ae->idx].w)) {
				case CRC_AF:___output(ae,0xF1);ae->nop+=3;ae->tick+=10;break;
				case CRC_BC:___output(ae,0xC1);ae->nop+=3;ae->tick+=10;break;
				case CRC_DE:___output(ae,0xD1);ae->nop+=3;ae->tick+=10;break;
				case CRC_HL:___output(ae,0xE1);ae->nop+=3;ae->tick+=10;break;
				case CRC_IX:___output(ae,0xDD);___output(ae,0xE1);ae->nop+=4;ae->tick+=14;break;
				case CRC_IY:___output(ae,0xFD);___output(ae,0xE1);ae->nop+=4;ae->tick+=14;break;
				default:
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use POP with AF,BC,DE,HL,IX,IY\n");
			}
		} while (ae->wl[ae->idx].t!=1);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"POP need at least one parameter\n");
	}
}
void _PUSH(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			switch (GetCRC(ae->wl[ae->idx].w)) {
				case CRC_AF:___output(ae,0xF5);ae->nop+=4;ae->tick+=11;break;
				case CRC_BC:___output(ae,0xC5);ae->nop+=4;ae->tick+=11;break;
				case CRC_DE:___output(ae,0xD5);ae->nop+=4;ae->tick+=11;break;
				case CRC_HL:___output(ae,0xE5);ae->nop+=4;ae->tick+=11;break;
				case CRC_IX:___output(ae,0xDD);___output(ae,0xE5);ae->nop+=5;ae->tick+=15;break;
				case CRC_IY:___output(ae,0xFD);___output(ae,0xE5);ae->nop+=5;ae->tick+=15;break;
				default:
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use PUSH with AF,BC,DE,HL,IX,IY\n");
			}
		} while (ae->wl[ae->idx].t!=1);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"PUSH need at least one parameter\n");
	}
}

void _IM(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		/* la valeur du parametre va definir l'opcode du IM */
		___output(ae,0xED);
		PushExpression(ae,ae->idx+1,E_EXPRESSION_IM);
		ae->idx++;
		ae->nop+=2;
		ae->tick+=8;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IM need one parameter\n");
	}
}

void _RLCA(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0x7);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RLCA does not need parameter\n");
	}
}
void _RRCA(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xF);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RRCA does not need parameter\n");
	}
}
void _NEG(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0x44);
		ae->nop+=2;
		ae->tick+=8;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"NEG does not need parameter\n");
	}
}
void _DAA(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0x27);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DAA does not need parameter\n");
	}
}
void _CPL(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0x2F);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CPL does not need parameter\n");
	}
}
void _RETI(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0x4D);
		ae->nop+=4;
		ae->tick+=14;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RETI does not need parameter\n");
	}
}
void _SCF(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0x37);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SCF does not need parameter\n");
	}
}
void _LDD(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xA8);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LDD does not need parameter\n");
	}
}
void _LDDR(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xB8);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LDDR does not need parameter\n");
	}
}
void _LDI(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xA0);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LDI does not need parameter\n");
	}
}
void _LDIR(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xB0);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LDIR does not need parameter\n");
	}
}
void _CCF(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0x3F);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CCF does not need parameter\n");
	}
}
void _CPD(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xA9);
		ae->nop+=4;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CPD does not need parameter\n");
	}
}
void _CPDR(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xB9);
		ae->nop+=4;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CPDR does not need parameter\n");
	}
}
void _CPI(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xA1);
		ae->nop+=4;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CPI does not need parameter\n");
	}
}
void _CPIR(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xB1);
		ae->nop+=4;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CPIR does not need parameter\n");
	}
}
void _OUTD(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xAB);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"OUTD does not need parameter\n");
	}
}
void _OTDR(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xBB);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"OTDR does not need parameter\n");
	}
}
void _OUTI(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xA3);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"OUTI does not need parameter\n");
	}
}
void _OTIR(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xB3);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"OTIR does not need parameter\n");
	}
}
void _RETN(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0x45);
		ae->nop+=4;
		ae->tick+=14;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RETN does not need parameter\n");
	}
}
void _IND(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xAA);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IND does not need parameter\n");
	}
}
void _INDR(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xBA);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INDR does not need parameter\n");
	}
}
void _INI(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xED);
		___output(ae,0xA2);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INI does not need parameter\n");
	}
}
void _INIR(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t==1) {
		___output(ae,0xED);
		___output(ae,0xB2);
		ae->nop+=5;
		ae->tick+=16;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INIR does not need parameter\n");
	}
}
void _EXX(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t==1) {
		___output(ae,0xD9);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"EXX does not need parameter\n");
	}
}
void _HALT(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t==1) {
		___output(ae,0x76);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"HALT does not need parameter\n");
	}
}

void _RLA(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t==1) {
		___output(ae,0x17);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RLA does not need parameter\n");
	}
}
void _RRA(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t==1) {
		___output(ae,0x1F);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RRA does not need parameter\n");
	}
}
void _RLD(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t==1) {
		___output(ae,0xED);
		___output(ae,0x6F);
		ae->nop+=5;
		ae->tick+=18;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RLD does not need parameter\n");
	}
}
void _RRD(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t==1) {
		___output(ae,0xED);
		___output(ae,0x67);
		ae->nop+=5;
		ae->tick+=18;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RRD does not need parameter\n");
	}
}


void _EXA(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t==1) {
		___output(ae,0x08);ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"EXA alias does not need parameter\n");
	}
}

void _NOP(struct s_assenv *ae) {
	int o;

	if (ae->wl[ae->idx].t) {
		___output(ae,0x00);
		ae->nop+=1;
		ae->tick+=4;
	} else if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		o=RoundComputeExpressionCore(ae,ae->wl[ae->idx+1].w,ae->codeadr,0);
		if (o>=0) {
			while (o>0) {
				___output(ae,0x00);
				ae->nop+=1;
				ae->tick+=4;
				o--;
			}
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"NOP is supposed to be used without parameter or with one optional parameter\n");
	}
}
void _DI(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
	___output(ae,0xF3);
	ae->nop+=1;
	ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DI does not need parameter\n");
	}
}
void _EI(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___output(ae,0xFB);
		ae->nop+=1;
		ae->tick+=4;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"EI does not need parameter\n");
	}
}

void _RST(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t!=2) {
		if (!strcmp(ae->wl[ae->idx+1].w,"(IY)") || !strcmp(ae->wl[ae->idx+1].w,"(IX)")) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RST cannot use IX or IY\n");
		} else {
			/* la valeur du parametre va definir l'opcode du RST */
			PushExpression(ae,ae->idx+1,E_EXPRESSION_RST);
		}
		ae->idx++;
		ae->nop+=4;
		ae->tick+=11;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"RST need one parameter\n");
	}
}

void _DJNZ(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		if (IsRegister(ae->wl[ae->idx+1].w)) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DJNZ cannot use register\n");
		} else if (strcmp("(IX)",ae->wl[ae->idx+1].w)==0 || strcmp("(IY)",ae->wl[ae->idx+1].w)==0) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DJNZ cannot use register\n");
		} else {
			___output(ae,0x10);
			PushExpression(ae,ae->idx+1,E_EXPRESSION_J8);
			ae->nop+=3;
			ae->tick+=13;
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DJNZ need one parameter\n");
	}
}

void _LD(struct s_assenv *ae) {
	/* on check qu'il y a au moins deux parametres */
	if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_A:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_I:___output(ae,0xED);___output(ae,0x57);ae->nop+=3;ae->tick+=9;break;
					case CRC_R:___output(ae,0xED);___output(ae,0x5F);ae->nop+=3;ae->tick+=9;break;
					case CRC_B:___output(ae,0x78);ae->nop+=1;ae->tick+=4;break;
					case CRC_C:___output(ae,0x79);ae->nop+=1;ae->tick+=4;break;
					case CRC_D:___output(ae,0x7A);ae->nop+=1;ae->tick+=4;break;
					case CRC_E:___output(ae,0x7B);ae->nop+=1;ae->tick+=4;break;
					case CRC_H:___output(ae,0x7C);ae->nop+=1;ae->tick+=4;break;
					case CRC_L:___output(ae,0x7D);ae->nop+=1;ae->tick+=4;break;
					case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x7C);ae->nop+=2;ae->tick+=8;break;
					case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x7D);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x7C);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x7D);ae->nop+=2;ae->tick+=8;break;
					case CRC_MHL:___output(ae,0x7E);ae->nop+=2;ae->tick+=7;break;
					case CRC_A:___output(ae,0x7F);ae->nop+=1;ae->tick+=4;break;
					case CRC_MBC:___output(ae,0x0A);ae->nop+=2;ae->tick+=7;break;
					case CRC_MDE:___output(ae,0x1A);ae->nop+=2;ae->tick+=7;break;
					default:
					/* (ix+expression) (iy+expression) (expression) expression */
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0x7E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0x7E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else if (StringIsMem(ae->wl[ae->idx+2].w)) {
						___output(ae,0x3A);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V16);
						ae->nop+=4;ae->tick+=13;
					} else {
						___output(ae,0x3E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=2;ae->tick+=7;
					}
				}
				break;
			case CRC_I:
				if (GetCRC(ae->wl[ae->idx+2].w)==CRC_A) {
					___output(ae,0xED);___output(ae,0x47);
					ae->nop+=3;ae->tick+=9;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LD I,A only\n");
				}
				break;
			case CRC_R:
				if (GetCRC(ae->wl[ae->idx+2].w)==CRC_A) {
					___output(ae,0xED);___output(ae,0x4F);
					ae->nop+=3;ae->tick+=9;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LD R,A only\n");
				}
				break;
			case CRC_B:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0x40);ae->nop+=1;ae->tick+=4;break;
					case CRC_C:___output(ae,0x41);ae->nop+=1;ae->tick+=4;break;
					case CRC_D:___output(ae,0x42);ae->nop+=1;ae->tick+=4;break;
					case CRC_E:___output(ae,0x43);ae->nop+=1;ae->tick+=4;break;
					case CRC_H:___output(ae,0x44);ae->nop+=1;ae->tick+=4;break;
					case CRC_L:___output(ae,0x45);ae->nop+=1;ae->tick+=4;break;
					case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x44);ae->nop+=2;ae->tick+=8;break;
					case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x45);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x44);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x45);ae->nop+=2;ae->tick+=8;break;
					case CRC_MHL:___output(ae,0x46);ae->nop+=2;ae->tick+=7;break;
					case CRC_A:___output(ae,0x47);ae->nop+=1;ae->tick+=4;break;
					default:
					/* (ix+expression) (iy+expression) expression */
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0x46);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0x46);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else {
						___output(ae,0x06);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=2;ae->tick+=7;
					}
				}
				break;
			case CRC_C:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0x48);ae->nop+=1;ae->tick+=4;break;
					case CRC_C:___output(ae,0x49);ae->nop+=1;ae->tick+=4;break;
					case CRC_D:___output(ae,0x4A);ae->nop+=1;ae->tick+=4;break;
					case CRC_E:___output(ae,0x4B);ae->nop+=1;ae->tick+=4;break;
					case CRC_H:___output(ae,0x4C);ae->nop+=1;ae->tick+=4;break;
					case CRC_L:___output(ae,0x4D);ae->nop+=1;ae->tick+=4;break;
					case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x4C);ae->nop+=2;ae->tick+=8;break;
					case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x4D);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x4C);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x4D);ae->nop+=2;ae->tick+=8;break;
					case CRC_MHL:___output(ae,0x4E);ae->nop+=2;ae->tick+=7;break;
					case CRC_A:___output(ae,0x4F);ae->nop+=1;ae->tick+=4;break;
					default:
					/* (ix+expression) (iy+expression) expression */
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0x4E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0x4E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else {
						___output(ae,0x0E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=2;ae->tick+=7;
					}
				}
				break;
			case CRC_D:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0x50);ae->nop+=1;ae->tick+=4;break;
					case CRC_C:___output(ae,0x51);ae->nop+=1;ae->tick+=4;break;
					case CRC_D:___output(ae,0x52);ae->nop+=1;ae->tick+=4;break;
					case CRC_E:___output(ae,0x53);ae->nop+=1;ae->tick+=4;break;
					case CRC_H:___output(ae,0x54);ae->nop+=1;ae->tick+=4;break;
					case CRC_L:___output(ae,0x55);ae->nop+=1;ae->tick+=4;break;
					case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x54);ae->nop+=2;ae->tick+=8;break;
					case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x55);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x54);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x55);ae->nop+=2;ae->tick+=8;break;
					case CRC_MHL:___output(ae,0x56);ae->nop+=2;ae->tick+=7;break;
					case CRC_A:___output(ae,0x57);ae->nop+=1;ae->tick+=4;break;
					default:
					/* (ix+expression) (iy+expression) expression */
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0x56);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0x56);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else {
						___output(ae,0x16);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=2;ae->tick+=7;
					}
				}
				break;
			case CRC_E:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0x58);ae->nop+=1;ae->tick+=4;break;
					case CRC_C:___output(ae,0x59);ae->nop+=1;ae->tick+=4;break;
					case CRC_D:___output(ae,0x5A);ae->nop+=1;ae->tick+=4;break;
					case CRC_E:___output(ae,0x5B);ae->nop+=1;ae->tick+=4;break;
					case CRC_H:___output(ae,0x5C);ae->nop+=1;ae->tick+=4;break;
					case CRC_L:___output(ae,0x5D);ae->nop+=1;ae->tick+=4;break;
					case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x5C);ae->nop+=2;ae->tick+=8;break;
					case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x5D);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x5C);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x5D);ae->nop+=2;ae->tick+=8;break;
					case CRC_MHL:___output(ae,0x5E);ae->nop+=2;ae->tick+=7;break;
					case CRC_A:___output(ae,0x5F);ae->nop+=1;ae->tick+=4;break;
					default:
					/* (ix+expression) (iy+expression) expression */
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0x5E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0x5E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else {
						___output(ae,0x1E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=2;ae->tick+=7;
					}
				}
				break;
			case CRC_IYH:case CRC_HY:case CRC_YH:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0xFD);___output(ae,0x60);ae->nop+=2;ae->tick+=8;break;
					case CRC_C:___output(ae,0xFD);___output(ae,0x61);ae->nop+=2;ae->tick+=8;break;
					case CRC_D:___output(ae,0xFD);___output(ae,0x62);ae->nop+=2;ae->tick+=8;break;
					case CRC_E:___output(ae,0xFD);___output(ae,0x63);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x64);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x65);ae->nop+=2;ae->tick+=8;break;
					case CRC_A:___output(ae,0xFD);___output(ae,0x67);ae->nop+=2;ae->tick+=8;break;
					default:
						___output(ae,0xFD);___output(ae,0x26);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=3;ae->tick+=11;
				}
				break;
			case CRC_IYL:case CRC_LY:case CRC_YL:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0xFD);___output(ae,0x68);ae->nop+=2;ae->tick+=8;break;
					case CRC_C:___output(ae,0xFD);___output(ae,0x69);ae->nop+=2;ae->tick+=8;break;
					case CRC_D:___output(ae,0xFD);___output(ae,0x6A);ae->nop+=2;ae->tick+=8;break;
					case CRC_E:___output(ae,0xFD);___output(ae,0x6B);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYH:case CRC_HY:case CRC_YH:___output(ae,0xFD);___output(ae,0x6C);ae->nop+=2;ae->tick+=8;break;
					case CRC_IYL:case CRC_LY:case CRC_YL:___output(ae,0xFD);___output(ae,0x6D);ae->nop+=2;ae->tick+=8;break;
					case CRC_A:___output(ae,0xFD);___output(ae,0x6F);ae->nop+=2;ae->tick+=8;break;
					default:
						___output(ae,0xFD);___output(ae,0x2E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=3;ae->tick+=11;
				}
				break;
			case CRC_IXH:case CRC_HX:case CRC_XH:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0xDD);___output(ae,0x60);ae->nop+=2;ae->tick+=8;break;
					case CRC_C:___output(ae,0xDD);___output(ae,0x61);ae->nop+=2;ae->tick+=8;break;
					case CRC_D:___output(ae,0xDD);___output(ae,0x62);ae->nop+=2;ae->tick+=8;break;
					case CRC_E:___output(ae,0xDD);___output(ae,0x63);ae->nop+=2;ae->tick+=8;break;
					case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x64);ae->nop+=2;ae->tick+=8;break;
					case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x65);ae->nop+=2;ae->tick+=8;break;
					case CRC_A:___output(ae,0xDD);___output(ae,0x67);ae->nop+=2;ae->tick+=8;break;
					default:
						___output(ae,0xDD);___output(ae,0x26);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=3;ae->tick+=11;
				}
				break;
			case CRC_IXL:case CRC_LX:case CRC_XL:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0xDD);___output(ae,0x68);ae->nop+=2;ae->tick+=8;break;
					case CRC_C:___output(ae,0xDD);___output(ae,0x69);ae->nop+=2;ae->tick+=8;break;
					case CRC_D:___output(ae,0xDD);___output(ae,0x6A);ae->nop+=2;ae->tick+=8;break;
					case CRC_E:___output(ae,0xDD);___output(ae,0x6B);ae->nop+=2;ae->tick+=8;break;
					case CRC_IXH:case CRC_HX:case CRC_XH:___output(ae,0xDD);___output(ae,0x6C);ae->nop+=2;ae->tick+=8;break;
					case CRC_IXL:case CRC_LX:case CRC_XL:___output(ae,0xDD);___output(ae,0x6D);ae->nop+=2;ae->tick+=8;break;
					case CRC_A:___output(ae,0xDD);___output(ae,0x6F);ae->nop+=2;ae->tick+=8;break;
					default:
						___output(ae,0xDD);___output(ae,0x2E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=3;ae->tick+=11;
				}
				break;
			case CRC_H:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0x60);ae->nop+=1;ae->tick+=4;break;
					case CRC_C:___output(ae,0x61);ae->nop+=1;ae->tick+=4;break;
					case CRC_D:___output(ae,0x62);ae->nop+=1;ae->tick+=4;break;
					case CRC_E:___output(ae,0x63);ae->nop+=1;ae->tick+=4;break;
					case CRC_H:___output(ae,0x64);ae->nop+=1;ae->tick+=4;break;
					case CRC_L:___output(ae,0x65);ae->nop+=1;ae->tick+=4;break;
					case CRC_MHL:___output(ae,0x66);ae->nop+=2;ae->tick+=7;break;
					case CRC_A:___output(ae,0x67);ae->nop+=1;ae->tick+=4;break;
					default:
					/* (ix+expression) (iy+expression) expression */
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0x66);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0x66);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else {
						___output(ae,0x26);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=2;ae->tick+=7;
					}
				}
				break;
			case CRC_L:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0x68);ae->nop+=1;ae->tick+=4;break;
					case CRC_C:___output(ae,0x69);ae->nop+=1;ae->tick+=4;break;
					case CRC_D:___output(ae,0x6A);ae->nop+=1;ae->tick+=4;break;
					case CRC_E:___output(ae,0x6B);ae->nop+=1;ae->tick+=4;break;
					case CRC_H:___output(ae,0x6C);ae->nop+=1;ae->tick+=4;break;
					case CRC_L:___output(ae,0x6D);ae->nop+=1;ae->tick+=4;break;
					case CRC_MHL:___output(ae,0x6E);ae->nop+=2;ae->tick+=7;break;
					case CRC_A:___output(ae,0x6F);ae->nop+=1;ae->tick+=4;break;
					default:
					/* (ix+expression) (iy+expression) expression */
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0x6E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0x6E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=5;ae->tick+=19;
					} else {
						___output(ae,0x2E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
						ae->nop+=2;ae->tick+=7;
					}
				}
				break;
			case CRC_MHL:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_B:___output(ae,0x70);ae->nop+=2;ae->tick+=7;break;
					case CRC_C:___output(ae,0x71);ae->nop+=2;ae->tick+=7;break;
					case CRC_D:___output(ae,0x72);ae->nop+=2;ae->tick+=7;break;
					case CRC_E:___output(ae,0x73);ae->nop+=2;ae->tick+=7;break;
					case CRC_H:___output(ae,0x74);ae->nop+=2;ae->tick+=7;break;
					case CRC_L:___output(ae,0x75);ae->nop+=2;ae->tick+=7;break;
					case CRC_A:___output(ae,0x77);ae->nop+=2;ae->tick+=7;break;
					default:
					/* expression */
					___output(ae,0x36);
					PushExpression(ae,ae->idx+2,E_EXPRESSION_V8);
					ae->nop+=3;ae->tick+=10;
				}
				break;
			case CRC_MBC:
				if (GetCRC(ae->wl[ae->idx+2].w)==CRC_A)  {
					___output(ae,0x02);
					ae->nop+=2;ae->tick+=7;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LD (BC),A only\n");
				}
				break;
			case CRC_MDE:
				if (GetCRC(ae->wl[ae->idx+2].w)==CRC_A)  {
					___output(ae,0x12);
					ae->nop+=2;ae->tick+=7;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LD (DE),A only\n");
				}
				break;
			case CRC_HL:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_BC:___output(ae,0x60);___output(ae,0x69);ae->nop+=2;ae->tick+=8;break;
					case CRC_DE:___output(ae,0x62);___output(ae,0x6B);ae->nop+=2;ae->tick+=8;break;
					case CRC_HL:___output(ae,0x64);___output(ae,0x6D);ae->nop+=2;ae->tick+=8;break;
					default:
					if (strncmp(ae->wl[ae->idx+2].w,"(IX+",4)==0) {
						/* enhanced LD HL,(IX+nn) */
						___output(ae,0xDD);___output(ae,0x66);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV81);
						___output(ae,0xDD);___output(ae,0x6E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=10;ae->tick+=19;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY+",4)==0) {
						/* enhanced LD HL,(IY+nn) */
						___output(ae,0xFD);___output(ae,0x66);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV81);
						___output(ae,0xFD);___output(ae,0x6E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=10;ae->tick+=19;ae->tick+=19;
					} else if (StringIsMem(ae->wl[ae->idx+2].w)) {
						___output(ae,0x2A);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V16);
						ae->nop+=5;ae->tick+=16;
					} else {
						___output(ae,0x21);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V16);
						ae->nop+=3;ae->tick+=10;
					}
				}
				break;
			case CRC_BC:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_BC:___output(ae,0x40);___output(ae,0x49);ae->nop+=2;ae->tick+=8;break;
					case CRC_DE:___output(ae,0x42);___output(ae,0x4B);ae->nop+=2;ae->tick+=8;break;
					case CRC_HL:___output(ae,0x44);___output(ae,0x4D);ae->nop+=2;ae->tick+=8;break;
					/* enhanced LD BC,IX / LD BC,IY */
					case CRC_IX:___output(ae,0xDD);___output(ae,0x44);ae->nop+=4;
						    ___output(ae,0xDD);___output(ae,0x4D);ae->tick+=16;break;
					case CRC_IY:___output(ae,0xFD);___output(ae,0x44);ae->nop+=4;
						    ___output(ae,0xFD);___output(ae,0x4D);ae->tick+=16;break;
					default:
					if (strncmp(ae->wl[ae->idx+2].w,"(IX+",4)==0) {
						/* enhanced LD BC,(IX+nn) */
						___output(ae,0xDD);___output(ae,0x46);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV81);
						___output(ae,0xDD);___output(ae,0x4E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=10;ae->tick+=19;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY+",4)==0) {
						/* enhanced LD BC,(IY+nn) */
						___output(ae,0xFD);___output(ae,0x46);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV81);
						___output(ae,0xFD);___output(ae,0x4E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=10;ae->tick+=19;ae->tick+=19;
					} else if (StringIsMem(ae->wl[ae->idx+2].w)) {
						___output(ae,0xED);___output(ae,0x4B);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV16);
						ae->nop+=6;ae->tick+=20;
					} else {
						___output(ae,0x01);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V16);
						ae->nop+=3;ae->tick+=10;
					}
				}
				break;
			case CRC_DE:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_BC:___output(ae,0x50);___output(ae,0x59);ae->nop+=2;ae->tick+=8;break;
					case CRC_DE:___output(ae,0x52);___output(ae,0x5B);ae->nop+=2;ae->tick+=8;break;
					case CRC_HL:___output(ae,0x54);___output(ae,0x5D);ae->nop+=2;ae->tick+=8;break;
					/* enhanced LD DE,IX / LD DE,IY */
					case CRC_IX:___output(ae,0xDD);___output(ae,0x54);ae->nop+=4;
						    ___output(ae,0xDD);___output(ae,0x5D);ae->tick+=16;break;
					case CRC_IY:___output(ae,0xFD);___output(ae,0x54);ae->nop+=4;
						    ___output(ae,0xFD);___output(ae,0x5D);ae->tick+=16;break;
					default:
					if (strncmp(ae->wl[ae->idx+2].w,"(IX+",4)==0) {
						/* enhanced LD DE,(IX+nn) */
						___output(ae,0xDD);___output(ae,0x56);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV81);
						___output(ae,0xDD);___output(ae,0x5E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=10;ae->tick+=19;ae->tick+=19;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY+",4)==0) {
						/* enhanced LD DE,(IY+nn) */
						___output(ae,0xFD);___output(ae,0x56);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV81);
						___output(ae,0xFD);___output(ae,0x5E);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						ae->nop+=10;ae->tick+=19;ae->tick+=19;
					} else if (StringIsMem(ae->wl[ae->idx+2].w)) {
						___output(ae,0xED);___output(ae,0x5B);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV16);
						ae->nop+=6;ae->tick+=20;
					} else {
						___output(ae,0x11);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_V16);
						ae->nop+=3;ae->tick+=10;
					}
				}
				break;
			case CRC_IX:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					/* enhanced LD IX,BC / LD IX,DE */
					case CRC_BC:___output(ae,0xDD);___output(ae,0x60);
						    ___output(ae,0xDD);___output(ae,0x69);ae->nop+=4;ae->tick+=16;break;
					case CRC_DE:___output(ae,0xDD);___output(ae,0x62);
						    ___output(ae,0xDD);___output(ae,0x6B);ae->nop+=4;ae->tick+=16;break;
					default:
				if (StringIsMem(ae->wl[ae->idx+2].w)) {
					___output(ae,0xDD);___output(ae,0x2A);
					PushExpression(ae,ae->idx+2,E_EXPRESSION_IV16);
					ae->nop+=6;ae->tick+=20;
				} else {
					___output(ae,0xDD);___output(ae,0x21);
					PushExpression(ae,ae->idx+2,E_EXPRESSION_IV16);
					ae->nop+=4;ae->tick+=14;
				}
				}
				break;
			case CRC_IY:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					/* enhanced LD IY,BC / LD IY,DE */
					case CRC_BC:___output(ae,0xFD);___output(ae,0x60);
						    ___output(ae,0xFD);___output(ae,0x69);ae->nop+=4;ae->tick+=16;break;
					case CRC_DE:___output(ae,0xFD);___output(ae,0x62);
						    ___output(ae,0xFD);___output(ae,0x6B);ae->nop+=4;ae->tick+=16;break;
					default:
				if (StringIsMem(ae->wl[ae->idx+2].w)) {
					___output(ae,0xFD);___output(ae,0x2A);
					PushExpression(ae,ae->idx+2,E_EXPRESSION_IV16);
					ae->nop+=6;ae->tick+=20;
				} else {
					___output(ae,0xFD);___output(ae,0x21);
					PushExpression(ae,ae->idx+2,E_EXPRESSION_IV16);
					ae->nop+=4;ae->tick+=14;
				}
				}
				break;
			case CRC_SP:
				switch (GetCRC(ae->wl[ae->idx+2].w)) {
					case CRC_HL:___output(ae,0xF9);ae->nop+=2;ae->tick+=6;break;
					case CRC_IX:___output(ae,0xDD);___output(ae,0xF9);ae->nop+=3;ae->tick+=10;break;
					case CRC_IY:___output(ae,0xFD);___output(ae,0xF9);ae->nop+=3;ae->tick+=10;break;
					default:
						if (StringIsMem(ae->wl[ae->idx+2].w)) {
							___output(ae,0xED);___output(ae,0x7B);
							PushExpression(ae,ae->idx+2,E_EXPRESSION_IV16);
							ae->nop+=6;ae->tick+=20;
						} else {
							___output(ae,0x31);
							PushExpression(ae,ae->idx+2,E_EXPRESSION_V16);
							ae->nop+=3;ae->tick+=10;
						}
				}
				break;
			default:
				/* (ix+expression) (iy+expression) (expression) expression */
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					switch (GetCRC(ae->wl[ae->idx+2].w)) {
						case CRC_B:___output(ae,0xDD);___output(ae,0x70);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_C:___output(ae,0xDD);___output(ae,0x71);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_D:___output(ae,0xDD);___output(ae,0x72);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_E:___output(ae,0xDD);___output(ae,0x73);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_H:___output(ae,0xDD);___output(ae,0x74);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_L:___output(ae,0xDD);___output(ae,0x75);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_A:___output(ae,0xDD);___output(ae,0x77);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_HL:___output(ae,0xDD);___output(ae,0x74);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV81);___output(ae,0xDD);___output(ae,0x75);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=10;ae->tick+=38;break;
						case CRC_DE:___output(ae,0xDD);___output(ae,0x72);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV81);___output(ae,0xDD);___output(ae,0x73);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=10;ae->tick+=38;break;
						case CRC_BC:___output(ae,0xDD);___output(ae,0x70);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV81);___output(ae,0xDD);___output(ae,0x71);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=10;ae->tick+=38;break;
						default:___output(ae,0xDD);___output(ae,0x36);
							PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
							PushExpression(ae,ae->idx+2,E_EXPRESSION_3V8);
							ae->nop+=6;ae->tick+=23;
					}
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					switch (GetCRC(ae->wl[ae->idx+2].w)) {
						case CRC_B:___output(ae,0xFD);___output(ae,0x70);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_C:___output(ae,0xFD);___output(ae,0x71);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_D:___output(ae,0xFD);___output(ae,0x72);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_E:___output(ae,0xFD);___output(ae,0x73);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_H:___output(ae,0xFD);___output(ae,0x74);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_L:___output(ae,0xFD);___output(ae,0x75);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_A:___output(ae,0xFD);___output(ae,0x77);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=5;ae->tick+=19;break;
						case CRC_HL:___output(ae,0xFD);___output(ae,0x74);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV81);___output(ae,0xFD);___output(ae,0x75);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=10;ae->tick+=38;break;
						case CRC_DE:___output(ae,0xFD);___output(ae,0x72);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV81);___output(ae,0xFD);___output(ae,0x73);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=10;ae->tick+=38;break;
						case CRC_BC:___output(ae,0xFD);___output(ae,0x70);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV81);___output(ae,0xFD);___output(ae,0x71);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);ae->nop+=10;ae->tick+=38;break;
						default:___output(ae,0xFD);___output(ae,0x36);
							PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
							PushExpression(ae,ae->idx+2,E_EXPRESSION_3V8);
							ae->nop+=6;ae->tick+=23;
					}
				} else if (StringIsMem(ae->wl[ae->idx+1].w)) {
					switch (GetCRC(ae->wl[ae->idx+2].w)) {
						case CRC_A:___output(ae,0x32);PushExpression(ae,ae->idx+1,E_EXPRESSION_V16);ae->nop+=4;ae->tick+=13;break;
						case CRC_BC:___output(ae,0xED);___output(ae,0x43);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV16);ae->nop+=6;ae->tick+=20;break;
						case CRC_DE:___output(ae,0xED);___output(ae,0x53);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV16);ae->nop+=6;ae->tick+=20;break;
						case CRC_HL:___output(ae,0x22);PushExpression(ae,ae->idx+1,E_EXPRESSION_V16);ae->nop+=5;ae->tick+=16;break;
						case CRC_IX:___output(ae,0xDD);___output(ae,0x22);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV16);ae->nop+=6;ae->tick+=20;break;
						case CRC_IY:___output(ae,0xFD);___output(ae,0x22);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV16);ae->nop+=6;ae->tick+=20;break;
						case CRC_SP:___output(ae,0xED);___output(ae,0x73);PushExpression(ae,ae->idx+1,E_EXPRESSION_IV16);ae->nop+=6;ae->tick+=20;break;
						default:
							MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LD (#nnnn),[A,BC,DE,HL,SP,IX,IY] only\n");
					}
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Unknown LD format\n");
				}
				break;
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LD needs two parameters\n");
	}
}


void _RLC(struct s_assenv *ae) {
	/* on check qu'il y a un ou deux parametres */
	if (ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_B:___output(ae,0xCB);___output(ae,0x0);ae->nop+=2;ae->tick+=8;break;
			case CRC_C:___output(ae,0xCB);___output(ae,0x1);ae->nop+=2;ae->tick+=8;break;
			case CRC_D:___output(ae,0xCB);___output(ae,0x2);ae->nop+=2;ae->tick+=8;break;
			case CRC_E:___output(ae,0xCB);___output(ae,0x3);ae->nop+=2;ae->tick+=8;break;
			case CRC_H:___output(ae,0xCB);___output(ae,0x4);ae->nop+=2;ae->tick+=8;break;
			case CRC_L:___output(ae,0xCB);___output(ae,0x5);ae->nop+=2;ae->tick+=8;break;
			case CRC_MHL:___output(ae,0xCB);___output(ae,0x6);ae->nop+=4;ae->tick+=15;break;
			case CRC_A:___output(ae,0xCB);___output(ae,0x7);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x6);
					ae->nop+=7;ae->tick+=23;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x6);
					ae->nop+=7;ae->tick+=23;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RLC reg8/(HL)/(IX+n)/(IY+n)\n");
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t!=2) {
		if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
			___output(ae,0xDD);
		} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
			___output(ae,0xFD);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RLC (IX+n),reg8\n");
		}
		___output(ae,0xCB);
		switch (GetCRC(ae->wl[ae->idx+2].w)) {
			case CRC_B:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x0);ae->nop+=7;ae->tick+=23;break;
			case CRC_C:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x1);ae->nop+=7;ae->tick+=23;break;
			case CRC_D:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x2);ae->nop+=7;ae->tick+=23;break;
			case CRC_E:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x3);ae->nop+=7;ae->tick+=23;break;
			case CRC_H:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x4);ae->nop+=7;ae->tick+=23;break;
			case CRC_L:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x5);ae->nop+=7;ae->tick+=23;break;
			case CRC_A:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x7);ae->nop+=7;ae->tick+=23;break;
			default:			
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RLC (IX+n),reg8\n");
		}
		ae->idx++;
		ae->idx++;
	}
}

void _RRC(struct s_assenv *ae) {
	/* on check qu'il y a un ou deux parametres */
	if (ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_B:___output(ae,0xCB);___output(ae,0x8);ae->nop+=2;ae->tick+=8;break;
			case CRC_C:___output(ae,0xCB);___output(ae,0x9);ae->nop+=2;ae->tick+=8;break;
			case CRC_D:___output(ae,0xCB);___output(ae,0xA);ae->nop+=2;ae->tick+=8;break;
			case CRC_E:___output(ae,0xCB);___output(ae,0xB);ae->nop+=2;ae->tick+=8;break;
			case CRC_H:___output(ae,0xCB);___output(ae,0xC);ae->nop+=2;ae->tick+=8;break;
			case CRC_L:___output(ae,0xCB);___output(ae,0xD);ae->nop+=2;ae->tick+=8;break;
			case CRC_MHL:___output(ae,0xCB);___output(ae,0xE);ae->nop+=4;ae->tick+=15;break;
			case CRC_A:___output(ae,0xCB);___output(ae,0xF);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0xE);
					ae->nop+=7;ae->tick+=23;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0xE);ae->tick+=23;
					ae->nop+=7;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RRC reg8/(HL)/(IX+n)/(IY+n)\n");
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t!=2) {
		if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
			___output(ae,0xDD);
		} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
			___output(ae,0xFD);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RRC (IX+n),reg8\n");
		}
		___output(ae,0xCB);
		switch (GetCRC(ae->wl[ae->idx+2].w)) {
			case CRC_B:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x8);ae->nop+=7;ae->tick+=23;break;
			case CRC_C:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x9);ae->nop+=7;ae->tick+=23;break;
			case CRC_D:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0xA);ae->nop+=7;ae->tick+=23;break;
			case CRC_E:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0xB);ae->nop+=7;ae->tick+=23;break;
			case CRC_H:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0xC);ae->nop+=7;ae->tick+=23;break;
			case CRC_L:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0xD);ae->nop+=7;ae->tick+=23;break;
			case CRC_A:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0xF);ae->nop+=7;ae->tick+=23;break;
			default:			
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RRC (IX+n),reg8\n");
		}
		ae->idx++;
		ae->idx++;
	}
}


void _RL(struct s_assenv *ae) {
	/* on check qu'il y a un ou deux parametres */
	if (ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_BC:___output(ae,0xCB);___output(ae,0x10);___output(ae,0xCB);___output(ae,0x11);ae->nop+=4;ae->tick+=16;break;
			case CRC_B:___output(ae,0xCB);___output(ae,0x10);ae->nop+=2;ae->tick+=8;break;
			case CRC_C:___output(ae,0xCB);___output(ae,0x11);ae->nop+=2;ae->tick+=8;break;
			case CRC_DE:___output(ae,0xCB);___output(ae,0x12);___output(ae,0xCB);___output(ae,0x13);ae->nop+=4;ae->tick+=16;break;
			case CRC_D:___output(ae,0xCB);___output(ae,0x12);ae->nop+=2;ae->tick+=8;break;
			case CRC_E:___output(ae,0xCB);___output(ae,0x13);ae->nop+=2;ae->tick+=8;break;
			case CRC_HL:___output(ae,0xCB);___output(ae,0x14);___output(ae,0xCB);___output(ae,0x15);ae->nop+=4;ae->tick+=16;break;
			case CRC_H:___output(ae,0xCB);___output(ae,0x14);ae->nop+=2;ae->tick+=8;break;
			case CRC_L:___output(ae,0xCB);___output(ae,0x15);ae->nop+=2;ae->tick+=8;break;
			case CRC_MHL:___output(ae,0xCB);___output(ae,0x16);ae->nop+=4;ae->tick+=15;break;
			case CRC_A:___output(ae,0xCB);___output(ae,0x17);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x16);
					ae->nop+=7;ae->tick+=23;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x16);
					ae->nop+=7;ae->tick+=23;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RL reg8/(HL)/(IX+n)/(IY+n)\n");
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t!=2) {
		if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
			___output(ae,0xDD);
		} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
			___output(ae,0xFD);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RL (IX+n),reg8\n");
		}
		___output(ae,0xCB);
		switch (GetCRC(ae->wl[ae->idx+2].w)) {
			case CRC_B:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x10);ae->nop+=7;ae->tick+=23;break;
			case CRC_C:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x11);ae->nop+=7;ae->tick+=23;break;
			case CRC_D:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x12);ae->nop+=7;ae->tick+=23;break;
			case CRC_E:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x13);ae->nop+=7;ae->tick+=23;break;
			case CRC_H:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x14);ae->nop+=7;ae->tick+=23;break;
			case CRC_L:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x15);ae->nop+=7;ae->tick+=23;break;
			case CRC_A:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x17);ae->nop+=7;ae->tick+=23;break;
			default:			
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RL (IX+n),reg8\n");
		}
		ae->idx++;
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RL (IX+n),reg8 or RL reg8/(HL)/(IX+n)/(IY+n)\n");
	}
}

void _RR(struct s_assenv *ae) {
	/* on check qu'il y a un ou deux parametres */
	if (ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_BC:___output(ae,0xCB);___output(ae,0x18);___output(ae,0xCB);___output(ae,0x19);ae->nop+=4;ae->tick+=16;break;
			case CRC_B:___output(ae,0xCB);___output(ae,0x18);ae->nop+=2;ae->tick+=8;break;
			case CRC_C:___output(ae,0xCB);___output(ae,0x19);ae->nop+=2;ae->tick+=8;break;
			case CRC_DE:___output(ae,0xCB);___output(ae,0x1A);___output(ae,0xCB);___output(ae,0x1B);ae->nop+=4;ae->tick+=16;break;
			case CRC_D:___output(ae,0xCB);___output(ae,0x1A);ae->nop+=2;ae->tick+=8;break;
			case CRC_E:___output(ae,0xCB);___output(ae,0x1B);ae->nop+=2;ae->tick+=8;break;
			case CRC_HL:___output(ae,0xCB);___output(ae,0x1C);___output(ae,0xCB);___output(ae,0x1D);ae->nop+=4;ae->tick+=16;break;
			case CRC_H:___output(ae,0xCB);___output(ae,0x1C);ae->nop+=2;ae->tick+=8;break;
			case CRC_L:___output(ae,0xCB);___output(ae,0x1D);ae->nop+=2;ae->tick+=8;break;
			case CRC_MHL:___output(ae,0xCB);___output(ae,0x1E);ae->nop+=4;ae->tick+=15;break;
			case CRC_A:___output(ae,0xCB);___output(ae,0x1F);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x1E);
					ae->nop+=7;ae->tick+=23;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x1E);
					ae->nop+=7;ae->tick+=23;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RR reg8/(HL)/(IX+n)/(IY+n)\n");
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t!=2) {
		if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
			___output(ae,0xDD);
		} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
			___output(ae,0xFD);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RR (IX+n),reg8\n");
		}
		___output(ae,0xCB);
		switch (GetCRC(ae->wl[ae->idx+2].w)) {
			case CRC_B:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x18);ae->nop+=7;ae->tick+=23;break;
			case CRC_C:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x19);ae->nop+=7;ae->tick+=23;break;
			case CRC_D:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x1A);ae->nop+=7;ae->tick+=23;break;
			case CRC_E:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x1B);ae->nop+=7;ae->tick+=23;break;
			case CRC_H:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x1C);ae->nop+=7;ae->tick+=23;break;
			case CRC_L:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x1D);ae->nop+=7;ae->tick+=23;break;
			case CRC_A:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x1F);ae->nop+=7;ae->tick+=23;break;
			default:			
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RR (IX+n),reg8\n");
		}
		ae->idx++;
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RR (IX+n),reg8 or RR reg8/(HL)/(IX+n)/(IY+n)\n");
	}
}

void _SLA(struct s_assenv *ae) {
	/* on check qu'il y a un ou deux parametres */
	if (ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_BC:___output(ae,0xCB);___output(ae,0x21);___output(ae,0xCB);___output(ae,0x10);ae->nop+=4;ae->tick+=16;break; /* SLA C : RL B */
			case CRC_B:___output(ae,0xCB);___output(ae,0x20);ae->nop+=2;ae->tick+=8;break;
			case CRC_C:___output(ae,0xCB);___output(ae,0x21);ae->nop+=2;ae->tick+=8;break;
			case CRC_DE:___output(ae,0xCB);___output(ae,0x23);___output(ae,0xCB);___output(ae,0x12);ae->nop+=4;ae->tick+=16;break; /* SLA E : RL D */
			case CRC_D:___output(ae,0xCB);___output(ae,0x22);ae->nop+=2;ae->tick+=8;break;
			case CRC_E:___output(ae,0xCB);___output(ae,0x23);ae->nop+=2;ae->tick+=8;break;
			case CRC_HL:___output(ae,0xCB);___output(ae,0x25);___output(ae,0xCB);___output(ae,0x14);ae->nop+=4;ae->tick+=16;break; /* SLA L : RL H */
			case CRC_H:___output(ae,0xCB);___output(ae,0x24);ae->nop+=2;ae->tick+=8;break;
			case CRC_L:___output(ae,0xCB);___output(ae,0x25);ae->nop+=2;ae->tick+=8;break;
			case CRC_MHL:___output(ae,0xCB);___output(ae,0x26);ae->nop+=4;ae->tick+=15;break;
			case CRC_A:___output(ae,0xCB);___output(ae,0x27);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x26);
					ae->nop+=7;ae->tick+=23;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x26);
					ae->nop+=7;ae->tick+=23;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SLA reg8/(HL)/(IX+n)/(IY+n)\n");
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t!=2) {
		if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
			___output(ae,0xDD);
		} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
			___output(ae,0xFD);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SLL (IX+n),reg8\n");
		}
		___output(ae,0xCB);
		switch (GetCRC(ae->wl[ae->idx+2].w)) {
			case CRC_B:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x20);ae->nop+=7;ae->tick+=23;break;
			case CRC_C:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x21);ae->nop+=7;ae->tick+=23;break;
			case CRC_D:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x22);ae->nop+=7;ae->tick+=23;break;
			case CRC_E:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x23);ae->nop+=7;ae->tick+=23;break;
			case CRC_H:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x24);ae->nop+=7;ae->tick+=23;break;
			case CRC_L:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x25);ae->nop+=7;ae->tick+=23;break;
			case CRC_A:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x27);ae->nop+=7;ae->tick+=23;break;
			default:			
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SLA (IX+n),reg8\n");
		}
		ae->idx++;
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SLA reg8/(HL)/(IX+n)/(IY+n) or SLA (IX+n),reg8\n");
	}
}

void _SRA(struct s_assenv *ae) {
	/* on check qu'il y a un ou deux parametres */
	if (ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_BC:___output(ae,0xCB);___output(ae,0x28);___output(ae,0xCB);___output(ae,0x19);ae->nop+=4;ae->tick+=16;break; /* SRA B : RR C */
			case CRC_B:___output(ae,0xCB);___output(ae,0x28);ae->nop+=2;ae->tick+=8;break;
			case CRC_C:___output(ae,0xCB);___output(ae,0x29);ae->nop+=2;ae->tick+=8;break;
			case CRC_DE:___output(ae,0xCB);___output(ae,0x2A);___output(ae,0xCB);___output(ae,0x1B);ae->nop+=4;ae->tick+=16;break; /* SRA D : RR E */
			case CRC_D:___output(ae,0xCB);___output(ae,0x2A);ae->nop+=2;ae->tick+=8;break;
			case CRC_E:___output(ae,0xCB);___output(ae,0x2B);ae->nop+=2;ae->tick+=8;break;
			case CRC_HL:___output(ae,0xCB);___output(ae,0x2C);___output(ae,0xCB);___output(ae,0x1D);ae->nop+=4;ae->tick+=16;break; /* SRA H : RR L */
			case CRC_H:___output(ae,0xCB);___output(ae,0x2C);ae->nop+=2;ae->tick+=8;break;
			case CRC_L:___output(ae,0xCB);___output(ae,0x2D);ae->nop+=2;ae->tick+=8;break;
			case CRC_MHL:___output(ae,0xCB);___output(ae,0x2E);ae->nop+=4;ae->tick+=15;break;
			case CRC_A:___output(ae,0xCB);___output(ae,0x2F);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x2E);
					ae->nop+=7;ae->tick+=23;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x2E);
					ae->nop+=7;ae->tick+=23;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SRA reg8/(HL)/(IX+n)/(IY+n)\n");
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t!=2) {
		if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
			___output(ae,0xDD);
		} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
			___output(ae,0xFD);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SRA (IX+n),reg8\n");
		}
		___output(ae,0xCB);
		switch (GetCRC(ae->wl[ae->idx+2].w)) {
			case CRC_B:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x28);ae->nop+=7;ae->tick+=23;break;
			case CRC_C:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x29);ae->nop+=7;ae->tick+=23;break;
			case CRC_D:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x2A);ae->nop+=7;ae->tick+=23;break;
			case CRC_E:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x2B);ae->nop+=7;ae->tick+=23;break;
			case CRC_H:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x2C);ae->nop+=7;ae->tick+=23;break;
			case CRC_L:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x2D);ae->nop+=7;ae->tick+=23;break;
			case CRC_A:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x2F);ae->nop+=7;ae->tick+=23;break;
			default:			
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SRA (IX+n),reg8\n");
		}
		ae->idx++;
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SRA reg8/(HL)/(IX+n)/(IY+n) or SRA (IX+n),reg8\n");
	}
}


void _SLL(struct s_assenv *ae) {
	/* on check qu'il y a un ou deux parametres */
	if (ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_BC:___output(ae,0xCB);___output(ae,0x31);___output(ae,0xCB);___output(ae,0x10);ae->nop+=4;ae->tick+=16;break; /* SLL C : RL B */
			case CRC_B:___output(ae,0xCB);___output(ae,0x30);ae->nop+=2;ae->tick+=8;break;
			case CRC_C:___output(ae,0xCB);___output(ae,0x31);ae->nop+=2;ae->tick+=8;break;
			case CRC_DE:___output(ae,0xCB);___output(ae,0x33);___output(ae,0xCB);___output(ae,0x12);ae->nop+=4;ae->tick+=16;break; /* SLL E : RL D */
			case CRC_D:___output(ae,0xCB);___output(ae,0x32);ae->nop+=2;ae->tick+=8;break;
			case CRC_E:___output(ae,0xCB);___output(ae,0x33);ae->nop+=2;ae->tick+=8;break;
			case CRC_HL:___output(ae,0xCB);___output(ae,0x35);___output(ae,0xCB);___output(ae,0x14);ae->nop+=4;ae->tick+=16;break; /* SLL L : RL H */
			case CRC_H:___output(ae,0xCB);___output(ae,0x34);ae->nop+=2;ae->tick+=8;break;
			case CRC_L:___output(ae,0xCB);___output(ae,0x35);ae->nop+=2;ae->tick+=8;break;
			case CRC_MHL:___output(ae,0xCB);___output(ae,0x36);ae->nop+=4;ae->tick+=15;break;
			case CRC_A:___output(ae,0xCB);___output(ae,0x37);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x36);
					ae->nop+=7;ae->tick+=23;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x36);
					ae->nop+=7;ae->tick+=23;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SLL reg8/(HL)/(IX+n)/(IY+n)\n");
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t!=2) {
		if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
			___output(ae,0xDD);
		} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
			___output(ae,0xFD);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SLL (IX+n),reg8\n");
		}
		___output(ae,0xCB);
		switch (GetCRC(ae->wl[ae->idx+2].w)) {
			case CRC_B:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x30);ae->nop+=7;ae->tick+=23;break;
			case CRC_C:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x31);ae->nop+=7;ae->tick+=23;break;
			case CRC_D:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x32);ae->nop+=7;ae->tick+=23;break;
			case CRC_E:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x33);ae->nop+=7;ae->tick+=23;break;
			case CRC_H:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x34);ae->nop+=7;ae->tick+=23;break;
			case CRC_L:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x35);ae->nop+=7;ae->tick+=23;break;
			case CRC_A:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x37);ae->nop+=7;ae->tick+=23;break;
			default:			
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SLL (IX+n),reg8\n");
		}
		ae->idx++;
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SLL reg8/(HL)/(IX+n)/(IY+n) or SLL (IX+n),reg8\n");
	}
}

void _SRL(struct s_assenv *ae) {
	/* on check qu'il y a un ou deux parametres */
	if (ae->wl[ae->idx+1].t==1) {
		switch (GetCRC(ae->wl[ae->idx+1].w)) {
			case CRC_BC:___output(ae,0xCB);___output(ae,0x38);___output(ae,0xCB);___output(ae,0x19);ae->nop+=4;ae->tick+=16;break; /* SRL B : RR C */
			case CRC_B:___output(ae,0xCB);___output(ae,0x38);ae->nop+=2;ae->tick+=8;break;
			case CRC_C:___output(ae,0xCB);___output(ae,0x39);ae->nop+=2;ae->tick+=8;break;
			case CRC_DE:___output(ae,0xCB);___output(ae,0x3A);___output(ae,0xCB);___output(ae,0x1B);ae->nop+=4;ae->tick+=16;break; /* SRL D : RR E */
			case CRC_D:___output(ae,0xCB);___output(ae,0x3A);ae->nop+=2;ae->tick+=8;break;
			case CRC_E:___output(ae,0xCB);___output(ae,0x3B);ae->nop+=2;ae->tick+=8;break;
			case CRC_HL:___output(ae,0xCB);___output(ae,0x3C);___output(ae,0xCB);___output(ae,0x1D);ae->nop+=4;ae->tick+=16;break; /* SRL H : RR L */
			case CRC_H:___output(ae,0xCB);___output(ae,0x3C);ae->nop+=2;ae->tick+=8;break;
			case CRC_L:___output(ae,0xCB);___output(ae,0x3D);ae->nop+=2;ae->tick+=8;break;
			case CRC_MHL:___output(ae,0xCB);___output(ae,0x3E);ae->nop+=4;ae->tick+=15;break;
			case CRC_A:___output(ae,0xCB);___output(ae,0x3F);ae->nop+=2;ae->tick+=8;break;
			default:
				if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
					___output(ae,0xDD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x3E);
					ae->nop+=7;ae->tick+=23;
				} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
					___output(ae,0xFD);___output(ae,0xCB);
					PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);
					___output(ae,0x3E);
					ae->nop+=7;ae->tick+=23;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SRL reg8/(HL)/(IX+n)/(IY+n)\n");
				}
		}
		ae->idx++;
	} else if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t!=2) {
		if (strncmp(ae->wl[ae->idx+1].w,"(IX",3)==0) {
			___output(ae,0xDD);
		} else if (strncmp(ae->wl[ae->idx+1].w,"(IY",3)==0) {
			___output(ae,0xFD);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SRL (IX+n),reg8\n");
		}
		___output(ae,0xCB);
		switch (GetCRC(ae->wl[ae->idx+2].w)) {
			case CRC_B:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x38);ae->nop+=7;ae->tick+=23;break;
			case CRC_C:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x39);ae->nop+=7;ae->tick+=23;break;
			case CRC_D:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x3A);ae->nop+=7;ae->tick+=23;break;
			case CRC_E:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x3B);ae->nop+=7;ae->tick+=23;break;
			case CRC_H:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x3C);ae->nop+=7;ae->tick+=23;break;
			case CRC_L:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x3D);ae->nop+=7;ae->tick+=23;break;
			case CRC_A:PushExpression(ae,ae->idx+1,E_EXPRESSION_IV8);___output(ae,0x3F);ae->nop+=7;ae->tick+=23;break;
			default:			
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SRL (IX+n),reg8\n");
		}
		ae->idx++;
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SRL reg8/(HL)/(IX+n)/(IY+n) or SRL (IX+n),reg8\n");
	}
}


void _BIT(struct s_assenv *ae) {
	int o;
	/* on check qu'il y a deux ou trois parametres 
	ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
	o=RoundComputeExpressionCore(ae,ae->wl[ae->idx+1].w,ae->codeadr,0);*/

	o=0;
	if (o<0 || o>7) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is BIT <value from 0 to 7>,... (%d)\n",o);
	} else {
		o=0x40+o*8;
		if (ae->wl[ae->idx+1].t==0 && ae->wl[ae->idx+2].t==1) {
			switch (GetCRC(ae->wl[ae->idx+2].w)) {
				case CRC_B:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x0+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_C:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x1+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_D:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x2+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_E:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x3+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_H:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x4+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_L:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x5+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_MHL:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x6+o);ae->nop+=3;ae->tick+=12;break;
				case CRC_A:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x7+o);ae->nop+=2;ae->tick+=8;break;
				default:
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0xCB);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x6+o);
						ae->nop+=6;ae->tick+=20;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0xCB);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x6+o);
						ae->nop+=6;ae->tick+=20;
					} else {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is BIT n,reg8/(HL)/(IX+n)/(IY+n)\n");
					}
			}
			ae->idx+=2;
		} else if (!ae->wl[ae->idx+1].t && !ae->wl[ae->idx+2].t && ae->wl[ae->idx+3].t==1) {
			if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
				___output(ae,0xDD);
			} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
				___output(ae,0xFD);
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is BIT (IX+n),reg8\n");
			}
			___output(ae,0xCB);
			switch (GetCRC(ae->wl[ae->idx+3].w)) {
				case CRC_B:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x0+o);ae->nop+=6;ae->tick+=20;break;
				case CRC_C:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x1+o);ae->nop+=6;ae->tick+=20;break;
				case CRC_D:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x2+o);ae->nop+=6;ae->tick+=20;break;
				case CRC_E:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x3+o);ae->nop+=6;ae->tick+=20;break;
				case CRC_H:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x4+o);ae->nop+=6;ae->tick+=20;break;
				case CRC_L:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x5+o);ae->nop+=6;ae->tick+=20;break;
				case CRC_A:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x7+o);ae->nop+=6;ae->tick+=20;break;
				default:			
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is BIT n,(IX+n),reg8\n");
			}
			ae->idx+=3;
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is BIT n,reg8/(HL)/(IX+n)[,reg8]/(IY+n)[,reg8]\n");
		}
	}
}

void _RES(struct s_assenv *ae) {
	int o;
	/* on check qu'il y a deux ou trois parametres 
	ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
	o=RoundComputeExpressionCore(ae,ae->wl[ae->idx+1].w,ae->codeadr,0); */
	o=0;
	if (o<0 || o>7) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RES <value from 0 to 7>,... (%d)\n",o);
	} else {
		o=0x80+o*8;
		if (ae->wl[ae->idx+1].t==0 && ae->wl[ae->idx+2].t==1) {
			switch (GetCRC(ae->wl[ae->idx+2].w)) {
				case CRC_B:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x0+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_C:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x1+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_D:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x2+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_E:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x3+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_H:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x4+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_L:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x5+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_MHL:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x6+o);ae->nop+=4;ae->tick+=15;break;
				case CRC_A:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x7+o);ae->nop+=2;ae->tick+=8;break;
				default:
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0xCB);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x6+o);
						ae->nop+=7;ae->tick+=23;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0xCB);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x6+o);
						ae->nop+=7;ae->tick+=23;
					} else {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RES n,reg8/(HL)/(IX+n)/(IY+n)\n");
					}
			}
			ae->idx+=2;
		} else if (!ae->wl[ae->idx+1].t && !ae->wl[ae->idx+2].t && ae->wl[ae->idx+3].t==1) {
			if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
				___output(ae,0xDD);
			} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
				___output(ae,0xFD);
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RES n,(IX+n),reg8\n");
			}
			___output(ae,0xCB);
			switch (GetCRC(ae->wl[ae->idx+3].w)) {
				case CRC_B:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x0+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_C:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x1+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_D:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x2+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_E:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x3+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_H:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x4+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_L:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x5+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_A:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x7+o);ae->nop+=7;ae->tick+=23;break;
				default:			
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RES n,(IX+n),reg8\n");
			}
			ae->idx+=3;
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is RES n,reg8/(HL)/(IX+n)[,reg8]/(IY+n)[,reg8]\n");
		}
	}
}

void _SET(struct s_assenv *ae) {
	int o;
	/* on check qu'il y a deux ou trois parametres 
	ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
	o=RoundComputeExpressionCore(ae,ae->wl[ae->idx+1].w,ae->codeadr,0); */
	o=0;
	if (o<0 || o>7) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SET <value from 0 to 7>,... (%d)\n",o);
	} else {
		o=0xC0+o*8;
		if (ae->wl[ae->idx+1].t==0 && ae->wl[ae->idx+2].t==1) {
			switch (GetCRC(ae->wl[ae->idx+2].w)) {
				case CRC_B:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x0+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_C:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x1+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_D:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x2+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_E:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x3+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_H:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x4+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_L:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x5+o);ae->nop+=2;ae->tick+=8;break;
				case CRC_MHL:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x6+o);ae->nop+=4;ae->tick+=15;break;
				case CRC_A:___output(ae,0xCB);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x7+o);ae->nop+=2;ae->tick+=8;break;
				default:
					if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
						___output(ae,0xDD);___output(ae,0xCB);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x6+o);
						ae->nop+=7;ae->tick+=23;
					} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
						___output(ae,0xFD);___output(ae,0xCB);
						PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);
						PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x6+o);
						ae->nop+=7;ae->tick+=23;
					} else {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SET n,reg8/(HL)/(IX+n)/(IY+n)\n");
					}
			}
			ae->idx+=2;
		} else if (!ae->wl[ae->idx+1].t && !ae->wl[ae->idx+2].t && ae->wl[ae->idx+3].t==1) {
			if (strncmp(ae->wl[ae->idx+2].w,"(IX",3)==0) {
				___output(ae,0xDD);
			} else if (strncmp(ae->wl[ae->idx+2].w,"(IY",3)==0) {
				___output(ae,0xFD);
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SET n,(IX+n),reg8\n");
			}
			___output(ae,0xCB);
			switch (GetCRC(ae->wl[ae->idx+3].w)) {
				case CRC_B:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x0+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_C:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x1+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_D:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x2+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_E:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x3+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_H:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x4+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_L:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x5+o);ae->nop+=7;ae->tick+=23;break;
				case CRC_A:PushExpression(ae,ae->idx+2,E_EXPRESSION_IV8);PushExpression(ae,ae->idx+1,E_EXPRESSION_BRS);___output(ae,0x7+o);ae->nop+=7;ae->tick+=23;break;
				default:			
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SET n,(IX+n),reg8\n");
			}
			ae->idx+=3;
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is SET n,reg8/(HL)/(IX+n)[,reg8]/(IY+n)[,reg8]\n");
		}
	}
}

void _DEFS(struct s_assenv *ae) {
	int i,r,v;
	if (ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Syntax is DEFS repeat,value or DEFS repeat\n");
	} else do {
		ae->idx++;
		if (!ae->wl[ae->idx].t) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0); /* doing FastTranslate but not a complete evaluation */
			r=RoundComputeExpressionCore(ae,ae->wl[ae->idx].w,ae->codeadr,0);
			if (r<0) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFS size must be greater or equal to zero\n");
			}
			for (i=0;i<r;i++) {
				/* keep flexibility */
				PushExpression(ae,ae->idx+1,E_EXPRESSION_0V8);
				ae->nop+=1;
			}
			ae->idx++;
		} else if (ae->wl[ae->idx].t==1) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
			r=RoundComputeExpressionCore(ae,ae->wl[ae->idx].w,ae->codeadr,0);
			v=0;
			if (r<0) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFS size must be greater or equal to zero\n");
			}
			for (i=0;i<r;i++) {
				___output(ae,v);
				ae->nop+=1;
			}
		}
	} while (!ae->wl[ae->idx].t);
}

void _DEFS_struct(struct s_assenv *ae) {
	int i,r,v;
	if (ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Syntax is DEFS repeat,value or DEFS repeat\n");
	} else do {
		ae->idx++;
		if (!ae->wl[ae->idx].t) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
			r=RoundComputeExpressionCore(ae,ae->wl[ae->idx].w,ae->codeadr,0);
			v=RoundComputeExpressionCore(ae,ae->wl[ae->idx+1].w,ae->codeadr,0);
			if (r<0) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFS size must be greater or equal to zero\n");
			}
			for (i=0;i<r;i++) {
				___output(ae,v);
				ae->nop+=1;
			}
			ae->idx++;
		} else if (ae->wl[ae->idx].t==1) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
			r=RoundComputeExpressionCore(ae,ae->wl[ae->idx].w,ae->codeadr,0);
			v=0;
			if (r<0) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFS size must be greater or equal to zero\n");
			}
			for (i=0;i<r;i++) {
				___output(ae,v);
				ae->nop+=1;
			}
		}
	} while (!ae->wl[ae->idx].t);
}

void _STR(struct s_assenv *ae) {
	unsigned char c;
	int i,tquote;

	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			if ((tquote=StringIsQuote(ae->wl[ae->idx].w))!=0) {
				i=1;
				while (ae->wl[ae->idx].w[i] && ae->wl[ae->idx].w[i]!=tquote) {
					if (ae->wl[ae->idx].w[i]=='\\') {
						i++;
						/* no conversion on escaped chars */
						c=ae->wl[ae->idx].w[i];
						switch (c) {
							case 'b':c='\b';break;
							case 'v':c='\v';break;
							case 'f':c='\f';break;
							case '0':c='\0';break;
							case 'r':c='\r';break;
							case 'n':c='\n';break;
							case 't':c='\t';break;
							default:break;
						}						
						if (ae->wl[ae->idx].w[i+1]!=tquote) {
							___output(ae,c);
						} else {
							___output(ae,c|0x80);
						}
					} else {
						/* charset conversion on the fly */
						if (ae->wl[ae->idx].w[i+1]!=tquote) {
							___output(ae,ae->charset[(unsigned int)ae->wl[ae->idx].w[i]]);
						} else {
							___output(ae,ae->charset[(unsigned int)ae->wl[ae->idx].w[i]]|0x80);
						}
					}

					i++;
				}
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"STR handle only quoted strings!\n");
			}
		} while (ae->wl[ae->idx].t==0);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"STR needs one or more quotes parameters\n");
	}
}

/* Microsoft IEEE-754 40bits float value */
void _DEFF(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			PushExpression(ae,ae->idx,E_EXPRESSION_0VRMike);
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFF needs one or more parameters\n");
		}
	}
}
void _DEFF_struct(struct s_assenv *ae) {
	unsigned char *rc;
	double v;
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			/* conversion des symboles connus */
			ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
			/* calcul de la valeur définitive de l'expression */
			v=ComputeExpressionCore(ae,ae->wl[ae->idx].w,ae->outputadr,0);
			/* conversion en réel Amsdos */
			rc=__internal_MakeRosoftREAL(ae,v,0);
			___output(ae,rc[0]);___output(ae,rc[1]);___output(ae,rc[2]);___output(ae,rc[3]);___output(ae,rc[4]);			
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFF needs one or more parameters\n");
		}
	}
}


void _DEFR(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			PushExpression(ae,ae->idx,E_EXPRESSION_0VR);
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFR needs one or more parameters\n");
		}
	}
}
void _DEFR_struct(struct s_assenv *ae) {
	unsigned char *rc;
	double v;
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			/* conversion des symboles connus */
			ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
			/* calcul de la valeur définitive de l'expression */
			v=ComputeExpressionCore(ae,ae->wl[ae->idx].w,ae->outputadr,0);
			/* conversion en réel Amsdos */
			rc=__internal_MakeAmsdosREAL(ae,v,0);
			___output(ae,rc[0]);___output(ae,rc[1]);___output(ae,rc[2]);___output(ae,rc[3]);___output(ae,rc[4]);			
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFR needs one or more parameters\n");
		}
	}
}

void _DEFB(struct s_assenv *ae) {
	int i,tquote;
	unsigned char c;
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			if ((tquote=StringIsQuote(ae->wl[ae->idx].w))!=0) {
				i=1;
				while (ae->wl[ae->idx].w[i] && ae->wl[ae->idx].w[i]!=tquote) {
					if (ae->wl[ae->idx].w[i]=='\\') {
						i++;
						/* no conversion on escaped chars */
						c=ae->wl[ae->idx].w[i];
						switch (c) {
							case 'e':___output(ae,0x1B);break;
							case 'a':___output(ae,0x07);break; // alarm
							case 'b':___output(ae,'\b');break;
							case 'v':___output(ae,'\v');break; // v-tab
							case 'f':___output(ae,'\f');break; // feed
							case '0':___output(ae,'\0');break;
							case 'r':___output(ae,'\r');break; // return
							case 'n':___output(ae,'\n');break; // carriage-return
							case 't':___output(ae,'\t');break; // tab
							default:
							___output(ae,c);
						}						
						ae->nop+=1;
					} else {
						/* charset conversion on the fly */
						___output(ae,ae->charset[(unsigned int)ae->wl[ae->idx].w[i]]);
						ae->nop+=1;
					}
					i++;
				}
			} else {
				PushExpression(ae,ae->idx,E_EXPRESSION_0V8);
				ae->nop+=1;
			}
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFB needs one or more parameters\n");
		}
	}
}
void _DEFB_struct(struct s_assenv *ae) {
	int i,tquote;
	unsigned char c;
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			if ((tquote=StringIsQuote(ae->wl[ae->idx].w))!=0) {
				i=1;
				while (ae->wl[ae->idx].w[i] && ae->wl[ae->idx].w[i]!=tquote) {
					if (ae->wl[ae->idx].w[i]=='\\') {
						i++;
						/* no conversion on escaped chars */
						c=ae->wl[ae->idx].w[i];
						switch (c) {
							case 'b':___output(ae,'\b');break;
							case 'v':___output(ae,'\v');break;
							case 'f':___output(ae,'\f');break;
							case '0':___output(ae,'\0');break;
							case 'r':___output(ae,'\r');break;
							case 'n':___output(ae,'\n');break;
							case 't':___output(ae,'\t');break;
							default:
							___output(ae,c);
							ae->nop+=1;
						}						
					} else {
						/* charset conversion on the fly */
						___output(ae,ae->charset[(int)ae->wl[ae->idx].w[i]]);
						ae->nop+=1;
					}
					i++;
				}
			} else {
				int v;
				ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
				v=RoundComputeExpressionCore(ae,ae->wl[ae->idx].w,ae->outputadr,0);
				___output(ae,v);
				ae->nop+=1;
			}
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFB needs one or more parameters\n");
		}
	}
}

void _DEFW(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			PushExpression(ae,ae->idx,E_EXPRESSION_0V16);
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFW needs one or more parameters\n");
		}
	}
}

void _DEFW_struct(struct s_assenv *ae) {
	int v;
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
			v=RoundComputeExpressionCore(ae,ae->wl[ae->idx].w,ae->outputadr,0);
			___output(ae,v&0xFF);___output(ae,(v>>8)&0xFF);
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFW needs one or more parameters\n");
		}
	}
}

void _DEFI(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			PushExpression(ae,ae->idx,E_EXPRESSION_0V32);
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFI needs one or more parameters\n");
		}
	}
}

void _DEFI_struct(struct s_assenv *ae) {
	int v;
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
			v=RoundComputeExpressionCore(ae,ae->wl[ae->idx].w,ae->outputadr,0);
			___output(ae,v&0xFF);___output(ae,(v>>8)&0xFF);___output(ae,(v>>16)&0xFF);___output(ae,(v>>24)&0xFF);
		} while (ae->wl[ae->idx].t==0);
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFI needs one or more parameters\n");
		}
	}
}

void _DEFB_as80(struct s_assenv *ae) {
	int i,tquote;
	int modadr=0;

	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			if ((tquote=StringIsQuote(ae->wl[ae->idx].w))!=0) {
				i=1;
				while (ae->wl[ae->idx].w[i] && ae->wl[ae->idx].w[i]!=tquote) {
					if (ae->wl[ae->idx].w[i]=='\\') i++;
					/* charset conversion on the fly */
					___output(ae,ae->charset[(int)ae->wl[ae->idx].w[i]]);
					ae->nop+=1;
					ae->codeadr--;modadr++;
					i++;
				}
			} else {
				PushExpression(ae,ae->idx,E_EXPRESSION_0V8);
				ae->codeadr--;modadr++;
				ae->nop+=1;
			}
		} while (ae->wl[ae->idx].t==0);
		ae->codeadr+=modadr;
	} else {
		if (ae->getstruct) {
			___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFB needs one or more parameters\n");
		}
	}
}

void _DEFW_as80(struct s_assenv *ae) {
	int modadr=0;
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			PushExpression(ae,ae->idx,E_EXPRESSION_0V16);
			ae->codeadr-=2;modadr+=2;
		} while (ae->wl[ae->idx].t==0);
		ae->codeadr+=modadr;
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFW needs one or more parameters\n");
		}
	}
}

void _DEFI_as80(struct s_assenv *ae) {
	int modadr=0;
	if (!ae->wl[ae->idx].t) {
		do {
			ae->idx++;
			PushExpression(ae,ae->idx,E_EXPRESSION_0V32);
			ae->codeadr-=4;modadr+=4;
		} while (ae->wl[ae->idx].t==0);
		ae->codeadr+=modadr;
	} else {
		if (ae->getstruct) {
			___output(ae,0);___output(ae,0);___output(ae,0);___output(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFI needs one or more parameters\n");
		}
	}
}
#if 0
void _DEFSTR(struct s_assenv *ae) {
	int i,tquote;
	unsigned char c;
	if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		if (StringIsQuote(ae->wl[ae->idx+1].w) && StringIsQuote(ae->wl[ae->idx+2].w)) {
				i=1;
				while (ae->wl[ae->idx].w[i] && ae->wl[ae->idx].w[i]!=tquote) {
					if (ae->wl[ae->idx].w[i]=='\\') {
						i++;
						/* no conversion on escaped chars */
						c=ae->wl[ae->idx].w[i];
						switch (c) {
							case 'b':___output(ae,'\b');break;
							case 'v':___output(ae,'\v');break;
							case 'f':___output(ae,'\f');break;
							case '0':___output(ae,'\0');break;
							case 'r':___output(ae,'\r');break;
							case 'n':___output(ae,'\n');break;
							case 't':___output(ae,'\t');break;
							default:
							___output(ae,c);
						}						
					} else {
						/* charset conversion on the fly */
						___output(ae,ae->charset[(int)ae->wl[ae->idx].w[i]]);
					}
					i++;
				}
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFSTR needs two parameters\n");
	}
}
#endif

#undef FUNC
#define FUNC "Directive CORE"

void __internal_UpdateLZBlockIfAny(struct s_assenv *ae) {
	/* there was a crunched block opened in the previous bank */
	if (ae->lz>=0) {
		//ae->lzsection[ae->ilz-1].iorgzone=ae->io-1;
		//ae->lzsection[ae->ilz-1].ibank=ae->activebank;
	}
	ae->lz=-1;
}


void __AMSDOS(struct s_assenv *ae) {
	ae->amsdos=1;
}

void __internal_EXPORT(struct s_assenv *ae, int exportval) {
	struct s_label *curlabel;
	struct s_expr_dico *curdic;
	int ialias,crc,freeflag;
	char *localname;

	if (ae->wl[ae->idx].t) {
		/* without parameter enable/disable export */
		ae->autorise_export=exportval;
	} else while (!ae->wl[ae->idx].t) {
		ae->idx++;
		freeflag=0;
	
		/* local label */	
		if (ae->wl[ae->idx].w[0]=='.' && ae->lastgloballabel) {
			localname=MemMalloc(strlen(ae->wl[ae->idx].w)+1+ae->lastgloballabellen);
			sprintf(localname,"%s%s",ae->lastgloballabel,ae->wl[ae->idx].w);
			freeflag=1;
		} else {
			localname=ae->wl[ae->idx].w;
		}
		crc=GetCRC(localname);

		if ((curlabel=SearchLabel(ae,localname,crc))!=NULL) {
			curlabel->autorise_export=exportval;
			ae->label[curlabel->backidx].autorise_export=exportval;
		} else {
			if ((curdic=SearchDico(ae,ae->wl[ae->idx].w,crc))!=NULL) {
				curdic->autorise_export=exportval;
			} else {
				if ((ialias=SearchAlias(ae,crc,ae->wl[ae->idx].w))!=-1) {
					ae->alias[ialias].autorise_export=exportval;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"(E)NOEXPORT did not found [%s] in variables, labels or aliases\n",ae->wl[ae->idx].w);
				}
			}
		}
		if (freeflag) MemFree(localname);
	}
}
void __NOEXPORT(struct s_assenv *ae) {
	__internal_EXPORT(ae,0);
}
void __ENOEXPORT(struct s_assenv *ae) {
	__internal_EXPORT(ae,1);
}

void __BUILDZX(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BUILDZX does not need a parameter\n");
	}
	if (!ae->forcesnapshot && !ae->forcetape && !ae->forcecpr && !ae->forceROM) {
		ae->forcezx=1;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot select ZX output when already in Amstrad ROM/cartridge/snapshot/tape output\n");
	}
}
void __BUILDCPR(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1 && strcmp(ae->wl[ae->idx+1].w,"EXTENDED")==0) {
		ae->extendedCPR=1;
	} else if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BUILDCPR unknown parameter\n");
	}
	if (!ae->forcesnapshot && !ae->forcetape && !ae->forcezx && !ae->forceROM) {
		ae->forcecpr=1;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot select Amstrad cartridge output when already in ZX/ROM/snapshot/tape output\n");
	}
}
void __BUILDROM(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BUILDROM does not need a parameter\n");
	}
	if (!ae->forcesnapshot && !ae->forcetape && !ae->forcezx && !ae->forcecpr) {
		ae->forceROM=1;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot select ROM output when already in ZX/cartridge/snapshot/tape output\n");
	}
}
void __BUILDSNA(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		if (strcmp(ae->wl[ae->idx+1].w,"V2")==0) {
		ae->snapshot.version=2;
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BUILDSNA unrecognized option\n");
		}
	}
	if (!ae->forcecpr && !ae->forcetape && !ae->forcezx && !ae->forceROM) {
		ae->forcesnapshot=1;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot select snapshot output when already in ZX/ROM/cartridge/tape output\n");
	}
}
void __BUILDTAPE(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BUILDTAPE does not need a parameter\n");
	}
	if (!ae->forcesnapshot && !ae->forcecpr && !ae->forcezx && !ae->forceROM) {
		ae->forcetape=1;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot select tape output when already in ZX/ROM/snapshot/cartridge output\n");
	}
}
	

void __LZSA1(struct s_assenv *ae) {
	struct s_lz_section curlz={0};
	
	if (!ae->wl[ae->idx].t) {
		ae->idx++;
		curlz.minmatch=atoi(ae->wl[ae->idx].w);
		if (!ae->wl[ae->idx].t) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LZSA1 directive may only have 1 parameter\n");
		}
	}
	#ifdef NO_3RD_PARTIES
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use 3rd parties cruncher with this version of RASM\n");
		FreeAssenv(ae);
		exit(-5);
	#endif
	
	if (ae->lz>=0 && ae->lz<ae->ilz && ae->lzsection[ae->ilz-1].lzversion) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot start a new LZ section inside another one (%d)\n",ae->lz);
		FreeAssenv(ae);
		exit(-5);
	}
	curlz.version=1;
	curlz.iw=ae->idx;
	curlz.iorgzone=ae->io-1;
	curlz.ibank=ae->activebank;
	curlz.memstart=ae->outputadr;
	curlz.memend=-1;
	curlz.lzversion=18;
	ae->lz=ae->ilz;
	ObjectArrayAddDynamicValueConcat((void**)&ae->lzsection,&ae->ilz,&ae->mlz,&curlz,sizeof(curlz));
}
void __LZSA2(struct s_assenv *ae) {
	struct s_lz_section curlz={0};
	
	if (!ae->wl[ae->idx].t) {
		ae->idx++;
		curlz.minmatch=atoi(ae->wl[ae->idx].w);
		if (!ae->wl[ae->idx].t) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LZSA2 directive may only have 1 parameter\n");
		}
	}
	#ifdef NO_3RD_PARTIES
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use 3rd parties cruncher with this version of RASM\n");
		FreeAssenv(ae);
		exit(-5);
	#endif
	
	if (ae->lz>=0 && ae->lz<ae->ilz && ae->lzsection[ae->ilz-1].lzversion) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot start a new LZ section inside another one (%d)\n",ae->lz);
		FreeAssenv(ae);
		exit(-5);
	}
	curlz.version=2;
	curlz.iw=ae->idx;
	curlz.iorgzone=ae->io-1;
	curlz.ibank=ae->activebank;
	curlz.memstart=ae->outputadr;
	curlz.memend=-1;
	curlz.lzversion=18;
	ae->lz=ae->ilz;
	ObjectArrayAddDynamicValueConcat((void**)&ae->lzsection,&ae->ilz,&ae->mlz,&curlz,sizeof(curlz));
}
void __LZAPU(struct s_assenv *ae) {
	struct s_lz_section curlz;
	
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LZ directive does not need any parameter\n");
		return;
	}
	#ifdef NO_3RD_PARTIES
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use 3rd parties cruncher with this version of RASM\n");
		FreeAssenv(ae);
		exit(-5);
	#endif
	
	if (ae->lz>=0 && ae->lz<ae->ilz && ae->lzsection[ae->ilz-1].lzversion) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot start a new LZ section inside another one (%d)\n",ae->lz);
		FreeAssenv(ae);
		exit(-5);
	}
	curlz.iw=ae->idx;
	curlz.iorgzone=ae->io-1;
	curlz.ibank=ae->activebank;
	curlz.memstart=ae->outputadr;
	curlz.memend=-1;
	curlz.lzversion=17;
	ae->lz=ae->ilz;
	ObjectArrayAddDynamicValueConcat((void**)&ae->lzsection,&ae->ilz,&ae->mlz,&curlz,sizeof(curlz));
}
void __LZ4(struct s_assenv *ae) {
	struct s_lz_section curlz;
	
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LZ directive does not need any parameter\n");
		return;
	}
	#ifdef NO_3RD_PARTIES
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use 3rd parties cruncher with this version of RASM\n");
		FreeAssenv(ae);
		exit(-5);
	#endif
	
	if (ae->lz>=0 && ae->lz<ae->ilz && ae->lzsection[ae->ilz-1].lzversion) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot start a new LZ section inside another one (%d)\n",ae->lz);
		FreeAssenv(ae);
		exit(-5);
	}
	curlz.iw=ae->idx;
	curlz.iorgzone=ae->io-1;
	curlz.ibank=ae->activebank;
	curlz.memstart=ae->outputadr;
	curlz.memend=-1;
	curlz.lzversion=4;
	ae->lz=ae->ilz;
	ObjectArrayAddDynamicValueConcat((void**)&ae->lzsection,&ae->ilz,&ae->mlz,&curlz,sizeof(curlz));
}
void __LZX7(struct s_assenv *ae) {
	struct s_lz_section curlz;
	
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LZ directive does not need any parameter\n");
		return;
	}
	#ifdef NO_3RD_PARTIES
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use 3rd parties cruncher with this version of RASM\n");
		FreeAssenv(ae);
		exit(-5);
	#endif
	
	if (ae->lz>=0 && ae->lz<ae->ilz && ae->lzsection[ae->ilz-1].lzversion) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot start a new LZ section inside another one (%d)\n",ae->lz);
		FreeAssenv(ae);
		exit(-5);
	}
	curlz.iw=ae->idx;
	curlz.iorgzone=ae->io-1;
	curlz.ibank=ae->activebank;
	curlz.memstart=ae->outputadr;
	curlz.memend=-1;
	curlz.lzversion=7;
	ae->lz=ae->ilz;
	ObjectArrayAddDynamicValueConcat((void**)&ae->lzsection,&ae->ilz,&ae->mlz,&curlz,sizeof(curlz));
}
void __LZEXO(struct s_assenv *ae) {
	struct s_lz_section curlz;
	
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LZ directive does not need any parameter\n");
		return;
	}
	#ifdef NO_3RD_PARTIES
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot use 3rd parties cruncher with this version of RASM\n");
		FreeAssenv(ae);
		exit(-5);
	#endif
	
	if (ae->lz>=0 && ae->lz<ae->ilz && ae->lzsection[ae->ilz-1].lzversion) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot start a new LZ section inside another one (%d)\n",ae->lz);
		FreeAssenv(ae);
		exit(-5);
	}
	curlz.iw=ae->idx;
	curlz.iorgzone=ae->io-1;
	curlz.ibank=ae->activebank;
	curlz.memstart=ae->outputadr;
	curlz.memend=-1;
	curlz.lzversion=8;
	ae->lz=ae->ilz;
	ObjectArrayAddDynamicValueConcat((void**)&ae->lzsection,&ae->ilz,&ae->mlz,&curlz,sizeof(curlz));
}
void __LZ48(struct s_assenv *ae) {
	struct s_lz_section curlz;

	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LZ directive does not need any parameter\n");
		return;
	}
	if (ae->lz>=0 && ae->lz<ae->ilz && ae->lzsection[ae->ilz-1].lzversion) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot start a new LZ section inside another one (%d)\n",ae->lz);
		FreeAssenv(ae);
		exit(-5);
	}
	curlz.iw=ae->idx;
	curlz.iorgzone=ae->io-1;
	curlz.ibank=ae->activebank;
	curlz.memstart=ae->outputadr;
	curlz.memend=-1;
	curlz.lzversion=48;
	ae->lz=ae->ilz;
	ObjectArrayAddDynamicValueConcat((void**)&ae->lzsection,&ae->ilz,&ae->mlz,&curlz,sizeof(curlz));
}
void __LZ49(struct s_assenv *ae) {
	struct s_lz_section curlz;
	
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LZ directive does not need any parameter\n");
		return;
	}
	if (ae->lz>=0 && ae->lz<ae->ilz && ae->lzsection[ae->ilz-1].lzversion) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot start a new LZ section inside another one (%d)\n",ae->lz);
		FreeAssenv(ae);
		exit(-5);
	}
	
	curlz.iw=ae->idx;
	curlz.iorgzone=ae->io-1;
	curlz.ibank=ae->activebank;
	curlz.memstart=ae->outputadr;
	curlz.memend=-1;
	curlz.lzversion=49;
	ae->lz=ae->ilz;
	ObjectArrayAddDynamicValueConcat((void**)&ae->lzsection,&ae->ilz,&ae->mlz,&curlz,sizeof(curlz));
}
void __LZCLOSE(struct s_assenv *ae) {
	struct s_lz_section curlz;

	if (!ae->ilz || ae->lz==-1) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot close LZ section as it wasn't opened\n");
		return;
	}
	
	ae->lzsection[ae->ilz-1].memend=ae->outputadr;
	ae->lzsection[ae->ilz-1].ilabel=ae->il;
	ae->lzsection[ae->ilz-1].iexpr=ae->ie;
	// commentaire
	curlz.iw=ae->idx;
	curlz.iorgzone=ae->io-1;
	curlz.ibank=ae->activebank;
	curlz.memstart=ae->outputadr;
	curlz.memend=-1;
	curlz.lzversion=0; // intermediate zone
	ae->lz=ae->ilz;
	ObjectArrayAddDynamicValueConcat((void**)&ae->lzsection,&ae->ilz,&ae->mlz,&curlz,sizeof(curlz));

	//ae->lz=-1;
}

void __LIMIT(struct s_assenv *ae) {
	if (ae->wl[ae->idx+1].t!=2) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		___output_set_limit(ae,RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->outputadr,0,0));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LIMIT directive need one integer parameter\n");
	}
}
void OverWriteCheck(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "OverWriteCheck"
	
	int i,j;
	
	/* overwrite checking */
	i=ae->io-1; {
		if (ae->orgzone[i].memstart!=ae->orgzone[i].memend) {
			for (j=0;j<ae->io-1;j++) {
				if (ae->orgzone[j].memstart!=ae->orgzone[j].memend && !ae->orgzone[j].nocode) {
					if (ae->orgzone[i].ibank==ae->orgzone[j].ibank) {
						if ((ae->orgzone[i].memstart>=ae->orgzone[j].memstart && ae->orgzone[i].memstart<ae->orgzone[j].memend)
							|| (ae->orgzone[i].memend>ae->orgzone[j].memstart && ae->orgzone[i].memend<ae->orgzone[j].memend)
							|| (ae->orgzone[i].memstart<=ae->orgzone[j].memstart && ae->orgzone[i].memend>=ae->orgzone[j].memend)) {
							ae->idx--;
							if (ae->orgzone[j].protect) {
								MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"PROTECTED section error [%s] L%d [#%04X-#%04X-B%d] with [%s] L%d [#%04X/#%04X]\n",ae->filename[ae->orgzone[j].ifile],ae->orgzone[j].iline,ae->orgzone[j].memstart,ae->orgzone[j].memend,ae->orgzone[j].ibank<32?ae->orgzone[j].ibank:0,ae->filename[ae->orgzone[i].ifile],ae->orgzone[i].iline,ae->orgzone[i].memstart,ae->orgzone[i].memend);
							} else {
								MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Assembling overwrite [%s] L%d [#%04X-#%04X-B%d] with [%s] L%d [#%04X/#%04X]\n",ae->filename[ae->orgzone[j].ifile],ae->orgzone[j].iline,ae->orgzone[j].memstart,ae->orgzone[j].memend,ae->orgzone[j].ibank<32?ae->orgzone[j].ibank:0,ae->filename[ae->orgzone[i].ifile],ae->orgzone[i].iline,ae->orgzone[i].memstart,ae->orgzone[i].memend);
							}
							i=j=ae->io;
							break;
						}
					}
				}
			}
		}
	}	
}

void ___new_memory_space(struct s_assenv *ae)
{
	#undef FUNC
	#define FUNC "___new_memory_space"
	
	unsigned char *mem;
	struct s_orgzone orgzone={0};

	__internal_UpdateLZBlockIfAny(ae);
	if (ae->io) {
		ae->orgzone[ae->io-1].memend=ae->outputadr;
	}
	if (ae->lz>=0) {
		if (!ae->nowarning) {
			rasm_printf(ae,KWARNING"[%s:%d] Warning: LZ section wasn't closed before a new memory space directive\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
			if (ae->erronwarn) MaxError(ae);
		}
		__LZCLOSE(ae);
	}
	ae->activebank=ae->nbbank;
	mem=MemMalloc(65536);
	memset(mem,0,65536);
	ObjectArrayAddDynamicValueConcat((void**)&ae->mem,&ae->nbbank,&ae->maxbank,&mem,sizeof(mem));

	ae->outputadr=0;
	ae->codeadr=0;
	orgzone.memstart=0;
	orgzone.ibank=ae->activebank;
	orgzone.nocode=ae->nocode=0;
	orgzone.inplace=1;
	ObjectArrayAddDynamicValueConcat((void**)&ae->orgzone,&ae->io,&ae->mo,&orgzone,sizeof(orgzone));

	OverWriteCheck(ae);
}

void __BANK(struct s_assenv *ae) {
	struct s_orgzone orgzone={0};
	int oldcode=0,oldoutput=0;
	int i;
	__internal_UpdateLZBlockIfAny(ae);

	if (ae->io) {
		ae->orgzone[ae->io-1].memend=ae->outputadr;
	}
	/* without parameter, create a new empty space */
	if (ae->wl[ae->idx].t==1) {
		___new_memory_space(ae);
		return;
	}
	
	ae->bankmode=1;
	/* using BANK without build mode will select cartridge output as default */
	if (!ae->forceROM && !ae->forcecpr && !ae->forcesnapshot && !ae->forcezx) ae->forcecpr=1;

	if (ae->wl[ae->idx+1].t!=2) {
		if (strcmp(ae->wl[ae->idx+1].w,"NEXT")==0) {
			/* are we in a temporary space or in the very last bank? */
			if (ae->activebank>=260-1) {
				___new_memory_space(ae);
				return;
			}
			/* switch to next bank! */
			ae->activebank++;
		} else {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
			ae->activebank=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
		}
		if (ae->forcecpr && (ae->activebank<0 || ae->activebank>31) && !ae->extendedCPR) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"FATAL - Bank selection must be from 0 to 31 in cartridge mode\n");
			FreeAssenv(ae);
			exit(2);
		} else if (ae->extendedCPR && (ae->activebank<0 || ae->activebank>256) && !ae->extendedCPR) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"FATAL - Bank selection must be from 0 to 256 in extended cartridge mode\n");
			FreeAssenv(ae);
			exit(2);
		} else if (ae->forcezx && (ae->activebank<0 || ae->activebank>7)) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"FATAL - Bank selection must be from 0 to 7 in ZX Spectrum mode\n");
			FreeAssenv(ae);
			exit(2);
		} else if (ae->forceROM && (ae->activebank<0 || ae->activebank>=256)) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"FATAL - Bank selection must be from 0 to 255 in ROM mode\n");
			FreeAssenv(ae);
			exit(2);
		} else if (ae->forcesnapshot && (ae->activebank<0 || ae->activebank>=260)) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"FATAL - Bank selection must be from 0 to 259 in snapshot mode\n");
			FreeAssenv(ae);
			exit(2);
		}
		/* bankset control */
		if (ae->forcesnapshot && ae->bankset[ae->activebank/4]) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot BANK %d was already select by a previous BANKSET %d\n",ae->activebank,(int)ae->activebank/4);
			ae->idx++;
			return;
		} else {
			ae->bankused[ae->activebank]=1;
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BANK directive need one integer parameter\n");
		return;
	}
	if (ae->lz>=0) {
		if (!ae->nowarning) {
			rasm_printf(ae,KWARNING"[%s:%d] Warning: LZ section wasn't closed before a new BANK directive\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
			if (ae->erronwarn) MaxError(ae);
		}
		__LZCLOSE(ae);
	}

	/* try to get an old ORG settings backward */
	for (i=ae->io-1;i>=0;i--) {
		if (ae->orgzone[i].ibank==ae->activebank) {
			oldcode=ae->orgzone[i].memend;
			oldoutput=ae->orgzone[i].memend;
			break;
		}
	}
	ae->outputadr=oldoutput;
	ae->codeadr=oldcode;
	orgzone.memstart=ae->outputadr;
	/* legacy */
	orgzone.ibank=ae->activebank;
	orgzone.nocode=ae->nocode=0;
	ObjectArrayAddDynamicValueConcat((void**)&ae->orgzone,&ae->io,&ae->mo,&orgzone,sizeof(orgzone));

	OverWriteCheck(ae);
}

void __BANKSET(struct s_assenv *ae) {
	struct s_orgzone orgzone={0};
	int ibank;

	__internal_UpdateLZBlockIfAny(ae);

	if (!ae->forcesnapshot && !ae->forcecpr && !ae->forcezx && !ae->forceROM) ae->forcesnapshot=1;
	if (!ae->forcesnapshot) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BANKSET directive is specific to snapshot output\n");
		return;
	}
	
	if (ae->io) {
		ae->orgzone[ae->io-1].memend=ae->outputadr;
	}
	ae->bankmode=1;
	
	if (ae->wl[ae->idx+1].t!=2) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		ae->activebank=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
		ae->activebank*=4;
		if (ae->forcesnapshot && (ae->activebank<0 || ae->activebank>=260)) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"FATAL - Bank set selection must be from 0 to 64 in snapshot mode\n");
			FreeAssenv(ae);
			exit(2);
		}
		/* control */
		ibank=ae->activebank;
		if (ae->bankused[ibank] || ae->bankused[ibank+1]|| ae->bankused[ibank+2]|| ae->bankused[ibank+3]) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot BANKSET because bank %d was already selected in single page mode\n",ibank);
			ae->idx++;
			return;
		} else {	
			ae->bankset[ae->activebank/4]=1; /* pas très heureux mais bon... */
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BANKSET directive need one integer parameter\n");
		return;
	}
	if (ae->lz>=0) {
		if (!ae->nowarning) {
			rasm_printf(ae,KWARNING"[%s:%d] Warning: LZ section wasn't closed before a new BANKSET directive\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
			if (ae->erronwarn) MaxError(ae);
		}
		__LZCLOSE(ae);
	}

	ae->outputadr=0;
	ae->codeadr=0;
	orgzone.memstart=0;
	orgzone.ibank=ae->activebank;
	orgzone.nocode=ae->nocode=0;
	ObjectArrayAddDynamicValueConcat((void**)&ae->orgzone,&ae->io,&ae->mo,&orgzone,sizeof(orgzone));

	OverWriteCheck(ae);
}


void __NameBANK(struct s_assenv *ae) {
	int ibank;

	ae->bankmode=1;
	if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		if (!StringIsQuote(ae->wl[ae->idx+2].w)) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Syntax is NAMEBANK <bank number>,'<string>'\n");
		} else {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
			ibank=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
			if (ibank<0 || ibank>=BANK_MAX_NUMBER) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"NAMEBANK selection must be from 0 to %d\n",BANK_MAX_NUMBER);
			} else {
				ae->iwnamebank[ibank]=ae->idx+2;
			}
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"NAMEBANK directive need one integer parameter and a string\n");
	}
}

/***
	Winape little compatibility for CPR writing!
*/
void __WRITE(struct s_assenv *ae) {
	int ok=0;
	int lower=-1,upper=-1,bank=-1;

	if (!ae->wl[ae->idx].t && strcmp(ae->wl[ae->idx+1].w,"DIRECT")==0 && !ae->wl[ae->idx+1].t) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+2].w,0);
		lower=RoundComputeExpression(ae,ae->wl[ae->idx+2].w,ae->codeadr,0,0);
		if (!ae->wl[ae->idx+2].t) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+3].w,0);
			upper=RoundComputeExpression(ae,ae->wl[ae->idx+3].w,ae->codeadr,0,0);
		}
		if (!ae->wl[ae->idx+3].t) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+4].w,0);
			bank=RoundComputeExpression(ae,ae->wl[ae->idx+4].w,ae->codeadr,0,0);
		}

		if (ae->maxam) {
			if (lower==65535) lower=-1;
			if (upper==65535) upper=-1;
			if (bank==65535) bank=-1;
		}

		if (lower!=-1) {
			if (lower>=0 && lower<8) {
				ae->idx+=1;
				__BANK(ae);	
				ok=1;
			} else {
				if (!ae->nowarning) {
					rasm_printf(ae,KWARNING"[%s:%d] Warning: WRITE DIRECT lower ROM ignored (value %d out of bounds 0-7)\n",GetCurrentFile(ae),ae->wl[ae->idx].l,lower);
					if (ae->erronwarn) MaxError(ae);
				}
			}
		} else if (upper!=-1) {
			if (upper>=0 && ((ae->forcecpr && upper<32) || (ae->forcesnapshot && upper<BANK_MAX_NUMBER))) {
				ae->idx+=2;
				__BANK(ae);	
				ok=1;
			} else {
				if (!ae->forcecpr && !ae->forcesnapshot) {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: WRITE DIRECT select a ROM without cartridge output\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
						if (ae->erronwarn) MaxError(ae);
					}
				} else {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: WRITE DIRECT upper ROM ignored (value %d out of bounds 0-31)\n",GetCurrentFile(ae),ae->wl[ae->idx].l,upper);
						if (ae->erronwarn) MaxError(ae);
					}
				}
			}
		} else if (bank!=-1) {
			/* selection de bank on ouvre un nouvel espace */
		} else {
			if (!ae->nowarning) {
				rasm_printf(ae,KWARNING"[%s:%d] Warning: meaningless WRITE DIRECT\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
				if (ae->erronwarn) MaxError(ae);
			}
		}
	}
	while (!ae->wl[ae->idx].t) ae->idx++;
	if (!ok) {
		___new_memory_space(ae);
	}
}
void __CHARSET(struct s_assenv *ae) {
	int i,s,e,v,tquote;

	if (ae->wl[ae->idx].t==1) {
		/* reinit charset */
		for (i=0;i<256;i++)
			ae->charset[i]=i;
	} else if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		/* string,value | byte,value */
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+2].w,0);
		v=RoundComputeExpression(ae,ae->wl[ae->idx+2].w,ae->codeadr,0,0);
		if (ae->wl[ae->idx+1].w[0]=='\'' || ae->wl[ae->idx+1].w[0]=='"') {
			tquote=ae->wl[ae->idx+1].w[0];
			if (ae->wl[ae->idx+1].w[strlen(ae->wl[ae->idx+1].w)-1]==tquote) {
				i=1;
				while (ae->wl[ae->idx+1].w[i] && ae->wl[ae->idx+1].w[i]!=tquote) {
					if (ae->wl[ae->idx+1].w[i]=='\\') i++;
					ae->charset[(int)ae->wl[ae->idx+1].w[i]]=(unsigned char)v++;
					i++;
				}
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CHARSET string,value has invalid quote!\n");
			}
		} else {
			i=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
			if (i>=0 && i<256) {
				ae->charset[i]=(unsigned char)v;
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CHARSET byte value must be 0-255\n");
			}
		}
		ae->idx+=2;
	} else if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && !ae->wl[ae->idx+2].t && ae->wl[ae->idx+3].t==1) {
		/* start,end,value */
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+2].w,0);
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+3].w,0);
		s=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
		e=RoundComputeExpression(ae,ae->wl[ae->idx+2].w,ae->codeadr,0,0);
		v=RoundComputeExpression(ae,ae->wl[ae->idx+3].w,ae->codeadr,0,0);
		ae->idx+=3;
		if (s<=e && s>=0 && e<256) {
			for (i=s;i<=e;i++) {
				ae->charset[i]=(unsigned char)v++;
			}
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CHARSET Winape directive wrong interval value\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CHARSET Winape directive wrong parameter count\n");
	}
}

void PushGlobal(struct s_assenv *ae) {
	char *zelast;
	if (ae->lastgloballabel) zelast=TxtStrDup(ae->lastgloballabel); else zelast=NULL;
	ObjectArrayAddDynamicValueConcat((void **)&ae->globalstack,&ae->igs,&ae->mgs,&zelast,sizeof(char *));

#if TRACE_LABEL
printf("==> PushGlobal on Stack [%s] igs=%d\n",zelast,ae->igs);
#endif
}

void PopGlobal(struct s_assenv *ae) {
	if (ae->igs) {
		ae->igs--;
#if TRACE_LABEL
printf("<== PopGlobal on Stack [%s] igs=%d\n",ae->globalstack[ae->igs],ae->igs+1);
#endif
		if (ae->lastglobalalloc) MemFree(ae->lastgloballabel);

		if (ae->globalstack[ae->igs]) {
			ae->lastgloballabel=TxtStrDup(ae->globalstack[ae->igs]);
			ae->lastgloballabellen=strlen(ae->lastgloballabel);
			ae->lastglobalalloc=1;
		} else {
			ae->lastgloballabel=NULL;
			ae->lastgloballabellen=0;
			ae->lastglobalalloc=0;
		}

		if (ae->globalstack[ae->igs]) MemFree(ae->globalstack[ae->igs]);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"PopGlobal INTERNAL ERROR / Please report\n");
	}
}


void __MACRO(struct s_assenv *ae) {
	struct s_macro curmacro={0};
	char *referentfilename,*zeparam;
	int refidx,idx,getparam=1;
	struct s_wordlist curwl;
	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t!=2) {
		/* get the name */
		curmacro.mnemo=ae->wl[ae->idx+1].w;
		curmacro.crc=GetCRC(curmacro.mnemo);
		if (ae->wl[ae->idx+1].t) {
			getparam=0;
		}
		/* overload forbidden */
		/* macro, keywords and directives forbidden */
		if (SearchMacro(ae,curmacro.crc,curmacro.mnemo)>=0) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Macro already defined with this name\n");
		} else {
			if ((SearchDico(ae,ae->wl[ae->idx+1].w,curmacro.crc))!=NULL) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Macro definition: There is already a variable with this name\n");
			} else {
				if ((SearchLabel(ae,ae->wl[ae->idx+1].w,curmacro.crc))!=NULL) {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Macro definition: There is already a label with this name\n");
				} else {
					if ((SearchAlias(ae,curmacro.crc,ae->wl[ae->idx+1].w))!=-1) {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Macro definition: There is already an alias with this name\n");
					} else {
						if (IsRegister(curmacro.mnemo)) {
							MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Macro definition: Cannot choose a register as macro name\n");
						}
					}
				}
			}
		}

		idx=ae->idx+2;
		while (ae->wl[idx].t!=2 && (GetCRC(ae->wl[idx].w)!=CRC_MEND || strcmp(ae->wl[idx].w,"MEND")!=0) && (GetCRC(ae->wl[idx].w)!=CRC_ENDM || strcmp(ae->wl[idx].w,"ENDM")!=0)) {
			if (GetCRC(ae->wl[idx].w)==CRC_MACRO || strcmp(ae->wl[idx].w,"MACRO")==0) {
				/* inception interdite */
				referentfilename=GetCurrentFile(ae);
				refidx=ae->idx;
				ae->idx=idx;
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"You cannot define a macro inside another one (MACRO %s in [%s] L%d)\n",ae->wl[refidx+1].w,referentfilename,ae->wl[refidx].l);
				__STOP(ae);
			}
			if (getparam) {
				/* on prepare les parametres au remplacement */
				zeparam=MemMalloc(strlen(ae->wl[idx].w)+3);
				if (ae->as80) {
					sprintf(zeparam,"%s",ae->wl[idx].w);
				} else {
					sprintf(zeparam,"{%s}",ae->wl[idx].w);
				}
				curmacro.nbparam++;
				curmacro.param=MemRealloc(curmacro.param,curmacro.nbparam*sizeof(char **));
				curmacro.param[curmacro.nbparam-1]=zeparam;
				if (ae->wl[idx].t) {
					/* duplicate parameters without brackets MUST be an OPTION */
					getparam=0;
				}
			} else {
				/* copie la liste de mots */	
				curwl=ae->wl[idx];
				ObjectArrayAddDynamicValueConcat((void **)&curmacro.wc,&curmacro.nbword,&curmacro.maxword,&curwl,sizeof(struct s_wordlist));
			}
			idx++;
		}
		if (ae->wl[idx].t==2) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Macro was not closed\n");
		}
		ObjectArrayAddDynamicValueConcat((void**)&ae->macro,&ae->imacro,&ae->mmacro,&curmacro,sizeof(curmacro));
		/* le quicksort n'est pas optimal mais on n'est pas supposé en créer des milliers */
		qsort(ae->macro,ae->imacro,sizeof(struct s_macro),cmpmacros);

		/* ajustement des mots lus */
		if (ae->wl[idx].t==2) idx--;
		ae->idx=idx;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"MACRO definition need at least one parameter for the name of the macro\n");
	}
}

struct s_wordlist *__MACRO_EXECUTE(struct s_assenv *ae, int imacro) {
	struct s_wordlist *cpybackup;
	int nbparam=0,idx,i,j,idad;
	int ifile,iline,iu,lenparam;
	double v;
	struct s_macro_position curmacropos={0};
	char *zeparam=NULL,*txtparamlist;
	int reload=0;
	
	idx=ae->idx;
	while (!ae->wl[idx].t) {
		nbparam++;
		idx++;
	}

	/* hack to secure macro without parameters with void argument */
	if (!ae->macro[imacro].nbparam) {
		if (nbparam) {
			if (nbparam==1 && strcmp(ae->wl[ae->idx+1].w,"(VOID)")==0) {
				nbparam=0;
				reload=1;
			}
		} else {
			if (ae->macrovoid) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"MACRO [%s] used without (void) and option -void used!\n",ae->macro[imacro].mnemo);
			}
		}
	}
	/* macro must avoid extra params! */
	
	/* cannot VOID a macro with parameters! */
	if (ae->macro[imacro].nbparam && strcmp(ae->wl[ae->idx+1].w,"(VOID)")==0) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"MACRO [%s] has %d parameter%s\n",ae->macro[imacro].mnemo,ae->macro[imacro].nbparam,ae->macro[imacro].nbparam>1?"s":"");
		while (!ae->wl[ae->idx].t) {
			ae->idx++;
		}
		ae->idx++;
	} else {
		if (nbparam!=ae->macro[imacro].nbparam) {
			lenparam=1; // macro without parameters!
			for (i=0;i<ae->macro[imacro].nbparam;i++) {
				lenparam+=strlen(ae->macro[imacro].param[i])+3;
			}
			txtparamlist=MemMalloc(lenparam);
			txtparamlist[0]=0;
			for (i=0;i<ae->macro[imacro].nbparam;i++) {
				strcat(txtparamlist,ae->macro[imacro].param[i]);
				strcat(txtparamlist," ");
			}

			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"MACRO [%s] was defined with %d parameter%s %s\n",ae->macro[imacro].mnemo,ae->macro[imacro].nbparam,ae->macro[imacro].nbparam>1?"s":"",txtparamlist);
			while (!ae->wl[ae->idx].t) {
				ae->idx++;
			}
			ae->idx++;
		} else {
			/* free macro call as we will overwrite it */
			MemFree(ae->wl[ae->idx].w);
			/* is there a void to free? */
			if (reload) {
				MemFree(ae->wl[ae->idx+1].w);
			}
			/* eval parameters? */
			for (i=0;i<nbparam;i++) {
				if (strncmp(ae->wl[ae->idx+1+i].w,"{EVAL}",6)==0) {
					/* parametre entre chevrons, il faut l'interpreter d'abord */
					zeparam=TxtStrDup(ae->wl[ae->idx+1+i].w+6);
					ExpressionFastTranslate(ae,&zeparam,1);
					v=ComputeExpressionCore(ae,zeparam,ae->codeadr,0);
					MemFree(zeparam);
					zeparam=MemMalloc(32);
					snprintf(zeparam,31,"%lf",v);
					zeparam[31]=0;
					MemFree(ae->wl[ae->idx+1+i].w);
					ae->wl[ae->idx+1+i].w=zeparam;
				}
			}
			/* backup parameters */
			cpybackup=MemMalloc((nbparam+1)*sizeof(struct s_wordlist));
			for (i=0;i<nbparam;i++) {
				cpybackup[i]=ae->wl[ae->idx+1+i];
			}
			/************************
			  insert macro position
			*************************/
			curmacropos.start=ae->idx;
			curmacropos.end=ae->idx+ae->macro[imacro].nbword;
			curmacropos.value=ae->macrocounter;
			/* which level? */
			curmacropos.pushed=0;
			if (!ae->imacropos) {
				curmacropos.level=1;
			} else {
				if (ae->macropos[ae->imacropos-1].end<=curmacropos.start) {
					/* same level */
					curmacropos.level=ae->macropos[ae->imacropos-1].level;
				} else {
					/* inception */
					curmacropos.level=ae->macropos[ae->imacropos-1].level+1;
				}
			}

			ObjectArrayAddDynamicValueConcat((void**)&ae->macropos,&ae->imacropos,&ae->mmacropos,&curmacropos,sizeof(curmacropos));
			
			/* are we in a repeat/while block? */
			for (iu=0;iu<ae->ir;iu++) if (ae->repeat[iu].maxim<ae->imacropos) ae->repeat[iu].maxim=ae->imacropos;
			for (iu=0;iu<ae->iw;iu++) if (ae->whilewend[iu].maxim<ae->imacropos) ae->whilewend[iu].maxim=ae->imacropos;
			
			/* update daddy macropos */
			for (idad=0;idad<ae->imacropos-1;idad++) {
				if (ae->macropos[idad].end>curmacropos.start) {
					ae->macropos[idad].end+=ae->macro[imacro].nbword-1-nbparam-reload; /* coz la macro compte un mot! */
				}
			}
			
	#if 0
			for (idad=0;idad<ae->imacropos;idad++) {
				printf("macropos[%d]=%d -> %d\n",idad,ae->macropos[idad].start,ae->macropos[idad].end);
			}
	#endif		
			/* insert at macro position and replace macro+parameters */
			if (ae->macro[imacro].nbword>1+nbparam+reload) {
				ae->nbword+=ae->macro[imacro].nbword-1-nbparam-reload;
				ae->wl=MemRealloc(ae->wl,ae->nbword*sizeof(struct s_wordlist));
			} else {
				/* si on réduit pas de realloc pour ne pas perdre de donnees */
				ae->nbword+=ae->macro[imacro].nbword-1-nbparam-reload;
			}
			iline=ae->wl[ae->idx].l;
			ifile=ae->wl[ae->idx].ifile;
			MemMove(&ae->wl[ae->idx+ae->macro[imacro].nbword],&ae->wl[ae->idx+reload+nbparam+1],(ae->nbword-ae->idx-ae->macro[imacro].nbword)*sizeof(struct s_wordlist));

			for (i=0;i<ae->macro[imacro].nbword;i++) {
				ae->wl[i+ae->idx].w=TxtStrDup(ae->macro[imacro].wc[i].w);
				ae->wl[i+ae->idx].l=iline;
				ae->wl[i+ae->idx].ifile=ifile;
				/* @@@sujet a evolution, ou double controle */
				ae->wl[i+ae->idx].t=ae->macro[imacro].wc[i].t;
				ae->wl[i+ae->idx].e=ae->macro[imacro].wc[i].e;
			}
			/* replace */
			idx=ae->idx;
			for (i=0;i<nbparam;i++) {
				for (j=idx;j<idx+ae->macro[imacro].nbword;j++) {
					/* tags in upper case for replacement in quotes */
					if (StringIsQuote(ae->wl[j].w)) {
						int lm,touched;
						for (lm=touched=0;ae->wl[j].w[lm];lm++) {
							if (ae->wl[j].w[lm]=='{') touched++; else if (ae->wl[j].w[lm]=='}') touched--; else if (touched) ae->wl[j].w[lm]=toupper(ae->wl[j].w[lm]);
						}
					}
//printf("MACRO_EXECUTE word[%d]=[%s] param[%d]=[%s] cpybackup[%d]=[%s]\n",j,ae->wl[j].w,i,ae->macro[imacro].param[i],i,cpybackup[i].w);
					ae->wl[j].w=TxtReplace(ae->wl[j].w,ae->macro[imacro].param[i],cpybackup[i].w,0);
				}
				MemFree(cpybackup[i].w);
			}
			MemFree(cpybackup);

			/* look for specific tags */
			for (j=idx;j<idx+ae->macro[imacro].nbword;j++) {
				switch (GetCRC(ae->wl[j].w)) {
					case CRC_AF_HIGH:if (strcmp(ae->wl[j].w,"AF.HIGH")==0) strcpy(ae->wl[j].w,"A");break;
					case CRC_AF_LOW: if (strcmp(ae->wl[j].w,"AF.LOW")==0) strcpy(ae->wl[j].w,"F");break;
					case CRC_BC_HIGH:if (strcmp(ae->wl[j].w,"BC.HIGH")==0) strcpy(ae->wl[j].w,"B");break;
					case CRC_BC_LOW: if (strcmp(ae->wl[j].w,"BC.LOW")==0) strcpy(ae->wl[j].w,"C");break;
					case CRC_DE_HIGH:if (strcmp(ae->wl[j].w,"DE.HIGH")==0) strcpy(ae->wl[j].w,"D");break;
					case CRC_DE_LOW: if (strcmp(ae->wl[j].w,"DE.LOW")==0) strcpy(ae->wl[j].w,"E");break;
					case CRC_HL_HIGH:if (strcmp(ae->wl[j].w,"HL.HIGH")==0) strcpy(ae->wl[j].w,"H");break;
					case CRC_HL_LOW: if (strcmp(ae->wl[j].w,"HL.LOW")==0) strcpy(ae->wl[j].w,"L");break;
					case CRC_IX_HIGH:if (strcmp(ae->wl[j].w,"IX.HIGH")==0) strcpy(ae->wl[j].w,"XH");break;
					case CRC_IX_LOW: if (strcmp(ae->wl[j].w,"IX.LOW")==0) strcpy(ae->wl[j].w,"XL");break;
					case CRC_IY_HIGH:if (strcmp(ae->wl[j].w,"IY.HIGH")==0) strcpy(ae->wl[j].w,"YH");break;
					case CRC_IY_LOW: if (strcmp(ae->wl[j].w,"IY.LOW")==0) strcpy(ae->wl[j].w,"YL");break;
					default:break;
				}
			}

			/* macro replaced, need to rollback index */
			//ae->idx--;
		}
	}
	/* a chaque appel de macro on incremente le compteur pour les labels locaux */
	ae->macrocounter++;

	return ae->wl;
}

/*
	ticker start, <var>
	ticker stop, <var>
*/
void __TICKER(struct s_assenv *ae) {
	struct s_expr_dico *tvar;
	struct s_ticker ticker;
	int crc,i;

	if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		crc=GetCRC(ae->wl[ae->idx+2].w);

		if (strcmp(ae->wl[ae->idx+1].w,"START")==0) {
			/* is there already a counter?  */
			for (i=0;i<ae->iticker;i++) {
				if (ae->ticker[i].crc==crc && strcmp(ae->wl[ae->idx+2].w,ae->ticker[i].varname)==0) {
					break;
				}
			}
			if (i==ae->iticker) {
				ticker.varname=TxtStrDup(ae->wl[ae->idx+2].w);
				ticker.crc=crc;
				ObjectArrayAddDynamicValueConcat((void **)&ae->ticker,&ae->iticker,&ae->mticker,&ticker,sizeof(struct s_ticker));
			}
			ae->ticker[i].nopstart=ae->nop;
			ae->ticker[i].tickerstart=ae->tick;
		} else if (strncmp(ae->wl[ae->idx+1].w,"STOP",4)==0) {
			for (i=0;i<ae->iticker;i++) {
				if (ae->ticker[i].crc==crc && strcmp(ae->wl[ae->idx+2].w,ae->ticker[i].varname)==0) {
					break;
				}
			}
			if (i<ae->iticker) {
				/* set var */
				if ((tvar=SearchDico(ae,ae->wl[ae->idx+2].w,crc))!=NULL) {
					/* compute nop count */
					if (ae->wl[ae->idx+1].w[4]=='Z') tvar->v=ae->tick-ae->ticker[i].tickerstart;
					else tvar->v=ae->nop-ae->ticker[i].nopstart;
				} else {
					/* create var with nop count */
					if (ae->wl[ae->idx+1].w[4]=='Z') ExpressionSetDicoVar(ae,ae->wl[ae->idx+2].w,ae->tick-ae->ticker[i].tickerstart);
					else ExpressionSetDicoVar(ae,ae->wl[ae->idx+2].w,ae->nop-ae->ticker[i].nopstart);
				}
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"TICKER not found\n");
			}
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"usage is TICKER start/stop(z),<variable>\n");
		}
		ae->idx+=2;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"usage is TICKER start/stop(z),<variable>\n");
	}
}

void __LET(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ae->idx++;
		ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
		RoundComputeExpression(ae,ae->wl[ae->idx].w,ae->codeadr,0,0);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LET useless Winape directive need one expression\n");
	}
}

void __RUN(struct s_assenv *ae) {
	int ramconf=0xC0;
	
	if (!ae->wl[ae->idx].t) {
		ae->current_run_idx=ae->idx+1;
		if (ae->forcezx) {
			if (!ae->wl[ae->idx].t) {
				PushExpression(ae,ae->idx+1,E_EXPRESSION_ZXRUN); // delayed RUN value
				PushExpression(ae,ae->idx+2,E_EXPRESSION_ZXSTACK); // delayed STACK value
				ae->idx+=2;
			} else {
				ae->idx++;
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"usage is RUN <adress>,<stack> (ZX mode)\n");
			}
		} else {
			PushExpression(ae,ae->idx+1,E_EXPRESSION_RUN); // delayed RUN value
			ae->idx++;
			if (!ae->wl[ae->idx].t) {
				ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
				ramconf=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
				ae->idx++;
				if (ramconf<0xC0 || ramconf>0xFF) {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: ram configuration out of bound %X forced to #C0\n",GetCurrentFile(ae),ae->wl[ae->idx].l,ramconf);
						if (ae->erronwarn) MaxError(ae);
					}
					ramconf=0xC0;
				}
				ae->snapshot.ramconfiguration=ramconf;
			}
		}
	} else {
		if (ae->forcezx) MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"usage is RUN <adress>,<stack> (ZX mode)\n");
		else MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"usage is RUN <adress>[,<ppi>]\n");
	}
	if (ae->rundefined && !ae->nowarning) {
		rasm_printf(ae,KWARNING"[%s:%d] Warning: run adress redefinition\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
		if (ae->erronwarn) MaxError(ae);
	}
	ae->rundefined=1;
}
void __BREAKPOINT(struct s_assenv *ae) {
	struct s_breakpoint breakpoint={0};
	
	if (ae->activebank>3) breakpoint.bank=1;
	if (ae->wl[ae->idx].t) {
		breakpoint.address=ae->codeadr;
		ObjectArrayAddDynamicValueConcat((void **)&ae->breakpoint,&ae->ibreakpoint,&ae->maxbreakpoint,&breakpoint,sizeof(struct s_breakpoint));
	} else 	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		breakpoint.address=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
		ObjectArrayAddDynamicValueConcat((void **)&ae->breakpoint,&ae->ibreakpoint,&ae->maxbreakpoint,&breakpoint,sizeof(struct s_breakpoint));
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is BREAKPOINT [adress]\n");
	}
}

void __SNASET(struct s_assenv *ae) {
	int myvalue,idx;

	if (!ae->forcecpr && !ae->forcetape && !ae->forcezx && !ae->forceROM) {
		ae->forcesnapshot=1;
	} else {
		if (!ae->nowarning) {
			rasm_printf(ae,KWARNING"[%s:%d] Warning: Cannot SNASET when already in ZX/ROM/cartridge/tape output\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
			if (ae->erronwarn) MaxError(ae);
		}
	}

	if (!ae->wl[ae->idx].t) {
		ae->idx++;
		/* TWO parameters */
		if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
			/* parameter value */
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
			myvalue=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);

			/* Z80 register/value */
			if (strcmp(ae->wl[ae->idx].w,"Z80_AF")==0) {
				ae->snapshot.registers.general.F=myvalue&0xFF;
				ae->snapshot.registers.general.A=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_F")==0) {
				ae->snapshot.registers.general.F=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_A")==0) {
				ae->snapshot.registers.general.A=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_BC")==0) {
				ae->snapshot.registers.general.C=myvalue&0xFF;
				ae->snapshot.registers.general.B=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_C")==0) {
				ae->snapshot.registers.general.C=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_B")==0) {
				ae->snapshot.registers.general.B=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_DE")==0) {
				ae->snapshot.registers.general.E=myvalue&0xFF;
				ae->snapshot.registers.general.D=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_E")==0) {
				ae->snapshot.registers.general.E=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_D")==0) {
				ae->snapshot.registers.general.D=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_HL")==0) {
				ae->snapshot.registers.general.L=myvalue&0xFF;
				ae->snapshot.registers.general.H=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_L")==0) {
				ae->snapshot.registers.general.L=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_H")==0) {
				ae->snapshot.registers.general.H=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_I")==0) {
				ae->snapshot.registers.regI=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_R")==0) {
				ae->snapshot.registers.R=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_IFF0")==0) {
				ae->snapshot.registers.IFF0=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_IFF1")==0) {
				ae->snapshot.registers.IFF1=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_IX")==0) {
				ae->snapshot.registers.LX=myvalue&0xFF;
				ae->snapshot.registers.HX=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_IXL")==0) {
				ae->snapshot.registers.LX=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_IXH")==0) {
				ae->snapshot.registers.HX=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_IY")==0) {
				ae->snapshot.registers.LY=myvalue&0xFF;
				ae->snapshot.registers.HY=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_IYL")==0) {
				ae->snapshot.registers.LY=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_IYH")==0) {
				ae->snapshot.registers.HY=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_SP")==0) {
				ae->snapshot.registers.LSP=myvalue&0xFF;
				ae->snapshot.registers.HSP=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_PC")==0) {
				ae->snapshot.registers.LPC=myvalue&0xFF;
				ae->snapshot.registers.HPC=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_IM")==0) {
				ae->snapshot.registers.IM=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_AFX")==0) {
				ae->snapshot.registers.alternate.F=myvalue&0xFF;
				ae->snapshot.registers.alternate.A=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_AX")==0) {
				ae->snapshot.registers.alternate.A=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_FX")==0) {
				ae->snapshot.registers.alternate.F=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_BCX")==0) {
				ae->snapshot.registers.alternate.C=myvalue&0xFF;
				ae->snapshot.registers.alternate.B=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_CX")==0) {
				ae->snapshot.registers.alternate.C=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_BX")==0) {
				ae->snapshot.registers.alternate.B=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_DEX")==0) {
				ae->snapshot.registers.alternate.E=myvalue&0xFF;
				ae->snapshot.registers.alternate.D=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_EX")==0) {
				ae->snapshot.registers.alternate.E=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_DX")==0) {
				ae->snapshot.registers.alternate.D=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_HLX")==0) {
				ae->snapshot.registers.alternate.L=myvalue&0xFF;
				ae->snapshot.registers.alternate.H=(myvalue>>8)&0xFF;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_LX")==0) {
				ae->snapshot.registers.alternate.L=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"Z80_HX")==0) {
				ae->snapshot.registers.alternate.H=myvalue;
				/* Gate Array / CRTC / PPI / FDD */
			} else if (strcmp(ae->wl[ae->idx].w,"GA_PEN")==0) {
				ae->snapshot.gatearray.selectedpen=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"GA_ROMCFG")==0) {
				ae->snapshot.gatearray.multiconfiguration=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"GA_RAMCFG")==0) {
				ae->snapshot.ramconfiguration=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_SEL")==0) {
				ae->snapshot.crtc.selectedregister=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"ROM_UP")==0) {
				ae->snapshot.romselect=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"PPI_A")==0) {
				ae->snapshot.ppi.portA=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"PPI_B")==0) {
				ae->snapshot.ppi.portB=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"PPI_C")==0) {
				ae->snapshot.ppi.portC=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"PPI_CTL")==0) {
				ae->snapshot.ppi.control=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"PSG_SEL")==0) {
				ae->snapshot.psg.selectedregister=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CPC_TYPE")==0) {
				ae->snapshot.CPCType=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"INT_NUM")==0) {
				ae->snapshot.interruptnumber=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"FDD_MOTOR")==0) {
				ae->snapshot.fdd.motorstate=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"FDD_TRACK")==0) {
				ae->snapshot.fdd.physicaltrack=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"PRNT_DATA")==0) {
				ae->snapshot.printerstrobe=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_TYPE")==0) {
				ae->snapshot.crtcstate.model=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_HCC")==0) {
				ae->snapshot.crtcstate.HCC=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_CLC")==0) {
				ae->snapshot.crtcstate.CLC=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_RLC")==0) {
				ae->snapshot.crtcstate.RLC=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_VAC")==0) { // Vertical Total Adjust Counter
				ae->snapshot.crtcstate.VTC=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_HSWC")==0) {
				ae->snapshot.crtcstate.HSC=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_VSWC")==0) {
				ae->snapshot.crtcstate.VSC=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_STATE")==0) {
				ae->snapshot.crtcstate.flags=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"GA_VSC")==0) {
				ae->snapshot.vsyncdelay=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"GA_ISC")==0) {
				ae->snapshot.interruptscanlinecounter=myvalue;
			} else if (strcmp(ae->wl[ae->idx].w,"INT_REQ")==0) {
				ae->snapshot.interruptrequestflag=myvalue;
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SNASET directive unknown non array settings\n");
			}
		} else if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
			/* index value */
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
			idx=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);

			/* parameter value */
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+2].w,0);
			myvalue=RoundComputeExpression(ae,ae->wl[ae->idx+2].w,ae->codeadr,0,0);

			if (strcmp(ae->wl[ae->idx].w,"GA_PAL")==0) {
				if (idx>=0 && idx<17) {
					ae->snapshot.gatearray.palette[idx]=myvalue;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SNASET directive needs [0-16] index for GA_PAL\n");
				}
			} else if (strcmp(ae->wl[ae->idx].w,"CRTC_REG")==0) {
				if (idx>=0 && idx<18) {
					ae->snapshot.crtc.registervalue[idx]=myvalue;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SNASET directive needs [0-17] index for CRTC_REG\n");
				}
			} else if (strcmp(ae->wl[ae->idx].w,"PSG_REG")==0) {
				if (idx>=0 && idx<16) {
					ae->snapshot.psg.registervalue[idx]=myvalue;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SNASET directive needs [0-15] index for PSG_REG\n");
				}
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SNASET directive unknown array settings\n");
			}
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SNASET directive need 2 or 3 parameters (see documentation for more informations)\n");
		}
	}


}


void __SETCPC(struct s_assenv *ae) {
	int mycpc;

	rasm_printf(ae,KWARNING"[%s:%d] Warning: SETCPC is deprecated, use SNASET CPC_TYPE,<type> instead\n",GetCurrentFile(ae),ae->wl[ae->idx].l);

	if (!ae->forcecpr && !ae->forceROM && !ae->forcezx) {
		ae->forcesnapshot=1;
	} else {
		if (!ae->nowarning) {
			rasm_printf(ae,KWARNING"[%s:%d] Warning: Cannot SETCPC when already in ZX/ROM/cartridge output\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
			if (ae->erronwarn) MaxError(ae);
		}
	}

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		mycpc=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
		ae->idx++;
		switch (mycpc) {
			case 0:
			case 1:
			case 2:
			case 4:
			case 5:
			case 6:
				ae->snapshot.CPCType=mycpc;
				break;
			default:
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SETCPC directive has wrong value (0,1,2,4,5,6 only)\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SETCPC directive need one integer parameter\n");
	}
}
void __SETCRTC(struct s_assenv *ae) {
	int mycrtc;

	rasm_printf(ae,KWARNING"[%s:%d] Warning: SETCRTC is deprecated, use SNASET CRTC_TYPE,<type> instead\n",GetCurrentFile(ae),ae->wl[ae->idx].l);

	if (!ae->forcecpr && !ae->forcezx && !ae->forceROM) {
		ae->forcesnapshot=1;
	} else {
		if (!ae->nowarning) {
			rasm_printf(ae,KWARNING"[%s:%d] Warning: Cannot SETCRTC when already in ZX/ROM/cartridge output\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
			if (ae->erronwarn) MaxError(ae);
		}
	}

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		mycrtc=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SETCRTC directive need one integer parameter\n");
		mycrtc=0;
	}
	switch (mycrtc) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			ae->snapshot.crtcstate.model=mycrtc;
			break;
		default:
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SETCRTC directive has wrong value (0,1,2,3,4 only)\n");
	}
}


void __LIST(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"LIST Winape directive does not need parameter\n");
	}
}
void __NOLIST(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"NOLIST Winape directive does not need parameter\n");
	}
}

void __BRK(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BRK Winape directive does not need parameter\n");
	} else {
		___output(ae,0xED);
		___output(ae,0xFF);
	}
}

void __STOP(struct s_assenv *ae) {
	rasm_printf(ae,"[%s:%d] STOP assembling requested\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
	while (ae->wl[ae->idx].t!=2) ae->idx++;
	ae->idx--;
	ae->stop=1;
}

void __PRINT(struct s_assenv *ae) {
	while (ae->wl[ae->idx].t!=1) {
		if (!StringIsQuote(ae->wl[ae->idx+1].w)) {
			char *string2print=NULL;
			int hex=0,bin=0,entier=0;
			
			if (strncmp(ae->wl[ae->idx+1].w,"{HEX}",5)==0) {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w+5);
				hex=1;
			} else if (strncmp(ae->wl[ae->idx+1].w,"{HEX2}",6)==0) {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w+6);
				hex=2;
			} else if (strncmp(ae->wl[ae->idx+1].w,"{HEX4}",6)==0) {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w+6);
				hex=4;
			} else if (strncmp(ae->wl[ae->idx+1].w,"{HEX8}",6)==0) {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w+6);
				hex=8;
			} else if (strncmp(ae->wl[ae->idx+1].w,"{BIN}",5)==0) {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w+5);
				bin=1;
			} else if (strncmp(ae->wl[ae->idx+1].w,"{BIN8}",6)==0) {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w+6);
				bin=8;
			} else if (strncmp(ae->wl[ae->idx+1].w,"{BIN16}",7)==0) {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w+7);
				bin=16;
			} else if (strncmp(ae->wl[ae->idx+1].w,"{BIN32}",7)==0) {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w+7);
				bin=32;
			} else if (strncmp(ae->wl[ae->idx+1].w,"{INT}",5)==0) {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w+5);
				entier=1;
			} else {
				string2print=TxtStrDup(ae->wl[ae->idx+1].w);
			}

			ExpressionFastTranslate(ae,&string2print,1);
			if (hex) {
				int zv;
				zv=RoundComputeExpressionCore(ae,string2print,ae->codeadr,0);
				switch (hex) {
					case 1:
						if (zv&0xFFFFFF00) {
							if (zv&0xFFFF0000) {
								rasm_printf(ae,"#%-8.08X ",zv);
							} else {
								rasm_printf(ae,"#%-4.04X ",zv);
							}
						} else {
							rasm_printf(ae,"#%-2.02X ",zv);
						}
						break;
					case 2:rasm_printf(ae,"#%-2.02X ",zv);break;
					case 4:rasm_printf(ae,"#%-4.04X ",zv);break;
					case 8:rasm_printf(ae,"#%-8.08X ",zv);break;
				}
			} else if (bin) {
				int zv,d;
				zv=RoundComputeExpressionCore(ae,string2print,ae->codeadr,0);
				/* remove useless sign bits */
				if (bin<32 && (zv&0xFFFF0000)==0xFFFF0000) {
					zv&=0xFFFF;
				}
				switch (bin) {
					case 1:if (zv&0xFF00) d=15; else d=7;break;
					case 8:d=7;break;
					case 16:d=15;break;
					case 32:d=31;break;
				}
				rasm_printf(ae,"%%");
				for (;d>=0;d--) {
					if ((zv>>d)&1) rasm_printf(ae,"1"); else rasm_printf(ae,"0");
				}
				rasm_printf(ae," ");
			} else if (entier) {
				rasm_printf(ae,"%d ",(int)RoundComputeExpressionCore(ae,string2print,ae->codeadr,0));
			} else {
				rasm_printf(ae,"%.2lf ",ComputeExpressionCore(ae,string2print,ae->codeadr,0));
			}
			MemFree(string2print);
		} else {
			char *varbuffer;
			int lm,touched;
			lm=strlen(ae->wl[ae->idx+1].w)-2;
			if (lm) {
				varbuffer=MemMalloc(lm+2);
				sprintf(varbuffer,"%-*.*s ",lm,lm,ae->wl[ae->idx+1].w+1);
				/* need to upper case tags */
				for (lm=touched=0;varbuffer[lm];lm++) {
					if (varbuffer[lm]=='{') touched++; else if (varbuffer[lm]=='}') touched--; else if (touched) varbuffer[lm]=toupper(varbuffer[lm]);
				}
				/* translate tag will check tag consistency */
				varbuffer=TranslateTag(ae,varbuffer,&touched,1,E_TAGOPTION_REMOVESPACE);
				varbuffer=TxtReplace(varbuffer,"\\b","\b",0);
				varbuffer=TxtReplace(varbuffer,"\\v","\v",0);
				varbuffer=TxtReplace(varbuffer,"\\f","\f",0);
				varbuffer=TxtReplace(varbuffer,"\\r","\r",0);
				varbuffer=TxtReplace(varbuffer,"\\n","\n",0);
				varbuffer=TxtReplace(varbuffer,"\\t","\t",0);
				rasm_printf(ae,"%s ",varbuffer);
				MemFree(varbuffer);
			}
		}
		ae->idx++;
	}
	rasm_printf(ae,"\n");
}

void __FAIL(struct s_assenv *ae) {
	__PRINT(ae);
	__STOP(ae);
	MaxError(ae);
}

void __ALIGN(struct s_assenv *ae) {
	int aval,ifill=-1;
	
	if (ae->io) {
		ae->orgzone[ae->io-1].memend=ae->outputadr;
	}
	if (!ae->wl[ae->idx].t) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		aval=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
		ae->idx++;
		/* align with fill ? */
		if (!ae->wl[ae->idx].t) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
			ifill=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,0);
			ae->idx++;
			if (ifill<0 || ifill>255) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ALIGN fill value must be 0 to 255\n");
				ifill=0;
			}
		}

		if (aval<1 || aval>65535) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ALIGN boundary must be greater than zero and lower than 65536\n");
			aval=1;
		}

		/* touch codeadr only if adress is misaligned */
		if (ae->codeadr%aval) {
			if (ifill==-1) {
				/* virtual ALIGN is moving outputadr the same value as codeadr move */
				ae->outputadr=ae->outputadr-(ae->codeadr%aval)+aval;
				ae->codeadr=ae->codeadr-(ae->codeadr%aval)+aval;
			} else {
				/* physical ALIGN fill bytes */
				while (ae->codeadr%aval) {
					___output(ae,ifill);
					ae->nop+=1;
				}
			}
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ALIGN <boundary>[,fill] directive need one or two integers parameters\n");
	}
}

void ___internal_skip_loop_block(struct s_assenv *ae, int eloopstyle) {
	int *loopstyle=NULL;
	int iloop=0,mloop=0;
	int cidx;

	cidx=ae->idx+2;

	IntArrayAddDynamicValueConcat(&loopstyle,&iloop,&mloop,eloopstyle);
	/* look for WEND */
	while (iloop) {
		if (strcmp(ae->wl[cidx].w,"REPEAT")==0) {
			if (ae->wl[cidx].t) {
				IntArrayAddDynamicValueConcat(&loopstyle,&iloop,&mloop,E_LOOPSTYLE_REPEATUNTIL);
			} else if (ae->wl[cidx+1].t) {
				IntArrayAddDynamicValueConcat(&loopstyle,&iloop,&mloop,E_LOOPSTYLE_REPEATN);
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid REPEAT syntax\n");
			}
		} else if (strcmp(ae->wl[cidx].w,"WHILE")==0) {
			if (!ae->wl[cidx].t && ae->wl[cidx+1].t) {
				IntArrayAddDynamicValueConcat(&loopstyle,&iloop,&mloop,E_LOOPSTYLE_WHILE);
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Invalid WHILE syntax\n");
			}
		} else if (strcmp(ae->wl[cidx].w,"WEND")==0) {
			iloop--;
			if (iloop<0) {
				iloop=0;
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WEND encountered that was not expected\n");
			} else if (loopstyle[iloop]!=E_LOOPSTYLE_WHILE) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WEND encountered but expecting %s\n",loopstyle[iloop]==E_LOOPSTYLE_REPEATN?"REND":"UNTIL");
			}
		} else if (strcmp(ae->wl[cidx].w,"REND")==0) {
			iloop--;
			if (iloop<0) {
				iloop=0;
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"REND encountered that was not expected\n");
			} else if (loopstyle[iloop]!=E_LOOPSTYLE_REPEATN) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"REND encountered but expecting %s\n",loopstyle[iloop]==E_LOOPSTYLE_REPEATUNTIL?"UNTIL":"WEND");
			}
		} else if (strcmp(ae->wl[cidx].w,"UNTIL")==0) {
			iloop--;
			if (iloop<0) {
				iloop=0;
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"UNTIL encountered that was not expected\n");
			} else if (loopstyle[iloop]!=E_LOOPSTYLE_REPEATUNTIL) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"UNTIL encountered but expecting %s\n",loopstyle[iloop]==E_LOOPSTYLE_REPEATN?"REND":"WEND");
			}
		}
		while (!ae->wl[cidx].t) cidx++;
		cidx++;
	}
	MemFree(loopstyle);
	ae->idx=cidx-1;
}

void __WHILE(struct s_assenv *ae) {
	struct s_whilewend whilewend={0};
	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		if (!ComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,2)) {
				/* skip while block */
				___internal_skip_loop_block(ae,E_LOOPSTYLE_WHILE);
				return;
		} else {

			/*************************************************/
			/********* PUSH Global on Stack ******************/
			/*************************************************/
			PushGlobal(ae);

			ae->idx++;
			whilewend.start=ae->idx;
			whilewend.cpt=0;
			whilewend.value=ae->whilecounter;
			whilewend.maxim=ae->imacropos;
			whilewend.while_counter=1;
			ae->whilecounter++;
			/* pour gérer les macros situés dans le while précedent après un repeat/while courant */
			if (ae->iw) whilewend.maxim=ae->whilewend[ae->iw-1].maxim;
			if (ae->ir && ae->repeat[ae->ir-1].maxim>whilewend.maxim) whilewend.maxim=ae->repeat[ae->ir-1].maxim;
			ObjectArrayAddDynamicValueConcat((void**)&ae->whilewend,&ae->iw,&ae->mw,&whilewend,sizeof(whilewend));
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is WHILE <expression>\n");
	}
}
void __WEND(struct s_assenv *ae) {
	if (ae->iw>0) {
		if (ae->wl[ae->idx].t==1) {
			if (ComputeExpression(ae,ae->wl[ae->whilewend[ae->iw-1].start].w,ae->codeadr,0,2)) {
				if (ae->whilewend[ae->iw-1].while_counter>65536) {

					/*************************************************/
					/********* POP Global on Stack *******************/
					/*************************************************/
					PopGlobal(ae);

					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Bypass infinite WHILE loop\n");
					ae->iw--;
					/* refresh macro check index */
					if (ae->iw) ae->imacropos=ae->whilewend[ae->iw-1].maxim;
				} else {
					ae->whilewend[ae->iw-1].cpt++; /* for local label */
					ae->whilewend[ae->iw-1].while_counter++;
					ae->idx=ae->whilewend[ae->iw-1].start;
					/* refresh macro check index */
					ae->imacropos=ae->whilewend[ae->iw-1].maxim;
				}
			} else {

				/*************************************************/
				/********* POP Global on Stack *******************/
				/*************************************************/
				PopGlobal(ae);

				ae->iw--;
				/* refresh macro check index */
				if (ae->iw) ae->imacropos=ae->whilewend[ae->iw-1].maxim;
			}
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WEND does not need any parameter\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WEND encounter whereas there is no referent WHILE\n");
	}
}

void __REPEAT(struct s_assenv *ae) {
	struct s_repeat currepeat={0};
	struct s_expr_dico *rvar;
	int crc;


	
	if (ae->wl[ae->idx+1].t!=2) {
		if (ae->wl[ae->idx].t==0) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
			currepeat.cpt=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,0,0,0);
			if (!currepeat.cpt) {
				/* skip repeat block */
				___internal_skip_loop_block(ae,E_LOOPSTYLE_REPEATN);
				return;
			} else if (currepeat.cpt<1 || currepeat.cpt>65536) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Repeat value (%d) must be from 1 to 65535. Skipping block\n",currepeat.cpt);
				___internal_skip_loop_block(ae,E_LOOPSTYLE_REPEATN);
				return;
			}
			ae->idx++;
			currepeat.start=ae->idx;
			if (ae->wl[ae->idx].t==0) {
				ae->idx++;
				if (ae->wl[ae->idx].t==1) {
					/* la variable peut exister -> OK */
					crc=GetCRC(ae->wl[ae->idx].w);
					if ((rvar=SearchDico(ae,ae->wl[ae->idx].w,crc))!=NULL) {
						rvar->v=1;
					} else {
						/* mais ne peut être un label ou un alias */
						ExpressionSetDicoVar(ae,ae->wl[ae->idx].w, 1);
					}
					currepeat.repeatvar=ae->wl[ae->idx].w;
					currepeat.repeatcrc=crc;
				} else {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"extended syntax is REPEAT <n>,<var>\n");
				}
			}
		} else {
			currepeat.start=ae->idx;
			currepeat.cpt=-1;
		}

		/*************************************************/
		/********* PUSH Global on Stack ******************/
		/*************************************************/
		PushGlobal(ae);

		currepeat.value=ae->repeatcounter;
		currepeat.repeat_counter=1;
		ae->repeatcounter++;
		/* pour gérer les macros situés dans le repeat précedent après le repeat courant */
		if (ae->ir) currepeat.maxim=ae->repeat[ae->ir-1].maxim;
		if (ae->iw && ae->whilewend[ae->iw-1].maxim>currepeat.maxim) currepeat.maxim=ae->whilewend[ae->iw-1].maxim;
		if (ae->imacropos>currepeat.maxim) currepeat.maxim=ae->imacropos;
		ObjectArrayAddDynamicValueConcat((void**)&ae->repeat,&ae->ir,&ae->mr,&currepeat,sizeof(currepeat));
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"wrong REPEAT usage\n");
	}
}

void __REND(struct s_assenv *ae) {
	struct s_expr_dico *rvar;
	if (ae->ir>0) {
		if (ae->repeat[ae->ir-1].cpt==-1) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"REND encounter whereas referent REPEAT was waiting for UNTIL\n");
		} else {
			ae->repeat[ae->ir-1].cpt--;
			ae->repeat[ae->ir-1].repeat_counter++;
			if ((rvar=SearchDico(ae,ae->repeat[ae->ir-1].repeatvar,ae->repeat[ae->ir-1].repeatcrc))!=NULL) {
				rvar->v=ae->repeat[ae->ir-1].repeat_counter;
			}
			if (ae->repeat[ae->ir-1].cpt) {
				ae->idx=ae->repeat[ae->ir-1].start;
				/* refresh macro check index */
				ae->imacropos=ae->repeat[ae->ir-1].maxim;
			} else {
				ae->ir--;
				/* refresh macro check index */
				if (ae->ir) ae->imacropos=ae->repeat[ae->ir-1].maxim;

				/*************************************************/
				/********* POP Global on Stack *******************/
				/*************************************************/
				PopGlobal(ae);

			}
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"REND encounter whereas there is no referent REPEAT\n");
	}
}

void __UNTIL(struct s_assenv *ae) {
	if (ae->ir>0) {
		if (ae->repeat[ae->ir-1].cpt>=0) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"[%s:%d] UNTIL encounter whereas referent REPEAT n was waiting for REND\n");
		} else {
			if (ae->wl[ae->idx].t==0 && ae->wl[ae->idx+1].t==1) {
				ae->repeat[ae->ir-1].repeat_counter++;
				ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
				if (!ComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,2)) {
					if (ae->repeat[ae->ir-1].repeat_counter>65536) {

						/*************************************************/
						/********* POP Global on Stack *******************/
						/*************************************************/
						PopGlobal(ae);

						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Bypass infinite REPEAT loop\n");
						ae->ir--;
						/* refresh macro check index */
						if (ae->ir) ae->imacropos=ae->repeat[ae->ir-1].maxim;
					} else {
						ae->idx=ae->repeat[ae->ir-1].start;
						ae->repeat[ae->ir-1].cpt--; /* for local label */
						/* refresh macro check index */
						ae->imacropos=ae->repeat[ae->ir-1].maxim;
					}
				} else {

					/*************************************************/
					/********* POP Global on Stack *******************/
					/*************************************************/
					PopGlobal(ae);

					ae->ir--;
					/* refresh macro check index */
					if (ae->ir) ae->imacropos=ae->repeat[ae->ir-1].maxim;
				}
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"UNTIL need one expression/evaluation as parameter\n");
			}
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"UNTIL encounter whereas there is no referent REPEAT\n");
	}
}

void __ASSERT(struct s_assenv *ae) {
	char Dot3[4];
	int rexpr;

	if (!ae->wl[ae->idx].t) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		if (strlen(ae->wl[ae->idx+1].w)>29) strcpy(Dot3,"..."); else strcpy(Dot3,"");
		rexpr=!!RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,1);
		if (!rexpr) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx+1].l,"ASSERT %.29s%s failed with ",ae->wl[ae->idx+1].w,Dot3);
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,1);
			rasm_printf(ae,"%s\n",ae->wl[ae->idx+1].w);
 			if (!ae->wl[ae->idx+1].t) {
				ae->idx++;
				rasm_printf(ae,"-> ");
				__PRINT(ae);
			}
			__STOP(ae);
		} else {
			while (!ae->wl[ae->idx].t) ae->idx++;
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ASSERT need one expression\n");
	}
}

void __IF(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};
	int rexpr;

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		rexpr=!!RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,1);
		ifthen.v=rexpr;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IF;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IF need one expression\n");
	}
}

void __IF_light(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		/* does not need to compute the value in shadow execution */
		ifthen.v=0;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IF;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IF need one expression\n");
	}
}

/* test if a label or a variable where used before */
void __IFUSED(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};
	int rexpr,crc;
	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		crc=GetCRC(ae->wl[ae->idx+1].w);
		if ((SearchDico(ae,ae->wl[ae->idx+1].w,crc))!=NULL) {
			rexpr=1;
		} else {
			if ((SearchLabel(ae,ae->wl[ae->idx+1].w,crc))!=NULL) {
				rexpr=1;
			} else {
				if ((SearchAlias(ae,crc,ae->wl[ae->idx+1].w))!=-1) {
					rexpr=1;
				} else {
					rexpr=SearchUsed(ae,ae->wl[ae->idx+1].w,crc);
				}
			}
		}
		ifthen.v=rexpr;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IFUSED;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IFUSED need one variable or label\n");
	}
}
void __IFNUSED(struct s_assenv *ae) {
	__IFUSED(ae);
	ae->ifthen[ae->ii-1].v=1-ae->ifthen[ae->ii-1].v;
	ae->ifthen[ae->ii-1].type=E_IFTHEN_TYPE_IFNUSED;
}
void __IFUSED_light(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};
	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ifthen.v=0;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IFUSED;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IFUSED need one variable or label\n");
	}
}
void __IFNUSED_light(struct s_assenv *ae) {
	__IFUSED_light(ae);
	ae->ifthen[ae->ii-1].type=E_IFTHEN_TYPE_IFNUSED;
}

/* test if a label or a variable exists */
void __IFDEF(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};
	int rexpr,crc;
	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		crc=GetCRC(ae->wl[ae->idx+1].w);
		if ((SearchDico(ae,ae->wl[ae->idx+1].w,crc))!=NULL) {
			rexpr=1;
		} else {
			if ((SearchLabel(ae,ae->wl[ae->idx+1].w,crc))!=NULL) {
				rexpr=1;
			} else {
				if ((SearchAlias(ae,crc,ae->wl[ae->idx+1].w))!=-1) {
					rexpr=1;
				} else {
					if (SearchMacro(ae,crc,ae->wl[ae->idx+1].w)>=0) {
						rexpr=1;
					} else {
						rexpr=0;
					}
				}
			}
		}
		ifthen.v=rexpr;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IFDEF;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IFDEF need one variable or label\n");
	}
}
void __IFDEF_light(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};
	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ifthen.v=0;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IFDEF;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IFDEF need one variable or label\n");
	}
}
void __IFNDEF(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};
	int rexpr,crc;


	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		crc=GetCRC(ae->wl[ae->idx+1].w);
		if ((SearchDico(ae,ae->wl[ae->idx+1].w,crc))!=NULL) {
			rexpr=0;
		} else {
			if ((SearchLabel(ae,ae->wl[ae->idx+1].w,crc))!=NULL) {
				rexpr=0;
			} else {
				if ((SearchAlias(ae,crc,ae->wl[ae->idx+1].w))!=-1) {
					rexpr=0;
				} else {
					if (SearchMacro(ae,crc,ae->wl[ae->idx+1].w)>=0) {
						rexpr=0;
					} else {
						rexpr=1;
					}
				}
			}
		}
		ifthen.v=rexpr;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IFNDEF;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IFNDEF need one variable or label\n");
	}
}
void __IFNDEF_light(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};

	
	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ifthen.v=0;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IFNDEF;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IFNDEF need one variable or label\n");
	}
}

void __UNDEF(struct s_assenv *ae) {

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		/* no error when the variable to UNDEF does not exist */
		DelDico(ae,ae->wl[ae->idx+1].w,GetCRC(ae->wl[ae->idx+1].w));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"syntax is UNDEF <variable>\n");
	}

}


void __SWITCH(struct s_assenv *ae) {
	struct s_switchcase curswitch={0};

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		/* switch store the value */
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		curswitch.refval=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,1);
		ObjectArrayAddDynamicValueConcat((void**)&ae->switchcase,&ae->isw,&ae->msw,&curswitch,sizeof(curswitch));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SWITCH need one expression\n");
	}
}
void __CASE(struct s_assenv *ae) {
	int rexpr;
	
	if (ae->isw) {
		if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
			rexpr=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,1);
			
			if (ae->switchcase[ae->isw-1].refval==rexpr) {
				ae->switchcase[ae->isw-1].execute=1;
				ae->switchcase[ae->isw-1].casematch=1;
			}
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CASE not need one parameter\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CASE encounter whereas there is no referent SWITCH\n");
	}
}
void __DEFAULT(struct s_assenv *ae) {
	
	if (ae->isw) {
		if (ae->wl[ae->idx].t==1) {
			/* aucun match avant, on active, sinon on laisse tel quel */
			if (!ae->switchcase[ae->isw-1].casematch) {
				ae->switchcase[ae->isw-1].execute=1;
			}
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFAULT does not need parameter\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFAULT encounter whereas there is no referent SWITCH\n");
	}
}
void __BREAK(struct s_assenv *ae) {
	
	if (ae->isw) {
		if (ae->wl[ae->idx].t==1) {
			ae->switchcase[ae->isw-1].execute=0;
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BREAK does not need parameter\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BREAK encounter whereas there is no referent SWITCH\n");
	}
}
void __SWITCH_light(struct s_assenv *ae) {
	struct s_switchcase curswitch={0};

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		/* shadow execution */
		curswitch.refval=0;
		curswitch.execute=0;
		ObjectArrayAddDynamicValueConcat((void**)&ae->switchcase,&ae->isw,&ae->msw,&curswitch,sizeof(curswitch));
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SWITCH need one expression\n");
	}
}
void __CASE_light(struct s_assenv *ae) {
	if (ae->isw) {
		/* shadowed execution */
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CASE encounter whereas there is no referent SWITCH\n");
	}
}
void __DEFAULT_light(struct s_assenv *ae) {
	
	if (ae->isw) {
		/* shadowed execution */
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DEFAULT encounter whereas there is no referent SWITCH\n");
	}
}
void __BREAK_light(struct s_assenv *ae) {
	if (ae->isw) {
		/* shadowed execution */
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"BREAK encounter whereas there is no referent SWITCH\n");
	}
}
void __ENDSWITCH(struct s_assenv *ae) {
	if (ae->isw) {
		if (ae->wl[ae->idx].t==1) {
			ae->isw--;
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ENDSWITCH does not need any parameter\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ENDSWITCH encounter whereas there is no referent SWITCH\n");
	}
}

void __IFNOT(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};
	int rexpr;

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		rexpr=!RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,1);
		ifthen.v=rexpr;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IFNOT;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		//IntArrayAddDynamicValueConcat(&ae->ifthen,&ae->ii,&ae->mi,rexpr);
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IFNOT need one expression\n");
	}
}
void __IFNOT_light(struct s_assenv *ae) {
	struct s_ifthen ifthen={0};

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ifthen.v=0;
		ifthen.filename=GetCurrentFile(ae);
		ifthen.line=ae->wl[ae->idx].l;
		ifthen.type=E_IFTHEN_TYPE_IFNOT;
		ObjectArrayAddDynamicValueConcat((void **)&ae->ifthen,&ae->ii,&ae->mi,&ifthen,sizeof(ifthen));
		//IntArrayAddDynamicValueConcat(&ae->ifthen,&ae->ii,&ae->mi,rexpr);
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"IFNOT need one expression\n");
	}
}

void __ELSE(struct s_assenv *ae) {
	if (ae->ii) {
		if (ae->wl[ae->idx].t==1) {
			/* ELSE a executer seulement si celui d'avant est a zero */
			switch (ae->ifthen[ae->ii-1].v) {
				case -1:break;
				case 0:ae->ifthen[ae->ii-1].v=1;break;
				case 1:ae->ifthen[ae->ii-1].v=0;break;
			}
			ae->ifthen[ae->ii-1].type=E_IFTHEN_TYPE_ELSE;
			ae->ifthen[ae->ii-1].line=ae->wl[ae->idx].l;
			ae->ifthen[ae->ii-1].filename=GetCurrentFile(ae);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ELSE does not need any parameter\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ELSE encounter whereas there is no referent IF\n");
	}
}
void __ELSEIF(struct s_assenv *ae) {

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ae->ifthen[ae->ii-1].type=E_IFTHEN_TYPE_ELSEIF;
		ae->ifthen[ae->ii-1].line=ae->wl[ae->idx].l;
		ae->ifthen[ae->ii-1].filename=GetCurrentFile(ae);
		if (ae->ifthen[ae->ii-1].v) {
			/* il faut signifier aux suivants qu'on va jusqu'au ENDIF */
			ae->ifthen[ae->ii-1].v=-1;
		} else {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
			ae->ifthen[ae->ii-1].v=!!RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->codeadr,0,1);
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ELSEIF need one expression\n");
	}
}
void __ELSEIF_light(struct s_assenv *ae) {

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		ae->ifthen[ae->ii-1].type=E_IFTHEN_TYPE_ELSEIF;
		ae->ifthen[ae->ii-1].line=ae->wl[ae->idx].l;
		ae->ifthen[ae->ii-1].filename=GetCurrentFile(ae);
		if (ae->ifthen[ae->ii-1].v) {
			/* il faut signifier aux suivants qu'on va jusqu'au ENDIF */
			ae->ifthen[ae->ii-1].v=-1;
		} else {
			ae->ifthen[ae->ii-1].v=0;
		}
		ae->idx++;
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ELSEIF need one expression\n");
	}
}
void __ENDIF(struct s_assenv *ae) {
	if (ae->ii) {
		if (ae->wl[ae->idx].t==1) {
			ae->ii--;
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ENDIF does not need any parameter\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ENDIF encounter whereas there is no referent IF\n");
	}
}

void __internal_PROTECT(struct s_assenv *ae, int memstart, int memend) {
	struct s_orgzone orgzone={0};

	/* add a fake ORG zone */
	ObjectArrayAddDynamicValueConcat((void**)&ae->orgzone,&ae->io,&ae->mo,&orgzone,sizeof(orgzone));
	/* then switch it with the current ORG */
	orgzone=ae->orgzone[ae->io-2];
	ae->orgzone[ae->io-2].memstart=memstart;
	ae->orgzone[ae->io-2].memend=memend;
	ae->orgzone[ae->io-2].ibank=ae->activebank;
	ae->orgzone[ae->io-2].protect=1;
	ae->orgzone[ae->io-1]=orgzone;
}

void __PROTECT(struct s_assenv *ae) {
	int memstart,memend;

	if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+2].w,0);
		memstart=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,0,0,0);
		memend=RoundComputeExpression(ae,ae->wl[ae->idx+2].w,0,0,0);
		__internal_PROTECT(ae,memstart,memend);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"PROTECT need two parameters: startadr,endadr\n");
	}
}

void ___org_close(struct s_assenv *ae) {
	__internal_UpdateLZBlockIfAny(ae);
	if (ae->lz>=0 && ae->lzsection[ae->ilz-1].lzversion) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Cannot ORG inside a LZ section\n");
		return;
	}
	/* close current ORG */
	if (ae->io) {
		ae->orgzone[ae->io-1].memend=ae->outputadr;
	}
}

void ___org_new(struct s_assenv *ae, int nocode) {
	struct s_orgzone orgzone={0};
	int i;
	
	/* check current ORG request */
	for (i=0;i<ae->io;i++) {
		/* aucun contrôle sur les ORG non écrits ou en NOCODE */
		if (ae->orgzone[i].memstart!=ae->orgzone[i].memend && !ae->orgzone[i].nocode) {
			if (ae->orgzone[i].ibank==ae->activebank) {
				if (ae->outputadr<ae->orgzone[i].memend && ae->outputadr>=ae->orgzone[i].memstart) {
					if (ae->orgzone[i].protect) {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ORG located a PROTECTED section [#%04X-#%04X-B%d] file [%s] line %d\n",ae->orgzone[i].memstart,ae->orgzone[i].memend,ae->orgzone[i].ibank<32?ae->orgzone[i].ibank:0,ae->filename[ae->orgzone[i].ifile],ae->orgzone[i].iline);
					} else {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ORG (output at #%04X) located in a previous ORG section [#%04X-#%04X-B%d] file [%s] line %d\n",ae->outputadr,ae->orgzone[i].memstart,ae->orgzone[i].memend,ae->orgzone[i].ibank<32?ae->orgzone[i].ibank:0,ae->filename[ae->orgzone[i].ifile],ae->orgzone[i].iline);
					}
				}
			}
		}
	}
	
	OverWriteCheck(ae);
	/* if there was a crunch block before, now closed */
	if (ae->lz>=0) {
		ae->lz=-1;
	}	
	orgzone.memstart=ae->outputadr;
	orgzone.ibank=ae->activebank;
	orgzone.ifile=ae->wl[ae->idx].ifile;
	orgzone.iline=ae->wl[ae->idx].l;
	orgzone.nocode=ae->nocode=nocode;

	if (nocode) {
		___output=___internal_output_nocode;
	} else {
		___output=___internal_output;
	}
	
	ObjectArrayAddDynamicValueConcat((void**)&ae->orgzone,&ae->io,&ae->mo,&orgzone,sizeof(orgzone));
}

void __ORG(struct s_assenv *ae) {
	___org_close(ae);
	
	if (ae->wl[ae->idx+1].t!=2) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,0);
		ae->codeadr=RoundComputeExpression(ae,ae->wl[ae->idx+1].w,ae->outputadr,0,0);
		if (!ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t!=2) {
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+2].w,0);
			ae->outputadr=RoundComputeExpression(ae,ae->wl[ae->idx+2].w,ae->outputadr,0,0);
			ae->idx+=2;
		} else {
			ae->outputadr=ae->codeadr;
			ae->idx++;
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"[%s:%d] ORG code location[,output location]\n");
		return;
	}
	
	___org_new(ae,ae->nocode);

	if (ae->outputadr==ae->codeadr) ae->orgzone[ae->io-1].inplace=1;
}
void __NOCODE(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t==1) {
		___org_close(ae);
		ae->codeadrbackup=ae->codeadr;
		ae->outputadrbackup=ae->outputadr;
		___org_new(ae,1);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"NOCODE directive does not need parameter\n");
	}
}
void __CODE(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		if (strcmp(ae->wl[ae->idx+1].w,"SKIP")==0) {
			___org_close(ae);
			ae->codeadr=ae->codeadrbackup;
			ae->outputadr=ae->outputadrbackup;
			___org_new(ae,1);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"unknown parameter for CODE directive\n");
		}
		ae->idx++;
	} else if (ae->wl[ae->idx].t==1) {
		___org_close(ae);
		___org_new(ae,0);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"CODE directive does not need parameter\n");
	}
}
void __STRUCT(struct s_assenv *ae) {
	#undef FUNC
	#define FUNC "__STRUCT"
	struct s_rasmstructfield rasmstructfield={0};
	struct s_rasmstruct rasmstruct={0};
	struct s_rasmstruct rasmstructalias={0};
	struct s_label curlabel={0};
	int crc,i,j,irs;
	/* filler */
	int localsize,cursize;

	if (!ae->wl[ae->idx].t) {
		if (ae->wl[ae->idx+1].t) {
			/**************************************************
			    s t r u c t u r e     d e c l a r a t i o n
			**************************************************/
			if (!ae->getstruct) {
				/* cannot be an existing label or EQU (but variable ok) */
				crc=GetCRC(ae->wl[ae->idx+1].w);
				if ((SearchLabel(ae,ae->wl[ae->idx+1].w,crc))!=NULL || (SearchAlias(ae,crc,ae->wl[ae->idx+1].w))!=-1) {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"STRUCT name must be different from existing labels ou aliases\n");
				} else {
					ae->backup_filename=GetCurrentFile(ae);
					ae->backup_line=ae->wl[ae->idx].l;
					ae->backup_outputadr=ae->outputadr;
					ae->backup_codeadr=ae->codeadr;
					ae->getstruct=1;
					/* STRUCT = NOCODE + ORG 0 */
					___org_close(ae);
					ae->codeadr=0;
					___org_new(ae,1);
					/* create struct */
					rasmstruct.name=TxtStrDup(ae->wl[ae->idx+1].w);
					rasmstruct.crc=GetCRC(rasmstruct.name);
					ObjectArrayAddDynamicValueConcat((void **)&ae->rasmstruct,&ae->irasmstruct,&ae->mrasmstruct,&rasmstruct,sizeof(rasmstruct));
					ae->idx++;
					
					/* wrapper for data capture */
					instruction[ICRC_DEFB].makemnemo=_DEFB_struct;instruction[ICRC_DB].makemnemo=_DEFB_struct;
					instruction[ICRC_DEFW].makemnemo=_DEFW_struct;instruction[ICRC_DW].makemnemo=_DEFW_struct;
					instruction[ICRC_DEFI].makemnemo=_DEFI_struct;
					instruction[ICRC_DEFF].makemnemo=_DEFF_struct;instruction[ICRC_DF].makemnemo=_DEFF_struct;
					instruction[ICRC_DEFR].makemnemo=_DEFR_struct;instruction[ICRC_DR].makemnemo=_DEFR_struct;
					instruction[ICRC_DEFS].makemnemo=_DEFS_struct;instruction[ICRC_DS].makemnemo=_DEFS_struct;
				}
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"STRUCT cannot be declared inside previous opened STRUCT [%s] Line %d\n",ae->backup_filename,ae->backup_line);
			}
		} else {
			/**************************************************
				s t r u c t u r e     i n s e r t i o n
			**************************************************/
			int nbelem=1;
#if TRACE_STRUCT
printf("structure insertion\n");
#endif
			/* insert struct param1 in memory with name param2 */
			crc=GetCRC(ae->wl[ae->idx+1].w);
			/* look for existing struct */
			for (irs=0;irs<ae->irasmstruct;irs++) {
				if (ae->rasmstruct[irs].crc==crc && strcmp(ae->rasmstruct[irs].name,ae->wl[ae->idx+1].w)==0) break;
			}
			if (irs==ae->irasmstruct) {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Unknown STRUCT %s\n",ae->wl[ae->idx+1].w);
			} else {
				/* create alias for sizeof */
				if (!ae->getstruct) {
					if (ae->wl[ae->idx+2].w[0]=='@') {
						rasmstructalias.name=MakeLocalLabel(ae,ae->wl[ae->idx+2].w,NULL);
					} else {
						rasmstructalias.name=TxtStrDup(ae->wl[ae->idx+2].w);
					}
				} else {
#if TRACE_STRUCT
printf("struct [%s] inside struct\n",ae->wl[ae->idx+2].w);
#endif
					/* struct inside struct */
					rasmstructalias.name=MemMalloc(strlen(ae->rasmstruct[ae->irasmstruct-1].name)+2+strlen(ae->wl[ae->idx+2].w));
					sprintf(rasmstructalias.name,"%s.%s",ae->rasmstruct[ae->irasmstruct-1].name,ae->wl[ae->idx+2].w);
				}
				rasmstructalias.crc=GetCRC(rasmstructalias.name);
				rasmstructalias.size=ae->rasmstruct[irs].size;
				rasmstructalias.ptr=ae->codeadr;
#if TRACE_STRUCT
printf("structalias [%s] ptr=%d size=%d\n",rasmstructalias.name,rasmstructalias.ptr,rasmstructalias.size);
#endif
				/* extra parameter to declare an array? */
				if (!ae->wl[ae->idx+2].t && !StringIsQuote(ae->wl[ae->idx+3].w)) {
					ExpressionFastTranslate(ae,&ae->wl[ae->idx+3].w,0);
					nbelem=RoundComputeExpression(ae,ae->wl[ae->idx+3].w,ae->outputadr,0,0);
					if (nbelem<1) {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Struct array need a positive number of elements!\n");
						nbelem=1;
					}
					ae->idx++;
				}
				rasmstructalias.nbelem=nbelem;
#if TRACE_STRUCT
printf("EVOL 119 - tableau! %d elem(s)\n",nbelem);
#endif
				ObjectArrayAddDynamicValueConcat((void **)&ae->rasmstructalias,&ae->irasmstructalias,&ae->mrasmstructalias,&rasmstructalias,sizeof(rasmstructalias));
				
				/* create label for global struct ptr */
				curlabel.iw=-1;
				curlabel.ptr=ae->codeadr;
				if (!ae->getstruct) {
					if (ae->wl[ae->idx+2].w[0]=='@') curlabel.name=MakeLocalLabel(ae,ae->wl[ae->idx+2].w,NULL); else curlabel.name=TxtStrDup(ae->wl[ae->idx+2].w);
					curlabel.crc=GetCRC(curlabel.name);
					PushLabelLight(ae,&curlabel);
				} else {
					/* or check for non-local name in struct declaration */
					if (ae->wl[ae->idx+2].w[0]=='@') {
						MakeError(ae,GetCurrentFile(ae),GetExpLine(ae,0),"Meaningless use of local label in a STRUCT definition\n");
					} else {
						curlabel.name=TxtStrDup(rasmstructalias.name);
						curlabel.crc=GetCRC(curlabel.name);
						PushLabelLight(ae,&curlabel);
					}
				}

				/* first field is in fact the very beginning of the structure */
				if (ae->getstruct) {
					rasmstructfield.name=TxtStrDup(ae->wl[ae->idx+2].w);
					rasmstructfield.offset=ae->codeadr;
					ObjectArrayAddDynamicValueConcat((void **)&ae->rasmstruct[ae->irasmstruct-1].rasmstructfield,
							&ae->rasmstruct[ae->irasmstruct-1].irasmstructfield,&ae->rasmstruct[ae->irasmstruct-1].mrasmstructfield,
							&rasmstructfield,sizeof(rasmstructfield));
				}				
				
				/* create subfields */
#if TRACE_STRUCT
printf("create subfields\n");
#endif
				curlabel.iw=-1;
				curlabel.ptr=ae->codeadr;
				curlabel.ibank=ae->activebank<BANK_MAX_NUMBER?ae->activebank:0;
				for (i=0;i<ae->rasmstruct[irs].irasmstructfield;i++) {
					curlabel.ptr=ae->codeadr+ae->rasmstruct[irs].rasmstructfield[i].offset;
					if (!ae->getstruct) {
						curlabel.name=MemMalloc(strlen(ae->wl[ae->idx+2].w)+strlen(ae->rasmstruct[irs].rasmstructfield[i].name)+2);
						sprintf(curlabel.name,"%s.%s",ae->wl[ae->idx+2].w,ae->rasmstruct[irs].rasmstructfield[i].name);
						if (ae->wl[ae->idx+2].w[0]=='@') {
							char *newlabel;
							newlabel=MakeLocalLabel(ae,curlabel.name,NULL);
							MemFree(curlabel.name);
							curlabel.name=newlabel;
						}
						curlabel.crc=GetCRC(curlabel.name);
						PushLabelLight(ae,&curlabel);
					/* are we using a struct in a struct definition? */
					} else {
						/* copy structname+label+offset in the structure */
						rasmstructfield.name=MemMalloc(strlen(ae->wl[ae->idx+2].w)+strlen(ae->rasmstruct[irs].rasmstructfield[i].name)+2);
						sprintf(rasmstructfield.name,"%s.%s",ae->wl[ae->idx+2].w,ae->rasmstruct[irs].rasmstructfield[i].name);
						rasmstructfield.offset=curlabel.ptr;
						ObjectArrayAddDynamicValueConcat((void **)&ae->rasmstruct[ae->irasmstruct-1].rasmstructfield,
								&ae->rasmstruct[ae->irasmstruct-1].irasmstructfield,&ae->rasmstruct[ae->irasmstruct-1].mrasmstructfield,
								&rasmstructfield,sizeof(rasmstructfield));
								
						/* need to push also generic label */
						curlabel.name=MemMalloc(strlen(ae->rasmstruct[ae->irasmstruct-1].name)+strlen(rasmstructfield.name)+2); /* overwrite PTR */
						sprintf(curlabel.name,"%s.%s",ae->rasmstruct[ae->irasmstruct-1].name,rasmstructfield.name);
						curlabel.crc=GetCRC(curlabel.name);
						PushLabelLight(ae,&curlabel);
					}					
#if TRACE_STRUCT
printf("pushLight [%s] %d:%X\n",curlabel.name,curlabel.ibank,curlabel.ptr);
#endif
				}

				/* is there any filler in the declaration? */
				localsize=0;

				/* déterminer si on est en remplissage par défaut ou remplissage surchargé */





#if TRACE_STRUCT
printf("struct new behaviour (scan for %d fields)\n",ae->rasmstruct[irs].irasmstructfield);
#endif
#if 0
				for (i=0;i<ae->rasmstruct[irs].irasmstructfield;i++) {

					if (!ae->wl[ae->idx+2+i].t || i+1>=ae->rasmstruct[irs].irasmstructfield) {
						/* si le champ est sur le même offset que le précédent, on le saute */
						if (i && ae->rasmstruct[irs].rasmstructfield[i].offset>ae->rasmstruct[irs].rasmstructfield[i-1].offset) continue;

#if TRACE_STRUCT
printf("get field? (%d)\n",irs);
#endif
						if (!StringIsQuote(ae->wl[ae->idx+i].w)) {
							ExpressionFastTranslate(ae,&ae->wl[ae->idx+i].w,1);
							zeval=RoundComputeExpressionCore(ae,ae->wl[ae->idx+i].w,ae->codeadr,0);
						} else {
							// push string
						}

						localsize+=ae->rasmstruct[irs].rasmstructfield[i].size;

						/* pour du single shot ?
						pushbyte(s) at ae->codeadr+ae->rasmstruct[irs].rasmstructfield[i].offset
						*/

						//ae->rasmstruct[irs].size;

					} else {
#if TRACE_STRUCT
printf("*break*\n");
#endif
						break;
					}
				}
#endif

				/* (LEGACY)  filler, on balance des zéros */
#if TRACE_STRUCT
printf("struct (almost) legacy filler from %d to %d-1\n",localsize,ae->rasmstruct[irs].size);
#endif
				while (nbelem) {
					for (i=cursize=0;i<ae->rasmstruct[irs].irasmstructfield && cursize<localsize;i++) {
						cursize+=ae->rasmstruct[irs].rasmstructfield[i].size;
					}
					for (;i<ae->rasmstruct[irs].irasmstructfield;i++) {
						for (j=0;j<ae->rasmstruct[irs].rasmstructfield[i].idata;j++) {
							___output(ae,ae->rasmstruct[irs].rasmstructfield[i].data[j]);
						}
					}
					nbelem--;
				}

#if 0
				for (i=localsize;i<ae->rasmstruct[irs].size;i++) ___output(ae,0);
#endif
				ae->idx+=2; // probablement à revoir dans le cas d'une init!!!
			}
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"STRUCT directive needs one or two parameters\n");
	}
}
void __ENDSTRUCT(struct s_assenv *ae) {
	#undef FUNC
	#define FUNC "__ENDSTRUCT"
	struct s_label curlabel={0};
	int i,newlen;
#if TRACE_STRUCT
	printf("endstruct\n");
#endif

	if (!ae->wl[ae->idx].t) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ENDSTRUCT directive does not need parameter\n");
	} else {
		if (ae->getstruct) {
			ae->rasmstruct[ae->irasmstruct-1].size=ae->codeadr;
			ae->getstruct=0;

			/* SIZEOF like Vasm with struct name */
			curlabel.name=TxtStrDup(ae->rasmstruct[ae->irasmstruct-1].name);
			curlabel.crc=ae->rasmstruct[ae->irasmstruct-1].crc;
			curlabel.iw=-1;
			curlabel.ptr=ae->rasmstruct[ae->irasmstruct-1].size;
			PushLabelLight(ae,&curlabel);
			
			/* compute size for each field */
#if TRACE_STRUCT
	printf("compute field size\n");
#endif
			newlen=strlen(ae->rasmstruct[ae->irasmstruct-1].name)+2;
			if (ae->rasmstruct[ae->irasmstruct-1].irasmstructfield) {
				for (i=0;i<ae->rasmstruct[ae->irasmstruct-1].irasmstructfield-1;i++) {
					ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].size=ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i+1].offset-ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].offset;
					ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].fullname=MemMalloc(newlen+strlen(ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].name));
					sprintf(ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].fullname,"%s.%s",ae->rasmstruct[ae->irasmstruct-1].name,ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].name);
					ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].crc=GetCRC(ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].fullname);
				}
				ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].size=ae->rasmstruct[ae->irasmstruct-1].size-ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].offset;
				ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].fullname=MemMalloc(newlen+strlen(ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].name));
				sprintf(ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].fullname,"%s.%s",ae->rasmstruct[ae->irasmstruct-1].name,ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].name);
				ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].crc=GetCRC(ae->rasmstruct[ae->irasmstruct-1].rasmstructfield[i].fullname);
			} else {
				rasm_printf(ae,KWARNING"[%s:%d] Warning: empty structure [%s]\n",GetCurrentFile(ae),ae->wl[ae->idx].l,curlabel.name);
				if (ae->erronwarn) MaxError(ae);
			}

			/* unwrap data capture */
#if TRACE_STRUCT
	printf("unwrap data capture\n");
#endif
			if (ae->as80==1 || ae->pasmo) {/* not for UZ80 */
				instruction[ICRC_DEFB].makemnemo=_DEFB_as80;instruction[ICRC_DB].makemnemo=_DEFB_as80;
				instruction[ICRC_DEFW].makemnemo=_DEFW_as80;instruction[ICRC_DW].makemnemo=_DEFW_as80;
				instruction[ICRC_DEFI].makemnemo=_DEFI_as80;
			} else {
				instruction[ICRC_DEFB].makemnemo=_DEFB;instruction[ICRC_DB].makemnemo=_DEFB;
				instruction[ICRC_DEFW].makemnemo=_DEFW;instruction[ICRC_DW].makemnemo=_DEFW;
				instruction[ICRC_DEFI].makemnemo=_DEFI;
			}
			instruction[ICRC_DEFF].makemnemo=_DEFF;instruction[ICRC_DF].makemnemo=_DEFF;
			instruction[ICRC_DEFR].makemnemo=_DEFR;instruction[ICRC_DR].makemnemo=_DEFR;
			instruction[ICRC_DEFS].makemnemo=_DEFS;instruction[ICRC_DS].makemnemo=_DEFS;

			/* like there was no byte */
			ae->outputadr=ae->backup_outputadr;
			ae->codeadr=ae->backup_codeadr;

			___org_close(ae);
			___org_new(ae,0);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"ENDSTRUCT encountered outside STRUCT declaration\n");
		}
	}
}

void __MEMSPACE(struct s_assenv *ae) {
	if (ae->wl[ae->idx].t) {
		___new_memory_space(ae);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"MEMSPACE directive does not need parameter\n");
	}
}

int (*_internal_getsample)(unsigned char *data, int *idx);
#undef FUNC
#define FUNC "_internal_AudioGetSampleValue"

int __internal_getsample8(unsigned char *data, int *idx) {
	int v;
	v=data[*idx]-128;*idx=*idx+1;return v;
}
int __internal_getsample16little(unsigned char *data, int *idx) {
	int cursample;
	char *sdata=(char *)data;
	cursample=sdata[*idx+1];*idx=*idx+2;
	return cursample;
}
int __internal_getsample24little(unsigned char *data, int *idx) {
	int cursample;
	char *sdata=(char *)data;
	cursample=sdata[*idx+2];*idx=*idx+3;
	return cursample;
}
/* big-endian */
int __internal_getsample16big(unsigned char *data, int *idx) {
	int cursample;
	char *sdata=(char *)data;
	cursample=sdata[*idx];*idx=*idx+2;
	return cursample;
}
int __internal_getsample24big(unsigned char *data, int *idx) {
	int cursample;
	char *sdata=(char *)data;
	cursample=sdata[*idx];*idx=*idx+3;
	return cursample;
}
/* float & endian shit */
int _isLittleEndian() /* from lz4.h */
{
    const union { U32 u; unsigned char c[4]; } one = { 1 };
    return one.c[0];
}

unsigned char * __internal_floatinversion(unsigned char *data) {
	static unsigned char bswap[4];
	bswap[0]=data[3];
	bswap[1]=data[2];
	bswap[2]=data[1];
	bswap[3]=data[0];
	return bswap;
}

int __internal_getsample32bigbig(unsigned char *data, int *idx) {
	float fsample;
	int cursample;
	fsample=*((float*)(data+*idx));
	*idx=*idx+4;
	cursample=(floor)((fsample+1.0)*127.5+0.5);
	return cursample;
}
int __internal_getsample32biglittle(unsigned char *data, int *idx) {
	float fsample;
	int cursample;
	fsample=*((float*)(__internal_floatinversion(data+*idx)));
	*idx=*idx+4;
	cursample=(floor)((fsample+1.0)*127.5+0.5);
	return cursample;
}

#define __internal_getsample32littlelittle __internal_getsample32bigbig
#define __internal_getsample32littlebig __internal_getsample32biglittle


void _AudioLoadSample(struct s_assenv *ae, unsigned char *data, unsigned int filesize, enum e_audio_sample_type sample_type, float normalize)
{
	#undef FUNC
	#define FUNC "AudioLoadSample"

	struct s_wav_header *wav_header;
	int i,j,n,idx,controlsize;
	int nbchannel,bitspersample,nbsample;
	int bigendian=0,cursample,wFormat;
	double frequency;
	double accumulator;
	unsigned char samplevalue=0, sampleprevious=0;
	int samplerepeat=0,ipause,mypsgreg;

	unsigned char *subchunk;
	int subchunksize;

	if (filesize<sizeof(struct s_wav_header)) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - this file is too small to be a valid WAV!\n");
		return;
	}

	wav_header=(struct s_wav_header *)data;

#if TRACE_HEXBIN
printf("AudioLoadSample filesize=%d st=%d normalize=%.2lf\n",filesize,sample_type,normalize);
#endif
	if (strncmp(wav_header->ChunkID,"RIFF",4)) {
		if (strncmp(wav_header->ChunkID,"RIFX",4)) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - unsupported audio sample type (chunkid must be 'RIFF' or 'RIFX')\n");
			return;
		} else {
			bigendian=1;
		}
	}
	if (strncmp(wav_header->Format,"WAVE",4)) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"[%s:%d] WAV import - unsupported audio sample type (format must be 'WAVE')\n");
		return;
	}
	controlsize=wav_header->SubChunk1Size[0]+wav_header->SubChunk1Size[1]*256+wav_header->SubChunk1Size[2]*65536+wav_header->SubChunk1Size[3]*256*65536;
	if (controlsize!=16) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - invalid wav chunk size (subchunk1 control)\n");
		return;
	}
	if (strncmp(wav_header->SubChunk1ID,"fmt",3)) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - unsupported audio sample type (subchunk1id must be 'fmt')\n");
		return;
	}

#if TRACE_HEXBIN
printf("AudioLoadSample getsubchunk\n");
#endif
	subchunk=(unsigned char *)&wav_header->SubChunk2ID;
	while (strncmp((char *)subchunk,"data",4)) {
		subchunksize=8+subchunk[4]+subchunk[5]*256+subchunk[6]*65536+subchunk[7]*256*65536;
		if (subchunksize>=filesize) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - data subchunk not found\n");
			return;
		}
		subchunk+=subchunksize;
	}
	subchunksize=subchunk[4]+subchunk[5]*256+subchunk[6]*65536+subchunk[7]*256*65536;
	controlsize=subchunksize;

	nbchannel=wav_header->NumChannels[0]+wav_header->NumChannels[1]*256;
	if (nbchannel<1) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - invalid number of audio channel\n");
		return;
	}

	wFormat=wav_header->AudioFormat[0]+wav_header->AudioFormat[1]*256;
	if (wFormat!=1 && wFormat!=3) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - invalid or unsupported wFormatTag (%04X)\n",wFormat);
		return;
	}

	frequency=wav_header->SampleRate[0]+wav_header->SampleRate[1]*256+wav_header->SampleRate[2]*65536+wav_header->SampleRate[3]*256*65536;
	switch (sample_type) {
		case AUDIOSAMPLE_DMAA:
		case AUDIOSAMPLE_DMAB:
		case AUDIOSAMPLE_DMAC:
			if (fabs(frequency/15125.0-1.0)>0.2) {
				if (!ae->nowarning) {
					rasm_printf(ae,KWARNING"[%s:%d] Warning: WAV sample frequency (%dHz) is very different from 15KHz DMA frequency\n",GetCurrentFile(ae),ae->wl[ae->idx].l,(int)frequency);
					if (ae->erronwarn) MaxError(ae);
				}
			}
		default:break;
	}

	bitspersample=wav_header->BitsPerSample[0]+wav_header->BitsPerSample[1]*256;
#if TRACE_HEXBIN
printf("AudioLoadSample bitpersample=%d | Format=%s\n",bitspersample,wFormat==1?"PCM":"IEEE Float");
#endif
	switch (bitspersample) {
		case 8:_internal_getsample=__internal_getsample8;break;
		case 16:if (!bigendian) _internal_getsample=__internal_getsample16little; else _internal_getsample=__internal_getsample16big;break;
		case 24:if (!bigendian) _internal_getsample=__internal_getsample24little; else _internal_getsample=__internal_getsample24big;break;
		case 32:if (wFormat==3) {
				if (!bigendian) {
						if (_isLittleEndian()) {
							_internal_getsample=__internal_getsample32littlelittle;
						} else {
							_internal_getsample=__internal_getsample32littlebig;
						}
					} else {
						if (_isLittleEndian()) {
							_internal_getsample=__internal_getsample32biglittle;
						} else {
							_internal_getsample=__internal_getsample32bigbig;
						}
					}
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - unsupported 32bits PCM\n",wFormat);
				return;
			}
			break;
		default:
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - unsupported bits per sample (%d)\n",bitspersample);
			return;
	}

	nbsample=controlsize/nbchannel/(bitspersample/8);
	if (controlsize+sizeof(struct s_wav_header)>filesize) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"WAV import - cannot read %d byte%s of audio whereas the file is %d bytes big!\n",controlsize,controlsize>1?"s":"",filesize);
		return;
	}

#if TRACE_HEXBIN
printf("nbsample=%d (sze=%d,chn=%d,bps=%d) st=%d\n",nbsample,controlsize,nbchannel,bitspersample,sample_type);
#endif
	
	idx=subchunk-data;
	switch (sample_type) {
		default:
		case AUDIOSAMPLE_SMP:
			for (i=0;i<nbsample;i++) {
				/* downmixing */
				accumulator=0.0;
				for (n=0;n<nbchannel;n++) {
					accumulator+=_internal_getsample(data,&idx);
				}
				/* normalize */
				cursample=MinMaxInt(floor(((accumulator/nbchannel)*normalize)+0.5)+128,0,255);
				
				/* PSG levels */
				samplevalue=ae->psgfine[cursample];
				
				/* output */
				___output(ae,samplevalue);
			}
			break;
		case AUDIOSAMPLE_SM2:
			/* +1 pour éviter le segfault */
			for (i=0;i<nbsample+1;i+=2) {
				for (j=0;j<2;j++) {
					/* downmixing */
					accumulator=0.0;
					for (n=0;n<nbchannel;n++) {
						accumulator+=_internal_getsample(data,&idx);
					}
					/* normalize */
					cursample=MinMaxInt(floor(((accumulator/nbchannel)*normalize)+0.5)+128,0,255);
					/* PSG levels & packing */
					samplevalue=(samplevalue<<4)+(ae->psgfine[cursample]);
				}
				
				/* output */
				___output(ae,samplevalue);
			}
			break;
		case AUDIOSAMPLE_SM4:
			/***
				SM4 format has two bits
				bits -> PSG value
				  00 ->  0
				  01 -> 13
				  10 -> 14
				  11 -> 15				
			***/
			/* +3 pour éviter le segfault */
			for (i=0;i<nbsample+3;i+=4) {
				for (j=0;j<4;j++) {
					/* downmixing */
					accumulator=0.0;
					for (n=0;n<nbchannel;n++) {
						accumulator+=_internal_getsample(data,&idx);
					}
					/* normalize */
					cursample=MinMaxInt(floor(((accumulator/nbchannel)*normalize)+0.5)+128,0,255);
					/* PSG levels & packing */
					samplevalue=(samplevalue<<2)+(ae->psgtab[cursample]>>2);
				}
				/* output */
				___output(ae,samplevalue);
			}
			break;
		case AUDIOSAMPLE_DMAA:
		case AUDIOSAMPLE_DMAB:
		case AUDIOSAMPLE_DMAC:
			switch (sample_type) {
				case AUDIOSAMPLE_DMAA:mypsgreg=0x8;break;
				case AUDIOSAMPLE_DMAB:mypsgreg=0x9;break;
				case AUDIOSAMPLE_DMAC:mypsgreg=0xA;break;
				default:printf("warning remover\n");
			}
			/* downmixing */
			accumulator=0.0;
			for (n=0;n<nbchannel;n++) {
				accumulator+=_internal_getsample(data,&idx);
			}
			/* normalize */
			cursample=MinMaxInt(floor(((accumulator/nbchannel)*normalize)+0.5)+128,0,255);
			/* PSG levels */
			sampleprevious=ae->psgtab[cursample];
			for (i=1;i<nbsample;i++) {
				/* downmixing */
				accumulator=0.0;
				for (n=0;n<nbchannel;n++) {
					accumulator+=_internal_getsample(data,&idx);
				}
				/* normalize */
				cursample=MinMaxInt(floor(((accumulator/nbchannel)*normalize)+0.5)+128,0,255);
				
				/* PSG levels */
				samplevalue=ae->psgtab[cursample];
				
				if (samplevalue==sampleprevious && i+1<nbsample) {
					samplerepeat++;
				} else {
					if (!samplerepeat) {
						/* DMA output */
						___output(ae,sampleprevious);
						___output(ae,mypsgreg); /* volume canal A/B/C */
					} else {
						/* DMA pause */
						___output(ae,sampleprevious);
						___output(ae,mypsgreg); /* volume canal A/B/C */
						while (samplerepeat) {
							ipause=samplerepeat<4096?samplerepeat:4095;
							___output(ae,ipause&0xFF);
							___output(ae,0x10 | ((ipause>>8) &0xF)); /* pause */
							
							samplerepeat-=4096;
							if (samplerepeat<0) samplerepeat=0;
						}
					}
					sampleprevious=samplevalue;
				}
			}
			/* if last sample is alone */
			if (!samplerepeat) {
				___output(ae,sampleprevious);
				___output(ae,mypsgreg); /* volume canal A/B/C */
			}
			break;
	}
}


#ifdef NOAPULTRA
  int LZSA_crunch(unsigned char *input_data,int input_size,unsigned char **lzdata,int *lzlen) {
	  lzdata=MemMalloc(4);
	  *lzlen=0;

printf("no LZSA support in this version!\n");
fprintf(stderr,"no LZSA support in this version!\n");

	  return 0;
  }
  int APULTRA_crunch(unsigned char *input_data,int input_size,unsigned char **lzdata,int *lzlen) {
	  lzdata=MemMalloc(4);
	  *lzlen=0;

printf("no AP-Ultra support in this version!\n");
fprintf(stderr,"no AP-Ultra support in this version!\n");

	  return 0;
  }
#endif

/*
	meta fonction qui gère le INCBIN standard plus les variantes SMP et DMA
*/
void __READ(struct s_assenv *ae) {
	if (!ae->wl[ae->idx].t) {
		int idx;

		idx=atoi(ae->wl[ae->idx+1].w);
		ae->idx++;

		if (idx>=0 && idx<ae->ih) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"File to include was not found [%s]\n",ae->hexbin[idx].filename);
		} else {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"internal error with text file import (index out of bounds)\n");
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"READ directive need a proper filename as argument\n");
	}
}

void __HEXBIN(struct s_assenv *ae) {
	#undef FUNC
	#define FUNC "__HEXBIN"

	int hbinidx,overwritecheck=1,crc;
	struct s_expr_dico *rvar;
	unsigned int idx;
	int size=0,offset=0;
	float amplification=1.0;
	int deload=0;
	int vtiles=0,remap=0,revert=0;
	int itiles=0,tilex=0;
	struct s_hexbin *curhexbin;
	unsigned char *newdata=NULL;
	int fileok=0,incwav=0;
	
	if (!ae->wl[ae->idx].t) {
		ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,1);
		hbinidx=RoundComputeExpressionCore(ae,ae->wl[ae->idx+1].w,ae->codeadr,0);
		if (hbinidx>=ae->ih) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"internal error with binary file import (index out of bounds)\n");
			return;
		}
#if TRACE_HEXBIN
printf("Hexbin idx=[%s] filename=[%s]\n",ae->wl[ae->idx+1].w,ae->hexbin[hbinidx].filename);
#endif
		
		if (!ae->wl[ae->idx+1].t) {
			if (strcmp("DSK",ae->wl[ae->idx+2].w)==0) {
				/* import binary from DSK */
			} else if (strchr("SD",ae->wl[ae->idx+2].w[0]) && ae->wl[ae->idx+2].w[1]=='M' && strchr("P24A",ae->wl[ae->idx+2].w[2]) && !ae->wl[ae->idx+2].w[3]) {
				/* SMP,SM2,SM4,DMA */
#if TRACE_HEXBIN
printf("Hexbin for WAV-> %s (no operation until delayed load)\n",ae->wl[ae->idx+2].w);
#endif
				incwav=1;
			} else {
				/* legacy binary file */
#if TRACE_HEXBIN
printf("Hexbin legacy datalen=%d\n",ae->hexbin[hbinidx].datalen);
#endif
				if (strcmp("REVERT",ae->wl[ae->idx+2].w)==0) {
					/* revert data */
					if (!ae->wl[ae->idx+2].t) {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN REVERT does not need extra parameters\n");
					}
#if TRACE_HEXBIN
printf(" -> REVERT loading\n");
#endif
					revert=1;
					offset=size=0; // full file
					ae->idx++;

				} else if (strcmp("REMAP",ae->wl[ae->idx+2].w)==0) {
					/* reorder tiles data */
					if (!ae->wl[ae->idx+2].t) {
						ExpressionFastTranslate(ae,&ae->wl[ae->idx+3].w,1);
						remap=RoundComputeExpressionCore(ae,ae->wl[ae->idx+3].w,ae->codeadr,0);
					} else {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN REMAP need a number of columns for reordering\n");
					}
#if TRACE_HEXBIN
printf(" -> REMAP loading\n");
#endif
					offset=size=0; // full file
					ae->idx+=2;

				} else if (strcmp("ITILES",ae->wl[ae->idx+2].w)==0) {
					/*** entrelace les tiles, besoin de hauteur et largeur de la tile ***/
					if (!ae->wl[ae->idx+2].t) {
						ExpressionFastTranslate(ae,&ae->wl[ae->idx+3].w,1);
						tilex=RoundComputeExpressionCore(ae,ae->wl[ae->idx+3].w,ae->codeadr,0);
						itiles=1;
					} else {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"usage is INCBIN'file',ITILES,width\n");
						tilex=0;
					}
#if TRACE_HEXBIN
printf(" -> ITILES loading\n");
#endif
					offset=size=0; // full file
					ae->idx+=2;
				} else if (strcmp("VTILES",ae->wl[ae->idx+2].w)==0) {
					/* import and reorder tiles */
					if (!ae->wl[ae->idx+2].t) {
						ExpressionFastTranslate(ae,&ae->wl[ae->idx+3].w,1);
						vtiles=RoundComputeExpressionCore(ae,ae->wl[ae->idx+3].w,ae->codeadr,0);
					} else {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN VTILES need a number of lines for reordering\n");
					}
#if TRACE_HEXBIN
printf(" -> VTILES loading\n");
#endif
					offset=size=0; // full file
					ae->idx+=2;
				} else {
					char *expwrk;
					
					expwrk=TxtStrDup(ae->wl[ae->idx+2].w);
					ExpressionFastTranslate(ae,&expwrk,1);
					offset=RoundComputeExpressionCore(ae,expwrk,ae->codeadr,0);
					MemFree(expwrk);
#if TRACE_HEXBIN
	printf("offset=%d\n",offset);
#endif
					if (!ae->wl[ae->idx+2].t) {
						if (ae->wl[ae->idx+3].w[0]) {
							expwrk=TxtStrDup(ae->wl[ae->idx+3].w);
							ExpressionFastTranslate(ae,&expwrk,1);
							size=RoundComputeExpressionCore(ae,expwrk,ae->codeadr,0);
							MemFree(expwrk);
						} else {
							size=0;
						}
#if TRACE_HEXBIN
	printf("size=%d\n",size);
#endif
						if (size<-65535 || size>65536) {
							MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN invalid size\n");
						}
						if (!ae->wl[ae->idx+3].t) {
							if (ae->wl[ae->idx+4].w[0]) {
								expwrk=TxtStrDup(ae->wl[ae->idx+4].w);
								ExpressionFastTranslate(ae,&expwrk,1);
								offset+=65536*RoundComputeExpressionCore(ae,expwrk,ae->codeadr,0);
								MemFree(expwrk);
							}
							if (!ae->wl[ae->idx+4].t) {
								if (strcmp(ae->wl[ae->idx+5].w,"OFF")==0) {
									overwritecheck=0;
								} else if (strcmp(ae->wl[ae->idx+5].w,"ON")==0) {
									overwritecheck=1;
#if TRACE_HEXBIN
	printf("mode OVERWRITE\n");
#endif
								} else if (ae->wl[ae->idx+5].w[0]) {
									MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN invalid overwrite value. Must be 'OFF' or 'ON'\n");
								}
								if (!ae->wl[ae->idx+5].t) {
									/* copy raw len to a (new) variable */
									crc=GetCRC(ae->wl[ae->idx+6].w);
									if ((rvar=SearchDico(ae,ae->wl[ae->idx+6].w,crc))!=NULL) {
										rvar->v=ae->hexbin[hbinidx].rawlen;
									} else {
										/* mais ne peut être un label ou un alias */
										ExpressionSetDicoVar(ae,ae->wl[ae->idx+6].w,ae->hexbin[hbinidx].rawlen);
									}
									ae->idx+=6;
								} else {
									ae->idx+=5;
								}
							} else {
								ae->idx+=4;
							}
						} else {
							ae->idx+=3;
						}
					} else {
						ae->idx+=2;
					}
				}
			}
		} else {
			ae->idx++;
		}

		curhexbin=&ae->hexbin[hbinidx];

		/* preprocessor cannot manage variables so here is the delayed load */
		if (ae->hexbin[hbinidx].datalen<0) {
			char *newfilename;
			int lm,touched;
			
#if TRACE_HEXBIN
printf("Hexbin -> as only the assembler know how to deal with var,\n");
printf("we look for tags in the name of a file which were not found\n");
#endif
			
			newfilename=TxtStrDup(curhexbin->filename);

			/* need to upper case tags */
			for (lm=touched=0;newfilename[lm];lm++) {
				if (newfilename[lm]=='{') touched++; else if (newfilename[lm]=='}') touched--; else if (touched) newfilename[lm]=toupper(newfilename[lm]);
			}
			/* on essaie d'interpréter le nom du fichier en dynamique */
			newfilename=TranslateTag(ae,newfilename,&touched,1,E_TAGOPTION_REMOVESPACE);

			/* Where is the file to load? */
			if (!FileExists(newfilename)) {
				int ilookfile;
				char *filename_toread;

				/* on cherche dans les include */
				for (ilookfile=0;ilookfile<ae->ipath && !fileok;ilookfile++) {
					filename_toread=MergePath(ae,ae->includepath[ilookfile],newfilename);
					if (FileExists(filename_toread)) {
						fileok=1;
						MemFree(newfilename);
						newfilename=TxtStrDup(filename_toread); // Merge renvoie un static
					}
				}
			} else {
				fileok=1;
			}

			if (fileok) {
#if TRACE_HEXBIN
printf("Hexbin -> surprise! we found the file!\n");
#endif
				curhexbin->rawlen=curhexbin->datalen=FileGetSize(newfilename);
				curhexbin->data=MemMalloc(curhexbin->datalen*1.3+10);
				if (FileReadBinary(newfilename,(char*)curhexbin->data,curhexbin->datalen)!=curhexbin->datalen) {
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"read error on file [%s]\n",newfilename);
					MemFree(newfilename);
					return;
				}
				FileReadBinaryClose(newfilename);
				deload=1;
			} else {
				/* still not found */
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"file not found [%s]\n",newfilename);
				MemFree(newfilename);
				return;
			}
			MemFree(newfilename);
		} 

		if (incwav) {
			/* SMP,SM2,SM4,DMA */
			int dma_args=3;
			int dma_channel=AUDIOSAMPLE_DMAC;
			int dma_int=0,dma_repeat=0;
			int mypsgreg=0xA;
#if TRACE_HEXBIN
printf("Hexbin -> %s\n",ae->wl[ae->idx+2].w);
#endif
			if (!ae->wl[ae->idx+2].t) {
				amplification=ComputeExpressionCore(ae,ae->wl[ae->idx+3].w,ae->codeadr,0);
#if TRACE_HEXBIN
printf("sample amplification=%.2lf\n",amplification);
#endif
			}

			switch (ae->wl[ae->idx+2].w[2]) {
				case 'P':_AudioLoadSample(ae,ae->hexbin[hbinidx].data,ae->hexbin[hbinidx].datalen, AUDIOSAMPLE_SMP,amplification);break;
				case '2':_AudioLoadSample(ae,ae->hexbin[hbinidx].data,ae->hexbin[hbinidx].datalen, AUDIOSAMPLE_SM2,amplification);break;
				case '4':_AudioLoadSample(ae,ae->hexbin[hbinidx].data,ae->hexbin[hbinidx].datalen, AUDIOSAMPLE_SM4,amplification);break;
				case 'A':/* DMA options */
					if (!ae->wl[ae->idx+2].t) {
						while (!ae->wl[ae->idx+dma_args].t) {
							dma_args++;
							if (strcmp(ae->wl[ae->idx+dma_args].w,"DMA_INT")==0) {
								dma_int=1;
							} else if (strcmp(ae->wl[ae->idx+dma_args].w,"DMA_REPEAT")==0) {
								if (!ae->wl[ae->idx+dma_args].t) {
									dma_args++;
									dma_repeat=ComputeExpressionCore(ae,ae->wl[ae->idx+dma_args].w,ae->codeadr,0);
									if (dma_repeat<1 || dma_repeat>4095) {
										MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DMA_REPEAT value out of bounds (1-4095)\n");
										dma_repeat=0;
									}
								} else {
									MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"DMA_REPEAT must be followed by another parameter\n");
								}
							} else if (strcmp(ae->wl[ae->idx+dma_args].w,"DMA_CHANNEL_A")==0) {
								dma_channel=AUDIOSAMPLE_DMAA;
								mypsgreg=0x8;
							} else if (strcmp(ae->wl[ae->idx+dma_args].w,"DMA_CHANNEL_B")==0) {
								dma_channel=AUDIOSAMPLE_DMAB;
								mypsgreg=0x9;
							} else if (strcmp(ae->wl[ae->idx+dma_args].w,"DMA_CHANNEL_C")==0) {
								dma_channel=AUDIOSAMPLE_DMAC;
								mypsgreg=0xA;
							} else {
								MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Unrecognized DMA option [%s]\n",ae->wl[ae->idx+dma_args].w);
							}
						}
					}
					if (dma_repeat) {
						___output(ae,dma_repeat&0xFF);
						___output(ae,0x20|((dma_repeat>>8)&0xF));
					}
					_AudioLoadSample(ae,ae->hexbin[hbinidx].data,ae->hexbin[hbinidx].datalen, dma_channel,amplification);
					if (dma_repeat) {
						___output(ae,0x01);
						___output(ae,0x40); /* LOOP */
					}
					___output(ae,0);
					___output(ae,mypsgreg); /* volume to zero */
					if (dma_int) {
						___output(ae,0x10);
						___output(ae,0x40); /* INT */
					}
					___output(ae,0x20);
					___output(ae,0x40); /* Mandatory STOP */
					break;

				default:printf("warning remover\n");break;
			}
			ae->idx+=2;
			return;
		}
		
		if (ae->hexbin[hbinidx].datalen>0) {
			if (hbinidx<ae->ih && hbinidx>=0) {
				/* pre-parametres OK (longueur+IDX struct) */
				if (size<0) {
#if TRACE_HEXBIN
printf("taille négative %d -> conversion en %d\n",size,ae->hexbin[hbinidx].datalen+size);
#endif
					size=ae->hexbin[hbinidx].datalen+size;
					if (size<1) {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN negative size is greater or equal to filesize\n");
					}
				}
				/* negative offset conversion */
				if (offset<0) {
#if TRACE_HEXBIN
printf("offset négatif %d -> conversion en %d\n",offset,ae->hexbin[hbinidx].datalen+offset);
#endif
					offset=ae->hexbin[hbinidx].datalen+offset;
				}
				if (!size) {
					if (!offset) {
						size=ae->hexbin[hbinidx].datalen;
					} else {
						size=ae->hexbin[hbinidx].datalen-offset;
					}
#if TRACE_HEXBIN
printf("taille nulle et offset=%d -> conversion en %d\n",offset,size);
#endif
				}
				if (size>ae->hexbin[hbinidx].datalen) {
					rasm_printf(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN size is greater than filesize\n");
				} else {
					if (size+offset>ae->hexbin[hbinidx].datalen) {
						MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN size+offset is greater than filesize\n");
					} else {
						/* OUTPUT DATA */
						unsigned char *outputdata;
						int outputidx=0;
						outputdata=MemMalloc(ae->hexbin[hbinidx].datalen);
#if TRACE_HEXBIN
printf("output fictif pour réorganiser les données\n");
#endif

						if (revert) {
							int p;
							p=size-1;
							while (p>=0) {
								outputdata[outputidx++]=ae->hexbin[hbinidx].data[p--];
							}
						} else if (itiles) {
							/* tiles data reordering */
							int tx,it;

							if (size % (tilex*8)) {
								MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN ITILES cannot reorder tiles %d bytewidth with file of size %d\n",tilex,size);
							} else {
								it=0;
								while (it<size) {
									for (tx=0;tx<tilex;tx++)    outputdata[outputidx++]=ae->hexbin[hbinidx].data[it+tx+0*tilex];
									for (tx=tilex-1;tx>=0;tx--) outputdata[outputidx++]=ae->hexbin[hbinidx].data[it+tx+1*tilex];
									for (tx=0;tx<tilex;tx++)    outputdata[outputidx++]=ae->hexbin[hbinidx].data[it+tx+3*tilex];
									for (tx=tilex-1;tx>=0;tx--) outputdata[outputidx++]=ae->hexbin[hbinidx].data[it+tx+2*tilex];
									for (tx=0;tx<tilex;tx++)    outputdata[outputidx++]=ae->hexbin[hbinidx].data[it+tx+6*tilex];
									for (tx=tilex-1;tx>=0;tx--) outputdata[outputidx++]=ae->hexbin[hbinidx].data[it+tx+7*tilex];
									for (tx=0;tx<tilex;tx++)    outputdata[outputidx++]=ae->hexbin[hbinidx].data[it+tx+5*tilex];
									for (tx=tilex-1;tx>=0;tx--) outputdata[outputidx++]=ae->hexbin[hbinidx].data[it+tx+4*tilex];
									it+=tilex*8;
								}
							}
						} else if (remap) {
							/* tiles data reordering */
							int tx,it,width;

							width=size/remap;

							if ((size % remap) || (remap*width>size)) {
								MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN REMAP cannot reorder %d columns%s with file of size %d\n",remap,remap>1?"s":"",size);
							} else {
								for (it=0;it<remap;it++) {
									for (tx=0;tx<width;tx++) {
										outputdata[outputidx++]=ae->hexbin[hbinidx].data[it+tx*remap];
									}
								}
							}
							
						} else if (vtiles) {
							/* tiles map reordering */
							int width,tilex,tiley;

							width=size/vtiles;

							if ((size % vtiles) || (vtiles*width>size)) {
								MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INCBIN VTILES cannot reorder %d line%s with file of size %d\n",vtiles,vtiles>1?"s":"",size);
							} else {
#if TRACE_HEXBIN
printf("Hexbin -> re-tiling MAP! width=%d\n",width);
#endif
								for (idx=tilex=tiley=0;idx<size;idx++) {
									outputdata[outputidx++]=ae->hexbin[hbinidx].data[tilex+tiley*width];
									tiley++;
									if (tiley>=vtiles) {
										tiley=0;
										tilex++;
									}
								}
							}
						} else {
#if TRACE_HEXBIN
printf("Hexbin -> Legacy output from %d to %d\n",offset,size+offset);
if (curhexbin->crunch) printf("CRUNCHED! (%d)\n",curhexbin->crunch);
#endif
							/* only from offset to size+offset */
							for (idx=offset;idx<size+offset;idx++) {
								outputdata[outputidx++]=ae->hexbin[hbinidx].data[idx];
							}

							switch (curhexbin->crunch) {
								#ifndef NO_3RD_PARTIES
								case 4:
									newdata=LZ4_crunch(outputdata,outputidx,&outputidx);
									MemFree(outputdata);
									outputdata=newdata;
									#if TRACE_PREPRO
									rasm_printf(ae,KVERBOSE"crunched with LZ4 into %d byte(s)\n",outputidx);
									#endif
									break;
								case 7:
									{
									size_t slzlen;
									newdata=ZX7_compress(optimize(outputdata, outputidx), outputdata, outputidx, &slzlen);
									outputidx=slzlen;
									MemFree(outputdata);
									outputdata=newdata;
									#if TRACE_PREPRO
									rasm_printf(ae,KVERBOSE"crunched with ZX7 into %d byte(s)\n",outputidx);
									#endif
									}
									break;
								case 8:
									if (outputidx>=1024) rasm_printf(ae,KWARNING"Exomizer is crunching %.1fkb this may take a while, be patient...\n",outputidx/1024.0);
									newdata=Exomizer_crunch(outputdata,outputidx,&outputidx);
									MemFree(outputdata);
									outputdata=newdata;
									#if TRACE_PREPRO
									rasm_printf(ae,KVERBOSE"crunched with Exomizer into %d byte(s)\n",outputidx);
									#endif
									break;
								case 17:
									if (outputidx>=1024) rasm_printf(ae,KWARNING"AP-Ultra is crunching %.1fkb this may take a while, be patient...\n",outputidx/1024.0);
									{
									int nnewlen;
									APULTRA_crunch(outputdata,outputidx,&newdata,&nnewlen);
									outputidx=nnewlen;
									}
									MemFree(outputdata);
									outputdata=newdata;
									#if TRACE_PREPRO
									rasm_printf(ae,KVERBOSE"crunched with AP-Ultra into %d byte(s)\n",outputidx);
									#endif
									break;
								case 18:
									if (outputidx>=16384 && curhexbin->version==2) rasm_printf(ae,KWARNING"LZSA2 is crunching %.1fkb this may take a while, be patient...\n",outputidx/1024.0);
									{
									int nnewlen;
									LZSA_crunch(outputdata,outputidx,&newdata,&nnewlen,curhexbin->version,curhexbin->minmatch);
									outputidx=nnewlen;
									}
									MemFree(outputdata);
									outputdata=newdata;
									#if TRACE_PREPRO
									rasm_printf(ae,KVERBOSE"crunched with LZSA%d into %d byte(s)\n",curhexbin->version,outputidx);
									#endif
									break;
								#endif
								case 48:
									newdata=LZ48_crunch(outputdata,outputidx,&outputidx);
									MemFree(outputdata);
									outputdata=newdata;
									#if TRACE_PREPRO
									rasm_printf(ae,KVERBOSE"crunched with LZ48 into %d byte(s)\n",outputidx);
									#endif
									break;
								case 49:
									newdata=LZ49_crunch(outputdata,outputidx,&outputidx);
									MemFree(outputdata);
									outputdata=newdata;
									#if TRACE_PREPRO
									rasm_printf(ae,KVERBOSE"crunched with LZ49 into %d byte(s)\n",outputidx);
									#endif
									break;
								default:break;
							}


							if (!overwritecheck) {
								rasm_printf(ae,KWARNING"INCBIN without overwrite check still not working...\n");
								if (ae->erronwarn) MaxError(ae);
							}
						}

						if (overwritecheck) {
							for (idx=0;idx<outputidx;idx++) {
								___output(ae,outputdata[idx]);
							}
						} else {
							___org_close(ae);
							___org_new(ae,0);
							/* hack to disable overwrite check */
							for (idx=0;idx<outputidx;idx++) {
								___output(ae,outputdata[idx]);
							}
							ae->orgzone[ae->io-1].nocode=2;
							___org_close(ae);
							___org_new(ae,0);
						}

						MemFree(outputdata);
					}
				}
			} else {
				MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INTERNAL - HEXBIN refer to unknown structure\n");
				FreeAssenv(ae);
				exit(2);
			}
		}
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"INTERNAL - HEXBIN need one HEX parameter\n");
		FreeAssenv(ae);
		exit(2);
	}
	
	/* generated names must be reloaded! */
	if (deload) {
		ae->hexbin[hbinidx].datalen=-1;
		MemFree(ae->hexbin[hbinidx].data);
	}
}

/*
save "nom",start,size -> save binary
save "nom",start,size,TAPE,"cdtname" -> save tape file
save "nom",start,size,AMSDOS -> save binary with Amsdos header
save "nom",start,size,DSK,"dskname" -> save binary on DSK data format
save "nom",start,size,DSK,"dskname",B -> select face
save "nom",start,size,DSK,B -> current DSK, choose face
save "nom",start,size,DSK -> current DSK, current face
*/
void __SAVE(struct s_assenv *ae) {
	#undef FUNC
	#define FUNC "__SAVE"

	struct s_save cursave={0};
	int ko=1;




	if (!ae->wl[ae->idx].t) {
		/* nom de fichier entre quotes ou bien mot clef DSK */
		if (!StringIsQuote(ae->wl[ae->idx+1].w)) {
			MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SAVE invalid filename quote\n");
			ko=0;
		} else {
			if (!ae->wl[ae->idx+1].t) {
				if (!ae->wl[ae->idx+2].t && ae->wl[ae->idx+3].t!=2) {
					cursave.ibank=ae->activebank;
					cursave.ioffset=ae->idx+2;
					ExpressionFastTranslate(ae,&ae->wl[ae->idx+2].w,1); // si on utilise des variables ça évite la grouille post traitement...
					cursave.isize=ae->idx+3;
					ExpressionFastTranslate(ae,&ae->wl[ae->idx+3].w,1); // idem
					cursave.iw=ae->idx+1;
					cursave.irun=ae->current_run_idx;
					if (!ae->wl[ae->idx+3].t) {
						if (strcmp(ae->wl[ae->idx+4].w,"TAPE")==0) {
							cursave.tape=1;
							if (!ae->wl[ae->idx+4].t) {
								cursave.iwdskname=ae->idx+5;
							} else {
								cursave.iwdskname=-1;
								MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"cannot autoselect TAPE, please specify a filename after TAPE arg\n");
							}
						} else if (strcmp(ae->wl[ae->idx+4].w,"AMSDOS")==0) {
							cursave.amsdos=1;
						} else if (strcmp(ae->wl[ae->idx+4].w,"HOBETA")==0) {
							cursave.hobeta=1;
						} else if (strcmp(ae->wl[ae->idx+4].w,"DSK")==0) {
#if TRACE_EDSK
	printf("DSK SAVE order [bnk: %d ioff: %d isiz: %d iw=%d [%s] [%s]\n",cursave.ibank,cursave.ioffset,cursave.isize,cursave.iw,ae->wl[ae->idx+2].w,ae->wl[ae->idx+3].w);
#endif
							cursave.dsk=1;
							if (!ae->wl[ae->idx+4].t) {
								cursave.iwdskname=ae->idx+5;
								if (!ae->wl[ae->idx+5].t) {
									/* face selection - 0 as default */
									switch (ae->wl[ae->idx+6].w[0]) {
										case '1':
										case 'B':
											cursave.face=1;
											break;
										case '0':
										case 'A':
										default:
											cursave.face=0;
											break;
									}
								}
							} else {
								if (ae->nbsave && ae->save[ae->nbsave-1].iwdskname!=-1) {
									cursave.iwdskname=ae->save[ae->nbsave-1].iwdskname; /* previous DSK */
									cursave.face=ae->save[ae->nbsave-1].face; /* previous face */
								} else {
									cursave.iwdskname=-1;
									MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"cannot autoselect DSK as there was not a previous selection\n");
								}
							}
						} else {
							MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"SAVE 4th parameter must be empty or AMSDOS or DSK\n");
							ko=0;
						}
					}
					ObjectArrayAddDynamicValueConcat((void**)&ae->save,&ae->nbsave,&ae->maxsave,&cursave,sizeof(cursave));
					ko=0;
				}
			}
		}
	}
	if (ko) {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Use SAVE 'filename',offset,size[,AMSDOS|DSK[,A|B|'dskname'[,A|B]]]\n");
	}
	while (!ae->wl[ae->idx].t) ae->idx++;
}


void __MODULE(struct s_assenv *ae) {
	#undef FUNC
	#define FUNC "__MODULE"

	if (!ae->wl[ae->idx].t && ae->wl[ae->idx+1].t==1) {
		if (strcmp(ae->wl[ae->idx+1].w,"OFF")==0) {
			if (ae->module || ae->modulen) MemFree(ae->module);
			ae->module=NULL;
			ae->modulen=0;
		} else {
			if (ae->modulen || ae->module) {
				MemFree(ae->module);
			}
			ae->modulen=strlen(ae->wl[ae->idx+1].w);
			ae->module=TxtStrDup(ae->wl[ae->idx+1].w);
		}
		ae->idx++;
	} else {
		if (ae->module || ae->modulen) MemFree(ae->module);
		ae->module=NULL;
		ae->modulen=0;
	}
}

void __SUMMEM(struct s_assenv *ae) {
	struct s_poker poker={0};

	if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		/* no poke in a NOCODE section */
		if (!ae->nocode) {
			poker.method=E_POKER_SUM8;
			poker.istart=ae->idx+1;
			poker.iend=ae->idx+2;
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,1);
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+2].w,1);
			poker.outputadr=ae->outputadr;
			poker.ibank=ae->activebank;
			ObjectArrayAddDynamicValueConcat((void**)&ae->poker,&ae->nbpoker,&ae->maxpoker,&poker,sizeof(poker));
		}
		___output(ae,0);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"usage is SUMMEM start,end\n");
	}
}

void __XORMEM(struct s_assenv *ae) {
	struct s_poker poker={0};

	if (!ae->wl[ae->idx].t && !ae->wl[ae->idx+1].t && ae->wl[ae->idx+2].t==1) {
		/* no poke in a NOCODE section */
		if (!ae->nocode) {
			poker.method=E_POKER_XOR8;
			poker.istart=ae->idx+1;
			poker.iend=ae->idx+2;
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+1].w,1);
			ExpressionFastTranslate(ae,&ae->wl[ae->idx+2].w,1);
			poker.outputadr=ae->outputadr;
			poker.ibank=ae->activebank;
			ObjectArrayAddDynamicValueConcat((void**)&ae->poker,&ae->nbpoker,&ae->maxpoker,&poker,sizeof(poker));
		}
		___output(ae,0);
	} else {
		MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"usage is XORMEM start,end\n");
	}
}

void __TIMESTAMP(struct s_assenv *ae) {
	char Ltimestamp[32];
	char LTMP[64];
	struct tm *local;
	char *timestr;
	time_t now;
	int idx=0;
	int cpt;

	time(&now);
	local=localtime(&now);

	if (!ae->wl[ae->idx].t) {
		ae->idx++;
		timestr=ae->wl[ae->idx].w+1;
	} else {
		timestr=Ltimestamp;
		strcpy(timestr,"[Y-M-D h:m]");
	}

	while (timestr[idx]) {
		switch (timestr[idx]) {
			case 'Y':
				cpt=0;
				while (timestr[idx]=='Y') {idx++;cpt++;}
				sprintf(LTMP,"%04d",local->tm_year+1900);
				switch (cpt) {
					case 2:	___output(ae,LTMP[2]);
						___output(ae,LTMP[3]);
						break;
					case 1:
					case 4:	___output(ae,LTMP[0]);
						___output(ae,LTMP[1]);
						___output(ae,LTMP[2]);
						___output(ae,LTMP[3]);
						break;
				}
				break;
			case 'M':
				while (timestr[idx]=='M') idx++;
				sprintf(LTMP,"%02d",local->tm_mon+1);
				___output(ae,LTMP[0]);
				___output(ae,LTMP[1]);
				break;
			case 'D':
				while (timestr[idx]=='D') idx++;
				sprintf(LTMP,"%02d",local->tm_mday);
				___output(ae,LTMP[0]);
				___output(ae,LTMP[1]);
				break;
			case 'h':
				while (timestr[idx]=='h') idx++;
				sprintf(LTMP,"%02d",local->tm_hour);
				___output(ae,LTMP[0]);
				___output(ae,LTMP[1]);
				break;
			case 'm':
				while (timestr[idx]=='m') idx++;
				sprintf(LTMP,"%02d",local->tm_min);
				___output(ae,LTMP[0]);
				___output(ae,LTMP[1]);
				break;
			case 's':
				while (timestr[idx]=='s') idx++;
				sprintf(LTMP,"%02d",local->tm_sec);
				___output(ae,LTMP[0]);
				___output(ae,LTMP[1]);
				break;
			default:
				if ((timestr[idx]=='\'' || timestr[idx]=='"') && !timestr[idx+1]) {} else ___output(ae,timestr[idx]);
				idx++;
				break;

		}
	}
}

struct s_asm_keyword instruction[]={
{"LD",0,_LD},
{"DEC",0,_DEC},
{"INC",0,_INC},
{"ADD",0,_ADD},
{"SUB",0,_SUB},
{"OR",0,_OR},
{"AND",0,_AND},
{"XOR",0,_XOR},
{"POP",0,_POP},
{"PUSH",0,_PUSH},
{"DJNZ",0,_DJNZ},
{"JR",0,_JR},
{"JP",0,_JP},
{"CALL",0,_CALL},
{"RET",0,_RET},
{"EX",0,_EX},
{"ADC",0,_ADC},
{"SBC",0,_SBC},
{"EXA",0,_EXA},
{"EXX",0,_EXX},
{"CP",0,_CP},
{"BIT",0,_BIT},
{"RES",0,_RES},
{"SET",0,_SET},
{"IN",0,_IN},
{"OUT",0,_OUT},
{"RLC",0,_RLC},
{"RRC",0,_RRC},
{"RL",0,_RL},
{"RR",0,_RR},
{"SLA",0,_SLA},
{"SRA",0,_SRA},
{"SLL",0,_SLL},
{"SL1",0,_SLL},
{"SRL",0,_SRL},
{"RST",0,_RST},
{"HALT",0,_HALT},
{"DI",0,_DI},
{"EI",0,_EI},
{"NOP",0,_NOP},
{"DEFF",0,_DEFF},
{"DEFR",0,_DEFR},
{"DEFB",0,_DEFB},
{"DEFM",0,_DEFB},
{"DF",0,_DEFF},
{"DR",0,_DEFR},
{"DM",0,_DEFB},
{"DB",0,_DEFB},
{"DEFW",0,_DEFW},
{"DW",0,_DEFW},
{"DEFS",0,_DEFS},
{"DS",0,_DEFS},
{"STR",0,_STR},
{"LDI",0,_LDI},
{"LDIR",0,_LDIR},
{"OUTI",0,_OUTI},
{"INI",0,_INI},
{"RLCA",0,_RLCA},
{"RRCA",0,_RRCA},
{"NEG",0,_NEG},
{"RLA",0,_RLA},
{"RRA",0,_RRA},
{"RLD",0,_RLD},
{"RRD",0,_RRD},
{"DAA",0,_DAA},
{"CPL",0,_CPL},
{"SCF",0,_SCF},
{"LDD",0,_LDD},
{"LDDR",0,_LDDR},
{"CCF",0,_CCF},
{"OUTD",0,_OUTD},
{"IND",0,_IND},
{"RETI",0,_RETI},
{"RETN",0,_RETN},
{"IM",0,_IM},
{"DEFI",0,_DEFI},
{"CPD",0,_CPD},
{"CPI",0,_CPI},
{"CPDR",0,_CPDR},
{"CPIR",0,_CPIR},
{"OTDR",0,_OTDR},
{"OTIR",0,_OTIR},
{"INDR",0,_INDR},
{"INIR",0,_INIR},
{"REPEAT",0,__REPEAT},
{"REND",0,__REND},
{"ENDREPEAT",0,__REND},
{"ENDREP",0,__REND},
{"UNTIL",0,__UNTIL},
{"ORG",0,__ORG},
{"PROTECT",0,__PROTECT},
{"WHILE",0,__WHILE},
{"WEND",0,__WEND},
{"READ",0,__READ},
{"INCLUDE",0,__READ}, // anti-label
{"HEXBIN",0,__HEXBIN},
{"ALIGN",0,__ALIGN},
{"ELSEIF",0,__ELSEIF},
{"ELSE",0,__ELSE},
{"IF",0,__IF},
{"ENDIF",0,__ENDIF},
{"IFNOT",0,__IFNOT},
{"IFDEF",0,__IFDEF},
{"IFNDEF",0,__IFNDEF},
{"IFUSED",0,__IFUSED},
{"IFNUSED",0,__IFNUSED},
{"UNDEF",0,__UNDEF},
{"CASE",0,__CASE},
{"BREAK",0,__BREAK},
{"DEFAULT",0,__DEFAULT},
{"SWITCH",0,__SWITCH},
{"ENDSWITCH",0,__ENDSWITCH},
{"WRITE",0,__WRITE},
{"CODE",0,__CODE},
{"NOCODE",0,__NOCODE},
{"MEMSPACE",0,__MEMSPACE},
{"MACRO",0,__MACRO},
{"TICKER",0,__TICKER},
{"LET",0,__LET},
{"ASSERT",0,__ASSERT},
{"CHARSET",0,__CHARSET},
{"RUN",0,__RUN},
{"SAVE",0,__SAVE},
{"BRK",0,__BRK},
{"NOLIST",0,__NOLIST},
{"LIST",0,__LIST},
{"STOP",0,__STOP},
{"PRINT",0,__PRINT},
{"FAIL",0,__FAIL},
{"BREAKPOINT",0,__BREAKPOINT},
{"BANK",0,__BANK},
{"BANKSET",0,__BANKSET},
{"NAMEBANK",0,__NameBANK},
{"LIMIT",0,__LIMIT},
{"LZEXO",0,__LZEXO},
{"LZX7",0,__LZX7},
{"LZAPU",0,__LZAPU},
{"LZSA1",0,__LZSA1},
{"LZSA2",0,__LZSA2},
{"LZ4",0,__LZ4},
{"LZ48",0,__LZ48},
{"LZ49",0,__LZ49},
{"LZCLOSE",0,__LZCLOSE},
{"SNASET",0,__SNASET},
{"BUILDZX",0,__BUILDZX},
{"BUILDCPR",0,__BUILDCPR},
{"BUILDSNA",0,__BUILDSNA},
{"BUILDROM",0,__BUILDROM},
{"BUILDTAPE",0,__BUILDTAPE},
{"SETCPC",0,__SETCPC},
{"SETCRTC",0,__SETCRTC},
{"AMSDOS",0,__AMSDOS},
{"OTD",0,_OUTD},
{"OTI",0,_OUTI},
{"SHL",0,_SLA},
{"SHR",0,_SRL},
{"STRUCT",0,__STRUCT},
{"ENDSTRUCT",0,__ENDSTRUCT},
{"ENDS",0,__ENDSTRUCT},
{"NOEXPORT",0,__NOEXPORT},
{"ENOEXPORT",0,__ENOEXPORT},
{"MODULE",0,__MODULE},
{"TIMESTAMP",0,__TIMESTAMP},
{"SUMMEM",0,__SUMMEM},
{"XORMEM",0,__XORMEM},
{"",0,NULL}
};

int IsDirective(char *zeexpression)
{
	int i,crc;

	crc=GetCRC(zeexpression);

	for (i=0;instruction[i].mnemo[0];i++) if (instruction[i].crc==crc && !strcmp(instruction[i].mnemo,zeexpression)) return 1;

	return 0;
}


int Assemble(struct s_assenv *ae, unsigned char **dataout, int *lenout, struct s_rasm_info **debug)
{
	#undef FUNC
	#define FUNC "Assemble"

	unsigned char *AmsdosHeader;
	struct s_expression curexp={0};
	struct s_wordlist *wordlist;
	struct s_label *curlabel;
	int icrc,curcrc,i,j,k;
	unsigned char *lzdata=NULL;
	int lzlen,lzshift,input_size;
	size_t slzlen;
	unsigned char *input_data;
	struct s_orgzone orgzone={0};
	int iorgzone,ibank,offset,endoffset,morgzone,saveorgzone;
	int il,maxrom;
	char *TMP_filename=NULL;
	int minmem=65536,maxmem=0,lzmove;
	char symbol_line[1024];
	int ifast,executed;
	/* debug */
	int curii,inhibe;
	int ok;




	rasm_printf(ae,KAYGREEN"Assembling\n");
#if TRACE_ASSEMBLE
printf("assembling (nberr=%d)\n",ae->nberr);
#endif
#if TRACE_GENERALE
printf("*** assembling ***\n");
#endif

	ae->retdebug=debug;

	srand((unsigned)time(0));
	
	wordlist=ae->wl;
	ae->wl=wordlist;
	/* start outside crunched section */
	ae->lz=-1;
	
	/* default orgzone */
	orgzone.ibank=BANK_MAX_NUMBER;
	orgzone.inplace=1;
	ObjectArrayAddDynamicValueConcat((void**)&ae->orgzone,&ae->io,&ae->mo,&orgzone,sizeof(orgzone));
	___output=___internal_output;
	/* init des automates */
	InitAutomate(ae->AutomateHexa,(unsigned char *)AutomateHexaDefinition);
	InitAutomate(ae->AutomateDigit,(unsigned char *)AutomateDigitDefinition);
	InitAutomate(ae->AutomateValidLabel,(unsigned char *)AutomateValidLabelDefinition);
	InitAutomate(ae->AutomateValidLabelFirst,(unsigned char *)AutomateValidLabelFirstDefinition);
	InitAutomate(ae->AutomateExpressionValidCharExtended,(unsigned char *)AutomateExpressionValidCharExtendedDefinition);
	InitAutomate(ae->AutomateExpressionValidCharFirst,(unsigned char *)AutomateExpressionValidCharFirstDefinition);
	InitAutomate(ae->AutomateExpressionValidChar,(unsigned char *)AutomateExpressionValidCharDefinition);
	ae->AutomateExpressionDecision['<']='<';
	ae->AutomateExpressionDecision['>']='>';
	ae->AutomateExpressionDecision['=']='=';
	ae->AutomateExpressionDecision['!']='!';
	ae->AutomateExpressionDecision[0]='E';
	/* gestion d'alias */
	ae->AutomateExpressionDecision['~']='~';
	/* set operator precedence */
	if (!ae->maxam) {
		for (i=0;i<256;i++) {
			ae->AutomateElement[i].string=NULL;
			switch (i) {
				/* priority 0 */
				case '(':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_OPEN;ae->AutomateElement[i].priority=0;break;
				case ')':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_CLOSE;ae->AutomateElement[i].priority=0;break;
				/* priority 1 */
				case 'b':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_NOT;ae->AutomateElement[i].priority=1;break;
				/* priority 2 */
				case '*':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_MUL;ae->AutomateElement[i].priority=2;break;
				case '/':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_DIV;ae->AutomateElement[i].priority=2;break;
				case 'm':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_MOD;ae->AutomateElement[i].priority=2;break;
				/* priority 3 */
				case '+':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_ADD;ae->AutomateElement[i].priority=3;break;
				case '-':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_SUB;ae->AutomateElement[i].priority=3;break;
				/* priority 4 */
				case '[':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_SHL;ae->AutomateElement[i].priority=4;break;
				case ']':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_SHR;ae->AutomateElement[i].priority=4;break;
				/* priority 5 */
				case 'l':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_LOWER;ae->AutomateElement[i].priority=5;break;
				case 'g':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_GREATER;ae->AutomateElement[i].priority=5;break;
				case 'e':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_EQUAL;ae->AutomateElement[i].priority=5;break;
				case 'n':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_NOTEQUAL;ae->AutomateElement[i].priority=5;break;
				case 'k':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_LOWEREQ;ae->AutomateElement[i].priority=5;break;
				case 'h':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_GREATEREQ;ae->AutomateElement[i].priority=5;break;
				/* priority 6 */
				case '&':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_AND;ae->AutomateElement[i].priority=6;break;
				/* priority 7 */
				case '^':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_XOR;ae->AutomateElement[i].priority=7;break;
				/* priority 8 */
				case '|':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_OR;ae->AutomateElement[i].priority=8;break;
				/* priority 9 */
				case 'a':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_BAND;ae->AutomateElement[i].priority=9;break;
				/* priority 10 */
				case 'o':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_BOR;ae->AutomateElement[i].priority=10;break;
				default:ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_END;
			}
		}
	} else {
		for (i=0;i<256;i++) {
			ae->AutomateElement[i].string=NULL;
			switch (i) {
				/* priority 0 */
				case '(':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_OPEN;ae->AutomateElement[i].priority=0;break;
				case ')':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_CLOSE;ae->AutomateElement[i].priority=0;break;
				/* priority 0.5 */
				case 'b':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_NOT;ae->AutomateElement[i].priority=128;break;
				/* priority 1 */
				case '*':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_MUL;ae->AutomateElement[i].priority=464;break;
				case '/':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_DIV;ae->AutomateElement[i].priority=464;break;
				case 'm':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_MOD;ae->AutomateElement[i].priority=464;break;
				case '+':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_ADD;ae->AutomateElement[i].priority=464;break;
				case '-':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_SUB;ae->AutomateElement[i].priority=464;break;
				case '[':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_SHL;ae->AutomateElement[i].priority=464;break;
				case ']':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_SHR;ae->AutomateElement[i].priority=464;break;
				case '&':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_AND;ae->AutomateElement[i].priority=464;break;
				case '^':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_XOR;ae->AutomateElement[i].priority=464;break;
				case '|':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_OR;ae->AutomateElement[i].priority=464;break;
				/* priority 2 */
				case 'l':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_LOWER;ae->AutomateElement[i].priority=664;break;
				case 'g':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_GREATER;ae->AutomateElement[i].priority=664;break;
				case 'e':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_EQUAL;ae->AutomateElement[i].priority=664;break;
				case 'n':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_NOTEQUAL;ae->AutomateElement[i].priority=664;break;
				case 'k':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_LOWEREQ;ae->AutomateElement[i].priority=664;break;
				case 'h':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_GREATEREQ;ae->AutomateElement[i].priority=664;break;
				/* priority 3 */
				case 'a':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_BAND;ae->AutomateElement[i].priority=6128;break;
				case 'o':ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_BOR;ae->AutomateElement[i].priority=6128;break;
				default:ae->AutomateElement[i].operator=E_COMPUTE_OPERATION_END;
			}
		}
	}

	/* psg conversion */
	for (i=j=0;i<100;i++) ae->psgtab[j++]=0;
	for (i=0;i<49;i++) ae->psgtab[j++]=13;
	for (i=0;i<35;i++) ae->psgtab[j++]=14;
	for (i=0;i<72;i++) ae->psgtab[j++]=15;
	if (j!=256) {
		rasm_printf(ae,"Internal error with PSG conversion table\n");
		exit(-44);
	}
	for (i=j=0;i<1;i++) ae->psgfine[j++]=0;
	for (i=0;i<1;i++) ae->psgfine[j++]=1;
	for (i=0;i<1;i++) ae->psgfine[j++]=2;
	for (i=0;i<2;i++) ae->psgfine[j++]=3;
	for (i=0;i<2;i++) ae->psgfine[j++]=4;
	for (i=0;i<2;i++) ae->psgfine[j++]=5;
	for (i=0;i<3;i++) ae->psgfine[j++]=6;
	for (i=0;i<4;i++) ae->psgfine[j++]=7;
	for (i=0;i<7;i++) ae->psgfine[j++]=8;
	for (i=0;i<9;i++) ae->psgfine[j++]=9;
	for (i=0;i<13;i++) ae->psgfine[j++]=10;
	for (i=0;i<19;i++) ae->psgfine[j++]=11;
	for (i=0;i<27;i++) ae->psgfine[j++]=12;
	for (i=0;i<37;i++) ae->psgfine[j++]=13;
	for (i=0;i<53;i++) ae->psgfine[j++]=14;
	for (i=0;i<75;i++) ae->psgfine[j++]=15;
	if (j!=256) {
		rasm_printf(ae,"Internal error with PSG conversion table\n");
		exit(-44);
	}
	/* default var */
	ae->autorise_export=1;
	ExpressionSetDicoVar(ae,"PI",3.1415926545);
	ExpressionSetDicoVar(ae,"ASSEMBLER_RASM",1);
	
	/* add a fictive expression to simplify test when parsing expressions */
	ObjectArrayAddDynamicValueConcat((void **)&ae->expression,&ae->ie,&ae->me,&curexp,sizeof(curexp));
	
	/* compute CRC for keywords and directives */
	for (icrc=0;instruction[icrc].mnemo[0];icrc++) instruction[icrc].crc=GetCRC(instruction[icrc].mnemo);
	for (icrc=0;math_keyword[icrc].mnemo[0];icrc++) math_keyword[icrc].crc=GetCRC(math_keyword[icrc].mnemo);

	if (ae->as80==1 || ae->pasmo) { /* not for UZ80 */
		for (icrc=0;instruction[icrc].mnemo[0];icrc++) {
			if (strcmp(instruction[icrc].mnemo,"DEFB")==0 || strcmp(instruction[icrc].mnemo,"DB")==0) {
				instruction[icrc].makemnemo=_DEFB_as80;
			} else if (strcmp(instruction[icrc].mnemo,"DEFW")==0 || strcmp(instruction[icrc].mnemo,"DW")==0) {
				instruction[icrc].makemnemo=_DEFW_as80;
			} else if (strcmp(instruction[icrc].mnemo,"DEFI")==0) {
				instruction[icrc].makemnemo=_DEFI_as80;
			}
		}
	} else {
		/* for multiple configurations with rasm embedded */
		for (icrc=0;instruction[icrc].mnemo[0];icrc++) {
			if (strcmp(instruction[icrc].mnemo,"DEFB")==0 || strcmp(instruction[icrc].mnemo,"DB")==0) {
				instruction[icrc].makemnemo=_DEFB;
			} else if (strcmp(instruction[icrc].mnemo,"DEFW")==0 || strcmp(instruction[icrc].mnemo,"DW")==0) {
				instruction[icrc].makemnemo=_DEFW;
			} else if (strcmp(instruction[icrc].mnemo,"DEFI")==0) {
				instruction[icrc].makemnemo=_DEFI;
			}
		}
	}
	
	for (icrc=0;instruction[icrc].mnemo[0];icrc++) {
		/* get indexes for DEF instructions */
		if (strcmp(instruction[icrc].mnemo,"DEFB")==0) {
			ICRC_DEFB=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DB")==0) {
			ICRC_DB=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DEFW")==0) {
			ICRC_DEFW=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DW")==0) {
			ICRC_DW=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DEFF")==0) {
			ICRC_DEFF=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DF")==0) {
			ICRC_DF=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DEFR")==0) {
			ICRC_DEFR=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DR")==0) {
			ICRC_DR=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DEFS")==0) {
			ICRC_DEFS=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DS")==0) {
			ICRC_DS=icrc;
		} else if (strcmp(instruction[icrc].mnemo,"DEFI")==0) {
			ICRC_DEFI=icrc;
		}
	}
	
	/* Execution des mots clefs */
	/**********************************************************
	       A S S E M B L I N G    M A I N    L O O P
	**********************************************************/
#if TRACE_ASSEMBLE
printf("init ok\n");
#endif
#if TRACE_GENERALE
printf("-loop\n");
#endif
	
	ae->idx=1;
	while (wordlist[ae->idx].t!=2) {
		curcrc=GetCRC(wordlist[ae->idx].w);
		/*********************
		 d e b u g   i n f o
		*********************/
		#if TRACE_ASSEMBLE
		{
			int iiii=0;
			printf(KVERBOSE"%d [%s] L%d [%s]e=%d ",ae->idx,ae->filename[wordlist[ae->idx].ifile],wordlist[ae->idx].l,wordlist[ae->idx].w,wordlist[ae->idx].e);
			while (!wordlist[ae->idx+iiii++].t) rasm_printf(ae," [%s]e=%d ",wordlist[ae->idx+iiii].w,wordlist[ae->idx+iiii].e);
			
			for (iiii=0;iiii<ae->imacropos;iiii++) {
				printf("M[%d] s=%d e=%d ",iiii,ae->macropos[iiii].start,ae->macropos[iiii].end);
			}
			printf("\n");
		}
		#endif

		/********************************************************************
		  c o n d i t i o n n a l    a s s e m b l y    m a n a g e m e n t
		********************************************************************/
		if (ae->ii || ae->isw) {
			/* inhibition of if/endif */
			for (inhibe=curii=0;curii<ae->ii;curii++) {
				if (!ae->ifthen[curii].v || ae->ifthen[curii].v==-1) {
					inhibe=1;
					break;
				}
			}
			/* when inhibited we are looking only for a IF/IFDEF/IFNOT/IFNDEF/ELSE/ELSEIF/ENDIF or SWITCH/CASE/DEFAULT/ENDSWITCH */
			if (inhibe) {
				/* this section does NOT need to be agressively optimized !!! */
				if (curcrc==CRC_ELSEIF && strcmp(wordlist[ae->idx].w,"ELSEIF")==0) {
					/* true IF needs to be done ONLY on the active level */
					if (curii==ae->ii-1) __ELSEIF(ae); else __ELSEIF_light(ae);
				} else if (curcrc==CRC_ELSE && strcmp(wordlist[ae->idx].w,"ELSE")==0) {
					__ELSE(ae);
				} else if (curcrc==CRC_ENDIF && strcmp(wordlist[ae->idx].w,"ENDIF")==0) {
					__ENDIF(ae);
				} else if (curcrc==CRC_IF && strcmp(wordlist[ae->idx].w,"IF")==0) {
					/* as we are inhibited we do not have to truly compute IF */
					__IF_light(ae);
				} else if (curcrc==CRC_IFDEF && strcmp(wordlist[ae->idx].w,"IFDEF")==0) {
					__IFDEF_light(ae);
				} else if (curcrc==CRC_IFNOT && strcmp(wordlist[ae->idx].w,"IFNOT")==0) {
					__IFNOT_light(ae);
				} else if (curcrc==CRC_IFUSED && strcmp(wordlist[ae->idx].w,"IFUSED")==0) {
					__IFUSED_light(ae);
				} else if (curcrc==CRC_IFNUSED && strcmp(wordlist[ae->idx].w,"IFNUSED")==0) {
					__IFNUSED_light(ae);
				} else if (curcrc==CRC_IFNDEF && strcmp(wordlist[ae->idx].w,"IFNDEF")==0) {
					__IFNDEF_light(ae);
				} else if (curcrc==CRC_SWITCH && strcmp(wordlist[ae->idx].w,"SWITCH")==0) {
					__SWITCH_light(ae);
				} else if (curcrc==CRC_CASE && strcmp(wordlist[ae->idx].w,"CASE")==0) {
					__CASE_light(ae);
				} else if (curcrc==CRC_ENDSWITCH && strcmp(wordlist[ae->idx].w,"ENDSWITCH")==0) {
					__ENDSWITCH(ae);
				} else if (curcrc==CRC_BREAK && strcmp(wordlist[ae->idx].w,"BREAK")==0) {
					__BREAK_light(ae);
				} else if (curcrc==CRC_DEFAULT && strcmp(wordlist[ae->idx].w,"DEFAULT")==0) {
					__DEFAULT_light(ae);
				}
				while (wordlist[ae->idx].t==0) ae->idx++;
				ae->idx++;
				continue;
			} else {
				/* inhibition of switch/case */
				for (curii=0;curii<ae->isw;curii++) {
					if (!ae->switchcase[curii].execute) {
						inhibe=2;
						break;
					}
				}
				if (inhibe) {
					/* this section does NOT need to be agressively optimized !!! */
					if (curcrc==CRC_CASE && strcmp(wordlist[ae->idx].w,"CASE")==0) {
						__CASE(ae);
					} else if (curcrc==CRC_ENDSWITCH && strcmp(wordlist[ae->idx].w,"ENDSWITCH")==0) {
						__ENDSWITCH(ae);
					} else if (curcrc==CRC_IF && strcmp(wordlist[ae->idx].w,"IF")==0) {
						/* as we are inhibited we do not have to truly compute IF */
						__IF_light(ae);
					} else if (curcrc==CRC_IFDEF && strcmp(wordlist[ae->idx].w,"IFDEF")==0) {
						__IFDEF(ae);
					} else if (curcrc==CRC_IFNOT && strcmp(wordlist[ae->idx].w,"IFNOT")==0) {
						__IFNOT(ae);
					} else if (curcrc==CRC_ELSE && strcmp(wordlist[ae->idx].w,"ELSE")==0) {
						__ELSE(ae);
					} else if (curcrc==CRC_ENDIF && strcmp(wordlist[ae->idx].w,"ENDIF")==0) {
						__ENDIF(ae);
					} else if (curcrc==CRC_ELSEIF && strcmp(wordlist[ae->idx].w,"ELSEIF")==0) {
						__ELSEIF(ae);
					} else if (curcrc==CRC_IFUSED && strcmp(wordlist[ae->idx].w,"IFUSED")==0) {
						__IFUSED(ae);
					} else if (curcrc==CRC_IFNUSED && strcmp(wordlist[ae->idx].w,"IFNUSED")==0) {
						__IFNUSED(ae);
					} else if (curcrc==CRC_IFNDEF && strcmp(wordlist[ae->idx].w,"IFNDEF")==0) {
						__IFNDEF(ae);
					} else if (curcrc==CRC_SWITCH && strcmp(wordlist[ae->idx].w,"SWITCH")==0) {
						__SWITCH(ae);
					} else if (curcrc==CRC_BREAK && strcmp(wordlist[ae->idx].w,"BREAK")==0) {
						__BREAK(ae);
					} else if (curcrc==CRC_DEFAULT && strcmp(wordlist[ae->idx].w,"DEFAULT")==0) {
						__DEFAULT(ae);
					}
					while (wordlist[ae->idx].t==0) ae->idx++;
					ae->idx++;
					continue;
				}				
			}
		}
		/*****************************************
		  m a c r o   p o s i t i o n s
		*****************************************/
		if (ae->imacropos) {
			int icheckm;

			/*
			printf("===== Macro positions idx=%d =====\n",ae->idx);
			for (icheckm=0;icheckm<ae->imacropos;icheckm++) {
				printf("macro[%d] start=%3d end=%3d level=%d \n",icheckm,ae->macropos[icheckm].start,ae->macropos[icheckm].end,ae->macropos[icheckm].level);
			}
			printf("---------------------------\n");
			*/

			/* we must close */
			for (icheckm=0;icheckm<ae->imacropos;icheckm++) {
				if (ae->idx==ae->macropos[icheckm].end) {
					/* contiguous macro management... */
					if (ae->macropos[icheckm].pushed) {
						PopGlobal(ae);
						ae->macropos[icheckm].pushed=0;
					}
				}
			}
			/* before opening */
			for (icheckm=0;icheckm<ae->imacropos;icheckm++) {
				if (ae->idx==ae->macropos[icheckm].start) {
					PushGlobal(ae);
					ae->macropos[icheckm].pushed=1;
				}
			}

			/* are we still in a macro? */
			if (ae->idx>=ae->macropos[0].end) {
				/* are we out of all repetition blocks? */
				if (!ae->ir && !ae->iw) {
					ae->imacropos=0;
				}
			}
		}
		/*****************************************
		  e x e c u t e    i n s t r u c t i o n
		*****************************************/
		executed=0;
		if ((ifast=ae->fastmatch[(int)wordlist[ae->idx].w[0]])!=-1) {
			while (instruction[ifast].mnemo[0]==wordlist[ae->idx].w[0]) {
				if (instruction[ifast].crc==curcrc && strcmp(instruction[ifast].mnemo,wordlist[ae->idx].w)==0) {
#if TRACE_ASSEMBLE
printf("-> mnemo\n");
#endif
					instruction[ifast].makemnemo(ae);
					executed=1;
					break;
				}
				ifast++;
			}
		}
		/*****************************************
		       e x e c u t e    m a c r o
		*****************************************/
		if (!executed) {
			/* is it a macro? */
			if ((ifast=SearchMacro(ae,curcrc,wordlist[ae->idx].w))>=0) {
#if TRACE_ASSEMBLE
printf("-> macro\n");
#endif
				wordlist=__MACRO_EXECUTE(ae,ifast);
				continue;
			}
		}
		/*********************************************************************
		  e x e c u t e    e x p r e s s i o n   o r    p u s h    l a b e l
		*********************************************************************/
		if (!ae->stop) {
			if (!executed) {
				/* no instruction executed, this is a label or an assignement */
				if (wordlist[ae->idx].e) {
#if TRACE_ASSEMBLE
printf("-> expr\n");
#endif
					ExpressionFastTranslate(ae,&wordlist[ae->idx].w,0);
					ComputeExpression(ae,wordlist[ae->idx].w,ae->codeadr,0,0);
				} else {
#if TRACE_ASSEMBLE
printf("-> label\n");
#endif
					PushLabel(ae);
				}
			} else {
#if TRACE_ASSEMBLE
printf("-> ajuste IDX\n");
#endif
				while (!wordlist[ae->idx].t) {
					ae->idx++;
				}
			}
			ae->idx++; 
		} else {
#if TRACE_ASSEMBLE
printf("-> STOP\n");
#endif
			break;
		}
	}
#if TRACE_ASSEMBLE
	rasm_printf(ae,KVERBOSE"%d [%s] L%d [%s] fin de la liste de mots\n",ae->idx,ae->filename[wordlist[ae->idx].ifile],wordlist[ae->idx].l,wordlist[ae->idx].w);
	printf("check ORG\n");
#endif
#if TRACE_GENERALE
printf("-check ORG\n");
#endif

	if (!ae->stop) {
		/* end of assembly, check there is no opened struct */
		if (ae->getstruct) {
			MakeError(ae,ae->backup_filename,ae->backup_line,"STRUCT declaration was not closed\n");
		}
		/* end of assembly, close the last ORG zone */
		if (ae->io) {
			ae->orgzone[ae->io-1].memend=ae->outputadr;
		}
		OverWriteCheck(ae);
		/* end of assembly, close crunched zone (if any) */
		__internal_UpdateLZBlockIfAny(ae);
	
		/* end of assembly, check for opened repeat and opened while loop */
		for (i=0;i<ae->ir;i++) {
			MakeError(ae,ae->filename[wordlist[ae->repeat[i].start].ifile],wordlist[ae->repeat[i].start].l,"REPEAT was not closed\n");
		}
		for (i=0;i<ae->iw;i++) {
			MakeError(ae,ae->filename[wordlist[ae->whilewend[i].start].ifile],wordlist[ae->whilewend[i].start].l,"WHILE was not closed\n");
		}
		/* is there any IF opened? -> need an evolution for a better error message */
		for (i=0;i<ae->ii;i++) {
			char instr[32];
			switch (ae->ifthen[i].type) {
				case E_IFTHEN_TYPE_IF:strcpy(instr,"IF");break;
				case E_IFTHEN_TYPE_IFNOT:strcpy(instr,"IFNOT");break;
				case E_IFTHEN_TYPE_IFDEF:strcpy(instr,"IFDEF");break;
				case E_IFTHEN_TYPE_IFNDEF:strcpy(instr,"IFNDEF");break;
				case E_IFTHEN_TYPE_ELSE:strcpy(instr,"ELSE");break;
				case E_IFTHEN_TYPE_ELSEIF:strcpy(instr,"ELSEIF");break;
				case E_IFTHEN_TYPE_IFUSED:strcpy(instr,"IFUSED");break;
				case E_IFTHEN_TYPE_IFNUSED:strcpy(instr,"IFNUSED");break;
				default:strcpy(instr,"<unknown>");
			}
			MakeError(ae,ae->ifthen[i].filename,ae->ifthen[i].line,"%s conditionnal block was not closed\n",instr);
		}
	}
#if TRACE_ASSEMBLE
printf("crunch if any %d blocks\n",ae->ilz);
#endif
	/***************************************************
	         c r u n c h   L Z   s e c t i o n s
	***************************************************/
	if (!ae->stop || !ae->nberr) {
		for (i=0;i<ae->ilz;i++) {
			/* on dépile les symboles dans l'ordre mais on ne reloge pas sur les zones intermédiaires ou post-crunched */	
			if (ae->lzsection[i].lzversion!=0) {
#if TRACE_ASSEMBLE
printf("Crunch LZ[%d] (%d) %s\n",i,ae->lzsection[i].lzversion,ae->lzsection[i].lzversion==8?"mizou":"");
#endif

				if (ae->lzsection[i].memend==-1) {
					/* patch idx */
					ae->idx=ae->lzsection[i].iw;
					MakeError(ae,GetCurrentFile(ae),ae->wl[ae->idx].l,"Crunched section was not closed\n");
					continue;
				}

				/* compute labels and expression inside crunched blocks */
				PopAllExpression(ae,i);

				ae->curlz=i;
				iorgzone=ae->lzsection[i].iorgzone;
				ibank=ae->lzsection[i].ibank;
				input_data=&ae->mem[ae->lzsection[i].ibank][ae->lzsection[i].memstart];
				input_size=ae->lzsection[i].memend-ae->lzsection[i].memstart;
				if (!input_size) {
					rasm_printf(ae,KWARNING"[%s:%d] Warning: crunched section is empty\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
					if (ae->erronwarn) MaxError(ae);
				} else {
					switch (ae->lzsection[i].lzversion) {
						case 7:
							#ifndef NO_3RD_PARTIES
							lzdata=ZX7_compress(optimize(input_data, input_size), input_data, input_size, &slzlen);
							lzlen=slzlen;
							#endif
							break;
						case 4:
							#ifndef NO_3RD_PARTIES
							lzdata=LZ4_crunch(input_data,input_size,&lzlen);
							#endif
							break;
						case 8:
							#ifndef NO_3RD_PARTIES
							if (input_size>=1024) rasm_printf(ae,KWARNING"Exomizer is crunching %.1fkb this may take a while, be patient...\n",input_size/1024.0);
							lzdata=Exomizer_crunch(input_data,input_size,&lzlen);
							#endif
							break;
						case 17:
							#ifndef NO_3RD_PARTIES
							if (input_size>=1024) rasm_printf(ae,KWARNING"AP-Ultra is crunching %.1fkb this may take a while, be patient...\n",input_size/1024.0);
							APULTRA_crunch(input_data,input_size,&lzdata,&lzlen);
							#endif
							break;
						case 18:
#if TRACE_ASSEMBLE
							printf("crunching bank %d ptr=%d lng=%d version=%d minmatch=%d\n",ae->lzsection[i].ibank,ae->lzsection[i].memstart,input_size,ae->lzsection[i].version,ae->lzsection[i].minmatch);
#endif
							#ifndef NO_3RD_PARTIES
							if (input_size>=1024) rasm_printf(ae,KWARNING"LZSA is crunching %.1fkb this may take a while, be patient...\n",input_size/1024.0);
							LZSA_crunch(input_data,input_size,&lzdata,&lzlen,ae->lzsection[i].version,ae->lzsection[i].minmatch);
							#endif
							break;
						case 48:
							lzdata=LZ48_crunch(input_data,input_size,&lzlen);
							break;
						case 49:
							lzdata=LZ49_crunch(input_data,input_size,&lzlen);
							break;
						default:
							rasm_printf(ae,"Internal error - unknown crunch method %d\n",ae->lzsection[i].lzversion);
							exit(-12);
					}
				}

#if TRACE_ASSEMBLE
				printf("lzsection[%d] type=%d start=%04X end=%04X crunched size=%d\n",i,ae->lzsection[i].lzversion,ae->lzsection[i].memstart,ae->lzsection[i].memend,lzlen);
#endif

				if (input_size<lzlen) {
					//MakeError(ae,ae->filename[ae->wl[ae->lzsection[i].iw].ifile],ae->wl[ae->lzsection[i].iw].l,"As the LZ section cannot crunch data, Rasm may not guarantee assembled file!\n");
					rasm_printf(ae,KWARNING"Warning: LZ section is bigger than original\n");
					if (ae->erronwarn) MaxError(ae);
				}

				lzshift=lzlen-(ae->lzsection[i].memend-ae->lzsection[i].memstart);
#if TRACE_ASSEMBLE
printf("lzshift=%d\n",lzshift);
#endif

				/*******************************************************************************
				  r e l o c a t e   d a t a   u n t i l   n o n   c o n t i g u o u s   O R G
				*******************************************************************************/
				morgzone=iorgzone;
				if (lzshift>0) {
					/* positif => plus gros @@TODO */
					//MemMove(ae->mem[ae->lzsection[i].ibank]+ae->lzsection[i].memend+lzshift,ae->mem[ae->lzsection[i].ibank]+ae->lzsection[i].memend,65536-ae->lzsection[i].memend-lzshift);
				} else if (lzshift<0) {
					/* when crunched we may have to move more than 1 ORG */
					while (morgzone+1<ae->io) {
						/* if next ORG zone is contiguous */
						if (ae->orgzone[morgzone+1].memstart==ae->orgzone[morgzone].memend && ae->orgzone[morgzone+1].ibank==ae->orgzone[morgzone].ibank) {
							morgzone++;
						} else break;
					}
					lzmove=ae->orgzone[morgzone].memend-ae->lzsection[i].memend;
#if TRACE_ASSEMBLE
printf("include %d other ORG for the memove size=%d\n",morgzone-iorgzone,lzmove);
#endif
					if (lzmove) {
						MemMove(ae->mem[ae->lzsection[i].ibank]+ae->lzsection[i].memend+lzshift,ae->mem[ae->lzsection[i].ibank]+ae->lzsection[i].memend,lzmove);
					}
				}
				memcpy(ae->mem[ae->lzsection[i].ibank]+ae->lzsection[i].memstart,lzdata,lzlen);
				MemFree(lzdata);
				/*******************************************************************
				  l a b e l    a n d    e x p r e s s i o n    r e l o c a t i o n
				*******************************************************************/
				/* relocate labels in the same ORG zone AND contiguous ORG when they are "in place" */
				il=ae->lzsection[i].ilabel;
				saveorgzone=iorgzone;
				do {
					while (il<ae->il && ae->label[il].iorgzone<iorgzone) il++;

					while (il<ae->il && ae->label[il].iorgzone==iorgzone) {
						curlabel=SearchLabel(ae,ae->label[il].iw!=-1?wordlist[ae->label[il].iw].w:ae->label[il].name,ae->label[il].crc);
						/* CANNOT be NULL */
						curlabel->ptr+=lzshift;
#if TRACE_ASSEMBLE
	printf("label [%s] shifte de %d valeur #%04X -> #%04X\n",curlabel->iw!=-1?wordlist[curlabel->iw].w:curlabel->name,lzshift,curlabel->ptr-lzshift,curlabel->ptr);
#endif
						il++;
					}
					iorgzone++;
					/* jump over contiguous ORG not "in place" */
					while (iorgzone<ae->io && !ae->orgzone[iorgzone].inplace && iorgzone<=morgzone) {
						//printf("label reallocation jump over ORG %d\n",iorgzone);
						iorgzone++;
					}
				} while (iorgzone<=morgzone);

				/* relocate expressions in the same ORG zone AND contiguous ORG */
				iorgzone=saveorgzone;
				il=ae->lzsection[i].iexpr;
				do {
					while (il<ae->il && ae->label[il].iorgzone<iorgzone) il++;

					while (il<ae->ie && ae->expression[il].iorgzone==iorgzone) {
						ae->expression[il].wptr+=lzshift;
						/* relocate only "in place" contiguous ORG */
						if (ae->orgzone[iorgzone].inplace) ae->expression[il].ptr+=lzshift;
#if TRACE_ASSEMBLE
	printf("expression [%s] shiftee ptr(%s)=#%04X wptr=#%04X\n", ae->expression[il].reference?ae->expression[il].reference:wordlist[ae->expression[il].iw].w, ae->orgzone[iorgzone].inplace?"relocated":"untouched",ae->expression[il].ptr, ae->expression[il].wptr);
#endif
						il++;
					}
					iorgzone++;
				} while (iorgzone<=morgzone);

				/* relocate crunched sections in the same ORG zone AND contiguous ORG */
				iorgzone=saveorgzone;
				il=i+1;
				do {
					while (il<ae->ilz && ae->lzsection[il].iorgzone==iorgzone && ae->lzsection[il].ibank==ibank) {

#if TRACE_ASSEMBLE
	rasm_printf(ae,"reloger lzsection[%d] O%d B%d shift=%d\n",il,ae->lzsection[il].iorgzone,ae->lzsection[il].ibank,lzshift);
#endif
						ae->lzsection[il].memstart+=lzshift;
						ae->lzsection[il].memend+=lzshift;
						il++;
					}
					iorgzone++;
				} while (iorgzone<=morgzone);

				iorgzone=saveorgzone;
				/* relocate ORG zone(s) */
				ae->orgzone[iorgzone].memend+=lzshift;
				while (iorgzone<morgzone) {
					iorgzone++;
					ae->orgzone[iorgzone].memstart+=lzshift;
					ae->orgzone[iorgzone].memend+=lzshift;
				}
			}
		}
		for (i=0;i<ae->ilz;i++) {
			if (ae->lzsection[i].lzversion==0) {
#if TRACE_ASSEMBLE
//printf("PopAllExpr on intermediate section[%d] (%d) %s\n",i,ae->lzsection[i].lzversion,ae->lzsection[i].lzversion==8?"mizou":"");
#endif
				/* compute labels and expression outside crunched blocks BUT after crunched */
				PopAllExpression(ae,i);
			}
		}
		if (ae->ilz) {
			/* compute expression placed after the last crunched block */
			PopAllExpression(ae,ae->ilz);
		}
		/* compute expression outside crunched blocks */
		PopAllExpression(ae,-1);
	}	

	/*******************************************************************************
	      p o k e r  
	*******************************************************************************/
	for (i=0;i<ae->nbpoker;i++) {
		int istart=-1,iend=-1;
		unsigned char xorval,sumval;

		switch (ae->poker[i].method) {
			case E_POKER_XOR8:
				xorval=0;
				ae->idx=ae->poker[i].istart; /* exp hack */
				ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
				istart=RoundComputeExpression(ae,ae->wl[ae->idx].w,0,0,0);
				ae->idx=ae->poker[i].iend; /* exp hack */
				ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
				iend=RoundComputeExpression(ae,ae->wl[ae->idx].w,0,0,0);
				for (j=istart;j<iend;j++) {
					xorval=xorval^ae->mem[ae->poker[i].ibank][j];
				}
				ae->mem[ae->poker[i].ibank][ae->poker[i].outputadr]=xorval;
	//printf("poker XOR from #%04X to #%04X at #%04X\n",istart,iend,ae->poker[i].outputadr);
				break;
			case E_POKER_SUM8:
				sumval=0;
				ae->idx=ae->poker[i].istart; /* exp hack */
				ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
				istart=RoundComputeExpression(ae,ae->wl[ae->idx].w,0,0,0);
				ae->idx=ae->poker[i].iend; /* exp hack */
				ExpressionFastTranslate(ae,&ae->wl[ae->idx].w,0);
				iend=RoundComputeExpression(ae,ae->wl[ae->idx].w,0,0,0);
				for (j=istart;j<iend;j++) {
					sumval+=ae->mem[ae->poker[i].ibank][j];
				}
				ae->mem[ae->poker[i].ibank][ae->poker[i].outputadr]=sumval;
				break;
			default:printf("warning remover\n");break;
		}

		for (j=0;j<ae->nbpoker;j++) {
			if (ae->poker[i].ibank==ae->poker[j].ibank) {
				if (ae->poker[j].outputadr>=istart && ae->poker[j].outputadr<iend) {
					if (!ae->nowarning) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: %s result is inside another %s calculation",GetCurrentFile(ae),ae->wl[ae->idx].l,ae->poker[i].method==E_POKER_SUM8?"SUMMEM":"XORMEM",ae->poker[j].method==E_POKER_SUM8?"SUMMEM":"XORMEM");
						ae->idx=ae->poker[j].istart;
						rasm_printf(ae,KWARNING"[%s:%d]\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
						if (ae->erronwarn) MaxError(ae);
					}
				}
			}
		}

	}

/***************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************
      W R I T E      O U T P U T      F I L E S
****************************************************************************************************************************************************************************************
***************************************************************************************************************************************************************************************/
	TMP_filename=MemMalloc(PATH_MAX);
#if 0
for (i=0;i<ae->io;i++) {
printf("ORG[%02d] start=%04X end=%04X ibank=%d nocode=%d protect=%d\n",i,ae->orgzone[i].memstart,ae->orgzone[i].memend,ae->orgzone[i].ibank,ae->orgzone[i].nocode,ae->orgzone[i].protect);
}
#endif
#if TRACE_ASSEMBLE
printf("output files\n");
#endif

	if (!ae->nberr && !ae->checkmode) {
		
		/* enregistrement des fichiers programmes par la commande SAVE */
		PopAllSave(ae);
	
		if (ae->nbsave==0 || ae->forcecpr || ae->forcesnapshot || ae->forceROM || ae->forcetape) {
			/*********************************************
			**********************************************
						  C A R T R I D G E
			**********************************************
			*********************************************/
			if (ae->forcecpr) {
				char ChunkName[32];
				int ChunkSize;
				unsigned char chunk_endian;
				
				if (ae->cartridge_name) {
					sprintf(TMP_filename,"%s",ae->cartridge_name);
				} else {
					if (!ae->extendedCPR) sprintf(TMP_filename,"%s.cpr",ae->outputfilename);
					else sprintf(TMP_filename,"%s.xpr",ae->outputfilename);
				}
				FileRemoveIfExists(TMP_filename);
				
				rasm_printf(ae,KIO"Write %scartridge file %s\n",ae->extendedCPR?"extended ":"",TMP_filename);
				for (i=maxrom=0;i<ae->io;i++) {
					if (ae->orgzone[i].ibank<256 && ae->orgzone[i].ibank>maxrom) maxrom=ae->orgzone[i].ibank;
				}
				/* construction du CPR */
				/* header blablabla */
				strcpy(ChunkName,"RIFF");
				FileWriteBinary(TMP_filename,ChunkName,4);

				if (!ae->extendedCPR) {
					ChunkSize=(maxrom+1)*(16384+8)+4;
					chunk_endian=ChunkSize&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>8)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>16)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>24)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					sprintf(ChunkName,"AMS!");
					FileWriteBinary(TMP_filename,ChunkName,4);
				} else {
					for (i=0;i<=maxrom;i+=32) {
						char xproutputname[256];
						sprintf(xproutputname,"xpr%02d.rom",i>>5);
						FileRemoveIfExists(xproutputname);
					}

					ChunkSize=(maxrom+1)*(16384+8)+4+10;
					chunk_endian=ChunkSize&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>8)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>16)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>24)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					sprintf(ChunkName,"CXME");
					FileWriteBinary(TMP_filename,ChunkName,4);
					ChunkName[0]='N';
					ChunkName[1]='B';
					ChunkName[2]='B';
					ChunkName[3]='K';
					FileWriteBinary(TMP_filename,ChunkName,4);
					ChunkSize=2;
					chunk_endian=ChunkSize&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>8)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>16)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>24)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					ChunkName[0]=0;
					ChunkName[1]=32;
					FileWriteBinary(TMP_filename,ChunkName,2);
				}
				
//				for (j=0;j<ae->io;j++) {
//printf("ORG[%03d]=B%02d/#%04X/#%04X\n",j,ae->orgzone[j].ibank,ae->orgzone[j].memstart,ae->orgzone[j].memend);
//				}
				for (i=0;i<=maxrom;i++) {
					offset=65536;
					endoffset=0;
					for (j=0;j<ae->io;j++) {
						if (ae->orgzone[j].protect) continue; /* protected zones exclusion */
						/* bank data may start anywhere (typically #0000 or #C000) */
						if (ae->orgzone[j].ibank==i && ae->orgzone[j].memstart!=ae->orgzone[j].memend) {
							if (ae->orgzone[j].memstart<offset) offset=ae->orgzone[j].memstart;
							if (ae->orgzone[j].memend>endoffset) endoffset=ae->orgzone[j].memend;
						}
					}
					if (endoffset>offset) {
						int lm=0;
						if (ae->iwnamebank[i]>0) {
							lm=strlen(ae->wl[ae->iwnamebank[i]].w)-2;
						}
						rasm_printf(ae,KVERBOSE"WriteCPR bank %2d of %5d byte%s start at #%04X",i,endoffset-offset,endoffset-offset>1?"s":" ",offset);
						if (endoffset-offset>16384) {
							rasm_printf(ae,"\nROM is too big!!!\n");
							FileWriteBinaryClose(TMP_filename);
							FileRemoveIfExists(TMP_filename);
							FreeAssenv(ae);
							exit(ABORT_ERROR);
						}
						if (lm) {
							rasm_printf(ae," (%-*.*s)\n",lm,lm,ae->wl[ae->iwnamebank[i]].w+1);
						} else {
							rasm_printf(ae,"\n");
						}
					} else {
						rasm_printf(ae,KVERBOSE"WriteCPR bank %2d (empty)\n",i);
					}
					ChunkSize=16384;
					if (ae->extendedCPR) {
						ChunkName[0]='C';
						ChunkName[1]='X';
						ChunkName[2]=i>>8;
						ChunkName[3]=i&255;
					} else sprintf(ChunkName,"cb%02d",i);
					FileWriteBinary(TMP_filename,ChunkName,4);
					chunk_endian=ChunkSize&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>8)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>16)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					chunk_endian=(ChunkSize>>24)&0xFF;FileWriteBinary(TMP_filename,(char*)&chunk_endian,1);
					if (offset>0xC000) {
						unsigned char filler[16384]={0};
						ChunkSize=65536-offset;
						if (ChunkSize) FileWriteBinary(TMP_filename,(char*)ae->mem[i]+offset,ChunkSize);
						/* ADD zeros until the end of the bank */
						FileWriteBinary(TMP_filename,(char*)filler,16384-ChunkSize);
						if (ae->xpr) {
							char xproutputname[256];
							sprintf(xproutputname,"xpr%02d.rom",i>>5);
							if (ChunkSize) FileWriteBinary(xproutputname,(char*)ae->mem[i]+offset,ChunkSize);
							FileWriteBinary(xproutputname,(char*)filler,16384-ChunkSize);
						}
					} else {
						FileWriteBinary(TMP_filename,(char*)ae->mem[i]+offset,ChunkSize);
						if (ae->xpr) {
							char xproutputname[256];
							sprintf(xproutputname,"xpr%02d.rom",i>>5);
							FileWriteBinary(xproutputname,(char*)ae->mem[i]+offset,ChunkSize);
						}
					}
				}
				FileWriteBinaryClose(TMP_filename);
				rasm_printf(ae,"Total %d bank%s (%dK)\n",maxrom+1,maxrom+1>1?"s":"",(maxrom+1)*16);
			/*********************************************
			**********************************************
						       R O M       
			**********************************************
			*********************************************/
			} else if (ae->forceROM) {
				unsigned char filler[16384]={0};
				int noflood=0;

				/* how many ROM? */
				for (i=maxrom=0;i<ae->io;i++) {
					if (ae->orgzone[i].ibank<256 && ae->orgzone[i].ibank>maxrom) maxrom=ae->orgzone[i].ibank;
				}

				for (i=0;i<=maxrom;i++) {
					/* number the file */
					if (ae->rom_name) {
						sprintf(TMP_filename,"%s%d.rom",ae->rom_name,i);
					} else {
						sprintf(TMP_filename,"%s%d.rom",ae->outputfilename,i);
					}
					FileRemoveIfExists(TMP_filename);

					offset=65536;
					endoffset=0;
					for (j=0;j<ae->io;j++) {
						if (ae->orgzone[j].protect) continue; /* protected zones exclusion */
						/* bank data may start anywhere (typically #0000 or #C000) */
						if (ae->orgzone[j].ibank==i && ae->orgzone[j].memstart!=ae->orgzone[j].memend) {
							if (ae->orgzone[j].memstart<offset) offset=ae->orgzone[j].memstart;
							if (ae->orgzone[j].memend>endoffset) endoffset=ae->orgzone[j].memend;
						}
					}
					if (endoffset>offset) {
						/* cannot be bigger than 16K */
						if (endoffset-offset>16384) {
							rasm_printf(ae,"\nROM is too big!!!\n");
							FileRemoveIfExists(TMP_filename);
							FreeAssenv(ae);
							exit(ABORT_ERROR);
						}
						/* to avoid ROM smaller than 16K at the end of working memory */
						if (offset>49152) offset=49152;

						if (i<4 || i+4>maxrom) rasm_printf(ae,KVERBOSE"WriteROM bank %3d of %5d byte%s start at #%04X\n",i,endoffset-offset,endoffset-offset>1?"s":" ",offset);
						else if (!noflood) {rasm_printf(ae,KVERBOSE"[...]\n");noflood=1;}

						FileWriteBinary(TMP_filename,(char *)(ae->mem[i]+offset),endoffset-offset);
						if (endoffset-offset<16384) FileWriteBinary(TMP_filename,(char*)filler,16384-(endoffset-offset));
						FileWriteBinaryClose(TMP_filename);
					} else {
						rasm_printf(ae,KVERBOSE"WriteROM bank %3d is empty\n",i);
						//FileWriteBinary(TMP_filename,(char*)filler,16384);
						//FileWriteBinaryClose(TMP_filename);
					}


				}

				rasm_printf(ae,"Total %d rom%s (%dK)\n",maxrom+1,maxrom+1>1?"s":"",(maxrom+1)*16);
					
			/*********************************************
			**********************************************
						  S N A P S H O T  
			**********************************************
			*********************************************/
			} else if (ae->forcesnapshot) {
				
				if (ae->forcezx) {
					unsigned char zxsnapheader[0x1A]={0};

					if (ae->snapshot_name) {
						sprintf(TMP_filename,"%s",ae->snapshot_name);
					} else {
						sprintf(TMP_filename,"%s.sna",ae->outputfilename);
					}
					FileRemoveIfExists(TMP_filename);
					
					/* do we have a bankset? */
					/* zx bootstrap */
					zxsnapheader[0x13]=0; /* 0:DI 4:EI */
					zxsnapheader[0x17]=ae->zxsnapshot.stack&0xFF;
					zxsnapheader[0x18]=(ae->zxsnapshot.stack>>8)&0xFF;
					zxsnapheader[0x19]=1;  /* IM 1 */
					
					//ae->zxsnapshot.stack&=0xFFFF;
					ae->mem[0][ae->zxsnapshot.stack]=ae->zxsnapshot.run&0xFF;
					ae->mem[0][ae->zxsnapshot.stack+1]=(ae->zxsnapshot.run>>8)&0xFF;
					
					rasm_printf(ae,KIO"Write 48K ZX snapshot file %s\n",TMP_filename);
					
					/* header */
					FileWriteBinary(TMP_filename,(char *)&zxsnapheader,27);
					/* data */
					if (ae->bankset[0]) {
						FileWriteBinary(TMP_filename,(char *)ae->mem[0]+16384,16384*3);
					} else {
						FileWriteBinary(TMP_filename,(char *)ae->mem[5],16384);
						FileWriteBinary(TMP_filename,(char *)ae->mem[2],16384);
						FileWriteBinary(TMP_filename,(char *)ae->mem[0],16384);
					}
					FileWriteBinaryClose(TMP_filename);
				} else {
					unsigned char packed[65536]={0};
					unsigned char *rlebank=NULL;
					char ChunkName[16];
					int ChunkSize;
					int bankset;
					int noflood=0;

					if (ae->snapshot.version==2 && ae->snapshot.CPCType>2) {
						if (!ae->nowarning) {
							rasm_printf(ae,KWARNING"[%s:%d] Warning: V2 snapshot cannot select a Plus model (forced to 6128)\n",GetCurrentFile(ae),ae->wl[ae->idx].l);
							if (ae->erronwarn) MaxError(ae);
						}
						ae->snapshot.CPCType=2; /* 6128 */
					}
					
					if (ae->snapshot_name) {
						sprintf(TMP_filename,"%s",ae->snapshot_name);
					} else {
						sprintf(TMP_filename,"%s.sna",ae->outputfilename);
					}
					FileRemoveIfExists(TMP_filename);
				
					maxrom=-1;	
					for (i=0;i<ae->io;i++) {
						if (ae->orgzone[i].ibank<BANK_MAX_NUMBER && ae->orgzone[i].ibank>maxrom && ae->orgzone[i].memstart!=ae->orgzone[i].memend) {
							maxrom=ae->orgzone[i].ibank;
						}
					}

	//printf("maxrom=%d\n",maxrom);
					/* construction du SNA */
					if (ae->snapshot.version==2) {
						if (maxrom>=4) {
							ae->snapshot.dumpsize[0]=128;
						} else if (maxrom>=0) {
							ae->snapshot.dumpsize[0]=64;
						}
					}
					if (maxrom==-1) {
						rasm_printf(ae,KWARNING"Warning: No byte were written in snapshot memory\n");
						if (ae->erronwarn) MaxError(ae);
					} else {
						rasm_printf(ae,KIO"Write snapshot v%d file %s\n",ae->snapshot.version,TMP_filename);
						
						/* header */
						FileWriteBinary(TMP_filename,(char *)&ae->snapshot,0x100);
						/* write all memory crunched */
						for (i=0;i<=maxrom;i+=4) {
							bankset=i>>2;
							if (ae->bankset[bankset]) {
								memcpy(packed,ae->mem[i],65536);
								if (i<4 || i+4>maxrom) rasm_printf(ae,KVERBOSE"WriteSNA bank %2d,%d,%d,%d packed\n",i,i+1,i+2,i+3);
								else if (!noflood) {rasm_printf(ae,KVERBOSE"[...]\n");noflood=1;}
							} else {
								memset(packed,0,65536);
								for (k=0;k<4;k++) {
									offset=65536;
									endoffset=0;
									for (j=0;j<ae->io;j++) {
										if (ae->orgzone[j].protect) continue; /* protected zones exclusion */
										/* bank data may start anywhere (typically #0000 or #C000) */
										if (ae->orgzone[j].ibank==i+k && ae->orgzone[j].memstart!=ae->orgzone[j].memend) {
											if (ae->orgzone[j].memstart<offset) offset=ae->orgzone[j].memstart;
											if (ae->orgzone[j].memend>endoffset) endoffset=ae->orgzone[j].memend;
										}
									}
									if (endoffset-offset>16384) {
										rasm_printf(ae,KERROR"\nBANK is too big!!!\n");
										FileWriteBinaryClose(TMP_filename);
										FileRemoveIfExists(TMP_filename);
										FreeAssenv(ae);
										exit(ABORT_ERROR);
									}
									/* banks are gathered in the 64K block */
									if (offset>0xC000) {
										ChunkSize=65536-offset;
										memcpy(packed+k*16384,(char*)ae->mem[i+k]+offset,ChunkSize);
									} else {
										memcpy(packed+k*16384,(char*)ae->mem[i+k]+offset,16384);
									}
									
									if (endoffset>offset) {
										int lm=0;
										if (ae->iwnamebank[i]>0) {
											lm=strlen(ae->wl[ae->iwnamebank[i]].w)-2;
										}
										if (i<4 || i+4>maxrom) rasm_printf(ae,KVERBOSE"WriteSNA bank %2d of %5d byte%s start at #%04X",i+k,endoffset-offset,endoffset-offset>1?"s":" ",offset);
										else if (!noflood) {rasm_printf(ae,KVERBOSE"[...]\n");noflood=1;}
										if (endoffset-offset>16384) {
											rasm_printf(ae,KERROR"\nRAM block is too big!!!\n");
											FileWriteBinaryClose(TMP_filename);
											FileRemoveIfExists(TMP_filename);
											FreeAssenv(ae);
											exit(ABORT_ERROR);
										}
										if (lm) {
											if (i<4 || i+4>maxrom) rasm_printf(ae,KVERBOSE" (%-*.*s)\n",lm,lm,ae->wl[ae->iwnamebank[i+k]].w+1);
										} else {
											if (i<4 || i+4>maxrom) rasm_printf(ae,"\n");
										}
									} else {
										if (i<4 || i+4>maxrom) rasm_printf(ae,KVERBOSE"WriteSNA bank %2d (empty)\n",i+k);
										else if (!noflood) {rasm_printf(ae,KVERBOSE"[...]\n");noflood=1;}
									}
								}
							}
							
							if (ae->snapshot.version==2) {
								/* snapshot v2 */
								FileWriteBinary(TMP_filename,(char*)&packed,65536);
								if (bankset) {
									/* v2 snapshot is 128K maximum */
									maxrom=7;
									break;
								}
							} else {
								/* compression par défaut avec snapshot v3 */
								rlebank=EncodeSnapshotRLE(packed,&ChunkSize);
								
								if (bankset>=0 && bankset<=8) {
									sprintf(ChunkName,"MEM%d",bankset);
								} else if (bankset>8 && bankset<=0x40) {
									/* extended chunk for 4M extension -> MX09 to MX40 (hexa numbered) */
									sprintf(ChunkName,"MX%02X",bankset);
								} else {
									MakeError(ae,"(core)",0,"internal error during snapshot write, please report (%d)\n",bankset);
								}
								
								FileWriteBinary(TMP_filename,ChunkName,4);
								if (rlebank!=NULL) {
									FileWriteBinary(TMP_filename,(char*)&ChunkSize,4);
									FileWriteBinary(TMP_filename,(char*)rlebank,ChunkSize);
									MemFree(rlebank);
								} else {
									ChunkSize=65536;
									FileWriteBinary(TMP_filename,(char*)&packed,ChunkSize);
								}
							}
						}

						/**************************************************************
								snapshot additional chunks in v3+ only
						**************************************************************/
						if (ae->snapshot.version>=3) {
							/* export breakpoint */
							if (ae->export_snabrk) {
								/* BRKS chunk for Winape emulator (unofficial) 
								
								2 bytes - adress
								1 byte  - 0=base 64K / 1=extended
								2 bytes - condition (zeroed)
								*/
								struct s_breakpoint breakpoint={0};
								unsigned char *brkschunk=NULL;
								unsigned int idx=8;
								
								/* add labels and local labels to breakpoint pool (if any) */
								for (i=0;i<ae->il;i++) {
									if (!ae->label[i].name) {
										if (strstr(ae->wl[ae->label[i].iw].w,".BRK")!=NULL) {
											breakpoint.address=ae->label[i].ptr;
											if (ae->label[i].ibank>3) breakpoint.bank=1; else breakpoint.bank=0;
											ObjectArrayAddDynamicValueConcat((void **)&ae->breakpoint,&ae->ibreakpoint,&ae->maxbreakpoint,&breakpoint,sizeof(struct s_breakpoint));
										}
										if (strncmp(ae->wl[ae->label[i].iw].w,"BRK",3)==0) {
											breakpoint.address=ae->label[i].ptr;
											if (ae->label[i].ibank>3) breakpoint.bank=1; else breakpoint.bank=0;
											ObjectArrayAddDynamicValueConcat((void **)&ae->breakpoint,&ae->ibreakpoint,&ae->maxbreakpoint,&breakpoint,sizeof(struct s_breakpoint));
										}
									} else {
										if (strncmp(ae->label[i].name,"@BRK",4)==0 || strstr(ae->label[i].name,".BRK")) {
											breakpoint.address=ae->label[i].ptr;
											if (ae->label[i].ibank>3) breakpoint.bank=1; else breakpoint.bank=0;
											ObjectArrayAddDynamicValueConcat((void **)&ae->breakpoint,&ae->ibreakpoint,&ae->maxbreakpoint,&breakpoint,sizeof(struct s_breakpoint));								
										}
									}
								}

								brkschunk=MemMalloc(ae->ibreakpoint*5+8);
								strcpy((char *)brkschunk,"BRKS");
								
								for (i=0;i<ae->ibreakpoint;i++) {
									brkschunk[idx++]=ae->breakpoint[i].address&0xFF;
									brkschunk[idx++]=(ae->breakpoint[i].address&0xFF00)/256;
									brkschunk[idx++]=ae->breakpoint[i].bank;
									brkschunk[idx++]=0;
									brkschunk[idx++]=0;
								}

								idx-=8;
								brkschunk[4]=idx&0xFF;
								brkschunk[5]=(idx>>8)&0xFF;
								brkschunk[6]=(idx>>16)&0xFF;
								brkschunk[7]=(idx>>24)&0xFF;
								FileWriteBinary(TMP_filename,(char*)brkschunk,idx+8); // 8 bytes for the chunk header
								MemFree(brkschunk);


								/* BRKC chunk for ACE emulator 
								minimal integration
								*/
								brkschunk=MemMalloc(ae->ibreakpoint*256);
								strcpy((char *)brkschunk,"BRKC");
								idx=8;
								
								for (i=0;i<ae->ibreakpoint;i++) {
									brkschunk[idx++]=0; /* 0:Execution */
									brkschunk[idx++]=0;
									brkschunk[idx++]=0;
									brkschunk[idx++]=0;
									brkschunk[idx++]=ae->breakpoint[i].address&0xFF;
									brkschunk[idx++]=(ae->breakpoint[i].address&0xFF00)/256;
									for (j=0;j<2+1+1+2+4+128;j++) {
										brkschunk[idx++]=0;
									}
									sprintf((char *)brkschunk+idx,"breakpoint%d",i); /* breakpoint user name? */
									idx+=64+8;
								}
								idx-=8;
								brkschunk[4]=idx&0xFF;
								brkschunk[5]=(idx>>8)&0xFF;
								brkschunk[6]=(idx>>16)&0xFF;
								brkschunk[7]=(idx>>24)&0xFF;
								FileWriteBinary(TMP_filename,(char *)brkschunk,idx+8); // 8 bytes for the chunk header
								MemFree(brkschunk);
							}
							/* export optionnel des symboles */
							if (ae->export_sna) {
								/* SYMB chunk for ACE emulator

								1 byte  - name size
								n bytes - name (without 0 to end the string)
								6 bytes - reserved for future use
								2 bytes - shitty big endian adress for the symbol
								*/
							
								unsigned char *symbchunk=NULL;
								unsigned int idx=8;
								int symbol_len;

								symbchunk=MemMalloc(8+ae->il*(1+255+6+2));
								strcpy((char *)symbchunk,"SYMB");

								for (i=0;i<ae->il;i++) {
									if (!ae->label[i].name) {
										symbol_len=strlen(ae->wl[ae->label[i].iw].w);
										if (symbol_len>255) symbol_len=255;
										symbchunk[idx++]=symbol_len;
										memcpy(symbchunk+idx,ae->wl[ae->label[i].iw].w,symbol_len);
										idx+=symbol_len;
										memset(symbchunk+idx,0,6);
										idx+=6;
										symbchunk[idx++]=(ae->label[i].ptr&0xFF00)/256;
										symbchunk[idx++]=ae->label[i].ptr&0xFF;
									} else {
										if (ae->export_local || !ae->label[i].local) {
											symbol_len=strlen(ae->label[i].name);
											if (symbol_len>255) symbol_len=255;
											symbchunk[idx++]=symbol_len;
											memcpy(symbchunk+idx,ae->label[i].name,symbol_len);
											idx+=symbol_len;
											memset(symbchunk+idx,0,6);
											idx+=6;
											symbchunk[idx++]=(ae->label[i].ptr&0xFF00)/256;
											symbchunk[idx++]=ae->label[i].ptr&0xFF;
										}
									}
								}
								if (ae->export_var) {
									unsigned char *subchunk=NULL;
									int retidx=0;
									/* var are part of fast tree search structure */
									subchunk=SnapshotDicoTree(ae,&retidx);
									if (retidx) {
										symbchunk=MemRealloc(symbchunk,idx+retidx);
										memcpy(symbchunk+idx,subchunk,retidx);
										idx+=retidx;
										SnapshotDicoInsert("FREE",0,&retidx);
									}
								}
								if (ae->export_equ) {
									symbchunk=MemRealloc(symbchunk,idx+ae->ialias*(1+255+6+2));

									for (i=0;i<ae->ialias;i++) {
										int tmpptr;
										symbol_len=strlen(ae->alias[i].alias);
										if (symbol_len>255) symbol_len=255;
										symbchunk[idx++]=symbol_len;
										memcpy(symbchunk+idx,ae->alias[i].alias,symbol_len);
										idx+=symbol_len;
										memset(symbchunk+idx,0,6);
										idx+=6;
										tmpptr=RoundComputeExpression(ae,ae->alias[i].translation,0,0,0);
										symbchunk[idx++]=(tmpptr&0xFF00)/256;
										symbchunk[idx++]=tmpptr&0xFF;
									}
								}
								idx-=8;
								symbchunk[4]=idx&0xFF;
								symbchunk[5]=(idx>>8)&0xFF;
								symbchunk[6]=(idx>>16)&0xFF;
								symbchunk[7]=(idx>>24)&0xFF;
								FileWriteBinary(TMP_filename,(char*)symbchunk,idx+8); // 8 bytes for the chunk header
							}
						} else {
							if (ae->export_snabrk) {
								if (!ae->nowarning) {
									rasm_printf(ae,KWARNING"Warning: breakpoint export is not supported with snapshot version 2\n");
									if (ae->erronwarn) MaxError(ae);
								}
							}
							if (ae->export_sna) {
								if (!ae->nowarning) {
									rasm_printf(ae,KWARNING"Warning: symbol export is not supported with snapshot version 2\n");
									if (ae->erronwarn) MaxError(ae);
								}
							}
						}

						FileWriteBinaryClose(TMP_filename);
						maxrom=(maxrom>>2)*4+4;
						rasm_printf(ae,KAYGREEN"Total %d bank%s (%dK)\n",maxrom,maxrom>1?"s":"",(maxrom)*16);
					}
				}
			/*********************************************
			**********************************************
					  B I N A R Y     F I L E
			**********************************************
			*********************************************/
			} else {
				int lastspaceid=-1;
				
				if (ae->binary_name) {
					sprintf(TMP_filename,"%s",ae->binary_name);
				} else {
					sprintf(TMP_filename,"%s.bin",ae->outputfilename);
				}
				FileRemoveIfExists(TMP_filename);

				/* en mode binaire classique on va recherche le dernier espace mémoire dans lequel on a travaillé qui n'est pas en 'nocode' */
				for (i=0;i<ae->io;i++) {
					/* uniquement si le ORG a ete suivi d'ecriture */
					if (ae->orgzone[i].memstart!=ae->orgzone[i].memend && ae->orgzone[i].nocode!=1) {
						lastspaceid=ae->orgzone[i].ibank;
					}
				}
				if (lastspaceid!=-1) {
					for (i=0;i<ae->io;i++) {
						if (ae->orgzone[i].protect) continue; /* protected zones exclusion */
						/* uniquement si le ORG a ete suivi d'ecriture et n'est pas en 'nocode' */
						if (ae->orgzone[i].ibank==lastspaceid && ae->orgzone[i].memstart!=ae->orgzone[i].memend && ae->orgzone[i].nocode!=1) {
							if (ae->orgzone[i].memstart<minmem) minmem=ae->orgzone[i].memstart;
							if (ae->orgzone[i].memend>maxmem) maxmem=ae->orgzone[i].memend;
						}
					}
				}
				if (maxmem-minmem<=0) {
					if (!ae->stop) {
						if (!ae->nowarning) {
							rasm_printf(ae,KWARNING"Warning: Not a single byte to output\n");
							if (ae->erronwarn) MaxError(ae);
						}
					}
					if (ae->flux) {
						*lenout=0;
					}
				} else {
					if (!ae->flux) {
						/***************************************************************
						*      T A P E    o u t p u t                                  *
						***************************************************************/
						if (ae->forcetape) {
							int run;
							if (ae->tape_name) {
								sprintf(TMP_filename,"%s",ae->tape_name);
							} else {
								sprintf(TMP_filename,"%s.cdt",ae->outputfilename);
							}
							run=ae->snapshot.registers.LPC+(ae->snapshot.registers.HPC<<8);
							if (run<0x100) run=minmem;
							__output_CDT(ae,TMP_filename,"TAPE.BIN",(char*)ae->mem[lastspaceid]+minmem,maxmem-minmem,minmem,run);
						} else {
						/***************************************************************
						*      F I L E    o u t p u t                                  *
						***************************************************************/
							rasm_printf(ae,KIO"Write binary file %s (%d byte%s)\n",TMP_filename,maxmem-minmem,maxmem-minmem>1?"s":"");
							if (ae->amsdos) {
								AmsdosHeader=MakeAMSDOSHeader(minmem,minmem,maxmem,TMP_filename); //@@TODO
								FileWriteBinary(TMP_filename,(char *)AmsdosHeader,128);
							}
							if (maxmem-minmem>0) {
								FileWriteBinary(TMP_filename,(char*)ae->mem[lastspaceid]+minmem,maxmem-minmem);
								FileWriteBinaryClose(TMP_filename);
							} else {
								if (ae->amsdos) {
									FileWriteBinaryClose(TMP_filename);
								}
							}
						}
					} else {
						*dataout=MemMalloc(maxmem-minmem+1);
						memcpy(*dataout,ae->mem[lastspaceid]+minmem,maxmem-minmem);
						*lenout=maxmem-minmem;
					}
				}
			}
		}
		/********************************
		*********************************
		  U N U S E D    W A R N I N G
		*********************************
		********************************/
		if (ae->warn_unused) {
			for (i=0;i<ae->ialias;i++) {
				if (strcmp(ae->alias[i].alias,"IX") && strcmp(ae->alias[i].alias,"IY")) {
					if (!ae->alias[i].used) {
						rasm_printf(ae,KWARNING"[%s:%d] Warning: alias %s declared but not used\n",ae->filename[ae->wl[ae->alias[i].iw].ifile],ae->wl[ae->alias[i].iw].l,ae->alias[i].alias);
						if (ae->erronwarn) MaxError(ae);
					}
				}
			}
			WarnLabelTree(ae);
			WarnDicoTree(ae);
		}

		/****************************
		*****************************
		  S Y M B O L   E X P O R T
		*****************************
		****************************/
		if (ae->export_sym && !ae->export_sna) {
			char *SymbolFileName;
			SymbolFileName=MemMalloc(PATH_MAX);

#define MAKE_SYMBOL_NAME if (ae->symbol_name) {sprintf(TMP_filename,"%s",ae->symbol_name);} else {sprintf(TMP_filename,"%s.sym",ae->outputfilename);}

			MAKE_SYMBOL_NAME
			FileRemoveIfExists(TMP_filename);
			
			if (ae->export_multisym) {
				/* multi-remove before writes */
				for (i=0;i<ae->nbbank;i++) {
					if (ae->symbol_name) {
						sprintf(TMP_filename,"%s.bank%d",ae->symbol_name,i);
					} else {
						sprintf(TMP_filename,"%s.sym.bank%d",ae->outputfilename,i);
					}
					FileRemoveIfExists(TMP_filename);
				}
				rasm_printf(ae,KIO"Write symbol files %s.bank*\n",TMP_filename);
			} else {
				rasm_printf(ae,KIO"Write symbol file %s\n",TMP_filename);
			}
			
			switch (ae->export_sym) {
				case 5:
					/* ZX export */
					for (i=0;i<ae->il;i++) {
						if (ae->label[i].autorise_export) {
							if (ae->export_multisym) {
								if (ae->symbol_name) {
									sprintf(TMP_filename,"%s.bank%d",ae->symbol_name,ae->label[i].ibank);
								} else {
									sprintf(TMP_filename,"%s.sym.bank%d",ae->outputfilename,ae->label[i].ibank);
								}
							}
							
							if (!ae->label[i].name) {
								sprintf(symbol_line,"%d:%04X %s\n",ae->label[i].ibank,ae->label[i].ptr,ae->wl[ae->label[i].iw].w);
								FileWriteLine(TMP_filename,symbol_line);
							} else {
								if (ae->export_local || !ae->label[i].local) {
									sprintf(symbol_line,"%d:%04X %s\n",ae->label[i].ibank,ae->label[i].ptr,ae->label[i].name);
									FileWriteLine(TMP_filename,symbol_line);
								}
							}
						}
					}
					FileWriteLineClose(TMP_filename);
					MAKE_SYMBOL_NAME
					if (ae->export_var) {
						/* var are part of fast tree search structure */
						ExportDicoTree(ae,TMP_filename,"%s %04X");
					}
					if (ae->export_equ) {
						for (i=0;i<ae->ialias;i++) {
							if (strcmp(ae->alias[i].alias,"IX") && strcmp(ae->alias[i].alias,"IY")) {
								sprintf(symbol_line,"%04X %s\n",RoundComputeExpression(ae,ae->alias[i].translation,0,-ae->alias[i].iw,0),ae->alias[i].alias);
								FileWriteLine(TMP_filename,symbol_line);
							}
						}
					}
					break;
				case 4:
					/* flexible */
					for (i=0;i<ae->il;i++) {
						if (ae->label[i].autorise_export) {
							if (ae->export_multisym) {
								if (ae->symbol_name) {
									sprintf(TMP_filename,"%s.bank%d",ae->symbol_name,ae->label[i].ibank);
								} else {
									sprintf(TMP_filename,"%s.sym.bank%d",ae->outputfilename,ae->label[i].ibank);
								}
							}

							if (!ae->label[i].name) {
								sprintf(symbol_line,ae->flexible_export,ae->wl[ae->label[i].iw].w,ae->label[i].ptr);
								FileWriteLine(TMP_filename,symbol_line);
							} else {
								if (ae->export_local || !ae->label[i].local) {
									sprintf(symbol_line,ae->flexible_export,ae->label[i].name,ae->label[i].ptr);
									FileWriteLine(TMP_filename,symbol_line);
								}
							}
						}
					}
					MAKE_SYMBOL_NAME
					if (ae->export_var) {
						/* var are part of fast tree search structure */
						ExportDicoTree(ae,TMP_filename,ae->flexible_export);
					}
					if (ae->export_equ) {
						for (i=0;i<ae->ialias;i++) {
							if (strcmp(ae->alias[i].alias,"IX") && strcmp(ae->alias[i].alias,"IY")) {
								sprintf(symbol_line,ae->flexible_export,ae->alias[i].alias,RoundComputeExpression(ae,ae->alias[i].translation,0,-ae->alias[i].iw,0));
								FileWriteLine(TMP_filename,symbol_line);
							}
						}
					}
					FileWriteLineClose(TMP_filename);
					break;
				case 3:
					/* Winape */
					for (i=0;i<ae->il;i++) {
						if (ae->label[i].autorise_export) {
							if (ae->export_multisym) {
								if (ae->symbol_name) {
									sprintf(TMP_filename,"%s.bank%d",ae->symbol_name,ae->label[i].ibank);
								} else {
									sprintf(TMP_filename,"%s.sym.bank%d",ae->outputfilename,ae->label[i].ibank);
								}
							}
							if (!ae->label[i].name) {
								sprintf(symbol_line,"%s #%04X\n",ae->wl[ae->label[i].iw].w,ae->label[i].ptr);
								FileWriteLine(TMP_filename,symbol_line);
							} else {
								if (ae->export_local || !ae->label[i].local) {
									sprintf(symbol_line,"%s #%04X\n",ae->label[i].name,ae->label[i].ptr);
									FileWriteLine(TMP_filename,symbol_line);
								}
							}
						}
					}
					MAKE_SYMBOL_NAME
					if (ae->export_var) {
						/* var are part of fast tree search structure */
						ExportDicoTree(ae,TMP_filename,"%s #%04X\n");
					}
					if (ae->export_equ) {
						for (i=0;i<ae->ialias;i++) {
							if (strcmp(ae->alias[i].alias,"IX") && strcmp(ae->alias[i].alias,"IY")) {
								sprintf(symbol_line,"%s #%04X\n",ae->alias[i].alias,RoundComputeExpression(ae,ae->alias[i].translation,0,-ae->alias[i].iw,0));
								FileWriteLine(TMP_filename,symbol_line);
							}
						}
					}
					FileWriteLineClose(TMP_filename);
					break;
				case 2:
					/* pasmo */
					for (i=0;i<ae->il;i++) {
						if (ae->label[i].autorise_export) {
							if (ae->export_multisym) {
								if (ae->symbol_name) {
									sprintf(TMP_filename,"%s.bank%d",ae->symbol_name,ae->label[i].ibank);
								} else {
									sprintf(TMP_filename,"%s.sym.bank%d",ae->outputfilename,ae->label[i].ibank);
								}
							}
							if (!ae->label[i].name) {
								sprintf(symbol_line,"%s EQU 0%04XH\n",ae->wl[ae->label[i].iw].w,ae->label[i].ptr);
								FileWriteLine(TMP_filename,symbol_line);
							} else {
								if (ae->export_local || !ae->label[i].local) {
									sprintf(symbol_line,"%s EQU 0%04XH\n",ae->label[i].name,ae->label[i].ptr);
									FileWriteLine(TMP_filename,symbol_line);
								}
							}
						}
					}
					MAKE_SYMBOL_NAME
					if (ae->export_var) {
						/* var are part of fast tree search structure */
						ExportDicoTree(ae,TMP_filename,"%s EQU 0%04XH\n");
					}
					if (ae->export_equ) {
						for (i=0;i<ae->ialias;i++) {
							if (strcmp(ae->alias[i].alias,"IX") && strcmp(ae->alias[i].alias,"IY")) {
								sprintf(symbol_line,"%s EQU 0%04XH\n",ae->alias[i].alias,RoundComputeExpression(ae,ae->alias[i].translation,0,-ae->alias[i].iw,0));
								FileWriteLine(TMP_filename,symbol_line);
							}
						}
					}
					FileWriteLineClose(TMP_filename);
					break;
				case 1:
					/* Rasm */
					for (i=0;i<ae->il;i++) {
						if (ae->label[i].autorise_export) {
							if (ae->export_multisym) {
								if (ae->symbol_name) {
									sprintf(TMP_filename,"%s.bank%d",ae->symbol_name,ae->label[i].ibank);
								} else {
									sprintf(TMP_filename,"%s.sym.bank%d",ae->outputfilename,ae->label[i].ibank);
								}
							}
							if (!ae->label[i].name) {
								sprintf(symbol_line,"%s #%X B%d\n",ae->wl[ae->label[i].iw].w,ae->label[i].ptr,ae->label[i].ibank>31?0:ae->label[i].ibank);
								FileWriteLine(TMP_filename,symbol_line);
							} else {
								if (ae->export_local) {
									sprintf(symbol_line,"%s #%X B%d\n",ae->label[i].name,ae->label[i].ptr,ae->label[i].ibank>31?0:ae->label[i].ibank);
									FileWriteLine(TMP_filename,symbol_line);
								}
							}
						}
					}
					MAKE_SYMBOL_NAME
					if (ae->export_var) {
						/* var are part of fast tree search structure */
						ExportDicoTree(ae,TMP_filename,"%s #%X B0\n");
					}
					if (ae->export_equ) {
						for (i=0;i<ae->ialias;i++) {
							if (strcmp(ae->alias[i].alias,"IX") && strcmp(ae->alias[i].alias,"IY") && ae->alias[i].autorise_export) {
								sprintf(symbol_line,"%s #%X B0\n",ae->alias[i].alias,RoundComputeExpression(ae,ae->alias[i].translation,0,-ae->alias[i].iw,0));
								FileWriteLine(TMP_filename,symbol_line);
							}
						}
					}
					FileWriteLineClose(TMP_filename);
					break;
				case 0:
				default:break;	
			}
			MemFree(SymbolFileName);
		}
		/*********************************
		**********************************
			   B R E A K P O I N T S
		**********************************
		*********************************/
		if (ae->export_brk) {
			struct s_breakpoint breakpoint={0};
			
			if (ae->breakpoint_name) {
				sprintf(TMP_filename,"%s",ae->breakpoint_name);
			} else {
				sprintf(TMP_filename,"%s.brk",ae->outputfilename);
			}		
			FileRemoveIfExists(TMP_filename);

			/* add labels and local labels to breakpoint pool (if any) */
			for (i=0;i<ae->il;i++) {
				if (!ae->label[i].name) {
					if (strncmp(ae->wl[ae->label[i].iw].w,"BRK",3)==0) {
						breakpoint.address=ae->label[i].ptr;
						if (ae->label[i].ibank>3) breakpoint.bank=1; else breakpoint.bank=0;
						ObjectArrayAddDynamicValueConcat((void **)&ae->breakpoint,&ae->ibreakpoint,&ae->maxbreakpoint,&breakpoint,sizeof(struct s_breakpoint));
					}
				} else {
					if (strncmp(ae->label[i].name,"@BRK",4)==0) {
						breakpoint.address=ae->label[i].ptr;
						if (ae->label[i].ibank>3) breakpoint.bank=1; else breakpoint.bank=0;
						ObjectArrayAddDynamicValueConcat((void **)&ae->breakpoint,&ae->ibreakpoint,&ae->maxbreakpoint,&breakpoint,sizeof(struct s_breakpoint));								
					}
				}
			}

			if (ae->ibreakpoint) {
				rasm_printf(ae,KIO"Write breakpoint file %s\n",TMP_filename);
				for (i=0;i<ae->ibreakpoint;i++) {
					sprintf(symbol_line,"#%04X\n",ae->breakpoint[i].address);
					FileWriteLine(TMP_filename,symbol_line);
				}
				FileWriteLineClose(TMP_filename);
			} else {
				if (!ae->nowarning) {
					rasm_printf(ae,KWARNING"Warning: no breakpoint to output\n",TMP_filename);
					if (ae->erronwarn) MaxError(ae);
					/* empty file */
					FileTruncate(TMP_filename);
				}
			}
		}

	} else {
		if (!ae->dependencies) rasm_printf(ae,KERROR"%d error%s\n",ae->nberr,ae->nberr>1?"s":"");
	}
#if TRACE_ASSEMBLE
printf("dependencies\n");
#endif
/*******************************************************************************************
                        E X P O R T     D E P E N D E N C I E S
*******************************************************************************************/
	if (ae->dependencies) {
		int trigdep=0;
		
		/* depends ALL */
		if (ae->outputfilename && strcmp(ae->outputfilename,"rasmoutput")) {
			trigdep=1;
			printf("%s",ae->outputfilename);
			if (ae->dependencies==E_DEPENDENCIES_MAKE) printf(" "); else printf("\n");
		}
		for (i=1;i<ae->ifile;i++) {
			trigdep=1;
			SimplifyPath(ae->filename[i]);
			printf("%s",ae->filename[i]);
			if (ae->dependencies==E_DEPENDENCIES_MAKE) printf(" "); else printf("\n");
		}
		for (i=0;i<ae->ih;i++) {
			trigdep=1;
			SimplifyPath(ae->hexbin[i].filename);
			printf("%s",ae->hexbin[i].filename);
			if (ae->dependencies==E_DEPENDENCIES_MAKE) printf(" "); else printf("\n");
		}
		if (ae->dependencies==E_DEPENDENCIES_MAKE && trigdep) printf("\n");
	}

/*******************************************************************************************
                           V E R B O S E     S H I T
*******************************************************************************************/
	if (ae->display_stats) {
		int parsed_size=0;
		rasm_printf(ae,KVERBOSE"------ statistics ------------------\n");
		rasm_printf(ae,KVERBOSE"%d file%s\n",ae->ifile,ae->ifile>1?"s":"");
		rasm_printf(ae,KVERBOSE"%d binary include%s\n",ae->ih,ae->ih>1?"s":"");
		rasm_printf(ae,KVERBOSE"%d word%s\n",ae->nbword-1,ae->nbword>2?"s":"");
		rasm_printf(ae,KVERBOSE"%d label%s\n",ae->il,ae->il>1?"s":"");
		rasm_printf(ae,KVERBOSE"%d struct%s\n",ae->irasmstruct,ae->irasmstruct>1?"s":"");
		rasm_printf(ae,KVERBOSE"%d var%s\n",ae->idic,ae->idic>1?"s":"");
		rasm_printf(ae,KVERBOSE"%d expression%s\n",ae->ie,ae->ie>1?"s":"");
		rasm_printf(ae,KVERBOSE"%d macro%s\n",ae->imacro,ae->imacro>1?"s":"");
		rasm_printf(ae,KVERBOSE"%d alias%s\n",ae->ialias,ae->ialias>1?"s":"");
		rasm_printf(ae,KVERBOSE"%d ORG zone%s\n",ae->io-1,ae->io>2?"s":"");
		rasm_printf(ae,KVERBOSE"%d virtual space%s\n",ae->nbbank,ae->nbbank>1?"s":"");
		rasm_printf(ae,KVERBOSE"\n");
		for (i=0;i<ae->nbword;i++) parsed_size+=strlen(ae->wl[i].w);
		rasm_printf(ae,KVERBOSE"%d byte%s of pure code\n",parsed_size,parsed_size>1?"s":"");
	}
/*******************************************************************************************
                                  C L E A N U P
*******************************************************************************************/
#if TRACE_ASSEMBLE
printf("cleanup\n");
#endif
#if TRACE_GENERALE
printf("-cleanup\n");
#endif
	if (TMP_filename) MemFree(TMP_filename);
	if (ae->nberr) {
		ok=-1;
		if (ae->flux && *dataout) {
			MemFree(*dataout);
			*dataout=NULL;
		}
		if (lenout) *lenout=0;
	} else {
		ok=0;
	}

	FreeAssenv(ae);
#if TRACE_ASSEMBLE
printf("end of assembling\n");
printf("-end ok=%d\n",ok);
#endif
#if TRACE_GENERALE
printf("-end ok=%d\n",ok);
#endif
	return ok;
}


void EarlyPrepSrc(struct s_assenv *ae, char **listing, char *filename) {
	int l,idx,c,quote_type=0;
	int mlc_start,mlc_idx;

	/* virer les commentaires en ;, // mais aussi multi-lignes et convertir les decalages, passer les chars en upper case */
	l=idx=0;
	while (listing[l]) {
		c=listing[l][idx++];

		if (!c) {
			l++;
			idx=0;
			continue;
		} else {
			if (!quote_type) {

				/* upper case */
				if (c>='a' && c<='z') {
					listing[l][idx-1]=c=c-'a'+'A';
				}

				if (c=='\'' && idx>2 && strncmp(&listing[l][idx-3],"AF'",3)==0) {
					/* il ne faut rien faire */
				} else if (c=='"' || c=='\'') {
					quote_type=c;
				} else if (c==';' || (c=='/' && listing[l][idx]=='/')) {
					idx--;
					while (listing[l][idx] && listing[l][idx]!=0x0D && listing[l][idx]!=0x0A) listing[l][idx++]=':';
					idx--;
				} else if (c=='>' && listing[l][idx]=='>' && !quote_type) {
					listing[l][idx-1]=']';
					listing[l][idx++]=' ';
					continue;
				} else if (c=='<' && listing[l][idx]=='<' && !quote_type) {
					listing[l][idx-1]='[';
					listing[l][idx++]=' ';
					continue;
				} else if (c=='/' && listing[l][idx]=='*' && !quote_type) {
					/* multi-line comment */
					mlc_start=l;
					mlc_idx=idx-1;
					idx++;
					while (1) {
						c=listing[l][idx++];
						if (!c) {
							idx=0;
							l++;
							if (!listing[l]) {
								MakeError(ae,filename,l+1,"opened comment to the end of the file\n");
								return;
							}
						} else if (c=='*' && listing[l][idx]=='/') {
							idx++;
							break;
						}
					}
					/* merge */
					if (mlc_start==l) {
						/* on the same line */
						while (mlc_idx<idx) listing[l][mlc_idx++]=' '; /* raz with spaces */
					} else {
						/* multi-line */
						listing[mlc_start][mlc_idx]=0; /* raz EOL */
						mlc_start++;
						while (mlc_start<l) listing[mlc_start++][0]=0; /* raz line */
						mlc_idx=0;
						while (mlc_idx<idx) listing[l][mlc_idx++]=' '; /* raz beginning of the line */
					}
				}
			} else {
				/* in quote */
				if (c=='\\') {
					if (listing[l][idx]) {
						idx++;
					}
				} else if (c==quote_type) {
					quote_type=0;
				}
			}
		}
	}
	l-=2;
	if (l>0)
	while (l>=0) {
		/* patch merge line with '\' only outside quotes */
		idx=strlen(listing[l])-1;
		
		if (idx>0) {
			while (idx && (listing[l][idx]==' ' || listing[l][idx]==0x0D || listing[l][idx]==0x0A || listing[l][idx]==0x0B)) {
				idx--;
			}

			if (listing[l][idx]=='\\') {
				/* fusion avec la ligne suivante qui est obligatoirement présente */
				listing[l]=MemRealloc(listing[l],strlen(listing[l])+strlen(listing[l+1])+1);
				strcpy(listing[l]+idx,listing[l+1]);
				strcpy(listing[l+1],"");
			}
		}
		l--;
	}
}

void PreProcessingSplitListing(struct s_listing **listing, int *il, int *ml, int idx, int end, int start)
{
	#undef FUNC
	#define FUNC "PreProcessingSplitListing"
	
	struct s_listing curlisting={0};

	/* split current line because there will be "before" include and "after include" line */
	ObjectArrayAddDynamicValueConcat((void**)listing,il,ml,&curlisting,sizeof(curlisting));
	MemMove(&((*listing)[idx+2]),&((*listing)[idx+1]),(*il-idx-2)*sizeof(struct s_listing));
	(*listing)[idx+1].ifile=(*listing)[idx].ifile;
	(*listing)[idx+1].iline=(*listing)[idx].iline;
	if ((*listing)[idx].listing[start]) {
		(*listing)[idx+1].listing=TxtStrDup((*listing)[idx].listing+start);
	} else {
		(*listing)[idx+1].listing=TxtStrDup(";");
	}
	strcpy((*listing)[idx].listing+end,":");
}

void PreProcessingInsertListing(struct s_listing **reflisting, int *il, int *ml, int idx, char **zelines, int ifile)
{
	#undef FUNC
	#define FUNC "PreProcessingSplitListing"
	
	struct s_listing *listing;
	int nbinsert,li,bil;
	for (li=nbinsert=0;zelines[li];li++) nbinsert++;
	bil=*il;
	if (*il+nbinsert>=*ml) {
		*il=*ml=*il+nbinsert;
		*reflisting=MemRealloc(*reflisting,sizeof(struct s_listing)*(*ml));
	} else {
		*il=*il+nbinsert;
	}
	listing=*reflisting;
	MemMove(&listing[idx+1+nbinsert],&listing[idx+1],(bil-idx-1)*sizeof(struct s_listing));
	
	for (li=0;zelines[li];li++) {
		listing[idx+1+li].ifile=ifile;
		listing[idx+1+li].iline=li+1;
		listing[idx+1+li].listing=zelines[li];
	}
}

int cmpkeyword(const void * a, const void * b)
{
	struct s_asm_keyword *sa,*sb;
	sa=(struct s_asm_keyword *)a;
	sb=(struct s_asm_keyword *)b;
	return strcmp(sa->mnemo,sb->mnemo);
}

struct s_assenv *PreProcessing(char *filename, int flux, const char *datain, int datalen, struct s_parameter *param)
{
	#undef FUNC
	#define FUNC "PreProcessing"

	#define CharWord "@ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.=_($)][+-*/^%#|&'\"\\m}{[]"

	struct s_assenv *ae=NULL;
	struct s_wordlist curw={0};
	struct s_wordlist *wordlist=NULL;
	int nbword=0,maxword=0;
	char **zelines=NULL;

	char *filename_toread;
	
	struct s_macro_fast *MacroFast=NULL;
	int idxmacrofast=0,maxmacrofast=0;
	
	struct s_listing *listing=NULL;
	struct s_listing curlisting;
	int ilisting=0,maxlisting=0;
	
	char **listing_include=NULL;
	int i,j,l=0,idx=0,c=0,li,le;
	char Automate[256]={0};
	struct s_hexbin curhexbin;
	char *newlistingline=NULL;
	struct s_label curlabel={0};
	char *labelsep1;
	char **labelines=NULL;
	/* state machine buffer */
	unsigned char *mem=NULL;
	char *w=NULL,*wtmp=NULL;
	int lw=0,mw=256;
	char *bval=NULL;
	int ival=0,sval=256;
	char *qval=NULL;
	int iqval=0,sqval=256;
	struct s_repeat_index *TABrindex=NULL;
	struct s_repeat_index *TABwindex=NULL;
	struct s_repeat_index rindex={0};
	struct s_repeat_index windex={0};
	int nri=0,mri=0,ri=0;
	int nwi=0,mwi=0,wi=0;
	/* state machine trigger */
	int waiting_quote=0,lquote=0;
	int macro_trigger=0;
	int escape_code=0;
	int quote_type=0;
	int incbin=0,include=0,crunch=0;
	int rewrite=0,hadcomma=0;
	int nbinstruction;
	int ifast,texpr;
	int ispace=0;
	char opassign=0;
	/* incbin bug */
	int incstartL=0;



#if TRACE_GENERALE
printf("*** preprocessing ***\n");
#endif

#if TRACE_PREPRO
printf("start prepro, alloc assenv\n");
#endif

	windex.cl=-1;
	windex.cidx=-1;
	rindex.cl=-1;
	rindex.cidx=-1;

#if TRACE_PREPRO
printf("malloc+memset\n");
#endif
	ae=MemMalloc(sizeof(struct s_assenv));
	memset(ae,0,sizeof(struct s_assenv));

	ae->module_separator[0]='_';
#if TRACE_PREPRO
printf("paramz 1\n");
#endif
	if (param) {
		ae->export_local=param->export_local;
		ae->export_sym=param->export_sym;
		ae->export_var=param->export_var;
		ae->export_equ=param->export_equ;
		ae->export_sna=param->export_sna;
		ae->export_snabrk=param->export_snabrk;
		if (param->export_sna || param->export_snabrk) {
			ae->forcesnapshot=1;
		}
		ae->export_brk=param->export_brk;
		ae->warn_unused=param->warn_unused;
		ae->display_stats=param->display_stats;
		ae->edskoverwrite=param->edskoverwrite;
		ae->rough=param->rough;
		ae->as80=param->as80;
		ae->pasmo=param->pasmo;
		ae->dams=param->dams;
		ae->macrovoid=param->macrovoid;
		if (param->v2) {
			ae->forcesnapshot=1;
			ae->snapshot.version=2;
		} else {
			ae->snapshot.version=3;
		}
		ae->xpr=param->xpr;
		ae->maxerr=param->maxerr;
		ae->extended_error=param->extended_error;
		ae->nowarning=param->nowarning;
		ae->erronwarn=param->erronwarn;
		ae->breakpoint_name=param->breakpoint_name;
		ae->symbol_name=param->symbol_name;
		ae->binary_name=param->binary_name;
		ae->flexible_export=param->flexible_export;
		ae->cartridge_name=param->cartridge_name;
		ae->snapshot_name=param->snapshot_name;
		ae->tape_name=param->tape_name;
		ae->rom_name=param->rom_name;
		ae->checkmode=param->checkmode;
		ae->noampersand=param->noampersand;
		ae->module_separator[0]=param->module_separator;
		if (param->rough) ae->maxam=0; else ae->maxam=1;
		/* additional symbols */
		for (i=0;i<param->nsymb;i++) {
			char *sep;
			sep=strchr(param->symboldef[i],'=');
			if (sep) {
				*sep=0;
				ExpressionSetDicoVar(ae,param->symboldef[i],atof(sep+1));
			}
		}
		if (param->msymb) {
			MemFree(param->symboldef);
			param->nsymb=param->msymb=0;
		}
		/* include paths */
		ae->includepath=param->pathdef;
		ae->ipath=param->npath;
		ae->mpath=param->mpath;
		/* old inline params */
		ae->dependencies=param->dependencies;
	}
#if TRACE_PREPRO
printf("init 0 amper=%d\n",ae->noampersand);
#endif
#if TRACE_GENERALE
printf("-init\n");
#endif
	/* generic init */
	ae->ctx1.maxivar=1;
	ae->ctx2.maxivar=1;
	ae->computectx=&ae->ctx1;
	ae->flux=flux;
	/* check snapshot structure */
	if (sizeof(ae->snapshot)!=0x100 || &ae->snapshot.fdd.motorstate-(unsigned char*)&ae->snapshot!=0x9C || &ae->snapshot.crtcstate.model-(unsigned char*)&ae->snapshot!=0xA4
		|| &ae->snapshot.romselect-(unsigned char*)&ae->snapshot!=0x55
		|| &ae->snapshot.interruptrequestflag-(unsigned char*)&ae->snapshot!=0xB4
		|| &ae->snapshot.CPCType-(unsigned char*)&ae->snapshot!=0x6D) {
		rasm_printf(ae,"snapshot structure integrity check KO\n");
		exit(349);
	}
	
	for (i=0;i<4;i++) {
		ae->bankgate[i]=0x7FC0; /* video memory has no paging */
		ae->setgate[i]=0x7FC0; /* video memory has no paging */
	}
	for (i=0;i<256;i++) {
		/* 4M expansion support on lower gate array port */
		ae->bankgate[i+4]=0x7FC4+(i&3)+((i&31)>>2)*8-0x100*(i>>5);
		ae->setgate[i+4] =0x7FC2      +((i&31)>>2)*8-0x100*(i>>5);
		//printf("%04X %04X\n",ae->bankgate[i+4],ae->setgate[i+4]);
	}
	
	memcpy(ae->snapshot.idmark,"MV - SNA",8);
	ae->snapshot.registers.IM=1;

	ae->snapshot.gatearray.palette[0]=0x04;
	ae->snapshot.gatearray.palette[1]=0x0A;
	ae->snapshot.gatearray.palette[2]=0x15;
	ae->snapshot.gatearray.palette[3]=0x1C;
	ae->snapshot.gatearray.palette[4]=0x18;
	ae->snapshot.gatearray.palette[5]=0x1D;
	ae->snapshot.gatearray.palette[6]=0x0C;
	ae->snapshot.gatearray.palette[7]=0x05;
	ae->snapshot.gatearray.palette[8]=0x0D;
	ae->snapshot.gatearray.palette[9]=0x16;
	ae->snapshot.gatearray.palette[10]=0x06;
	ae->snapshot.gatearray.palette[11]=0x17;
	ae->snapshot.gatearray.palette[12]=0x1E;
	ae->snapshot.gatearray.palette[13]=0x00;
	ae->snapshot.gatearray.palette[14]=0x1F;
	ae->snapshot.gatearray.palette[15]=0x0E;
	ae->snapshot.gatearray.palette[16]=0x04;

	ae->snapshot.gatearray.multiconfiguration=0x8D; // lower/upper ROM off + mode 1
	ae->snapshot.CPCType=2; /* 6128 */
	ae->snapshot.crtcstate.model=0; /* CRTC 0 */
	ae->snapshot.vsyncdelay=2;
	strncpy((char *)ae->snapshot.unused6+3+0x20,RASM_VERSION,sizeof(ae->snapshot.unused6)-1-(3+0x20));
	/* CRTC default registers */
	ae->snapshot.crtc.registervalue[0]=0x3F;
	ae->snapshot.crtc.registervalue[1]=40;
	ae->snapshot.crtc.registervalue[2]=46;
	ae->snapshot.crtc.registervalue[3]=0x8E;
	ae->snapshot.crtc.registervalue[4]=38;
	ae->snapshot.crtc.registervalue[6]=25;
	ae->snapshot.crtc.registervalue[7]=30;
	ae->snapshot.crtc.registervalue[9]=7;
	ae->snapshot.crtc.registervalue[12]=0x30;
	ae->snapshot.psg.registervalue[7]=0x3F; /* audio mix all channels OFF */
	/* PPI Init */
	ae->snapshot.ppi.control=0x82;
	/* standard stack */
	ae->snapshot.registers.HSP=0xC0;

	/*
		Winape		sprintf(symbol_line,"%s #%4X\n",ae->label[i].name,ae->label[i].ptr);
		pasmo		sprintf(symbol_line,"%s EQU 0%4XH\n",ae->label[i].name,ae->label[i].ptr);
		rasm 		sprintf(symbol_line,"%s #%X B%d\n",ae->wl[ae->label[i].iw].w,ae->label[i].ptr,ae->label[i].ibank>31?0:ae->label[i].ibank);
	*/
#if TRACE_PREPRO
printf("paramz\n");
#endif
	if (param && param->labelfilename) {
		for (j=0;param->labelfilename[j] && param->labelfilename[j][0];j++) {
			rasm_printf(ae,"Label import from [%s]\n",param->labelfilename[j]);
			ae->label_filename=param->labelfilename[j];
			ae->label_line=1;
			labelines=FileReadLines(param->labelfilename[j]);
			i=0;
			while (labelines[i]) {
				/* upper case */
				for (j=0;labelines[i][j];j++) labelines[i][j]=toupper(labelines[i][j]);

				if ((labelsep1=strstr(labelines[i],": EQU 0"))!=NULL) {
					/* sjasm */
					*labelsep1=0;
					curlabel.name=labelines[i];
					curlabel.iw=-1;
					curlabel.crc=GetCRC(curlabel.name);
					curlabel.ptr=strtol(labelsep1+6,NULL,16);
					PushLabelLight(ae,&curlabel);
				} else if ((labelsep1=strstr(labelines[i]," EQU 0"))!=NULL) {
					/* pasmo */
					*labelsep1=0;
					curlabel.name=labelines[i];
					curlabel.iw=-1;
					curlabel.crc=GetCRC(curlabel.name);
					curlabel.ptr=strtol(labelsep1+6,NULL,16);
					//ObjectArrayAddDynamicValueConcat((void **)&ae->label,&ae->il,&ae->ml,&curlabel,sizeof(curlabel));
					PushLabelLight(ae,&curlabel);
				} else if ((labelsep1=strstr(labelines[i]," "))!=NULL) {
					/* Winape / rasm */
					if (*(labelsep1+1)=='#') {
						*labelsep1=0;
						curlabel.name=labelines[i];
						curlabel.iw=-1;
						curlabel.crc=GetCRC(curlabel.name);
						curlabel.ptr=strtol(labelsep1+2,NULL,16);
						//ObjectArrayAddDynamicValueConcat((void **)&ae->label,&ae->il,&ae->ml,&curlabel,sizeof(curlabel));
						PushLabelLight(ae,&curlabel);
					}
				}
				i++;
				ae->label_line++;
			}
			MemFree(labelines);
		}
		ae->label_filename=NULL;
		ae->label_line=0;
	}
#if TRACE_PREPRO
printf("init 3\n");
#endif
	/* 32 CPR default roms but 260+ max snapshot RAM pages + one workspace */
	for (i=0;i<BANK_MAX_NUMBER+1;i++) {
		mem=MemMalloc(65536);
		memset(mem,0,65536);
		ObjectArrayAddDynamicValueConcat((void**)&ae->mem,&ae->nbbank,&ae->maxbank,&mem,sizeof(mem));
	}
#if TRACE_PREPRO
printf("nbbank=%d initialised\n",ae->nbbank);
#endif
	ae->activebank=BANK_MAX_NUMBER;
	ae->maxptr=65536;
	for (i=0;i<256;i++) { ae->charset[i]=(unsigned char)i; }

	if (param && param->outputfilename) {
		ae->outputfilename=TxtStrDup(param->outputfilename);
	} else if (param && param->automatic_radix && param->filename) {
		int rilook;
		rilook=strlen(param->filename);
		ae->outputfilename=TxtStrDup(param->filename);
		/* look for extension */
		while (rilook && ae->outputfilename[rilook]!='.') {
			/* end of scan with directory reference or nothing found */
			if (ae->outputfilename[rilook]=='/' || ae->outputfilename[rilook]=='\\') rilook=0; else rilook--;
		}
		if (ae->outputfilename[rilook]=='.') {
			ae->outputfilename[rilook]=0;
		}
	} else {
		ae->outputfilename=TxtStrDup("rasmoutput");
	}
	/* si on est en ligne de commande ET que le fichier n'est pas trouvé */
	if (param && param->filename && !FileExists(param->filename)) {
		char *LTryExtension[]={".asm",".z80",".o",".s",".inc",".src",".dam",".mxm",".txt",
					".ASM",".Z80",".O",".S",".INC",".SRC",".DAM",".MXM",".TXT","",NULL};

		int iguess=1;
#if TRACE_PREPRO
		printf("TRY EXT\n");
#endif
		l=strlen(param->filename);
		param->filename=MemRealloc(param->filename,l+6);
		/* si le nom du fichier termine par un . on n'ajoute que l'extension, sinon on l'ajoute avec le . */
		if (param->filename[l-1]=='.') strcat(param->filename,"asm"); else strcat(param->filename,".asm");

		while (!FileExists(param->filename) && LTryExtension[iguess]!=NULL) {
			TxtReplace(param->filename,LTryExtension[iguess-1],LTryExtension[iguess],0); /* no realloc with this */
			if (!FileExists(param->filename)) {
				param->filename[l]=0;
			}
			iguess++;
		}
		filename=param->filename;
	}

	if (param && param->filename && !FileExists(param->filename)) {
		rasm_printf(ae,"Cannot find file [%s]\n",param->filename);
		exit(-1802);
	}
	
	if (param) rasm_printf(ae,KAYGREEN"Pre-processing [%s]\n",param->filename);
	for (nbinstruction=0;instruction[nbinstruction].mnemo[0];nbinstruction++);
	qsort(instruction,nbinstruction,sizeof(struct s_asm_keyword),cmpkeyword);
	for (i=0;i<256;i++) { ae->fastmatch[i]=-1; }
	for (i=0;i<nbinstruction;i++) { if (ae->fastmatch[(int)instruction[i].mnemo[0]]==-1) ae->fastmatch[(int)instruction[i].mnemo[0]]=i; } 
	for (i=0;CharWord[i];i++) {Automate[((int)CharWord[i])&0xFF]=1;}
	 /* separators */
	Automate[' ']=2;
	Automate[',']=2;
	Automate['\t']=2;
	/* end of line */
	Automate[':']=3; /* les 0x0A et 0x0D seront deja  remplaces en ':' */
	/* expression */
	Automate['=']=4; /* on stocke l'emplacement de l'egalite */
	Automate['<']=4; /* ou des operateurs */
	Automate['>']=4; /* d'evaluation */
	Automate['!']=4;
	
	StateMachineResizeBuffer(&w,256,&mw);
	StateMachineResizeBuffer(&bval,256,&sval);
	StateMachineResizeBuffer(&qval,256,&sqval);
	w[0]=0;
	bval[0]=0;
	qval[0]=0;
	
#if TRACE_PREPRO
printf("read file/flux\n");
#endif
#if TRACE_GENERALE
printf("-read/flux\n");
#endif

	if (!ae->flux) {
		zelines=FileReadLines(filename);
		FieldArrayAddDynamicValueConcat(&ae->filename,&ae->ifile,&ae->maxfile,filename);
	} else {
		int flux_nblines=0;
		int flux_curpos;

		/* copie des données */
		for (i=0;i<datalen;i++) {
			if (datain[i]=='\n') flux_nblines++;
		}
		zelines=MemMalloc(sizeof(char *)*(flux_nblines+2));
		flux_nblines=0;
		flux_curpos=0;
		for (i=0;i<datalen;i++) {
			if (datain[i]=='\n') {
				/* almost empty lines must allocate CR+terminator */
				zelines[flux_nblines]=MemMalloc(i-flux_curpos+2);
				/* copy data+CR */
				memcpy(zelines[flux_nblines],datain+flux_curpos,i-flux_curpos+1);
				/* et on ajoute un petit zéro à la fin! */
				zelines[flux_nblines][i-flux_curpos+1]=0;
#if 0
if (flux_nblines<50) printf("%02d[%s]\n",flux_nblines,zelines[flux_nblines]);
#endif
				flux_curpos=i+1;
				flux_nblines++;
			}
		}
		if (i>flux_curpos) {
			zelines[flux_nblines]=MemMalloc(i-flux_curpos+1);
			memcpy(zelines[flux_nblines],datain+flux_curpos,i-flux_curpos);
			zelines[flux_nblines][i-flux_curpos]=0;
			flux_nblines++;
		}
		/* terminator */
		zelines[flux_nblines]=NULL;

		/* en mode flux on prend le repertoire courant en reference */
		FieldArrayAddDynamicValueConcat(&ae->filename,&ae->ifile,&ae->maxfile,CURRENT_DIR);
	}	

#if TRACE_PREPRO
printf("remove comz, do includes\n");
#endif
#if TRACE_GENERALE
printf("-comz/include\n");
#endif
	EarlyPrepSrc(ae,zelines,ae->filename[ae->ifile-1]);

	for (i=0;zelines[i];i++) {
		curlisting.ifile=0;
		curlisting.iline=i+1;
		curlisting.listing=zelines[i];
		ObjectArrayAddDynamicValueConcat((void**)&listing,&ilisting,&maxlisting,&curlisting,sizeof(curlisting));
	}
	MemFree(zelines);

	/* on s'assure que la derniere instruction est prise en compte a peu de frais */
	if (ilisting) {
		datalen=strlen(listing[ilisting-1].listing);
		listing[ilisting-1].listing=MemRealloc(listing[ilisting-1].listing,datalen+2);
		listing[ilisting-1].listing[datalen]=':';
		listing[ilisting-1].listing[datalen+1]=0;
	}

	waiting_quote=quote_type=0;

	l=idx=0;
	while (l<ilisting) {

#if TRACE_PREPRO
if (!idx) printf("[%s]\n",listing[l].listing);
#endif

		c=listing[l].listing[idx++];
		if (!c) {
			l++;
			idx=0;
			continue;
		} else if (c=='\\' && !waiting_quote) {
			idx++;
			continue;
		} else if (c==0x0D || c==0x0A) {
			listing[l].listing[idx-1]=':';
			c=':';
		} else if (c=='\'' && idx>2 && strncmp(&listing[l].listing[idx-3],"AF'",3)==0) {
			/* rien */
		} else if (c=='"' || c=='\'') {
			if (!quote_type) {
				quote_type=c;
				lquote=l;
			} else {
				if (c==quote_type) {
					quote_type=0;
				}
			}
		}

		if (waiting_quote) {
			/* expecting quote and NOTHING else */
			switch (waiting_quote) {
				case 1:
					if (c==quote_type) waiting_quote=2; else {
						/* enforce there is only spaces or tabs between READ/INC and string */
						if (c!=' ' && c!=0x9) {
							MakeError(ae,ae->filename[listing[l].ifile],listing[l].iline,"A quoted string must follow INCLUDE/READ directive\n");
							waiting_quote=0;
							idx--;
							continue;
						}
					}
					break;
				case 2:
					if (!quote_type) {
						waiting_quote=3;
						qval[iqval]=0;
					} else {
						qval[iqval++]=c;
						StateMachineResizeBuffer(&qval,iqval,&sqval);
						qval[iqval]=0;
					}
			}
			if (waiting_quote==3) {
				if (incbin) {
					int fileok=0,ilookfile;
					/* qval contient le nom du fichier a lire */
					filename_toread=MergePath(ae,ae->filename[listing[l].ifile],qval);
					if (FileExists(filename_toread)) {
						fileok=1;
					} else {
						for (ilookfile=0;ilookfile<ae->ipath && !fileok;ilookfile++) {
							filename_toread=MergePath(ae,ae->includepath[ilookfile],qval);
							if (FileExists(filename_toread)) {
								fileok=1;
							}
						}
					}
					curhexbin.filename=TxtStrDup(filename_toread);
					curhexbin.crunch=crunch;
					switch (crunch) {
						case 18:
							curhexbin.version=1;
							curhexbin.minmatch=5;
							break;
						case 19:
							curhexbin.crunch=18;
							curhexbin.version=2;
							curhexbin.minmatch=2;
							break;
						default:break;
					}

					/* TAG + info */
					curhexbin.datalen=-1;
					curhexbin.data=NULL;
					/* not yet an error, we will know later when executing the code */
					ObjectArrayAddDynamicValueConcat((void**)&ae->hexbin,&ae->ih,&ae->mh,&curhexbin,sizeof(curhexbin));

					/* v0.130 handling error case with filename on multiple lines */
					if (incstartL!=l) {
						int iconcat=incstartL;
						int ilen;

						MakeError(ae,ae->filename[listing[incstartL].ifile],listing[incstartL].iline,"INCBIN filename cannot be on multiple lines\n");

						ilen=strlen(listing[iconcat].listing)+1;
						idx+=ilen-rewrite;
						while (iconcat<l) {
							int tlen;
							iconcat++;
							tlen=strlen(listing[iconcat].listing);
							ilen+=tlen;
							if (iconcat<l) idx+=tlen;
						}

						iconcat=incstartL;
						listing[iconcat].listing=MemRealloc(listing[iconcat].listing,ilen);
						while (iconcat<l) {
							iconcat++;
							strcat(listing[incstartL].listing,listing[iconcat].listing);
							listing[iconcat].listing[0]=0;
						}
						l=incstartL;

						/* patch data to remain Assembler silent about this */
						for (i=rewrite;i<idx;i++) listing[incstartL].listing[i]=' ';
						/* delete entry */
						MemFree(curhexbin.filename);
						ae->ih--;
					} else {
						/* insertion */
						le=strlen(listing[l].listing);

						newlistingline=MemMalloc(le+32);
						memcpy(newlistingline,listing[l].listing,rewrite);
						rewrite+=sprintf(newlistingline+rewrite,"HEXBIN #%X",ae->ih-1);
						strcat(newlistingline+rewrite,listing[l].listing+idx);
						idx=rewrite;
						MemFree(listing[l].listing);
						listing[l].listing=newlistingline;
					}
					incbin=0;
				} else if (include) {
					/* qval contient le nom du fichier a lire */
					int fileok=0,ilookfile;
					/* cette notion n'existe pas dans le cas normal */
					curhexbin.datalen=0;
					/* qval contient le nom du fichier a lire */
					filename_toread=MergePath(ae,ae->filename[listing[l].ifile],qval);
					if (FileExists(filename_toread)) {
						fileok=1;
					} else {
						for (ilookfile=0;ilookfile<ae->ipath && !fileok;ilookfile++) {
							filename_toread=MergePath(ae,ae->includepath[ilookfile],qval);
							if (FileExists(filename_toread)) {
								fileok=1;
							}
						}
					}

					/* v0.130 handling error case with filename on multiple lines */
					if (incstartL!=l) {
						int iconcat=incstartL;
						int ilen;

						MakeError(ae,ae->filename[listing[incstartL].ifile],listing[incstartL].iline,"INCLUDE filename cannot be on multiple lines\n");
						ilen=strlen(listing[iconcat].listing)+1;
						idx+=ilen-rewrite;
						while (iconcat<l) {
							int tlen;
							iconcat++;
							tlen=strlen(listing[iconcat].listing);
							ilen+=tlen;
							if (iconcat<l) idx+=tlen;
						}

						iconcat=incstartL;
						listing[iconcat].listing=MemRealloc(listing[iconcat].listing,ilen);
						while (iconcat<l) {
							iconcat++;
							strcat(listing[incstartL].listing,listing[iconcat].listing);
							listing[iconcat].listing[0]=0;
						}
						l=incstartL;

					} else {
						if (fileok) {
							#if TRACE_PREPRO
							rasm_printf(ae,KBLUE"include [%s]\n",filename_toread);
							#endif
							
							/* lecture */
							listing_include=FileReadLines(filename_toread);
							FieldArrayAddDynamicValueConcat(&ae->filename,&ae->ifile,&ae->maxfile,filename_toread);
							/* virer les commentaires + pré-traitement */
							EarlyPrepSrc(ae,listing_include,ae->filename[ae->ifile-1]);

							/* split de la ligne en cours + suppression de l'instruction include */
							PreProcessingSplitListing(&listing,&ilisting,&maxlisting,l,rewrite,idx);
							/* insertion des nouvelles lignes + reference fichier + numeros de ligne */
							PreProcessingInsertListing(&listing,&ilisting,&maxlisting,l,listing_include,ae->ifile-1);
							
							MemFree(listing_include); /* free le tableau mais pas les lignes */
							listing_include=NULL;
							idx=0; /* on reste sur la meme ligne mais on se prepare a relire du caractere 0! */
						} else {
							/********************************************
							*      E R R O R    M a n a g e m e n t     *
							********************************************/
							char tmp_insert[128];

							for (i=rewrite;i<idx;i++) listing[incstartL].listing[i]=' ';
							sprintf(tmp_insert,"READ %d",ae->ih);
							memcpy(listing[incstartL].listing+rewrite,tmp_insert,strlen(tmp_insert));
							
							filename_toread=MergePath(ae,ae->filename[listing[l].ifile],qval);
							curhexbin.filename=TxtStrDup(filename_toread);
							curhexbin.crunch=crunch;

							/* TAG + info */
							curhexbin.datalen=-2;
							curhexbin.data=NULL;
							/* not yet an error, we will know later when executing the code */
							ObjectArrayAddDynamicValueConcat((void**)&ae->hexbin,&ae->ih,&ae->mh,&curhexbin,sizeof(curhexbin));
						}
					}
					include=0;
				}
				waiting_quote=0;
				qval[0]=0;
				iqval=0;
			}
		} else {
			/* classic behaviour */

			/* looking for include/incbin */
			if (((c>='A' && c<='Z') || (c>='0' && c<='9') || c=='@' || c=='_')&& !quote_type) {
				bval[ival++]=c;
				StateMachineResizeBuffer(&bval,ival,&sval);
				bval[ival]=0;
			} else {
				switch (bval[0]) {
					case 'I':
						if (bval[1]=='N' && bval[2]=='C') {
							if (strcmp(bval,"INCLUDE")==0) {
								incstartL=l;
								include=1;
								waiting_quote=1;
								rewrite=idx-7-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCLZSA2")==0) {
								incstartL=l;
								incbin=1;
								crunch=19;
								waiting_quote=1;
								rewrite=idx-8-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCLZSA1")==0) {
								incstartL=l;
								incbin=1;
								crunch=18;
								waiting_quote=1;
								rewrite=idx-8-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCAPU")==0) {
								incstartL=l;
								incbin=1;
								crunch=17;
								waiting_quote=1;
								rewrite=idx-6-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCLZ4")==0) {
								incstartL=l;
								incbin=1;
								crunch=4;
								waiting_quote=1;
								rewrite=idx-6-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCEXO")==0) {
								incstartL=l;
								incbin=1;
								crunch=8;
								waiting_quote=1;
								rewrite=idx-6-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCZX7")==0) {
								incstartL=l;
								incbin=1;
								crunch=7;
								waiting_quote=1;
								rewrite=idx-6-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCL48")==0) {
								incstartL=l;
								incbin=1;
								crunch=48;
								waiting_quote=1;
								rewrite=idx-6-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCL49")==0) {
								incstartL=l;
								incbin=1;
								crunch=49;
								waiting_quote=1;
								rewrite=idx-6-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCBIN")==0) {
								incstartL=l;
								incbin=1;
								crunch=0;
								waiting_quote=1;
								rewrite=idx-6-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							} else if (strcmp(bval,"INCWAV")==0) {
								incstartL=l;
								incbin=1;
								crunch=0;
								waiting_quote=1;
								rewrite=idx-6-1;
								/* quote right after keyword */
								if (c==quote_type) {
									waiting_quote=2;
								}
							}
						}
						break;

					case 'U':
						/* code dupliqué du REND */
						if (strcmp(bval,"UNTIL")==0) {
							/* retrouver la structure repeat_index correspondant a l'ouverture */
							for (ri=nri-1;ri>=0;ri--) {
								if (TABrindex[ri].cl==-1) {
									TABrindex[ri].cl=c;
									TABrindex[ri].cidx=idx;
									break;
								}
							}
							if (ri==-1) {
								MakeError(ae,ae->filename[listing[l].ifile],listing[l].iline,"%s refers to unknown REPEAT\n",bval);
								//exit(1);
							}
						}
						break;

					case 'R':
						if (strcmp(bval,"READ")==0) {
							incstartL=l;
							include=1;
							waiting_quote=1;
							rewrite=idx-4-1;
							/* quote right after keyword */
							if (c==quote_type) {
								waiting_quote=2;
							}
						/* code dupliqué du UNTIL */
						} else if (strcmp(bval,"REND")==0) {
							/* retrouver la structure repeat_index correspondant a l'ouverture */
							for (ri=nri-1;ri>=0;ri--) {
								if (TABrindex[ri].cl==-1) {
									TABrindex[ri].cl=c;
									TABrindex[ri].cidx=idx;
									break;
								}
							}
							if (ri==-1) {
								MakeError(ae,ae->filename[listing[l].ifile],listing[l].iline,"%s refers to unknown REPEAT\n",bval);
								//exit(1);
							}
						} else if (strcmp(bval,"REPEAT")==0) {
							/* remplir la structure repeat_index */
							rindex.ol=listing[l].iline;
							rindex.oidx=idx;
							rindex.ifile=ae->ifile-1;
							ObjectArrayAddDynamicValueConcat((void**)&TABrindex,&nri,&mri,&rindex,sizeof(rindex));
						}
						break;

					case 'W':
						if (strcmp(bval,"WHILE")==0) {
							/* remplir la structure repeat_index */
							windex.ol=listing[l].iline;
							windex.oidx=idx;
							windex.ifile=ae->ifile-1;
							ObjectArrayAddDynamicValueConcat((void**)&TABwindex,&nwi,&mwi,&windex,sizeof(windex));
						} else if (strcmp(bval,"WEND")==0) {
							/* retrouver la structure repeat_index correspondant a l'ouverture */
							for (wi=nwi-1;wi>=0;wi--) {
								if (TABwindex[wi].cl==-1) {
									TABwindex[wi].cl=c;
									TABwindex[wi].cidx=idx;
									break;
								}
							}
							if (wi==-1) {
								MakeError(ae,ae->filename[listing[l].ifile],listing[l].iline,"WEND refers to unknown WHILE\n");
								//exit(1);
							}
								
						}
						break;

					default: if (ae->noampersand && c=='&') {
							  listing[l].listing[idx-1]='#';
#if TRACE_PREPRO
printf("patch & => #\n");
#endif
						}
						break;
				}

#if TRACE_PREPRO
printf("bval=[%s]\n",bval);
#endif
				bval[0]=0;
				ival=0;
			}
		}
	}
	if (waiting_quote && ilisting) {
		MakeError(ae,ae->filename[listing[ilisting-1].ifile],listing[ilisting-1].iline,"A quoted string must follow INCLUDE/READ directive\n");
	}

#if TRACE_PREPRO
printf("check quotes and repeats\n");
#endif
	if (quote_type) {
		MakeError(ae,ae->filename[listing[lquote].ifile],listing[lquote].iline,"quote opened was not closed\n");
		//exit(1);
	}

	/* repeat expansion check */
	for (ri=0;ri<nri;ri++) {
		if (TABrindex[ri].cl==-1) {
			MakeError(ae,ae->filename[TABrindex[ri].ifile],TABrindex[ri].ol,"REPEAT was not closed\n");
		}
	}

	/* creer une liste de mots */
	curw.w=TxtStrDup("BEGIN");
	curw.l=0;
	curw.ifile=0;
	curw.t=1;
	curw.e=0;
	ObjectArrayAddDynamicValueConcat((void**)&wordlist,&nbword,&maxword,&curw,sizeof(curw));

	/* pour les calculs d'adresses avec IX et IY on enregistre deux variables bidons du meme nom */
	curw.e=2;
	curw.w=TxtStrDup("IX~0");
	ObjectArrayAddDynamicValueConcat((void**)&wordlist,&nbword,&maxword,&curw,sizeof(curw));
	curw.w=TxtStrDup("IY~0");
	ObjectArrayAddDynamicValueConcat((void**)&wordlist,&nbword,&maxword,&curw,sizeof(curw));
	curw.e=0;

#if TRACE_PREPRO
	l=0;
	while (l<ilisting) {
		rasm_printf(ae,"listing[%d]\n%s\n",l,listing[l].listing);
		l++;
	}
#endif	
#if TRACE_GENERALE
printf("-build wordlist\n");
#endif

	texpr=quote_type=0;
	l=lw=idx=0;
	ispace=0;
	w[0]=0;
	while (l<ilisting) {
		c=listing[l].listing[idx++];
		if (!c) {
			idx=0;
			l++;
			continue;
		}

		if (!quote_type) {
#if TRACE_PREPRO
//printf("c='%c' automate[c]=%d\n",c>31?c:'.',Automate[((int)c)&0xFF]);			
#endif
			switch (Automate[((int)c)&0xFF]) {
				case 0:
					MakeError(ae,ae->filename[listing[l].ifile],listing[l].iline,"invalid char '%c' (%d) char %d\n",c,c,idx);
#if TRACE_PREPRO
printf("c='%c' automate[c]=%d\n",c>31?c:'.',Automate[((int)c)&0xFF]);			
#endif
					exit(1);
					break;
				case 1:
					if (c=='\'' && idx>2 && strncmp(&listing[l].listing[idx-3],"AF'",3)==0) {
						w[lw++]=c;
						StateMachineResizeBuffer(&w,lw,&mw);
						w[lw]=0;
						break;
					} else if (c=='\'' || c=='"') {
						quote_type=c;
						/* debut d'une quote, on finalise le mot -> POURQUOI DONC? */
						//idx--;
#if TRACE_PREPRO
printf("quote\n");
#endif
						/* on finalise le mot si on est en début d'une nouvelle instruction ET que c'est un SAVE */
						if (strcmp(w,"SAVE")==0) {
							idx--;
						} else {
							w[lw++]=c;
							StateMachineResizeBuffer(&w,lw,&mw);
							w[lw]=0;
							break;
						}
					} else {
						if (c!=' ' && c!='\t') {
							w[lw++]=c;
							StateMachineResizeBuffer(&w,lw,&mw);
							w[lw]=0;
						} else {
							/* Winape/Maxam operator compatibility on expressions */
#if TRACE_PREPRO
printf("1/2 Winape maxam operator test for [%s]\n",w+ispace);
#endif
							if (texpr) {
								if (strcmp(w+ispace,"AND")==0) {
									w[ispace]='&';
									lw=ispace+1;
								} else if (strcmp(w+ispace,"OR")==0) {
#if TRACE_PREPRO
printf("conversion OR vers |\n");
#endif
									w[ispace]='|';
									lw=ispace+1;
								} else if (strcmp(w+ispace,"MOD")==0) {
									w[ispace]='m';
									lw=ispace+1;
								} else if (strcmp(w+ispace,"XOR")==0) {
									w[ispace]='^';
									lw=ispace+1;
								} else if (strcmp(w+ispace,"%")==0) {
									w[ispace]='m';
									lw=ispace+1;
								}
							}
							ispace=lw;
						}
						break;
					}
				case 2:
					/* separator (space, tab, comma) */
#if TRACE_PREPRO
printf("*** separator='%c'\n",c);
#endif
					
					/* patch argument suit une expression d'évaluation (ASSERT) */
					if (c==',') hadcomma=1;
					
					if (lw) {
						w[lw]=0;
						if (texpr && !wordlist[nbword-1].t && wordlist[nbword-1].e && !hadcomma) {
							/* pour compatibilite Winape avec AND,OR,XOR */
#if TRACE_PREPRO
printf("2/2 Winape maxam operator test for expression [%s]\n",w+ispace);
#endif
							if (strcmp(w,"AND")==0) {
								wtmp=TxtStrDup("&");
							} else if (strcmp(w,"OR")==0) {
								wtmp=TxtStrDup("|");
							} else if (strcmp(w,"XOR")==0) {
								wtmp=TxtStrDup("^");
							} else if (strcmp(w,"%")==0) {
								wtmp=TxtStrDup("m");
							} else {
								wtmp=TxtStrDup(w);
							}
							/* on concatène le nouveau mot à l'expression */
							nbword--;
							lw=0;
							for (li=0;wordlist[nbword].w[li];li++) {
								w[lw++]=wordlist[nbword].w[li];
								StateMachineResizeBuffer(&w,lw,&mw);
							}
							w[lw]=0;
							MemFree(wordlist[nbword].w);
							
							for (li=0;wtmp[li];li++) {
								w[lw++]=wtmp[li];
								StateMachineResizeBuffer(&w,lw,&mw);
							}
							w[lw]=0;
							MemFree(wtmp);
							/* et on modifie l'automate pour la suite! */
							Automate[' ']=1;
							Automate['\t']=1;
							ispace=lw;
						} else if (strcmp(w,"EQU")==0) {
							/* il y avait un mot avant alors on va reorganiser la ligne */
							nbword--;
							lw=0;
							for (li=0;wordlist[nbword].w[li];li++) {
								w[lw++]=wordlist[nbword].w[li];
								StateMachineResizeBuffer(&w,lw,&mw);
							}
							MemFree(wordlist[nbword].w);
							curw.e=lw+1;
							/* on ajoute l'egalite d'alias*/
							w[lw++]='~';
							StateMachineResizeBuffer(&w,lw,&mw);
							w[lw]=0;
							Automate[' ']=1;
							Automate['\t']=1;
							ispace=lw;
							texpr=1;
						} else {
							curw.w=TxtStrDup(w);
							curw.l=listing[l].iline;
							curw.ifile=listing[l].ifile;
							curw.t=0;
#if TRACE_PREPRO
if (curw.w[0]=='=') {
	printf("(1) bug prout\n");
	exit(1);
}
printf("ajout du mot [%s]\n",curw.w);
#endif
							ObjectArrayAddDynamicValueConcat((void**)&wordlist,&nbword,&maxword,&curw,sizeof(curw));
							//texpr=0; /* reset expr */
							curw.e=0;
							lw=0;
							w[lw]=0;

							/* match keyword? then next spaces will be ignored*/
							if (macro_trigger) {
								struct s_macro_fast curmacrofast;
								Automate[' ']=1;
								Automate['\t']=1;
								ispace=0;
								texpr=1;
#if TRACE_PREPRO
printf("macro trigger w=[%s]\n",curw.w);
#endif
								/* add macro name to instruction pool for preprocessor but not struct or write */
								if (macro_trigger=='M') {
									curmacrofast.mnemo=curw.w;
									curmacrofast.crc=GetCRC(curw.w);
									ObjectArrayAddDynamicValueConcat((void**)&MacroFast,&idxmacrofast,&maxmacrofast,&curmacrofast,sizeof(struct s_macro_fast));	
								}
								macro_trigger=0;
							} else {
								int keymatched=0;
								if ((ifast=ae->fastmatch[(int)curw.w[0]])!=-1) {
									while (instruction[ifast].mnemo[0]==curw.w[0]) {
										if (strcmp(instruction[ifast].mnemo,curw.w)==0) {
											keymatched=1;														
											if (strcmp(curw.w,"MACRO")==0 || strcmp(curw.w,"STRUCT")==0 || strcmp(curw.w,"WRITE")==0) {
/* @@TODO AS80 compatibility patch!!! */
												macro_trigger=curw.w[0];
											} else {
												Automate[' ']=1;
												Automate['\t']=1;
												ispace=0;
												/* instruction en cours, le reste est a interpreter comme une expression */
#if TRACE_PREPRO
printf("instruction en cours\n");												
#endif
												texpr=1;
											}
											break;
										}
										ifast++;
									}
								}
								if (!keymatched) {
									int macrocrc;
									macrocrc=GetCRC(curw.w);
									for (keymatched=0;keymatched<idxmacrofast;keymatched++) {
										if (MacroFast[keymatched].crc==macrocrc)
										if (strcmp(MacroFast[keymatched].mnemo,curw.w)==0) {
												Automate[' ']=1;
												Automate['\t']=1;
												ispace=0;
												/* macro en cours, le reste est a interpreter comme une expression */
												texpr=1;
												break;
										}
									}
								}
							}
						}
					} else {
						if (hadcomma) {
							MakeError(ae,ae->filename[listing[l].ifile],listing[l].iline,"empty parameter\n");
						}
					}
					break;
				case 3:
					/* fin de ligne, on remet l'automate comme il faut */
#if TRACE_PREPRO
printf("EOL\n");																	
#endif
					macro_trigger=0;
					Automate[' ']=2;
					Automate['\t']=2;
					ispace=0;
					texpr=0;
					/* si le mot lu a plus d'un caractère */
					if (lw) {
						if (!wordlist[nbword-1].t && (wordlist[nbword-1].e || w[0]=='=') && !hadcomma) {
							/* cas particulier d'ecriture libre */
							/* bugfix inhibition 19.06.2018 */
							/* ajout du terminateur? */
							w[lw]=0;
#if TRACE_PREPRO
printf("nbword=%d w=[%s] ->",nbword,w);fflush(stdout);
#endif
							nbword--;
							wordlist[nbword].w=MemRealloc(wordlist[nbword].w,strlen(wordlist[nbword].w)+lw+1);
							strcat(wordlist[nbword].w,w);
#if TRACE_PREPRO
printf("%s\n",wordlist[nbword].w);
#endif
							/* on change de type! */
							wordlist[nbword].t=1;
							//ObjectArrayAddDynamicValueConcat((void**)&wordlist,&nbword,&maxword,&curw,sizeof(curw));
							curw.e=0;
							lw=0;
							w[lw]=0;
						} else if (nbword && strcmp(w,"EQU")==0) {
							/* il y avait un mot avant alors on va reorganiser la ligne */
							nbword--;
							lw=0;
							for (li=0;wordlist[nbword].w[li];li++) {
								w[lw++]=wordlist[nbword].w[li];
								StateMachineResizeBuffer(&w,lw,&mw);
								w[lw]=0;
							}
							MemFree(wordlist[nbword].w);
							/* on ajoute l'egalite ou comparaison! */
							curw.e=lw+1;
							w[lw++]='=';
							StateMachineResizeBuffer(&w,lw,&mw);
							w[lw]=0;
							Automate[' ']=1;
							Automate['\t']=1;
						} else {
							/* mot de fin de ligne, à priori pas une expression */
							curw.w=TxtStrDup(w);
							curw.l=listing[l].iline;
							curw.ifile=listing[l].ifile;
							curw.t=1;
#if TRACE_PREPRO
printf("mot de fin de ligne = [%s]\n",curw.w);							
if (curw.w[0]=='=') {
	printf("(3) bug prout\n");
	exit(1);
}
#endif
							ObjectArrayAddDynamicValueConcat((void**)&wordlist,&nbword,&maxword,&curw,sizeof(curw));
							curw.e=0;
							lw=0;
							w[lw]=0;
							hadcomma=0;
						}
					} else {
						/* sinon c'est le précédent qui était terminateur d'instruction */
						wordlist[nbword-1].t=1;
						w[lw]=0;
					}
					hadcomma=0;
					break;
				case 4:
#if TRACE_PREPRO
printf("expr operator=%c\n",c);
#endif
					/* expression/condition */
					texpr=1;

					/* patch operator assignment with useless spacing v121 */
					if (lw==1 && c=='=') {
						switch (w[0]) {
							case '[':case ']':case '+':case '*':case '-':case '/':case '|':case '&':
#if TRACE_PREPRO
printf("*** patch prepro operator assignment + useless spacing\n");
#endif
								opassign=w[0];lw=0;
								break;
							default:opassign=0;break;
						}
					} else {
						opassign=0;
					}

				    if (lw) {
						Automate[' ']=1;
						Automate['\t']=1;
						if (!curw.e) {
							curw.e=lw+1;
							w[lw++]=c;
							StateMachineResizeBuffer(&w,lw,&mw);
							w[lw]=0;
						} else {
							w[lw++]=c;
							StateMachineResizeBuffer(&w,lw,&mw);
							w[lw]=0;
						}
					} else {
						/* 2018.06.06 évolution sur le ! (not) */
#if TRACE_PREPRO
printf("*** operateur commence le mot\n");
printf("mot precedent=[%s] t=%d\n",wordlist[nbword-1].w,wordlist[nbword-1].t);
#endif
						if (hadcomma && c=='!') {
							/* on peut commencer un argument par un NOT */
							w[lw++]=c;
							StateMachineResizeBuffer(&w,lw,&mw);
							w[lw]=0;
							/* automate déjà modifié rien de plus */
						} else if (!wordlist[nbword-1].t) {
							/* il y avait un mot avant alors on va reorganiser la ligne */
							/* patch NOT -> SAUF si c'est une directive */
							int keymatched=0;
							if ((ifast=ae->fastmatch[(int)wordlist[nbword-1].w[0]])!=-1) {
								while (instruction[ifast].mnemo[0]==wordlist[nbword-1].w[0]) {
									if (strcmp(instruction[ifast].mnemo,wordlist[nbword-1].w)==0) {
										keymatched=1;														
										break;
									}
									ifast++;
								}
							}
							if (!keymatched) {
								int macrocrc;
								macrocrc=GetCRC(wordlist[nbword-1].w);
								for (i=0;i<idxmacrofast;i++) {
									if (MacroFast[i].crc==macrocrc)
									if (strcmp(MacroFast[i].mnemo,wordlist[nbword-1].w)==0) {
										keymatched=1;
										break;
									}
								}
							}
							if (!keymatched) {
								nbword--;
								for (li=0;wordlist[nbword].w[li];li++) {
									w[lw++]=wordlist[nbword].w[li];
									StateMachineResizeBuffer(&w,lw,&mw);
									w[lw]=0;
								}
								MemFree(wordlist[nbword].w);
								/* on ajoute l'egalite ou comparaison! */
								if (opassign) {
									#if TRACE_PREPRO
									printf("*** patch opassign en finalisation de mot\n");
									#endif
									w[lw++]=opassign;
									StateMachineResizeBuffer(&w,lw,&mw);
									w[lw]=0;
								}
								curw.e=lw+1;
							}
							w[lw++]=c;
							StateMachineResizeBuffer(&w,lw,&mw);
							w[lw]=0;
							/* et on modifie l'automate pour la suite! */
							Automate[' ']=1;
							Automate['\t']=1;
						} else {
							MakeError(ae,ae->filename[listing[l].ifile],listing[l].iline,"cannot start expression with '=','!','<','>'\n");
						}
					}
					break;
				default:
					rasm_printf(ae,KERROR"Internal error (Automate wrong value=%d)\n",Automate[c]);
					exit(-1);
			}
		} else {
			/* lecture inconditionnelle de la quote */
#if TRACE_PREPRO
printf("quote[%d]=%c\n",lw,c);
#endif
			w[lw++]=c;
			StateMachineResizeBuffer(&w,lw,&mw);
			w[lw]=0;
			if (!escape_code) {
				if (c=='\\') escape_code=1;
				if (lw>1 && c==quote_type) {
					quote_type=0;
				}
			} else {
				escape_code=0;
			}
		}
	}

#if TRACE_PREPRO
printf("END\n");
#endif

	curw.w="END";
	curw.l=0;
	curw.t=2;
	curw.ifile=0;
	ObjectArrayAddDynamicValueConcat((void**)&wordlist,&nbword,&maxword,&curw,sizeof(curw));
#if TRACE_PREPRO
	rasm_printf(ae,KVERBOSE"wordlist contains %d element%s\n",nbword,nbword>1?"s":"");
#endif
	ae->nbword=nbword;

	/* switch words for macro declaration with AS80 & UZ80 */
	if (param && param->as80) {
		for (l=0;l<nbword;l++) {
			if (!wordlist[l].t && !wordlist[l].e && strcmp(wordlist[l+1].w,"MACRO")==0) {
				char *wtmp;
				wtmp=wordlist[l+1].w;
				wordlist[l+1].w=wordlist[l].w;
				wordlist[l].w=wtmp;
			}
		}
	}
	
#if TRACE_PREPRO
	for (l=0;l<nbword;l++) {
		rasm_printf(ae,"[%s]e=%d ",wordlist[l].w,wordlist[l].e);
		if (wordlist[l].t) rasm_printf(ae,"(%d)\n",wordlist[l].t);
	}

printf("free\n");
#endif

	MemFree(bval);
	MemFree(qval);
	MemFree(w);

	for (l=0;l<ilisting;l++) {
			MemFree(listing[l].listing);
	}	
	MemFree(listing);
	/* wordlist 
		type 0: label or instruction followed by parameter(s)
		type 1: last word of the line, last parameter of an instruction
		type 2: very last word of the list
		e tag: non zero if there is comparison or equality
	*/
	ae->wl=wordlist;
	if (param) {
		MemFree(param->filename);
	}
	if (MacroFast) MemFree(MacroFast);
	if (TABwindex) MemFree(TABwindex);
	if (TABrindex) MemFree(TABrindex);
#if TRACE_PREPRO
printf("return ae\n");
#endif
	return ae;
}

int Rasm(struct s_parameter *param)
{
	#undef FUNC
	#define FUNC "Rasm"

	struct s_assenv *ae=NULL;

	/* read and preprocess source */
	ae=PreProcessing(param->filename,0,NULL,0,param);
	/* assemble */
	return Assemble(ae,NULL,NULL,NULL);
}

/* fonction d'export */

int RasmAssemble(const char *datain, int lenin, unsigned char **dataout, int *lenout)
{
	static int cpt=0;
	struct s_assenv *ae=NULL;

	if (datain==NULL && lenin==0) return cpt; else cpt++;

	if (lenout) *lenout=0;	
	ae=PreProcessing(NULL,1,datain,lenin,NULL);
	return Assemble(ae,dataout,lenout,NULL);
}

int RasmAssembleInfo(const char *datain, int lenin, unsigned char **dataout, int *lenout, struct s_rasm_info **debug)
{
	static int cpt=0;
	struct s_assenv *ae=NULL;
	int ret;

	if (datain==NULL && lenin==0) return cpt; else cpt++;

	ae=PreProcessing(NULL,1,datain,lenin,NULL);
	ret=Assemble(ae,dataout,lenout,debug);
	return ret;
}

int RasmAssembleInfoParam(const char *datain, int lenin, unsigned char **dataout, int *lenout, struct s_rasm_info **debug, struct s_parameter *param)
{
	static int cpt=0;
	struct s_assenv *ae=NULL;
	int ret;

	if (datain==NULL && lenin==0) return cpt; else cpt++;

	ae=PreProcessing(NULL,1,datain,lenin,param);
	ret=Assemble(ae,dataout,lenout,debug);
	return ret;
}

#define AUTOTEST_PAGELABELGEN "buildsna: bank: cpt=5: ld bc,{page}miam{cpt}: bank cpt: nop: miam{cpt} nop: assert {page}miam{cpt}==0x7FC5 "

#define AUTOTEST_NOINCLUDE "truc equ 0:if truc:include'bite':endif:nop"

#define AUTOTEST_BADINCLUDE "include 'truc\n .bin' \n nop nop"

#define AUTOTEST_BADINCBIN "incl49 'truc\n .bin' \n nop nop"

#define AUTOTEST_BADINCLUDE02 "read: ldir : cp ';'"

#define AUTOTEST_SETINSIDE "ld hl,0=0xC9FB"

#define AUTOTEST_OPERATOR_CONVERSION "ld hl,10 OR 20:ld a,40 and 10:ld bc,5 MOD 2:ld a,(ix+45 xor 45)"

#define AUTOTEST_OPERATOR_MODULO "revar=46: devar=5 : var=46%5 : assert var==1: var=46 % 5 : assert var==1: var=46 mod 5 : assert var==1:" \
	"var=revar%5 : assert var==1: var=revar%devar : assert var==1: var=46%devar : assert var==1: var=revar % 5 : assert var==1:" \
	"var=revar % devar : assert var==1: var=46 % devar : assert var==1: var=revar % %101 : assert var==1: var=revar%%101 : assert var==1: nop"

#define AUTOTEST_UNDEF "mavar=10: ifdef mavar: undef mavar: endif: ifdef mavar: fail 'undef did not work': endif:nop "

#define AUTOTEST_INSTRMUSTFAILED "ld a,b,c:ldi a: ldir bc:exx hl,de:exx de:ex bc,hl:ex hl,bc:ex af,af:ex hl,hl:ex hl:exx hl: "\
	"neg b:push b:push:pop:pop c:sub ix:add ix:add:sub:di 2:ei 3:ld i,c:ld r,e:rl:rr:rlca a:sla:sll:"\
	"ldd e:lddr hl:adc ix:adc b,a:xor 12,13:xor b,1:xor:or 12,13:or b,1:or:and 12,13:and b,1:and:inc:dec"

#define AUTOTEST_VIRGULE "defb 5,5,,5"
#define AUTOTEST_VIRGULE2 "print '5,,5':nop"

#define AUTOTEST_OVERLOADMACPRM "macro test,idx: defb idx:endm:macro test2,idx:defb {idx}:endm:repeat 2,idx:test idx-1:test2 idx-1:rend"

#define AUTOTEST_IFDEFMACRO	"macro test:nop:endm:ifndef test:error:else:test:endif:ifdef test:test:else:error:endif:nop"

#define AUTOTEST_PRINTVAR "label1:   macro test, param:        print 'param {param}', {hex}{param}:    endm::    test label1: nop"

#define AUTOTEST_PRINTSPACE "idx=5: print 'grouik { idx + 3 } ':nop"

#define AUTOTEST_NOT 	"myvar=10:myvar=10+myvar:if 5!=3:else:print glop:endif:ifnot 5:print glop:else:endif:" \
						"ifnot 0:else:print glop:endif:if !(5):print glop:endif:if !(0):else:print glop:endif:" \
						"ya=!0:if ya==1:else:print glop:endif:if !5:print glop:endif:ya = 0:ya =! 0:if ya == 1:" \
						"else:print glop:endif:if ! 5:print glop:endif:if 1-!( !0 && !0):else:print glop:endif:nop" 


#define AUTOTEST_MACRO "macro glop:@glop:ld hl,@next:djnz @glop:@next:mend:macro glop2:@glop:glop:ld hl,@next:djnz @glop:glop:" \
                       "@next:mend:cpti=0:repeat:glop:cpt=0:glop:repeat:glop2:repeat 1:@glop:dec a:ld hl,@next:glop2:glop2:" \
                       "jr nz,@glop:@next:rend:cpt=cpt+1:glop2:until cpt<3:cpti=cpti+1:glop2:until cpti<3"

#define AUTOTEST_MACRO_ADV 	"idx=10:macro mac2 param1,param2:ld hl,{param1}{idx+10}{param2}:{param1}{idx+10}{param2}:djnz {param1}{idx+10}{param2}:mend: " \
							"mac2 label,45:mac2 glop,10:djnz glop2010:jp label2045"
							
#define AUTOTEST_MACROPAR	"macro unemac, param1, param2:defb '{param1}':defb {param2}:mend:unemac grouik,'grouik'"

#define AUTOTEST_OPCODES "nop::ld bc,#1234::ld (bc),a::inc bc:inc b:dec b:ld b,#12:rlca:ex af,af':add hl,bc:ld a,(bc):dec bc:" \
                         "inc c:dec c:ld c,#12:rrca::djnz $:ld de,#1234:ld (de),a:inc de:inc d:dec d:ld d,#12:rla:jr $:" \
                         "add hl,de:ld a,(de):dec de:inc e:dec e:ld e,#12:rra::jr nz,$:ld hl,#1234:ld (#1234),hl:inc hl:inc h:" \
                         "dec h:ld h,#12:daa:jr z,$:add hl,hl:ld hl,(#1234):dec hl:inc l:dec l:ld l,#12:cpl::jr nc,$:" \
                         "ld sp,#1234:ld (#1234),a:inc sp:inc (hl):dec (hl):ld (hl),#12:scf:jr c,$:add hl,sp:ld a,(#1234):" \
                         "dec sp:inc a:dec a:ld a,#12:ccf::ld b,b:ld b,c:ld b,d:ld b,e:ld b,h:ld b,l:ld b,(hl):ld b,a:ld c,b:" \
                         "ld c,c:ld c,d:ld c,e:ld c,h:ld c,l:ld c,(hl):ld c,a::ld d,b:ld d,c:ld d,d:ld d,e:ld d,h:ld d,l:" \
                         "ld d,(hl):ld d,a:ld e,b:ld e,c:ld e,d:ld e,e:ld e,h:ld e,l:ld e,(hl):ld e,a::ld h,b:ld h,c:ld h,d:" \
                         "ld h,e:ld h,h:ld h,l:ld h,(hl):ld h,a:ld l,b:ld l,c:ld l,d:ld l,e:ld l,h:ld l,l:ld l,(hl):ld l,a::" \
                         "ld (hl),b:ld (hl),c:ld (hl),d:ld (hl),e:ld (hl),h:ld (hl),l:halt:ld (hl),a:ld a,b:ld a,c:ld a,d:" \
                         "ld a,e:ld a,h:ld a,l:ld a,(hl):ld a,a::add b:add c:add d:add e:add h:add l:add (hl):add a:adc b:" \
                         "adc c:adc d:adc e:adc h:adc l:adc (hl):adc a::sub b:sub c:sub d:sub e:sub h:sub l:sub (hl):sub a:" \
                         "sbc b:sbc c:sbc d:sbc e:sbc h:sbc l:sbc (hl):sbc a::and b:and c:and d:and e:and h:and l:and (hl):" \
                         "and a:xor b:xor c:xor d:xor e:xor h:xor l:xor (hl):xor a::or b:or c:or d:or e:or h:or l:or (hl):" \
                         "or a:cp b:cp c:cp d:cp e:cp h:cp l:cp (hl):cp a::ret nz:pop bc:jp nz,#1234:jp #1234:call nz,#1234:" \
                         "push bc:add #12:rst 0:ret z:ret:jp z,#1234:nop:call z,#1234:call #1234:adc #12:rst 8::ret nc:pop de:" \
                         "jp nc,#1234:out (#12),a:call nc,#1234:push de:sub #12:rst #10:ret c:exx:jp c,#1234:in a,(#12):" \
                         "call c,#1234:nop:sbc #12:rst #18::ret po:pop hl:jp po,#1234:ex (sp),hl:call po,#1234:push hl:" \
                         "and #12:rst #20:ret pe:jp (hl):jp pe,#1234:ex de,hl:call pe,#1234:nop:xor #12:rst #28::ret p:pop af:" \
                         "jp p,#1234:di:call p,#1234:push af:or #12:rst #30:ret m:ld sp,hl:jp m,#1234:ei:call m,#1234:nop:" \
                         "cp #12:rst #38:in b,(c):out (c),b:sbc hl,bc:ld (#1234),bc:neg:retn:im 0:ld i,a:in c,(c):out (c),c:" \
                         "adc hl,bc:ld bc,(#1234):reti:ld r,a::in d,(c):out (c),d:sbc hl,de:ld (#1234),de:retn:im 1:ld a,i:" \
                         "in e,(c):out (c),e:adc hl,de:ld de,(#1234):im 2:ld a,r::in h,(c):out (c),h:sbc hl,hl:rrd:in l,(c):" \
                         "out (c),l:adc hl,hl:rld::in 0,(c):out (c),0:sbc hl,sp:ld (#1234),sp:in a,(c):out (c),a:adc hl,sp:" \
                         "ld sp,(#1234)::ldi:cpi:ini:outi:ldd:cpd:ind:outd::ldir:cpir:inir:otir:lddr:cpdr:indr:otdr::rlc b:" \
                         "rlc c:rlc d:rlc e:rlc h:rlc l:rlc (hl):rlc a:rrc b:rrc c:rrc d:rrc e:rrc h:rrc l:rrc (hl):rrc a::" \
                         "rl b:rl c:rl d:rl e:rl h:rl l:rl (hl):rl a:rr b:rr c:rr d:rr e:rr h:rr l:rr (hl):rr a:sla b:sla c:" \
                         "sla d:sla e:sla h:sla l:sla (hl):sla a:sra b:sra c:sra d:sra e:sra h:sra l:sra (hl):sra a::sll b:" \
                         "sll c:sll d:sll e:sll h:sll l:sll (hl):sll a:srl b:srl c:srl d:srl e:srl h:srl l:srl (hl):srl a::" \
                         "bit 0,b:bit 0,c:bit 0,d:bit 0,e:bit 0,h:bit 0,l:bit 0,(hl):bit 0,a::bit 1,b:bit 1,c:bit 1,d:bit 1,e:" \
                         "bit 1,h:bit 1,l:bit 1,(hl):bit 1,a::bit 2,b:bit 2,c:bit 2,d:bit 2,e:bit 2,h:bit 2,l:bit 2,(hl):" \
                         "bit 2,a::bit 3,b:bit 3,c:bit 3,d:bit 3,e:bit 3,h:bit 3,l:bit 3,(hl):bit 3,a::bit 4,b:bit 4,c:" \
                         "bit 4,d:bit 4,e:bit 4,h:bit 4,l:bit 4,(hl):bit 4,a::bit 5,b:bit 5,c:bit 5,d:bit 5,e:bit 5,h:bit 5,l:" \
                         "bit 5,(hl):bit 5,a::bit 6,b:bit 6,c:bit 6,d:bit 6,e:bit 6,h:bit 6,l:bit 6,(hl):bit 6,a::bit 7,b:" \
                         "bit 7,c:bit 7,d:bit 7,e:bit 7,h:bit 7,l:bit 7,(hl):bit 7,a::res 0,b:res 0,c:res 0,d:res 0,e:res 0,h:" \
                         "res 0,l:res 0,(hl):res 0,a::res 1,b:res 1,c:res 1,d:res 1,e:res 1,h:res 1,l:res 1,(hl):res 1,a::" \
                         "res 2,b:res 2,c:res 2,d:res 2,e:res 2,h:res 2,l:res 2,(hl):res 2,a::res 3,b:res 3,c:res 3,d:res 3,e:" \
                         "res 3,h:res 3,l:res 3,(hl):res 3,a::res 4,b:res 4,c:res 4,d:res 4,e:res 4,h:res 4,l:res 4,(hl):" \
                         "res 4,a::res 5,b:res 5,c:res 5,d:res 5,e:res 5,h:res 5,l:res 5,(hl):res 5,a::res 6,b:res 6,c:" \
                         "res 6,d:res 6,e:res 6,h:res 6,l:res 6,(hl):res 6,a::res 7,b:res 7,c:res 7,d:res 7,e:res 7,h:res 7,l:" \
                         "res 7,(hl):res 7,a::set 0,b:set 0,c:set 0,d:set 0,e:set 0,h:set 0,l:set 0,(hl):set 0,a::set 1,b:" \
                         "set 1,c:set 1,d:set 1,e:set 1,h:set 1,l:set 1,(hl):set 1,a::set 2,b:set 2,c:set 2,d:set 2,e:set 2,h:" \
                         "set 2,l:set 2,(hl):set 2,a::set 3,b:set 3,c:set 3,d:set 3,e:set 3,h:set 3,l:set 3,(hl):set 3,a::" \
                         "set 4,b:set 4,c:set 4,d:set 4,e:set 4,h:set 4,l:set 4,(hl):set 4,a::set 5,b:set 5,c:set 5,d:set 5,e:" \
                         "set 5,h:set 5,l:set 5,(hl):set 5,a::set 6,b:set 6,c:set 6,d:set 6,e:set 6,h:set 6,l:set 6,(hl):" \
                         "set 6,a::set 7,b:set 7,c:set 7,d:set 7,e:set 7,h:set 7,l:set 7,(hl):set 7,a::add ix,bc::add ix,de::" \
                         "ld ix,#1234:ld (#1234),ix:inc ix:inc xh:dec xh:ld xh,#12:add ix,ix:ld ix,(#1234):dec ix:inc xl:" \
                         "dec xl:ld xl,#12::inc (ix+#12):dec (ix+#12):ld (ix+#12),#34:add ix,sp::ld b,xh:ld b,xl:" \
                         "ld b,(ix+#12):ld c,xh:ld c,xl:ld c,(ix+#12):::ld d,xh:ld d,xl:ld d,(ix+#12):ld e,xh:ld e,xl:" \
                         "ld e,(ix+#12)::ld xh,b:ld xh,c:ld xh,d:ld xh,e:ld xh,xh:ld xh,xl:ld h,(ix+#12):ld xh,a:ld xl,b:" \
                         "ld xl,c:ld xl,d:ld xl,e:ld xl,xh:ld xl,xl:ld l,(ix+#12):ld xl,a::ld (ix+#12),b:ld (ix+#12),c:" \
                         "ld (ix+#12),d:ld (ix+#12),e:ld (ix+#12),h:ld (ix+#12),l:ld (ix+#12),a:ld a,xh:ld a,xl:" \
                         "ld a,(ix+#12)::add xh:add xl:add (ix+#12):adc xh:adc xl:adc (ix+#12)::sub xh:sub xl:sub (ix+#12):" \
                         "sbc xh:sbc xl:sbc (ix+#12)::and xh:and xl:and (ix+#12):xor xh:xor xl:xor (ix+#12)::or xh:or xl:" \
                         "or (ix+#12):cp xh:cp xl:cp (ix+#12)::pop ix:ex (sp),ix:push ix:jp (ix)::ld sp,ix:::rlc (ix+#12),b:" \
                         "rlc (ix+#12),c:rlc (ix+#12),d:rlc (ix+#12),e:rlc (ix+#12),h:rlc (ix+#12),l:rlc (ix+#12):" \
                         "rlc (ix+#12),a:rrc (ix+#12),b:rrc (ix+#12),c:rrc (ix+#12),d:rrc (ix+#12),e:rrc (ix+#12),h:" \
                         "rrc (ix+#12),l:rrc (ix+#12):rrc (ix+#12),a::rl (ix+#12),b:rl (ix+#12),c:rl (ix+#12),d:rl (ix+#12),e:" \
                         "rl (ix+#12),h:rl (ix+#12),l:rl (ix+#12):rl (ix+#12),a:rr (ix+#12),b:rr (ix+#12),c:rr (ix+#12),d:" \
                         "rr (ix+#12),e:rr (ix+#12),h:rr (ix+#12),l:rr (ix+#12):rr (ix+#12),a::sla (ix+#12),b:sla (ix+#12),c:" \
                         "sla (ix+#12),d:sla (ix+#12),e:sla (ix+#12),h:sla (ix+#12),l:sla (ix+#12):sla (ix+#12),a:" \
                         "sra (ix+#12),b:sra (ix+#12),c:sra (ix+#12),d:sra (ix+#12),e:sra (ix+#12),h:sra (ix+#12),l:" \
                         "sra (ix+#12):sra (ix+#12),a::sll (ix+#12),b:sll (ix+#12),c:sll (ix+#12),d:sll (ix+#12),e:" \
                         "sll (ix+#12),h:sll (ix+#12),l:sll (ix+#12):sll (ix+#12),a:srl (ix+#12),b:srl (ix+#12),c:" \
                         "srl (ix+#12),d:srl (ix+#12),e:srl (ix+#12),h:srl (ix+#12),l:srl (ix+#12):srl (ix+#12),a::" \
                         "bit 0,(ix+#12):bit 1,(ix+#12):bit 2,(ix+#12):bit 3,(ix+#12):bit 4,(ix+#12):bit 5,(ix+#12):" \
                         "bit 6,(ix+#12):bit 7,(ix+#12):bit 0,(ix+#12),d:bit 1,(ix+#12),b:bit 2,(ix+#12),c:bit 3,(ix+#12),d:" \
                         "bit 4,(ix+#12),e:bit 5,(ix+#12),h:bit 6,(ix+#12),l:bit 7,(ix+#12),a:::res 0,(ix+#12),b:" \
                         "res 0,(ix+#12),c:res 0,(ix+#12),d:res 0,(ix+#12),e:res 0,(ix+#12),h:res 0,(ix+#12),l:res 0,(ix+#12):" \
                         "res 0,(ix+#12),a::res 1,(ix+#12),b:res 1,(ix+#12),c:res 1,(ix+#12),d:res 1,(ix+#12),e:" \
                         "res 1,(ix+#12),h:res 1,(ix+#12),l:res 1,(ix+#12):res 1,(ix+#12),a::res 2,(ix+#12),b:" \
                         "res 2,(ix+#12),c:res 2,(ix+#12),d:res 2,(ix+#12),e:res 2,(ix+#12),h:res 2,(ix+#12),l:res 2,(ix+#12):" \
                         "res 2,(ix+#12),a::res 3,(ix+#12),b:res 3,(ix+#12),c:res 3,(ix+#12),d:res 3,(ix+#12),e:" \
                         "res 3,(ix+#12),h:res 3,(ix+#12),l:res 3,(ix+#12):res 3,(ix+#12),a::res 4,(ix+#12),b:" \
                         "res 4,(ix+#12),c:res 4,(ix+#12),d:res 4,(ix+#12),e:res 4,(ix+#12),h:res 4,(ix+#12),l:" \
                         "res 4,(ix+#12):res 4,(ix+#12),a::res 5,(ix+#12),b:res 5,(ix+#12),c:res 5,(ix+#12),d:" \
                         "res 5,(ix+#12),e:res 5,(ix+#12),h:res 5,(ix+#12),l:res 5,(ix+#12):res 5,(ix+#12),a::" \
                         "res 6,(ix+#12),b:res 6,(ix+#12),c:res 6,(ix+#12),d:res 6,(ix+#12),e:res 6,(ix+#12),h:" \
                         "res 6,(ix+#12),l:res 6,(ix+#12):res 6,(ix+#12),a::res 7,(ix+#12),b:res 7,(ix+#12),c:" \
                         "res 7,(ix+#12),d:res 7,(ix+#12),e:res 7,(ix+#12),h:res 7,(ix+#12),l:res 7,(ix+#12):" \
                         "res 7,(ix+#12),a::set 0,(ix+#12),b:set 0,(ix+#12),c:set 0,(ix+#12),d:set 0,(ix+#12),e:" \
                         "set 0,(ix+#12),h:set 0,(ix+#12),l:set 0,(ix+#12):set 0,(ix+#12),a::set 1,(ix+#12),b:" \
                         "set 1,(ix+#12),c:set 1,(ix+#12),d:set 1,(ix+#12),e:set 1,(ix+#12),h:set 1,(ix+#12),l:" \
                         "set 1,(ix+#12):set 1,(ix+#12),a::set 2,(ix+#12),b:set 2,(ix+#12),c:set 2,(ix+#12),d:" \
                         "set 2,(ix+#12),e:set 2,(ix+#12),h:set 2,(ix+#12),l:set 2,(ix+#12):set 2,(ix+#12),a::" \
                         "set 3,(ix+#12),b:set 3,(ix+#12),c:set 3,(ix+#12),d:set 3,(ix+#12),e:set 3,(ix+#12),h:" \
                         "set 3,(ix+#12),l:set 3,(ix+#12):set 3,(ix+#12),a::set 4,(ix+#12),b:set 4,(ix+#12),c:" \
                         "set 4,(ix+#12),d:set 4,(ix+#12),e:set 4,(ix+#12),h:set 4,(ix+#12),l:set 4,(ix+#12):" \
                         "set 4,(ix+#12),a::set 5,(ix+#12),b:set 5,(ix+#12),c:set 5,(ix+#12),d:set 5,(ix+#12),e:" \
                         "set 5,(ix+#12),h:set 5,(ix+#12),l:set 5,(ix+#12):set 5,(ix+#12),a::set 6,(ix+#12),b:" \
                         "set 6,(ix+#12),c:set 6,(ix+#12),d:set 6,(ix+#12),e:set 6,(ix+#12),h:set 6,(ix+#12),l:" \
                         "set 6,(ix+#12):set 6,(ix+#12),a::set 7,(ix+#12),b:set 7,(ix+#12),c:set 7,(ix+#12),d:" \
                         "set 7,(ix+#12),e:set 7,(ix+#12),h:set 7,(ix+#12),l:set 7,(ix+#12):set 7,(ix+#12),a::add iy,bc::" \
                         "add iy,de::ld iy,#1234:ld (#1234),iy:inc iy:inc yh:dec yh:ld yh,#12:add iy,iy:ld iy,(#1234):dec iy:" \
                         "inc yl:dec yl:ld yl,#12::inc (iy+#12):dec (iy+#12):ld (iy+#12),#34:add iy,sp::ld b,yh:ld b,yl:" \
                         "ld b,(iy+#12):ld c,yh:ld c,yl:ld c,(iy+#12):::ld d,yh:ld d,yl:ld d,(iy+#12):ld e,yh:ld e,yl:" \
                         "ld e,(iy+#12)::ld yh,b:ld yh,c:ld yh,d:ld yh,e:ld yh,yh:ld yh,yl:ld h,(iy+#12):ld yh,a:ld yl,b:" \
                         "ld yl,c:ld yl,d:ld yl,e:ld yl,yh:ld yl,yl:ld l,(iy+#12):ld yl,a::ld (iy+#12),b:ld (iy+#12),c:" \
                         "ld (iy+#12),d:ld (iy+#12),e:ld (iy+#12),h:ld (iy+#12),l:ld (iy+#12),a:ld a,yh:ld a,yl:" \
                         "ld a,(iy+#12)::add yh:add yl:add (iy+#12):adc yh:adc yl:adc (iy+#12)::sub yh:sub yl:" \
                         "sub (iy+#12):sbc yh:sbc yl:sbc (iy+#12)::and yh:and yl:and (iy+#12):xor yh:xor yl:xor (iy+#12)::" \
                         "or yh:or yl:or (iy+#12):cp yh:cp yl:cp (iy+#12)::pop iy:ex (sp),iy:push iy:jp (iy)::ld sp,iy::" \
                         "rlc (iy+#12),b:rlc (iy+#12),c:rlc (iy+#12),d:rlc (iy+#12),e:rlc (iy+#12),h:rlc (iy+#12),l:" \
                         "rlc (iy+#12):rlc (iy+#12),a:rrc (iy+#12),b:rrc (iy+#12),c:rrc (iy+#12),d:rrc (iy+#12),e:" \
                         "rrc (iy+#12),h:rrc (iy+#12),l:rrc (iy+#12):rrc (iy+#12),a::rl (iy+#12),b:rl (iy+#12),c:" \
                         "rl (iy+#12),d:rl (iy+#12),e:rl (iy+#12),h:rl (iy+#12),l:rl (iy+#12):rl (iy+#12),a:rr (iy+#12),b:" \
                         "rr (iy+#12),c:rr (iy+#12),d:rr (iy+#12),e:rr (iy+#12),h:rr (iy+#12),l:rr (iy+#12):rr (iy+#12),a::" \
                         "sla (iy+#12),b:sla (iy+#12),c:sla (iy+#12),d:sla (iy+#12),e:sla (iy+#12),h:sla (iy+#12),l:" \
                         "sla (iy+#12):sla (iy+#12),a:sra (iy+#12),b:sra (iy+#12),c:sra (iy+#12),d:sra (iy+#12),e:" \
                         "sra (iy+#12),h:sra (iy+#12),l:sra (iy+#12):sra (iy+#12),a::sll (iy+#12),b:sll (iy+#12),c:" \
                         "sll (iy+#12),d:sll (iy+#12),e:sll (iy+#12),h:sll (iy+#12),l:sll (iy+#12):sll (iy+#12),a:" \
                         "srl (iy+#12),b:srl (iy+#12),c:srl (iy+#12),d:srl (iy+#12),e:srl (iy+#12),h:srl (iy+#12),l:" \
                         "srl (iy+#12):srl (iy+#12),a::bit 0,(iy+#12):bit 1,(iy+#12):bit 2,(iy+#12):bit 3,(iy+#12):" \
                         "bit 4,(iy+#12):bit 5,(iy+#12):bit 6,(iy+#12):bit 7,(iy+#12)::res 0,(iy+#12),b:res 0,(iy+#12),c:" \
                         "res 0,(iy+#12),d:res 0,(iy+#12),e:res 0,(iy+#12),h:res 0,(iy+#12),l:res 0,(iy+#12):" \
                         "res 0,(iy+#12),a::res 1,(iy+#12),b:res 1,(iy+#12),c:res 1,(iy+#12),d:res 1,(iy+#12),e:" \
                         "res 1,(iy+#12),h:res 1,(iy+#12),l:res 1,(iy+#12):res 1,(iy+#12),a::res 2,(iy+#12),b:" \
                         "res 2,(iy+#12),c:res 2,(iy+#12),d:res 2,(iy+#12),e:res 2,(iy+#12),h:res 2,(iy+#12),l:" \
                         "res 2,(iy+#12):res 2,(iy+#12),a::res 3,(iy+#12),b:res 3,(iy+#12),c:res 3,(iy+#12),d:" \
                         "res 3,(iy+#12),e:res 3,(iy+#12),h:res 3,(iy+#12),l:res 3,(iy+#12):res 3,(iy+#12),a::" \
                         "res 4,(iy+#12),b:res 4,(iy+#12),c:res 4,(iy+#12),d:res 4,(iy+#12),e:res 4,(iy+#12),h:" \
                         "res 4,(iy+#12),l:res 4,(iy+#12):res 4,(iy+#12),a::res 5,(iy+#12),b:res 5,(iy+#12),c:" \
                         "res 5,(iy+#12),d:res 5,(iy+#12),e:res 5,(iy+#12),h:res 5,(iy+#12),l:res 5,(iy+#12):" \
                         "res 5,(iy+#12),a::res 6,(iy+#12),b:res 6,(iy+#12),c:res 6,(iy+#12),d:res 6,(iy+#12),e:" \
                         "res 6,(iy+#12),h:res 6,(iy+#12),l:res 6,(iy+#12):res 6,(iy+#12),a::res 7,(iy+#12),b:" \
                         "res 7,(iy+#12),c:res 7,(iy+#12),d:res 7,(iy+#12),e:res 7,(iy+#12),h:res 7,(iy+#12),l:" \
                         "res 7,(iy+#12):res 7,(iy+#12),a::set 0,(iy+#12),b:set 0,(iy+#12),c:set 0,(iy+#12),d:" \
                         "set 0,(iy+#12),e:set 0,(iy+#12),h:set 0,(iy+#12),l:set 0,(iy+#12):set 0,(iy+#12),a::" \
                         "set 1,(iy+#12),b:set 1,(iy+#12),c:set 1,(iy+#12),d:set 1,(iy+#12),e:set 1,(iy+#12),h:" \
                         "set 1,(iy+#12),l:set 1,(iy+#12):set 1,(iy+#12),a::set 2,(iy+#12),b:set 2,(iy+#12),c:" \
                         "set 2,(iy+#12),d:set 2,(iy+#12),e:set 2,(iy+#12),h:set 2,(iy+#12),l:set 2,(iy+#12):" \
                         "set 2,(iy+#12),a::set 3,(iy+#12),b:set 3,(iy+#12),c:set 3,(iy+#12),d:set 3,(iy+#12),e:" \
                         "set 3,(iy+#12),h:set 3,(iy+#12),l:set 3,(iy+#12):set 3,(iy+#12),a::set 4,(iy+#12),b:" \
                         "set 4,(iy+#12),c:set 4,(iy+#12),d:set 4,(iy+#12),e:set 4,(iy+#12),h:set 4,(iy+#12),l:" \
                         "set 4,(iy+#12):set 4,(iy+#12),a::set 5,(iy+#12),b:set 5,(iy+#12),c:set 5,(iy+#12),d:" \
                         "set 5,(iy+#12),e:set 5,(iy+#12),h:set 5,(iy+#12),l:set 5,(iy+#12):set 5,(iy+#12),a::" \
                         "set 6,(iy+#12),b:set 6,(iy+#12),c:set 6,(iy+#12),d:set 6,(iy+#12),e:set 6,(iy+#12),h:" \
                         "set 6,(iy+#12),l:set 6,(iy+#12):set 6,(iy+#12),a::set 7,(iy+#12),b:set 7,(iy+#12),c:" \
                         "set 7,(iy+#12),d:set 7,(iy+#12),e:set 7,(iy+#12),h:set 7,(iy+#12),l:set 7,(iy+#12):" \
                         "set 7,(iy+#12),a:"

#define AUTOTEST_LABNUM 	"mavar=67:label{mavar}truc:ld hl,7+2*label{mavar}truc:mnt=1234567:lab2{mavar}{mnt}:" \
				"ld de,lab2{mavar}{mnt}:lab3{mavar}{mnt}h:ld de,lab3{mavar}{mnt}h"

#define AUTOTEST_EQUNUM		"mavar = 9:monlabel{mavar+5}truc:unalias{mavar+5}heu equ 50:autrelabel{unalias14heu}:ld hl,autrelabel50"

#define AUTOTEST_EQUDOLLAR	" ld hl,une_equ: ld de,deux_equ: ldir: une_equ equ $+2: deux_equ equ $+4: defw 1: preums: defw 2: deuze: defw 3,4,5,6: assert une_equ == preums: assert deux_equ == deuze"


#define AUTOTEST_DELAYNUM	"macro test  label:    dw {label}:    endm:    repeat 3, idx:idx2 = idx-1:" \
				" test label_{idx2}:    rend:repeat 3, idx:label_{idx-1}:nop:rend"

#define AUTOTEST_STRUCT		"org #1000:label1 :struct male:age    defb 0:height defb 0:endstruct:struct female:" \
				"age    defb 0:height defb 0:endstruct:struct couple:struct male husband:" \
				"struct female wife:endstruct:if $-label1!=0:stop:endif:ld a,(ix+couple.wife.age):" \
				"ld bc,couple:ld bc,{sizeof}couple:struct couple mycouple:" \
				"if mycouple.husband != mycouple.husband.age:stop:endif:ld hl,mycouple:" \
				"ld hl,mycouple.wife.age:ld bc,{sizeof}mycouple:macro cmplastheight p1:" \
				"ld hl,@mymale.height:ld a,{p1}:cp (hl):ld bc,{sizeof}@mymale:ld hl,@mymale:ret:" \
				"struct male @mymale:mend:cmplastheight 5:cmplastheight 3:nop"

#define AUTOTEST_STRUCT2	"struct bite: preums defb 0: deuze defw 1: troize defs 10: endstruct:" \
				" if {sizeof}bite.preums!=1 || {sizeof}bite.deuze!=2 || {sizeof}bite.troize!=10: stop: endif: nop "

#define AUTOTEST_REPEAT		"ce=100:repeat 2,ce:repeat 5,cx:repeat 5,cy:defb cx*cy:rend:rend:rend:assert cx==6 && cy==6 && ce==3:" \
							"cpt=0:repeat:cpt=cpt+1:until cpt>4:assert cpt==5"

#define AUTOTEST_REPEATKO	"repeat 5:nop"

#define AUTOTEST_WHILEKO	"while 5:nop"

#define AUTOTEST_TICKER		"repeat 2: ticker start, mc:out (0),a:out (c),a:out (c),h:out (c),0:ticker stop, mc:if mc!=15:ld hl,bite:else:nop:endif:rend"

#define AUTOTEST_ORG		"ORG #8000,#1000:defw $:ORG $:defw $"

#define AUTOTEST_BANKORG	"bank 0:nop:org #5:nop:bank 1:unevar=10:bank 0:assert $==6:ret:bank 1:assert $==0:bank 0:assert $==7"

#define AUTOTEST_VAREQU		"label1 equ #C000:label2 equ (label1*2)/16:label3 equ label1-label2:label4 equ 15:var1=50*3+2:var2=12*label1:var3=label4-8:var4=label2:nop"

#define AUTOTEST_FORMAT		"hexa=#12A+$23B+45Ch+0x56D:deci=123.45+-78.54*2-(7-7)*2:bina=0b101010+1010b-%1111:assert hexa==3374 && deci==-33.63 && bina==37:nop"

#define AUTOTEST_CHARSET	"charset 'abcde',0:defb 'abcde':defb 'a','b','c','d','e':defb 'a',1*'b','c'*1,1*'d','e'*1:charset:" \
							"defb 'abcde':defb 'a','b','c','d','e':defb 'a',1*'b','c'*1,1*'d','e'*1"

#define AUTOTEST_CHARSET2	"charset 97,97+26,0:defb 'roua':charset:charset 97,10:defb 'roua':charset 'o',5:defb 'roua':charset 'ou',6:defb 'roua'"

#define AUTOTEST_NOCODE		"let monorg=$:NoCode:Org 0:Element1 db 0:Element2 dw 3:Element3 ds 50:Element4 defb 'rdd':Org 0:pouet defb 'nop':" \
							"Code:Org monorg:cpt=$+element2+element3+element4:defs cpt,0"

#define AUTOTEST_LZSEGMENT	"org #100:debut:jr nz,zend:lz48:repeat 128:nop:rend:lzclose:jp zend:lz48:repeat 2:dec a:jr nz,@next:ld a,5:@next:jp debut:rend:" \
							"lzclose:zend"

#define AUTOTEST_MULTILZ	"bank: jr suivant: lz48: defs 100,#FF: lzclose: suivant jr lafin: lz49: defs 100,#FF: lzclose: lafin: bank: jr preums: " \
				" preums jr suivant2: lzapu: defs 100,#FF: lzclose: suivant2 jr lafin2: lzexo: defs 100,#FF: lzclose: lafin2 "

#define AUTOTEST_MULTILZORG " tabeul: defw script0: defw script1: defw script2: " \
				"script0: org #4000,$: lz48: start1: jr end1: defs 100: djnz start1: end1: lzclose: " \
				"org $: script1: org #4000,$: lz49: start2: jr end2: defs 100: djnz start2: end2: lzclose: " \
				"org $: script2: org #4000,$: lz49: start3: jr end3: defs 100: djnz start3: end3: lzclose: " \
				"org $: ei: ret "

#define AUTOTEST_LZDEFERED	"lz48:defs 20:lzclose:defb $"


#define AUTOTEST_PAGETAG	"bankset 0:org #5000:label1:bankset 1:org #9000:label2:bankset 2:" \
							"assert {page}label1==0x7FC0:assert {page}label2==0x7FC6:assert {pageset}label1==#7FC0:assert {pageset}label2==#7FC2:nop"
							
#define AUTOTEST_PAGETAG2	"bankset 0:call maroutine:bank 4:org #C000:autreroutine:nop:" \
							"ret:bank 5:org #8000:maroutine:ldir:ret:bankset 2:org #9000:troize:nop:" \
							"assert {page}maroutine==#7FC5:assert {pageset}maroutine==#7FC2:assert {page}autreroutine==#7FC4:" \
							"assert {pageset}autreroutine==#7FC2:assert {page}troize==#7FCE:assert {pageset}troize==#7FCA"							
							
#define AUTOTEST_PAGETAG3	"buildsna:bank 2:assert {bank}$==2:assert {page}$==0x7FC0:assert {pageset}$==#7FC0:" \
							"bankset 1:org #4000:assert {bank}$==5:assert {page}$==0x7FC5:assert {pageset}$==#7FC2"
							
#define AUTOTEST_SWITCH		"mavar=4:switch mavar:case 1:nop:case 4:defb 4:case 3:defb 3:break:case 2:nop:case 4:defb 4:endswitch"

#define AUTOTEST_PREPRO0	"\n\n\n\n;bitch\n\nnop\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nxor a\n\n\n\n\n\n\n\n\n\n\n\n"

#define AUTOTEST_PREPRO1	" macro DEBUG_INK0, coul:        out (c), a: endm: ifndef NDEBUG:DEBUG_BORDER_NOPS = 0: endif"

#define AUTOTEST_PREPRO2	" nop : mycode other tartine ldir : defw tartine,other, mycode : assert tartine==other && other==mycode && mycode==1 && $==9"

#define AUTOTEST_PREPRO3	"nop\n \n /* test ' ici */\n nop /* retest */ 5\n /* bonjour\n prout\n caca\n pipi */" \
				" nop\n nop /* nop */ : nop\n ; grouik /*\n \n /* ; pouet */ ld hl,#2121\n "

#define AUTOTEST_PREPRO4	"glop=0\n nop ; glop=1\n assert glop==0\n nop\n glop=0\n nop // glop=1\n assert glop==0\n nop\n glop=0 : /* glop=1 */ nop\n" \
				"assert glop==0\n nop\n \n glop=0 : /* glop=1 // */ nop\n assert glop==0\n nop\n glop=0 : /* glop=1 ; */ nop\n" \
				"assert glop==0\n nop\n glop=0 ; /* glop=1\n nop // glop=2 /*\n assert glop==0\n nop\n"
							
#define AUTOTEST_PROXIM		"routine:.step1:jp .step2:.step2:jp .step1:deuze:nop:.step1:djnzn .step1:djnz routine.step2"							

#define AUTOTEST_TAGPRINT	"unevar=12:print 'trucmuche{unevar}':print '{unevar}':print '{unevar}encore','pouet{unevar}{unevar}':ret"

#define AUTOTEST_TAGFOLLOW	"ret:uv=1234567890:unlabel_commeca_{uv} equ pouetpouetpouettroulala:pouetpouetpouettroulala:assert unlabel_commeca_{uv}>0"

#define AUTOTEST_TAGREALLOC	"zoomscroller_scroller_height equ 10: another_super_long_equ equ 256: pinouille_super_long_useless_equ equ 0: " \
				"zz equ 1111111111: org #100: repeat zoomscroller_scroller_height,idx: " \
				"zoomscroller_buffer_line_{idx-1}_{pinouille_super_long_useless_equ} nop: zoomscroller_buffer_line_{idx-1}_{zz} nop: " \
				"rend: align 256: repeat zoomscroller_scroller_height,idx: zoomscroller_buffer_line_{idx-1}_{pinouille_super_long_useless_equ}_duplicate nop: " \
				"zoomscroller_buffer_line_{idx-1}_{zz}_duplicate nop: rend: repeat zoomscroller_scroller_height, line: idx = line - 1: " \
				"assert zoomscroller_buffer_line_{idx}_{pinouille_super_long_useless_equ} + another_super_long_equ == "\
				"zoomscroller_buffer_line_{idx}_{pinouille_super_long_useless_equ}_duplicate: " \
				"assert zoomscroller_buffer_line_{idx}_{zz} + another_super_long_equ == zoomscroller_buffer_line_{idx}_{zz}_duplicate: rend"

#define AUTOTEST_DEFUSED	"ld hl,labelused :ifdef labelused:fail 'labelexiste':endif:ifndef labelused:else:fail 'labelexiste':endif:" \
				"ifnused labelused:fail 'labelused':endif:ifused labelused:else:fail 'labelused':endif:labelused"

#define AUTOTEST_SAVEINVALID1	"nop : save'gruik',20,-100"

#define AUTOTEST_SAVEINVALID2	"nop : save'gruik',-20,100"

#define AUTOTEST_SAVEINVALID3	"nop : save'gruik',40000,30000"

#define AUTOTEST_MACROPROX	" macro unemacro: nop: endm: global_label: ld hl, .table: .table"

#define AUTOTEST_MACRO_CONF01   " nop: macro label1: nop: mend: macro label1: nop: mend:"
#define AUTOTEST_MACRO_CONF02   " label1=0: macro label1: nop: mend: nop:"
#define AUTOTEST_MACRO_CONF03   " label1 equ #100: macro label1: nop: mend: nop:"
#define AUTOTEST_MACRO_CONF04   " label1 nop: macro label1: nop: mend: nop"

#define AUTOTEST_PROXBACK	" macro grouik: @truc djnz @truc: .unprox djnz .unprox: mend:" \
				"ld b,2: unglobal nop: djnz unglobal: ld b,2: .unprox nop: djnz .unprox: beforelocal=$ :" \
				"repeat 2: @unlocal: ld b,2: .unprox nop: djnz .unprox: grouik : djnz .unprox : rend: assert .unprox < beforelocal : nop"
#define AUTOTEST_LOCAPROX	"repeat 1: @label  nop: .prox   nop: @label2 nop: djnz @label.prox: rend"

#define AUTOTEST_QUOTES		"defb 'rdd':str 'rdd':charset 'rd',0:defb '\\r\\d':str '\\r\\d'"

#define AUTOTEST_NEGATIVE	"ld a,-5: ld bc,-0x3918:ld de,0+-#3918:ld de,-#3918:var1=-5+6:var2=-#3918+#3919:assert var1+var2==2"

#define AUTOTEST_FORMULA1	"a=5:b=2:assert int(a/b)==3:assert !a+!b==0:a=a*100:b=b*100:assert a*b==100000:ld hl,a*b-65536:a=123+-5*(-6/2)-50*2<<1"

#define AUTOTEST_FORMULA2 "vala= (0.5+(4*0.5))*6:valb= int((0.5+(4*0.5))*6):nop:if vala!=valb:push erreur:endif"

#define AUTOTEST_SHIFTMAX	"a=45: a=a>>256: assert a==0:nop"

#define AUTOTEST_FRAC		"mavar=frac(5.5):assert mavar==0.5:assert frac(6.6)==0.6:assert frac(1.1)==0.1:assert frac(1)==0:assert frac(100000)==0:nop"

#define AUTOTEST_RND		"repeat:glop=rnd(20):until glop==10:nop"

/* test override control between bank and bankset in snapshot mode + temp workspace */
#define AUTOTEST_BANKSET "buildsna:bank 0:nop:bank 1:nop:bank:nop:bank 2:nop:bank 3:nop:bankset 1:nop:bank 8:nop:bank 9:nop:bank 10:nop:bank 11:nop"

#define AUTOTEST_LIMITOK "org #100:limit #102:nop:limit #103:ld a,0:protect #105,#107:limit #108:xor a:org $+3:inc a" 

#define AUTOTEST_LIMITKO	"limit #100:org #100:add ix,ix"

#define AUTOTEST_DEFS "defs 256,0"

#define AUTOTEST_LIMIT03	"limit -1 : nop"
#define AUTOTEST_LIMIT04	"limit #10001 : nop"
#define AUTOTEST_LIMIT05	"org #FFFF : ldir"
#define AUTOTEST_LIMIT06	"org #FFFF : nop"

#define AUTOTEST_LIMIT07	"org #ffff : Start:  equ $ :    di :     ld hl,#c9fb :  ld (#38),hl"

#define AUTOTEST_DELAYED_RUN "run _start:nop:_start nop"

#define AUTOTEST_INHIBITION	"if 0:ifused truc:ifnused glop:ifdef bidule:ifndef machin:ifnot 1:nop:endif:nop:else:nop:endif:endif:endif:endif:endif"

#define AUTOTEST_LZ4	"lz4:repeat 10:nop:rend:defb 'roudoudoudouoneatxkjhgfdskljhsdfglkhnopnopnopnop':lzclose"

#define AUTOTEST_INCBIN1	"incbin 'autotest_include.raw'"
#define AUTOTEST_INCBIN2	"incbin 'autotest_include.raw',1000"
#define AUTOTEST_INCBIN3	"incbin 'autotest_include.raw',0,1000"

#define AUTOTEST_LZ4_A	"lz4 : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZ4_B	"inclz4 'autotest_include.raw'"
#define AUTOTEST_LZ4_C	"inclz4 'autotest_include.raw',100"
#define AUTOTEST_LZ4_D	"lz4 : incbin 'autotest_include.raw',100 : lzclose"
#define AUTOTEST_LZ4_E	"inclz4 'autotest_include.raw',0,1000"
#define AUTOTEST_LZ4_F	"lz4 : incbin'autotest_include.raw',0,1000 : lzclose"

#define AUTOTEST_LZ48_A	"lz48 : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZ48_B	"incl48 'autotest_include.raw'"
#define AUTOTEST_LZ48_C	"incl48 'autotest_include.raw',100"
#define AUTOTEST_LZ48_D	"lz48 : incbin 'autotest_include.raw',100 : lzclose"
#define AUTOTEST_LZ48_E	"incl48 'autotest_include.raw',0,1000"
#define AUTOTEST_LZ48_F	"lz48 : incbin'autotest_include.raw',0,1000 : lzclose"

#define AUTOTEST_LZ49_A	"lz49 : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZ49_B	"incl49 'autotest_include.raw'"
#define AUTOTEST_LZ49_C	"incl49 'autotest_include.raw',100"
#define AUTOTEST_LZ49_D	"lz49 : incbin 'autotest_include.raw',100 : lzclose"
#define AUTOTEST_LZ49_E	"incl49 'autotest_include.raw',0,1000"
#define AUTOTEST_LZ49_F	"lz49 : incbin'autotest_include.raw',0,1000 : lzclose"

#define AUTOTEST_LZAPU_A	"lzapu : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZAPU_B	"incapu 'autotest_include.raw'"
#define AUTOTEST_LZAPU_C	"incapu 'autotest_include.raw',100"
#define AUTOTEST_LZAPU_D	"lzapu : incbin 'autotest_include.raw',100 : lzclose"
#define AUTOTEST_LZAPU_E	"incapu 'autotest_include.raw',0,1000"
#define AUTOTEST_LZAPU_F	"lzapu : incbin'autotest_include.raw',0,1000 : lzclose"

#define AUTOTEST_LZSA2_A	"lzsa2 : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZSA2_Abis	"lzsa2 2 : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZSA2_B	"inclzsa2 'autotest_include.raw'"
#define AUTOTEST_LZSA2_C	"inclzsa2 'autotest_include.raw',100"
#define AUTOTEST_LZSA2_D	"lzsa2 : incbin 'autotest_include.raw',100 : lzclose"
#define AUTOTEST_LZSA2_E	"inclzsa2 'autotest_include.raw',0,1000"
#define AUTOTEST_LZSA2_F	"lzsa2 : incbin'autotest_include.raw',0,1000 : lzclose"

#define AUTOTEST_LZSA1_A	"lzsa1 : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZSA1_Abis	"lzsa1 2 : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZSA1_B	"inclzsa1 'autotest_include.raw'"
#define AUTOTEST_LZSA1_C	"inclzsa1 'autotest_include.raw',100"
#define AUTOTEST_LZSA1_D	"lzsa1 : incbin 'autotest_include.raw',100 : lzclose"
#define AUTOTEST_LZSA1_E	"inclzsa1 'autotest_include.raw',0,1000"
#define AUTOTEST_LZSA1_F	"lzsa1 : incbin'autotest_include.raw',0,1000 : lzclose"

#define AUTOTEST_LZEXO_A	"lzexo : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZEXO_B	"incexo 'autotest_include.raw'"
#define AUTOTEST_LZEXO_C	"incexo 'autotest_include.raw',100"
#define AUTOTEST_LZEXO_D	"lzexo : incbin 'autotest_include.raw',100 : lzclose"
#define AUTOTEST_LZEXO_E	"incexo 'autotest_include.raw',0,1000"
#define AUTOTEST_LZEXO_F	"lzexo : incbin'autotest_include.raw',0,1000 : lzclose"

#define AUTOTEST_LZX7_A	"lzx7 : incbin 'autotest_include.raw' : lzclose"
#define AUTOTEST_LZX7_B	"inczx7 'autotest_include.raw'"
#define AUTOTEST_LZX7_C	"inczx7 'autotest_include.raw',100"
#define AUTOTEST_LZX7_D	"lzx7 : incbin 'autotest_include.raw',100 : lzclose"
#define AUTOTEST_LZX7_E	"inczx7 'autotest_include.raw',0,1000"
#define AUTOTEST_LZX7_F	"lzx7 : incbin'autotest_include.raw',0,1000 : lzclose"


#define AUTOTEST_MAXERROR	"repeat 20:aglapi:rend:nop"

//#define AUTOTEST_REAL	"defr 0,0.5,-0.5 : float 0,0.5,-0.5"

#define AUTOTEST_ENHANCED_LD	"ld h,(ix+11): ld l,(ix+10): ld h,(iy+21): ld l,(iy+20): ld b,(ix+11): ld c,(ix+10):" \
			"ld b,(iy+21): ld c,(iy+20): ld d,(ix+11): ld e,(ix+10): ld d,(iy+21): ld e,(iy+20): ld hl,(ix+10): " \
			"ld hl,(iy+20):ld bc,(ix+10):ld bc,(iy+20): ld de,(ix+10):ld de,(iy+20)"

#define AUTOTEST_ENHANCED_PUSHPOP	"push bc,de,hl,ix,iy,af:pop hl,bc,de,iy,ix,af:nop 2:" \
				"push bc:push de:push hl:push ix:push iy:push af:"\
				"pop hl:pop bc:pop de:pop iy:pop ix:pop af:nop:nop"
#define AUTOTEST_ENHANCED_LD2 "ld (ix+0),hl: ld (ix+0),de: ld (ix+0),bc: ld (iy+0),hl: ld (iy+0),de: ld (iy+0),bc:"\
"ld (ix+1),h: ld (ix+0),l: ld (ix+1),d: ld (ix+0),e: ld (ix+1),b: ld (ix+0),c: ld (iy+1),h: ld (iy+0),l: ld (iy+1),d: ld (iy+0),e: ld (iy+1),b: ld (iy+0),c"

#define AUTOTEST_ENHANCED_LD3 "ld bc,ix : ld bc,iy : ld de,ix : ld de,iy : ld ix,bc : ld ix,de : ld iy,bc : ld iy,de:"\
	"ld b,hx : ld c,lx : ld b,hy : ld c,ly : ld d,hx : ld e,lx : ld d,hy : ld e,ly :"\
	"ld hx,b : ld lx,c : ld hx,d : ld lx,e : ld hy,b : ld ly,c : ld hy,d : ld ly,e"

#define AUTOTEST_INHIBITION2 "ifdef roudoudou:macro glop bank,page,param:ld a,{bank}:ld hl,{param}{bank}:if {bank}:nop:else:exx:" \
	"endif::switch {param}:nop:case 4:nop:case {param}:nop:default:nop:break:endswitch:endif:defb 'coucou'"

#define AUTOTEST_INHIBITIONMAX "roudoudou:ifndef roudoudou:if pouet:macro glop bank,page,param:ifdef nanamouskouri:ld hl,{param}{bank}:"\
	"elseif aglapi:exx:endif:if {bank}:nop:elseif {grouik}:exx:endif:switch {bite}:nop:case {nichon}:nop:default:nop:break:endswitch:else:"\
	"ifnot {jojo}:exx:endif:endif:else:defb 'coucou':endif"
	
#define AUTOTEST_NOEXPORT	"unlabel nop:.unlocal nop:unevar=5:unequ equ 10:noexport unlabel, .unlocal, "\
							"unlabel.unlocal, unevar, unequ:enoexport unlabel, .unlocal, unlabel.unlocal, unevar, unequ"
	
#define AUTOTEST_UNDERVAR "coucou nop:_coucou nop:_mavar=5:jr _coucou+_mavar:print _mavar"

#define AUTOTEST_CODESKIP "org #100: nop: old_dollar=$: nocode: defs 10: assert $==old_dollar+10: code: "\
		"assert $==old_dollar+10: org #200: nop: old_dollar=$: nocode: defs 10: assert $==old_dollar+10: code skip: assert $==old_dollar"

#define AUTOTEST_EMBEDDED_ERRORS "nop : rien : rien : rien : glop nop : glapi nop"

#define AUTOTEST_EMBEDDED_LABELS " disarkCounter = 0:MACRO dkps:PLY_AKG_DisarkPointerRegionStart_{disarkCounter}:ENDM" \
		":MACRO dkpe\nPLY_AKG_DisarkPointerRegionEnd_{disarkCounter}:\ndisarkCounter = disarkCounter + 1:ENDM:\ndkps\ndkpe\ndkps"

#define AUTOTEST_TAGLOOP "tab1 = #100:tab2 = #200:tab3 = #300:macro genreg channel,psg:label{channel} EQU tab{psg} + channeloffset:"\
			"endm:channel=1:while channel <= 9:psg=floor(((channel-1)/3)+1):channeloffset=((channel-1) % 3)*2:"\
			"genreg {channel},{psg}:channel=channel+1:wend:assert LABEL5==#0202:assert LABEL6==#0204:assert LABEL7==#0300:"\
			"assert LABEL8==#0302:nop"

#define AUTOTEST_PLUSCOLOR " myval=0x123 : assert getr(myval)==2 : assert getb(myval)==3 : assert getg(myval)==1 : assert getv(myval)==1" \
			" : assert setr(2)+setv(1)+setb(3)==0x123 : assert setv(-2)==setv(0) && setb(220)==setb(15) : nop "

#define AUTOTEST_ASSERT " macro zem,nom,adr: overflow equ $-{adr}: assert $<{adr},{nom}: nop: mend: zem 'bidule',#2FFF :" \
			" macro zem2,nom,adr: overflow2 equ $-{adr}: assert $<{adr},{nom},{adr},overflow2: nop: mend: zem2 'bidule',#2FFF :" \
			" macro zem3,nom,adr: overflow3 equ $-{adr}: assert $<{adr},{nom},{hex}{adr},overflow3: nop: mend: zem3 'bidule',#2FFF "

#define AUTOTEST_OPERATORASSIGN "a=5 : a*=3 : assert a==15 : a/=5 : assert a==3 : a&=2 : assert a==2 : a+=10 : assert a==12 : "\
				"a-=3 : assert a==9 : a%=5 : assert a==4 : a|=1 : assert a==5 : a<<=3 : assert a==40 : "\
				"a>>=1 : assert a==20 : nop"
#define AUTOTEST_OPERATORASSIGN2 "a=1 : a+=2 : assert a==3 : repeat 2 : a+=10 : rend : assert a==23: a=1 : a-=2 : assert a==-1 : repeat 2 : a-=10 : rend : assert a==-21: " \
				"a=3 : a*=2 : assert a==6 : repeat 2 : a*=10 : rend : assert a==600: a=600 : a/=2 : assert a==300 : repeat 2 : a/=10 : rend : assert a==3 : nop"
#define AUTOTEST_OPERATORASSIGN3 "a=1 : a += 2 : assert a==3 : repeat 2 : a += 10 : rend : assert a==23: a=1 : a -= 2 : assert a==-1 : repeat 2 : a -= 10 : rend : assert a==-21: " \
				"a=3 : a *= 2 : assert a==6 : repeat 2 : a *= 10 : rend : assert a==600: a=600 : a /= 2 : assert a==300 : repeat 2 : a /= 10 : rend : assert a==3 : nop"

#define AUTOTEST_MODULE01 "org 0: nop: preums nop  : .prox1      : djnz preums : djnz .prox1 : defs 200 : djnz gourni_preums: djnz gourni_preums.prox1: djnz grouik_preums: " \
			"djnz grouik_preums.prox1: module grouik: preums nop: .prox1: djnz preums: djnz .prox1: djnz gourni_preums: module gourni: ld (.prox1),a: ld (preums.prox1),a: " \
			"preums nop: .prox1: djnz preums: djnz .prox1: djnz grouik_preums: assert preums!=1: module : plop: : module quatre: : jp plop: bip: jr bip: " \
			" : assert preums==1: module OFF: : assert preums==1 "

#define AUTOTEST_PUSHPOPGLOBAL "unglobal: nop: repeat 2: @unlocal: .prox nop: rend: .prox nop: apres=$: macro unemacro: @inside: nop: .prox: nop: assert .prox>@inside: djnz .prox: " \
			"mend: macro deuxmacros: @inside: nop: .prox: nop: unemacro: nop: djnz .prox: assert  .prox>@inside: mend: unemacro (void): assert .prox<apres: " \
			"djnz .prox: deuxmacros (void): djnz .prox: assert .prox<apres "
#define AUTOTEST_MODULE02 "unglobal: nop: .prox: nop: module un: deuxglobal: nop: .prox: nop: djnz .prox: nop: doublon: nop: module deux: troisglobal: nop: " \
			" .prox: nop: djnz .prox: nop: doublon: nop: assert doublon>troisglobal: module: jr un_doublon: jr deux_doublon: jr un_deuxglobal.prox: jr deux_troisglobal.prox "

#define AUTOTEST_GETNOP_LD "ticker start,testouille:"\
			"ld a,1: ld a,b: ld b,a: ld a,i: ld i,a: ld a,r: ld r,a:"\
			"ld a,(5): ld a,(hl): ld a,(de): ld (de),a: ld (hl),h: ld a,lx: ld lx,c:"\
			"ld (1234),a: ld hl,1234: ld bc,1234: ld ix,1234: ld hl,(123): ld bc,(123): ld ix,(123): ld sp,(123):"\
			"ld (123),hl: ld (123),de: ld (123),iy: ld (123),sp: ld yh,45: ld sp,hl: ld sp,ix:"\
			"ticker stop,testouille:"\
			"v1=getnop('ld a,1: ld a,b: ld b,a: ld a,i: ld i,a: ld a,r: ld r,a'):"\
			"v1+=getnop('ld a,(5): ld a,(hl): ld a,(de): ld (de),a: ld (hl),h: ld a,lx: ld lx,c'):"\
			"v1+=getnop('ld (1234),a: ld hl,1234: ld bc,1234: ld ix,1234: ld hl,(123): ld bc,(123): ld ix,(123): ld sp,(123)'):"\
			"v1+=getnop('ld (123),hl: ld (123),de: ld (123),iy: ld (123),sp: ld yh,45: ld sp,hl: ld sp,ix'):"\
			"assert v1==testouille"

#define AUTOTEST_TICKERNOP_FULL "ticker start,v1 : rla: rlca: rrca: rra: nop: ccf: daa: scf: cpl: exx: ei: di : ticker stop,v1:"\
"ticker start,v2 :im 0: neg : rst #10: retn : reti:ticker stop,v2:"\
"ticker start,v3 :cpir : cpdr : cpd : cpi : outi : outd : ldd :ldi :ldir:lddr:inir:indr:otir:otdr:ind:ini:ticker stop,v3:"\
"ticker start,v4 :rld:rrd:cp (hl):cp #12:cp a:cp c:cp (ix+0):cp (iy+5):cp ly:cp xl:ticker stop,v4:"\
"assert v1==getnop('rla: rlca: rrca: rra: nop: ccf: daa: scf: cpl: exx: ei: di'):"\
"assert v2==getnop('im : neg : rst : retn : reti'):"\
"assert v3==getnop('cpir : cpdr : cpd : cpi : outi : outd : ldd :ldi :ldir:lddr:inir:indr:otir:otdr:ind:ini'):"\
"assert v4==getnop('rld:rrd:cp (hl):cp #12:cp a:cp c:cp (ix+0):cp (iy+5):cp ly:cp xl'):"\
"ticker start,v5 : ex af,af' : ex hl,de : ex de,hl : ex (sp),hl : ex hl,(sp) : ex (sp),ix : ex ix,(sp) : exx : ticker stop,v5:"\
"assert v5==getnop(\"ex af,af' : ex hl,de : ex de,hl : ex (sp),hl : ex hl,(sp) : ex (sp),ix : ex ix,(sp) : exx\"):"\
"ticker start,v6 : push af : push bc : push ix : pop ix : pop bc : pop af : ticker stop,v6:"\
"ticker start,v7 : sla a : sla b : sla (hl) : sla (ix+5) : sla (ix-20),b : ticker stop,v7:"\
"assert v7==getnop('sla a : sla b : sla (hl) : sla (ix+5) : sla (ix-20),b'):"\
"ticker start,v8 : sll a : sll b : sll (hl) : sll (ix+5) : sll (ix-20),b : ticker stop,v8:"\
"assert v8==getnop('sll a : sll b : sll (hl) : sll (ix+5) : sll (ix-20),b'):"\
"ticker start,v9 : sra a : sra b : sra (hl) : sra (ix+5) : sra (ix-20),b : ticker stop,v9:"\
"assert v9==getnop('sra a : sra b : sra (hl) : sra (ix+5) : sra (ix-20),b'):"\
"ticker start,v10 : srl a : srl b : srl (hl) : srl (ix+5) : srl (ix-20),b : ticker stop,v10:"\
"assert v10==getnop('srl a : srl b : srl (hl) : srl (ix+5) : srl (ix-20),b'):"\
"ticker start,v11 : rl a : rl b : rl (hl) : rl (ix+5) : rl (ix-20),b : ticker stop,v11:"\
"assert v11==getnop('rl a : rl b : rl (hl) : rl (ix+5) : rl (ix-20),b'):"\
"ticker start,v12 : rlc a : rlc b : rlc (hl) : rlc (ix+5) : rlc (ix-20),b : ticker stop,v12:"\
"assert v12==getnop('rlc a : rlc b : rlc (hl) : rlc (ix+5) : rlc (ix-20),b'):"\
"ticker start,v13 : rr a : rr b : rr (hl) : rr (ix+5) : rr (ix-20),b : ticker stop,v13:"\
"assert v13==getnop('rr a : rr b : rr (hl) : rr (ix+5) : rr (ix-20),b'):"\
"ticker start,v14 : rrc a : rrc b : rrc (hl) : rrc (ix+5) : rrc (ix-20),b : ticker stop,v14:"\
"assert v14==getnop('rrc a : rrc b : rrc (hl) : rrc (ix+5) : rrc (ix-20),b'):"\
"ticker start,v20 : out (c),a : out (c),c : out (0),a : out (c),0 : in a,(c) : in f,(c) : in a,(0) : ticker stop,v20:"\
"assert v20==getnop('out (c),a : out (c),c : out (0),a : out (c),0 : in a,(c) : in f,(c) : in a,(0) '):"\
"ticker start,v21 : add a,a : add b : add ix,bc : add iy,sp : add hl,de : add (hl) : add xl : add (ix+3) : add #12 : ticker stop,v21:"\
"assert v21==getnop('add a,a : add b : add ix,bc : add iy,sp : add hl,de : add (hl) : add xl : add (ix+3) : add #12'):"\
"ticker start,v22 : adc hl,bc : adc hl,hl : sbc hl,hl : sbc hl,sp : sbc hl,bc : adc hl,sp : ticker stop,v22:"\
"assert v22==getnop('adc hl,bc : adc hl,hl : sbc hl,hl : sbc hl,sp : sbc hl,bc : adc hl,sp'):"\
"ticker start,v23 : sub a : sub a,b : sub c : sub #44 : sub (hl) : sub xl : sub (ix+20) : ticker stop,v23:"\
"assert v23==getnop('sub a : sub a,b : sub c : sub #44 : sub (hl) : sub xl : sub (ix+20)'):"\
"ticker start,v24 : xor a : xor b : xor c : xor #44 : xor (hl) : xor xl : xor (ix+20) : ticker stop,v24:"\
"assert v24==getnop('xor a : xor b : xor c : xor #44 : xor (hl) : xor xl : xor (ix+20)'):"\
"ticker start,v25 : and a : and b : and c : and #44 : and (hl) : and xl : and (ix+20) : ticker stop,v25:"\
"assert v25==getnop('and a : and b : and c : and #44 : and (hl) : and xl : and (ix+20)'):"\
"ticker start,v26 : or a : or b : or c : or #44 : or (hl) : or xl : or (ix+20) : ticker stop,v26:"\
"assert v26==getnop('or a : or b : or c : or #44 : or (hl) : or xl : or (ix+20)'):"\
"ticker start,v27 : bit 0,a : bit 1,b : bit 2,c : bit 3,d : bit 4,(hl) : bit 5,(ix+0) : bit 6,(ix+0),e  : ticker stop,v27:"\
"assert v27==getnop('bit 0,a : bit 1,b : bit 2,c : bit 3,d : bit 4,(hl) : bit 5,(ix+0) : bit 6,(ix+0),e'):"\
"ticker start,v28 : res 0,a : res 1,b : res 2,c : res 3,d : res 4,(hl) : res 5,(ix+0) : res 6,(ix+0),e  : ticker stop,v28:"\
"assert v28==getnop('res 0,a : res 1,b : res 2,c : res 3,d : res 4,(hl) : res 5,(ix+0) : res 6,(ix+0),e'):"\
"ticker start,v29 : set 0,a : set 1,b : set 2,c : set 3,d : set 4,(hl) : set 5,(ix+0) : set 6,(ix+0),e  : ticker stop,v29:"\
"assert v29==getnop('set 0,a : set 1,b : set 2,c : set 3,d : set 4,(hl) : set 5,(ix+0) : set 6,(ix+0),e'):"\
"ticker start,v30 : dec a : dec b : dec lx : dec bc : dec hl : dec sp : dec ix : dec (hl) : dec (ix+100) : ticker stop,v30:"\
"assert v30==getnop('dec a : dec b : dec lx : dec bc : dec hl : dec sp : dec ix : dec (hl) : dec (ix+100) '):"\
"ticker start,v31 : inc a : inc b : inc lx : inc bc : inc hl : inc sp : inc ix : inc (hl) : inc (ix+100)  : ticker stop,v31:"\
"assert v31==getnop('inc a : inc b : inc lx : inc bc : inc hl : inc sp : inc ix : inc (hl) : inc (ix+100) '):"\
"ticker start,v32 : jp 0 : jp c,0 : jp pe,0 : jp (ix) : jp (hl) : djnz $ : ticker stop,v32:"\
"assert v32==getnop('jp 0 : jp c,0 : jp pe,0 : jp (ix) : jp (hl) : djnz $'):"\
"ticker start,v40 : ld a,i : ld a,r : ld r,a : ld i,a : ld a,a : ld b,c : ld d,e : ld a,yl : ld d,yh : ld a,(bc)  : ticker stop,v40:"\
"assert v40==getnop('ld a,i : ld a,r : ld r,a : ld i,a : ld a,a : ld b,c : ld d,e : ld a,yl : ld d,yh : ld a,(bc)'):"\
"ticker start,v41 : ld a,(de) : ld a,(hl) : ld l,(hl) : ld (hl),d : ld (hl),a : ld a,#12 : ld b,#12 : ld a,(#1234) : ld (#1234),a : ticker stop,v41:"\
"assert v41==getnop('ld a,(de) : ld a,(hl) : ld l,(hl) : ld (hl),d : ld (hl),a : ld a,#12 : ld b,#12 : ld a,(#1234) : ld (#1234),a'):"\
"ticker start,v42 : ld bc,123 : ld hl,123 : ld sp,123 : ld bc,(123) : ld hl,(123) : ld sp,(123) : ld ix,123 : ld ix,(123) : ticker stop,v42:"\
"assert v42==getnop('ld bc,123 : ld hl,123 : ld sp,123 : ld bc,(123) : ld hl,(123) : ld sp,(123) : ld ix,123 : ld ix,(123)'):"\
"ticker start,v43 : ld (123),bc : ld (123),hl : ld (123),ix : ld (123),sp : ld hy,#12 : ld ly,b : ld sp,hl : ld sp,ix : ticker stop,v43:"\
"assert v43==getnop('ld (123),bc : ld (123),hl : ld (123),ix : ld (123),sp : ld hy,#12 : ld ly,b : ld sp,hl : ld sp,ix'):"\
"ticker start,v44 : ld a,(ix+0) : ld h,(ix+0) : ld (ix+0),a : ld (ix+0),l : ld (ix+0),#12 : ticker stop,v44:"\
"assert v44==getnop('ld a,(ix+0) : ld h,(ix+0) : ld (ix+0),a : ld (ix+0),l : ld (ix+0),#12')"

#define AUTOTEST_TICKER_FULL "ticker start,v1 : rla: rlca: rrca: rra: nop: ccf: daa: scf: cpl: exx: ei: di : ticker stopzx,v1:"\
"ticker start,v2 :im 0: neg : rst #10: retn : reti:ticker stopzx,v2:"\
"ticker start,v3 :cpir : cpdr : cpd : cpi : outi : outd : ldd :ldi :ldir:lddr:inir:indr:otir:otdr:ind:ini:ticker stopzx,v3:"\
"ticker start,v4 :rld:rrd::cp (hl):cp #12:cp a:cp c:cp (ix+0):cp (iy+5):cp ly:cp xl:ticker stopzx,v4:"\
"assert v1==gettick('rla: rlca: rrca: rra: nop: ccf: daa: scf: cpl: exx: ei: di'):"\
"assert v2==gettick('im : neg : rst : retn : reti'):"\
"assert v3==gettick('cpir : cpdr : cpd : cpi : outi : outd : ldd :ldi :ldir:lddr:inir:indr:otir:otdr:ind:ini'):"\
"assert v4==gettick('rld:rrd:cp (hl):cp #12:cp a:cp c:cp (ix+0):cp (iy+5):cp ly:cp xl'):"\
"ticker start,v5 : ex af,af' : ex hl,de : ex de,hl : ex (sp),hl : ex hl,(sp) : ex (sp),ix : ex ix,(sp) : exx : ticker stopzx,v5:"\
"assert v5==gettick(\"ex af,af' : ex hl,de : ex de,hl : ex (sp),hl : ex hl,(sp) : ex (sp),ix : ex ix,(sp) : exx\"):"\
"ticker start,v6 : push af : push bc : push ix : pop ix : pop bc : pop af : ticker stopzx,v6:"\
"ticker start,v7 : sla a : sla b : sla (hl) : sla (ix+5) : sla (ix-20),b : ticker stopzx,v7:"\
"assert v7==gettick('sla a : sla b : sla (hl) : sla (ix+5) : sla (ix-20),b'):"\
"ticker start,v8 : sll a : sll b : sll (hl) : sll (ix+5) : sll (ix-20),b : ticker stopzx,v8:"\
"assert v8==gettick('sll a : sll b : sll (hl) : sll (ix+5) : sll (ix-20),b'):"\
"ticker start,v9 : sra a : sra b : sra (hl) : sra (ix+5) : sra (ix-20),b : ticker stopzx,v9:"\
"assert v9==gettick('sra a : sra b : sra (hl) : sra (ix+5) : sra (ix-20),b'):"\
"ticker start,v10 : srl a : srl b : srl (hl) : srl (ix+5) : srl (ix-20),b : ticker stopzx,v10:"\
"assert v10==gettick('srl a : srl b : srl (hl) : srl (ix+5) : srl (ix-20),b'):"\
"ticker start,v11 : rl a : rl b : rl (hl) : rl (ix+5) : rl (ix-20),b : ticker stopzx,v11:"\
"assert v11==gettick('rl a : rl b : rl (hl) : rl (ix+5) : rl (ix-20),b'):"\
"ticker start,v12 : rlc a : rlc b : rlc (hl) : rlc (ix+5) : rlc (ix-20),b : ticker stopzx,v12:"\
"assert v12==gettick('rlc a : rlc b : rlc (hl) : rlc (ix+5) : rlc (ix-20),b'):"\
"ticker start,v13 : rr a : rr b : rr (hl) : rr (ix+5) : rr (ix-20),b : ticker stopzx,v13:"\
"assert v13==gettick('rr a : rr b : rr (hl) : rr (ix+5) : rr (ix-20),b'):"\
"ticker start,v14 : rrc a : rrc b : rrc (hl) : rrc (ix+5) : rrc (ix-20),b : ticker stopzx,v14:"\
"assert v14==gettick('rrc a : rrc b : rrc (hl) : rrc (ix+5) : rrc (ix-20),b'):"\
"ticker start,v20 : out (c),a : out (c),c : out (0),a : out (c),0 : in a,(c) : in f,(c) : in a,(0) : ticker stopzx,v20:"\
"assert v20==gettick('out (c),a : out (c),c : out (0),a : out (c),0 : in a,(c) : in f,(c) : in a,(0) '):"\
"ticker start,v21 : add a,a : add b : add ix,bc : add iy,sp : add hl,de : add (hl) : add xl : add (ix+3) : add #12 : ticker stopzx,v21:"\
"assert v21==gettick('add a,a : add b : add ix,bc : add iy,sp : add hl,de : add (hl) : add xl : add (ix+3) : add #12'):"\
"ticker start,v22 : adc hl,bc : adc hl,hl : sbc hl,hl : sbc hl,sp : sbc hl,bc : adc hl,sp : ticker stopzx,v22:"\
"assert v22==gettick('adc hl,bc : adc hl,hl : sbc hl,hl : sbc hl,sp : sbc hl,bc : adc hl,sp'):"\
"ticker start,v23 : sub a : sub a,b : sub c : sub #44 : sub (hl) : sub xl : sub (ix+20) : ticker stopzx,v23:"\
"assert v23==gettick('sub a : sub a,b : sub c : sub #44 : sub (hl) : sub xl : sub (ix+20)'):"\
"ticker start,v24 : xor a : xor b : xor c : xor #44 : xor (hl) : xor xl : xor (ix+20) : ticker stopzx,v24:"\
"assert v24==gettick('xor a : xor b : xor c : xor #44 : xor (hl) : xor xl : xor (ix+20)'):"\
"ticker start,v25 : and a : and b : and c : and #44 : and (hl) : and xl : and (ix+20) : ticker stopzx,v25:"\
"assert v25==gettick('and a : and b : and c : and #44 : and (hl) : and xl : and (ix+20)'):"\
"ticker start,v26 : or a : or b : or c : or #44 : or (hl) : or xl : or (ix+20) : ticker stopzx,v26:"\
"assert v26==gettick('or a : or b : or c : or #44 : or (hl) : or xl : or (ix+20)'):"\
"ticker start,v27 : bit 0,a : bit 1,b : bit 2,c : bit 3,d : bit 4,(hl) : bit 5,(ix+0) : bit 6,(ix+0),e  : ticker stopzx,v27:"\
"assert v27==gettick('bit 0,a : bit 1,b : bit 2,c : bit 3,d : bit 4,(hl) : bit 5,(ix+0) : bit 6,(ix+0),e'):"\
"ticker start,v28 : res 0,a : res 1,b : res 2,c : res 3,d : res 4,(hl) : res 5,(ix+0) : res 6,(ix+0),e  : ticker stopzx,v28:"\
"assert v28==gettick('res 0,a : res 1,b : res 2,c : res 3,d : res 4,(hl) : res 5,(ix+0) : res 6,(ix+0),e'):"\
"ticker start,v29 : set 0,a : set 1,b : set 2,c : set 3,d : set 4,(hl) : set 5,(ix+0) : set 6,(ix+0),e  : ticker stopzx,v29:"\
"assert v29==gettick('set 0,a : set 1,b : set 2,c : set 3,d : set 4,(hl) : set 5,(ix+0) : set 6,(ix+0),e'):"\
"ticker start,v30 : dec a : dec b : dec lx : dec bc : dec hl : dec sp : dec ix : dec (hl) : dec (ix+100) : ticker stopzx,v30:"\
"assert v30==gettick('dec a : dec b : dec lx : dec bc : dec hl : dec sp : dec ix : dec (hl) : dec (ix+100) '):"\
"ticker start,v31 : inc a : inc b : inc lx : inc bc : inc hl : inc sp : inc ix : inc (hl) : inc (ix+100)  : ticker stopzx,v31:"\
"assert v31==gettick('inc a : inc b : inc lx : inc bc : inc hl : inc sp : inc ix : inc (hl) : inc (ix+100) '):"\
"ticker start,v32 : jp 0 : jp c,0 : jp pe,0 : jp (ix) : jp (hl) : djnz $ : ticker stopzx,v32:"\
"assert v32==gettick('jp 0 : jp c,0 : jp pe,0 : jp (ix) : jp (hl) : djnz $'):"\
"ticker start,v40 : ld a,i : ld a,r : ld r,a : ld i,a : ld a,a : ld b,c : ld d,e : ld a,yl : ld d,yh : ld a,(bc)  : ticker stopzx,v40:"\
"assert v40==gettick('ld a,i : ld a,r : ld r,a : ld i,a : ld a,a : ld b,c : ld d,e : ld a,yl : ld d,yh : ld a,(bc)'):"\
"ticker start,v41 : ld a,(de) : ld a,(hl) : ld l,(hl) : ld (hl),d : ld (hl),a : ld a,#12 : ld b,#12 : ld a,(#1234) : ld (#1234),a : ticker stopzx,v41:"\
"assert v41==gettick('ld a,(de) : ld a,(hl) : ld l,(hl) : ld (hl),d : ld (hl),a : ld a,#12 : ld b,#12 : ld a,(#1234) : ld (#1234),a'):"\
"ticker start,v42 : ld bc,123 : ld hl,123 : ld sp,123 : ld bc,(123) : ld hl,(123) : ld sp,(123) : ld ix,123 : ld ix,(123) : ticker stopzx,v42:"\
"assert v42==gettick('ld bc,123 : ld hl,123 : ld sp,123 : ld bc,(123) : ld hl,(123) : ld sp,(123) : ld ix,123 : ld ix,(123)'):"\
"ticker start,v43 : ld (123),bc : ld (123),hl : ld (123),ix : ld (123),sp : ld hy,#12 : ld ly,b : ld sp,hl : ld sp,ix : ticker stopzx,v43:"\
"assert v43==gettick('ld (123),bc : ld (123),hl : ld (123),ix : ld (123),sp : ld hy,#12 : ld ly,b : ld sp,hl : ld sp,ix'):"\
"ticker start,v44 : ld a,(ix+0) : ld h,(ix+0) : ld (ix+0),a : ld (ix+0),l : ld (ix+0),#12 : ticker stopzx,v44:"\
"assert v44==gettick('ld a,(ix+0) : ld h,(ix+0) : ld (ix+0),a : ld (ix+0),l : ld (ix+0),#12')"


#define AUTOTEST_SNASET "buildsna:bank 0:nop:"\
	":snaset Z80_AF,0x1234 :snaset Z80_A,0x11 :snaset Z80_F,0x11 :snaset Z80_BC,0x11 :snaset Z80_B,0x11 :snaset Z80_C,0x11 :snaset Z80_DE,0x11 :snaset Z80_D,0x11"\
	":snaset Z80_E,0x11 :snaset Z80_HL,0x11 :snaset Z80_H,0x11 :snaset Z80_L,0x11 :snaset Z80_R,0x11 :snaset Z80_I,0x11 :snaset Z80_IFF0,0x11 :snaset Z80_IFF1,0x11"\
	":snaset Z80_IX,0x11 :snaset Z80_IY,0x11 :snaset Z80_IXL,0x11 :snaset Z80_IXH,0x11 :snaset Z80_IYL,0x11 :snaset Z80_IYH,0x11 :snaset Z80_SP,0x11 :snaset Z80_PC,0x11"\
	":snaset Z80_IM,0x11 :snaset Z80_AFX,0x11 :snaset Z80_AX,0x11 :snaset Z80_FX,0x11 :snaset Z80_BCX,0x11 :snaset Z80_BX,0x11 :snaset Z80_CX,0x11 :snaset Z80_DEX,0x11"\
	":snaset Z80_DX,0x11 :snaset Z80_EX,0x11 :snaset Z80_HLX,0x11 :snaset Z80_HX,0x11 :snaset Z80_LX,0x11 :snaset FDD_MOTOR,0x11 :snaset FDD_TRACK,0x11 :snaset PRNT_DATA,0x11"\
	":snaset PPI_A,0x1 :snaset PPI_B,0x1 :snaset PPI_C,0x1 :snaset PPI_CTL,0x1 :snaset INT_NUM,0x1 :snaset INT_REQ,0x11 :snaset PSG_SEL,0x1 :snaset CRTC_SEL,0x1"\
	":snaset CRTC_TYPE,0x1 :snaset CRTC_HCC,0x11 :snaset CRTC_CLC,0x11 :snaset CRTC_RLC,0x11 :snaset CRTC_VAC,0x11 :snaset CRTC_VSWC,0x11 :snaset CRTC_HSWC,0x11"\
	":snaset CRTC_STATE,0x11 :snaset GA_VSC,0x11 :snaset GA_ISC,0x11 :snaset GA_PEN,0x11 :snaset GA_ROMCFG,0x11 :snaset GA_RAMCFG,0x11 :snaset ROM_UP,0x11"\
	":snaset CRTC_REG,0,0x11 :snaset CRTC_REG,1,0x11 :snaset CRTC_REG,16,0x11 :snaset PSG_REG,0,0x11 :snaset PSG_REG,5,0x11 :snaset PSG_REG,15,0x11: bank : nop"

#define AUTOTEST_INKCONV "assert s2h_ink(0)==0x54 :assert s2h_ink(1)==0x44 :assert s2h_ink(2)==0x55 :assert s2h_ink(3)==0x5C :assert s2h_ink(4)==0x58 :assert s2h_ink(5)==0x5D "\
":assert s2h_ink(6)==0x4C :assert s2h_ink(7)==0x45 :assert s2h_ink(8)==0x4D :assert s2h_ink(9)==0x56 :assert s2h_ink(10)==0x46 :assert s2h_ink(11)==0x57 "\
":assert s2h_ink(12)==0x5E :assert s2h_ink(13)==0x40 :assert soft2hard_ink(14)==0x5F :assert soft2hard_ink(15)==0x4E :assert soft2hard_ink(16)==0x47 "\
":assert soft2hard_ink(17)==0x4F :assert soft2hard_ink(18)==0x52 :assert soft2hard_ink(19)==0x42 :assert soft2hard_ink(20)==0x53 :assert soft2hard_ink(21)==0x5A "\
":assert soft2hard_ink(22)==0x59 :assert soft2hard_ink(23)==0x5B :assert soft2hard_ink(24)==0x4A :assert soft2hard_ink(25)==0x43 :assert soft2hard_ink(26)==0x4B "\
":nop :assert h2s_ink(0)==13 :assert h2s_ink(0x40)==13 :assert h2s_ink(1)==13 :assert h2s_ink(2)==19 :assert h2s_ink(3)==25 :assert h2s_ink(4)==1 "\
":assert h2s_ink(5)==7 :assert h2s_ink(6)==10 :assert h2s_ink(7)==16 :assert h2s_ink(8)==7 :assert h2s_ink(9)==25 :assert h2s_ink(10)==24 "\
":assert h2s_ink(11)==26 :assert h2s_ink(12)==6 :assert h2s_ink(13)==8 :assert h2s_ink(14)==15 :assert hard2soft_ink(15)==17 :assert hard2soft_ink(16)==1 "\
":assert hard2soft_ink(17)==19 :assert hard2soft_ink(18)==18 :assert hard2soft_ink(19)==20 :assert hard2soft_ink(20)==0 :assert hard2soft_ink(21)==2 "\
":assert hard2soft_ink(22)==9 :assert hard2soft_ink(23)==11 :assert hard2soft_ink(24)==4 :assert hard2soft_ink(25)==22 :assert hard2soft_ink(26)==21 "\
":assert hard2soft_ink(27)==23 :assert hard2soft_ink(28)==3 :assert hard2soft_ink(29)==5 :assert h2s_ink(30)==12 :assert h2s_ink(31)==14 :nop "

#define AUTOTEST_ECPR1 "buildcpr extended : bank 32 : label1 : assert {bank}label1==32 : nop"

#define AUTOTEST_BANKPROX " buildcpr: bank 0: grouik: .tape1: nop: ld hl,{bank}.tape2: bank 1: ld hl,{bank}.tape2: defw {bank}.tape2: .tape2: nop: "

struct s_autotest_keyword {
	char *keywordtest;
	int result;
};

struct s_autotest_keyword autotest_keyword[]={
	{"ld ly,h",1}, {"ld lx,h",1}, {"ld ly,l",1}, {"ld lx,l",1}, {"ld hy,h",1}, {"ld hx,h",1}, {"ld hy,l",1}, {"ld hx,l",1},
	{"nop",0},{"nop 2",0},{"nop a",1},{"nop (hl)",1},{"nop nop",1},{"nop grouik",1},
	{"ldir",0},{"ldir 5",1},{"ldir ldir",1},{"ldir (hl)",1},{"ldir a",1},{"ldir grouik",1},
	{"ldi",0},{"ldi 5",1},{"ldi ldi",1},{"ldi (hl)",1},{"ldi a",1},{"ldi grouik",1},
	{"lddr",0},{"lddr 5",1},{"lddr lddr",1},{"lddr (hl)",1},{"lddr a",1},{"lddr groudk",1},
	{"ldd",0},{"ldd 5",1},{"ldd ldd",1},{"ldd (hl)",1},{"ldd a",1},{"ldd groudk",1},
	{"jr $",0},{"jr 0",0},{"jr jr",1},{"jr (hl)",1},{"jr a",1},
	{"jr c,$",0},{"jr c,0",0},{"jr c,jr",1},{"jr c,(hl)",1},{"jr c,a",1},
	{"jr nc,$",0},{"jr nc,0",0},{"jr nc,jr",1},{"jr nc,(hl)",1},{"jr nc,a",1},
	{"jr z,$",0},{"jr 0",0},{"jr jr",1},{"jr (hl)",1},{"jr a",1},
	{"jr nz,$",0},{"jr 0",0},{"jr jr",1},{"jr (hl)",1},{"jr a",1},
	{"jp $",0},{"jp 0",0},{"jp jp",1},{"jp (hl)",0},{"jp (ix)",0},{"jp (iy)",0},{"jp (de)",1},{"jp a",1},
	{"jp (ix+5)",1}, {"jp (ix-5)",1}, {"jp (iy-5)",1}, {"jp (iy+5)",1},
	{"jp c,$",0},{"jp c,0",0},{"jp c,jp",1},   {"jp c,(hl)",1}, {"jp c,(ix)",1}, {"jp c,(iy)",1},{"jp c,(de)",1},{"jp c,a",1},
	{"jp nc,$",0},{"jp nc,0",0},{"jp nc,jp",1},{"jp nc,(hl)",1},{"jp nc,(ix)",1},{"jp nc,(iy)",1},{"jp nc,(de)",1},{"jp nc,a",1},
	{"jp z,$",0},{"jp z,0",0},{"jp z,jp",1},   {"jp z,(hl)",1}, {"jp z,(ix)",1}, {"jp z,(iy)",1},{"jp z,(de)",1},{"jp z,a",1},
	{"jp nz,$",0},{"jp nz,0",0},{"jp nz,jp",1},{"jp nz,(hl)",1},{"jp nz,(ix)",1},{"jp nz,(iy)",1},{"jp nz,(de)",1},{"jp nz,a",1},
	{"jp pe,$",0},{"jp pe,0",0},{"jp pe,jp",1},{"jp pe,(hl)",1},{"jp pe,(ix)",1},{"jp pe,(iy)",1},{"jp pe,(de)",1},{"jp pe,a",1},
	{"jp po,$",0},{"jp po,0",0},{"jp po,jp",1},{"jp po,(hl)",1},{"jp po,(ix)",1},{"jp po,(iy)",1},{"jp po,(de)",1},{"jp po,a",1},
	{"jp p,$",0},{"jp p,0",0},{"jp p,jp",1},   {"jp p,(hl)",1}, {"jp p,(ix)",1}, {"jp p,(iy)",1},{"jp p,(de)",1},{"jp p,a",1},
	{"jp m,$",0},{"jp m,0",0},{"jp m,jp",1},   {"jp m,(hl)",1}, {"jp m,(ix)",1}, {"jp m,(iy)",1},{"jp m,(de)",1},{"jp m,a",1},
	{"ret",0},{"ret c",0},{"ret nc",0},{"ret pe",0},{"ret po",0},{"ret m",0},{"ret p",0},{"reti",0},{"ret ret",1},{"ret 5",1},{"ret (hl)",1},{"ret a",1},
	{"xor a",0},{"xor a,b",1},{"xor",1},{"xor (de)",1},{"xor (hl)",0},{"xor (bc)",1},{"xor (ix+0)",0},{"xor (iy+0)",0},{"xor xor",1},
	{"and a",0},{"and a,b",1},{"xor",1},{"and (de)",1},{"and (hl)",0},{"and (bc)",1},{"and (ix+0)",0},{"and (iy+0)",0},{"and xor",1},
	{"or a",0},{"or a,b",1},{"xor",1},{"or (de)",1},{"or (hl)",0},{"or (bc)",1},{"or (ix+0)",0},{"or (iy+0)",0},{"or xor",1},
	{"add",1},{"add a",0},{"add a,a",0},{"add add",1},{"add (hl)",0},{"add (de)",1},{"add xh",0},{"add grouik",1},
	{"add hl,ix",1},{"add hl,iy",1},{"add ix,iy",1},{"add iy,ix",1},{"add hl,0",1},{"add hl,grouik",1},{"add ix,hl",1},{"add iy,hl",1},
	{"adc",1},{"adc a",0},{"adc a,a",0},{"adc adc",1},{"adc (hl)",0},{"adc (de)",1},{"adc xh",0},{"adc grouik",1},
	{"adc hl,ix",1},{"adc hl,iy",1},{"adc ix,iy",1},{"adc iy,ix",1},{"adc hl,0",1},{"adc hl,grouik",1},{"adc ix,hl",1},{"adc iy,hl",1},
	{"sub",1},{"sub a",0},{"sub a,a",0},{"sub sub",1},{"sub (hl)",0},{"sub (de)",1},{"sub xh",0},{"sub grouik",1},
	{"sub hl,ix",1},{"sub hl,iy",1},{"sub ix,iy",1},{"sub iy,ix",1},{"sub hl,0",1},{"sub hl,grouik",1},{"sub ix,hl",1},{"sub iy,hl",1},
	{"sbc",1},{"sbc a",0},{"sbc a,a",0},{"sbc sbc",1},{"sbc (hl)",0},{"sbc (de)",1},{"sbc xh",0},{"sbc grouik",1},
	{"sbc hl,ix",1},{"sbc hl,iy",1},{"sbc ix,iy",1},{"sbc iy,ix",1},{"sbc hl,0",1},{"sbc hl,grouik",1},{"sbc ix,hl",1},{"sbc iy,hl",1},
	{"exx",0},{"exx hl",1},{"exx hl,de",1},{"exx af,af'",1},{"exx exx",1},{"exx 5",1},
	{"ex",1},{"ex af,af'",0},{"ex hl,de",0},{"ex hl,bc",1},{"ex hl,hl",1},{"ex hl,ix",1},
	{"cp",1},{"cp cp ",1},{"cp $",0},{"cp '$'",0},{"cp 'c'",0},{"cp 5",0},{"cp c",0},{"cp a,5",0},{"cp a,c",0},{"cp hl",1},{"cp (hl)",0},{"cp a,(hl)",0},{"cp (de)",1},{"cp de",1},
	{"cpi",0},{"cpi (hl)",1},{"cpi a",1},{"cpi 5",1},
	{"cpd",0},{"cpd (hl)",1},{"cpd a",1},{"cpd 5",1},
	{"cpir",0},{"cpir (hl)",1},{"cpir a",1},{"cpir 5",1},
	{"cpdr",0},{"cpdr (hl)",1},{"cpdr a",1},{"cpdr 5",1},
	{"call #1234",0},{"call call",1},{"call (hl)",1},{"call (ix)",1},{"call (iy)",1},{"call (de)",1},{"call hl",1},{"call bc",1},{"call a",1},{"call 5,5",1},
	{"rst 5",1},{"rst",1},{"rst 0",0},{"rst rst",1},{"rst (hl)",1},{"rst (ix)",1},{"rst (iy)",1},{"rst z",1},{"rst z,0",1},
	{"djnz",1},{"djnz $",0},{"djnz $,0",1},{"djnz djnz",1},{"djnz (hl)",1},
	{"djnz (ix)",1},{"djnz (iy)",1},{"djnz (bc)",1},{"djnz bc",1},{"djnz ix",1},{"djnz iy",1},{"djnz hl",1},
	{"push",1},{"push push",1},{"push pop",1},{"push af'",1},{"push (ix)",1},{"push (hl)",1},{"push (#1234)",1},{"push #1234",1},
	{"pop",1},{"pop pop",1},{"pop push",1},{"pop af'",1},{"pop (ix)",1},{"pop (hl)",1},{"pop (#1234)",1},{"pop #1234",1},
	{"set -1,a",1},{"set 9,a",1},{"set 0,xh",1},{"set 0,ix",1},{"set 0",1},{"set",1},{"set set",1},{"set 0,a,a",1},{"set 0,(ix+0),xh",1},
	{"bit -1,a",1},{"bit 9,a",1},{"bit 0,xh",1},{"bit 0,ix",1},{"bit 0",1},{"bit",1},{"bit bit",1},{"bit 0,a,a",1},{"bit 0,(ix+0),xh",1},
	{"res -1,a",1},{"res 9,a",1},{"res 0,xh",1},{"res 0,ix",1},{"res 0",1},{"res",1},{"res res",1},{"res 0,a,a",1},{"res 0,(ix+0),xh",1},
	{"srl",1},{"srl srl",1},{"srl hl",0}, /* srl hl is a kind of macro */
	{"rld a",1},{"rld (hl)",1},{"rld rld",1},{"rld 5",1},{"rld (ix)",1},
	{"rrd a",1},{"rrd (hl)",1},{"rrd rrd",1},{"rrd 5",1},{"rrd (ix)",1},
	{"cpl a",1},{"cpl (hl)",1},{"cpl cpl",1},{"cpl 0",1},
	{"daa daa daa",1},{"daa 0",1},{"daa (hl)",1},
	{"scf scf",1},{"scf 0",1},{"scf (hl)",1},
	{"ccf ccf",1},{"ccf 0",1},{"ccf (hl)",1},
	{"out",1},{"out out",1},{"out (c)",1},{"out (c),xh",1},{"out 0",1},
	{"out (c),hl",1},{"out (hl),c",1},{"out (c),(ix+0)",1},{"out (c),a,b",1},
	{"outi 0",1},{"outi (hl)",1},
	{"otir 0",1},{"otir (hl)",1},
	{"otdr 0",1},{"otdr (hl)",1},
	{"outd 0",1},{"outd (hl)",1},
	{"in",1},{"in in",1},{"in (c)",1},{"in xh,(c)",1},{"in 0",1},
	{"in hl,(c)",1},{"in c,(hl)",1},{"in (c),(ix+0)",1},{"in a,(c),b",1},
	{"ini 0",1},{"ini (hl)",1},
	{"inir 0",1},{"inir (hl)",1},
	{"indr 0",1},{"indr (hl)",1},
	{"ind 0",1},{"ind (hl)",1},
	{"di 5",1},{"di di",1},{"di hl",1},{"di a",1},
	{"ei 5",1},{"ei ei",1},{"ei hl",1},{"ei a",1},
	{"im",1},{"im 3",1},{"im -1",1},{"im (hl)",1},
	{"halt 5",1},{"reti 5",1},{"retn 5",1},{"ld i,b",1},{"ld b,i",1},

	{"repeat 5:nop:rend",0},{"repeat 100000:a=5:rend",1},{"repeat -5:nop:rend",1},{"repeat repeat:nop:rend",1},
	{"macro bidule:nop:mend:bidule",0},{"macro bidule:nop:macro glop:nop:mend:mend:bidule",1},
	{"macro bidule:nop",1},{"macro bidule:nop:mend:macro glop:nop:bidule",1},


	{"defb getnop(1)",1},{"defb getnop(-1)",1},{"defb getnop(10)",1},{"defb getnop(\"rien\")",1},{"defb getnop()",1},{"defb getnop(\"djNz\")",0},
	{"assert getnop('nop')==1 : nop",0},
	{"assert getnop('ldi')==5 : nop",0}, {"assert getnop('ldd')==5 : nop",0},
	{"assert getnop('dec a')==1 : nop",0}, {"assert getnop('dec b')==1 : nop",0}, {"assert getnop('dec c')==1 : nop",0},
	{"assert getnop('dec d')==1 : nop",0}, {"assert getnop('dec e')==1 : nop",0}, {"assert getnop('dec h')==1 : nop",0},
	{"assert getnop('dec l')==1 : nop",0}, {"assert getnop('dec xl')==2 : nop",0}, {"assert getnop('dec yl')==2 : nop",0},
	{"assert getnop('dec xh')==2 : nop",0}, {"assert getnop('dec yh')==2 : nop",0}, {"assert getnop('dec bc')==2 : nop",0},
	{"assert getnop('dec de')==2 : nop",0}, {"assert getnop('dec hl')==2 : nop",0}, {"assert getnop('dec sp')==2 : nop",0},
	{"assert getnop('dec ix')==3 : nop",0}, {"assert getnop('dec iy')==3 : nop",0}, {"assert getnop('dec (hl)')==3 : nop",0},
	{"assert getnop('dec (ix-50)')==6 : nop",0}, {"assert getnop('dec (iy+1)')==6 : nop",0},
	{"assert getnop('set 0,a')==2 : nop",0}, {"assert getnop('set 2,a')==2 : nop",0}, {"assert getnop('set 5,a')==2 : nop",0},
	{"assert getnop('set 0,b')==2 : nop",0}, {"assert getnop('set 0,c')==2 : nop",0}, {"assert getnop('set 0,d')==2 : nop",0},
	{"assert getnop('set 0,e')==2 : nop",0}, {"assert getnop('set 0,h')==2 : nop",0}, {"assert getnop('set 0,l')==2 : nop",0},
	{"assert getnop('set 0,(hl)')==4 : nop",0}, {"assert getnop('set 1,(ix+12),d')==7 : nop",0}, {"assert getnop('set 3,(iy-34),h')==7 : nop",0},
	{"assert getnop('res 0,a')==2 : nop",0}, {"assert getnop('res 2,a')==2 : nop",0}, {"assert getnop('res 5,a')==2 : nop",0},
	{"assert getnop('res 0,b')==2 : nop",0}, {"assert getnop('res 0,c')==2 : nop",0}, {"assert getnop('res 0,d')==2 : nop",0},
	{"assert getnop('res 0,e')==2 : nop",0}, {"assert getnop('res 0,h')==2 : nop",0}, {"assert getnop('res 0,l')==2 : nop",0},
	{"assert getnop('res 0,(hl)')==4 : nop",0}, {"assert getnop('res 1,(ix+12),d')==7 : nop",0}, {"assert getnop('res 3,(iy-34),h')==7 : nop",0},
	{"assert getnop('bit 0,a')==2 : nop",0}, {"assert getnop('bit 2,a')==2 : nop",0}, {"assert getnop('bit 5,a')==2 : nop",0},
	{"assert getnop('bit 0,b')==2 : nop",0}, {"assert getnop('bit 0,c')==2 : nop",0}, {"assert getnop('bit 0,d')==2 : nop",0},
	{"assert getnop('bit 0,e')==2 : nop",0}, {"assert getnop('bit 0,h')==2 : nop",0}, {"assert getnop('bit 0,l')==2 : nop",0},
	{"assert getnop('bit 0,(hl)')==3 : nop",0}, {"assert getnop('bit 1,(ix+12),d')==6 : nop",0}, {"assert getnop('bit 3,(iy-34),h')==6 : nop",0},
	{"assert getnop(\"ex af,af' \")==1 : nop",0}, {"assert getnop('ex hl,de ')==1 : nop",0}, {"assert getnop('ex de,hl ')==1 : nop",0},
	{"assert getnop('ex (sp),hl')==6 : nop",0}, {"assert getnop('ex (sp),ix ')==7 : nop",0}, {"assert getnop('ex (sp),iy ')==7 : nop",0},

	{"assert getnop('exx ')==1 : nop",0}, {"assert getnop('rla ')==1 : nop",0}, {"assert getnop('rlca')==1 : nop",0}, {"assert getnop('rrca')==1 : nop",0},
	{"assert getnop('rra')==1 : nop",0}, {"assert getnop('ccf')==1 : nop",0}, {"assert getnop('daa')==1 : nop",0}, {"assert getnop('scf')==1 : nop",0},
	{"assert getnop('cpl')==1 : nop",0}, {"assert getnop('ei')==1 : nop",0}, {"assert getnop('di')==1 : nop",0}, {"assert getnop('im')==2 : nop",0},
	{"assert getnop('neg')==2 : nop",0}, {"assert getnop('rst')==4 : nop",0}, {"assert getnop('retn')==4 : nop",0}, {"assert getnop('reti')==4 : nop",0},
	{"assert getnop('rld')==5 : nop",0}, {"assert getnop('rrd')==5 : nop",0}, {"assert getnop('outi')==5 : nop",0}, {"assert getnop('outd')==5 : nop",0},
	{"assert getnop('ind')==5 : nop",0}, {"assert getnop('ini')==5 : nop",0}, {"assert getnop(\"ret\")==3 : nop",0}, {"assert getnop(\"ret nz\")==2 : nop",0},
	{"assert getnop(\"djNz\")==3 : nop",0}, {"assert getnop(\"jr\")==3 : nop",0}, {"assert getnop(\"jr nz\")==3 : nop",0}, {"assert getnop(\"jp (ix)\")==2 : nop",0},
	{"assert getnop(\"jp (iy)\")==2 : nop",0}, {"assert getnop(\"jp (hl)\")==1 : nop",0},

	{"assert getnop(' pop af ' )==3 : nop",0}, {"assert getnop(' pop bc ' )==3 : nop",0}, {"assert getnop(' pop de ' )==3 : nop",0},
	{"assert getnop(' pop hl ' )==3 : nop",0}, {"assert getnop(' pop ix ' )==4 : nop",0}, {"assert getnop(' pop iy ' )==4 : nop",0},
	{"assert getnop(' push af ' )==4 : nop",0}, {"assert getnop(' push bc ' )==4 : nop",0}, {"assert getnop(' push de ' )==4 : nop",0},
	{"assert getnop(' push hl ' )==4 : nop",0}, {"assert getnop(' push ix ' )==5 : nop",0}, {"assert getnop(' push iy ' )==5 : nop",0},

	{"assert getnop('add a,a')==1 : nop",0}, {"assert getnop('add a,b')==1 : nop",0}, {"assert getnop('add a,c')==1 : nop",0},
	{"assert getnop('add a,d')==1 : nop",0}, {"assert getnop('add a,e')==1 : nop",0}, {"assert getnop('add a,h')==1 : nop",0}, {"assert getnop('add a,l')==1 : nop",0},
	{"assert getnop('add a')==1 : nop",0}, {"assert getnop('add b')==1 : nop",0}, {"assert getnop('add c')==1 : nop",0},
	{"assert getnop('add d')==1 : nop",0}, {"assert getnop('add e')==1 : nop",0}, {"assert getnop('add h')==1 : nop",0}, {"assert getnop('add l')==1 : nop",0},
	{"assert getnop('add a,#12')==2 : nop",0}, {"assert getnop('add #12')==2 : nop",0}, {"assert getnop('add a,(hl)')==2 : nop",0},
	{"assert getnop('add (hl)')==2 : nop",0}, {"assert getnop('add a,xl')==2 : nop",0}, {"assert getnop('add xl')==2 : nop",0},
	{"assert getnop('add hl,bc')==3 : nop",0}, {"assert getnop('add hl,de')==3 : nop",0}, {"assert getnop('add hl,hl')==3 : nop",0}, {"assert getnop('add hl,sp')==3 : nop",0},
	{"assert getnop('add ix,bc')==4 : nop",0}, {"assert getnop('add ix,de')==4 : nop",0}, {"assert getnop('add ix,ix')==4 : nop",0}, {"assert getnop('add ix,sp')==4 : nop",0},
	{"assert getnop('add a,(ix+12)' )==5 : nop",0}, {"assert getnop('add (iy-102)' )==5 : nop",0},

	{"assert getnop('adc a,a')==1 : nop",0}, {"assert getnop('adc a,b')==1 : nop",0}, {"assert getnop('adc a,c')==1 : nop",0},
	{"assert getnop('adc a,d')==1 : nop",0}, {"assert getnop('adc a,e')==1 : nop",0}, {"assert getnop('adc a,h')==1 : nop",0}, {"assert getnop('adc a,l')==1 : nop",0},
	{"assert getnop('adc a')==1 : nop",0}, {"assert getnop('adc b')==1 : nop",0}, {"assert getnop('adc c')==1 : nop",0},
	{"assert getnop('adc d')==1 : nop",0}, {"assert getnop('adc e')==1 : nop",0}, {"assert getnop('adc h')==1 : nop",0}, {"assert getnop('adc l')==1 : nop",0},
	{"assert getnop('adc a,#12')==2 : nop",0}, {"assert getnop('adc #12')==2 : nop",0}, {"assert getnop('adc a,(hl)')==2 : nop",0},
	{"assert getnop('adc (hl)')==2 : nop",0}, {"assert getnop('adc a,xl')==2 : nop",0}, {"assert getnop('adc xl')==2 : nop",0},
	{"assert getnop('adc hl,bc')==4 : nop",0}, {"assert getnop('adc hl,de')==4 : nop",0}, {"assert getnop('adc hl,hl')==4 : nop",0}, {"assert getnop('adc hl,sp')==4 : nop",0},
	{"assert getnop('adc a,(ix+12)' )==5 : nop",0}, {"assert getnop('adc (iy-102)' )==5 : nop",0},

	{"assert getnop('sbc a,a')==1 : nop",0}, {"assert getnop('sbc a,b')==1 : nop",0}, {"assert getnop('sbc a,c')==1 : nop",0},
	{"assert getnop('sbc a,d')==1 : nop",0}, {"assert getnop('sbc a,e')==1 : nop",0}, {"assert getnop('sbc a,h')==1 : nop",0}, {"assert getnop('sbc a,l')==1 : nop",0},
	{"assert getnop('sbc a')==1 : nop",0}, {"assert getnop('sbc b')==1 : nop",0}, {"assert getnop('sbc c')==1 : nop",0},
	{"assert getnop('sbc d')==1 : nop",0}, {"assert getnop('sbc e')==1 : nop",0}, {"assert getnop('sbc h')==1 : nop",0}, {"assert getnop('sbc l')==1 : nop",0},
	{"assert getnop('sbc a,#12')==2 : nop",0}, {"assert getnop('sbc #12')==2 : nop",0}, {"assert getnop('sbc a,(hl)')==2 : nop",0},
	{"assert getnop('sbc (hl)')==2 : nop",0}, {"assert getnop('sbc a,xl')==2 : nop",0}, {"assert getnop('sbc xl')==2 : nop",0},
	{"assert getnop('sbc hl,bc')==4 : nop",0}, {"assert getnop('sbc hl,de')==4 : nop",0}, {"assert getnop('sbc hl,hl')==4 : nop",0}, {"assert getnop('sbc hl,sp')==4 : nop",0},
	{"assert getnop('sbc a,(ix+12)' )==5 : nop",0}, {"assert getnop('sbc (iy-102)' )==5 : nop",0},

	{"assert getnop('sub a,a')==1 : nop",0}, {"assert getnop('sub a,b')==1 : nop",0}, {"assert getnop('sub a,c')==1 : nop",0},
	{"assert getnop('sub a,d')==1 : nop",0}, {"assert getnop('sub a,e')==1 : nop",0}, {"assert getnop('sub a,h')==1 : nop",0}, {"assert getnop('sub a,l')==1 : nop",0},
	{"assert getnop('sub a')==1 : nop",0}, {"assert getnop('sub b')==1 : nop",0}, {"assert getnop('sub c')==1 : nop",0},
	{"assert getnop('sub d')==1 : nop",0}, {"assert getnop('sub e')==1 : nop",0}, {"assert getnop('sub h')==1 : nop",0}, {"assert getnop('sub l')==1 : nop",0},
	{"assert getnop('sub a,#12')==2 : nop",0}, {"assert getnop('sub #12')==2 : nop",0}, {"assert getnop('sub a,(hl)')==2 : nop",0},
	{"assert getnop('sub (hl)')==2 : nop",0}, {"assert getnop('sub a,xl')==2 : nop",0}, {"assert getnop('sub xl')==2 : nop",0},
	{"assert getnop('sub a,(ix+12)' )==5 : nop",0}, {"assert getnop('sub (iy-102)' )==5 : nop",0},

	{"assert getnop('out (c),a' )==4 : nop",0}, {"assert getnop('out (c),b' )==4 : nop",0}, {"assert getnop('out (c),c' )==4 : nop",0},
	{"assert getnop('out (c),d' )==4 : nop",0}, {"assert getnop('out (c),e' )==4 : nop",0}, {"assert getnop('out (c),h' )==4 : nop",0},
	{"assert getnop('out (c),l' )==4 : nop",0}, {"assert getnop('out (c),0' )==4 : nop",0}, {"assert getnop('out (12),a')==3 : nop",0},
	{"assert getnop('out (c),l : out (c),0 : out (12),a')==11 : nop",0},
	{"assert getnop('in a,(c)' )==4 : nop",0}, {"assert getnop('in b,(c)' )==4 : nop",0}, {"assert getnop('in a,(0)' )==3 : nop",0},
	{"assert getnop('in a,(0) : in a,(c)' )==7 : nop",0},

	{"assert getnop('or a')==1 : nop",0}, {"assert getnop('or b')==1 : nop",0}, {"assert getnop('or c')==1 : nop",0},
	{"assert getnop('or d')==1 : nop",0}, {"assert getnop('or e')==1 : nop",0}, {"assert getnop('or h')==1 : nop",0},
	{"assert getnop('or l')==1 : nop",0}, {"assert getnop('or (hl)')==2 : nop",0}, {"assert getnop('or ixl')==2 : nop",0},
	{"assert getnop('or ixh')==2 : nop",0}, {"assert getnop('or xl')==2 : nop",0}, {"assert getnop('or xh')==2 : nop",0},
	{"assert getnop('or lx')==2 : nop",0}, {"assert getnop('or hx')==2 : nop",0}, {"assert getnop('or (ix+d)')==5 : nop",0},
	{"assert getnop('or (iy-d)')==5 : nop",0}, {"assert getnop('or a,(iy-d)')==5 : nop",0},
	{"assert getnop('xor a')==1 : nop",0}, {"assert getnop('xor b')==1 : nop",0}, {"assert getnop('xor c')==1 : nop",0},
	{"assert getnop('xor d')==1 : nop",0}, {"assert getnop('xor e')==1 : nop",0}, {"assert getnop('xor h')==1 : nop",0},
	{"assert getnop('xor l')==1 : nop",0}, {"assert getnop('xor (hl)')==2 : nop",0}, {"assert getnop('xor ixl')==2 : nop",0},
	{"assert getnop('xor ixh')==2 : nop",0}, {"assert getnop('xor xl')==2 : nop",0}, {"assert getnop('xor xh')==2 : nop",0},
	{"assert getnop('xor lx')==2 : nop",0}, {"assert getnop('xor hx')==2 : nop",0}, {"assert getnop('xor (ix+d)')==5 : nop",0},
	{"assert getnop('xor (iy-d)')==5 : nop",0}, {"assert getnop('xor a,(iy-d)')==5 : nop",0},
	{"assert getnop('and a')==1 : nop",0}, {"assert getnop('and b')==1 : nop",0}, {"assert getnop('and c')==1 : nop",0},
	{"assert getnop('and d')==1 : nop",0}, {"assert getnop('and e')==1 : nop",0}, {"assert getnop('and h')==1 : nop",0},
	{"assert getnop('and l')==1 : nop",0}, {"assert getnop('and (hl)')==2 : nop",0}, {"assert getnop('and ixl')==2 : nop",0},
	{"assert getnop('and ixh')==2 : nop",0}, {"assert getnop('and xl')==2 : nop",0}, {"assert getnop('and xh')==2 : nop",0},
	{"assert getnop('and lx')==2 : nop",0}, {"assert getnop('and hx')==2 : nop",0}, {"assert getnop('and (ix+d)')==5 : nop",0},
	{"assert getnop('and (iy-d)')==5 : nop",0}, {"assert getnop('and a,(iy-d)')==5 : nop",0},

	{"assert getnop('rr a')==2 : nop",0}, {"assert getnop('rr b')==2 : nop",0}, {"assert getnop('rr (hl)')==4 : nop",0},
	{"assert getnop('rr (ix+5)')==7 : nop",0}, {"assert getnop('rr (iy-111)')==7 : nop",0}, {"assert getnop('rr (ix+2),b')==7 : nop",0},
	{"assert getnop('rl a')==2 : nop",0}, {"assert getnop('rl b')==2 : nop",0}, {"assert getnop('rl (hl)')==4 : nop",0},
	{"assert getnop('rl (ix+5)')==7 : nop",0}, {"assert getnop('rl (iy-111)')==7 : nop",0}, {"assert getnop('rl (ix+2),b')==7 : nop",0},
	{"assert getnop('rrc a')==2 : nop",0}, {"assert getnop('rrc b')==2 : nop",0}, {"assert getnop('rrc (hl)')==4 : nop",0},
	{"assert getnop('rrc (ix+5)')==7 : nop",0}, {"assert getnop('rrc (iy-111)')==7 : nop",0}, {"assert getnop('rrc (ix+2),b')==7 : nop",0},
	{"assert getnop('rlc a')==2 : nop",0}, {"assert getnop('rlc b')==2 : nop",0}, {"assert getnop('rlc (hl)')==4 : nop",0},
	{"assert getnop('rlc (ix+5)')==7 : nop",0}, {"assert getnop('rlc (iy-111)')==7 : nop",0}, {"assert getnop('rlc (ix+2),b')==7 : nop",0},
	{"assert getnop('sla a')==2 : nop",0}, {"assert getnop('sla b')==2 : nop",0}, {"assert getnop('sla (hl)')==4 : nop",0},
	{"assert getnop('sla (ix+5)')==7 : nop",0}, {"assert getnop('sla (iy-111)')==7 : nop",0}, {"assert getnop('sla (ix+2),b')==7 : nop",0},
	{"assert getnop('sra a')==2 : nop",0}, {"assert getnop('sra b')==2 : nop",0}, {"assert getnop('sra (hl)')==4 : nop",0},
	{"assert getnop('sra (ix+5)')==7 : nop",0}, {"assert getnop('sra (iy-111)')==7 : nop",0}, {"assert getnop('sra (ix+2),b')==7 : nop",0},
	{"assert getnop('sll a')==2 : nop",0}, {"assert getnop('sll b')==2 : nop",0}, {"assert getnop('sll (hl)')==4 : nop",0},
	{"assert getnop('sll (ix+5)')==7 : nop",0}, {"assert getnop('sll (iy-111)')==7 : nop",0}, {"assert getnop('sll (ix+2),b')==7 : nop",0},
	{"assert getnop('srl a')==2 : nop",0}, {"assert getnop('srl b')==2 : nop",0}, {"assert getnop('srl (hl)')==4 : nop",0},
	{"assert getnop('srl (ix+5)')==7 : nop",0}, {"assert getnop('srl (iy-111)')==7 : nop",0}, {"assert getnop('srl (ix+2),b')==7 : nop",0},

	/* wrong snapshot settings */
	{"buildsna:bank 0:nop:snaset crtc_type",1},
	{"buildsna:bank 0:nop:snaset crtc_type,3,2",1},
	{"buildsna:bank 0:nop:snaset crtc_reg,3",1},
	{"buildsna:bank 0:nop:snaset crtc_reg,20,0",1},
	{"buildsna:bank 0:nop:snaset crtc_reg,-1,0",1},
	{"buildsna:bank 0:nop:snaset psg_reg,3",1},
	{"buildsna:bank 0:nop:snaset psg_reg,20,0",1},
	{"buildsna:bank 0:nop:snaset psg_reg,-1,0",1},
	{"buildsna:bank 0:nop:snaset ga_pal,3",1},
	{"buildsna:bank 0:nop:snaset ga_pal,20,0",1},
	{"buildsna:bank 0:nop:snaset ga_pal,-1,0",1},
	{"glop=rnd(5):nop",0},{"glop=rnd(0):nop",1},{"glop=rnd():nop",1},{"glop=rnd(-1):nop",1},{"pifou=8:glop=rnd(pifou):defb glop",0},
	/* wrong include usage */
	{"incbin",1},{"include",1},{"incbin'",1},{"include'",1},{"'include",1}, {"'incbin",1},
	{"incexo",1},{"inczx7",1},{"incaaa",0}, {"inc",1}, {"incl48",1},{"incl49",1},{"incapu",1},{"inclz4",1},
	{"incexo'",1},{"inczx7'",1},{"incaaa'",1}, {"inc'",1}, {"incl48'",1},{"incl49'",1},{"incapu'",1},{"inclz4'",1},
	{"incexb",0}, {"includee",0}, {"incexb'",1}, {"includee'",1}, 
	/*
	{"",},{"",},{"",},{"",},{"",},
	{"",},{"",},{"",},{"",},{"",},{"",},
	{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},
	{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},
	{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},{"",},
	*/
	{NULL,0}
};

void MiniDump(unsigned char *opcode, int opcodelen) {
	#undef FUNC
	#define FUNC "MiniDump"

	int i;
	printf("%d byte%s to dump\n",opcodelen,opcodelen>1?"s":"");
	for (i=0;i<opcodelen;i++) {
		printf("%02X ",opcode[i]);
	}
	printf("\n");
}
						
void RasmAutotest(void)
{
	#undef FUNC
	#define FUNC "RasmAutotest"

	struct s_parameter param;
	struct s_rasm_info *debug;
	unsigned char *opcode=NULL;
	int opcodelen,ret;
	int cpt=0,chk,i,j,k,idx,sko=0;
	char *tmpstr3,**tmpsplit;

#ifdef RDD
	printf("\n%d bytes\n",_static_library_memory_used);
#endif
	
	/* Autotest preprocessing */
	ret=RasmAssemble(AUTOTEST_VIRGULE,strlen(AUTOTEST_VIRGULE),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (double comma must trigger an error) ret=%d\n",cpt,ret);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing preprocessor comma management in a string OK\n");
	
	ret=RasmAssemble(AUTOTEST_VIRGULE2,strlen(AUTOTEST_VIRGULE2),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (double comma in a string must be OK)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing preprocessor comma management in a string OK\n");
	
	ret=RasmAssemble(AUTOTEST_PREPRO0,strlen(AUTOTEST_PREPRO0),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (uninitialize memory read!)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing preprocessor non regression OK\n");
	
	ret=RasmAssemble(AUTOTEST_PREPRO1,strlen(AUTOTEST_PREPRO1),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (freestyle case 1)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing preprocessor freestyle OK\n");
	
	ret=RasmAssemble(AUTOTEST_PREPRO2,strlen(AUTOTEST_PREPRO2),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (freestyle case 2)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing preprocessor freestyle variation 2 OK\n");

	ret=RasmAssemble(AUTOTEST_PREPRO3,strlen(AUTOTEST_PREPRO3),&opcode,&opcodelen);
	if (!ret && opcodelen==12 && opcode[11]==0x21) {} else {printf("Autotest %03d ERROR (multi-line comment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing multi-line comments 1 OK\n");

	ret=RasmAssemble(AUTOTEST_PREPRO4,strlen(AUTOTEST_PREPRO4),&opcode,&opcodelen);
	if (!ret && opcodelen==12) {} else {printf("Autotest %03d ERROR (multi-line comment 2)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing multi-line comments 2 OK\n");

	ret=RasmAssemble(AUTOTEST_OPERATOR_CONVERSION,strlen(AUTOTEST_OPERATOR_CONVERSION),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (maxam operator conversion)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing Maxam operator conversion OK\n");
	
	ret=RasmAssemble(AUTOTEST_OPERATOR_MODULO,strlen(AUTOTEST_OPERATOR_MODULO),&opcode,&opcodelen);
	if (!ret && opcodelen==1) {} else {printf("Autotest %03d ERROR (modulo operator conversion)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing modulo operator conversion OK\n");
	
	ret=RasmAssemble(AUTOTEST_NOINCLUDE,strlen(AUTOTEST_NOINCLUDE),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (include missing file)\n",cpt);exit(-1);} // file not found MUST trigger an error! => NOT ANYMORE
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing include on a missing file OK\n");
	
	ret=RasmAssemble(AUTOTEST_BADINCLUDE,strlen(AUTOTEST_BADINCLUDE),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (bad include test)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing include filename on multiple lines OK\n");
	
	ret=RasmAssemble(AUTOTEST_BADINCLUDE02,strlen(AUTOTEST_BADINCLUDE02),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (bad include test 2)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing wrong READ usage OK\n");
	
	ret=RasmAssemble(AUTOTEST_BADINCBIN,strlen(AUTOTEST_BADINCBIN),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (bad incbin test)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing incbin filename on multiple lines OK\n");
	
	ret=RasmAssemble(AUTOTEST_FORMAT,strlen(AUTOTEST_FORMAT),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (digit formats)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing digit formats OK\n");
	
	/* Autotest assembling */
	ret=RasmAssemble(AUTOTEST_OPCODES,strlen(AUTOTEST_OPCODES),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (all opcodes)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing all opcodes OK\n");

	/********************************************
	           Autotest assembly options
	*********************************************/

	#define AUTOTEST_PARAM_AMPER "ld a,&10 : assert &10==#10"
	memset(&param,0,sizeof(struct s_parameter));
	param.noampersand=1;
	ret=RasmAssembleInfoParam(AUTOTEST_PARAM_AMPER,strlen(AUTOTEST_PARAM_AMPER),&opcode,&opcodelen,&debug,&param);
	if (!ret) {} else {printf("Autotest %03d ERROR (testing compilation option -amper)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing command line parameter -amper OK\n");

	#define AUTOTEST_PARAM_MAXAM "aaa=10+5*3 : assert aaa==45 : nop"
	memset(&param,0,sizeof(struct s_parameter));
	param.rough=0.0; // 0.5 as default
	ret=RasmAssembleInfoParam(AUTOTEST_PARAM_MAXAM,strlen(AUTOTEST_PARAM_MAXAM),&opcode,&opcodelen,&debug,&param);
	if (!ret) {} else {printf("Autotest %03d ERROR (testing compilation option -m)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing command line parameter -m OK\n");

	#define AUTOTEST_PARAM_DAMS ".grouik : djnz grouik"
	memset(&param,0,sizeof(struct s_parameter));
	param.dams=1;
	ret=RasmAssembleInfoParam(AUTOTEST_PARAM_DAMS,strlen(AUTOTEST_PARAM_DAMS),&opcode,&opcodelen,&debug,&param);
	if (!ret) {} else {printf("Autotest %03d ERROR (testing compilation option -dams)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing command line parameter -dams OK\n");

	#define AUTOTEST_PARAM_PASMO "org #1234 : defw $,$"
	memset(&param,0,sizeof(struct s_parameter));
	param.pasmo=1;
	ret=RasmAssembleInfoParam(AUTOTEST_PARAM_PASMO,strlen(AUTOTEST_PARAM_PASMO),&opcode,&opcodelen,&debug,&param);
	if (!ret && opcodelen==4 && opcode[0]==opcode[2] && opcode[1]==opcode[3]) {} else {printf("Autotest %03d ERROR (testing compilation option -pasmo)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing command line parameter -pasmo OK\n");

	#define AUTOTEST_PARAM_VOID "macro grouik : nop : mend : grouik"
	memset(&param,0,sizeof(struct s_parameter));
	param.macrovoid=1;
	ret=RasmAssembleInfoParam(AUTOTEST_PARAM_VOID,strlen(AUTOTEST_PARAM_VOID),&opcode,&opcodelen,&debug,&param);
	if (ret) {} else {printf("Autotest %03d ERROR (testing compilation option -void)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing command line parameter -void OK\n");

	#define AUTOTEST_PARAM_VOID2 "macro grouik : nop : mend : grouik (void)"
	memset(&param,0,sizeof(struct s_parameter));
	param.macrovoid=1;
	ret=RasmAssembleInfoParam(AUTOTEST_PARAM_VOID2,strlen(AUTOTEST_PARAM_VOID2),&opcode,&opcodelen,&debug,&param);
	if (!ret) {} else {printf("Autotest %03d ERROR (testing compilation option (bis) -void)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing command line parameter (bis) -void OK\n");

	#define AUTOTEST_PARAM_TWE "ld a,300"
	memset(&param,0,sizeof(struct s_parameter));
	param.erronwarn=1;
	ret=RasmAssembleInfoParam(AUTOTEST_PARAM_TWE,strlen(AUTOTEST_PARAM_TWE),&opcode,&opcodelen,&debug,&param);
	if (ret) {} else {printf("Autotest %03d ERROR (testing compilation option -twe)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing command line parameter -twe OK\n");

/*
	#define AUTOTEST_PARAM_ ""
	memset(&param,0,sizeof(struct s_parameter));
	param.dams=1;
	ret=RasmAssembleInfoParam(AUTOTEST_PARAM_,strlen(AUTOTEST_PARAM_),&opcode,&opcodelen,&debug,&param);
	if (!ret) {} else {printf("Autotest %03d ERROR (testing compilation option -)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing command line parameter - OK\n");

	#define AUTOTEST_PARAM_ ""
	memset(&param,0,sizeof(struct s_parameter));
	param.dams=1;
	ret=RasmAssembleInfoParam(AUTOTEST_PARAM_,strlen(AUTOTEST_PARAM_),&opcode,&opcodelen,&debug,&param);
	if (!ret) {} else {printf("Autotest %03d ERROR (testing compilation option -)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing command line parameter - OK\n");
*/



	/* Autotest single instruction writes that must failed */
	tmpstr3=TxtStrDup(AUTOTEST_INSTRMUSTFAILED);
	tmpsplit=TxtSplitWithChar(tmpstr3,':');

	for (i=0;tmpsplit[i];i++) {
		ret=RasmAssemble(tmpsplit[i],strlen(tmpsplit[i]),&opcode,&opcodelen);
		if (ret) {} else {printf("Autotest %03d ERROR (opcodes that must fail) -> [%s]\n",cpt,tmpsplit[i]);exit(-1);}
		if (opcode) MemFree(opcode);opcode=NULL;
	}

	MemFree(tmpstr3);FreeFields(tmpsplit);
	cpt++;
printf("testing various opcode tests OK\n");

	idx=0;
	while (autotest_keyword[idx].keywordtest) {
		ret=RasmAssemble(autotest_keyword[idx].keywordtest,strlen(autotest_keyword[idx].keywordtest),&opcode,&opcodelen);
		if (!ret && !autotest_keyword[idx].result) {
		} else if (ret && autotest_keyword[idx].result) {
		} else {
			printf("Autotest %03d ERROR ([%s] test) has %d error%s instead of %serror expected\n",cpt,autotest_keyword[idx].keywordtest,ret,ret>1?"s":"",autotest_keyword[idx].result?"":"no ");
			sko++;
		}
		if (opcode) MemFree(opcode);opcode=NULL;
		idx++;
	}
	if (sko) {
		printf("moar various opcode tests did not pass!\n");
		exit(-1);
	}

	cpt++;
printf("testing moar various opcode tests OK\n");
	
	ret=RasmAssemble(AUTOTEST_ORG,strlen(AUTOTEST_ORG),&opcode,&opcodelen);
	if (!ret && opcodelen==4 && opcode[1]==0x80 && opcode[2]==2 && opcode[3]==0x10) {} else {printf("Autotest %03d ERROR (ORG relocation)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing ORG relocation OK\n");

	ret=RasmAssemble(AUTOTEST_MAXERROR,strlen(AUTOTEST_MAXERROR),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (must return an error code!)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing error code OK\n");

	ret=RasmAssemble(AUTOTEST_BANKORG,strlen(AUTOTEST_BANKORG),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (BANK org adr)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing BANK/ORG OK\n");

	ret=RasmAssemble(AUTOTEST_LIMITOK,strlen(AUTOTEST_LIMITOK),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (limit ok)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LIMIT 1 OK\n");
	
	ret=RasmAssemble(AUTOTEST_LIMITKO,strlen(AUTOTEST_LIMITKO),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (out of limit)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LIMIT 2 OK\n");
	
	ret=RasmAssemble(AUTOTEST_LIMIT03,strlen(AUTOTEST_LIMIT03),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (limit: negative limit)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing inegative LIMIT OK\n");
	
	ret=RasmAssemble(AUTOTEST_LIMIT04,strlen(AUTOTEST_LIMIT04),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (limit: max limit test)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing max LIMIT OK\n");
	
	ret=RasmAssemble(AUTOTEST_LIMIT05,strlen(AUTOTEST_LIMIT05),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (limit: ldir in #FFFF)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing 16 bits opcode overriding LIMIT OK\n");
	
	ret=RasmAssemble(AUTOTEST_LIMIT06,strlen(AUTOTEST_LIMIT06),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (limit: nop in #FFFF)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing opcode overriding LIMIT OK\n");
	
	ret=RasmAssemble(AUTOTEST_LIMIT07,strlen(AUTOTEST_LIMIT07),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (limit: ld hl,#1234 in #FFFF)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing opcode with variable overriding LIMIT OK\n");
	
	ret=RasmAssemble(AUTOTEST_DELAYED_RUN,strlen(AUTOTEST_DELAYED_RUN),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (delayed RUN set)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing delayed RUN OK\n");
	
	ret=RasmAssemble(AUTOTEST_LZSEGMENT,strlen(AUTOTEST_LZSEGMENT),&opcode,&opcodelen);
	if (!ret && opcodelen==23 && opcode[1]==21 && opcode[9]==23) {} else {printf("Autotest %03d ERROR (LZ segment relocation)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ segment relocation OK\n");

#ifndef NOAPULTRA
	ret=RasmAssemble(AUTOTEST_MULTILZ,strlen(AUTOTEST_MULTILZ),&opcode,&opcodelen);
	if (!ret && opcodelen==43 && opcode[3]==6 && opcode[11]==0x1F) {} else {printf("Autotest %03d ERROR (multi-LZ segment relocation with multiple BANK)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing multi-LZ segment relocation with multiple banks OK\n");
#else
printf("*** multi-LZ segment test disabled as there is no APUltra support for this version ***\n");
#endif
	ret=RasmAssemble(AUTOTEST_MULTILZORG,strlen(AUTOTEST_MULTILZORG),&opcode,&opcodelen);
	if (!ret && opcodelen==38 && opcode[0]==6 && opcode[2]==0x10
			&& opcode[4]==0x1A && opcode[0x23]==0xFF && opcode[0x24]==0xFB) {} else {printf("Autotest %03d ERROR (LZ segments mixed with ORG)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ segment + ORG relocation OK\n");


	ret=RasmAssemble(AUTOTEST_LZDEFERED,strlen(AUTOTEST_LZDEFERED),&opcode,&opcodelen);
	if (!ret && opcodelen==7 && opcode[6]==6) {} else {printf("Autotest %03d ERROR (LZ segment + defered $)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ segment + defered $ OK\n");
	
	ret=RasmAssemble(AUTOTEST_LZ4,strlen(AUTOTEST_LZ4),&opcode,&opcodelen);
	if (!ret && opcodelen==49 && opcode[0]==0x15 && opcode[4]==0x44 && opcode[0xB]==0xF0) {} else {printf("Autotest %03d ERROR (LZ4 segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ4 segment OK\n");
	
	ret=RasmAssemble(AUTOTEST_DEFS,strlen(AUTOTEST_DEFS),&opcode,&opcodelen);
	if (!ret && opcodelen==256 && opcode[0]==0) {} else {printf("Autotest %03d ERROR (defs)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing DEFS OK\n");

	ret=RasmAssemble(AUTOTEST_BANKSET,strlen(AUTOTEST_BANKSET),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (bank/bankset)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing BANK/BANKSET OK\n");

	ret=RasmAssemble(AUTOTEST_PAGETAG,strlen(AUTOTEST_PAGETAG),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (page/pageset)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing prefix PAGE/PAGESET 1 OK\n");

	ret=RasmAssemble(AUTOTEST_PAGETAG2,strlen(AUTOTEST_PAGETAG2),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (page/pageset 2)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing prefix PAGE/PAGESET 2 OK\n");

	ret=RasmAssemble(AUTOTEST_PAGETAG3,strlen(AUTOTEST_PAGETAG3),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (page/pageset 3)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing prefix PAGE/PAGESET 3 OK\n");

	ret=RasmAssemble(AUTOTEST_UNDEF,strlen(AUTOTEST_UNDEF),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (simple undef)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing UNDEF OK\n");

	ret=RasmAssemble(AUTOTEST_TAGPRINT,strlen(AUTOTEST_TAGPRINT),&opcode,&opcodelen);
	if (!ret && opcodelen==1 && opcode[0]==0xC9) {} else {printf("Autotest %03d ERROR (tag inside printed string)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing tags inside printed string OK\n");
	
	ret=RasmAssemble(AUTOTEST_TAGFOLLOW,strlen(AUTOTEST_TAGFOLLOW),&opcode,&opcodelen);
	if (!ret && opcodelen==1 && opcode[0]==0xC9) {} else {printf("Autotest %03d ERROR (tag+alias fast translating)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing tag+alias in fast translate OK\n");
	
	ret=RasmAssemble(AUTOTEST_TAGREALLOC,strlen(AUTOTEST_TAGREALLOC),&opcode,&opcodelen);
	if (!ret && opcodelen==276) {} else {printf("Autotest %03d ERROR (tag realloc with fast translate)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing tag realloc with fast translate OK\n");
	
	ret=RasmAssemble(AUTOTEST_TAGLOOP,strlen(AUTOTEST_TAGLOOP),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (generated alias inside loop with generated var names )\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing generated alias inside loop with generated var names OK\n");
	
	ret=RasmAssemble(AUTOTEST_PRINTVAR,strlen(AUTOTEST_PRINTVAR),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (param inside printed string)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing param inside printed string OK\n");
	
	ret=RasmAssemble(AUTOTEST_PRINTSPACE,strlen(AUTOTEST_PRINTSPACE),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (space inside tag string for PRINT directive)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing space inside tag string for PRINT directive OK\n");

	ret=RasmAssemble(AUTOTEST_INHIBITION,strlen(AUTOTEST_INHIBITION),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (conditionnal inhibition)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing conditionnal inhibition OK\n");

	ret=RasmAssemble(AUTOTEST_SWITCH,strlen(AUTOTEST_SWITCH),&opcode,&opcodelen);
	if (!ret && opcodelen==3 && opcode[0]==4 && opcode[1]==3 && opcode[2]==4) {} else {printf("Autotest %03d ERROR (switch case)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing SWITCH/CASE OK\n");
	
	ret=RasmAssemble(AUTOTEST_NOCODE,strlen(AUTOTEST_NOCODE),&opcode,&opcodelen);
	if (!ret && opcodelen==57) {} else {printf("Autotest %03d ERROR (code/nocode)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing code/nocode OK\n");
	
	ret=RasmAssemble(AUTOTEST_VAREQU,strlen(AUTOTEST_VAREQU),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (var & equ)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing var & equ OK\n");
	
	ret=RasmAssemble(AUTOTEST_CHARSET,strlen(AUTOTEST_CHARSET),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (simple charset)\n",cpt);exit(-1);}
	if (opcodelen!=30 || memcmp(opcode,opcode+5,5) || memcmp(opcode+10,opcode+5,5)) {printf("Autotest %03d ERROR (simple charset)\n",cpt);exit(-1);}
	if (memcmp(opcode+15,opcode+20,5) || memcmp(opcode+15,opcode+25,5)) {printf("Autotest %03d ERROR (simple charset)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing simple charset OK\n");

	ret=RasmAssemble(AUTOTEST_CHARSET2,strlen(AUTOTEST_CHARSET2),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (extended charset)\n",cpt);exit(-1);}
	for (i=chk=0;i<opcodelen;i++) chk+=opcode[i];
	if (opcodelen!=16 || chk!=0x312) {printf("Autotest %03d ERROR (extended charset)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing extended charset OK\n");
	
	ret=RasmAssemble(AUTOTEST_QUOTES,strlen(AUTOTEST_QUOTES),&opcode,&opcodelen);
	if (!ret && opcodelen==10 && opcode[5]==0xE4 && opcode[6]==0x0D && opcode[7]==0x64 && opcode[8]==0x0D && opcode[9]==0xE4) {}
		else {printf("Autotest %03d ERROR (quotes & escaped chars)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing quoted & escaped chars OK\n");
	
	ret=RasmAssemble(AUTOTEST_NOT,strlen(AUTOTEST_NOT),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (not operator)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing NOT operator OK\n");
	
	ret=RasmAssemble(AUTOTEST_MACRO,strlen(AUTOTEST_MACRO),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (macro usage)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing macro usage OK\n");
	
	ret=RasmAssemble(AUTOTEST_MACRO_CONF01,strlen(AUTOTEST_MACRO_CONF01),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (macro name conflict with another macro)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing macro usage 2 OK\n");
	
	ret=RasmAssemble(AUTOTEST_MACRO_CONF02,strlen(AUTOTEST_MACRO_CONF02),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (macro name conflict with a variable)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing macro usage 3 OK\n");
	
	ret=RasmAssemble(AUTOTEST_MACRO_CONF03,strlen(AUTOTEST_MACRO_CONF03),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (macro name conflict with an alias)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing macro usage 4 OK\n");
	
	ret=RasmAssemble(AUTOTEST_MACRO_CONF04,strlen(AUTOTEST_MACRO_CONF04),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (macro name conflict with a label)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing macro usage 5 OK\n");
	
	ret=RasmAssemble(AUTOTEST_ASSERT,strlen(AUTOTEST_ASSERT),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (assert usage)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing assert usage (+prepro regression cases) OK\n");
	
	ret=RasmAssemble(AUTOTEST_MACROPAR,strlen(AUTOTEST_MACROPAR),&opcode,&opcodelen);
	if (!ret && opcodelen==12 && memcmp(opcode,"GROUIKgrouik",12)==0) {} else {printf("Autotest %03d ERROR (macro string param)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing macro string parameter OK\n");
	
	ret=RasmAssemble(AUTOTEST_MACRO_ADV,strlen(AUTOTEST_MACRO_ADV),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (macro param)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing macro parameters OK\n");
	
	ret=RasmAssemble(AUTOTEST_OVERLOADMACPRM,strlen(AUTOTEST_OVERLOADMACPRM),&opcode,&opcodelen);
	if (!ret && opcodelen==4 && opcode[0]==1 && opcode[1]==0 && opcode[2]==2 && opcode[3]==1) {} else {printf("Autotest %03d ERROR (macro param overload)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing macro parameter overload OK\n");
	
	ret=RasmAssemble(AUTOTEST_IFDEFMACRO,strlen(AUTOTEST_IFDEFMACRO),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (ifdef macro)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing IFDEF & MACRO OK\n");
	
	ret=RasmAssemble(AUTOTEST_LABNUM,strlen(AUTOTEST_LABNUM),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (variables in labels)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing variables in labels OK\n");

	ret=RasmAssemble(AUTOTEST_EQUNUM,strlen(AUTOTEST_EQUNUM),&opcode,&opcodelen);
	if (!ret && opcodelen==3) {} else {printf("Autotest %03d ERROR (variables in aliases) r=%d l=%d\n",cpt,ret,opcodelen);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing variables in aliases OK\n");

	ret=RasmAssemble(AUTOTEST_EQUDOLLAR,strlen(AUTOTEST_EQUDOLLAR),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (delayed EQU + $) r=%d l=%d\n",cpt,ret,opcodelen);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing delayed EQU + $ OK\n");

	ret=RasmAssemble(AUTOTEST_DELAYNUM,strlen(AUTOTEST_DELAYNUM),&opcode,&opcodelen);
	if (!ret && opcodelen==9 && opcode[0]==6 && opcode[2]==7 && opcode[4]==8) {} else {printf("Autotest %03d ERROR (delayed expr labels) r=%d l=%d\n",cpt,ret,opcodelen);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing delayed expression labels OK\n");

	ret=RasmAssemble(AUTOTEST_MODULE01,strlen(AUTOTEST_MODULE01),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (modules)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing modules + proximity OK\n");

	ret=RasmAssemble(AUTOTEST_MODULE02,strlen(AUTOTEST_MODULE02),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (modules 2)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing modules + proximity (bis) OK\n");

	ret=RasmAssemble(AUTOTEST_PROXIM,strlen(AUTOTEST_PROXIM),&opcode,&opcodelen);
	if (!ret && opcode[1]==3) {} else {printf("Autotest %03d ERROR (proximity labels)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing proximity labels OK\n");

	ret=RasmAssembleInfo(AUTOTEST_BANKPROX,strlen(AUTOTEST_BANKPROX),&opcode,&opcodelen,&debug);
	if (!ret) {} else {printf("Autotest %03d ERROR (BANK tag + proximity labels)\n",cpt);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing BANK tag + proximity labels OK\n");

	ret=RasmAssemble(AUTOTEST_STRUCT,strlen(AUTOTEST_STRUCT),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (structs)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing structs OK\n");
	
	ret=RasmAssemble(AUTOTEST_STRUCT2,strlen(AUTOTEST_STRUCT2),&opcode,&opcodelen);
	if (!ret && opcodelen==1) {} else {printf("Autotest %03d ERROR (sizeof struct fields)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing SIZEOF struct fields OK\n");

	ret=RasmAssemble(AUTOTEST_REPEAT,strlen(AUTOTEST_REPEAT),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (extended repeat)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing REPEAT cases OK\n");

	ret=RasmAssemble(AUTOTEST_REPEATKO,strlen(AUTOTEST_REPEATKO),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (repeat without end must return error)\n",cpt);exit(-1);}
printf("testing REPEAT without REND OK\n");

	ret=RasmAssemble(AUTOTEST_WHILEKO,strlen(AUTOTEST_WHILEKO),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (while without end must return error)\n",cpt);exit(-1);}
printf("testing WHILE without WEND OK\n");

	ret=RasmAssemble(AUTOTEST_TICKER,strlen(AUTOTEST_TICKER),&opcode,&opcodelen);
	if (!ret && opcodelen==18) {} else {printf("Autotest %03d ERROR (ticker (re)count)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing ticker OK\n");

	ret=RasmAssemble(AUTOTEST_DEFUSED,strlen(AUTOTEST_DEFUSED),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (ifdef ifused)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing IFDEF / IFUSED OK\n");

	ret=RasmAssemble(AUTOTEST_SAVEINVALID1,strlen(AUTOTEST_SAVEINVALID1),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (invalid size for SAVE)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing invalid size for SAVE OK\n");

	ret=RasmAssemble(AUTOTEST_SAVEINVALID2,strlen(AUTOTEST_SAVEINVALID2),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (invalid offset for SAVE)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing invalid offset (too low) for SAVE OK\n");

	ret=RasmAssemble(AUTOTEST_SAVEINVALID3,strlen(AUTOTEST_SAVEINVALID3),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (invalid offset for SAVE)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing invalid offset (too high) for SAVE OK\n");

	ret=RasmAssemble(AUTOTEST_INHIBITION2,strlen(AUTOTEST_INHIBITION2),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (if switch inhibition)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing if/switch inhibition OK\n");

	ret=RasmAssemble(AUTOTEST_INHIBITIONMAX,strlen(AUTOTEST_INHIBITIONMAX),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (moar inhibition)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing more inhibition cases OK\n");

	ret=RasmAssemble(AUTOTEST_MACROPROX,strlen(AUTOTEST_MACROPROX),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (macro + prox)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing regression case with proximity label following macro definition OK\n");

	ret=RasmAssemble(AUTOTEST_PROXBACK,strlen(AUTOTEST_PROXBACK),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (boucle/macro + prox = recup global)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing global label back as reference when leaving macro and loops OK\n");

	ret=RasmAssemble(AUTOTEST_LOCAPROX,strlen(AUTOTEST_LOCAPROX),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (far local+prox access)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing far access of a proximity label refering to local OK\n");

	ret=RasmAssemble(AUTOTEST_PUSHPOPGLOBAL,strlen(AUTOTEST_PUSHPOPGLOBAL),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (get back global for proximity inside macro+loops)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing push/pop global label for proximity inside macro+loops OK\n");

	ret=RasmAssemble(AUTOTEST_NEGATIVE,strlen(AUTOTEST_NEGATIVE),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (formula case 0)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing formula case 0 OK\n");
	
	ret=RasmAssemble(AUTOTEST_OPERATORASSIGN,strlen(AUTOTEST_OPERATORASSIGN),&opcode,&opcodelen);
	if (!ret && opcodelen==1) {} else {printf("Autotest %03d ERROR (operator assignment)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing operator assignment OK\n");
	
	ret=RasmAssemble(AUTOTEST_OPERATORASSIGN2,strlen(AUTOTEST_OPERATORASSIGN2),&opcode,&opcodelen);
	if (!ret && opcodelen==1) {} else {printf("Autotest %03d ERROR (operator assignment + repeat)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing operator assignment + repeat OK\n");
	
	ret=RasmAssemble(AUTOTEST_OPERATORASSIGN3,strlen(AUTOTEST_OPERATORASSIGN3),&opcode,&opcodelen);
	if (!ret && opcodelen==1) {} else {printf("Autotest %03d ERROR (operator assignment + repeat)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing operator assignment + repeat + spacing OK\n");
	
	ret=RasmAssemble(AUTOTEST_FORMULA1,strlen(AUTOTEST_FORMULA1),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (formula case 1)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing formula case 1 OK\n");
	
	ret=RasmAssemble(AUTOTEST_SETINSIDE,strlen(AUTOTEST_SETINSIDE),&opcode,&opcodelen);
	if (ret) {} else {printf("Autotest %03d ERROR (set var inside expression must trigger error)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing var set inside an expression must trigger an error OK\n");
	
	ret=RasmAssemble(AUTOTEST_CODESKIP,strlen(AUTOTEST_CODESKIP),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (code skip)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing code skip OK\n");
	
	ret=RasmAssemble(AUTOTEST_FORMULA2,strlen(AUTOTEST_FORMULA2),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (formula case 2 function+multiple parenthesis)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing formula functions + multiple parenthesis OK\n");

	ret=RasmAssemble(AUTOTEST_GETNOP_LD,strlen(AUTOTEST_GETNOP_LD),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (math function GETNOP with multiple LD syncronised with TICKER)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing synchronisation between TICKER and GETNOP on multiple LD OK\n");

	ret=RasmAssemble(AUTOTEST_TICKERNOP_FULL,strlen(AUTOTEST_TICKERNOP_FULL),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (math function GETNOP with almost full instruction set)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing synchronisation between TICKER and GETNOP on almost full instruction set OK\n");

	ret=RasmAssemble(AUTOTEST_TICKER_FULL,strlen(AUTOTEST_TICKER_FULL),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (math function GETTICK with almost full instruction set)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing synchronisation between TICKER and GETTICK on almost full instruction set OK\n");

	ret=RasmAssemble(AUTOTEST_SHIFTMAX,strlen(AUTOTEST_SHIFTMAX),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (shifting more than 31 must give zero result)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing shifting more than 31 OK\n");

	ret=RasmAssemble(AUTOTEST_PLUSCOLOR,strlen(AUTOTEST_PLUSCOLOR),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (formula func for Plus color management)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing formula function for Plus color management OK\n");
	
	ret=RasmAssemble(AUTOTEST_FRAC,strlen(AUTOTEST_FRAC),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (formula func FRAC)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing formula FRAC function OK\n");
	
	ret=RasmAssemble(AUTOTEST_RND,strlen(AUTOTEST_RND),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (formula func RND)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing formula RND function OK\n");
	
	ret=RasmAssemble(AUTOTEST_UNDERVAR,strlen(AUTOTEST_UNDERVAR),&opcode,&opcodelen);
	if (!ret && opcodelen==4) {} else {printf("Autotest %03d ERROR (var starting with _)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing var names starting with '_' OK\n");
	
	ret=RasmAssemble(AUTOTEST_NOEXPORT,strlen(AUTOTEST_NOEXPORT),&opcode,&opcodelen);
	if (!ret && opcodelen==2) {} else {printf("Autotest %03d ERROR (noexport/enoexport)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing export/noexport OK\n");
	
	ret=RasmAssemble(AUTOTEST_ENHANCED_LD,strlen(AUTOTEST_ENHANCED_LD),&opcode,&opcodelen);
	if (!ret && memcmp(opcode,opcode+opcodelen/2,opcodelen/2)==0) {} else {printf("Autotest %03d ERROR (enhanced LD)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing enhanced LD OK\n");
	
	ret=RasmAssemble(AUTOTEST_ENHANCED_LD2,strlen(AUTOTEST_ENHANCED_LD2),&opcode,&opcodelen);
	if (!ret && memcmp(opcode,opcode+opcodelen/2,opcodelen/2)==0) {} else {printf("Autotest %03d ERROR (enhanced LD 2)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing enhanced LD variante OK\n");
	
	ret=RasmAssemble(AUTOTEST_ENHANCED_LD3,strlen(AUTOTEST_ENHANCED_LD3),&opcode,&opcodelen);
	if (!ret && memcmp(opcode,opcode+opcodelen/2,opcodelen/2)==0) {} else {printf("Autotest %03d ERROR (enhanced LD 3)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing enhanced LD variante OK\n");
	
	ret=RasmAssemble(AUTOTEST_ENHANCED_PUSHPOP,strlen(AUTOTEST_ENHANCED_PUSHPOP),&opcode,&opcodelen);
	if (!ret && memcmp(opcode,opcode+opcodelen/2,opcodelen/2)==0) {} else {printf("Autotest %03d ERROR (enhanced PUSH/POP/NOP)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing enhanced PUSH/POP OK\n");

	ret=RasmAssembleInfo(AUTOTEST_INKCONV,strlen(AUTOTEST_INKCONV),&opcode,&opcodelen,&debug);
	if (!ret && opcodelen==2) {} else {printf("Autotest %03d ERROR (gate array color conversion)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing gate array color conversion OK\n");

	ret=RasmAssembleInfo(AUTOTEST_SNASET,strlen(AUTOTEST_SNASET),&opcode,&opcodelen,&debug);
	if (!ret) {} else {printf("Autotest %03d ERROR (snapshot settings)\n",cpt);MiniDump(opcode,opcodelen);for (i=0;i<debug->nberror;i++) printf("%d -> %s\n",i,debug->error[i].msg);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
printf("testing snapshot settings OK\n");

	ret=RasmAssemble(AUTOTEST_PAGELABELGEN,strlen(AUTOTEST_PAGELABELGEN),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (pagelabelgen)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing page tag with generated label name OK\n");

	ret=RasmAssembleInfo(AUTOTEST_EMBEDDED_ERRORS,strlen(AUTOTEST_EMBEDDED_ERRORS),&opcode,&opcodelen,&debug);
	if (ret && debug->nberror==2 && debug->nbsymbol==3) {
/*		
		printf("\n");
		for (i=0;i<debug->nberror;i++) {
			printf("%d -> %s\n",i,debug->error[i].msg);
		}
		for (i=0;i<debug->nbsymbol;i++) {
			printf("%d -> %s=%d\n",i,debug->symbol[i].name,debug->symbol[i].v);
		}
*/
		RasmFreeInfoStruct(debug);
	} else {printf("Autotest %03d ERROR (embedded error struct) err=%d nberr=%d (2) nbsymb=%d (3)\n",cpt,ret,debug->nberror,debug->nbsymbol);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing internal error struct OK\n");

	ret=RasmAssembleInfo(AUTOTEST_EMBEDDED_LABELS,strlen(AUTOTEST_EMBEDDED_LABELS),&opcode,&opcodelen,&debug);
	if (!ret && debug->nbsymbol==3) {
		/*
		printf("\nnbsymbol=%d\n",debug->nbsymbol);
		for (i=0;i<debug->nbsymbol;i++) {
			printf("%d -> %s=%d\n",i,debug->symbol[i].name,debug->symbol[i].v);
		}*/
		RasmFreeInfoStruct(debug);
	} else {printf("Autotest %03d ERROR (embedded test)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing internal label struct OK\n");

#ifdef RDD
	printf("\n%d bytes\n",_static_library_memory_used);
	{
		int filelen;
	tmpstr3=FileReadContent("./test/PlayerAky.asm",&filelen);
	printf(".");fflush(stdout);ret=RasmAssembleInfo(tmpstr3,filelen,&opcode,&opcodelen,&debug);
	if (!ret) {} else {printf("Autotest %03d ERROR (PlayerAky)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	RasmFreeInfoStruct(debug);
	MemFree(tmpstr3);
	}

	printf("\n%d bytes\n",_static_library_memory_used);
#endif

	/************************** include testing + crunch ****************/		
	idx=0;
	opcode=MemMalloc(8000);
	for (k=40;k>1;k--) {
		for (i='A';i<'F';i++) {
			for (j=0;j<k;j++) opcode[idx++]=i;
		}
	}
	for (k=1;k<40;k++) {
		for (i='A';i<'F';i++) {
			for (j=0;j<k;j++) opcode[idx++]=i;
		}
	}
	FileRemoveIfExists("autotest_include.raw");
	FileWriteBinary("autotest_include.raw",(char *)opcode,idx);
	FileWriteBinaryClose("autotest_include.raw");
	MemFree(opcode);opcode=NULL;

	ret=RasmAssemble(AUTOTEST_INCBIN1,strlen(AUTOTEST_INCBIN1),&opcode,&opcodelen);
	if (!ret && opcodelen==7995) {} else {printf("Autotest %03d ERROR (INCBIN)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN OK\n");

	ret=RasmAssemble(AUTOTEST_INCBIN2,strlen(AUTOTEST_INCBIN2),&opcode,&opcodelen);
	if (!ret && opcodelen==6995) {} else {printf("Autotest %03d ERROR (INCBIN+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN+offset OK\n");

	ret=RasmAssemble(AUTOTEST_INCBIN3,strlen(AUTOTEST_INCBIN3),&opcode,&opcodelen);
	if (!ret && opcodelen==1000) {} else {printf("Autotest %03d ERROR (INCBIN+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN+size OK\n");

	ret=RasmAssemble(AUTOTEST_LZ4_A,strlen(AUTOTEST_LZ4_A),&opcode,&opcodelen);
	if (!ret && opcodelen==544) {} else {printf("Autotest %03d ERROR (INCBIN + LZ4 segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZ4 segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZ4_B,strlen(AUTOTEST_LZ4_B),&opcode,&opcodelen);
	if (!ret && opcodelen==544) {} else {printf("Autotest %03d ERROR (INCLZ4)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCLZ4 OK\n");

	ret=RasmAssemble(AUTOTEST_LZ4_C,strlen(AUTOTEST_LZ4_C),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCLZ4+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZ4_D,strlen(AUTOTEST_LZ4_D),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZ4+INCBIN+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ4 variant+offset OK\n");

	ret=RasmAssemble(AUTOTEST_LZ4_E,strlen(AUTOTEST_LZ4_E),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCLZ4+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZ4_F,strlen(AUTOTEST_LZ4_F),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZ4+INCBIN+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ4 variant+size OK\n");

	ret=RasmAssemble(AUTOTEST_LZ48_A,strlen(AUTOTEST_LZ48_A),&opcode,&opcodelen);
	if (!ret && opcodelen==738) {} else {printf("Autotest %03d ERROR (INCBIN + LZ48 segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZ48 segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZ48_B,strlen(AUTOTEST_LZ48_B),&opcode,&opcodelen);
	if (!ret && opcodelen==738) {} else {printf("Autotest %03d ERROR (INCL48)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCL48 OK\n");

	ret=RasmAssemble(AUTOTEST_LZ48_C,strlen(AUTOTEST_LZ48_C),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCL48+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZ48_D,strlen(AUTOTEST_LZ48_D),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZ48+INCBIN+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ48 variant+offset OK\n");

	ret=RasmAssemble(AUTOTEST_LZ48_E,strlen(AUTOTEST_LZ48_E),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCLZ48+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZ48_F,strlen(AUTOTEST_LZ48_F),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZ48+INCBIN+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ48 variant+size OK\n");

	ret=RasmAssemble(AUTOTEST_LZ49_A,strlen(AUTOTEST_LZ49_A),&opcode,&opcodelen);
	if (!ret && opcodelen==717) {} else {printf("Autotest %03d ERROR (INCBIN + LZ49 segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZ49 segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZ49_B,strlen(AUTOTEST_LZ49_B),&opcode,&opcodelen);
	if (!ret && opcodelen==717) {} else {printf("Autotest %03d ERROR (INCL49)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCL49 OK\n");

	ret=RasmAssemble(AUTOTEST_LZ49_C,strlen(AUTOTEST_LZ49_C),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCL49+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZ49_D,strlen(AUTOTEST_LZ49_D),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZ49+INCBIN+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ49 variant+offset OK\n");

	ret=RasmAssemble(AUTOTEST_LZ49_E,strlen(AUTOTEST_LZ49_E),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCL49+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZ49_F,strlen(AUTOTEST_LZ49_F),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZ49+INCBIN+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZ49 variant+size OK\n");


	ret=RasmAssemble(AUTOTEST_LZEXO_A,strlen(AUTOTEST_LZEXO_A),&opcode,&opcodelen);
	if (!ret && opcodelen==335) {} else {printf("Autotest %03d ERROR (INCBIN + LZEXO segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZEXO segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZEXO_B,strlen(AUTOTEST_LZEXO_B),&opcode,&opcodelen);
	if (!ret && opcodelen==335) {} else {printf("Autotest %03d ERROR (INCEXO)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCEXO OK\n");

	ret=RasmAssemble(AUTOTEST_LZEXO_C,strlen(AUTOTEST_LZEXO_C),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCEXO+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZEXO_D,strlen(AUTOTEST_LZEXO_D),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZEXO+INCBIN+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZEXO variant+offset OK\n");

	ret=RasmAssemble(AUTOTEST_LZEXO_E,strlen(AUTOTEST_LZEXO_E),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCEXO+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZEXO_F,strlen(AUTOTEST_LZEXO_F),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZEXO+INCBIN+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZEXO variant+size OK\n");


	ret=RasmAssemble(AUTOTEST_LZX7_A,strlen(AUTOTEST_LZX7_A),&opcode,&opcodelen);
	if (!ret && opcodelen==535) {} else {printf("Autotest %03d ERROR (INCBIN + LZX7 segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZX7 segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZX7_B,strlen(AUTOTEST_LZX7_B),&opcode,&opcodelen);
	if (!ret && opcodelen==535) {} else {printf("Autotest %03d ERROR (INCZX7)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCZX7 OK\n");

	ret=RasmAssemble(AUTOTEST_LZX7_C,strlen(AUTOTEST_LZX7_C),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCZX7+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZX7_D,strlen(AUTOTEST_LZX7_D),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZX7+INCBIN+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZX7 variant+offset OK\n");

	ret=RasmAssemble(AUTOTEST_LZX7_E,strlen(AUTOTEST_LZX7_E),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCZX7+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZX7_F,strlen(AUTOTEST_LZX7_F),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZX7+INCBIN+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZX7 variant+size OK\n");


#ifndef NOAPULTRA
	ret=RasmAssemble(AUTOTEST_LZAPU_A,strlen(AUTOTEST_LZAPU_A),&opcode,&opcodelen);
	if (!ret && opcodelen==396) {} else {printf("Autotest %03d ERROR (INCBIN + LZAPU segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZAPU segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZAPU_B,strlen(AUTOTEST_LZAPU_B),&opcode,&opcodelen);
	if (!ret && opcodelen==396) {} else {printf("Autotest %03d ERROR (INCAPU)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCAPU OK\n");

	ret=RasmAssemble(AUTOTEST_LZAPU_C,strlen(AUTOTEST_LZAPU_C),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCAPU+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZAPU_D,strlen(AUTOTEST_LZAPU_D),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZAPU+INCBIN+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZAPU variant+offset OK\n");

	ret=RasmAssemble(AUTOTEST_LZAPU_E,strlen(AUTOTEST_LZAPU_E),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCAPU+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZAPU_F,strlen(AUTOTEST_LZAPU_F),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZAPU+INCBIN+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZAPU variant+size OK\n");



	ret=RasmAssemble(AUTOTEST_LZSA1_A,strlen(AUTOTEST_LZSA1_A),&opcode,&opcodelen);
	if (!ret && opcodelen<=442) {} else {printf("Autotest %03d ERROR (INCBIN + LZSA1 segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZSA1 segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZSA1_Abis,strlen(AUTOTEST_LZSA1_Abis),&opcode,&opcodelen);
	if (!ret && opcodelen<=437) {} else {printf("Autotest %03d ERROR (INCBIN + LZSA1 segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZSA1 segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZSA1_B,strlen(AUTOTEST_LZSA1_B),&opcode,&opcodelen);
	if (!ret && opcodelen<=442) {} else {printf("Autotest %03d ERROR (INCLZSA1)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCLZSA1 OK\n");

	ret=RasmAssemble(AUTOTEST_LZSA1_C,strlen(AUTOTEST_LZSA1_C),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCLZSA1+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZSA1_D,strlen(AUTOTEST_LZSA1_D),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZSA1+INCBIN+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZSA1 variant+offset OK\n");

	ret=RasmAssemble(AUTOTEST_LZSA1_E,strlen(AUTOTEST_LZSA1_E),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCLZSA1+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZSA1_F,strlen(AUTOTEST_LZSA1_F),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZSA1+INCBIN+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZSA1 variant+size OK\n");




	ret=RasmAssemble(AUTOTEST_LZSA2_A,strlen(AUTOTEST_LZSA2_A),&opcode,&opcodelen);
	if (!ret && opcodelen<=463) {} else {printf("Autotest %03d ERROR (INCBIN + LZSA2 segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZSA2 segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZSA2_Abis,strlen(AUTOTEST_LZSA2_Abis),&opcode,&opcodelen);
	if (!ret && opcodelen<=462) {} else {printf("Autotest %03d ERROR (INCBIN + LZSA2 segment)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCBIN + LZSA2 segment OK\n");

	ret=RasmAssemble(AUTOTEST_LZSA2_B,strlen(AUTOTEST_LZSA2_B),&opcode,&opcodelen);
	if (!ret && opcodelen<=463) {} else {printf("Autotest %03d ERROR (INCLZSA2)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing INCLZSA2 OK\n");

	ret=RasmAssemble(AUTOTEST_LZSA2_C,strlen(AUTOTEST_LZSA2_C),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCLZSA2+offset)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZSA2_D,strlen(AUTOTEST_LZSA2_D),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZSA2+INCBIN+offset) %d!=%d\n",cpt,i,opcodelen);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZSA2 variant+offset OK\n");

	ret=RasmAssemble(AUTOTEST_LZSA2_E,strlen(AUTOTEST_LZSA2_E),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (INCLZSA2+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
	i=opcodelen;
	ret=RasmAssemble(AUTOTEST_LZSA2_F,strlen(AUTOTEST_LZSA2_F),&opcode,&opcodelen);
	if (!ret && i==opcodelen) {} else {printf("Autotest %03d ERROR (LZSA2+INCBIN+size)\n",cpt);MiniDump(opcode,opcodelen);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing LZSA2 variant+size OK\n");

#else
printf("*** LZAPU and INCAPU   tests disabled as there is no APUltra support for this version ***\n");
printf("*** LZSA1 and INCLZSA1 tests disabled as there is no LZSA v1 support for this version ***\n");
printf("*** LZSA2 and INCLZSA2 tests disabled as there is no LZSA v2 support for this version ***\n");
#endif


	ret=RasmAssemble(AUTOTEST_ECPR1,strlen(AUTOTEST_ECPR1),&opcode,&opcodelen);
	if (!ret) {} else {printf("Autotest %03d ERROR (extended CPR test 1)\n",cpt);exit(-1);}
	if (opcode) MemFree(opcode);opcode=NULL;cpt++;
printf("testing simple extended CPR behaviour OK\n");



	FileRemoveIfExists("autotest_include.raw");
	FileRemoveIfExists("rasmoutput.xpr");
	FileRemoveIfExists("rasmoutput.cpr");
	FileRemoveIfExists("rasmoutput.sna");
	
	ret=RasmAssemble(NULL,0,&opcode,&opcodelen)+RasmAssembleInfo(NULL,0,&opcode,&opcodelen,&debug)+RasmAssembleInfoParam(NULL,0,&opcode,&opcodelen,&debug,&param);
	printf("All internal tests OK => %d tests done\n",ret);



	#ifdef RDD
	/* private dev lib tools */
printf("checking memory\n");
	CloseLibrary();
	#endif
	exit(0);
}


/******************************************************
LZ48 v005 / LZ49 v002
******************************************************/
int LZ48_encode_extended_length(unsigned char *odata, int length)
{
	int ioutput=0;

	while (length>=255) {
		odata[ioutput++]=0xFF;
		length-=255;
	}
	/* if the last value is 255 we must encode 0 to end extended length */
	/*if (length==0) rasm_printf(ae,"bugfixed!\n");*/
	odata[ioutput++]=(unsigned char)length;
	return ioutput;
}

int LZ48_encode_block(unsigned char *odata,unsigned char *data, int literaloffset,int literalcpt,int offset,int maxlength)
{
	int ioutput=1;
	int token=0;
	int i;

	if (offset<0 || offset>255) {
		fprintf(stderr,"internal offset error!\n");
		exit(-2);
	}
	
	if (literalcpt<15) {
		token=literalcpt<<4; 
	} else {
		token=0xF0;
		ioutput+=LZ48_encode_extended_length(odata+ioutput,literalcpt-15);
	}

	for (i=0;i<literalcpt;i++) odata[ioutput++]=data[literaloffset++];

	if (maxlength<18) {
		if (maxlength>2) {
			token|=(maxlength-3);
		} else {
			/* endoffset has no length */
		}
	} else {
		token|=0xF;
		ioutput+=LZ48_encode_extended_length(odata+ioutput,maxlength-18);
	}

	odata[ioutput++]=(unsigned char)offset-1;
	
	odata[0]=(unsigned char)token;
	return ioutput;
}

unsigned char *LZ48_encode_legacy(unsigned char *data, int length, int *retlength)
{
	int i,startscan,current=1,token,ioutput=1,curscan;
	int maxoffset=0,maxlength,matchlength,literal=0,literaloffset=1;
	unsigned char *odata=NULL;
	
	odata=MemMalloc((size_t)length*1.5+10);
	if (!odata) {
		fprintf(stderr,"malloc(%.0lf) - memory full\n",(size_t)length*1.5+10);
		exit(-1);
	}

	/* first byte always literal */
	odata[0]=data[0];

	/* force short data encoding */
	if (length<5) {
		token=(length-1)<<4;
		odata[ioutput++]=(unsigned char)token;
		for (i=1;i<length;i++) odata[ioutput++]=data[current++];
		odata[ioutput++]=0xFF;
		*retlength=ioutput;
		return odata;
	}

	while (current<length) {
		maxlength=0;
		startscan=current-255;
		if (startscan<0) startscan=0;
		while (startscan<current) {
			matchlength=0;
			curscan=current;
			for (i=startscan;curscan<length;i++) {
				if (data[i]==data[curscan++]) matchlength++; else break;
			}
			if (matchlength>=3 && matchlength>maxlength) {
				maxoffset=startscan;
				maxlength=matchlength;
			}
			startscan++;
		}
		if (maxlength) {
			ioutput+=LZ48_encode_block(odata+ioutput,data,literaloffset,literal,current-maxoffset,maxlength);
			current+=maxlength;
			literaloffset=current;
			literal=0;
		} else {
			literal++;
			current++;
		}
	}
	ioutput+=LZ48_encode_block(odata+ioutput,data,literaloffset,literal,0,0);
	*retlength=ioutput;
	return odata;
}

int LZ49_encode_extended_length(unsigned char *odata, int length)
{
	int ioutput=0;

	while (length>=255) {
		odata[ioutput++]=0xFF;
		length-=255;
	}
	/* if the last value is 255 we must encode 0 to end extended length */
	/*if (length==0) rasm_printf(ae,"bugfixed!\n");*/
	odata[ioutput++]=(unsigned char)length;
	return ioutput;
}

int LZ49_encode_block(unsigned char *odata,unsigned char *data, int literaloffset,int literalcpt,int offset,int maxlength)
{
	int ioutput=1;
	int token=0;
	int i;

	if (offset<0 || offset>511) {
		fprintf(stderr,"internal offset error!\n");
		exit(-2);
	}
	
	if (literalcpt<7) {
		token=literalcpt<<4; 
	} else {
		token=0x70;
		ioutput+=LZ49_encode_extended_length(odata+ioutput,literalcpt-7);
	}

	for (i=0;i<literalcpt;i++) odata[ioutput++]=data[literaloffset++];

	if (maxlength<18) {
		if (maxlength>2) {
			token|=(maxlength-3);
		} else {
			/* endoffset has no length */
		}
	} else {
		token|=0xF;
		ioutput+=LZ49_encode_extended_length(odata+ioutput,maxlength-18);
	}

	if (offset>255) {
		token|=0x80;
		offset-=256;
	}	
	odata[ioutput++]=(unsigned char)offset-1;
	
	odata[0]=(unsigned char)token;
	return ioutput;
}

unsigned char *LZ49_encode_legacy(unsigned char *data, int length, int *retlength)
{
	int i,startscan,current=1,token,ioutput=1,curscan;
	int maxoffset=0,maxlength,matchlength,literal=0,literaloffset=1;
	unsigned char *odata=NULL;
	
	odata=MemMalloc((size_t)(length*1.5+10));
	if (!odata) {
		fprintf(stderr,"malloc(%.0lf) - memory full\n",(size_t)length*1.5+10);
		exit(-1);
	}

	/* first byte always literal */
	odata[0]=data[0];

	/* force short data encoding */
	if (length<5) {
		token=(length-1)<<4;
		odata[ioutput++]=(unsigned char)token;
		for (i=1;i<length;i++) odata[ioutput++]=data[current++];
		odata[ioutput++]=0xFF;
		*retlength=ioutput;
		return odata;
	}

	while (current<length) {
		maxlength=0;
		startscan=current-511;
		if (startscan<0) startscan=0;
		while (startscan<current) {
			matchlength=0;
			curscan=current;
			for (i=startscan;curscan<length;i++) {
				if (data[i]==data[curscan++]) matchlength++; else break;
			}
			if (matchlength>=3 && matchlength>maxlength) {
				maxoffset=startscan;
				maxlength=matchlength;
			}
			startscan++;
		}
		if (maxlength) {
			ioutput+=LZ49_encode_block(odata+ioutput,data,literaloffset,literal,current-maxoffset,maxlength);
			current+=maxlength;
			literaloffset=current;
			literal=0;
		} else {
			literal++;
			current++;
		}
	}
	ioutput+=LZ49_encode_block(odata+ioutput,data,literaloffset,literal,0,0);
	*retlength=ioutput;
	return odata;
}


/***************************************
	semi-generic body of program
***************************************/

#ifndef INTEGRATED_ASSEMBLY

/*
	Usage
	display the mandatory parameters
*/
void Usage(int help)
{
	#undef FUNC
	#define FUNC "Usage"
	
	printf("%s (c) 2017 Edouard BERGE (use -n option to display all licenses / -autotest for self-testing)\n",RASM_VERSION);
	#ifndef NO_3RD_PARTIES
	printf("LZ4 (c) Yann Collet / ZX7 (c) Einar Saukas / Exomizer 2 (c) Magnus Lind / LZSA & AP-Ultra (c) Emmanuel Marty\n");
	#endif
	printf("\n");
	printf("SYNTAX: rasm <inputfile> [options]\n");
	printf("\n");

	if (help) {
		printf("FILENAMES:\n");
		printf("-oa                      automatic radix from input filename\n");
		printf("-o  <outputfile radix>   choose a common radix for all files\n");
		printf("-or <ROM filename(s)>    choose a radix filename for ROM output\n");
		printf("-ob <binary filename>    choose a full filename for binary output\n");
		printf("-oc <cartridge filename> choose a full filename for cartridge output\n");
		printf("-oi <snapshot filename>  choose a full filename for snapshot output\n");
		printf("-os <symbol filename>    choose a full filename for symbol output\n");
		printf("-ot <tape filename>      choose a full filename for tape output\n");
		printf("-ok <breakpoint filename>choose a full filename for breakpoint output\n");
		printf("-I<path>                 set a path for files to read\n");
		printf("-no                      disable all file output\n");
		printf("DEPENDENCIES EXPORT:\n");
		printf("-depend=make             output dependencies on a single line\n");
		printf("-depend=list             output dependencies as a list\n");
		printf("if 'binary filename' is set then it will be outputed first\n");
		printf("SYMBOLS EXPORT:\n");
		printf("-s  export symbols %%s #%%X B%%d (label,adr,cprbank)\n");
		printf("-sz export symbols with ZX emulator convention\n");
		printf("-sp export symbols with Pasmo convention\n");
		printf("-sw export symbols with Winape convention\n");
		printf("-ss export symbols in the snapshot (SYMB chunk for ACE)\n");
		printf("-sc <format> export symbols with source code convention\n");
		printf("-sm export symbol in multiple files (one per bank)\n");
		printf("-l  <labelfile> import symbol file (winape,pasmo,rasm)\n");
		printf("-eb export breakpoints\n");
		printf("-wu warn for unused symbols (alias, var or label)\n");
		printf("SYMBOLS ADDITIONAL OPTIONS:\n");
		printf("-sl export also local symbol\n");
		printf("-sv export also variables symbol\n");
		printf("-sq export also EQU symbol\n");
		printf("-sa export all symbols (like -sl -sv -sq option)\n");
		printf("-Dvariable=value import value for variable\n");
		printf("COMPATIBILITY:\n");
		printf("-m     Maxam style calculations\n");
		printf("-dams  Dams 'dot' label convention\n");
		printf("-ass   AS80  behaviour mimic\n");
		printf("-uz    UZ80  behaviour mimic\n");
		printf("-pasmo PASMO behaviour mimic\n");
		printf("-amper use ampersand for hex values\n");
		printf("-msep <separator> set separator for modules\n");
		
		printf("EDSK generation/update:\n");
		printf("-eo overwrite files on disk if it already exists\n");
		printf("SNAPSHOT:\n");
		printf("-sb export breakpoints in snapshot (BRKS & BRKC chunks)\n");
		printf("-ss export symbols in the snapshot (SYMB chunk for ACE)\n");
		printf("-v2 export snapshot version 2 instead of version 3\n");
		printf("PARSING:\n");
		printf("-me <value>    set maximum number of error (0 means no limit)\n");
		printf("-twe           treat warnings as errors\n");
		printf("-xr            extended error display\n");
		printf("-w             disable warnings\n");
		printf("-void          force void usage with macro without parameter\n");
		printf("\n");
	} else {
		printf("use option -h for help\n");
		printf("\n");
	}
	
	exit(ABORT_ERROR);
}

void Licenses()
{
	#undef FUNC
	#define FUNC "Licenses"

printf("            ____ \n");
printf("           |  _ \\ __ _ ___ _ __ ___  \n");
printf("           | |_) / _` / __| '_ ` _ \\ \n");
printf("           |  _ < (_| \\__ \\ | | | | |\n");
printf("           |_| \\_\\__,_|___/_| |_| |_|\n");
printf("\n");	
printf("          is using MIT 'expat' license\n");
printf("\" Copyright (c) BERGE Edouard (roudoudou)\n\n");

printf("Permission  is  hereby  granted,  free  of charge,\n");
printf("to any person obtaining a copy  of  this  software\n");
printf("and  associated  documentation/source   files   of\n");
printf("RASM, to deal in the Software without restriction,\n");
printf("including without limitation the  rights  to  use,\n");
printf("copy,   modify,   merge,   publish,    distribute,\n");
printf("sublicense,  and/or  sell  copies of the Software,\n");
printf("and  to  permit  persons  to  whom the Software is\n");
printf("furnished  to  do  so,  subject  to  the following\n");
printf("conditions:\n");

printf("The above copyright  notice  and  this  permission\n");
printf("notice   shall   be  included  in  all  copies  or\n");
printf("substantial portions of the Software.\n");
printf("The   Software   is   provided  'as is',   without\n");
printf("warranty   of   any   kind,  express  or  implied,\n");
printf("including  but  not  limited  to the warranties of\n");
printf("merchantability,   fitness   for   a    particular\n");
printf("purpose  and  noninfringement.  In  no event shall\n");
printf("the  authors  or  copyright  holders be liable for\n");
printf("any  claim, damages  or other  liability,  whether\n");
printf("in  an  action  of  contract, tort  or  otherwise,\n");
printf("arising from,  out of  or in connection  with  the\n");
printf("software  or  the  use  or  other  dealings in the\n");
printf("Software. \"\n");

#ifndef NO_3RD_PARTIES
printf("\n\n\n\n");
printf("******* license for LZ4 cruncher / sources were modified ***********\n\n\n\n");

printf("BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)\n");

printf("Redistribution and use in source and binary forms, with or without\n");
printf("modification, are permitted provided that the following conditions are\n");
printf("met:\n\n");

printf("    * Redistributions of source code must retain the above copyright\n");
printf("notice, this list of conditions and the following disclaimer.\n");
printf("    * Redistributions in binary form must reproduce the above\n");
printf("copyright notice, this list of conditions and the following disclaimer\n");
printf("in the documentation and/or other materials provided with the\n");
printf("distribution.\n\n");

printf("THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n");
printf("'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n");
printf("LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n");
printf("A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n");
printf("OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n");
printf("SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n");
printf("LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n");
printf("DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n");
printf("THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n");
printf("(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n");
printf("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n");

printf("You can contact the author at :\n");
printf(" - LZ4 homepage : http://www.lz4.org\n");
printf(" - LZ4 source repository : https://github.com/lz4/lz4\n");


printf("\n\n\n\n");
printf("******* license for ZX7 cruncher / sources were modified ***********\n\n\n\n");

printf(" * (c) Copyright 2012 by Einar Saukas. All rights reserved.\n");
printf(" *\n");
printf(" * Redistribution and use in source and binary forms, with or without\n");
printf(" * modification, are permitted provided that the following conditions are met:\n");
printf(" *     * Redistributions of source code must retain the above copyright\n");
printf(" *       notice, this list of conditions and the following disclaimer.\n");
printf(" *     * Redistributions in binary form must reproduce the above copyright\n");
printf(" *       notice, this list of conditions and the following disclaimer in the\n");
printf(" *       documentation and/or other materials provided with the distribution.\n");
printf(" *     * The name of its author may not be used to endorse or promote products\n");
printf(" *       derived from this software without specific prior written permission.\n");
printf(" *\n");
printf(" * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND\n");
printf(" * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n");
printf(" * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n");
printf(" * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY\n");
printf(" * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n");
printf(" * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n");
printf(" * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\n");
printf(" * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n");
printf(" * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n");
printf(" * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n");


printf("\n\n\n\n");
printf("******* license for exomizer cruncher / sources were modified ***********\n\n\n\n");

printf(" * Copyright (c) 2005 Magnus Lind.\n");
printf(" *\n");
printf(" * This software is provided 'as-is', without any express or implied warranty.\n");
printf(" * In no event will the authors be held liable for any damages arising from\n");
printf(" * the use of this software.\n");
printf(" *\n");
printf(" * Permission is granted to anyone to use this software, alter it and re-\n");
printf(" * distribute it freely for any non-commercial, non-profit purpose subject to\n");
printf(" * the following restrictions:\n");
printf(" *\n");
printf(" *   1. The origin of this software must not be misrepresented; you must not\n");
printf(" *   claim that you wrote the original software. If you use this software in a\n");
printf(" *   product, an acknowledgment in the product documentation would be\n");
printf(" *   appreciated but is not required.\n");
printf(" *\n");
printf(" *   2. Altered source versions must be plainly marked as such, and must not\n");
printf(" *   be misrepresented as being the original software.\n");
printf(" *\n");
printf(" *   3. This notice may not be removed or altered from any distribution.\n");
printf(" *\n");
printf(" *   4. The names of this software and/or it's copyright holders may not be\n");
printf(" *   used to endorse or promote products derived from this software without\n");
printf(" *   specific prior written permission.\n");


printf("\n\n\n\n");
printf("******* license for AP-Ultra & LZSA crunchers ****************************\n\n\n\n");
printf(" * apultra.c - command line compression utility for the apultra library\n");
printf(" * Copyright (C) 2019 Emmanuel Marty\n");
printf(" *          https://github.com/emmanuel-marty\n");
printf(" *\n");
printf(" LZSA code use Zlib license\n");
printf(" Match finder use CC0 license due to using portions of code from Eric Bigger's\n");
printf("                                  Wimlib in the suffix array-based matchfinder\n");
printf(" *\n");
printf(" * This software is provided 'as-is', without any express or implied\n");
printf(" * warranty.  In no event will the authors be held liable for any damages\n");
printf(" * arising from the use of this software.\n");
printf(" *\n");
printf(" * Permission is granted to anyone to use this software for any purpose,\n");
printf(" * including commercial applications, and to alter it and redistribute it\n");
printf(" * freely, subject to the following restrictions:\n");
printf(" *\n");
printf(" * 1. The origin of this software must not be misrepresented; you must not\n");
printf(" *    claim that you wrote the original software. If you use this software\n");
printf(" *    in a product, an acknowledgment in the product documentation would be\n");
printf(" *    appreciated but is not required.\n");
printf(" * 2. Altered source versions must be plainly marked as such, and must not be\n");
printf(" *    misrepresented as being the original software.\n");
printf(" * 3. This notice may not be removed or altered from any source distribution.\n");
printf(" *\n");
printf(" * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori\n");
printf(" *\n");
printf(" * Inspired by cap by Sven-Ake Dahl. https://github.com/svendahl/cap\n");
printf(" * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/\n");
printf(" * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4\n");
printf(" * With help and support from spke <zxintrospec@gmail.com>\n");
printf("\n\n\n\n");
printf("*** license for CDT export (record11() function and some other code extracts) ***\n\n\n\n");
printf("Author: CNGSoft http://cngsoft.no-ip.org , the tool is in GPL2 licence,\nCopyright (C) 2007 Free Software Foundation, Inc. https://fsf.org\n");
printf("\n");
printf("\n");
#endif

printf("\n\n");



	exit(0);
}

int _internal_check_flexible(char *fxp) {
	#undef FUNC
	#define FUNC "_internal_check_flexible"

	char *posval,*posvar;
	int cpt=0,i;

	posvar=strstr(fxp,"%s");
	posval=strstr(fxp,"%");
	if (!posval || !posvar) {
		printf("invalid flexible export string, need 2 formated fields, example: \"%%s %%d\"\n");
		exit(1);
	}
	if (posval<posvar) {
		printf("invalid flexible export string, must be %%s before the %% for value, example: \"%%s %%d\"\n");
		exit(1);
	}
	for (i=0;fxp[i];i++) if (fxp[i]=='%') cpt++;
	if (cpt>2) {
		printf("invalid flexible export string, must be only two formated fields, example: \"%%s %%d\"\n");
		exit(1);
	}

	return 1;
}
/*
	ParseOptions
	
	used to parse command line and configuration file
*/
int ParseOptions(char **argv,int argc, struct s_parameter *param)
{
	#undef FUNC
	#define FUNC "ParseOptions"
	
	char *sep;
	int i=0;

	if (strcmp(argv[i],"-autotest")==0) {
		RasmAutotest();
	} else if (strcmp(argv[i],"-uz")==0) {
		param->as80=2;
	} else if (strcmp(argv[i],"-msep")==0) {
		if (i+1<argc) {
			param->module_separator=argv[++i][0];
		} else Usage(1);
	} else if (strcmp(argv[i],"-twe")==0) {
		param->erronwarn=1;
	} else if (strcmp(argv[i],"-pasmo")==0) {
		param->pasmo=1;
	} else if (strcmp(argv[i],"-ass")==0) {
		param->as80=1;
	} else if (strcmp(argv[i],"-amper")==0 || strcmp(argv[i],"--noampersand")==0) {
		param->noampersand=1;
	} else if (strcmp(argv[i],"-eb")==0) {
		param->export_brk=1;
	} else if (strcmp(argv[i],"-wu")==0) {
		param->warn_unused=1;
	} else if (strcmp(argv[i],"-xpr")==0) {
		param->xpr=1;
	} else if (strcmp(argv[i],"-dams")==0) {
	} else if (strcmp(argv[i],"-void")==0) {
		param->macrovoid=1;
	} else if (strcmp(argv[i],"-xr")==0) {
		param->extended_error=1;
	} else if (strcmp(argv[i],"-eo")==0) {
		param->edskoverwrite=1;
	} else if (strcmp(argv[i],"-depend=make")==0) {
		param->dependencies=E_DEPENDENCIES_MAKE;
		param->checkmode=1;
	} else if (strcmp(argv[i],"-depend=list")==0) {
		param->dependencies=E_DEPENDENCIES_LIST;
		param->checkmode=1;
	} else if (strcmp(argv[i],"-no")==0) {
		param->checkmode=1;
	} else if (strcmp(argv[i],"-w")==0) {
		param->nowarning=1;
	} else if (argv[i][0]=='-')	{
		switch(argv[i][1])
		{
			case 'I':
				if (argv[i][2]) {
					char *curpath;
					int l;
					l=strlen(argv[i]);
					curpath=MemMalloc(l); /* strlen(path)+2 */
					strcpy(curpath,argv[i]+2);
#ifdef OS_WIN
					if (argv[i][l-1]!='/' && argv[i][l-1]!='\\') strcat(curpath,"\\");
#else
					if (argv[i][l-1]!='/' && argv[i][l-1]!='\\') strcat(curpath,"/");
#endif
					FieldArrayAddDynamicValueConcat(&param->pathdef,&param->npath,&param->mpath,curpath);
					MemFree(curpath);
				} else {
					Usage(1);
				}
				break;
			case 'D':
				if ((sep=strchr(argv[i],'='))!=NULL) {
					if (sep!=argv[i]+2) {
						FieldArrayAddDynamicValueConcat(&param->symboldef,&param->nsymb,&param->msymb,argv[i]+2);
					} else {
						Usage(1);
					}
				} else {
					Usage(1);
				}
				break;
			case 'm':
				switch (argv[i][2]) {
					case 0:
						param->rough=0.0;
						return i;
					case 'e':
						if (argv[i][3]) Usage(1);
						if (i+1<argc) {
							param->maxerr=atoi(argv[++i]);
							return i;
						}
						Usage(1);
						break;
					default:Usage(1);break;
				}
				Usage(1);
				break;
			case 's':
				if (argv[i][2] && argv[i][3]) Usage(1);

				switch (argv[i][2]) {
					case 0:param->export_sym=1;return 0;
					case 'z':
						param->export_sym=5;return 0;
					case 'm':
						param->export_multisym=1;return 0;
					case 'b':
						param->export_snabrk=1;return 0;
					case 'p':
						param->export_sym=2;return 0;
					case 'w':
						param->export_sym=3;return 0;
					case 'c':
						if (i+1<argc) {
							param->export_sym=4;
							param->flexible_export=TxtStrDup(argv[++i]);
							param->flexible_export=MemRealloc(param->flexible_export,strlen(param->flexible_export)+3);
							strcat(param->flexible_export,"\n");
							/* check export string */
							if (_internal_check_flexible(param->flexible_export)) return i; else Usage(1);
						}
						Usage(1);
						break;
					case 'l':
						param->export_local=1;return 0;
					case 'v':
						param->export_var=1;return 0;
					case 'q':
						param->export_equ=1;return 0;
					case 'a':
						param->export_local=1;
						param->export_var=1;
						param->export_equ=1;
						return 0;
					case 's':
						param->export_local=1;
						param->export_sym=1;
						param->export_sna=1;return 0;
					default:
						Usage(1);
						break;
				}
				Usage(1);
				break;
			case 'l':
				if (argv[i][2]) Usage(1);
				if (i+1<argc) {
					FieldArrayAddDynamicValue(&param->labelfilename,argv[++i]);
					break;
				}	
				Usage(1);
				break;
		case 'i':
printf("@@@\n@@@ --> deprecated option, there is no need to use -i option to set input file <--\n@@@\n");
				Usage(0);
				break;
			case 'o':
				if (argv[i][2] && argv[i][3]) Usage(1);
				switch (argv[i][2]) {
					case 0:
						if (i+1<argc && param->outputfilename==NULL) {
							param->outputfilename=argv[++i];
							break;
						}	
						Usage(1);
						break;
					case 'a':
						param->automatic_radix=1;
						break;
					case 't':
						if (i+1<argc && param->tape_name==NULL) {
							param->tape_name=argv[++i];
							break;
						}
						Usage(1);
						break;
					case 'r':
						if (i+1<argc && param->rom_name==NULL) {
							param->rom_name=argv[++i];
							break;
						}
						Usage(1);
						break;
					case 'i':
						if (i+1<argc && param->snapshot_name==NULL) {
							param->snapshot_name=argv[++i];
							break;
						}
						Usage(1);
						break;
					case 'b':
						if (i+1<argc && param->binary_name==NULL) {
							param->binary_name=argv[++i];
							break;
						}
						Usage(1);
						break;
					case 'c':
						if (i+1<argc && param->cartridge_name==NULL) {
							param->cartridge_name=argv[++i];
							break;
						}
						Usage(1);
						break;
					case 'k':
						if (i+1<argc && param->breakpoint_name==NULL) {
							param->breakpoint_name=argv[++i];
							break;
						}
						Usage(1);
						break;
					case 's':
						if (i+1<argc && param->symbol_name==NULL) {
							param->symbol_name=argv[++i];
							break;
						}
						Usage(1);
						break;
					default:
						Usage(1);
						break;
				}
				break;
			case 'd':if (!argv[i][2]) printf("deprecated option -d\n"); else Usage(1);
				break;
			case 'a':if (!argv[i][2]) printf("deprecated option -a\n"); else Usage(1);
				break;
			case 'c':if (!argv[i][2]) printf("deprecated option -c\n"); else Usage(1);
				break;
			case 'v':
				if (!argv[i][2]) {
					param->display_stats=1;
				} else if (argv[i][2]=='2') {
					param->v2=1;
				}
				break;
			case 'n':if (!argv[i][2]) Licenses(); else Usage(1);
			case 'h':Usage(1);break;
			default:
				Usage(1);break;
		}
	} else {
		if (param->filename==NULL) {
			param->filename=TxtStrDup(argv[i]);
		} else if (param->outputfilename==NULL) {
			param->outputfilename=argv[i];
		} else Usage(1);
	}
	return i;
}

/*
	GetParametersFromCommandLine	
	retrieve parameters from command line and fill pointers to file names
*/
void GetParametersFromCommandLine(int argc, char **argv, struct s_parameter *param)
{
	#undef FUNC
	#define FUNC "GetParametersFromCommandLine"
	int i;
	
	for (i=1;i<argc;i++)
		i+=ParseOptions(&argv[i],argc-i,param);

	if (!param->filename) Usage(0);
	if (param->export_local && !param->export_sym) Usage(1); // à revoir?
}

/*
	main
	
	check parameters
	execute the main processing
*/


int main(int argc, char **argv)
{
	#undef FUNC
	#define FUNC "main"

	struct s_parameter param={0};
	int ret;

	param.maxerr=20;
	param.rough=0.5;
	param.module_separator='_';

	GetParametersFromCommandLine(argc,argv,&param);
	ret=Rasm(&param);
	#ifdef RDD
	/* private dev lib tools */
printf("checking memory\n");
	CloseLibrary();
	#endif
	exit(ret);
	return 0; // Open WATCOM Warns without this...
}

#endif


