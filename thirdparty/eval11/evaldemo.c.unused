/*
 EVALDEMO.C  Version 1.1

 Program to demonstrate evaluateExpression
 Copyright (c) 1996 - 1999 Parsifal Software. All Rights
 Reserved.

 Usage: evaldemo <filename>\n where the file holds
 expressions to be evaluated, with optional comments.

 An example main program which simply reads a file into a string
 in memory and calls evaluateExpression to evaluate the
 expressions in the string and store the results in a simple
 symbol table. It then prints out the contents of the symbol
 table to stdout in a form which allows it to be read as input
 again, should that be desired for initialization purposes.
 Finally, if there was an error, it prints a diagnostic message
 on stderr.

 For further information about this program, contact
   Parsifal Software
   http://www.parsifalsoft.com
   info@parsifalsoft.com
   1-800-879-2755, Voice/Fax 1-508-358-2564
   P.O. Box 219
   Wayland, MA 01778
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "evaldefs.h"

/* Main Program */

int main(int argc, char *argv[]) {
  int i;
  FILE *input;
  long fileLength;
  size_t stringLength;
  int errorFlag;
  char *expressionString;

  /* Check for enough arguments */
  if (argc != 2) {
    printf("Usage: evaldemo <filename>\n");
    return 1;
  }

  /* Open input file */
  input = fopen(argv[1],"r");
  if (input == NULL) {
    printf("Cannot open %s\n", argv[1]);
    return 2;
  }

  /* find out how big the file is */
  if (fseek(input, SEEK_SET, SEEK_END)) {
    printf("Strange problems with %s\n", argv[1]);
    return 3;
  }
  fileLength = ftell(input);
	if (fileLength < 0 ) {    // -1L is error return
    printf("Error getting file length (%d) of %s\n", fileLength, argv[1]);
    return 4;
  }

  /* fseek to beginning of file */
  if (fseek(input, 0, SEEK_SET)) {
    printf("Strange problems with %s\n", argv[1]);
    return 5;
  }

  /* Allocate storage for input string */
  expressionString = (char*)malloc(fileLength + 1);
  if (expressionString == NULL) {
    printf("Insufficient memory\n");
    return 6;
  }

  /* read file */
  stringLength = fread(expressionString, 1, (unsigned)fileLength, input);
  if (stringLength == 0) {
    printf("Unable to read %s\n", argv[1]);
		free(expressionString);
		fclose(input);
    return 7;
  }
  expressionString[stringLength] = 0;             // Terminate string with null

  /* evaluate expressions */
  errorFlag = evaluateExpression(expressionString);     // Call parser function
  free(expressionString);
  fclose(input);

  /* print values of variables */
  for (i = 0; i < nVariables; i++) printf("%s = %g;\n",
                                           variable[i].name,
                                           variable[i].value);
  /* check for error */
  if (errorFlag) fprintf(stderr,"File %s: %s at line %d, column %d\n",
                            argv[1],
                            errorRecord.message,
                            errorRecord.line,
                            errorRecord.column);
  /* done */
  return 0;
}
/*  End of EVALDEMO.C */
