// *** CAO Assignment - Project 3 Simulation *** //


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define BASE_MEM_ADDR 4000;
#define STAGE_IDLE 0;
#define STAGE_BUSY 1;
#define NO_STALL 0;
#define STALL 1;
#define REG_STATUS_VALID 1
#define REG_STATUS_INVALID 0
#define STATUS_VALID 1
#define STATUS_INVALID 0

const char str_movc[]	 = "MOVC";
const char str_add[]     = "ADD";
const char str_sub[] 	 = "SUB";
const char str_mul[] 	 = "MUL";
const char str_div[]	 = "DIV";
const char str_or[]	 = "OR";
const char str_and[]	 = "AND";
const char str_exor[]	 = "EX-OR";
const char str_load[]    = "LOAD";
const char str_store[]   = "STORE";
const char str_bz[] 	 = "BZ";
const char str_bnz[] 	 = "BNZ";
const char str_jump[]	 = "JUMP";
const char str_halt[] 	 = "HALT";
const char str_nop[]	 = "NOP";
const char str_jal[]   = "JAL";
int temp_jal_value, dep_branch_PC_addrs;

const bool log_yes	 = true;
bool true_dependency = !true;
bool decode_is_bz_or_bnz;
bool intfu_data_fwd, mul_data_fwd, div_data_fwd;
bool loaded_src1_val =false, loaded_src2_val=false;
int temp_src1val, temp_src2val;
char filename[20];
int instr_no, temp_jal_value;
int rob_head, rob_tail;

int tot_cycle;
char tempstr[20], *str;


// *Pipeline Stage - Structure Definition* //

struct instruction_info {
	int  PC;
	char instr_string[5];
	int src_reg1;
	int src_reg2;
	int src_reg1val;
	bool loaded_reg1val;
	int src_reg2val;
	bool loaded_reg2val;
	int dest_reg;
	int dest_regval;
	int tgt_mem_addr;
	int tgt_mem_data;	
	int litrl;
  	char instr[20];
	int phy_reg;
	int cycle_cnt;
	int lsq_cycle_cnt;
	bool rob_commit;

};

struct stage_info {
	struct instruction_info *input_instr;
	struct instruction_info *output_instr;
	bool stalled;
	bool iq_empty;
	bool lsq_empty;
	bool rob_empty;
};


struct issue_queue {
	struct stage_info *IQ[16];	
};

struct load_store_queue {
	struct stage_info *LSQ[32];	
};

struct re_order_buff {
	struct stage_info *ROB[32];	
};



// *Code Memory - Structure Definition* //

struct code_line {
	int  line_no;
	int  address;
	char instr_string[25];
};

struct code_mem {
	struct code_line inpstr[25];
};



// *Data Memory - Structure Definition* //

struct data_memory {
	int base_addr;
	int data_val [4000];
};


// *Register - Structure Definition* //

struct Register_ {
	int regno;
	int regval;
	bool status;	//0-Invalid & 1=Valid //
};


struct register_file {
	struct Register_ Regstr[16];
};


// *Physical Register Files* //

struct physical_register_struct {

	int allocated;
	int rename;
	int status;
	int value;

};

struct physical_register {

	struct physical_register_struct Regstr[32];
};


// *Flags - Structure Definition* //

struct Flags {
	bool zero;
	int zero_status;
	bool carry;
	bool carry_status;
	bool neg;
	bool neg_status;
};


// *Collecting Stats* //

struct stats {
	int cycle;
	int instr_no;
};


// *pipeline stage variables* //

struct pipeline {

	bool fetch; 
	bool decode; 
	bool exe1; 
	bool exe2; 
	bool memory; 
	bool wrtbck;

}stage;


struct rename_table {
	int prf[16];
	bool sts[16];
}renam_tbl;



void init_stage_fetch_input(struct stage_info *stage_info);
void init_stage_fetch_output(struct stage_info *stage_info);

void init_stage_decode_input(struct stage_info *stage_info);
void init_stage_decode_output(struct stage_info *stage_info);

void init_stage_non_mul_exe_input(struct stage_info *stage_info);
void init_stage_non_mul_exe_output(struct stage_info *stage_info);

void init_stage_mul_exe1_input(struct stage_info *stage_info);
void init_stage_mul_exe1_output(struct stage_info *stage_info);

void init_stage_mul_exe2_input(struct stage_info *stage_info);
void init_stage_mul_exe2_output(struct stage_info *stage_info);

void init_stage_div_exe1_input(struct stage_info *stage_info);
void init_stage_div_exe1_output(struct stage_info *stage_info);

void init_stage_div_exe2_input(struct stage_info *stage_info);
void init_stage_div_exe2_output(struct stage_info *stage_info);

void init_stage_div_exe3_input(struct stage_info *stage_info);
void init_stage_div_exe3_output(struct stage_info *stage_info);

void init_stage_div_exe4_input(struct stage_info *stage_info);
void init_stage_div_exe4_output(struct stage_info *stage_info);

void init_stage_memory_input(struct stage_info *stage_info);
void init_stage_memory_output(struct stage_info *stage_info);

void init_stage_wrtbck_input(struct stage_info *stage_info);
void init_stage_wrtbck_output(struct stage_info *stage_info);

void init_data_memory(struct data_memory *data);	
void init_registers(struct register_file *register_file);
void init_phy_registers(struct physical_register *phy_reg);
void init_stat(struct stats *stats);	
void init_flags(struct Flags *Flags);

void init_issue_queue_input(struct issue_queue *issue_queue, int a);
//void init_issue_queue_output(struct issue_queue *issue_queue);


void init_load_store_queue_input(struct load_store_queue *load_store_queue, int a);
//void init_load_store_queue_output(struct load_store_queue *load_store_queue);

void init_re_order_buff_input(struct re_order_buff *re_order_buff, int a);
//void init_re_order_buff_output(struct re_order_buff *re_order_buff);


void getinputfile(struct code_mem *inpt); 
void displayinputfile(struct code_mem *inpt);
void init_rename_table(struct rename_table renam_tbl);



void fetch_instruction(struct stats *stats, struct code_mem *code_mem, struct stage_info *stage_info);
void decode_instruction(struct stage_info *stage_info, struct stage_info *stage_info1, struct register_file *register_file, struct Flags *Flags, struct physical_register *physical_register, struct rename_table rename_table);

void non_mul_exe_instruction(struct stage_info *stage_info, struct re_order_buff *re_order_buff, struct physical_register *physical_register);
void mul_exe1_instruction(struct stage_info *stage_info, struct stage_info *stage_info1);
void mul_exe2_instruction(struct stage_info *stage_info, struct stage_info *stage_info1, struct re_order_buff *re_order_buff, struct physical_register *physical_register);
void div_exe1_instruction(struct stage_info *stage_info, struct stage_info *stage_info1);
void div_exe2_instruction(struct stage_info *stage_info, struct stage_info *stage_info1);
void div_exe3_instruction(struct stage_info *stage_info, struct stage_info *stage_info1);
void div_exe4_instruction(struct stage_info *stage_info, struct stage_info *stage_info1, struct re_order_buff *re_order_buff, struct physical_register *physical_register);

void memory_instruction(struct stage_info *stage_info, struct data_memory *data_memory, struct register_file *register_file, struct re_order_buff *re_order_buff);
void wrtbck_instruction(struct stage_info *stage_info, struct stage_info *stage_info1, struct stage_info *stage_info2, struct register_file *register_file, struct Flags *Flags);

void process_load_store_queue(struct load_store_queue *load_store_queue, struct stage_info *stage_info);
void process_rob(struct re_order_buff *re_order_buff, struct register_file *register_file, struct physical_register *physical_register, struct rename_table renam_table);
void issue_queue(struct issue_queue *issue_queue, struct stage_info *stage_info, struct stage_info *stage_info1, struct stage_info *stage_info2);
void mark_rob_commit(struct re_order_buff *re_order_buff, int pc);
void forward_data_DRF(struct stage_info *stage_info, struct stage_info *stage_info1, struct stage_info *stage_info2, struct stage_info *stage_info3, struct stage_info *stage_info4);
void forward_data_IQ(struct stage_info *stage_info, struct stage_info *stage_info1, struct stage_info *stage_info2, struct stage_info *stage_info3, struct stage_info *stage_info4, struct issue_queue *issue_queue);


void clear_dependencies(struct stage_info *stage_info, struct register_file *register_file);
void flush_pipeline(struct stage_info *stage_info, struct stage_info *stage_info1, struct register_file *register_file);

void non_mul_exe_data_forward(struct stage_info *stage_info, struct stage_info *stage_info1, struct register_file *register_file, struct physical_register *physical_register);
void mul_exe_data_forward(struct stage_info *stage_info, struct stage_info *stage_info1, struct register_file *register_file);
void div_exe_data_forward(struct stage_info *stage_info, struct stage_info *stage_info1, struct register_file *register_file);
//void forward_data(struct stage_info *stage_info, struct stage_info *stage_info1, struct stage_info *stage_info2, struct stage_info *stage_info3, struct register_file *register_file);
void push_data(struct stage_info *stage_info, struct issue_queue *issue_queue, struct load_store_queue *load_store_queue, struct re_order_buff *re_order_buff);
void resolve_dependency(struct stage_info *stage_info, struct stage_info *stage_info1, struct register_file *register_file);

void print_data_values(struct data_memory *data);
void print_register_values(struct register_file *register_file);

void clear_function_units(struct stage_info *stage_info, struct stage_info *stage_info1, struct stage_info *stage_info2);


	// *** ========= Main Function =========== *** //

int main()

{


struct stage_info *fetch, *decode, *non_mul_exe, *mul_exe1, *mul_exe2, *div_exe1, *div_exe2, *div_exe3, *div_exe4, *memory, *wrtbck;
struct issue_queue *I_Q;
struct load_store_queue *L_S_Q;
struct re_order_buff *R_O_B;
struct stats *num;
struct register_file *Reg;
struct physical_register *phy_reg;
struct data_memory *data;
struct Flags *flag;
struct code_mem *inpt;

fetch  = (struct stage_info*) malloc(sizeof(struct stage_info));
decode = (struct stage_info*) malloc(sizeof(struct stage_info));
non_mul_exe = (struct stage_info*) malloc(sizeof(struct stage_info));
mul_exe1   = (struct stage_info*) malloc(sizeof(struct stage_info)); 
mul_exe2   = (struct stage_info*) malloc(sizeof(struct stage_info));
div_exe1   = (struct stage_info*) malloc(sizeof(struct stage_info));
div_exe2   = (struct stage_info*) malloc(sizeof(struct stage_info));
div_exe3   = (struct stage_info*) malloc(sizeof(struct stage_info));
div_exe4   = (struct stage_info*) malloc(sizeof(struct stage_info));
memory = (struct stage_info*) malloc(sizeof(struct stage_info));
wrtbck = (struct stage_info*) malloc(sizeof(struct stage_info));

I_Q	= (struct issue_queue*)	malloc(sizeof(struct issue_queue));
L_S_Q	= (struct load_store_queue*)	malloc(sizeof(struct load_store_queue));
R_O_B	= (struct re_order_buff*)	malloc(sizeof(struct re_order_buff));

num    	= (struct stats*) 		malloc(sizeof(struct stats));
Reg	= (struct register_file*) 	malloc(sizeof(struct register_file));
phy_reg	= (struct physical_register*)	malloc(sizeof(struct physical_register));
data	= (struct data_memory*) 	malloc(sizeof(struct data_memory));	
flag	= (struct Flags*)		  malloc(sizeof(struct Flags));
inpt  = (struct code_mem*)  malloc(sizeof(struct code_mem));

fetch->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
fetch->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));

decode->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
decode->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));

non_mul_exe->input_instr = (struct instruction_info*) malloc (sizeof(struct instruction_info));
non_mul_exe->output_instr = (struct instruction_info*) malloc (sizeof(struct instruction_info));

mul_exe1->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
mul_exe1->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));

mul_exe2->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
mul_exe2->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));

div_exe1->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
div_exe1->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));

div_exe2->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
div_exe2->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));

div_exe3->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
div_exe3->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));

div_exe4->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
div_exe4->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));

memory->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
memory->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));

wrtbck->input_instr  = (struct instruction_info*) malloc(sizeof(struct instruction_info));
wrtbck->output_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));


for(int n=0; n<16; n++)
	{
	I_Q->IQ[n] = (struct stage_info*) malloc(sizeof(struct stage_info));
	I_Q->IQ[n]->input_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));
	}

for(int n=0; n<32; n++)
	{
	L_S_Q->LSQ[n] = (struct stage_info*) malloc(sizeof(struct stage_info));
	L_S_Q->LSQ[n]->input_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));
	}

for(int n=0; n<32; n++)
	{
	R_O_B->ROB[n] = (struct stage_info*) malloc(sizeof(struct stage_info));
	R_O_B->ROB[n]->input_instr = (struct instruction_info*) malloc(sizeof(struct instruction_info));
	}

int input, a;
int simulation=1; 
rob_head = 0;
rob_tail = 0;

getinputfile(inpt);			// Load the input file to program memory 



do
	{

	
	printf("1. Display Input File\n");
	printf("2. Initialize \n");
	printf("3. Simulate \n");
	printf("4. Display Values\n");
  	printf("5. Exit\n");


	printf("Option:");
	scanf("%d",&input);


	switch(input)
		{

    		case 1:
    		displayinputfile(inpt);
    		break;

	
		case 2:
		// Initialize the stage info for all stages 

		init_stage_fetch_input(fetch);	
		init_stage_fetch_output(fetch);	
		
		init_stage_decode_input(decode);
		init_stage_decode_output(decode);
		
		init_stage_non_mul_exe_input(non_mul_exe);
		init_stage_non_mul_exe_output(non_mul_exe);		
		
		init_stage_mul_exe1_input(mul_exe1);
		init_stage_mul_exe1_output(mul_exe1);

		init_stage_mul_exe2_input(mul_exe2);
		init_stage_mul_exe2_output(mul_exe2);

		init_stage_div_exe1_input(div_exe1);
		init_stage_div_exe1_output(div_exe1);

		init_stage_div_exe2_input(div_exe2);
		init_stage_div_exe2_output(div_exe2);

		init_stage_div_exe3_input(div_exe3);
		init_stage_div_exe3_output(div_exe3);

		init_stage_div_exe4_input(div_exe4);
		init_stage_div_exe4_output(div_exe4);

		init_stage_memory_input(memory);
		init_stage_memory_output(memory);

    		for(a=0; a<16; a++)
    		{
		init_issue_queue_input(I_Q, a);}
    
    		for(a=0; a<32; a++)
    		{
		init_load_store_queue_input(L_S_Q, a);}

		for(a=0; a<32; a++)
		{
		init_re_order_buff_input(R_O_B, a);} 


		init_data_memory(data);		// Initialize the data memory 
		init_registers(Reg);		// Initialize the register file
 	  	init_phy_registers(phy_reg);	// Initialize physical registers	
		init_flags(flag);		// Initialize the flag values 
		init_stat(num);			// Initialize the stat - number of instruction completed 
		init_rename_table(renam_tbl);

		printf("All pipeline stages are initialized.\n");		
		break;

			
		case 3:
   
		printf("Enter the number of cycles to be simulated:\n");				
		scanf("%d", &simulation);
		tot_cycle = 1;
		if(log_yes) printf("=========================================\n");	

		while(tot_cycle <= simulation)
			{

			if(log_yes)	printf("cycle: %d\n", tot_cycle);							
			
			    clear_function_units(non_mul_exe, mul_exe2, div_exe4);

			process_rob(R_O_B, Reg, phy_reg, renam_tbl);

			process_load_store_queue(L_S_Q, memory);
			
			memory_instruction(memory, data, Reg, R_O_B);

			div_exe4_instruction(div_exe3, div_exe4, R_O_B, phy_reg);
			div_exe3_instruction(div_exe2, div_exe3);
			div_exe2_instruction(div_exe1, div_exe2);
			div_exe1_instruction(decode, div_exe1);

			mul_exe2_instruction(mul_exe1, mul_exe2, R_O_B, phy_reg);
			mul_exe1_instruction(decode, mul_exe1);
      			
			non_mul_exe_instruction(non_mul_exe, R_O_B, phy_reg);


				
			push_data(decode, I_Q, L_S_Q, R_O_B);		
			
			decode_instruction(fetch, decode, Reg, flag, phy_reg, renam_tbl);						


      forward_data_DRF(decode, non_mul_exe, mul_exe2, div_exe4, memory);
      forward_data_IQ(decode, non_mul_exe, mul_exe2, div_exe4, memory, I_Q);
      //printf("changed. \n");
									
			issue_queue(I_Q, non_mul_exe, mul_exe1, div_exe1);

							
			fetch_instruction(num, inpt, fetch);
			
			if(log_yes) printf("=========================================\n");			

			if(strcmp(wrtbck->output_instr->instr_string, str_halt) == 0)
				{
				break;
				}
			
			tot_cycle++;
			};
				
			printf("simulation entered is: %d\n", simulation);
		break;	

		
		case 4:			
		print_register_values(Reg);	// Print current register values 
		print_data_values(data);	// Print data values
		break;

		case 5:
   		break;

		default:
		printf("Invalid Value\n");
			
			
		}

	}while(input != 5);

return(0);

}



	// --------------------------------------------------- //





		// * * * Get the FETCH instruction * * * //

