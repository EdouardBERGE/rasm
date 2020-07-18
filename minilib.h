#define __FILENAME__ "minilib.h"

#include<string.h>
#include<stdlib.h>
#include<stddef.h>
#include<stdint.h>
#include<stdarg.h>
#include<stdio.h>
#include<errno.h>
#include<ctype.h>
#include<time.h>
#include<math.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h> 
#ifdef OS_WIN
#include<io.h>
#include<limits.h>
#include<direct.h>
#define snprintf _snprintf
#endif
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#ifndef OS_WIN
#include<unistd.h>
#endif
#include<string.h>
#include<setjmp.h>
#include<errno.h>
#include<sys/timeb.h>

#ifndef ARG_MAX
#define ARG_MAX 8191
#endif



#define MemFree free
#define MemRealloc realloc
#define MemMalloc malloc
#define MemMove memmove
#ifdef OS_WIN
#define TxtStrDup _strdup
#else
#define TxtStrDup strdup
#endif

#define loginfo(...); {printf(__VA_ARGS__);printf("\n");}
#define logdebug(...); {printf(__VA_ARGS__);printf("\n");}
#define logwarn(...); {printf(__VA_ARGS__);printf("\n");}
#define logerr(...); {printf(__VA_ARGS__);printf("\n");}

#define INTERNAL_ERROR 1
#define ABORT_ERROR 7

#define MAX_LINE_BUFFER 16384

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static int _static_library_nbfile_opened=0;
static int _static_library_nbfile_opened_max=0;

void _internal_ObjectArrayAddDynamicValue(void **zearray, void *zeobject, int object_size,int curline, char *curfunc, char *cursource);
void _internal_ObjectArrayAddDynamicValueConcat(void **zearray, int *nbfields, int *maxfields, void *zeobject, int object_size, int curline, char *curfunc, char *cursource);
#define ObjectArrayAddDynamicValue(zearray,zeobject,objsize) _internal_ObjectArrayAddDynamicValue(zearray,zeobject,objsize,__LINE__,FUNC,__FILENAME__)
#define ObjectArrayAddDynamicValueConcat(zearray,nbv,maxv,zeobject,objsize) _internal_ObjectArrayAddDynamicValueConcat(zearray,nbv,maxv,zeobject,objsize,__LINE__,FUNC,__FILENAME__)
void _internal_IntArrayAddDynamicValueConcat(int **zearray, int *nbval, int *maxval, int zevalue, int curline, char *curfunc, char *cursource);
#define IntArrayAddDynamicValueConcat(zearray,nbv,maxv,zevalue) _internal_IntArrayAddDynamicValueConcat(zearray,nbv,maxv,zevalue,__LINE__,FUNC,__FILENAME__)
void _internal_FieldArrayAddDynamicValue(char ***zearray, char *zevalue, int curline, char *curfunc, char *cursource);
#define FieldArrayAddDynamicValue(zearray,zevalue) _internal_FieldArrayAddDynamicValue(zearray,zevalue,__LINE__,FUNC,__FILENAME__)
void _internal_FieldArrayAddDynamicValueConcat(char ***zearray, int *nbfields, int *maxfields, char *zevalue, int curline, char *curfunc, char *cursource);
#define FieldArrayAddDynamicValueConcat(zearray,nb,maxf,zevalue) _internal_FieldArrayAddDynamicValueConcat(zearray,nb,maxf,zevalue,__LINE__,FUNC,__FILENAME__)


long long FileGetSize(char *filename);

int MinMaxInt(int zeval, int zemin, int zemax)
{
        #undef FUNC
        #define FUNC "MinMaxInt"

        if (zeval<zemin) return zemin;
        if (zeval>zemax) return zemax;
        return zeval;
}

/* (c) FSF */
#ifdef __WATCOMC__
size_t strnlen (s, maxlen)
     register const char *s;
     size_t maxlen;
{
  register const char *e;
  size_t n;

  for (e = s, n = 0; *e && n < maxlen; e++, n++)
    ;
  return n;
}
#endif


char *TxtStrDupLen(char *str, int *len)
{
	#undef FUNC
	#define FUNC "TxtStrDupLen"
	
	char *newstr;
	*len=strlen(str)+1;
	newstr=MemMalloc(*len);
	strcpy(newstr,str);
	return newstr;
}

