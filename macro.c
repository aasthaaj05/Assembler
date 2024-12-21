#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int search_pntab(char str[]);

// Structure to represent the Macro Name Table (MNT), which stores macro names and their properties.
struct macroNameTab
{
	char macroName[11];
	
	// Number of positional parameters in the macro.
	int no_of_pp;
	
	// Index pointer to the corresponding Macro Definition Table (MDT) entry.
	int mdtp;
}mnt[20];

// Structure to represent the Parameter Name Table (PNTab), which stores parameter names for macros.
struct pnTab
{
	char pName[8];
}pntab[30];

// Structure to represent the Macro Definition Table (MDT), which stores each line of macro definition.
struct macroDefTab
{
	char label[8];
	char opcode[11];
	char operand[40];
}mdt[80];

// Structure to represent the Actual Parameter Table (APTAB), which stores values of actual parameters used in macro calls.
struct apTab
{
	char value[8];
}aptab[10];

//global variables

//hold the input file name, a temporary buffer for reading lines from the file, and tokenized strings
char srcfile[11], buffer[200];
char tok[12][40];

//Pointers (indexes) for the macro definition table, macro name table, parameter name table, and argument parameter table
int mdt_ptr, mnt_ptr, pntab_ptr;

int mec, aptab_ptr;
FILE *fp,*fd;
int num,ntok,srch,i,j,k;

//opens the file fname for reading and displays its contents line by line
void show_file(char fname[])
{
	FILE *fp;
	if((fp = fopen(fname, "r"))== NULL)
	{
		printf("\n\tError : Can't open file ..!");
		getchar();
		exit(0);
	}
	while(fgets(buffer, 200, fp))
		printf("%s", buffer);
}

//reads the macro definitions and updates the macro name table (MNT) and parameter name table (PNTAB)
void set_tables()
{
	int pp=0;
	
	//reads a line from the input file (fgets(buffer, 200, fp)),  tokenizes it into parts (ntok = sscanf(...)), and processes 
	fgets(buffer, 200, fp);
	ntok = sscanf(buffer, "%s%s%s%s%s%s",tok[0],tok[1],tok[2],tok[3], tok[4], tok[5]);
	
	//first token is stored as the macro name in the macro name table (mnt)
	strcpy(mnt[mnt_ptr].macroName, tok[0]);
	mnt[mnt_ptr].mdtp = mdt_ptr;
	
	strcat(tok[ntok-1], ",");
	for(num =1 ; num<ntok; num++)
	{
			k=strlen(tok[num])-1;
			tok[num][k] = '\0';
			
			//second token onward are stored as parameters in the parameter name table (pntab)
			strcpy(pntab[pntab_ptr++].pName, tok[num]);
			pp++;
	}
	
	//The number of parameters (pp) is counted and stored in the macro name table entry
	mnt[mnt_ptr].no_of_pp = pp;
}

// identifying and setting up macros in a source file (mac.txt)
void pass_one()
{

	if( (fp = fopen("mac.txt", "r"))==NULL )
	{
		printf("\n\tError : Can't open file ..!");
		getchar();
		exit(0);
	}
	
	// Initialize pointers for Macro Definition Table (MDT) and Macro Name Table (MNT).
	mdt_ptr = mnt_ptr=0;
	
	// Loop through each line in the source file
	while(fgets(buffer, 200, fp))
	{
		// Check if the current line contains the keyword "MACRO", indicating the start of a macro definition.
		if(strstr(buffer, "MACRO"))
		{
			pntab_ptr = 0;
			set_tables();
			
			// Process each line in the macro definition until "MEND" (macro end) is found.
			while(fgets(buffer, 200, fp))
			{
				// Tokenize the line into up to three tokens (opcode and operands).
				ntok = sscanf(buffer, "%s%s%s",tok[1],tok[2],tok[3]);
				
				// Check if the current line contains "MEND", indicating the end of this macro definition.
				if(strstr(buffer, "MEND"))
				{
					strcpy(mdt[mdt_ptr].label, "");
					strcpy(mdt[mdt_ptr].opcode, "MEND");
					strcpy(mdt[mdt_ptr++].operand, "");
					mnt_ptr++;
					break;
				}
				else
				{
					// If the second token (tok[2]) starts with '&', it's a parameter to be processed.
					if(tok[2][0] == '&')
					{
						k=strlen(tok[2])-1;
						tok[2][k] = '\0';

						// Find the parameter's position in pntab and convert it to (P, index) format.
						k = search_pntab(tok[2]);
						sprintf(tok[2], "(P, %d),", k);
					}
					if(tok[3][0] == '&')
					{
						k = search_pntab(tok[3]);
						sprintf(tok[3], "(P, %d)", k);
					}
					
					// Add the processed line (label, opcode, and operand) to the Macro Definition Table (mdt).
					strcpy(mdt[mdt_ptr].label, "");
					strcpy(mdt[mdt_ptr].opcode, tok[1]);
					sprintf(mdt[mdt_ptr++].operand, "%s %s", tok[2], tok[3]);
				}
			}
		}
	}
}