void fetch_instruction(struct stats *num, struct code_mem *inpt, struct stage_info *fetch)
{


if(strcmp(fetch->output_instr->instr_string, str_nop) == 0)
	{
	printf("FETCH     :\t\t%s\n", fetch->output_instr->instr_string);
	return;
	}

if (fetch->stalled == true || fetch->output_instr->PC != 0)		//Proceed FETCH stage if no stall & Get the next instruction from program memory
	{
	printf("FETCH     :%d\t%s\t(Stalled)\n", fetch->output_instr->PC, fetch->output_instr->instr);
	return;
	}

if(strcmp(fetch->output_instr->instr_string, str_nop) != 0)
	{

	strcpy(tempstr, inpt->inpstr[num->instr_no].instr_string);
	str = strtok(tempstr, ", ");

	if(str == NULL)
	  return;

	init_stage_fetch_input(fetch);		// Clear FETCH structure
	init_stage_fetch_output(fetch);		
		
	fetch->input_instr->PC  = inpt->inpstr[num->instr_no].address;				// Retrieve instruction from input string 

	strcpy(fetch->input_instr->instr_string, str);
	strcpy(fetch->input_instr->instr, inpt->inpstr[num->instr_no].instr_string);

	str = NULL;
	fetch->output_instr->PC = fetch->input_instr->PC;
	strcpy(fetch->output_instr->instr_string, fetch->input_instr->instr_string);
	strcpy(fetch->output_instr->instr, fetch->input_instr->instr);

	//fetch->stalled = true;
	
	printf("FETCH     :%d\t%s\n", fetch->output_instr->PC, fetch->output_instr->instr);
	
	init_stage_fetch_input(fetch);

	num->instr_no = num->instr_no + 1;
	
	}

}

				// * * * DECODE Stage * * * //

void decode_instruction(struct stage_info *fetch, struct stage_info *decode, struct register_file *Reg, struct Flags *flag, struct physical_register *phy_reg, struct rename_table renam_tbl)
{

	//printf("culprit. \n");

//char *str;


if(strcmp(decode->output_instr->instr_string, str_nop)== 0)
	{
	printf("DECODE    :\t\t%s\n", decode->output_instr->instr_string);
	return;
	}

if(decode->output_instr->PC != 0 )
	printf("DEOCDE    :%d\t%s\t(Stalled)\n", decode->output_instr->PC, decode->output_instr->instr);


if(decode->input_instr->PC == 0)
{
	if (fetch->output_instr->PC == 0 || decode->output_instr->PC != 0)
 	{
	return;
	}

	
init_stage_decode_input(decode);	//Clear DECODE stage 
init_stage_decode_output(decode);	

decode->input_instr->PC = fetch->output_instr->PC;			// Move FETCH instruction output to DECODE instruction input
strcpy(decode->input_instr->instr_string, fetch->output_instr->instr_string);
strcpy(decode->input_instr->instr, fetch->output_instr->instr);

init_stage_fetch_output(fetch);


str = strtok(NULL,", ");		//Load the second value from the input string


if (str != NULL)
	{
	str = str+1;

	if (strcmp(decode->input_instr->instr_string, str_movc) == 0 || strcmp(decode->input_instr->instr_string, str_add) == 0 ||
		strcmp(decode->input_instr->instr_string, str_sub) == 0  || strcmp(decode->input_instr->instr_string, str_mul)  == 0 ||
		strcmp(decode->input_instr->instr_string, str_or) == 0   || strcmp(decode->input_instr->instr_string, str_and)  == 0 ||
		strcmp(decode->input_instr->instr_string, str_exor) == 0 || strcmp(decode->input_instr->instr_string, str_load) == 0 ||
		strcmp(decode->input_instr->instr_string, str_div) == 0  || strcmp(decode->input_instr->instr_string,str_jal) == 0 )
		{
		decode->input_instr->dest_reg = atoi(str);   
		}

	if (strcmp(decode->input_instr->instr_string, str_store) == 0 || strcmp(decode->input_instr->instr_string, str_jump) == 0)
		{
		decode->input_instr->src_reg1 = atoi(str);
		}

	if (strcmp(decode->input_instr->instr_string, str_bz) == 0 || strcmp(decode->input_instr->instr_string,str_bnz) == 0)
		{
		decode->input_instr->litrl = atoi(str);
		}

	}



str = strtok(NULL,", ");		//Load the third value from the input string

if (str != NULL)
	{
	str = str+1;

	if (strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || 
		strcmp(decode->input_instr->instr_string, str_mul) == 0 || strcmp(decode->input_instr->instr_string, str_or) == 0 ||
		strcmp(decode->input_instr->instr_string, str_and) == 0 || strcmp(decode->input_instr->instr_string, str_exor) == 0 ||
		strcmp(decode->input_instr->instr_string,str_load) == 0 || strcmp(decode->input_instr->instr_string, str_div) == 0 || 
		strcmp(decode->input_instr->instr_string,str_jal) == 0)
		{
		decode->input_instr->src_reg1 = atoi(str);
		}

	if (strcmp(decode->input_instr->instr_string, str_store) == 0)
		{
		decode->input_instr->src_reg2 = atoi(str);
		}

	if (strcmp(decode->input_instr->instr_string, str_movc) == 0 || strcmp(decode->input_instr->instr_string, str_jump) == 0 || strcmp(decode->input_instr->instr_string,str_jal) == 0) 		
		{
		decode->input_instr->litrl = atoi(str);
		}  


	
}

str = strtok(NULL,", ");		//load final value from the input string

if (str != NULL)
	{
	str = str+1;


	if (strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || 
			strcmp(decode->input_instr->instr_string, str_mul) == 0 || strcmp(decode->input_instr->instr_string, str_or) == 0 ||
			strcmp(decode->input_instr->instr_string, str_and) == 0 || strcmp(decode->input_instr->instr_string, str_exor) == 0 ||
			strcmp(decode->input_instr->instr_string, str_div) == 0)
		{
		decode->input_instr->src_reg2 = atoi(str);
		}

	if (strcmp(decode->input_instr->instr_string, str_store) == 0 || strcmp(decode->input_instr->instr_string, str_load) == 0 || strcmp(decode->input_instr->instr_string, str_jal) == 0)
		{
		decode->input_instr->litrl = atoi(str);
		}

	}

	// For Branch Instruction, Store the previous instruction PC address
if (strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || 
	strcmp(decode->input_instr->instr_string, str_mul) == 0 || strcmp(decode->input_instr->instr_string, str_div) == 0 )
	{
	dep_branch_PC_addrs = decode->input_instr->PC;
	}

if(strcmp(decode->input_instr->instr_string, str_bz) == 0 || strcmp(decode->input_instr->instr_string, str_bnz) == 0 ) 
	{
	decode_is_bz_or_bnz = true;
	//decode_addrs_bz_or_bnz = decode->input_instr->PC;
	flag->zero_status = STATUS_INVALID;
	}
else
	{
	decode_is_bz_or_bnz = false;
	//decode_addrs_bz_or_bnz = 0;
	flag->zero_status = STATUS_VALID;
	}

}


// Assign a physical register for Architecture register and make the Architecture register invalid.

if(strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || 
		strcmp(decode->input_instr->instr_string, str_mul) == 0  || strcmp(decode->input_instr->instr_string, str_or) == 0    ||
		strcmp(decode->input_instr->instr_string, str_and) == 0  || strcmp(decode->input_instr->instr_string, str_exor) == 0  ||
		strcmp(decode->input_instr->instr_string, str_movc) == 0 || strcmp(decode->input_instr->instr_string, str_load) == 0  ||
		strcmp(decode->input_instr->instr_string, str_div) == 0  || strcmp(decode->input_instr->instr_string, str_jal) == 0)		
		{
		
		Reg->Regstr[decode->input_instr->dest_reg].status = REG_STATUS_INVALID;			

		int k = 0;

		while(k<32)
			{
			if(phy_reg->Regstr[k].allocated  == false)
				break;
			k++;
			}
		
		phy_reg->Regstr[k].allocated = true;
		phy_reg->Regstr[k].status = false; 
		phy_reg->Regstr[k].rename = false;
		phy_reg->Regstr[k].value = 0;	
		
		//renam_tbl.arf[] = decode->input_instr->dest_reg;
		renam_tbl.prf[decode->input_instr->dest_reg] = k;
		renam_tbl.sts[decode->input_instr->dest_reg] = true;

		decode->input_instr->phy_reg = k;
		}


// Load register value from ARF and PRF and make the flag ON for the input registers

if(strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || 
		strcmp(decode->input_instr->instr_string, str_mul) == 0  || strcmp(decode->input_instr->instr_string, str_or) == 0    ||
		strcmp(decode->input_instr->instr_string, str_and) == 0  || strcmp(decode->input_instr->instr_string, str_exor) == 0  ||
		strcmp(decode->input_instr->instr_string, str_load) == 0 || strcmp(decode->input_instr->instr_string, str_store) == 0 ||
		strcmp(decode->input_instr->instr_string, str_jump) == 0 || strcmp(decode->input_instr->instr_string, str_div) == 0   || 
    		strcmp(decode->input_instr->instr_string, str_jal) == 0)
		{
		
		if(Reg->Regstr[decode->input_instr->src_reg1].status == REG_STATUS_VALID)
			{
			decode->input_instr->src_reg1val = Reg->Regstr[decode->input_instr->src_reg1].regval;		//load source register1 value from ARF
			decode->input_instr->loaded_reg1val = true;
			}
		else if(renam_tbl.sts[decode->input_instr->src_reg1] && phy_reg->Regstr[decode->input_instr->src_reg1].status)
			{				
			decode->input_instr->src_reg1val = phy_reg->Regstr[decode->input_instr->src_reg1].value;	//load source register1 value from PRF
			decode->input_instr->loaded_reg1val = true;			
			}
					 
		
		if(strcmp(decode->input_instr->instr_string, str_load) != 0 && strcmp(decode->input_instr->instr_string, str_jump) != 0 && strcmp(decode->input_instr->instr_string, str_jal) != 0) 
			{

			if(Reg->Regstr[decode->input_instr->src_reg2].status == REG_STATUS_VALID)
				{
				decode->input_instr->src_reg2val = Reg->Regstr[decode->input_instr->src_reg2].regval;		//load source register2 value from ARF
				decode->input_instr->loaded_reg2val = true;
				}
			else if(renam_tbl.sts[decode->input_instr->src_reg2] && phy_reg->Regstr[decode->input_instr->src_reg2].status)
				{				
				decode->input_instr->src_reg2val = phy_reg->Regstr[decode->input_instr->src_reg2].value;	//load source register2 value from PRF
				decode->input_instr->loaded_reg2val = true;			
				}

			}
						
		}



/*

//if(decode->input_instr->PC != 0 && decode->stalled != true)
//{
//******* CHECK *TRUE DEPENDENCY* FOR THE INPUT REGISTER 

	// IF INPUT REGISTER IS *INVALD*, THEN DO NOT PROCEED


if(strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || 
		strcmp(decode->input_instr->instr_string, str_mul) == 0  || strcmp(decode->input_instr->instr_string, str_or) == 0    ||
		strcmp(decode->input_instr->instr_string, str_and) == 0  || strcmp(decode->input_instr->instr_string, str_exor) == 0  ||
		strcmp(decode->input_instr->instr_string, str_div) == 0)		
		{

		if(Reg->Regstr[decode->input_instr->src_reg1].status == REG_STATUS_INVALID || Reg->Regstr[decode->input_instr->src_reg2].status == REG_STATUS_INVALID || Reg->Regstr[decode->input_instr->dest_reg].status == REG_STATUS_INVALID)
			{
			printf("DECODE    :%d\t%s\t(Stalled)\n", decode->input_instr->PC, decode->input_instr->instr);
    			return;
			}			
		}

	//For STORE Instruction
if(strcmp(decode->input_instr->instr_string, str_store) == 0)
	{
	if(Reg->Regstr[decode->input_instr->src_reg1].status == REG_STATUS_INVALID || Reg->Regstr[decode->input_instr->src_reg2].status == REG_STATUS_INVALID)
		{
		printf("DECODE    :%d\t%s\t(Stalled)\n", decode->input_instr->PC, decode->input_instr->instr);
		return;
		}
	}

	//For MOVC Instruction
if(strcmp(decode->input_instr->instr_string, str_movc) == 0)
	{
	if(Reg->Regstr[decode->input_instr->dest_reg].status == REG_STATUS_INVALID)
		{
		printf("DECODE    :%d\t%s\t(Stalled)\n", decode->input_instr->PC, decode->input_instr->instr);
		return;
		}
	}

	//For LOAD Instruction
if(strcmp(decode->input_instr->instr_string, str_load) == 0)
	{
	if(Reg->Regstr[decode->input_instr->src_reg1].status == REG_STATUS_INVALID || Reg->Regstr[decode->input_instr->dest_reg].status == REG_STATUS_INVALID)
		{
		printf("DECODE    :%d\t%s\t(Stalled)\n", decode->input_instr->PC, decode->input_instr->instr);
		return;
		}
	}

	//For JUMP Instruction
if(strcmp(decode->input_instr->instr_string, str_jump) == 0)
	{
	if(Reg->Regstr[decode->input_instr->src_reg1].status == REG_STATUS_INVALID)
		{
		printf("DECODE    :%d\t%s\t(Stalled)\n", decode->input_instr->PC, decode->input_instr->instr);
		return;
		}	
	}

	//For JAL Instruction
if(strcmp(decode->input_instr->instr_string, str_jal) == 0)	
	{
	if(Reg->Regstr[decode->input_instr->src_reg1].status == REG_STATUS_INVALID || Reg->Regstr[decode->input_instr->dest_reg].status == REG_STATUS_INVALID)
		{
		printf("DECODE    :%d\t%s\t(Stalled)\n", decode->input_instr->PC, decode->input_instr->instr);
		return;
		}
	}


	// For branch Instruction, check the ZERO FLAG Status

if((strcmp(decode->input_instr->instr_string, str_bz) == 0 || strcmp(decode->input_instr->instr_string, str_bnz) == 0 ) && flag->zero_status == STATUS_INVALID)
	{
	printf("DECODE    :%d\t%s\t(Stalled)\n", decode->input_instr->PC, decode->input_instr->instr);
	return;
	}



//******* NO DEPENDENCY ON OUTPUT REGISTER, MAKE THE OUTPUT REGISTER *INVALID* AND PROCEED ***** //


if(strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || 
		strcmp(decode->input_instr->instr_string, str_mul) == 0  || strcmp(decode->input_instr->instr_string, str_or) == 0    ||
		strcmp(decode->input_instr->instr_string, str_and) == 0  || strcmp(decode->input_instr->instr_string, str_exor) == 0  ||
		strcmp(decode->input_instr->instr_string, str_movc) == 0 || strcmp(decode->input_instr->instr_string, str_load) == 0  ||
		strcmp(decode->input_instr->instr_string, str_div) == 0  || strcmp(decode->input_instr->instr_string, str_jal) == 0)		
		{
		Reg->Regstr[decode->input_instr->dest_reg].status = REG_STATUS_INVALID;		
		}
  
// Load register contents

if(strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || 
		strcmp(decode->input_instr->instr_string, str_mul) == 0  || strcmp(decode->input_instr->instr_string, str_or) == 0    ||
		strcmp(decode->input_instr->instr_string, str_and) == 0  || strcmp(decode->input_instr->instr_string, str_exor) == 0  ||
		strcmp(decode->input_instr->instr_string, str_load) == 0 || strcmp(decode->input_instr->instr_string, str_store) == 0 ||
		strcmp(decode->input_instr->instr_string, str_jump) == 0 || strcmp(decode->input_instr->instr_string, str_div) == 0   || 
    		strcmp(decode->input_instr->instr_string, str_jal) == 0)
		{
		decode->input_instr->src_reg1val = Reg->Regstr[decode->input_instr->src_reg1].regval;			//load source register1 value 
		
			if(strcmp(decode->input_instr->instr_string, str_load) != 0 && strcmp(decode->input_instr->instr_string, str_jump) != 0 && strcmp(decode->input_instr->instr_string, str_jal) != 0) 
			{
			decode->input_instr->src_reg2val = Reg->Regstr[decode->input_instr->src_reg2].regval;		//load source register2 value
			}
		}


*/

decode->output_instr->PC = decode->input_instr->PC;
strcpy(decode->output_instr->instr_string, decode->input_instr->instr_string);
decode->output_instr->src_reg1 = decode->input_instr->src_reg1;
decode->output_instr->src_reg2 = decode->input_instr->src_reg2;
decode->output_instr->src_reg1val = decode->input_instr->src_reg1val;
decode->output_instr->src_reg2val = decode->input_instr->src_reg2val;
decode->output_instr->loaded_reg1val = decode->input_instr->loaded_reg1val;
decode->output_instr->loaded_reg2val = decode->input_instr->loaded_reg2val;
decode->output_instr->dest_reg = decode->input_instr->dest_reg;
decode->output_instr->tgt_mem_addr = decode->input_instr->tgt_mem_addr;
decode->output_instr->tgt_mem_data = decode->input_instr->tgt_mem_data;
decode->output_instr->litrl = decode->input_instr->litrl;
decode->output_instr->cycle_cnt = 0;
decode->output_instr->phy_reg = decode->input_instr->phy_reg;
strcpy(decode->output_instr->instr, decode->input_instr->instr);


//decode->stalled = true;

printf("DECODE    :%d\t%s\n", decode->output_instr->PC, decode->output_instr->instr);

init_stage_decode_input(decode);	

//}

}


		// * * * Push Data into Issue Queue, Load Store Queue, Re Order Buffer * * * //

