/*
** Copyright (c) 2002-2004  Hughes Technologies Pty Ltd.  All rights
** reserved.
**
** Terms under which this software may be used or copied are
** provided in the  specific license associated with this product.
**
** Hughes Technologies disclaims all warranties with regard to this
** software, including all implied warranties of merchantability and
** fitness, in no event shall Hughes Technologies be liable for any
** special, indirect or consequential damages or any damages whatsoever
** resulting from loss of use, data or profits, whether in an action of
** contract, negligence or other tortious action, arising out of or in
** connection with the use or performance of this software.
**
**
** $Id: ember.c,v 1.1 2004/05/09 18:06:24 bambi Exp $
**
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#else
#include <unistd.h>
#endif

#include "httpd.h"
#include "httpd_priv.h"

/* NOTE : This ifdef is for the entire file! */

#ifdef HAVE_EMBER
#include <ember.h>

/**************************************************************************
** GLOBAL VARIABLES
**************************************************************************/


/**************************************************************************
** PRIVATE ROUTINES
**************************************************************************/

int _httpd_executeEmber(server, data)
	httpd	*server;
	char	*data;
{
	ember	*script;

	script = eCreateScript();
	script->www.parseHtml = 0;
	script->www.outputHtml = 1;
	script->www.rawHTTP = 1;
	script->useRuntimeConfig = 1;
	script->stdoutFD = server->clientSock;
	eBufferSource(script, data);
	if (eParseScript(script) < 0)
	{
		fprintf(stdout,"Error at line %d of script '%s'.\n",
			eGetLineNum(script), eGetSourceName(script));
		fprintf(stdout,"Error is '%s'\n\n", eGetErrorMsg(script));
		return(-1);
	}
	if (eRunScript(script) < 0)
	{
		fprintf(stdout,"Runtime error at line %d of script '%s'.\n",
			eGetLineNum(script), eGetCurFileName(script));
		fprintf(stdout,"Error is '%s'\n\n", eGetErrorMsg(script));
	}
	eFreeScript(script);
	return(0);
}

/**************************************************************************
** PUBLIC ROUTINES
**************************************************************************/



#endif /* HAVE_EMBER*/
