#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <libelf.h>
#include <gelf.h>

#define BUFFER_SIZE 1024
#define DIE(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)


/* function which returns the type of the symbol
 using the default notation of nm
 in case of error returns 'x' */
char get_symbol_type(char* symbol_name,char *filename) {
    char cmd[BUFFER_SIZE];
    char output[BUFFER_SIZE];
    char type;
    int addr;

    // create command to run nm on the current executable
    snprintf(cmd, BUFFER_SIZE, "nm %s | grep  '%s$'",
             filename, symbol_name);

    // run command and capture output
    FILE* stream = popen(cmd, "r");
    if (stream == NULL) {
		return 'x';
    }
    fgets(output, BUFFER_SIZE, stream);
    pclose(stream);

    // parse symbol type from output
    if (sscanf(output, "%x %c", &addr, &type) != 2) {
		return 'x';
    }

    return type;
}

/*function which prints the symbols from the symbol table that nm prints this way:
[i] Name: 
    Type: 
    Value: 
    Section Number: 
    Section Value: 
*/
void print_symbol_table(Elf *elf, Elf_Scn *scn, char *filename) {
    Elf_Data *data;
    GElf_Shdr shdr;
    int count = 0;
	int y =0; // output number
	int cs =0; // section counter
	char sec[20];
	unsigned char bind;
	char p_type;
	char symbol_name[BUFFER_SIZE];

    /* Get the descriptor.  */
    if (gelf_getshdr(scn, &shdr) != &shdr)
        DIE("(getshdr) %s", elf_errmsg(-1));

    data = elf_getdata(scn, NULL);
    count = shdr.sh_size / shdr.sh_entsize;

	// count the number of sections
	for (int i = 0; i < count; ++i) {
			GElf_Sym sym;
			gelf_getsym(data, i, &sym);
			if (sym.st_info == 3){
				cs += 1;
			}
	}
	
	Elf64_Addr section[cs];	// array which holds the values of sections 
	cs = 0;
	
	fprintf(stderr, "Printing symbol table.\n");
	for (int i = 0; i < count; ++i) {
			GElf_Sym sym;
			gelf_getsym(data, i, &sym);
			
			if ( sym.st_name != 0){ // do not print symbols without name
				if (sym.st_info != 4){	//do not print file-type
					y += 1;
					// translate the type
					bind = ELF64_ST_BIND(sym.st_info);
					if (bind == STB_WEAK){
						p_type = 'w';
					}else if (sym.st_shndx == SHN_UNDEF){
						if (bind == STB_LOCAL){
							p_type = 'u';
						}else{
							p_type = 'U';
						}
					}else{
						strcpy(symbol_name, elf_strptr(elf, shdr.sh_link, sym.st_name));
						p_type = get_symbol_type(symbol_name,filename);
					}
					if (p_type == 'x'){
						if (bind == STB_LOCAL){
							p_type = 'u';
						}else{
							p_type = 'U';
						}
					}
					// for beautifying the outpout
					if (y>=10){
						// print [y], y = number of printed symbol
						// print the name of the symbol
						fprintf(stderr,"[%d] Name: %s\n",y,elf_strptr(elf, shdr.sh_link, sym.st_name));
						fprintf(stderr,"     Type: %c\n",p_type);	// print the type of the symbol 
						if ( sym.st_value == 0){	// leave blank the following values	
							printf("     Value:\n");
							printf("     Section Number:\n");
							printf("     Section Value:\n");
						}else {
							fprintf(stderr, "     Value: %016lx \n",sym.st_value);	// print the value of the symbol
							sprintf(sec,"%d",sym.st_shndx);
							fprintf(stderr, "     Section Number: %d \n",atoi(sec));// print the number of the relevant section
							fprintf(stderr, "     Section Value: %016lx \n",section[atoi(sec)]);// print the value of the relevant section
						}
					}else{
						// print [y], y = number of printed symbol
						// print the name of the symbol
						fprintf(stderr,"[%d] Name: %s\n",y,elf_strptr(elf, shdr.sh_link, sym.st_name));
						fprintf(stderr,"    Type: %c\n",p_type);	// print the type of the symbol
						if ( sym.st_value == 0){	// leave blank the following values	
							printf("    Value:\n");
							printf("    Section Number:\n");
							printf("    Section Value:\n");
						}else {
							fprintf(stderr, "    Value: %016lx \n",sym.st_value);	// print the value of the symbol
							sprintf(sec,"%d",sym.st_shndx);
							fprintf(stderr, "    Section Number: %d \n",atoi(sec));	// print the number of the relevant section
							fprintf(stderr, "    Section: %016lx \n",section[atoi(sec)]);	// print the value of the relevant section
						}
					}
				}
			}
			// store the values of the sections in the array
			if (sym.st_info == 3){
				if (cs == 0){
					section [cs] = sym.st_value;
				}
				cs += 1;
				section [cs] = sym.st_value;
			}
	}
}