void _internal_ObjectArrayAddDynamicValueConcat(void **zearray, int *nbfields, int *maxfields, void *zeobject, int object_size, int curline, char *curfunc, char *cursource)
{
	#undef FUNC
	#define FUNC "ObjectArrayAddDynamicValueConcat"

	char *dst;

	if ((*zearray)==NULL) {
		*nbfields=1;
		*maxfields=3;
		(*zearray)=malloc((*maxfields)*object_size);
	} else {
		*nbfields=(*nbfields)+1;
		if (*nbfields>=*maxfields) {
			*maxfields=(*maxfields)*2;
			(*zearray)=realloc((*zearray),(*maxfields)*object_size);
		}
	}
	/* using direct calls because it is more interresting to know which is the caller */
	dst=((char *)(*zearray))+((*nbfields)-1)*object_size;
	/* control of memory for overflow */
	memcpy(dst,zeobject,object_size);
}

/***
	Add a value to a list of integers
	Reallocate memory on the fly

	zearray: pointer to the array of integers
	nbval:   pointer to the current index
	maxval:  pointer to the current allocated memory
	zevalue: integer value to concat to	
	       
	output: the pointer to the array is modified because the
	function reallocate memory to store the new string at
	the end of the array.
*/
void _internal_IntArrayAddDynamicValueConcat(int **zearray, int *nbval, int *maxval, int zevalue, int curline, char *curfunc, char *cursource)
{
	#undef FUNC
	#define FUNC "IntArrayAddDynamicValue"

	if ((*zearray)==NULL) {
		*nbval=1;
		*maxval=4;
		(*zearray)=MemMalloc(sizeof(int)*(*maxval));
	} else {
		*nbval=*nbval+1;
		if (*nbval>*maxval) {
			*maxval=(*maxval)*2;
			(*zearray)=realloc((*zearray),sizeof(int)*(*maxval));
		}
	}
	(*zearray)[(*nbval)-1]=zevalue;
}

/***
	CSVGetFieldArrayNumber
	
	count array elements
*/
int CSVGetFieldArrayNumber(char **myfield)
{
	#undef FUNC
	#define FUNC "CSVGetFieldArrayNumber"
	
	int n=0;
	
	if (myfield==NULL)
		return 0;
	
	while (myfield[n]!=NULL) n++;
	
	return n;

}
/***
 *      TxtSplit
 *
 *      split the parameter string into an array of strings
 */
