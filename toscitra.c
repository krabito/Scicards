static char *toscitrans= "@(#) $Revision: 1.01 ";
/************************************************************
 * toscitrans.c is used to replace the scicards node name   *
 * in the scicards.xref file with the signal name given in  *
 * the schematic.  Created by K. Rabito                     *
 *                                                          *
 *  INPUTS: SCICARDS.XRE   node name cross reference list.  *
 *          SCICARDS.NET   scicards schematic netlist.      *
 *                                                          *
 *  OUTPUT: SCICARDS.NET   scicards schematic netlist with  *
 *                         signal names replacing scicard   *
 *                         node names.                      *
 *             ERROR.LOG   error file created if there were *
 *                         errors during translation.       *
 *                                                          *
 *          SCI_NET.BAK    original scicards.net file.      *
 *                                                          *
 *  Note: This program is intended to be portable to the    *
 *        UNIX environment.  In the UNIX environment the    *
 *        input file "scicards.xre" referenced above is     *
 *        called "scicards.xref".                           *
 *                                                          *
 *  Revision  |  Date      |  Description                   *
 * -------------------------------------------------------- *
 *  1.0       | 5/2/1990   |  Initial Release               *
 * -------------------------------------------------------- *
 *  1.01      | 5/17/1990  |  Add error.log for conversion  *
 *            |            |  errors.                       *
 ************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <alloc.h>       /* malloc.h in the Unix environment */
#include <string.h>
#include <sys\stat.h>

#define SPACE   ((char)0x20)
#define NINE    ((char)0x39)
#define ZERO    ((char)0x30)
#define N       ((char)0x4E)
#define NODECOL  3
#define CONNCOL  4

      /*  Typedef's used in the program */

typedef char snode[6];    /* typedef for scicard node name, max of 6 */
typedef char signam[16];  /* typedef for signal name, max of 16 */
typedef struct {
	       signam schname;
	       snode scinode;
	       } netname;

       /* Define global file pointers. */

FILE *infile, *dupfile, *outfile;

	/* Function declarations */

/***********************************************
 Function "quit".  Apon read errors, puts files
 back in orginal state and closes files.
 ***********************************************/
 void quit(errorstatus)
 int errorstatus;
 {
 switch (errorstatus) {
	case 1: fclose(infile);
		break;
	case 2: fclose(infile);
		fclose(dupfile);
		break;
	case 3: fclose(outfile);
		fclose(dupfile);
		break;
 } /* End of switch */
 exit(0);
 } /* END of function quit */

/************************************************************
 Function "header".
 Tests to see if the linebuffer is a valid signal name line.It
 ignores header lines from scicards cross reference files
 ( scicards.xre ).  Function header returns a value of 1 if
 there is a valid line.
 ************************************************************/

int header(lineptr)
 char *lineptr;
{
 char *tstline;
 int HCNT;     /* tstchar; */
 int VALIDLINE, VALIDCHAR;   /* 1 = true */

 VALIDLINE = VALIDCHAR = 1;
 tstline = lineptr;
 HCNT = strlen(tstline) - 2;
  if (!((tstline[HCNT] >= ZERO ) && ( tstline[HCNT] <= NINE))) {
       VALIDLINE = 0;
      }
 else {
      while ((VALIDCHAR == 1 ) && ( tstline[HCNT] != SPACE )) {
      if((tstline[HCNT] == N) || (!(tstline[HCNT] < ZERO) && !(tstline[HCNT] > NINE))) {
	    VALIDCHAR = 1;
	    VALIDLINE = 1;
	    HCNT--;
	    }
       else {
	    VALIDCHAR = 0;
	} /* End test for N or 0-9 */
      } /* End of while loop */
 } /* End test for invalid line type. */
 return(VALIDLINE);
 } /* End function 'header' */

/************************************************************
 Function "linecnt".
 Determines the line count in scicards.xre.
 If file cannot be open, function linecnt returns a value of '-1'.
 ************************************************************/

int linecnt(fname)
 char *fname;
{
 int LCNT = 0;
 char linebfr[80];     /* file scicards.xre has a string max of 80 */

 if ((infile = fopen( fname, "r")) != NULL ){
      while ( fgets(linebfr, sizeof(linebfr), infile)) {
      LCNT++;
      } /* end of while */
      }
 else {
      LCNT = -1;  /* returns an error, file not found */
      }
 fclose(infile);  /* close file for safety */
 return(LCNT);
} /* End of linecnt */

/***********************************************
 Function "split".
 After a valid line is read in from disk, function split
 will split the line into a struct of signal name and
 node name.  If error returns a -1.
 ***********************************************/