/*function which prints the symbols from the dynamic symbol table that nm --dyn prints this way:
[i] Name: 
    Type: 
    Value:
*/
void print_dynamic_symbol_table(Elf *elf, Elf_Scn *scn, char *filename) {
    Elf_Data *data;
    GElf_Shdr shdr;
    int count = 0;
	int y=0;	// output counter
	unsigned char bind;
	char p_type;
	char symbol_name[BUFFER_SIZE];


    /* Get the descriptor.  */
    if (gelf_getshdr(scn, &shdr) != &shdr)
        DIE("(getshdr) %s", elf_errmsg(-1));

    data = elf_getdata(scn, NULL);
    count = shdr.sh_size / shdr.sh_entsize;
	

    fprintf(stderr, "Printing dynamic symbol table.\n");
    for (int i = 0; i < count; ++i) {
         GElf_Sym sym;
         gelf_getsym(data, i, &sym);
		 
		 if ( sym.st_name != 0){	// do not print symbols without name
			if (sym.st_info != 4){	// do not print file-type
				y += 1;
				
				//translate the type
				bind = ELF64_ST_BIND(sym.st_info);
				if (bind == STB_WEAK){
					p_type = 'w';
				}else if (sym.st_shndx == SHN_UNDEF){
					if (bind == STB_LOCAL){
						p_type = 'u';
					}else{
						p_type = 'U';
					}
				}else{
					strcpy(symbol_name, elf_strptr(elf, shdr.sh_link, sym.st_name));
					p_type = get_symbol_type(symbol_name,filename);
				}
				if (p_type == 'x'){
					if (bind == STB_LOCAL){
						p_type = 'u';
					}else{
						p_type = 'U';
					}
				}
				// for beautifying the outpout
				if (y>=10){
					
					// print [y], y = number of printed symbol
					// print the name of the symbol
					fprintf(stderr,"[%d] Name: %s\n",y,elf_strptr(elf, shdr.sh_link, sym.st_name));
					fprintf(stderr,"     Type: %c\n",p_type);	// print the type of the symbol
					if ( sym.st_value == 0){	// leave blank the value	
						printf("     Value:\n");
					}else {
						fprintf(stderr, "     Value: %016lx \n",sym.st_value);
					}
				}else{ 
					// print [y], y = number of printed symbol
					// print the name of the symbol
					fprintf(stderr,"[%d] Name: %s\n",y,elf_strptr(elf, shdr.sh_link, sym.st_name));
					fprintf(stderr,"    Type: %c\n",p_type);	// print the type of the symbol
					if ( sym.st_value == 0){
						printf("    Value:\n");
					}else {
						fprintf(stderr, "    Value: %016lx \n",sym.st_value);	// print the value of the symbol
					}
				}
			}
		}
    }
}

void load_file(char *filename) {

    Elf *elf;
    Elf_Scn *symtab;    /* To be used for printing the symbol table.  */
    Elf_Scn *dynsym;    /* To be used for printing the dynamic symbol table. */
	int flag = 0 ;		/* Flag for finding symbol table. */

    /* Initilization.  */
    if (elf_version(EV_CURRENT) == EV_NONE)
        DIE("(version) %s", elf_errmsg(-1));

    int fd = open(filename, O_RDONLY);

    elf = elf_begin(fd, ELF_C_READ, NULL);
    if (!elf)
        DIE("(begin) %s", elf_errmsg(-1));

    /* Loop over sections.  */
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    size_t shstrndx;
    if (elf_getshdrstrndx(elf, &shstrndx) != 0)
        DIE("(getshdrstrndx) %s", elf_errmsg(-1));
		
	int s_index = 0;
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        if (gelf_getshdr(scn, &shdr) != &shdr)
            DIE("(getshdr) %s", elf_errmsg(-1));

        s_index++;
		
        /* Locate symbol table.  */
        if (!strcmp(elf_strptr(elf, shstrndx, shdr.sh_name), ".symtab")){
            symtab = scn;
			flag = 1;
		}
		
        /* Locate dynamic symbol table. */
        if (!strcmp(elf_strptr(elf, shstrndx, shdr.sh_name), ".dynsym"))
            dynsym = scn;
    }
	
	if (flag == 1){	// file in not stripped 
		print_symbol_table(elf, symtab,filename);
		printf("\n");
		printf("\n");
    }else {	// file is stripped
		printf("The binary is stripped, so there is not symbol table. \n");
	}
	print_dynamic_symbol_table(elf,dynsym,filename);
}

int main(int argc, char *argv[]) {

    if (argc < 2)
        DIE("usage: inspector <filename>");

    load_file(argv[1]);

    return 1;
}