#ifndef CONFIG_H
#define CONFIG_H
/*
 * config.h (for ANTLR, DLG, and SORCERER)
 *
 * This is a simple configuration file that doesn't have config stuff
 * in it, but it's a start.
 *
 * SOFTWARE RIGHTS
 *
 * We reserve no LEGAL rights to the Purdue Compiler Construction Tool
 * Set (PCCTS) -- PCCTS is in the public domain.  An individual or
 * company may do whatever they wish with source code distributed with
 * PCCTS or the code generated by PCCTS, including the incorporation of
 * PCCTS, or its output, into commerical software.
 * 
 * We encourage users to develop software with PCCTS.  However, we do ask
 * that credit is given to us for developing PCCTS.  By "credit",
 * we mean that if you incorporate our source code into one of your
 * programs (commercial product, research project, or otherwise) that you
 * acknowledge this fact somewhere in the documentation, research report,
 * etc...  If you like PCCTS and have developed a nice tool with the
 * output, please mention that you developed it using PCCTS.  In
 * addition, we ask that this header remain intact in our source code.
 * As long as these guidelines are kept, we expect to continue enhancing
 * this system and expect to make other tools available as they are
 * completed.
 *
 * Used by PCCTS 1.33 (SORCERER 1.00B11 and up)
 * Terence Parr
 * Parr Research Corporation
 * with Purdue University and AHPCRC, University of Minnesota
 * 1989-1995
 */

/* This file knows about the following ``environments''
	UNIX (default)
	DOS  (use #define PC)
	MAC  (use #define MPW; has a few things for THINK C, Metrowerks)
 */

/*
*  Define PC32 if in a 32-bit PC environment (e.g. extended DOS or Win32).
*  The macros tested here are defined by Watcom, Microsoft, Borland,
*  and djgpp, respectively, when they are used as 32-bit compilers.
*  Users of these compilers *must* be sure to define PC in their
*  makefiles for this to work correctly.
*/
#ifdef PC
# if (defined(__WATCOM__) || defined(_WIN32) || defined(__WIN32__) || \
   defined(__GNUC__) || defined(__GNUG__))
#     ifndef PC32
#        define PC32
#     endif
#  endif
#endif

#ifdef PC
#define ATOKEN_H			"AToken.h"
#define ATOKPTR_H			"ATokPtr.h"
#define ATOKPTR_C			"ATokPtr.cpp"
#define ATOKENBUFFER_H		"ATokBuf.h"
#define ATOKENBUFFER_C          "ATokBuf.cpp"
#define ATOKENSTREAM_H		"ATokStr.h"
#define APARSER_H			"AParser.h"
#define APARSER_C                       "AParser.cpp"
#define ASTBASE_H			"ASTBase.h"
#define ASTBASE_C                   "ASTBase.cpp"
#define PCCTSAST_C                      "PCCTSAST.cpp"
#define LIST_C                          "List.cpp"
#define DLEXERBASE_H		"DLexBase.h"
#define DLEXERBASE_C            "DLexBase.cpp"
#define DLEXER_C                        "DLexer.cpp"
#define STREESUPPORT_C		"STreeSup.C"
#else
#define ATOKEN_H			"AToken.h"
#define ATOKPTR_H			"ATokPtr.h"
#define ATOKPTR_C			"ATokPtr.cpp"
#define ATOKENBUFFER_H		"ATokenBuffer.h"
#define ATOKENBUFFER_C		"ATokenBuffer.cpp"
#define ATOKENSTREAM_H		"ATokenStream.h"
#define APARSER_H			"AParser.h"
#define APARSER_C			"AParser.cpp"
#define ASTBASE_H			"ASTBase.h"
#define ASTBASE_C		    "ASTBase.cpp"
#define PCCTSAST_C			"PCCTSAST.cpp"
#define LIST_C				"List.cpp"
#define DLEXERBASE_H		"DLexerBase.h"
#define DLEXERBASE_C		"DLexerBase.cpp"
#define DLEXER_C			"DLexer.cpp"
#define STREESUPPORT_C		"STreeSupport.cpp"
#endif