int split(node, slength, lpos, getname, nodemax)
 char *node;
 int slength, lpos;
 netname *getname;
 int *nodemax;
{
 int startcnt, endcnt; /* start and end of characters to be copied. */
 int validnode = 1;    /* true if character is N or 0 .. 9 */
 int cntr, gposition;
 gposition = lpos;
 endcnt = slength - 2;
 startcnt = endcnt;
 while ((validnode == 1 ) && ( node[startcnt] != SPACE )) {
  if((node[startcnt] == N) || (!(node[startcnt] < ZERO) && !(node[startcnt] > NINE))) {
      validnode = 1;
      if( node[startcnt] == N ){ /* removes SPACE before N in node name. */
	 cntr = startcnt;
	 }
      startcnt--;
      }
 else {
      validnode = 0;
      printf("INVALID SCICARDS NODE NAME AT \n\n\t%s\n",node);
      printf("Check xref file and net file for improper names.\n");
      return(-1);
   } /* End if test for N or 0-9 */
 } /* End of while loop */
 if (validnode == 1) {            /* copies scicards node name. */
     strncpy((getname + gposition) -> scinode, node + cntr, (slength - cntr - 1));
     if( *nodemax < strlen((getname + gposition) -> scinode)) {
       *nodemax = strlen((getname + gposition) -> scinode);
       } /* END of if, Gets max string length of scinode. Used to */
	 /* adjust string length */
    } /* End if */
/* Get signal name and put into structure. */
 while ( node[startcnt] == SPACE ) {
	startcnt--;
 } /* End of while loop */
 strncpy((getname + gposition) -> schname, node , startcnt + 1);
 gposition++;
 return(gposition);
 } /* end of split function */

/***********************************************
 Function "goodline".  Checks to see if line is
 not over 80 characters.  If over 80 returns 0.
 ***********************************************/
 int goodline( xline, lc, openfile)
 char *xline;
 int lc;
 char *openfile;
 {
 int gline = 1;
 if ( xline[lc-1] != '\n') {
   printf("\n\nERROR\nLine size greater than 80 characters ");
   printf("at\n\n%s\nin file %s\n\n",xline, openfile);
	  printf("Check line length!  To long.\n");
	  gline = 0;
	  } /* end if */
 return(gline);
 } /* END of function goodline */

/***********************************************
 Function "replace".
 After a line is read from scicards.net and is
 a valid node or connector line. Then function
 replace will replace the scicards node name
 with the associate signal name store in system
 memory.  Returns a 0 if 'N' is not found after
 the third SPACE on node strings, after the fourth
 SPACE on connector strings.
 ***********************************************/

