// 测试open bios中的栈表信息
/* tag: defines the stacks, program counter and ways to access those
 *
 * Copyright (C) 2003 Patrick Mauritz, Stefan Reinauer
 *
 * See the file "COPYING" for further information about
 * the copyright and warranty status of this work.
 */


#include "stack.h"
#define FMT_CELL_x "#lx"
#define printk(fmt) printf(fmt)
#define dstacksize 512
int dstackcnt = 0;
cell dstack[dstacksize];

#define rstacksize 512
int rstackcnt = 0;
cell rstack[rstacksize];

/* Rstack value saved before entering forth interpreter in debugger */
int dbgrstackcnt = 0;


void printdstack(void)
{
	int i;
	printf("dstack:");
	for (i = 0; i <= dstackcnt; i++) {
		printf(" 0x%" FMT_CELL_x , dstack[i]);
	}
	printf("\n");
}

void printrstack(void)
{
	int i;
	printf("rstack:");
	for (i = 0; i <= rstackcnt; i++) {
		printf(" 0x%" FMT_CELL_x , rstack[i]);
	}
	printf("\n");
}



void
push_str( const char *str )
{
	PUSH( pointer2cell(str) );
	PUSH( str ? strlen(str) : 0 );
}


int main(int argc, char * argv[])
{

	int i = 0;

	printf("===========Start dump stack push/pop====================\n");
	printdstack();

	for(i=0; i<3; ++i){
		printf("push %#x\n",i);
		PUSH(i);
	}
	printdstack();
	for(i=0; i<3; ++i){
		printf("pop %#lx\n",POP());
	}
	printdstack();


	printf("===========Start dump stack str====================\n");

	char * str = "hello world\n";
	for(i=0; i<3; ++i){
		printf("push %p\n",str);
		push_str(str);
	}
	printdstack();

	return 0;
}

