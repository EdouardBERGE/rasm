#define PROGRAM_NAME      "RASM"
#define PROGRAM_VERSION   "1.8"
#define PROGRAM_DATE      "xx/12/2022"
#define PROGRAM_COPYRIGHT "© 2017 BERGE Edouard / roudoudou from Resistance"
#define RELEASE_NAME      "Flying monkeys"

#define RASM_VERSION PROGRAM_NAME" v"PROGRAM_VERSION" (build "PROGRAM_DATE")"
#define RASM_SNAP_VERSION PROGRAM_NAME" v"PROGRAM_VERSION

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
	int utf8enable;
	int freequote;
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
	int v2,remu;
	int warn_unused;
	char *symbol_name;
	char *binary_name;
	char *cartridge_name;
	char *snapshot_name;
	char *rom_name;
	char *tape_name;
	char *breakpoint_name;
	char *cprinfo_name;
	char **symboldef;
	int nsymb,msymb;
	char **pathdef;
	int npath,mpath;
	int noampersand;
	int cprinfo,cprinfoexport;
	char module_separator;
	int enforce_symbol_case;
	int verbose_assembling;
	int macro_multi_line;
};

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
	int run,start;
	unsigned char *emuram;
	int lenram;
};

#ifndef INSIDE_RASM

//extern "C" {
int RasmAssemble(const char *datain, int lenin, unsigned char **dataout, int *lenout);
int RasmAssembleInfo(const char *datain, int lenin, unsigned char **dataout, int *lenout, struct s_rasm_info **debug);
int RasmAssembleInfoIntoRAM(const char *datain, int lenin, struct s_rasm_info **debug, unsigned char *emuram, int ramsize);
int RasmAssembleInfoParam(const char *datain, int lenin, unsigned char **dataout, int *lenout, struct s_rasm_info **debug, struct s_parameter *param);
void RasmFreeInfoStruct(struct s_rasm_info *debug);
//};

#endif