void push_data(struct stage_info *decode, struct issue_queue *I_Q, struct load_store_queue *L_S_Q, struct re_order_buff *R_O_B)
{

int i;

	//Identify the empty slot in the ISSUE QUEUE and insert the instruction

	i = 0;

	while(i<16)
		{
		if(I_Q->IQ[i]->iq_empty)
			break;		
		i++;
		}

	
	if(decode->output_instr->PC != 0)
		{
		// Insert the entry into IQ in the free slot
		
		I_Q->IQ[i]->input_instr->PC = decode->output_instr->PC;
		strcpy(I_Q->IQ[i]->input_instr->instr_string, decode->output_instr->instr_string);
		I_Q->IQ[i]->input_instr->src_reg1 	= decode->output_instr->src_reg1;
		I_Q->IQ[i]->input_instr->src_reg1val	= decode->output_instr->src_reg1val;
		I_Q->IQ[i]->input_instr->src_reg2 	= decode->output_instr->src_reg2;
		I_Q->IQ[i]->input_instr->src_reg2val	= decode->output_instr->src_reg2val;
		I_Q->IQ[i]->input_instr->dest_reg 	= decode->output_instr->dest_reg;
		I_Q->IQ[i]->input_instr->loaded_reg1val	= decode->output_instr->loaded_reg1val;
		I_Q->IQ[i]->input_instr->loaded_reg2val	= decode->output_instr->loaded_reg2val;
		I_Q->IQ[i]->input_instr->dest_regval	= decode->input_instr->dest_regval;
		I_Q->IQ[i]->input_instr->tgt_mem_addr	= decode->output_instr->tgt_mem_addr;
		I_Q->IQ[i]->input_instr->tgt_mem_data	= decode->output_instr->tgt_mem_data;
		I_Q->IQ[i]->input_instr->litrl		= decode->output_instr->litrl;
		I_Q->IQ[i]->input_instr->cycle_cnt	= 0;
	       	I_Q->IQ[i]->input_instr->phy_reg	= decode->output_instr->phy_reg; 	
		I_Q->IQ[i]->iq_empty = false;
		strcpy(I_Q->IQ[i]->input_instr->instr, decode->output_instr->instr);

		printf("IQ        :%d\t%s\n", I_Q->IQ[i]->input_instr->PC, I_Q->IQ[i]->input_instr->instr);

		}


	//Identify the empty slot in LOAD STORE QUEUE and insert the instruction for LOAD & STORE instruction

	if( strcmp(decode->output_instr->instr_string, str_load) == 0 || strcmp(decode->output_instr->instr_string, str_store) == 0 )	
		{	
	
		i = 0;

		while(i<32)
			{
			if(L_S_Q->LSQ[i]->lsq_empty)
				break;		
			i++;
			}
	
	
		if(decode->output_instr->PC != 0)
			{
			//LSQ By-Pass implementation
     
			L_S_Q->LSQ[i]->input_instr->PC = decode->output_instr->PC;
			strcpy(L_S_Q->LSQ[i]->input_instr->instr_string, decode->output_instr->instr_string);
			L_S_Q->LSQ[i]->input_instr->src_reg1 		= decode->output_instr->src_reg1;
			L_S_Q->LSQ[i]->input_instr->src_reg1val		= decode->output_instr->src_reg1val;
			L_S_Q->LSQ[i]->input_instr->src_reg2 		= decode->output_instr->src_reg2;
			L_S_Q->LSQ[i]->input_instr->src_reg2val		= decode->output_instr->src_reg2val;
			L_S_Q->LSQ[i]->input_instr->loaded_reg1val	= decode->output_instr->loaded_reg1val;
			L_S_Q->LSQ[i]->input_instr->loaded_reg2val	= decode->output_instr->loaded_reg2val;
			L_S_Q->LSQ[i]->input_instr->dest_reg 		= decode->output_instr->dest_reg;
			L_S_Q->LSQ[i]->input_instr->dest_regval		= decode->input_instr->dest_regval;
			L_S_Q->LSQ[i]->input_instr->tgt_mem_addr	= decode->output_instr->tgt_mem_addr;
			L_S_Q->LSQ[i]->input_instr->tgt_mem_data	= decode->output_instr->tgt_mem_data;
			L_S_Q->LSQ[i]->input_instr->litrl		= decode->output_instr->litrl;
			L_S_Q->LSQ[i]->input_instr->phy_reg		= decode->output_instr->phy_reg;
			L_S_Q->LSQ[i]->input_instr->cycle_cnt		= decode->output_instr->cycle_cnt;
			L_S_Q->LSQ[i]->input_instr->lsq_cycle_cnt	= 0;   
		       	L_S_Q->LSQ[i]->lsq_empty = false;	
			strcpy(L_S_Q->LSQ[i]->input_instr->instr, decode->output_instr->instr);	
			
			//printf("load/store instruction -> LSQ \n ");
			}

		}

	//Identify the empty slot in RE-ORDER BUFFER and insert the instruction
/*
	i = 0;

	while(i<32)
		{
		if(R_O_B->ROB[i]->rob_empty)
			break;		
		i++;
		}*/

	if(decode->output_instr->PC != 0)
		{
		//R_O_B->rob_tail = R_O_B->rob_tail + 1;
		
    i = rob_tail;
		R_O_B->ROB[i]->input_instr->PC 	= decode->output_instr->PC;
		strcpy(R_O_B->ROB[i]->input_instr->instr_string, decode->output_instr->instr_string);
		R_O_B->ROB[i]->input_instr->src_reg1 		= decode->output_instr->src_reg1;
		R_O_B->ROB[i]->input_instr->src_reg1val		= decode->output_instr->src_reg1val;
		R_O_B->ROB[i]->input_instr->src_reg2 		= decode->output_instr->src_reg2;
		R_O_B->ROB[i]->input_instr->src_reg2val		= decode->output_instr->src_reg2val;
		R_O_B->ROB[i]->input_instr->dest_reg 		= decode->output_instr->dest_reg;
		R_O_B->ROB[i]->input_instr->loaded_reg1val	= decode->output_instr->loaded_reg1val;
		R_O_B->ROB[i]->input_instr->loaded_reg2val	= decode->output_instr->loaded_reg2val;
		R_O_B->ROB[i]->input_instr->dest_regval		= decode->input_instr->dest_regval;
		R_O_B->ROB[i]->input_instr->tgt_mem_addr	= decode->output_instr->tgt_mem_addr;
		R_O_B->ROB[i]->input_instr->tgt_mem_data	= decode->output_instr->tgt_mem_data;
		R_O_B->ROB[i]->input_instr->litrl		= decode->output_instr->litrl;
		R_O_B->ROB[i]->input_instr->phy_reg		= decode->output_instr->phy_reg;
		R_O_B->ROB[i]->input_instr->cycle_cnt		= decode->output_instr->cycle_cnt;
		R_O_B->ROB[i]->rob_empty = false;
		strcpy(R_O_B->ROB[i]->input_instr->instr, decode->output_instr->instr);	

    rob_tail = rob_tail + 1;
		//printf("instruction entered into ROB. \n");
		}

	init_stage_decode_output(decode);


}


			// * * * Issue Queue Processing * * * //

void issue_queue(struct issue_queue *I_Q, struct stage_info *non_mul_exe, struct stage_info *mul_exe1, struct stage_info *div_exe1)
{

int div=-1, mul=-1, non_mul=-1, i;
int div_clk=0, mul_clk=0, non_mul_clk=0;

  
  //Increment the clock counter
  for(i=0; i<16; i++)
  {
    if(I_Q->IQ[i]->iq_empty == false)
      I_Q->IQ[i]->input_instr->cycle_cnt = I_Q->IQ[i]->input_instr->cycle_cnt + 1;      
  }


  //Identify the earliest instruction
  for(i=0; i<16; i++)
    {
      if(strcmp(I_Q->IQ[i]->input_instr->instr_string, str_div) == 0 && I_Q->IQ[i]->input_instr->loaded_reg1val && I_Q->IQ[i]->input_instr->loaded_reg2val )
        if(I_Q->IQ[i]->input_instr->cycle_cnt >= div_clk)
          {
          div_clk = I_Q->IQ[i]->input_instr->cycle_cnt;
          div = i;
          }


      if(strcmp(I_Q->IQ[i]->input_instr->instr_string, str_mul) == 0 && I_Q->IQ[i]->input_instr->loaded_reg1val && I_Q->IQ[i]->input_instr->loaded_reg2val )
        if(I_Q->IQ[i]->input_instr->cycle_cnt >= mul_clk)
          {
          mul_clk = I_Q->IQ[i]->input_instr->cycle_cnt;
          mul = i;
          }
          
      //if(strcmp(I_Q->IQ[i]->input_instr->instr_string, str_div) != 0 && strcmp(I_Q->IQ[i]->input_instr->instr_string, str_mul) != 0)
        //{
        bool chk = false;
        
        if(strcmp(I_Q->IQ[i]->input_instr->instr_string, str_movc) ==0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_bz) ==0 ||
        strcmp(I_Q->IQ[i]->input_instr->instr_string, str_bnz) ==0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_halt) ==0)  
        chk=true;
        
        if( (strcmp(I_Q->IQ[i]->input_instr->instr_string, str_load) == 0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_jump) == 0 || 
				strcmp(I_Q->IQ[i]->input_instr->instr_string, str_jal) == 0) && I_Q->IQ[i]->input_instr->loaded_reg1val)
        chk = true;
        
        
	if(strcmp(I_Q->IQ[i]->input_instr->instr_string, str_add) == 0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_sub) == 0 || 
		 strcmp(I_Q->IQ[i]->input_instr->instr_string, str_or) == 0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_and) == 0  || 
     			strcmp(I_Q->IQ[i]->input_instr->instr_string, str_exor) == 0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_store) == 0 )
         	if(I_Q->IQ[i]->input_instr->loaded_reg1val && I_Q->IQ[i]->input_instr->loaded_reg2val)       
           		chk = true;
        
              
        
        if(I_Q->IQ[i]->input_instr->cycle_cnt >= non_mul_clk && chk)
          {
          non_mul_clk = I_Q->IQ[i]->input_instr->cycle_cnt;
          non_mul = i;
          }                    
        //}
    }


if(div >-1 && I_Q->IQ[div]->iq_empty == false )
  {

  init_stage_div_exe1_input(div_exe1);		//Clear DIV-EXE1 stages
  
	div_exe1->input_instr->PC = I_Q->IQ[div]->input_instr->PC; 
	strcpy(div_exe1->input_instr->instr_string, I_Q->IQ[div]->input_instr->instr_string);
	div_exe1->input_instr->src_reg1 = I_Q->IQ[div]->input_instr->src_reg1; 	
	div_exe1->input_instr->src_reg1val = I_Q->IQ[div]->input_instr->src_reg1val;	
	div_exe1->input_instr->src_reg2 = I_Q->IQ[div]->input_instr->src_reg2; 	
	div_exe1->input_instr->src_reg2val = I_Q->IQ[div]->input_instr->src_reg2val;	
	div_exe1->input_instr->dest_reg = I_Q->IQ[div]->input_instr->dest_reg; 	
	div_exe1->input_instr->dest_regval = I_Q->IQ[div]->input_instr->dest_regval;	
	div_exe1->input_instr->tgt_mem_addr = I_Q->IQ[div]->input_instr->tgt_mem_addr;	
	div_exe1->input_instr->tgt_mem_data = I_Q->IQ[div]->input_instr->tgt_mem_data;	
	div_exe1->input_instr->litrl = I_Q->IQ[div]->input_instr->litrl;
	div_exe1->input_instr->phy_reg = I_Q->IQ[div]->input_instr->phy_reg;		
	strcpy(div_exe1->input_instr->instr, I_Q->IQ[div]->input_instr->instr);	  

  	//Clear Issue Queue Entry
  	init_issue_queue_input(I_Q, div);

  }


if(mul > -1 && I_Q->IQ[mul]->iq_empty == false )
  {

  init_stage_mul_exe1_input(mul_exe1);		//Clear MUL-EXE1 stages
  
	mul_exe1->input_instr->PC = I_Q->IQ[mul]->input_instr->PC; 
	strcpy(mul_exe1->input_instr->instr_string, I_Q->IQ[mul]->input_instr->instr_string);
	mul_exe1->input_instr->src_reg1 = I_Q->IQ[mul]->input_instr->src_reg1; 	
	mul_exe1->input_instr->src_reg1val = I_Q->IQ[mul]->input_instr->src_reg1val;	
	mul_exe1->input_instr->src_reg2 = I_Q->IQ[mul]->input_instr->src_reg2; 	
	mul_exe1->input_instr->src_reg2val = I_Q->IQ[mul]->input_instr->src_reg2val;	
	mul_exe1->input_instr->dest_reg = I_Q->IQ[mul]->input_instr->dest_reg; 	
	mul_exe1->input_instr->dest_regval = I_Q->IQ[mul]->input_instr->dest_regval;	
	mul_exe1->input_instr->tgt_mem_addr = I_Q->IQ[mul]->input_instr->tgt_mem_addr;	
	mul_exe1->input_instr->tgt_mem_data = I_Q->IQ[mul]->input_instr->tgt_mem_data;	
	mul_exe1->input_instr->litrl = I_Q->IQ[mul]->input_instr->litrl;
	mul_exe1->input_instr->phy_reg = I_Q->IQ[mul]->input_instr->phy_reg;		
	strcpy(mul_exe1->input_instr->instr, I_Q->IQ[mul]->input_instr->instr);	  

  	//Clear Issue Queue Entry
  	init_issue_queue_input(I_Q, mul);

  }
  
  
if(non_mul > -1 && I_Q->IQ[non_mul]->iq_empty == false )
  {

  init_stage_non_mul_exe_input(non_mul_exe);		//Clear NON-MUL-EXE stages
  
	non_mul_exe->input_instr->PC = I_Q->IQ[non_mul]->input_instr->PC; 
	strcpy(non_mul_exe->input_instr->instr_string, I_Q->IQ[non_mul]->input_instr->instr_string);
	non_mul_exe->input_instr->src_reg1 = I_Q->IQ[non_mul]->input_instr->src_reg1; 	
	non_mul_exe->input_instr->src_reg1val = I_Q->IQ[non_mul]->input_instr->src_reg1val;	
	non_mul_exe->input_instr->src_reg2 = I_Q->IQ[non_mul]->input_instr->src_reg2; 	
	non_mul_exe->input_instr->src_reg2val = I_Q->IQ[non_mul]->input_instr->src_reg2val;	
	non_mul_exe->input_instr->dest_reg = I_Q->IQ[non_mul]->input_instr->dest_reg; 	
	non_mul_exe->input_instr->dest_regval = I_Q->IQ[non_mul]->input_instr->dest_regval;	
	non_mul_exe->input_instr->tgt_mem_addr = I_Q->IQ[non_mul]->input_instr->tgt_mem_addr;	
	non_mul_exe->input_instr->tgt_mem_data = I_Q->IQ[non_mul]->input_instr->tgt_mem_data;	
	non_mul_exe->input_instr->litrl = I_Q->IQ[non_mul]->input_instr->litrl;
	non_mul_exe->input_instr->phy_reg = I_Q->IQ[non_mul]->input_instr->phy_reg;		
	strcpy(non_mul_exe->input_instr->instr, I_Q->IQ[non_mul]->input_instr->instr);
		

  	//Clear Issue Queue Entry
  	init_issue_queue_input(I_Q, non_mul);

  }


}

			// * * * NON MULTIPLICATION Arithmatic Operation * * * //

