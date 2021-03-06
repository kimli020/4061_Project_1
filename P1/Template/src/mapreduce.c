#include "mapreduce.h"
#include "sys/stat.h"

int main(int argc, char *argv[]) {

	if(argc < 4) {
		printf("Less number of arguments.\n");
		printf("./mapreduce #mappers #reducers inputFile\n");
		exit(0);
	}
	



	// ###### DO NOT REMOVE ######
	int nMappers 	= strtol(argv[1], NULL, 10);
	int nReducers 	= strtol(argv[2], NULL, 10);
	char *inputFile = argv[3];
	
	//check if input file exists:
	int result = access(inputFile, F_OK); 
  	if( result == -1 ) {
   		printf("Illegal: input file must exist \n");
  		exit(1);
  	}
	
	 //Check if file is empty by getting the position of EOF. HAS TO COME BEFORE CALL TO bookkeepingCode()!
	 FILE *fp; 
	 fp = fopen(inputFile,"r");
	 fseek(fp, 0, SEEK_END); // go to EOF
	 if (ftell(fp) == 0)	//if current position is 0, file has nothing, empty
	  {
	 	 printf("Your input is void and hollow, not worth my time. \n");
	 	 fclose(fp);
	 	 exit(1);
	  }else {

    		fclose(fp);
   
   	} 
	
	// ###### DO NOT REMOVE ######
	bookeepingCode();
	

	//check for negative input #'s
	if ( (nMappers <= 0) || (nReducers <= 0) )
  	{
		printf("Illegal: # of file must be greater than 0 \n");
		exit(1);
   	}




	if(nMappers < nReducers)
	{
		printf("Illegal: nReducers > nMappers \n");
		exit(1);
	}

	// ###### DO NOT REMOVE ######
	pid_t pid = fork();
	if(pid == 0){
		//send chunks of data to the mappers in RR fashion
		sendChunkData(inputFile, nMappers);
		exit(0);
	}
	sleep(1);

	//Check if file is empty by getting the position of EOF
	//Issue: throws error on the terminal execution AFTER the execution with empty file: Error occured for ENDACKRECV: Identifier removed
	// FILE *fp = fopen(argv[3],"r");
	// fseek(fp, 0, SEEK_END); // go to EOF
	// if (ftell(fp) == 0)	//if current position is 0, file has nothing, empty
	//  {
	// 	 printf("Your input is void and hollow, not worth my time. \n");
	// 	 fclose(fp);
	// 	 return 0;
	//  }
	//  else
	//  {
	// 	 fclose(fp);
	//  }

	// Method 2: using sys/stat.h library
	// Still throws error?
	// struct stat st;
	//
	// if (stat(inputFile, &st) != 0)
	// {
	// 		return -1;	//error
	// }
	// else if (st.st_size == 0)
	// {
	// 		printf("File is empty. \n");
	// 		return 1;	//error: file empty
	// }
	// printf("file size: %zd\n", st.st_size);

	// To do
	// spawn mappers processes and run 'mapper' executable using exec
	pid_t pid2;
	for(int i=0; i < nMappers; i++)
	{
		pid2 = fork();
		if(pid2==0)	//child process
		{
			//Send chunks of data to mappers in RR fashion
			char *binaryPath = "./mapper";
			char str[2];
			sprintf(str, "%d", i+1);
			char *args[] = {binaryPath, str, '\0'};
			int x = execvp(binaryPath, args);
		}

		if(pid2 > 0)	//parent process
		{ /*do nothing for now*/ }
		else //error
		{
			perror("fork problem in mapreduce.c - map");
			exit(-1);
		}
	}

	// To do
	// wait for all children to complete execution
	int status;
	while ((pid = wait(&status)) > 0); //wait for ALL processes
	sleep(1);
	printf("\nDone map.\n");


	// ###### DO NOT REMOVE ######
    // shuffle sends the word.txt files generated by mapper
    // to reducer based on a hash function
	pid = fork();
	if(pid == 0){
		shuffle(nMappers, nReducers);
		exit(0);
	}
	sleep(1);


	// To do
	// spawn reducer processes and run 'reducer' executable using exec
	pid_t pid3;
	for(int i=0; i < nReducers; i++)
	{
		pid3 = fork();
		if(pid3==0)	//child process
		{
			//Send chunks of data to reducers in RR fashion
			char *binaryPath = "./reducer";
			char str[2];
			sprintf(str, "%d", i+1);
			char *args[] = {binaryPath, str, '\0'};
			int y = execvp(binaryPath, args);
		}

		if(pid3 > 0)	//parent process
		{ /*do nothing for now*/ }
		else //error
		{
			perror("fork problem in mapreduce.c - reduce");
			exit(-1);
		}
	}

	// To do
	// wait for all children to complete execution
	int status2;
	while ((pid3 = wait(&status2)) > 0); //wait for ALL processes
	sleep(1);
	printf("\nDone reduce.\n");

	return 0;
}
