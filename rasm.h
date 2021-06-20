
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
};

#ifndef INSIDE_RASM

//extern "C" {
int RasmAssemble(const char *datain, int lenin, unsigned char **dataout, int *lenout);
int RasmAssembleInfo(const char *datain, int lenin, unsigned char **dataout, int *lenout, struct s_rasm_info **debug);
void RasmFreeInfoStruct(struct s_rasm_info *debug);
//};

#endif