void non_mul_exe_instruction(struct stage_info *non_mul_exe, struct re_order_buff *R_O_B, struct physical_register *phy_reg)
{


if(non_mul_exe->input_instr->PC == 0)
	return;

init_stage_non_mul_exe_output(non_mul_exe);


		//execute arithmatic operation


if(strcmp(non_mul_exe->input_instr->instr_string, str_add) == 0)
		{
		non_mul_exe->input_instr->dest_regval = non_mul_exe->input_instr->src_reg1val + non_mul_exe->input_instr->src_reg2val;	//ADD 
		}

if(strcmp(non_mul_exe->input_instr->instr_string, str_sub) == 0)
		{
		non_mul_exe->input_instr->dest_regval = non_mul_exe->input_instr->src_reg1val - non_mul_exe->input_instr->src_reg2val;	//SUB
		}

if(strcmp(non_mul_exe->input_instr->instr_string, str_or) == 0)
		{
		non_mul_exe->input_instr->dest_regval = non_mul_exe->input_instr->src_reg1val | non_mul_exe->input_instr->src_reg2val;	//OR
		}

if(strcmp(non_mul_exe->input_instr->instr_string, str_and) == 0)
		{
		non_mul_exe->input_instr->dest_regval = non_mul_exe->input_instr->src_reg1val & non_mul_exe->input_instr->src_reg2val;	//AND
		}

if(strcmp(non_mul_exe->input_instr->instr_string, str_exor) == 0)
		{
		non_mul_exe->input_instr->dest_regval = non_mul_exe->input_instr->src_reg1val ^ non_mul_exe->input_instr->src_reg2val;	//EX-OR
		}

if(strcmp(non_mul_exe->input_instr->instr_string, str_store) == 0)
		{
		non_mul_exe->input_instr->tgt_mem_addr = non_mul_exe->input_instr->src_reg2val + non_mul_exe->input_instr->litrl;	//STORE
		}

if(strcmp(non_mul_exe->input_instr->instr_string, str_load) == 0)
		{
		non_mul_exe->input_instr->tgt_mem_addr = non_mul_exe->input_instr->src_reg1val + non_mul_exe->input_instr->litrl;	//LOAD
		}

if(strcmp(non_mul_exe->input_instr->instr_string, str_jump) == 0)
		{
		non_mul_exe->input_instr->tgt_mem_addr = non_mul_exe->input_instr->src_reg1val + non_mul_exe->input_instr->litrl;	//JUMP
		}

if(strcmp(non_mul_exe->input_instr->instr_string, str_jal) == 0)
		{
		non_mul_exe->input_instr->tgt_mem_addr = non_mul_exe->input_instr->src_reg1val + non_mul_exe->input_instr->litrl;	//JAL
		non_mul_exe->input_instr->dest_regval = non_mul_exe->input_instr->PC + 4;
   		}


non_mul_exe->output_instr->PC = non_mul_exe->input_instr->PC;
strcpy(non_mul_exe->output_instr->instr_string, non_mul_exe->input_instr->instr_string);
non_mul_exe->output_instr->src_reg1 = non_mul_exe->input_instr->src_reg1;
non_mul_exe->output_instr->src_reg2 = non_mul_exe->input_instr->src_reg2;
non_mul_exe->output_instr->src_reg1val = non_mul_exe->input_instr->src_reg1val;
non_mul_exe->output_instr->src_reg2val = non_mul_exe->input_instr->src_reg2val;
non_mul_exe->output_instr->dest_reg = non_mul_exe->input_instr->dest_reg;
non_mul_exe->output_instr->dest_regval = non_mul_exe->input_instr->dest_regval;
non_mul_exe->output_instr->tgt_mem_addr = non_mul_exe->input_instr->tgt_mem_addr;
non_mul_exe->output_instr->tgt_mem_data = non_mul_exe->input_instr->tgt_mem_data;
non_mul_exe->output_instr->litrl = non_mul_exe->input_instr->litrl;
non_mul_exe->output_instr->phy_reg = non_mul_exe->input_instr->phy_reg;
strcpy(non_mul_exe->output_instr->instr, non_mul_exe->input_instr->instr);
non_mul_exe->output_instr->rob_commit = true;


phy_reg->Regstr[non_mul_exe->input_instr->dest_reg].status = true; 
phy_reg->Regstr[non_mul_exe->input_instr->dest_reg].value = non_mul_exe->input_instr->dest_regval;	


//mark ROB entry to commit
mark_rob_commit(R_O_B, non_mul_exe->output_instr->PC);


if(strcmp(non_mul_exe->output_instr->instr_string, str_nop) == 0)
	{
	printf("EXE       :\t\t%s\n", non_mul_exe->output_instr->instr_string);
	}
else
	{
	printf("EXE       :%d\t%s\n", non_mul_exe->output_instr->PC, non_mul_exe->output_instr->instr);
	}

	
init_stage_non_mul_exe_input(non_mul_exe);

}


		// *** MUL EXE 1 Stage*** //
		//==========================//

void mul_exe1_instruction(struct stage_info *decode, struct stage_info *mul_exe1)

{

if(mul_exe1->input_instr->PC == 0)
  return;


init_stage_mul_exe1_output(mul_exe1);		

mul_exe1->output_instr->PC = mul_exe1->input_instr->PC;
strcpy(mul_exe1->output_instr->instr_string, mul_exe1->input_instr->instr_string);
mul_exe1->output_instr->src_reg1 = mul_exe1->input_instr->src_reg1;
mul_exe1->output_instr->src_reg2 = mul_exe1->input_instr->src_reg2;
mul_exe1->output_instr->src_reg1val = mul_exe1->input_instr->src_reg1val;
mul_exe1->output_instr->src_reg2val = mul_exe1->input_instr->src_reg2val;
mul_exe1->output_instr->dest_regval = mul_exe1->input_instr->dest_regval;
mul_exe1->output_instr->dest_reg = mul_exe1->input_instr->dest_reg;
mul_exe1->output_instr->dest_regval = mul_exe1->input_instr->dest_regval;
mul_exe1->output_instr->tgt_mem_addr = mul_exe1->input_instr->tgt_mem_addr;
mul_exe1->output_instr->tgt_mem_data = mul_exe1->input_instr->tgt_mem_data;
mul_exe1->output_instr->litrl = mul_exe1->input_instr->litrl;
mul_exe1->output_instr->phy_reg = mul_exe1->input_instr->phy_reg;
strcpy(mul_exe1->output_instr->instr, mul_exe1->input_instr->instr);

	
printf("MUL EXE1  :%d\t%s\n", mul_exe1->output_instr->PC, mul_exe1->output_instr->instr);

init_stage_mul_exe1_input(mul_exe1);

  
}


	// *** MUL EXE2 Stage *** //	

void mul_exe2_instruction(struct stage_info *mul_exe1, struct stage_info *mul_exe2, struct re_order_buff *R_O_B, struct physical_register *phy_reg)
{

if(mul_exe2->output_instr->PC != 0)
	printf("MUL EXE2  :%d\t%s\t(Stalled)\n", mul_exe2->output_instr->PC, mul_exe2->output_instr->instr);
	
	
if (mul_exe1->output_instr->PC == 0 || mul_exe2->output_instr->PC != 0 || mul_exe2->stalled == true)	
	{
	//mul_exe_data_forward(decode, mul_exe2, Reg);		//Data Forwarding
	return;
	}


init_stage_mul_exe2_input(mul_exe2);		//Clear MUL-EXE2 stage
init_stage_mul_exe2_output(mul_exe2);		

mul_exe2->input_instr->PC = mul_exe1->output_instr->PC;
strcpy(mul_exe2->input_instr->instr_string, mul_exe1->output_instr->instr_string);
mul_exe2->input_instr->src_reg1 = mul_exe1->output_instr->src_reg1;
mul_exe2->input_instr->src_reg2 = mul_exe1->output_instr->src_reg2;
mul_exe2->input_instr->src_reg1val = mul_exe1->output_instr->src_reg1val;
mul_exe2->input_instr->src_reg2val = mul_exe1->output_instr->src_reg2val;
mul_exe2->input_instr->dest_regval = mul_exe1->output_instr->dest_regval;
mul_exe2->input_instr->dest_reg = mul_exe1->output_instr->dest_reg;
mul_exe2->input_instr->dest_regval = mul_exe1->output_instr->dest_regval;
mul_exe2->input_instr->tgt_mem_addr = mul_exe1->output_instr->tgt_mem_addr;
mul_exe2->input_instr->tgt_mem_data = mul_exe1->output_instr->tgt_mem_data;
mul_exe2->input_instr->litrl = mul_exe1->output_instr->litrl;
mul_exe2->input_instr->phy_reg = mul_exe1->output_instr->phy_reg;
strcpy(mul_exe2->input_instr->instr, mul_exe1->output_instr->instr);


mul_exe2->input_instr->dest_regval = mul_exe2->input_instr->src_reg1val * mul_exe2->input_instr->src_reg2val;	//MUL FUNCTION


mul_exe2->output_instr->PC = mul_exe2->input_instr->PC;
strcpy(mul_exe2->output_instr->instr_string, mul_exe2->input_instr->instr_string);
mul_exe2->output_instr->src_reg1 = mul_exe2->input_instr->src_reg1;
mul_exe2->output_instr->src_reg2 = mul_exe2->input_instr->src_reg2;
mul_exe2->output_instr->src_reg1val = mul_exe2->input_instr->src_reg1val;
mul_exe2->output_instr->src_reg2val = mul_exe2->input_instr->src_reg2val;
mul_exe2->output_instr->dest_reg = mul_exe2->input_instr->dest_reg;
mul_exe2->output_instr->dest_regval = mul_exe2->input_instr->dest_regval;
mul_exe2->output_instr->tgt_mem_addr = mul_exe2->input_instr->tgt_mem_addr;
mul_exe2->output_instr->tgt_mem_data = mul_exe2->input_instr->tgt_mem_data;
mul_exe2->output_instr->litrl = mul_exe2->input_instr->litrl;
mul_exe2->output_instr->phy_reg = mul_exe2->input_instr->phy_reg;
strcpy(mul_exe2->output_instr->instr, mul_exe2->input_instr->instr);
mul_exe2->output_instr->rob_commit = true;

phy_reg->Regstr[mul_exe2->input_instr->dest_reg].status = true; 
phy_reg->Regstr[mul_exe2->input_instr->dest_reg].value = mul_exe2->input_instr->dest_regval;	


//mark ROB entry to commit
mark_rob_commit(R_O_B, mul_exe2->output_instr->PC);


printf("MUL EXE2  :%d\t%s \n", mul_exe2->output_instr->PC, mul_exe2->output_instr->instr);

init_stage_mul_exe2_input(mul_exe2);
init_stage_mul_exe1_output(mul_exe1);



}


    		// * * * DIVISION Function Unit * * * //

void div_exe1_instruction(struct stage_info *decode, struct stage_info *div_exe1)

{

if(div_exe1->input_instr->PC == 0)
	return;

	
init_stage_div_exe1_output(div_exe1);		


div_exe1->output_instr->PC = div_exe1->input_instr->PC;
strcpy(div_exe1->output_instr->instr_string, div_exe1->input_instr->instr_string);
div_exe1->output_instr->src_reg1 = div_exe1->input_instr->src_reg1;
div_exe1->output_instr->src_reg2 = div_exe1->input_instr->src_reg2;
div_exe1->output_instr->src_reg1val = div_exe1->input_instr->src_reg1val;
div_exe1->output_instr->src_reg2val = div_exe1->input_instr->src_reg2val;
div_exe1->output_instr->dest_regval = div_exe1->input_instr->dest_regval;
div_exe1->output_instr->dest_reg = div_exe1->input_instr->dest_reg;
div_exe1->output_instr->dest_regval = div_exe1->input_instr->dest_regval;
div_exe1->output_instr->tgt_mem_addr = div_exe1->input_instr->tgt_mem_addr;
div_exe1->output_instr->tgt_mem_data = div_exe1->input_instr->tgt_mem_data;
div_exe1->output_instr->litrl = div_exe1->input_instr->litrl;
div_exe1->output_instr->phy_reg = div_exe1->input_instr->phy_reg;
strcpy(div_exe1->output_instr->instr, div_exe1->input_instr->instr);
	
printf("DIV_EXE1  :%d\t%s\n", div_exe1->output_instr->PC, div_exe1->output_instr->instr);

init_stage_div_exe1_input(div_exe1);

}



//	DIV2 Function Unit


void div_exe2_instruction(struct stage_info *div_exe1, struct stage_info *div_exe2)
{

if(div_exe2->output_instr->PC != 0)
	printf("DIV EXE2  :%d\t%s\t(Stalled)\n", div_exe2->output_instr->PC, div_exe2->output_instr->instr);


if (div_exe1->output_instr->PC == 0 || div_exe2->output_instr->PC != 0 || div_exe2->stalled == true)	
	{
	return;
	}


init_stage_div_exe2_input(div_exe2);		//Clear MUL-EXE2 stage
init_stage_div_exe2_output(div_exe2);		

div_exe2->input_instr->PC = div_exe1->output_instr->PC;
strcpy(div_exe2->input_instr->instr_string, div_exe1->output_instr->instr_string);
div_exe2->input_instr->src_reg1 = div_exe1->output_instr->src_reg1;
div_exe2->input_instr->src_reg2 = div_exe1->output_instr->src_reg2;
div_exe2->input_instr->src_reg1val = div_exe1->output_instr->src_reg1val;
div_exe2->input_instr->src_reg2val = div_exe1->output_instr->src_reg2val;
div_exe2->input_instr->dest_regval = div_exe1->output_instr->dest_regval;
div_exe2->input_instr->dest_reg = div_exe1->output_instr->dest_reg;
div_exe2->input_instr->dest_regval = div_exe1->output_instr->dest_regval;
div_exe2->input_instr->tgt_mem_addr = div_exe1->output_instr->tgt_mem_addr;
div_exe2->input_instr->tgt_mem_data = div_exe1->output_instr->tgt_mem_data;
div_exe2->input_instr->litrl = div_exe1->output_instr->litrl;
div_exe2->input_instr->phy_reg = div_exe1->output_instr->phy_reg;
strcpy(div_exe2->input_instr->instr, div_exe1->output_instr->instr);


div_exe2->output_instr->PC = div_exe2->input_instr->PC;
strcpy(div_exe2->output_instr->instr_string, div_exe2->input_instr->instr_string);
div_exe2->output_instr->src_reg1 = div_exe2->input_instr->src_reg1;
div_exe2->output_instr->src_reg2 = div_exe2->input_instr->src_reg2;
div_exe2->output_instr->src_reg1val = div_exe2->input_instr->src_reg1val;
div_exe2->output_instr->src_reg2val = div_exe2->input_instr->src_reg2val;
div_exe2->output_instr->dest_reg = div_exe2->input_instr->dest_reg;
div_exe2->output_instr->dest_regval = div_exe2->input_instr->dest_regval;
div_exe2->output_instr->tgt_mem_addr = div_exe2->input_instr->tgt_mem_addr;
div_exe2->output_instr->tgt_mem_data = div_exe2->input_instr->tgt_mem_data;
div_exe2->output_instr->litrl = div_exe2->input_instr->litrl;
div_exe2->output_instr->phy_reg = div_exe2->input_instr->phy_reg;
strcpy(div_exe2->output_instr->instr, div_exe2->input_instr->instr);


printf("DIV_EXE2  :%d\t%s\n", div_exe2->output_instr->PC, div_exe2->output_instr->instr);

init_stage_div_exe2_input(div_exe2);
init_stage_div_exe1_output(div_exe1);

}



//	DIV3 Function unit

void div_exe3_instruction(struct stage_info *div_exe2, struct stage_info *div_exe3)
{

if(div_exe3->output_instr->PC != 0)
	printf("DIV EXE3  :%d\t%s\t(Stalled)\n", div_exe3->output_instr->PC, div_exe3->output_instr->instr);

	
if (div_exe2->output_instr->PC == 0 || div_exe3->output_instr->PC != 0 || div_exe3->stalled == true)	
	{
	return;
	}


init_stage_div_exe3_input(div_exe3);		//Clear MUL-EXE2 stage
init_stage_div_exe3_output(div_exe3);		

div_exe3->input_instr->PC = div_exe2->output_instr->PC;
strcpy(div_exe3->input_instr->instr_string, div_exe2->output_instr->instr_string);
div_exe3->input_instr->src_reg1 = div_exe2->output_instr->src_reg1;
div_exe3->input_instr->src_reg2 = div_exe2->output_instr->src_reg2;
div_exe3->input_instr->src_reg1val = div_exe2->output_instr->src_reg1val;
div_exe3->input_instr->src_reg2val = div_exe2->output_instr->src_reg2val;
div_exe3->input_instr->dest_reg = div_exe2->output_instr->dest_reg;
div_exe3->input_instr->dest_regval = div_exe2->output_instr->dest_regval;
div_exe3->input_instr->tgt_mem_addr = div_exe2->output_instr->tgt_mem_addr;
div_exe3->input_instr->tgt_mem_data = div_exe2->output_instr->tgt_mem_data;
div_exe3->input_instr->litrl = div_exe2->output_instr->litrl;
div_exe3->input_instr->phy_reg = div_exe2->output_instr->phy_reg;
strcpy(div_exe3->input_instr->instr, div_exe2->output_instr->instr);


div_exe3->output_instr->PC = div_exe3->input_instr->PC;
strcpy(div_exe3->output_instr->instr_string, div_exe3->input_instr->instr_string);
div_exe3->output_instr->src_reg1 = div_exe3->input_instr->src_reg1;
div_exe3->output_instr->src_reg2 = div_exe3->input_instr->src_reg2;
div_exe3->output_instr->src_reg1val = div_exe3->input_instr->src_reg1val;
div_exe3->output_instr->src_reg2val = div_exe3->input_instr->src_reg2val;
div_exe3->output_instr->dest_reg = div_exe3->input_instr->dest_reg;
div_exe3->output_instr->dest_regval = div_exe3->input_instr->dest_regval;
div_exe3->output_instr->tgt_mem_addr = div_exe3->input_instr->tgt_mem_addr;
div_exe3->output_instr->tgt_mem_data = div_exe3->input_instr->tgt_mem_data;
div_exe3->output_instr->litrl = div_exe3->input_instr->litrl;
div_exe3->output_instr->phy_reg = div_exe3->input_instr->phy_reg;
strcpy(div_exe3->output_instr->instr, div_exe3->input_instr->instr);


printf("DIV_EXE3  :%d\t%s\n", div_exe3->output_instr->PC, div_exe3->output_instr->instr);

init_stage_div_exe3_input(div_exe3);
init_stage_div_exe2_output(div_exe2);

}