char **TxtSplitWithChar(char *in_str, char split_char)
{
        #undef FUNC
        #define FUNC "TxtSplitWithChar"

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

/***
	Add a value to a list of strings
	Reallocate memory on the fly
	
	input: pointer to an array of string
	       string value
	       
	output: the pointer to the array is modified because the
	function reallocate memory to store the new string at
	the end of the array.
*/
void _internal_FieldArrayAddDynamicValue(char ***zearray, char *zevalue, int curline, char *curfunc, char *cursource)
{
	#undef FUNC
	#define FUNC "FieldArrayAddDynamicValue"
	int nbfield;
	if ((*zearray)==NULL) nbfield=2; else nbfield=CSVGetFieldArrayNumber((*zearray))+2;
	/* using direct calls because it is more interresting to know which is the caller */
	(*zearray)=realloc((*zearray),nbfield*sizeof(char *));
	(*zearray)[nbfield-2]=TxtStrDup(zevalue);
	(*zearray)[nbfield-1]=NULL;
}
void _internal_FieldArrayAddDynamicValueConcat(char ***zearray, int *nbfields, int *maxfields, char *zevalue, int curline, char *curfunc, char *cursource)
{
	#undef FUNC
	#define FUNC "FieldArrayAddDynamicValueConcat"

	if ((*zearray)==NULL) {
		*nbfields=1;
		*maxfields=10;
		(*zearray)=realloc(NULL,(*maxfields)*sizeof(char *));
	} else {
		*nbfields=(*nbfields)+1;
		if (*nbfields>=*maxfields) {
			*maxfields=(*maxfields)*2;
			(*zearray)=MemRealloc((*zearray),(*maxfields)*sizeof(char *));
		}
	}
	/* using direct calls because it is more interresting to know which is the caller */
	(*zearray)[(*nbfields)-1]=TxtStrDup(zevalue);
	(*zearray)[(*nbfields)]=NULL;
}



/***
	CSVFreeFields

	free allocated memory for fields
*/
void CSVFreeFields(char **fields)
{
	#undef FUNC
	#define FUNC "CSVFreeFields"
	
	int i=0;
	if (fields!=NULL)
	{
		/*loginfo("%8X",fields);
		if (fields[i]==NULL)
			loginfo("NULL i=%d",i); */
		
		while (fields[i]!=NULL)
		{
			MemFree(fields[i]);
			i++;
		}
		MemFree(fields);
	}
}
void FreeArrayDynamicValue(char ***zearray)
{
	CSVFreeFields(*zearray);
	*zearray=NULL;
}


void CSVFreeFields(char **fields);
#define FreeFields(fields) CSVFreeFields(fields)


/************************** File operation ******************************/


/***
	s_fileid
	structure used by FileReadLine and FileWriteLine to manage multiple files at a time
*/
struct s_fileid
{
	FILE *file_id;
	char *filename;
	char opening_type[4];
	int cpt;
	struct s_fileid *next;
	/* v2 */
	int closed;
	unsigned long curpos;
};

static struct s_fileid *fileidROOT=NULL;

/***
	FileGetStructFromName
	
	input: filename
	output: file structure
		NULL if not found
*/
struct s_fileid *FileGetStructFromName(char *filename)
{
	#undef FUNC
	#define FUNC "FileGetStructFromName"
	struct s_fileid *curfile;
	struct s_fileid *prevfile;

	if (!filename)
	{
		logerr("filename must not be NULL");
		exit(ABORT_ERROR);
	}

	if (strnlen(filename,PATH_MAX)==PATH_MAX)
	{
		logerr("cannot open this file because the argument size is bigger than PATH_MAX (%d)",PATH_MAX);
		exit(ABORT_ERROR);
	}
	
	/* try to find the file in the list */
	curfile=fileidROOT;
	prevfile=NULL;
	while (curfile!=NULL)
	{
		if (!strcmp(curfile->filename,filename)) {		
			break;
		} else {
			prevfile=curfile;
			curfile=curfile->next;
		}
	}
	/* put the struct to the top of the list */
	if (curfile && prevfile) {
		prevfile->next=curfile->next;
		curfile->next=fileidROOT;
		fileidROOT=curfile;
	}
	
	return curfile;
}

/***
	FileOpen function
	
	open a file in any mode (r,w,a,r+,w+,a+)
	check if the file is already open
	check for conflicts	
*/
FILE *FileOpen(char *filename, char *opening_type)
{
	#undef FUNC
	#define FUNC "FileOpen"
	struct s_fileid *curfile;
	struct s_fileid *oldfile;
	
	/* check parameters coherency */
	if (strlen(opening_type)>3)
	{
		logerr("illegal opening type (too long)");
		exit(ABORT_ERROR);
	}	
	if (strcmp(opening_type,"a") && strcmp(opening_type,"w") && strcmp(opening_type,"r")
	 && strcmp(opening_type,"a+")  && strcmp(opening_type,"w+") && strcmp(opening_type,"r+") && strcmp(opening_type,"rb"))
	{
		logerr("illegal opening type [%s]\nallowed options are: r,w,a,r+,w+,a+",opening_type);
		exit(ABORT_ERROR);
	}		
	
	curfile=FileGetStructFromName(filename);
	
	/* if curfile is NULL then the file is not opened yet */
	if (!curfile)
	{
		/* insert a new record */
		_static_library_nbfile_opened++;
		if (_static_library_nbfile_opened>_static_library_nbfile_opened_max)
			_static_library_nbfile_opened_max=_static_library_nbfile_opened;
		curfile=MemMalloc(sizeof(struct s_fileid));
		memset(curfile,0,sizeof(struct s_fileid));
		curfile->filename=MemMalloc(strlen(filename)+1);
		strcpy(curfile->filename,filename);
		strcpy(curfile->opening_type,opening_type);
		curfile->next=fileidROOT;
		curfile->cpt=0;
		fileidROOT=curfile;
	}
	else
	{
		if (strcmp(curfile->opening_type,opening_type))
		{
			logerr("You can't open the file [%s] in [%s] mode cause it's already open in [%s] mode",filename,opening_type,curfile->opening_type);
			exit(ABORT_ERROR);
		}
		if (!curfile->closed) {
			/* already opened, just return the id */
			return curfile->file_id;
		}
	}
	
	curfile->file_id=fopen(filename,opening_type);
	if (!curfile->file_id)
	{
		if (errno==EMFILE) {
			/* too many files opened, close the latest of the list */
			oldfile=fileidROOT;
			while (oldfile->next && !oldfile->next->closed) oldfile=oldfile->next;
			if (oldfile==curfile) {
				logerr("cannot open a single file!");
				exit(INTERNAL_ERROR);
			}
			/* save position and close the file */
			oldfile->curpos=ftell(oldfile->file_id);
			fclose(oldfile->file_id);
			oldfile->file_id=0;
			oldfile->closed=1;
			/* then try again */
			curfile->file_id=fopen(filename,opening_type);
			if (curfile->file_id) {
				if (curfile->curpos) {
					fseek(curfile->file_id,curfile->curpos,SEEK_SET);
					curfile->curpos=0;
				}

				logdebug("opening file [%s] in %s mode (too many opened files limit reached)",filename,opening_type);
				return curfile->file_id;
			}
		}
		
		logerr("failed to open file [%s] with mode [%s]",filename,opening_type);
		logerr("check empty space and permissions");
		exit(ABORT_ERROR);
	}
	
	/* go on previous position */
	if (curfile->curpos) {
		fseek(curfile->file_id,curfile->curpos,SEEK_SET);
		curfile->curpos=0;
	}
	
	//logdebug("opening file [%s] in %s mode",filename,opening_type);
	return curfile->file_id;
}

struct s_fileid *FileGetStructFromID(FILE *file_id);

void FileSeekBinary(char *filename,int pos, int st)
{
	#undef FUNC
	#define FUNC "FileSeekBinary"
	FILE *last_id=NULL;
	struct s_fileid *curfile=NULL;
	int err;
	
	last_id=FileOpen(filename,"r");
	if ((err=fseek(last_id,pos,st))!=0) {
		logerr("error seeking %s (%d)",filename,ferror(last_id));
		exit(ABORT_ERROR);
	}
	
	curfile=FileGetStructFromID(last_id);
	switch (st) {
		case SEEK_SET: curfile->curpos=pos; break;
		case SEEK_CUR: curfile->curpos+=pos; break;
		case SEEK_END: logerr("TODO§§§"); break;
		default:logerr("unknown SEEK mode!");exit(INTERNAL_ERROR);
	}
}

/***
	FileGetStructFromID
	
	retrieve the file structure from the tree, with his ID
*/
struct s_fileid *FileGetStructFromID(FILE *file_id)
{
	#undef FUNC
	#define FUNC "FileGetStructFromID"
	struct s_fileid *curfile;
	struct s_fileid *prevfile;
	
	curfile=fileidROOT;
	prevfile=NULL;
	while (curfile!=NULL)
	{
		if (curfile->file_id==file_id) {
			break;
		} else {
			prevfile=curfile;
			curfile=curfile->next;
		}
	}
	if (!curfile)
	{
		logerr("ID requested for an unknown file! (was supposed to be opened)");
		exit(INTERNAL_ERROR);
	}
	/* put the struct to the top of the list */
	if (prevfile) {
		prevfile->next=curfile->next;
		curfile->next=fileidROOT;
		fileidROOT=curfile;
	}
	
	return curfile;
}

/***
	FileClose function
	
	check for closing return code
	free the memory file structure
*/
void FileClose(FILE *file_id)
{
	#undef FUNC
	#define FUNC "FileClose"
	struct s_fileid *curfile;
	struct s_fileid *unlinkcell;
	
	curfile=FileGetStructFromID(file_id);

	if (!curfile->closed) {
		if (fclose(curfile->file_id))
		{
			logerr("error while closing file [%s]",curfile->filename);
		}
	} else {
		/* already closed */
	}

	/* unlink the cell from ROOT */
	if (curfile==fileidROOT)
	{
		fileidROOT=curfile->next;
	}
	else
	{
		unlinkcell=fileidROOT;
		while (unlinkcell->next!=curfile)
			unlinkcell=unlinkcell->next;
		unlinkcell->next=curfile->next;
	}
	MemFree(curfile->filename);
	MemFree(curfile);
	_static_library_nbfile_opened--;
}

/***
	FileAddCPT
	
	add n to counter when reading or writing in a file
*/
void FileAddCPT(FILE *file_id, int n)
{
	#undef FUNC
	#define FUNC "FileAddCPT"
	struct s_fileid *curfile;
	
	curfile=FileGetStructFromID(file_id);
	curfile->cpt+=n;
}
#define FileIncCPT(file_id) FileAddCPT(file_id,1)

/***
	FileGetCPT
	
	Get file counter information
	input: file_id
*/
int FileGetCPT(FILE *file_id)
{
	#undef FUNC
	#define FUNC "FileGetCPT"
	struct s_fileid *curfile;
	
	curfile=FileGetStructFromID(file_id);
	return curfile->cpt;
}
/***
	FileGetCPTFromName
	
	Get file counter information
	input: filename
*/
int FileGetCPTFromName(char *filename)
{
	#undef FUNC
	#define FUNC "FileGetCPTFromName"
	struct s_fileid *curfile;
	
	curfile=FileGetStructFromName(filename);
	if (!curfile)
	{
		logerr("You requested a counter for a file that is not opened! [%s]",filename);
		exit(INTERNAL_ERROR);
	}
	return curfile->cpt;
}



char *_internal_fgetsClose(char *buffer, int maxlen, FILE *f)
{
	#undef FUNC
	#define FUNC "_internal_fgetsClose"
	return NULL;
}

/***
	FileReadLine/FileReadLineXML function
	
	input:
	- filename

	output:
	- a static buffer with the line read
	
	this function can handle many file simultaneously
	just use different filenames without regarding to opened/closed pointer
	the opened handles are automatically closed when the end of the file is reached	
	you are only limited by the system, not by the code
	
	the XML version ass carriage returns after closing tag
	this can be usefull for parsing when reading XML file in a single line
*/
enum e_reading_mode {
RAW_READING,
CLOSE_READING
};

char *_internal_fgetsmulti(char *filename, int read_mode)
{
	#undef FUNC
	#define FUNC "_internal_fgetsmulti"
	static char buffer[MAX_LINE_BUFFER+1]={0};
	FILE *last_id=NULL;
	char * (*_file_get_string)(char *, int, FILE *);
	
	last_id=FileOpen(filename,"r");
	
	switch (read_mode)
	{
		case RAW_READING:_file_get_string=fgets;break;
		case CLOSE_READING:_file_get_string=_internal_fgetsClose;break;
		default:logerr("Unknown read mode! (%d)",read_mode);
	}	
	
	if (_file_get_string(buffer,MAX_LINE_BUFFER,last_id)!=NULL)
	{
		FileIncCPT(last_id);
		if (strnlen(buffer,MAX_LINE_BUFFER)==MAX_LINE_BUFFER)
		{
			logerr("line %d is too long! More than %d characters\n",FileGetCPT(last_id),MAX_LINE_BUFFER);
			exit(INTERNAL_ERROR);
		}
		return buffer;
	}
	else
	{
		/* End of file, we close the handle */
		logdebug("%d line(s) read from %s",FileGetCPT(last_id),filename);
		FileClose(last_id);
		return NULL;
	}
}

char **_internal_fgetsmultilines(char *filename, int read_mode)
{
	#undef FUNC
	#define FUNC "_internal_fgetsmultilines"
	static char buffer[MAX_LINE_BUFFER+1]={0};
	FILE *last_id=NULL;
	char * (*_file_get_string)(char *, int, FILE *);
	char **lines_buffer=NULL;
	int cur_line=0,max_line=0;
	
	last_id=FileOpen(filename,"r");
	
	switch (read_mode)
	{
		case RAW_READING:_file_get_string=fgets;break;
		case CLOSE_READING:_file_get_string=_internal_fgetsClose;break;
		default:logerr("Unknown read mode! (%d)",read_mode);
	}	


	while (_file_get_string(buffer,MAX_LINE_BUFFER,last_id)!=NULL)
	{
		FileIncCPT(last_id);
		if (strnlen(buffer,MAX_LINE_BUFFER)==MAX_LINE_BUFFER)
		{
			logerr("line %d is too long! More than %d characters\n",FileGetCPT(last_id),MAX_LINE_BUFFER);
			exit(INTERNAL_ERROR);
		}
		FieldArrayAddDynamicValueConcat(&lines_buffer,&cur_line,&max_line,buffer);
	}
	/* if file is empty, we allocate an empty struct */
	if (!lines_buffer) {
		/* create the end of the list */
		lines_buffer=MemMalloc(sizeof(char*));
		lines_buffer[0]=NULL;
	}

	/* End of file, we close the handle */
	logdebug("%d line(s) read from %s",FileGetCPT(last_id),filename);
	FileClose(last_id);

	return lines_buffer;
}

#define FileReadClose(filename) _internal_fgetsmulti(filename,CLOSE_READING)
#define FileReadLine(filename) _internal_fgetsmulti(filename,RAW_READING)

long FileReadPos(char *filename)
{
	#undef FUNC
	#define FUNC "FileReadPos"

	FILE *last_id;

	last_id=FileOpen(filename,"r");
	return ftell(last_id);
}

/***

	FileWriteLine function
	
	input:
	- filename
	- a buffer with the line to write
	
	the function do not close the file until a NULL buffer is sent.
	It prevents from too much write flushes
	
	you can use it with many files simultaneously but do not forget
	to close your files if you use this function in a loop of filenames
	or the system will warn you
*/

void FileWriteLine(char *filename, char *buffer)
{
	#undef FUNC
	#define FUNC "FileWriteLine"
	FILE *last_id=NULL;
	
	last_id=FileOpen(filename,"a+");

	if (buffer!=NULL)
	{	
		fputs(buffer,last_id);
		FileIncCPT(last_id);
	}
	else
	{
		/* NULL buffer sent, this means End of file, we close the handle */
		//logdebug("%d line(s) written to %s",FileGetCPT(last_id),filename);
		FileClose(last_id);
	}
}
void FileWriteLines(char *filename, char **buffer)
{
	#undef FUNC
	#define FUNC "FileWriteLines"
	FILE *last_id=NULL;
	int i;
	
	last_id=FileOpen(filename,"a+");
	for (i=0;buffer[i];i++) {
		fputs(buffer[i],last_id);
		FileIncCPT(last_id);
	}
	//logdebug("%d line(s) written to %s",FileGetCPT(last_id),filename);
	FileClose(last_id);
}

/***
	FileReadBinary function
	
	read n bytes into buffer
	return number of byte really read
	
	as other File functions, you can use it with many files simultaneously
*/
int FileReadBinary(char *filename,char *data,int n)
{
	#undef FUNC
	#define FUNC "FileReadBinary"
	FILE *last_id=NULL;
	int nn;
	
	last_id=FileOpen(filename,"rb");

	if (data==NULL)
	{
		FileClose(last_id);
		return 0;
	}
	
	nn=fread(data,1,n,last_id);
	//printf("%d bytes lus\n",nn);
	FileAddCPT(last_id,nn);
	if (nn!=n)
	{
		/* if eof is set, this is not an error */
		if (feof(last_id))
		{
			//logdebug("%d byte(s) read from %s",FileGetCPT(last_id),filename);
		}
		else
		if (ferror(last_id))
		{
			if (!nn) {
				logerr("cannot read %s",filename);
			}
			else {
				logerr("error %d bytes were read during reading of %d bytes of %s",nn,n,filename);
			}
			exit(ABORT_ERROR);
		}
		else
		{
			logerr("error during read of %s (but no error neither eof flag set)",filename);
			exit(INTERNAL_ERROR);
		}				
		FileClose(last_id);
	}
	return nn;
}

/***
	FileTruncate function
	set file to zero size then leave	
*/
int FileTruncate(char *filename)
{
#undef FUNC
#define FUNC "FileTruncate"
FILE *last_id=NULL;

#ifdef OS_WIN
int sr;
last_id=FileOpen(filename,"w");
sr=_setmode(_fileno(last_id), _O_BINARY );
if (sr==-1) {
logerr("FATAL: cannot set binary mode for writing");
exit(ABORT_ERROR);
}
#else
last_id=FileOpen(filename,"a+");
#endif
FileClose(last_id);
return 0;
}

/***
	FileWriteBinary function
	
	write n bytes from buffer to file
	return number of byte really written
	
	as other File functions, you can use it with many files simultaneously
*/
int FileWriteBinary(char *filename,char *data,int n)
{
	#undef FUNC
	#define FUNC "FileWriteBinary"
	FILE *last_id=NULL;
	int nn;
								
	#ifdef OS_WIN
	last_id=FileOpen(filename,"w");
	#else
	last_id=FileOpen(filename,"a+");
	#endif
	if (data!=NULL)
	{	 
		#ifdef OS_WIN
		int sr;
		sr=_setmode(_fileno(last_id), _O_BINARY );
		if (sr==-1) {
			logerr("FATAL: cannot set binary mode for writing");
			exit(ABORT_ERROR);
		}				
		#endif
		nn=fwrite(data,1,n,last_id);
		FileAddCPT(last_id,nn);
		/* we must always write the same amount of data */
		if (n!=nn)
		{
			if (ferror(last_id))
			{
				if (!nn) {
					logerr("cannot write %s",filename);
					logerr("%s",strerror(errno));
				}
				else{
					logerr("error during write of %s (%d byte(s))",filename,FileGetCPT(last_id));
				}
				exit(ABORT_ERROR);
			}
			else
			{
				logerr("error during write of %s (but no error neither eof flag set) %d byte(s) written",filename,FileGetCPT(last_id));
				exit(INTERNAL_ERROR);
			}
		}
	}
	else
	{
		/* NULL buffer sent, this means End of file, we close the handle */
		//logdebug("%d byte(s) written to %s",FileGetCPT(last_id),filename);
		FileClose(last_id);
		nn=0;
	}
	return nn;
}
/*** macro to close binary files */
#define FileWriteLineClose(filename) FileWriteLine(filename,NULL)
#define FileReadBinaryClose(filename) FileReadBinary(filename,NULL,0)
#define FileWriteBinaryClose(filename) FileWriteBinary(filename,NULL,0)

/***
	FileGetStat
	
	input: filename
	return the stat structure of a given file
*/
struct stat *FileGetStat(char *filename)
{
	#undef FUNC
	#define FUNC "FileGetStat"
	struct stat *filestat;
#ifdef OS_WIN
	struct _stat winstat;
#endif

	if (!filename)
	{
		logerr("filename must not be NULL");
		exit(ABORT_ERROR);
	}
	/* check after by the system but... */
	if (strnlen(filename,PATH_MAX)==PATH_MAX)
	{
		logerr("cannot open this file because the argument size is bigger than PATH_MAX (%d)",PATH_MAX);
		logerr("[%s]",filename);
		exit(ABORT_ERROR);
	}
	
	filestat=MemMalloc(sizeof(struct stat));
	memset(filestat,0,sizeof(struct stat));
#ifdef OS_WIN
	if (_stat(filename,&winstat)!=0)
#else	
	if (stat(filename,filestat) && errno!=ENOENT)
#endif
	{
		logerr("stat %s failed",filename);
		switch (errno)
		{
			case EACCES:logerr("Search permission is denied for one of the directories in the path prefix of path.");break;
			case EBADF:logerr("filedes is bad.");break;
			case EFAULT:logerr("Bad address.");break;
			//case ELOOP:logerr("Too many symbolic links encountered while traversing the path.");break;
			case ENAMETOOLONG:logerr("File name too long.");break;
			case ENOMEM:logerr("Out of memory (i.e. kernel memory).");break;
			case ENOTDIR:logerr("A component of the path is not a directory.");break;
			default:logerr("Unknown error %d during stat: %s",errno,strerror(errno));
		}
		exit(ABORT_ERROR);
	}
#ifdef OS_WIN
	filestat->st_size=winstat.st_size;
	filestat->st_mode=winstat.st_mode;
	filestat->st_atime=winstat.st_atime; /* last access       */
	filestat->st_mtime=winstat.st_mtime; /* last modification */
	filestat->st_ctime=winstat.st_ctime; /* time of creation  */
#endif
	return filestat;
}

/***
	FileGetSize
	
	input: filename
	output: the size in bytes of the file
*/
long long FileGetSize(char *filename)
{
	#undef FUNC
	#define FUNC "FileGetSize"
	struct stat *filestat;
	long long nn;
	
	filestat=FileGetStat(filename);
	nn=filestat->st_size;
	MemFree(filestat);
	//logdebug("size of %s = %d (%dkb)",filename,nn,nn/1024);
	return nn;
}

/***
 *	append a string
 *	return the number of char appened
 */
int strappend(char *Adst, char *Asrc)
{
	int Lcpt=0;

	/* must be at the end to concat */
	while (*Adst) {
		Adst++;
	}

	/* concat and count */
	while (*Asrc) {
		*Adst++=*Asrc++;
		Lcpt++;
	}
	*Adst=0;

	return Lcpt;
}

char * TxtTrimEnd(char *in_str)
{
	#undef FUNC
	#define FUNC "TxtTrimEnd"

	char *curs;
	char *space=NULL;

	curs=in_str;
	/* read the string and save the first space position, unfollowed by another character */
	while (*curs) {
		//if (*curs==' ' || *curs==0x0D || *curs==0x0A || *curs=='\t') {
		/* this include tab, space, line feed, carriage return, ...*/
		if (*curs<=' ') {
			if (!space) {
				space=curs;
			}
		} else {
			space=NULL;
		}
		curs++;
	}
	if (space) {
		*space=0;
	}

	return in_str;
}

int LookForFile(char *filename, char *dirname)
{
	#undef FUNC
	#define FUNC "LookForFile"

	char fullpath[PATH_MAX+1];	
	int pathlen;

	/* checked by DirReadEntry but it is more convenient to have an error inside this function */
	if (!filename)
	{
		logerr("You must specify at least a filename with the path, or a filename and a dirname. Filename cannot be NULL!");
		exit(INTERNAL_ERROR);
	}

	pathlen=strnlen(filename,PATH_MAX);
	if (pathlen==PATH_MAX)
	{
		logerr("cannot look for this file or directory because the argument size is bigger than PATH_MAX (%d)",PATH_MAX);
		exit(ABORT_ERROR);
	}

	if (!dirname) {
#ifdef OS_WIN
		if (_access(filename,0)==0) return 1; else return 0;
#else
		if (access(filename,F_OK)!=-1) return 1; else return 0;
#endif
	} else {
		pathlen+=strnlen(dirname,PATH_MAX-pathlen);
		if (pathlen>=PATH_MAX-1)
		{
			logerr("cannot look for this file or directory because the full path size is bigger than PATH_MAX (%d)",PATH_MAX);
			exit(ABORT_ERROR);
		}
		sprintf(fullpath,"%s/%s",dirname,filename);
#ifdef OS_WIN
		if (_access(fullpath,0)==0) return 1; else return 0;
#else
		if (access(fullpath,F_OK)!=-1) return 1; else return 0;
#endif
	}
}

#define FileExists(zefile) LookForFile(zefile,NULL)

int _internal_do_remove(char *zename, char *zetype)
{
	#undef FUNC
	#define FUNC "_do_remove"

	if (zename==NULL)
	{
		logerr("the argument cannot be NULL");
		exit(INTERNAL_ERROR);
	}
	if (strnlen(zename,PATH_MAX)==PATH_MAX)
	{
		logerr("cannot remove this file or directory because the argument size is bigger than PATH_MAX (%d)",PATH_MAX);
		exit(ABORT_ERROR);
	}
	
	if (remove(zename))
	{
		switch (errno)
		{
			case EACCES:logerr("Write permission is denied for the directory from which the %s [%s] is to be removed, or the directory has the sticky bit set and you do not own the file.",zetype,zename);break;
			case EBUSY:logerr("This error indicates that the %s [%s] is being used by the system in such a way that it can't be unlinked. For example, you might see this error if the file name specifies the root directory or a mount point for a file system.",zetype,zename);break;
			case ENOENT:logerr("The %s named [%s] to be deleted doesn't exist.",zetype,zename);break;
			case EPERM:logerr("On some systems unlink cannot be used to delete the name of a directory [%s], or at least can only be used this way by a privileged user. To avoid such problems, use rmdir to delete directories.",zename);break;
			case EROFS:logerr("The directory containing the %s named [%s] to be deleted is on a read-only file system and can't be modified.",zetype,zename);break;
			case ENOTEMPTY:logerr("The directory [%s] to be deleted is not empty.",zename);break;
			default:logerr("Unknown error %d during remove [%s]: %s",errno,zename,strerror(errno));
		}
		exit(ABORT_ERROR);
	}
	//logdebug("%s deleted",zename);
	return 0;	
}

#define FileRemove(filename) _internal_do_remove(filename,"file")

void FileRemoveIfExists(char *filename)
{
	#undef FUNC
	#define FUNC "FileRemoveIfExists"
	if (FileExists(filename))
		FileRemove(filename);
}


#undef __FILENAME__