/* SORCERER Stuff */
#ifdef PC
#define STPARSER_H			"STreePar.h"
#define STPARSER_C			"STreePar.C"
#else
#define STPARSER_H			"STreeParser.h"
#define STPARSER_C			"STreeParser.cpp"
#endif

#ifdef MPW
#define CPP_FILE_SUFFIX		".cp"
#define CPP_FILE_SUFFIX_NO_DOT	"cp"
#define OBJ_FILE_SUFFIX		".o"
#else
#ifdef PC
#define CPP_FILE_SUFFIX		".cpp"
#define CPP_FILE_SUFFIX_NO_DOT	"cpp"
#define OBJ_FILE_SUFFIX		".obj"
#else
#define CPP_FILE_SUFFIX		".cpp"
#define CPP_FILE_SUFFIX_NO_DOT	"cpp"
#define OBJ_FILE_SUFFIX		".o"
#endif
#endif

/* User may redefine how line information looks */
#define LineInfoFormatStr "# %d \"%s\"\n"

#ifdef MPW		/* Macintosh Programmer's Workshop */
#define ErrHdr "File \"%s\"; Line %d #"
#else
#define ErrHdr "%s, line %d:"
#endif


/* must assume old K&R cpp here, can't use #if defined(..)... */

#ifdef MPW
#define TopDirectory	":"
#define DirectorySymbol	":"
#define OutputDirectoryOption "Directory where all output files should go (default=\":\")"
#else
#ifdef PC
#define TopDirectory	"."
#define DirectorySymbol	"\\"
#define OutputDirectoryOption "Directory where all output files should go (default=\".\")"
#else
#define TopDirectory	"."
#define DirectorySymbol	"/"
#define OutputDirectoryOption "Directory where all output files should go (default=\".\")"
#endif
#endif

#ifdef MPW

/* Make sure we have prototypes for all functions under MPW */

#include <string.h>
#include <stdlib.h>
#include <CursorCtl.h>
#ifdef __cplusplus
extern "C" {
#endif
extern void fsetfileinfo (char *filename, unsigned long newcreator, unsigned long newtype);
#ifdef __cplusplus
}
#endif

/* File creators for various popular development environments */

#define MAC_FILE_CREATOR 'MPS '   /* MPW Text files */
#if 0
#define MAC_FILE_CREATOR 'KAHL'   /* THINK C/Symantec C++ Text files */
#endif
#if 0
#define MAC_FILE_CREATOR 'MMCC'   /* Metrowerks C/C++ Text files */
#endif

#endif

#ifdef MPW
#define DAWDLE	SpinCursor(1)
#else
#define DAWDLE
#endif


/*
 * useless definitions of special_inits() and special_fopen_actions()
 * deleted -- GPW 1997/09/06
 */

/* Define usable bits for set.c stuff */
#define BytesPerWord	sizeof(unsigned)
#define	WORDSIZE		(sizeof(unsigned)*8)
#define LogWordSize     (WORDSIZE==16?4:5)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef VAXC
#define PCCTS_EXIT_SUCCESS 1
#define PCCTS_EXIT_FAILURE 0
#define zzDIE		return 0;
#define zzDONE	return 1;

#else /* !VAXC */

#define PCCTS_EXIT_SUCCESS 0
#define PCCTS_EXIT_FAILURE 1
#define zzDIE		return 1;
#define zzDONE	return 0;

#endif

#ifdef USER_ZZMODE_STACK
# ifndef ZZSTACK_MAX_MODE
#  define  ZZSTACK_MAX_MODE 32
# endif
# define  ZZMAXSTK (ZZSTACK_MAX_MODE * 2)
#endif

#endif