//	DIV4 Function unit

void div_exe4_instruction(struct stage_info *div_exe3, struct stage_info *div_exe4, struct re_order_buff *R_O_B, struct physical_register *phy_reg)
{

if(div_exe4->output_instr->PC != 0)
	printf("DIV EXE4  :%d\t%s\t(Stalled)\n", div_exe4->output_instr->PC, div_exe4->output_instr->instr);

if (div_exe3->output_instr->PC == 0 || div_exe4->output_instr->PC != 0 || div_exe4->stalled == true)	
	{
	//div_exe_data_forward(decode, div_exe4, Reg);		//Data Forwarding
	return;
	}


init_stage_div_exe4_input(div_exe4);		//Clear MUL-EXE2 stage
init_stage_div_exe4_output(div_exe4);		

div_exe4->input_instr->PC = div_exe3->output_instr->PC;
strcpy(div_exe4->input_instr->instr_string, div_exe3->output_instr->instr_string);
div_exe4->input_instr->src_reg1 = div_exe3->output_instr->src_reg1;
div_exe4->input_instr->src_reg2 = div_exe3->output_instr->src_reg2;
div_exe4->input_instr->src_reg1val = div_exe3->output_instr->src_reg1val;
div_exe4->input_instr->src_reg2val = div_exe3->output_instr->src_reg2val;
div_exe4->input_instr->dest_regval = div_exe3->output_instr->dest_regval;
div_exe4->input_instr->dest_reg = div_exe3->output_instr->dest_reg;
div_exe4->input_instr->dest_regval = div_exe3->output_instr->dest_regval;
div_exe4->input_instr->tgt_mem_addr = div_exe3->output_instr->tgt_mem_addr;
div_exe4->input_instr->tgt_mem_data = div_exe3->output_instr->tgt_mem_data;
div_exe4->input_instr->litrl = div_exe3->output_instr->litrl;
div_exe4->input_instr->phy_reg = div_exe3->output_instr->phy_reg;
strcpy(div_exe4->input_instr->instr, div_exe3->output_instr->instr);

div_exe4->input_instr->dest_regval = div_exe4->input_instr->src_reg1val / div_exe4->input_instr->src_reg2val;	//DIV	Function
//printf("Div result=%d \n", div_exe4->input_instr->dest_regval);

div_exe4->output_instr->PC = div_exe4->input_instr->PC;
strcpy(div_exe4->output_instr->instr_string, div_exe4->input_instr->instr_string);
div_exe4->output_instr->src_reg1 = div_exe4->input_instr->src_reg1;
div_exe4->output_instr->src_reg2 = div_exe4->input_instr->src_reg2;
div_exe4->output_instr->src_reg1val = div_exe4->input_instr->src_reg1val;
div_exe4->output_instr->src_reg2val = div_exe4->input_instr->src_reg2val;
div_exe4->output_instr->dest_reg = div_exe4->input_instr->dest_reg;
div_exe4->output_instr->dest_regval = div_exe4->input_instr->dest_regval;
div_exe4->output_instr->tgt_mem_addr = div_exe4->input_instr->tgt_mem_addr;
div_exe4->output_instr->tgt_mem_data = div_exe4->input_instr->tgt_mem_data;
div_exe4->output_instr->litrl = div_exe4->input_instr->litrl;
div_exe4->output_instr->phy_reg = div_exe4->input_instr->phy_reg;
strcpy(div_exe4->output_instr->instr, div_exe4->input_instr->instr);

div_exe4->output_instr->rob_commit = true;


phy_reg->Regstr[div_exe4->input_instr->dest_reg].status = true; 
phy_reg->Regstr[div_exe4->input_instr->dest_reg].value = div_exe4->input_instr->dest_regval;	


//mark ROB entry to commit
mark_rob_commit(R_O_B, div_exe4->output_instr->PC);



printf("DIV EXE4  :%d\t%s\n", div_exe4->output_instr->PC, div_exe4->output_instr->instr);

init_stage_div_exe4_input(div_exe4);
init_stage_div_exe3_output(div_exe3);


}




            // * * * Process LSQ Entry to Memory * * * //

void process_load_store_queue(struct load_store_queue *L_S_Q, struct stage_info *memory)
{


//init_stage_memory_input(memory);

if(L_S_Q->LSQ[0]->lsq_empty == false)
	{	
  	memory->input_instr->PC = L_S_Q->LSQ[0]->input_instr->PC;
	strcpy(memory->input_instr->instr_string, L_S_Q->LSQ[0]->input_instr->instr_string);
	memory->input_instr->src_reg1 = L_S_Q->LSQ[0]->input_instr->src_reg1;
	memory->input_instr->src_reg2 = L_S_Q->LSQ[0]->input_instr->src_reg2;
	memory->input_instr->src_reg1val = L_S_Q->LSQ[0]->input_instr->src_reg1val;
	memory->input_instr->src_reg2val = L_S_Q->LSQ[0]->input_instr->src_reg2val;
	memory->input_instr->dest_reg = L_S_Q->LSQ[0]->input_instr->dest_reg;
	memory->input_instr->dest_regval = L_S_Q->LSQ[0]->input_instr->dest_regval;
	memory->input_instr->tgt_mem_addr = L_S_Q->LSQ[0]->input_instr->tgt_mem_addr;
	memory->input_instr->tgt_mem_data = L_S_Q->LSQ[0]->input_instr->tgt_mem_data;
	memory->input_instr->litrl = L_S_Q->LSQ[0]->input_instr->litrl;
	memory->input_instr->lsq_cycle_cnt = 1;
	memory->input_instr->phy_reg = L_S_Q->LSQ[0]->input_instr->phy_reg;
	strcpy(memory->input_instr->instr, L_S_Q->LSQ[0]->input_instr->instr);

	printf("LSQ       :\t\t%s\n", L_S_Q->LSQ[0]->input_instr->PC, L_S_Q->LSQ[0]->input_instr->instr);
	}


}

    				// * * * MEMORY Stage * * * //


void memory_instruction(struct stage_info *memory, struct data_memory *data, struct register_file *Reg, struct re_order_buff *R_O_B)

{

if(memory->input_instr->PC == 0)
	return;

memory->input_instr->lsq_cycle_cnt++;


if(memory->input_instr->lsq_cycle_cnt <3)
	return;

init_stage_memory_output(memory);


if(strcmp(memory->input_instr->instr_string, str_store) == 0)
		{
		data->data_val[memory->input_instr->tgt_mem_addr] = memory->input_instr->src_reg1val;		//STORE
		}

if(strcmp(memory->input_instr->instr_string, str_load) == 0)
		{
		memory->input_instr->tgt_mem_data = data->data_val[memory->input_instr->tgt_mem_addr]; 		//LOAD
		}


memory->output_instr->PC = memory->input_instr->PC;
strcpy(memory->output_instr->instr_string, memory->input_instr->instr_string);
memory->output_instr->src_reg1 = memory->input_instr->src_reg1;
memory->output_instr->src_reg2 = memory->input_instr->src_reg2;
memory->output_instr->src_reg1val = memory->input_instr->src_reg1val;
memory->output_instr->src_reg2val = memory->input_instr->src_reg2val;
memory->output_instr->dest_reg = memory->input_instr->dest_reg;
memory->output_instr->dest_regval = memory->input_instr->dest_regval;
memory->output_instr->tgt_mem_addr = memory->input_instr->tgt_mem_addr;
memory->output_instr->tgt_mem_data = memory->input_instr->tgt_mem_data;
memory->output_instr->litrl = memory->input_instr->litrl;
memory->output_instr->phy_reg = memory->input_instr->phy_reg;
memory->output_instr->rob_commit = true;
strcpy(memory->output_instr->instr, memory->input_instr->instr);


//Mark ROB entry to commit
mark_rob_commit(R_O_B, memory->output_instr->PC);


if(strcmp(memory->output_instr->instr_string, str_nop)== 0)
	{
	printf("MEMORY    :\t\t%s\n", memory->output_instr->instr_string);
	}
else
	{
	printf("MEMORY    :%d\t%s\n", memory->output_instr->PC, memory->output_instr->instr);
     	}


init_stage_memory_input(memory);
      
}


			// * * * Process ROB * * * // 


void process_rob(struct re_order_buff *R_O_B, struct register_file *Reg, struct physical_register *phy_reg, struct rename_table renam_tbl)
{

int i = 0;
int AR=-1, PR=-1;
bool rls_phy_reg = true;

//printf("rob head: %d \n", rob_head);


while(i<=1)
	{

  if(R_O_B->ROB[rob_head+i]->input_instr->rob_commit)
		{
    AR = R_O_B->ROB[rob_head+i]->input_instr->dest_reg;
		PR = R_O_B->ROB[rob_head+i]->input_instr->phy_reg;
	
		//Move the AR value 
		Reg->Regstr[AR].regval = R_O_B->ROB[rob_head+i]->input_instr->dest_regval;
		
		//Release AR if it points to the same PRF
		if(PR == renam_tbl.prf[AR])	
			{
			Reg->Regstr[AR].status = REG_STATUS_VALID;
			renam_tbl.prf[AR] = -1;
			renam_tbl.sts[AR] = false;
			}

		printf("ROB       :%d\t%s\n", R_O_B->ROB[rob_head+i]->input_instr->PC, R_O_B->ROB[rob_head+i]->input_instr->instr);

		
		//Clear the ROB head entry	
		init_re_order_buff_input(R_O_B, rob_head);

		//Increament ROB head pointer
		rob_head++;
		
		//Release PR if it does not depend on any other instruction
		for(int j=rob_head; j<=rob_tail; j++)
			{
			
			int PR1 = R_O_B->ROB[j]->input_instr->phy_reg;	
			if(PR == PR1)
				rls_phy_reg = false;
			}

		if(rls_phy_reg)
			{
			phy_reg->Regstr[PR].allocated = false;
			phy_reg->Regstr[PR].status = false; 
			//phy_reg->RegstPR[k].rename = false;
			phy_reg->Regstr[PR].value = 0;	
			
			}

		i= i+1;
		}
	else break;
	
	}


}


	// * * * Mark ROB Entry for Committing * * * //

void mark_rob_commit(struct re_order_buff *R_O_B, int PC)
{
	
for(int i=0; i<32; i++)
	{
		if(R_O_B->ROB[i]->input_instr->PC == PC)
			{
      R_O_B->ROB[i]->input_instr->rob_commit = true; 
			//printf("%d \t rob marked committed. \n", PC);
      }
	}

}



	// * * * Forward Data to DECODE Stage* * * //

void forward_data_DRF(struct stage_info *decode, struct stage_info *non_mul_exe, struct stage_info *mul_exe2, struct stage_info *div_exe4, struct stage_info *memory)
{

	
//Load source register1 value

if(strcmp(decode->output_instr->instr_string, str_load) == 0 || strcmp(decode->output_instr->instr_string, str_jump) == 0 || strcmp(decode->output_instr->instr_string, str_jal) == 0) 
	{
	if(decode->output_instr->loaded_reg1val != true)
		{
		if(decode->output_instr->src_reg1 == non_mul_exe->output_instr->dest_reg && non_mul_exe->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg1val =  non_mul_exe->output_instr->dest_regval;
			decode->output_instr->loaded_reg1val = true;
			}
		
		if(decode->output_instr->src_reg1 == mul_exe2->output_instr->dest_reg && mul_exe2->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg1val =  mul_exe2->output_instr->dest_regval;
			decode->output_instr->loaded_reg1val = true;
			}

		if(decode->output_instr->src_reg1 == div_exe4->output_instr->dest_reg && div_exe4->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg1val =  div_exe4->output_instr->dest_regval;
			decode->output_instr->loaded_reg1val = true;
			}
		
		if( strcmp(memory->output_instr->instr_string, str_load) == 0 && decode->output_instr->src_reg1 == memory->output_instr->dest_reg && memory->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg1val =  memory->output_instr->dest_regval;
			decode->output_instr->loaded_reg1val = true;
			}

		}

	}


if(strcmp(decode->output_instr->instr_string, str_add) == 0 || strcmp(decode->output_instr->instr_string, str_sub) == 0 || 
		strcmp(decode->output_instr->instr_string, str_mul) == 0  || strcmp(decode->output_instr->instr_string, str_or) == 0    ||
		strcmp(decode->output_instr->instr_string, str_and) == 0  || strcmp(decode->output_instr->instr_string, str_exor) == 0  ||
		strcmp(decode->output_instr->instr_string, str_load) == 0 || strcmp(decode->output_instr->instr_string, str_store) == 0 ||
		strcmp(decode->output_instr->instr_string, str_jump) == 0 || strcmp(decode->output_instr->instr_string, str_div) == 0   )
		{			

//Load source register1 value
			
	if(decode->output_instr->loaded_reg1val != true)
		{
		if(decode->output_instr->src_reg1 == non_mul_exe->output_instr->dest_reg && non_mul_exe->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg1val =  non_mul_exe->output_instr->dest_regval;
			decode->output_instr->loaded_reg1val = true;
			}
		
		if(decode->output_instr->src_reg1 == mul_exe2->output_instr->dest_reg && mul_exe2->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg1val =  mul_exe2->output_instr->dest_regval;
			decode->output_instr->loaded_reg1val = true;
			}

		if(decode->output_instr->src_reg1 == div_exe4->output_instr->dest_reg && div_exe4->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg1val =  div_exe4->output_instr->dest_regval;
			decode->output_instr->loaded_reg1val = true;
			}
		
		if( strcmp(memory->output_instr->instr_string, str_load) == 0 && decode->output_instr->src_reg1 == memory->output_instr->dest_reg && memory->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg1val =  memory->output_instr->dest_regval;
			decode->output_instr->loaded_reg1val = true;
			}

		}
		
//Load source register2 value

	if(decode->output_instr->loaded_reg2val != true)
		{
		if(decode->output_instr->src_reg2 == non_mul_exe->output_instr->dest_reg && non_mul_exe->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg2val =  non_mul_exe->output_instr->dest_regval;
			decode->output_instr->loaded_reg2val = true;
			}
		
		if(decode->output_instr->src_reg2 == mul_exe2->output_instr->dest_reg && mul_exe2->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg2val =  mul_exe2->output_instr->dest_regval;
			decode->output_instr->loaded_reg2val = true;
			}

		if(decode->output_instr->src_reg2 == div_exe4->output_instr->dest_reg && div_exe4->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg2val =  div_exe4->output_instr->dest_regval;
			decode->output_instr->loaded_reg2val = true;
			}
		
		if( strcmp(memory->output_instr->instr_string, str_load) == 0 && decode->output_instr->src_reg2 == memory->output_instr->dest_reg && memory->output_instr->PC != 0)
			{			
			decode->output_instr->src_reg2val =  memory->output_instr->dest_regval;
			decode->output_instr->loaded_reg2val = true;
			}

		}


	}


}



	// * * * Forward Data to Issue Queue * * * //

void forward_data_IQ(struct stage_info *decode, struct stage_info *non_mul_exe, struct stage_info *mul_exe2, struct stage_info *div_exe4, struct stage_info *memory, struct issue_queue *I_Q)
{


int i;

//Load source register1 value

for(i=0; i<16; i++)
	{

	if(strcmp(I_Q->IQ[i]->input_instr->instr_string, str_load) == 0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_jump) == 0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_jal) == 0) 
		{
		if(I_Q->IQ[i]->input_instr->loaded_reg1val != true)
			{
			if(I_Q->IQ[i]->input_instr->src_reg1 == non_mul_exe->output_instr->dest_reg && non_mul_exe->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg1val =  non_mul_exe->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg1val = true;
				}
			
			if(I_Q->IQ[i]->input_instr->src_reg1 == mul_exe2->output_instr->dest_reg && mul_exe2->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg1val =  mul_exe2->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg1val = true;
				}

			if(I_Q->IQ[i]->input_instr->src_reg1 == div_exe4->output_instr->dest_reg && div_exe4->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg1val =  div_exe4->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg1val = true;
				}
		
			if( strcmp(memory->output_instr->instr_string, str_load) == 0 && I_Q->IQ[i]->input_instr->src_reg1 == memory->output_instr->dest_reg && memory->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg1val =  memory->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg1val = true;
				}

			}

		}


	}