int search_pntab(char str[])
{
	for(i=0; i<pntab_ptr; i++)
		if(!strcasecmp(str, pntab[i].pName))
			return i;
	return -1;
}

void print_mdt()
{
	printf("\n\t---------------------------------------------------------");
	printf("\n\t| No| LABEL | OPCODE |            OPERANDS             |");
	printf("\n\t---------------------------------------------------------");
	for(i=0; i<mdt_ptr; i++)
		printf("\n\n\t|%2d]|%7s|%8s|%35s |", i, mdt[i].label, mdt[i].opcode,
						mdt[i].operand);
}

void print_mnt()
{
	printf("\n\t-------------------------");
	printf("\n\t|   Name  | #PP | MDTP |");
	printf("\n\t-------------------------");
	for(i=0; i<mnt_ptr; i++)
		printf("\n\t| %8s|%5d|%5d|"//%5d|%6d|%7d|%6d |"
		, mnt[i].macroName, mnt[i].no_of_pp, mnt[i].mdtp);

}

int search_mnt(char str[])
{
	for(i=0; i<mnt_ptr; i++)
		if(!strcasecmp(str, mnt[i].macroName))
			return i;
	return -1;
}

// The function assumes the token is of the format "value)", where 'value' is an integer
int cal(char tok[])
{
	char name[20];
	for(j=0; tok[j] != ')';j++)
		name[j] = tok[j];
	name[j] = '\0';
	return(atoi(name));
}

//macro expansions are handled and the output is written to an intermediate file for further processing.
void pass_two()
{
	if((fp = fopen("mac.txt", "r"))== NULL)
	{
		printf("\n\tError: cannot open file..!");
		exit(0);
	}
	
	if((fd = fopen("inter.txt","w"))== NULL)
	{
		printf("\n\tError: cannot open file..!");
		exit(0);
	}
	
	// Read each line from the source file.
	while(fgets(buffer, 200, fp))
	{
		// Skip the macro definition (ignore lines between "MACRO" and "MEND").
		if(strstr(buffer, "MACRO")!=NULL)
		{
			// Continue reading lines until reaching "MEND" to complete the macro definition.
			while(fgets(buffer,200,fp) )
			{
				if((strstr(buffer, "MEND"))!=NULL)
					break;
			}
			continue;
		}
		ntok = sscanf(buffer, "%s%s%s%s%s%s%s",tok[1], tok[2], tok[3],tok[4],tok[5],tok[6],tok[7]);
		
		// Check if the first token is a macro name by searching in the Macro Name Table (MNT).
		srch = search_mnt(tok[1]);
		
		// Not a macro, so copy the line as-is to the intermediate file.
		if(srch== -1)
		{
			fprintf(fd, "%s", buffer);
			continue;
		}
		
		// Add commas to the parameter tokens for consistent parsing.
		strcat(tok[ntok],",");
		tok[2][strlen(tok[2])-1] ='\0';
		tok[3][strlen(tok[3])-1] ='\0';
		tok[4][strlen(tok[4])-1] ='\0';
		
		// Fill the Actual Parameter Table (APT) with actual parameter values
		for(j=0,i=2;j<mnt[srch].no_of_pp;j++,i++)
		{
			strcpy(aptab[j].value, tok[i]);
		}
		mec = mnt[srch].mdtp;
		fprintf(fd, "\n" );
		
		// Loop through the Macro Definition Table (MDT) entries until reaching "MEND".
		while(strcasecmp(mdt[mec].opcode, "MEND"))
		{
			ntok=sscanf(mdt[mec].operand, "%s%s%s%s%s%s%s%s%s%s",tok[0],tok[1], tok[2], tok[3],tok[4], tok[5], tok[6], tok[7], tok[8], tok[9]);
			if(strcmp(tok[0],"(P,")==0)
			{
				 i = cal(tok[1]);
				strcpy(tok[0], aptab[i].value);
				sprintf(buffer, "%s  %s  %s    ",
				mdt[mec].opcode,tok[0],tok[2]);
			}else if(strcmp(tok[1],"(P,")==0)
			{
				i = cal(tok[2]);
				strcpy(tok[1], aptab[i].value);
				sprintf(buffer, "%s  %s  %s    ",mdt[mec].opcode,tok[0],tok[1]);
			}else
				sprintf(buffer, "%s  %s  %s    ", mdt[mec].opcode,tok[0],tok[1]);
			fprintf(fd, "\t  %s\n", buffer);
			mec++;
		}
		fprintf(fd, "\n");

	}
	fclose(fd);
	fclose(fp);
}
void main()
{
	
	pass_one();
	printf("\n\n\n\tMacro Name Table............\n\n");
	print_mnt();
	getchar();

	printf("\n\n\n\tMacro Definition Table............\n\n");
	print_mdt();
	getchar();

	
	pass_two();
	printf("\n\t----------------------------------------------------------");
	printf("\n\tSOURCE CODE.....\n");
	show_file("mac.txt");
	getchar();

	printf("\n\t----------------------------------------------------------");
	printf("\n\tEXPANDED CODE.....\n");
	show_file("inter.txt");
	getchar();
}