int replace(node, nettype, numline, xrefname, snodemax)
 char *node, nettype;  /* node - string read in file, nettype node type. */
 int numline;          /* number of lines read in in xrefile. */
 netname *xrefname;    /* xref file structure stored in memory. */
 int snodemax; /* Maximum node string length. */
{
 int startcnt, endcnt; /* start and end of characters to be copied. */
 static char searchn[6];
 char newstring[40];
 char chkstring[40];
 char endstring[7];
 char errorfile[13];
 int slen, siglen, addret;
 static int gposition;
 int test;
 int status = 1; /* Status of 1 returns no errors in transfer. */
 int chkspace = 0;
 FILE *outerror;
 strcpy(errorfile,"error.log");
 slen = strlen(node) - 2; /* subtract 2 to remove '\n' and NULL. */
 startcnt = endcnt = 0;
 for(test = 0; test < 41;test++){ /* for loops are needed for the */
    newstring[test] = '\0';       /* generic UNIX C compiler to NULL */
    chkstring[test] = '\0';       /* the strings out. */
    }
 for(test = 0; test < 8;test++){
    endstring[test] = '\0';
    }
 strcpy(searchn," ");     /* clear searchn. */
 switch (nettype) {
    case 'N':             /* find scinode name to be replaced. */
	while((endcnt < slen ) && (chkspace < NODECOL)){
	     if(node[endcnt] == SPACE)chkspace++;
	     endcnt++;
	     startcnt = endcnt;
	 } /* END while check for third space. */
	 if(node[endcnt] == N){
	   strncpy(searchn, (node + startcnt), snodemax);
	   strncpy(newstring, node, startcnt );
	   while((strcmp(searchn, (xrefname + gposition) -> scinode) != 0) && (gposition < numline)) {
	      gposition++;
	   } /* END of while searchn */
	   if(gposition == numline){
	     status = -1;
	     if ((outerror = fopen(errorfile, "a")) == NULL) {   /* open error file */
		 printf("Could not open %s for error logging.\n");
		 }
	      else {
		    fputs("Could not find signal name for:\t",outerror);
		    fputs(node,outerror);
		    fclose(outerror);
		   }
	   } /* END if errfile fails to open. */
	  if( strlen(searchn) > strlen((xrefname + gposition) -> schname)) {
	    siglen = strlen(searchn);
	    }
	  else siglen = strlen((xrefname + gposition) -> schname);
	  strncat(newstring, (xrefname + gposition) -> schname, siglen);
	  addret = strlen(newstring);
	  if( newstring[addret - 1] != '\n' )strcat(newstring," \n");
	  strcpy(node,newstring);
	  }
	 else  {
	       status = 0;
	       }
	break;
    case 'C':  /* find scinode name to be replaced in connector field. */
	while((endcnt < slen ) && (chkspace < CONNCOL)){
	     if(node[endcnt] == SPACE)chkspace++;
	     endcnt++;
	     startcnt = endcnt;
	 } /* END while check for fourth space. */
	 strncpy(newstring,node,startcnt);
	 if(node[endcnt] == N){  /* Check for 'N' after fourth SPACE. */
	   while((endcnt < slen ) && (node[endcnt] != SPACE)){
		 endcnt++;
	      } /* END while. */
	   strncpy(endstring, node + endcnt, (strlen(node) - endcnt + 1));
	   strncpy(searchn, (node + startcnt), snodemax);
	   while((strcmp(searchn, (xrefname + gposition) -> scinode) != 0) && (gposition < numline)) {
		 gposition++;
	      } /* END of while searchn */
	   if(gposition == numline){
	     status = -1;
	     if ((outerror = fopen(errorfile, "a")) == NULL) {   /* open error file */
		 printf("Could not open %s for error logging.\n");
		 }
	      else {
		    fputs("Could not find signal name for:\t",outerror);
		    fputs(node,outerror);
		    fclose(outerror);
		   }
	   } /* END if errfile fails to open. */
	   siglen = strlen((xrefname + gposition) -> schname);
	   strncpy(newstring + startcnt, (xrefname + gposition) -> schname, siglen);
	   slen = startcnt + siglen;
	   strncpy(newstring + slen, endstring, strlen(node) - endcnt);
	   test = 0;
	   while(newstring[test] != '\n')test++;
	   strncpy(chkstring, newstring, test + 1);
	   strcpy(node, chkstring);  /* chkstring has any extra characters */
	   }                         /* removed after '\n'. */
	 else {
	      status = 0;
	      }
      break;
      } /* END switch */
gposition = 0;
return(status);
} /* END function replace */

/***********************************************
 Function "rd_only".  Changes sci_net.bak to read
 only.  If program is interrupted during sci_net.bak
 being open for read, file is deleted.
 ***********************************************/
 void rd_only(openfile)
 char *openfile;
{
 int status;
 status = chmod(openfile, S_IREAD);
 if(status < 0) {
     printf("Could not make file %s read only!\n",openfile);
 }
} /* END function rd_only. */

/***********************************************
 Function "rd_wr".  Changes sci_net.bak to be writable
 after being read only during the replacing function.
 ***********************************************/
 void rd_wr(openfile)
 char *openfile;
{
 int status;
 status = chmod(openfile, S_IREAD | S_IWRITE);
 if(status < 0) {
     printf("Could not make file %s read only!\n",openfile);
   }
} /* END function rd_wr. */

/***********************************************
 Function "workdisp". Displays flashing working.
 ***********************************************/
 void workdisp()
{
 printf("working");
 printf("\b\b\b\b\b\b\b");
 printf("       ");
 printf("\b\b\b\b\b\b\b");
} /* END function workdisp */

/********************************************************
 Function "_adjust" will adjust the sigxref structure
 stored in memory to have the same length in characters.
 This prevents when comparing N40 to N401 to be the same.
 *********************************************************/
 void _adjust(strname, scimax, lastpos)
 netname *strname;
 int *scimax;  /* max scnode length. */
 int lastpos;  /* last pointer to scixref structure in memory. */
{
 int position;
 int chrcnt;
 int diff;
 for(position = 0; position <= lastpos; position++){
    diff = *scimax - strlen((strname + position) -> scinode);
    for(chrcnt = 0; chrcnt < diff; chrcnt++){
       strcat((strname + position) -> scinode, " ");
       } /* END for that appends spaces to end of scinode string. */
    } /* END for that increments memory pointer in scixref structure. */
} /* END function _adjust */

/****************************************************/