for(i=0; i<16; i++)
	{


	if(strcmp(I_Q->IQ[i]->input_instr->instr_string, str_add) == 0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_sub) == 0 || 
		strcmp(I_Q->IQ[i]->input_instr->instr_string, str_mul) == 0  || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_or) == 0    ||
		strcmp(I_Q->IQ[i]->input_instr->instr_string, str_and) == 0  || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_exor) == 0  ||
		strcmp(I_Q->IQ[i]->input_instr->instr_string, str_load) == 0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_store) == 0 ||
		strcmp(I_Q->IQ[i]->input_instr->instr_string, str_jump) == 0 || strcmp(I_Q->IQ[i]->input_instr->instr_string, str_div) == 0   )
		{			

	//Load source register1 value
			
		if(I_Q->IQ[i]->input_instr->loaded_reg1val != true)
			{
			if(I_Q->IQ[i]->input_instr->src_reg1 == non_mul_exe->output_instr->dest_reg && non_mul_exe->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg1val =  non_mul_exe->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg1val = true;
				}
		
			if(I_Q->IQ[i]->input_instr->src_reg1 == mul_exe2->output_instr->dest_reg && mul_exe2->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg1val =  mul_exe2->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg1val = true;
				}

			if(I_Q->IQ[i]->input_instr->src_reg1 == div_exe4->output_instr->dest_reg && div_exe4->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg1val =  div_exe4->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg1val = true;
				}
		
			if( strcmp(memory->output_instr->instr_string, str_load) == 0 && I_Q->IQ[i]->input_instr->src_reg1 == memory->output_instr->dest_reg && memory->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg1val =  memory->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg1val = true;
				}

			}
		
	//Load source register2 value

		if(I_Q->IQ[i]->input_instr->loaded_reg2val != true)
			{
			if(I_Q->IQ[i]->input_instr->src_reg2 == non_mul_exe->output_instr->dest_reg && non_mul_exe->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg2val =  non_mul_exe->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg2val = true;
				}
		
			if(I_Q->IQ[i]->input_instr->src_reg2 == mul_exe2->output_instr->dest_reg && mul_exe2->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg2val =  mul_exe2->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg2val = true;
				}

			if(I_Q->IQ[i]->input_instr->src_reg2 == div_exe4->output_instr->dest_reg && div_exe4->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg2val =  div_exe4->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg2val = true;
				}
		
			if( strcmp(memory->output_instr->instr_string, str_load) == 0 && I_Q->IQ[i]->input_instr->src_reg2 == memory->output_instr->dest_reg && memory->output_instr->PC != 0)
				{			
				I_Q->IQ[i]->input_instr->src_reg2val =  memory->output_instr->dest_regval;
				I_Q->IQ[i]->input_instr->loaded_reg2val = true;
				}

			}


		}


	}


}



void clear_function_units(struct stage_info *non_mul_exe, struct stage_info *mul_exe2, struct stage_info *div_exe4)
{
	init_stage_non_mul_exe_output(non_mul_exe);
	init_stage_mul_exe2_output(mul_exe2);
	init_stage_div_exe4_output(div_exe4);
}


/*

			// * * * WRITE BACK Stage * * * //

void wrtbck_instruction(struct stage_info *decode, struct stage_info *memory, struct stage_info *wrtbck, struct register_file *Reg, struct Flags *flag)

{


if (memory->output_instr->PC == 0)	//Proceed WRITEBACK stage if no stall
	{
	return;
	}


init_stage_wrtbck_input(wrtbck);	//Clear WRTBCK stages 	
init_stage_wrtbck_output(wrtbck);	


wrtbck->input_instr->PC = memory->output_instr->PC;
strcpy(wrtbck->input_instr->instr_string, memory->output_instr->instr_string);
wrtbck->input_instr->dest_regval = memory->output_instr->dest_regval;
wrtbck->input_instr->dest_reg = memory->output_instr->dest_reg;
wrtbck->input_instr->tgt_mem_addr = memory->output_instr->tgt_mem_addr;
wrtbck->input_instr->tgt_mem_data = memory->output_instr->tgt_mem_data;
wrtbck->input_instr->litrl = memory->output_instr->litrl;
strcpy(wrtbck->input_instr->instr, memory->output_instr->instr);


wrtbck->output_instr->PC = wrtbck->input_instr->PC;
strcpy(wrtbck->output_instr->instr_string, wrtbck->input_instr->instr_string);
wrtbck->output_instr->dest_regval = wrtbck->input_instr->dest_regval;
wrtbck->output_instr->dest_reg = wrtbck->input_instr->dest_reg;
wrtbck->output_instr->tgt_mem_addr = wrtbck->input_instr->tgt_mem_addr;
wrtbck->output_instr->tgt_mem_data = wrtbck->input_instr->tgt_mem_data;
wrtbck->output_instr->litrl = wrtbck->input_instr->litrl;
strcpy(wrtbck->output_instr->instr, wrtbck->input_instr->instr);



// Perform REGISTER WRITE operation

if(strcmp(wrtbck->input_instr->instr_string, str_add) == 0 || strcmp(wrtbck->input_instr->instr_string, str_sub) == 0 || 
		strcmp(wrtbck->input_instr->instr_string, str_mul) == 0 || strcmp(wrtbck->input_instr->instr_string, str_or) == 0 ||
		strcmp(wrtbck->input_instr->instr_string, str_and) == 0 || strcmp(wrtbck->input_instr->instr_string, str_exor) == 0 ||
		strcmp(wrtbck->input_instr->instr_string, str_div) == 0)
		{
		Reg->Regstr[wrtbck->output_instr->dest_reg].regval = wrtbck->output_instr->dest_regval;		//ADD,SUB,MUL,DIV,OR,AND,EX-OR -> Write register value
		
		
		if(decode_is_bz_or_bnz && (dep_branch_PC_addrs == wrtbck->output_instr->PC))
			{	
			flag->zero_status = STATUS_VALID;
			if(wrtbck->output_instr->dest_regval == 0)		//Set the ZERO Flag
				{
				flag->zero = true;
				}
			else
				{
				flag->zero = false;
				}
			}
		}

if(strcmp(wrtbck->input_instr->instr_string, str_load) == 0)
		{
		Reg->Regstr[wrtbck->output_instr->dest_reg].regval = wrtbck->output_instr->tgt_mem_data;	//LOAD
		}

if(strcmp(wrtbck->input_instr->instr_string, str_movc) == 0)
		{
		Reg->Regstr[wrtbck->output_instr->dest_reg].regval = wrtbck->output_instr->litrl;		//MOVC -> Write register with literal	
		}

if(strcmp(wrtbck->input_instr->instr_string, str_jal) == 0 )
		{
		Reg->Regstr[wrtbck->output_instr->dest_reg].regval = wrtbck->output_instr->dest_regval;		//JAL
		//load_jal_dest_reg = false;
    		}


//  MAKE THE OUPUT REGISTER TO VALID 

if(strcmp(wrtbck->output_instr->instr_string, str_add) == 0 || strcmp(wrtbck->output_instr->instr_string, str_sub) == 0 || 
		strcmp(wrtbck->output_instr->instr_string, str_mul) == 0  || strcmp(wrtbck->output_instr->instr_string, str_or) == 0    ||
		strcmp(wrtbck->output_instr->instr_string, str_and) == 0  || strcmp(wrtbck->output_instr->instr_string, str_exor) == 0  ||
		strcmp(wrtbck->output_instr->instr_string, str_movc) == 0 || strcmp(wrtbck->output_instr->instr_string, str_load) == 0  ||
    		strcmp(wrtbck->output_instr->instr_string, str_div) == 0  || strcmp(wrtbck->output_instr->instr_string, str_jal) == 0)		
		{
		Reg->Regstr[wrtbck->output_instr->dest_reg].status = REG_STATUS_VALID;
		}


printf("WRITEBACK :%d\t%s\n", wrtbck->output_instr->PC, wrtbck->output_instr->instr);

init_stage_memory_output(memory);
init_stage_wrtbck_input(wrtbck);	


}


*/

			// * * * FLUSH PIPELINE STAGES FOR HALT * * * //

void flush_pipeline(struct stage_info *fetch, struct stage_info *decode, struct register_file *Reg)
{

int temp_pc;

	init_stage_fetch_input(fetch);			//Flush FETCH stage
	
	temp_pc = fetch->output_instr->PC;	
	init_stage_fetch_output(fetch);	

	fetch->output_instr->PC = temp_pc;
	strcpy(fetch->output_instr->instr_string, str_nop);

	if(decode->input_instr->PC != 0)		//Flush DECODE Stage	
		{	
		temp_pc = decode->input_instr->PC;	//If previous instruction is held for dependency
		}
	else
		{
		temp_pc = decode->output_instr->PC;	//if previous instruction is in output of DECODE stage
		}

	
	if(decode->output_instr->PC != 0)
		{
		if( strcmp(decode->output_instr->instr_string, str_add) == 0 || strcmp(decode->output_instr->instr_string, str_sub) == 0 || strcmp(decode->output_instr->instr_string, str_or) == 0 || 
			strcmp(decode->output_instr->instr_string, str_and) == 0 || strcmp(decode->output_instr->instr_string, str_exor) == 0 || strcmp(decode->output_instr->instr_string, str_mul) == 0 || 
			strcmp(decode->output_instr->instr_string, str_div) == 0 || strcmp(decode->output_instr->instr_string, str_movc) == 0 || strcmp(decode->output_instr->instr_string, str_load) == 0 || 
			strcmp(decode->output_instr->instr_string, str_jal) == 0 )		
		Reg->Regstr[decode->output_instr->dest_reg].status = REG_STATUS_VALID; 
		//printf("Removed reg dependency. \n");
		}
	
	init_stage_decode_input(decode);
	init_stage_decode_output(decode);
	
	decode->output_instr->PC = temp_pc;
	strcpy(decode->output_instr->instr_string, str_nop);

}


/*

   
					// * * * Data Forwarding Logic * * * //

void forward_data(struct stage_info *decode, struct stage_info *non_mul_exe, struct stage_info *mul_exe2, struct stage_info *div_exe4, struct register_file *Reg)
{

bool data_forward = false;

if(decode->input_instr->PC == 0)
	{	
	return;				//No instruction @ DECODE input 
	}
	
if(non_mul_exe->output_instr->PC == 0 && mul_exe2->output_instr->PC == 0 && div_exe4->output_instr->PC == 0)
	{
	return;				//No Instruction in any of the function unit output
	}

if(strcmp(decode->input_instr->instr_string, str_movc) == 0 || strcmp(decode->input_instr->instr_string, str_bz) == 0 || strcmp(decode->input_instr->instr_string, str_bnz) == 0 || 
		strcmp(decode->input_instr->instr_string, str_halt) == 0)
	{
	return;				//Data forwarding not required for MOVC, BZ, BNZ, HALT as no source registers available for these instructions
	}

if((strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || strcmp(decode->input_instr->instr_string, str_or) == 0 || 
		strcmp(decode->input_instr->instr_string, str_and) == 0 || strcmp(decode->input_instr->instr_string, str_exor) == 0 || strcmp(decode->input_instr->instr_string, str_mul) == 0 || 
		strcmp(decode->input_instr->instr_string, str_div) == 0 || strcmp(decode->input_instr->instr_string, str_movc) == 0 || strcmp(decode->input_instr->instr_string, str_load) == 0 || 
		strcmp(decode->input_instr->instr_string, str_jal) == 0 ) && (Reg->Regstr[decode->input_instr->dest_reg].status == REG_STATUS_INVALID))	
	{
	return;				//Data forwarding can not be done if destination register is invalid
	}



// Instruction which needs one source register value

if( (strcmp(decode->input_instr->instr_string, str_load) == 0 || strcmp(decode->input_instr->instr_string, str_jump) == 0 || strcmp(decode->input_instr->instr_string, str_jal) == 0) 
		&& Reg->Regstr[decode->input_instr->src_reg1].status == REG_STATUS_INVALID )
	{
	
	if( (decode->input_instr->src_reg1 == non_mul_exe->output_instr->dest_reg) && ( strcmp(non_mul_exe->output_instr->instr_string, str_movc) == 0 || strcmp(non_mul_exe->output_instr->instr_string, str_add) == 0 || 
											strcmp(non_mul_exe->output_instr->instr_string, str_sub) == 0 || strcmp(non_mul_exe->output_instr->instr_string, str_or) == 0   || 
											strcmp(non_mul_exe->output_instr->instr_string, str_and) == 0 || strcmp(non_mul_exe->output_instr->instr_string, str_exor) == 0 || 
											strcmp(non_mul_exe->output_instr->instr_string, str_jal) == 0 ) )
		{
		decode->input_instr->src_reg1val = non_mul_exe->output_instr->dest_regval;
		data_forward = true;
		}
	
	if(decode->input_instr->src_reg1 == mul_exe2->output_instr->dest_reg && mul_exe2->output_instr->PC != 0)
		{
		decode->input_instr->src_reg1val = mul_exe2->output_instr->dest_regval;
		data_forward = true;
		}

	if(decode->input_instr->src_reg1 == div_exe4->output_instr->dest_reg && div_exe4->output_instr->PC != 0)
		{
		decode->input_instr->src_reg1val = div_exe4->output_instr->dest_regval;
		data_forward = true;
		}	
	
	}

// Instruction which needs two source registers values


if( strcmp(decode->input_instr->instr_string, str_add) == 0 || strcmp(decode->input_instr->instr_string, str_sub) == 0 || strcmp(decode->input_instr->instr_string, str_or) == 0 || 
	strcmp(decode->input_instr->instr_string, str_and) == 0 || strcmp(decode->input_instr->instr_string, str_exor) == 0 || strcmp(decode->input_instr->instr_string, str_store) == 0 ||
	strcmp(decode->input_instr->instr_string, str_mul) == 0 || strcmp(decode->input_instr->instr_string, str_div) == 0 )
	{
	

	if(Reg->Regstr[decode->input_instr->src_reg2].status == REG_STATUS_INVALID && loaded_src2_val == false)
		{
		if( (decode->input_instr->src_reg2 == non_mul_exe->output_instr->dest_reg) && ( strcmp(non_mul_exe->output_instr->instr_string, str_movc) == 0 || 
									strcmp(non_mul_exe->output_instr->instr_string, str_add) == 0 || strcmp(non_mul_exe->output_instr->instr_string, str_sub) == 0 || 
									strcmp(non_mul_exe->output_instr->instr_string, str_or) == 0  || strcmp(non_mul_exe->output_instr->instr_string, str_and) == 0 || 
									strcmp(non_mul_exe->output_instr->instr_string, str_exor) == 0 || strcmp(non_mul_exe->output_instr->instr_string, str_jal) == 0 ) )
			{
			temp_src2val = non_mul_exe->output_instr->dest_regval;
			//printf("data fwd - Invalid. reg2: %d\n", decode->input_instr->src_reg2);
			loaded_src2_val = true;		
			}
		if(decode->input_instr->src_reg2 == mul_exe2->output_instr->dest_reg && mul_exe2->output_instr->PC != 0)
			{
			temp_src2val = mul_exe2->output_instr->dest_regval;
			//printf("data fwd - Invalid. reg2: %d\n", decode->input_instr->src_reg2);
			loaded_src2_val = true;
			}

		if(decode->input_instr->src_reg2 == div_exe4->output_instr->dest_reg && div_exe4->output_instr->PC != 0)
			{
			temp_src2val = div_exe4->output_instr->dest_regval;
			//printf("data fwd - Invalid. reg2: %d\n", decode->input_instr->src_reg2);
			loaded_src2_val = true;
			}	
				
		}

	if(Reg->Regstr[decode->input_instr->src_reg1].status == REG_STATUS_INVALID && loaded_src1_val == false)
		{
		if( (decode->input_instr->src_reg1 == non_mul_exe->output_instr->dest_reg) && ( strcmp(non_mul_exe->output_instr->instr_string, str_movc) == 0 || 
									strcmp(non_mul_exe->output_instr->instr_string, str_add) == 0 || strcmp(non_mul_exe->output_instr->instr_string, str_sub) == 0 || 
									strcmp(non_mul_exe->output_instr->instr_string, str_or) == 0  || strcmp(non_mul_exe->output_instr->instr_string, str_and) == 0 || 
									strcmp(non_mul_exe->output_instr->instr_string, str_exor) == 0 || strcmp(non_mul_exe->output_instr->instr_string, str_jal) == 0 ) )
			{
			temp_src1val = non_mul_exe->output_instr->dest_regval;
			//printf("data fwd - Invalid. reg1 : %d\n", decode->input_instr->src_reg1);
			loaded_src1_val = true;		
			}
		if(decode->input_instr->src_reg1 == mul_exe2->output_instr->dest_reg && mul_exe2->output_instr->PC != 0)
			{
			temp_src1val = mul_exe2->output_instr->dest_regval;
			//printf("data fwd - Invalid. reg1 : %d\n", decode->input_instr->src_reg1);
			loaded_src1_val = true;
			}

		if(decode->input_instr->src_reg1 == div_exe4->output_instr->dest_reg && div_exe4->output_instr->PC != 0)
			{
			temp_src1val = div_exe4->output_instr->dest_regval;
			//printf("data fwd - Invalid. reg1 : %d\n", decode->input_instr->src_reg1);
			loaded_src1_val = true;
			}							
		
		}
	

	if(Reg->Regstr[decode->input_instr->src_reg1].status == REG_STATUS_VALID && loaded_src1_val == false)
		{
		temp_src1val = Reg->Regstr[decode->input_instr->src_reg1].regval;
		//printf("data fwd - valid. reg1 : %d\n", decode->input_instr->src_reg1);
		loaded_src1_val = true;
		}

	if(Reg->Regstr[decode->input_instr->src_reg2].status == REG_STATUS_VALID && loaded_src2_val == false)
		{
		temp_src2val = Reg->Regstr[decode->input_instr->src_reg2].regval;
		//printf("data fwd - valid. reg2 : %d\n", decode->input_instr->src_reg2);
		loaded_src2_val = true;
		}

	}


if(loaded_src1_val && loaded_src2_val)
	{	
	decode->input_instr->src_reg1val = temp_src1val;
	decode->input_instr->src_reg2val = temp_src2val;
	data_forward = true;
	}


// Load DECODE instruction output 

if(data_forward)
  {
  decode->output_instr->PC = decode->input_instr->PC;
  strcpy(decode->output_instr->instr_string, decode->input_instr->instr_string);
  decode->output_instr->src_reg1 = decode->input_instr->src_reg1;
  decode->output_instr->src_reg2 = decode->input_instr->src_reg2;
  decode->output_instr->src_reg1val = decode->input_instr->src_reg1val;
  decode->output_instr->src_reg2val = decode->input_instr->src_reg2val;
  decode->output_instr->dest_reg = decode->input_instr->dest_reg;
  decode->output_instr->tgt_mem_addr = decode->input_instr->tgt_mem_addr;
  decode->output_instr->tgt_mem_data = decode->input_instr->tgt_mem_data;
  decode->output_instr->litrl = decode->input_instr->litrl;
  strcpy(decode->output_instr->instr, decode->input_instr->instr);
  
  //printf(" *** Data Forwarded *** \n");
  init_stage_decode_input(decode);  
  
  loaded_src1_val = false;
  loaded_src2_val = false;
  
  temp_src1val = 0;
  temp_src2val = 0;

  }


}

*/


	// * * * Function: getinputfile * * * //



