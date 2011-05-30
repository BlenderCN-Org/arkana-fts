// DaoMain.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include"stdio.h"
#include"stdlib.h"
#include"string.h"


#include"dao.h"
#include"signal.h"

DAO_INIT_MODULE

static DaoVmSpace *vmSpace = NULL;

static void DaoSignalHandler( int sig )
{
	DaoVmSpace_Stop( vmSpace, 1);
    printf( "keyboard interrupt...\n" );
}


int main(int argc, char* argv[])
{
	DString *opts, *args;
	int i, k, idsrc;

	vmSpace = DaoInit();

	opts = DString_New(1);
	args = DString_New(1);


	idsrc = -1;
	for(i=1; i<argc; i++){
		/* also allows execution of script files without suffix .dao */
		if( argv[i][0] != '-' ){
			idsrc = i;
			break;
		}
	}

	k = idsrc;
	if( k < 0 ) k = argc;

	for(i=1; i<k; i++ ){
		DString_AppendMBS( opts, argv[i] );
		DString_AppendMBS( opts, " " );
	}

	DaoVmSpace_ParseOptions( vmSpace, opts );

	if( idsrc >= 0 ){
		for(i=idsrc; i<argc; i++ ){
			DString_AppendMBS( args, argv[i] );
			DString_AppendChar( args, '\0' );
		}
	}else if( argc==1 ){
		DString_AppendChar( opts, '\0' );
		DString_AppendMBS( opts, "-vi" );
		DaoVmSpace_ParseOptions( vmSpace, opts );
	}
	if( strstr( DString_GetMBS( opts ), "v" ) ){
		printf( "\n  A simple shell for the Dao Virtual Machine.\n" );
		printf( "  Copyright(C) 2006-2010, Fu Limin.\n" );
		printf( "  This shell is distributed under GNU General Public License.\n" );
	}
	if( DaoVmSpace_GetOptions( vmSpace ) & DAO_EXEC_INTERUN )
		signal( SIGINT, DaoSignalHandler );

	if( ! DaoVmSpace_RunMain( vmSpace, args ) ) return 1;
	DString_Delete( opts );
	DString_Delete( args );
	DaoQuit();
	return 0;
}