main()  /* toscicards */
{
 typedef char xrefstring[81];  /* typedef for input string in scicards.xre */
 static netname *scixref;
 char xrefile[13], netfile[13], backfile[13]; /* File character strings. */
 xrefstring nodestring;
 int *scinodemax, rtst = 1;  /* *scinodemax - max snode length, rtst - replace tst. */
 int converror = 0; /* conversion error flag. Set to 1 if error. */
 int linei = 0, ic, mem_size = 0;
 strcpy(xrefile,"scicards.xref");
 strcpy(netfile,"scicards.net");
 strcpy(backfile,"sci_net.bak");
 scinodemax =(int *) malloc(sizeof(int));
 *scinodemax = 0;
 if ((mem_size = linecnt(xrefile)) < 1 ) {
    printf("\nFile %s empty or does not exist.  Exiting!\n",xrefile);
    exit(1);
    }
 scixref = (netname *) calloc(mem_size,sizeof(netname));
 if ( scixref == NULL ) {
     printf("\n ERROR! Not enough memory for this size schematic.  Exiting!\n");
     exit(1);
     }
 printf("\nReading %s file - ",xrefile);
 infile = fopen(xrefile, "r");
 while ( fgets(nodestring,sizeof(nodestring),infile)) {
       ic = strlen(nodestring);
       if ( goodline(nodestring, ic, xrefile) == 0 ) quit(1);
     /*  workdisp(); */
       if ( header(nodestring) == 1 ) {
	  linei = split(nodestring, ic, linei, scixref, scinodemax);
	  if (linei < 0 ) quit(1);
	  } /* end if */
	} /* end while */
 fclose(infile);
 printf("DONE\n");
 _adjust(scixref, scinodemax, linei);
/***********************************************************
*    Read in scicards.net file and replace scinode name    *
*    with the signal name in file.  Create sci_net.bak     *
*    which is a copy of the original scicards.net          *
***********************************************************/
 if ((infile = fopen(netfile, "r")) == NULL) {
    printf("Could not open %s, EXITING",netfile);
    exit(0);
    }
 if ((dupfile = fopen(backfile, "w")) == NULL) {
    printf("Could not open %s, EXITING",backfile);
    exit(0);
    }
 printf("\nCreating backup file %s - ",backfile);
 while ( fgets(nodestring,sizeof(nodestring),infile)) {
       ic = strlen(nodestring);
    /*   workdisp(); */
       if ( goodline(nodestring, ic, netfile) == 0 ) quit(2);
       fputs(nodestring,dupfile);
       } /* end while */
 fclose(dupfile);
 fclose(infile);
 printf("DONE\n");
 rd_only(backfile);
 if ((outfile = fopen(netfile, "w")) == NULL) {  /* open net file for write */
    printf("Could not open %s, EXITING",netfile);
    exit(0);
    }
 if ((dupfile = fopen(backfile, "r")) == NULL) {   /* open bak file for read */
    printf("Could not open %s, EXITING",backfile);
    exit(0);
    }
 printf("\nReplacing Node Names with signal names in %s - ",netfile);
 while ( fgets(nodestring,sizeof(nodestring),dupfile)) {
       ic = strlen(nodestring);
       workdisp();
       switch (nodestring[0]) {
	case 'C' : rtst = replace(nodestring, *nodestring, linei, scixref, *scinodemax, converror);
		   if(rtst == 0){
		      printf("\nConversion ERROR!  Check file ");
		      printf("%s, may have been previously converted.\n",netfile);
		      printf("Rename %s to %s and remove %s before restarting!\n",backfile,netfile,backfile);
		      quit(3);
		      }
		   if(rtst == -1)converror = 1;
		   fputs(nodestring,outfile);        /* connector */
		   break;
	case 'N' : rtst = replace(nodestring, *nodestring, linei, scixref, *scinodemax, converror);
		   if(rtst == 0){
		      printf("\nConversion ERROR!  Check file ");
		      printf("%s may have been previously converted.\n",netfile);
		      printf("Rename %s to %s and remove %s before restarting!\n",backfile,netfile,backfile);
		      quit(3);
		      }
		   if(rtst == -1)converror = 1;
		   fputs(nodestring,outfile);        /* node */
		   break;
	 default : fputs(nodestring,outfile);
	 } /* END switch */
       } /* end while */
 fclose(dupfile);
 fclose(outfile);
 rd_wr(backfile);
 printf("DONE\n\n");
 if(converror == 1 )printf("ERROR!  Check error.log for conversion errors.\n");
 free(scinodemax);
 free(scixref);
} /* End of MAIN */