void getinputfile(struct code_mem *inpt)
{

FILE *fp;
int i=0;

printf("Enter Input File Name: \n");
scanf("%s", filename);

//fp = fopen("input1.c", "r"); 
fp = fopen(filename, "r");

while(!feof(fp))
	{

	i++;

	//fread(inpt.inpstr[i].instr_string, 1, 20, fp);
  	fscanf(fp, "%s", inpt->inpstr[i].instr_string);

	//if(feof(fp))
		//return;

	inpt->inpstr[i].line_no = i;

	if (i==1)
	{
	inpt->inpstr[i].address = BASE_MEM_ADDR; 
	}
	else
	{	
	inpt->inpstr[i].address = inpt->inpstr[i-1].address + 4 ; 
	}

	} ;

fclose(fp);

}

	//Display the input file

void displayinputfile(struct code_mem *inpt)
{

for(int i=1; strcmp(inpt->inpstr[i].instr_string, "") != 0; i++)
  {
  printf("%d\t", inpt->inpstr[i].line_no);
  printf("%d\t", inpt->inpstr[i].address);
  printf("%s\n", inpt->inpstr[i].instr_string);
  }

}



	// * * * Initialization for all the stages of pipeline  * * * // 

		//------------------//


void init_stage_fetch_input(struct stage_info *fetch)

{

fetch->input_instr->PC = 0;
strcpy(fetch->input_instr->instr_string, " ");
fetch->input_instr->src_reg1 	= 0;
fetch->input_instr->src_reg1val	= 0;
fetch->input_instr->loaded_reg1val = false;
fetch->input_instr->src_reg2 	= 0;
fetch->input_instr->src_reg2val	= 0;
fetch->input_instr->loaded_reg2val = false;
fetch->input_instr->dest_reg 	= 0;
fetch->input_instr->dest_regval	= 0;
fetch->input_instr->tgt_mem_addr	= 0;
fetch->input_instr->tgt_mem_data	= 0;
fetch->input_instr->litrl		= 0;
fetch->input_instr->cycle_cnt	= 0;
fetch->input_instr->rob_commit = false;
fetch->input_instr->phy_reg	= -1;
strcpy(fetch->input_instr->instr, " ");

}



void init_stage_fetch_output(struct stage_info *fetch)
{

fetch->output_instr->PC = 0;
strcpy(fetch->output_instr->instr_string, " ");
fetch->output_instr->src_reg1 	= 0;
fetch->output_instr->src_reg1val= 0;
fetch->output_instr->loaded_reg1val = false;
fetch->output_instr->src_reg2 	= 0;
fetch->output_instr->src_reg2val= 0;
fetch->output_instr->loaded_reg2val = false;
fetch->output_instr->dest_reg 	= 0;
fetch->output_instr->dest_regval	= 0;
fetch->output_instr->tgt_mem_addr	= 0;
fetch->output_instr->tgt_mem_data	= 0;
fetch->output_instr->litrl		= 0;
fetch->output_instr->cycle_cnt	= 0;
fetch->output_instr->rob_commit = false;
strcpy(fetch->output_instr->instr, " ");
fetch->output_instr->phy_reg	= -1;

fetch->stalled = !true;


}


		//------------------//


void init_stage_decode_input(struct stage_info *decode)
{

decode->input_instr->PC = 0;
strcpy(decode->input_instr->instr_string, " ");
decode->input_instr->src_reg1 	= 0;
decode->input_instr->src_reg1val	= 0;
decode->input_instr->loaded_reg1val = false;
decode->input_instr->src_reg2 	= 0;
decode->input_instr->src_reg2val	= 0;
decode->input_instr->loaded_reg2val = false;
decode->input_instr->dest_reg 	= 0;
decode->input_instr->dest_regval	= 0;
decode->input_instr->tgt_mem_addr	= 0;
decode->input_instr->tgt_mem_data	= 0;
decode->input_instr->litrl		= 0;
decode->input_instr->cycle_cnt	= 0;
decode->input_instr->rob_commit = false;
decode->input_instr->phy_reg	= -1;
strcpy(decode->input_instr->instr, " ");

}

	

void init_stage_decode_output(struct stage_info *decode)
{

decode->output_instr->PC = 0;
strcpy(decode->output_instr->instr_string, " ");
decode->output_instr->src_reg1 	= 0;
decode->output_instr->src_reg1val= 0;
decode->output_instr->loaded_reg1val = false;
decode->output_instr->src_reg2 	= 0;
decode->output_instr->src_reg2val	= 0;
decode->output_instr->loaded_reg2val = false;
decode->output_instr->dest_reg 	= 0;
decode->output_instr->dest_regval	= 0;
decode->output_instr->tgt_mem_addr	= 0;
decode->output_instr->tgt_mem_data	= 0;
decode->output_instr->litrl		= 0;
decode->output_instr->cycle_cnt	= 0;
decode->output_instr->rob_commit = false;
decode->output_instr->phy_reg	= -1;
strcpy(decode->output_instr->instr, " ");

decode->stalled = !true;


}

		//------------------//


void init_stage_non_mul_exe_input(struct stage_info *non_mul_exe)
{

non_mul_exe->input_instr->PC = 0;
strcpy(non_mul_exe->input_instr->instr_string, " ");
non_mul_exe->input_instr->src_reg1	= 0;
non_mul_exe->input_instr->src_reg1val	= 0;
non_mul_exe->input_instr->loaded_reg1val = false;
non_mul_exe->input_instr->src_reg2	= 0;
non_mul_exe->input_instr->src_reg2val	= 0;
non_mul_exe->input_instr->loaded_reg2val = false;
non_mul_exe->input_instr->dest_reg	= 0;
non_mul_exe->input_instr->dest_regval	= 0;
non_mul_exe->input_instr->tgt_mem_addr	= 0;
non_mul_exe->input_instr->tgt_mem_data	= 0;
non_mul_exe->input_instr->litrl		= 0;
non_mul_exe->input_instr->cycle_cnt	= 0;
non_mul_exe->input_instr->rob_commit = false;
non_mul_exe->input_instr->phy_reg	= -1;
strcpy(non_mul_exe->input_instr->instr, " ");


}


void init_stage_non_mul_exe_output(struct stage_info *non_mul_exe)
{
	
non_mul_exe->output_instr->PC = 0;
strcpy(non_mul_exe->output_instr->instr_string, " ");
non_mul_exe->output_instr->src_reg1	= 0;
non_mul_exe->output_instr->src_reg1val	= 0;
non_mul_exe->output_instr->src_reg2	= 0;
non_mul_exe->output_instr->src_reg2val	= 0;
non_mul_exe->output_instr->dest_reg	= 0;
non_mul_exe->output_instr->dest_regval	= 0;
non_mul_exe->output_instr->tgt_mem_addr	= 0;
non_mul_exe->output_instr->tgt_mem_data	= 0;
non_mul_exe->output_instr->litrl	= 0;
non_mul_exe->output_instr->cycle_cnt	= 0;
non_mul_exe->output_instr->rob_commit = false;
non_mul_exe->output_instr->loaded_reg1val = false;
non_mul_exe->output_instr->loaded_reg2val = false;
non_mul_exe->output_instr->phy_reg	= -1;
strcpy(non_mul_exe->output_instr->instr, " ");

non_mul_exe->stalled = !true;


}

void init_stage_mul_exe1_input(struct stage_info *mul_exe1)
{

mul_exe1->input_instr->PC = 0;
strcpy(mul_exe1->input_instr->instr_string, " ");
mul_exe1->input_instr->src_reg1 	= 0;
mul_exe1->input_instr->src_reg1val	= 0;
mul_exe1->input_instr->src_reg2 	= 0;
mul_exe1->input_instr->src_reg2val	= 0;
mul_exe1->input_instr->dest_reg 	= 0;
mul_exe1->input_instr->dest_regval	= 0;
mul_exe1->input_instr->tgt_mem_addr	= 0;
mul_exe1->input_instr->tgt_mem_data	= 0;
mul_exe1->input_instr->litrl	= 0;
mul_exe1->input_instr->cycle_cnt	= 0;
mul_exe1->input_instr->loaded_reg1val = false;
mul_exe1->input_instr->loaded_reg2val = false;
mul_exe1->input_instr->rob_commit = false;
mul_exe1->input_instr->phy_reg	= -1;
strcpy(mul_exe1->input_instr->instr, " ");

}

void init_stage_mul_exe1_output(struct stage_info *mul_exe1)
{


mul_exe1->output_instr->PC = 0;
strcpy(mul_exe1->output_instr->instr_string, " ");
mul_exe1->output_instr->src_reg1 	= 0;
mul_exe1->output_instr->src_reg1val = 0;
mul_exe1->output_instr->src_reg2 	= 0;
mul_exe1->output_instr->src_reg2val	= 0;
mul_exe1->output_instr->dest_reg 	= 0;
mul_exe1->output_instr->dest_regval	= 0;
mul_exe1->output_instr->tgt_mem_addr= 0;
mul_exe1->output_instr->tgt_mem_data= 0;
mul_exe1->output_instr->litrl	= 0;
mul_exe1->output_instr->cycle_cnt	= 0;
mul_exe1->output_instr->loaded_reg1val = false;
mul_exe1->output_instr->loaded_reg2val = false;
mul_exe1->output_instr->rob_commit = false;
mul_exe1->output_instr->phy_reg	= -1;
strcpy(mul_exe1->output_instr->instr, " ");

mul_exe1->stalled = !true;

}


		//------------------//


void init_stage_mul_exe2_input(struct stage_info *mul_exe2)
{

mul_exe2->input_instr->PC = 0;
strcpy(mul_exe2->input_instr->instr_string, " ");
mul_exe2->input_instr->src_reg1 	= 0;
mul_exe2->input_instr->src_reg1val	= 0;
mul_exe2->input_instr->src_reg2 	= 0;
mul_exe2->input_instr->src_reg2val	= 0;
mul_exe2->input_instr->dest_reg 	= 0;
mul_exe2->input_instr->dest_regval	= 0;
mul_exe2->input_instr->tgt_mem_addr	= 0;
mul_exe2->input_instr->tgt_mem_data	= 0;
mul_exe2->input_instr->litrl	= 0;
mul_exe2->input_instr->cycle_cnt	= 0;
mul_exe2->input_instr->loaded_reg1val = false;
mul_exe2->input_instr->loaded_reg2val = false;
mul_exe2->input_instr->rob_commit = false;
mul_exe2->input_instr->phy_reg	= -1;
strcpy(mul_exe2->input_instr->instr, " ");

}


void init_stage_mul_exe2_output(struct stage_info *mul_exe2)
{

mul_exe2->output_instr->PC = 0;
strcpy(mul_exe2->output_instr->instr_string, " ");
mul_exe2->output_instr->src_reg1 	= 0;
mul_exe2->output_instr->src_reg1val = 0;
mul_exe2->output_instr->src_reg2 	= 0;
mul_exe2->output_instr->src_reg2val	= 0;
mul_exe2->output_instr->dest_reg 	= 0;
mul_exe2->output_instr->dest_regval	= 0;
mul_exe2->output_instr->tgt_mem_addr= 0;
mul_exe2->output_instr->tgt_mem_data= 0;
mul_exe2->output_instr->litrl	= 0;
mul_exe2->output_instr->cycle_cnt	= 0;
mul_exe2->output_instr->loaded_reg1val = false;
mul_exe2->output_instr->loaded_reg2val = false;
mul_exe2->output_instr->rob_commit = false;
mul_exe2->output_instr->phy_reg	= -1;
strcpy(mul_exe2->output_instr->instr, " ");

mul_exe2->stalled = !true;

}


		//-------------------//


void init_stage_div_exe1_input(struct stage_info *div_exe1)
{
div_exe1->input_instr->PC = 0;
strcpy(div_exe1->input_instr->instr_string, " ");
div_exe1->input_instr->src_reg1 	= 0;
div_exe1->input_instr->src_reg1val	= 0;
div_exe1->input_instr->src_reg2 	= 0;
div_exe1->input_instr->src_reg2val	= 0;
div_exe1->input_instr->dest_reg 	= 0;
div_exe1->input_instr->dest_regval	= 0;
div_exe1->input_instr->tgt_mem_addr	= 0;
div_exe1->input_instr->tgt_mem_data	= 0;
div_exe1->input_instr->litrl	= 0;
div_exe1->input_instr->cycle_cnt	= 0;
div_exe1->input_instr->loaded_reg1val = false;
div_exe1->input_instr->loaded_reg2val = false;
div_exe1->input_instr->rob_commit = false;
div_exe1->input_instr->phy_reg	= -1;
strcpy(div_exe1->input_instr->instr, " ");
}

void init_stage_div_exe1_output(struct stage_info *div_exe1)
{
div_exe1->output_instr->PC = 0;
strcpy(div_exe1->output_instr->instr_string, " ");
div_exe1->output_instr->src_reg1 	= 0;
div_exe1->output_instr->src_reg1val 	= 0;
div_exe1->output_instr->src_reg2 	= 0;
div_exe1->output_instr->src_reg2val	= 0;
div_exe1->output_instr->dest_reg 	= 0;
div_exe1->output_instr->dest_regval	= 0;
div_exe1->output_instr->tgt_mem_addr	= 0;
div_exe1->output_instr->tgt_mem_data	= 0;
div_exe1->output_instr->litrl		= 0;
div_exe1->output_instr->cycle_cnt	= 0;
div_exe1->output_instr->loaded_reg1val = false;
div_exe1->output_instr->loaded_reg2val = false;
div_exe1->output_instr->rob_commit = false;
div_exe1->output_instr->phy_reg	= -1;
strcpy(div_exe1->output_instr->instr, " ");

div_exe1->stalled = !true;

}

//

void init_stage_div_exe2_input(struct stage_info *div_exe2)
{
div_exe2->input_instr->PC = 0;
strcpy(div_exe2->input_instr->instr_string, " ");
div_exe2->input_instr->src_reg1 	= 0;
div_exe2->input_instr->src_reg1val	= 0;
div_exe2->input_instr->src_reg2 	= 0;
div_exe2->input_instr->src_reg2val	= 0;
div_exe2->input_instr->dest_reg 	= 0;
div_exe2->input_instr->dest_regval	= 0;
div_exe2->input_instr->tgt_mem_addr	= 0;
div_exe2->input_instr->tgt_mem_data	= 0;
div_exe2->input_instr->litrl		= 0;
div_exe2->input_instr->cycle_cnt	= 0;
div_exe2->input_instr->loaded_reg1val = false;
div_exe2->input_instr->loaded_reg2val = false;
div_exe2->input_instr->rob_commit = false;
div_exe2->input_instr->phy_reg	= -1;
strcpy(div_exe2->input_instr->instr, " ");
}

void init_stage_div_exe2_output(struct stage_info *div_exe2)
{
div_exe2->output_instr->PC = 0;
strcpy(div_exe2->output_instr->instr_string, " ");
div_exe2->output_instr->src_reg1 	= 0;
div_exe2->output_instr->src_reg1val 	= 0;
div_exe2->output_instr->src_reg2 	= 0;
div_exe2->output_instr->src_reg2val	= 0;
div_exe2->output_instr->dest_reg 	= 0;
div_exe2->output_instr->dest_regval	= 0;
div_exe2->output_instr->tgt_mem_addr	= 0;
div_exe2->output_instr->tgt_mem_data	= 0;
div_exe2->output_instr->litrl		= 0;
div_exe2->output_instr->cycle_cnt	= 0;
div_exe2->output_instr->loaded_reg1val = false;
div_exe2->output_instr->loaded_reg2val = false;
div_exe2->output_instr->rob_commit = false;
div_exe2->output_instr->phy_reg	= -1;
strcpy(div_exe2->output_instr->instr, " ");

div_exe2->stalled = !true;

}

//

void init_stage_div_exe3_input(struct stage_info *div_exe3)
{
div_exe3->input_instr->PC = 0;
strcpy(div_exe3->input_instr->instr_string, " ");
div_exe3->input_instr->src_reg1 	= 0;
div_exe3->input_instr->src_reg1val	= 0;
div_exe3->input_instr->src_reg2 	= 0;
div_exe3->input_instr->src_reg2val	= 0;
div_exe3->input_instr->dest_reg 	= 0;
div_exe3->input_instr->dest_regval	= 0;
div_exe3->input_instr->tgt_mem_addr	= 0;
div_exe3->input_instr->tgt_mem_data	= 0;
div_exe3->input_instr->litrl		= 0;
div_exe3->input_instr->cycle_cnt	= 0;
div_exe3->input_instr->loaded_reg1val = false;
div_exe3->input_instr->loaded_reg2val = false;
div_exe3->input_instr->rob_commit = false;
div_exe3->input_instr->phy_reg	= -1;
strcpy(div_exe3->input_instr->instr, " ");
}

void init_stage_div_exe3_output(struct stage_info *div_exe3)
{
div_exe3->output_instr->PC = 0;
strcpy(div_exe3->output_instr->instr_string, " ");
div_exe3->output_instr->src_reg1 	= 0;
div_exe3->output_instr->src_reg1val 	= 0;
div_exe3->output_instr->src_reg2 	= 0;
div_exe3->output_instr->src_reg2val	= 0;
div_exe3->output_instr->dest_reg 	= 0;
div_exe3->output_instr->dest_regval	= 0;
div_exe3->output_instr->tgt_mem_addr	= 0;
div_exe3->output_instr->tgt_mem_data	= 0;
div_exe3->output_instr->litrl		= 0;
div_exe3->output_instr->cycle_cnt	= 0;
div_exe3->output_instr->loaded_reg1val = false;
div_exe3->output_instr->loaded_reg2val = false;
div_exe3->output_instr->rob_commit = false;
div_exe3->output_instr->phy_reg	= -1;
strcpy(div_exe3->output_instr->instr, " ");

div_exe3->stalled = !true;

}

//

void init_stage_div_exe4_input(struct stage_info *div_exe4)
{
div_exe4->input_instr->PC = 0;
strcpy(div_exe4->input_instr->instr_string, " ");
div_exe4->input_instr->src_reg1 	= 0;
div_exe4->input_instr->src_reg1val	= 0;
div_exe4->input_instr->src_reg2 	= 0;
div_exe4->input_instr->src_reg2val	= 0;
div_exe4->input_instr->dest_reg 	= 0;
div_exe4->input_instr->dest_regval	= 0;
div_exe4->input_instr->tgt_mem_addr	= 0;
div_exe4->input_instr->tgt_mem_data	= 0;
div_exe4->input_instr->litrl		= 0;
div_exe4->input_instr->cycle_cnt	= 0;
div_exe4->input_instr->loaded_reg1val = false;
div_exe4->input_instr->loaded_reg2val = false;
div_exe4->input_instr->rob_commit = false;
div_exe4->input_instr->phy_reg	= -1;
strcpy(div_exe4->input_instr->instr, " ");
}

void init_stage_div_exe4_output(struct stage_info *div_exe4)
{
div_exe4->output_instr->PC = 0;
strcpy(div_exe4->output_instr->instr_string, " ");
div_exe4->output_instr->src_reg1 	= 0;
div_exe4->output_instr->src_reg1val 	= 0;
div_exe4->output_instr->src_reg2 	= 0;
div_exe4->output_instr->src_reg2val	= 0;
div_exe4->output_instr->dest_reg 	= 0;
div_exe4->output_instr->dest_regval	= 0;
div_exe4->output_instr->tgt_mem_addr	= 0;
div_exe4->output_instr->tgt_mem_data	= 0;
div_exe4->output_instr->litrl		= 0;
div_exe4->output_instr->cycle_cnt	= 0;
div_exe4->output_instr->loaded_reg1val = false;
div_exe4->output_instr->loaded_reg2val = false;
div_exe4->output_instr->rob_commit = false;
div_exe4->output_instr->phy_reg	= -1;
strcpy(div_exe4->output_instr->instr, " ");

div_exe4->stalled = !true;

}


		//------------------//


void init_stage_memory_input(struct stage_info *memory)
{

memory->input_instr->PC = 0;
strcpy(memory->input_instr->instr_string, " ");
memory->input_instr->src_reg1 		= 0;
memory->input_instr->src_reg1val	= 0;
memory->input_instr->src_reg2 		= 0;
memory->input_instr->src_reg2val	= 0;
memory->input_instr->dest_reg 		= 0;
memory->input_instr->dest_regval	= 0;
memory->input_instr->tgt_mem_addr	= 0;
memory->input_instr->tgt_mem_data	= 0;
memory->input_instr->litrl		= 0;
memory->input_instr->cycle_cnt	= 0;
memory->input_instr->loaded_reg1val = false;
memory->input_instr->loaded_reg2val = false;
memory->input_instr->rob_commit = false;
memory->input_instr->phy_reg	= -1;
strcpy(memory->input_instr->instr, " ");

}


void init_stage_memory_output(struct stage_info *memory)
{

memory->output_instr->PC = 0;
strcpy(memory->output_instr->instr_string, " ");
memory->output_instr->src_reg1 		= 0;
memory->output_instr->src_reg1val 	= 0;
memory->output_instr->src_reg2 		= 0;
memory->output_instr->src_reg2val	= 0;
memory->output_instr->dest_reg 		= 0;
memory->output_instr->dest_regval	= 0;
memory->output_instr->tgt_mem_addr	= 0;
memory->output_instr->tgt_mem_data	= 0;
memory->output_instr->litrl		= 0;
memory->output_instr->cycle_cnt	= 0;
memory->output_instr->loaded_reg1val = false;
memory->output_instr->loaded_reg2val = false;
memory->output_instr->rob_commit = false;
memory->output_instr->phy_reg	= -1;
strcpy(memory->output_instr->instr, " ");

memory->stalled = !true;

}


	//------------------//

void init_stage_wrtbck_input(struct stage_info *wrtbck)
{

wrtbck->input_instr->PC = 0;
strcpy(wrtbck->input_instr->instr_string, " ");
wrtbck->input_instr->src_reg1 		= 0;
wrtbck->input_instr->src_reg1val	= 0;
wrtbck->input_instr->src_reg2 		= 0;
wrtbck->input_instr->src_reg2val	= 0;
wrtbck->input_instr->dest_reg 		= 0;
wrtbck->input_instr->dest_regval	= 0;
wrtbck->input_instr->tgt_mem_addr	= 0;
wrtbck->input_instr->tgt_mem_data	= 0;
wrtbck->input_instr->litrl		= 0;
strcpy(wrtbck->input_instr->instr, " ");

}



void init_stage_wrtbck_output(struct stage_info *wrtbck)
{

wrtbck->output_instr->PC = 0;
strcpy(wrtbck->output_instr->instr_string, " ");
wrtbck->output_instr->src_reg1 		= 0;
wrtbck->output_instr->src_reg1val 	= 0;
wrtbck->output_instr->src_reg2 		= 0;
wrtbck->output_instr->src_reg2val	= 0;
wrtbck->output_instr->dest_reg 		= 0;
wrtbck->output_instr->dest_regval	= 0;
wrtbck->output_instr->tgt_mem_addr	= 0;
wrtbck->output_instr->tgt_mem_data	= 0;
wrtbck->output_instr->litrl		= 0;
strcpy(wrtbck->output_instr->instr, " ");

wrtbck->stalled =!true;

}


	// * * * Initialize data memory * * * //


void init_data_memory(struct data_memory *data)

{

data->base_addr = 0;

for(int i=0; i<4000; i++)
	{	
	data->data_val[i] = 0;		//Initialize data memory value to 0
	}
//data->data_val[220] = 220;
//data->data_val[152] = 333;
}



	// * * * Initialize registers * * * //

void init_registers(struct register_file *Reg)

{

for(int i=0; i<16; i++)
	{
	Reg->Regstr[i].regno = i;
	Reg->Regstr[i].regval = -1;	//-1= No value loaded 
	Reg->Regstr[i].status = 1;	//1=Valid; 0=Invalid
	}
}


	// * * * Initialize Physical Registers * * * //


void init_phy_registers(struct physical_register *phy_reg)
{

for(int i=0; i<32; i++)
	{
	phy_reg->Regstr[i].allocated = 0;
	phy_reg->Regstr[i].rename = 0;
	phy_reg->Regstr[i].status = 0;
	phy_reg->Regstr[i].value = 0;
	}	

}

	// * * * Initialize issue Queue * * * //

void init_issue_queue_input(struct issue_queue *I_Q, int i)
{

	I_Q->IQ[i]->input_instr->PC = 0;
	strcpy(I_Q->IQ[i]->input_instr->instr_string, " ");
	I_Q->IQ[i]->input_instr->src_reg1 	= 0;
	I_Q->IQ[i]->input_instr->src_reg1val	= 0;
	I_Q->IQ[i]->input_instr->src_reg2 	= 0;
	I_Q->IQ[i]->input_instr->src_reg2val	= 0;
	I_Q->IQ[i]->input_instr->dest_reg 	= 0;
	I_Q->IQ[i]->input_instr->dest_regval	= 0;
	I_Q->IQ[i]->input_instr->tgt_mem_addr	= 0;
	I_Q->IQ[i]->input_instr->tgt_mem_data	= 0;
	I_Q->IQ[i]->input_instr->litrl		= 0;
	I_Q->IQ[i]->input_instr->cycle_cnt	= 0; 
	I_Q->IQ[i]->iq_empty = true;
	I_Q->IQ[i]->input_instr->loaded_reg1val = false;
	I_Q->IQ[i]->input_instr->loaded_reg2val = false;
	strcpy(I_Q->IQ[i]->input_instr->instr, " ");	

}

/*
void init_issue_queue_output(struct issue_queue *I_Q)
{

for(int i=0; i<16; i++)
	{
	I_Q->IQ[i]->output_instr->PC = 0;
	strcpy(I_Q->IQ[i]->output_instr->instr_string, " ");
	I_Q->IQ[i]->output_instr->src_reg1 		= 0;
	I_Q->IQ[i]->output_instr->src_reg1val	= 0;
	I_Q->IQ[i]->output_instr->src_reg2 		= 0;
	I_Q->IQ[i]->output_instr->src_reg2val	= 0;
	I_Q->IQ[i]->output_instr->dest_reg 		= 0;
	I_Q->IQ[i]->output_instr->dest_regval	= 0;
	I_Q->IQ[i]->output_instr->tgt_mem_addr	= 0;
	I_Q->IQ[i]->output_instr->tgt_mem_data	= 0;
	I_Q->IQ[i]->output_instr->litrl		= 0;
	I_Q->IQ[i]->output_instr->cycle_cnt		= 0; 
	strcpy(I_Q->IQ[i]->output_instr->instr, " ");
	}

}
*/

	// * * * Initialize Load Store Queue * * * //

void init_load_store_queue_input(struct load_store_queue *L_S_Q, int i)
{

//for(int i=0; i<32; i++)
	//{	
	L_S_Q->LSQ[i]->input_instr->PC = 0;
	strcpy(L_S_Q->LSQ[i]->input_instr->instr_string, " ");
	L_S_Q->LSQ[i]->input_instr->src_reg1 		= 0;
	L_S_Q->LSQ[i]->input_instr->src_reg1val	= 0;
	L_S_Q->LSQ[i]->input_instr->src_reg2 		= 0;
	L_S_Q->LSQ[i]->input_instr->src_reg2val	= 0;
	L_S_Q->LSQ[i]->input_instr->dest_reg 		= 0;
	L_S_Q->LSQ[i]->input_instr->dest_regval	= 0;
	L_S_Q->LSQ[i]->input_instr->tgt_mem_addr	= 0;
	L_S_Q->LSQ[i]->input_instr->tgt_mem_data	= 0;
	L_S_Q->LSQ[i]->input_instr->litrl		= 0;
	L_S_Q->LSQ[i]->input_instr->cycle_cnt		= 0;
       	L_S_Q->LSQ[i]->lsq_empty = true;	
	L_S_Q->LSQ[i]->input_instr->loaded_reg1val = false;
	L_S_Q->LSQ[i]->input_instr->loaded_reg2val = false;
	strcpy(L_S_Q->LSQ[i]->input_instr->instr, " ");	
	//}

	
}

/*	
void init_load_store_queue_output(struct load_store_queue *L_S_Q)
{

for(int i=0; i<32; i++)
	{	
	L_S_Q->LSQ[i]->output_instr->PC = 0;
	strcpy(L_S_Q->LSQ[i]->output_instr->instr_string, " ");
	L_S_Q->LSQ[i]->output_instr->src_reg1 		= 0;
	L_S_Q->LSQ[i]->output_instr->src_reg1val	= 0;
	L_S_Q->LSQ[i]->output_instr->src_reg2 		= 0;
	L_S_Q->LSQ[i]->output_instr->src_reg2val	= 0;
	L_S_Q->LSQ[i]->output_instr->dest_reg 		= 0;
	L_S_Q->LSQ[i]->output_instr->dest_regval	= 0;
	L_S_Q->LSQ[i]->output_instr->tgt_mem_addr	= 0;
	L_S_Q->LSQ[i]->output_instr->tgt_mem_data	= 0; 
	L_S_Q->LSQ[i]->output_instr->litrl		= 0;
	L_S_Q->LSQ[i]->output_instr->cycle_cnt	= 0; 
	strcpy(L_S_Q->LSQ[i]->output_instr->instr, " ");	
	}

	
}
*/

	// * * * Initialize Re Order Buffer * * * //

void init_re_order_buff_input(struct re_order_buff *R_O_B, int i)
{

	R_O_B->ROB[i]->input_instr->PC = 0;
	strcpy(R_O_B->ROB[i]->input_instr->instr_string, " ");
	R_O_B->ROB[i]->input_instr->src_reg1 		= 0;
	R_O_B->ROB[i]->input_instr->src_reg1val	= 0;
	R_O_B->ROB[i]->input_instr->src_reg2 		= 0;
	R_O_B->ROB[i]->input_instr->src_reg2val	= 0;
	R_O_B->ROB[i]->input_instr->dest_reg 		= 0;
	R_O_B->ROB[i]->input_instr->dest_regval	= 0;
	R_O_B->ROB[i]->input_instr->tgt_mem_addr	= 0;
	R_O_B->ROB[i]->input_instr->tgt_mem_data	= 0;
	R_O_B->ROB[i]->input_instr->litrl		= 0;
	R_O_B->ROB[i]->input_instr->cycle_cnt		= 0;
	R_O_B->ROB[i]->rob_empty = true;
	R_O_B->ROB[i]->input_instr->loaded_reg1val = false;
	R_O_B->ROB[i]->input_instr->loaded_reg2val = false;
	R_O_B->ROB[i]->input_instr->rob_commit = false;	
	strcpy(R_O_B->ROB[i]->input_instr->instr, " ");	
	
}
	
 /*
void init_re_order_buff_output(struct re_order_buff *R_O_B)
{

for(int i=0; i<32; i++)
	{	
	R_O_B->ROB[i]->output_instr->PC = 0;
	strcpy(R_O_B->ROB[i]->output_instr->instr_string, " ");
	R_O_B->ROB[i]->output_instr->src_reg1 		= 0;
	R_O_B->ROB[i]->output_instr->src_reg1val	= 0;
	R_O_B->ROB[i]->output_instr->src_reg2 		= 0;
	R_O_B->ROB[i]->output_instr->src_reg2val	= 0;
	R_O_B->ROB[i]->output_instr->dest_reg 		= 0;
	R_O_B->ROB[i]->output_instr->dest_regval	= 0;
	R_O_B->ROB[i]->output_instr->tgt_mem_addr	= 0;
	R_O_B->ROB[i]->output_instr->tgt_mem_data	= 0;
	R_O_B->ROB[i]->output_instr->litrl		= 0;
	R_O_B->ROB[i]->output_instr->cycle_cnt		= 0;
	strcpy(R_O_B->ROB[i]->output_instr->instr, " ");	
	}
	

}
*/

	// * * * Initialize the Instruction cycles * * * //
	
void init_rename_table(struct rename_table renam_tbl)
{

for(int i=0; i<16; i++)
	{
	//renam_tbl.arf[i] = -1;
	renam_tbl.prf[i] = -1;
	renam_tbl.sts[i] = false;		
	}

}


	// * * * Initialize the Instruction cycles * * * //

void init_stat(struct stats *num)

{

num->cycle = 0;
num->instr_no =1;

}

	// * * * Initialize the flag values * * * //

void init_flags(struct Flags *flag)

{

flag->zero = 0;
flag->zero_status = STATUS_VALID;
flag->carry = 0;
flag->carry_status = true;
flag->neg = 0;
flag->neg_status = true;

}


	// * * * Print register values * * * //

void print_register_values(struct register_file *Reg)

{
printf("\n");
	
for(int i=0; i<16; i++)
	{
		
	printf("R%d :", Reg->Regstr[i].regno); 
	printf("%d ", Reg->Regstr[i].regval);

	if (Reg->Regstr[i].status)
		{
		printf(":Valid");
		}
	else
		{
		printf(":Invalid");
		}
	printf("\t\t");
	if(i==3 || i==7 || i==11 || i==15)
		{
		printf("\n");
		}
	}
printf("\n");
}

	// * * * Print data memory values * * * //

void print_data_values(struct data_memory *data)

{

float j;	
for(int i=0; i<100; i++)
	{
	printf("D[%d] = ", (i*4));
	printf("%d\t\t", data->data_val[i*4]);
		if((j=i%5) == 0)	printf("\n");
		
	}
	printf("\n");
}

