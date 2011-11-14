// includes definition
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <strings.h> /* strcasecmp */
#include <unistd.h>
#include "moGenerator.h"
// end

// macro for screen printing
#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)
// #define _DEBUG_ 1
#define PRINT_INFORMATION
//#define _DEBUG_MODELICA 1
// end

// mocro for error message print
#define _IMPORT_ERROR_LOG_
#define WHERESTR  "[date %s, time %s, file %s, line %d]: "
#define WHEREARG  __DATE__, __TIME__, __FILE__, __LINE__
// #define ERRORPRINT2(...)       fprintf(stderr, __VA_ARGS__)
// #define ERRORPRINT(_fmt, ...)  ERRORPRINT2(WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#define ERRORPRINT       fprintf(stderr, WHERESTR, WHEREARG)
// end

// mocro for log message print
#define _IMPORT_LOG_
#define WHERESTR  "[date %s, time %s, file %s, line %d]: "
#define WHEREARG  __DATE__, __TIME__, __FILE__, __LINE__
#define LOGPRINT2(...)       fprintf(logFile, __VA_ARGS__)
#define LOGPRINT(_fmt, ...)  LOGPRINT2(WHERESTR _fmt, WHEREARG, __VA_ARGS__)
// end

// handling platform dependency
#if defined(__MINGW32__) || defined(_MSC_VER)
#include <windows.h>
#define ERROR_ENVVAR_NOT_FOUND           203L
#define getLastErrorSystem GetLastError
#define getEnvVariable GetEnvironmentVariable
#else
#include <errno.h>
#define ERROR_ENVVAR_NOT_FOUND           NULL
#define getLastErrorSystem errno
#define getEnvVariable getenv
#endif
// end

// macro for some global variables
#define FMU_BINARIES_Win32 "binaries\\win32\\"
#define BUFFERSIZE 4096
#define PATHSIZE 1024

#define MODEL_DESCRIPTION "modelDescription.xml"
#define FMU_TEMPLATE "..\\source\\fmuModelica.tmp"
#define FMU_PATH "..\\fmu"
#define BIN_PATH "..\\bin"
//end

#define USE_UNZIP
// return codes of the unzip tool
#define UNZIP_NO_ERROR 0 // sucess
#define UNZIP_WARNINGS 1 // one or more warning errors, but successfully completed anyway
#define UNZIP_NOZIP 9 // no zip files found
#define UNZIP_METHOD_DECRYPTION_ERROR 81 // unsupported compression methods or unsupported decryption
#define UNZIP_WRONG_PASS 82 // no files were found due to bad decryption password
// end

// macro for the 7z tool
#define DECOMPRESS_CMD "7z x -aoa -o"
// return codes of the 7z command line tool
#define SEVEN_ZIP_NO_ERROR 0 // success
#define SEVEN_ZIP_WARNING 1  // e.g., one or more files were locked during zip
#define SEVEN_ZIP_ERROR 2
#define SEVEN_ZIP_COMMAND_LINE_ERROR 7
#define SEVEN_ZIP_OUT_OF_MEMORY 8
#define SEVEN_ZIP_STOPPED_BY_USER 255
// end

const char* variabilityNames[4] = {"constant", "parameter", "discrete", ""};
const char* causalityNames[4] = {"input", "output", "", ""};
const char* startValueFixed[2] = {"false","true"};
const char* fmiBooleanValue[2] = {"false","true"};
// FILE* errLogFile;

// function that returns the name of the FMU from given
// FMU path, more details see FMI for model exchange 1.0
char* getFMUname(const char* fmupath){
	char* fmuname;
	int i, counter, tmp;
	
	counter = strlen(fmupath);
	
	#ifdef _DEBUG_
	printf("#### length of fmupath[1] (%s): %d\n",fmupath,counter);	
	#endif
	
	i = counter-5;
	tmp = i;
	while(fmupath[i]!='/') i--;
	
	#ifdef _DEBUG_
	printf("#### Result: fmupath[%d]: %c\n",i,fmupath[i]);
	#endif
	
	fmuname = (char*)calloc(tmp-i+1, sizeof(char));
	strncpy(fmuname,&fmupath[i+1],tmp-i);
	
	#ifdef _DEBUG_
	printf("#### fmuname is: %s\n",fmuname);
	#endif
	
	return fmuname;
}

// functions that returns the dll path from decompression path
// and model identifier
char* getDllPath(const char* decompPath, const char* mid){
	char * fmudllpath;
	char * pch;
	char * ret_fmudllpath;
	int i;	
	char *tmpStr = NULL;
	int lenStr1 = strlen(FMU_BINARIES_Win32)+strlen(decompPath)+strlen(mid)+4+1;
	int lenStr2 = 0;
	int strcount = 0;
	
	tmpStr = (char *)calloc(lenStr1, sizeof(char));
	fmudllpath = (char *)calloc(lenStr1, sizeof(char));
	strcpy(fmudllpath,decompPath);
	strcat(fmudllpath,FMU_BINARIES_Win32);
	strcat(fmudllpath,mid);
	strcat(fmudllpath,".dll");
	strcpy(tmpStr,fmudllpath);
	
	pch = strtok(fmudllpath,"\\");	
	while(pch!=NULL){
		 lenStr2 += strlen(pch);
		 strcount++;
		 pch = strtok(NULL,"\\");
	}
	ret_fmudllpath = (char*)calloc(lenStr2+(strcount-1)*2, sizeof(char));
	pch = strtok(tmpStr,"\\");
	
	for(i=1;i<strcount;i++){
		strcat(ret_fmudllpath,pch);
		strcat(ret_fmudllpath,"\\\\");
		pch = strtok(NULL,"\\");
		
		#ifdef _DEBUG_
		// printf("#### ret_fmudllpath = %s\n",ret_fmudllpath);
		#endif
	}
	strcat(ret_fmudllpath,pch);
	
	#ifdef _DEBUG_
	// printf("#### ret_fmudllpath = %s\n",ret_fmudllpath);
	// printf("#### strlen(ret_fmudllpath) = %d\n",strlen(ret_fmudllpath));
	#endif
	
	/* free(pch); DO NOT FREE pch - strtok() modifies the first argument, it does not return a new string!!! */
	free(fmudllpath);
	
	return ret_fmudllpath;	
}

// function that returns the name of the model description xml file
char* getNameXMLfile(const char * decompPath, const char * modeldes){
	char * xmlfile = NULL;
	xmlfile = (char *)calloc(strlen(decompPath)+strlen(modeldes)+1, sizeof(char));
	strcpy(xmlfile,decompPath);
	strcat(xmlfile,modeldes);
	return xmlfile;
}

// function that returns the name of the decompression path
char* getDecompPath(const char * omPath, const char* mid){
	int n;
	char * decompPath = NULL;
	
	if (!getEnvVariable("OPENMODELICAHOME", omPath, PATHSIZE)) {
        if (getLastErrorSystem() == ERROR_ENVVAR_NOT_FOUND) {
            ERRORPRINT; fprintf(stderr,"#### Error: Environment variable \"%s\" not defined\n","OPENMODELICAHOME");
        }
        else {
            ERRORPRINT; fprintf(stderr,"#### Error: Could not get value of \"%s\"\n","OPENMODELICAHOME");
        }
        exit(EXIT_FAILURE); // error       
    }
	#ifdef _DEBUG_
		printf("#### %s Enviroment: %s\n",QUOTEME(__LINE__),omPath);
	#endif
	n= strlen(omPath)+strlen(mid)+6;
	decompPath = (char*)calloc(n, sizeof(char));
	sprintf(decompPath,"%sfmu\\%s\\",omPath,mid);
	#ifdef _DEBUG_
		// printf("#### %s decompPath: %s\n",QUOTEME(__LINE__),decompPath);
	#endif
	return decompPath;
}

// function that decompresses the given FMU archive either via OMDev
// inherent unzip command or attached 7z tool
int decompress(const char* fmuPath, const char* decompPath) {
  int err;
  int n;
  char * cmd = NULL; // needed to be freed

#ifdef USE_UNZIP
  n = strlen(fmuPath) + strlen(decompPath) + 17;
  cmd = (char*) calloc(n, sizeof(char));
  sprintf(cmd, "unzip -o \"%s\" -d \"%s\"", fmuPath, decompPath);
  err = system(cmd);
  free(cmd); // free
  if(err!=UNZIP_NO_ERROR){
	switch(err){
		case UNZIP_WARNINGS:
				printf("some warnings occurred during decompression, success anyway...\n");
				break;
		case UNZIP_NOZIP:
				printf("no zip files found...\n");
				break;
		case UNZIP_METHOD_DECRYPTION_ERROR:
				printf("unsupported compression methods or unsupported decryption...\n");
				break;
		case UNZIP_WRONG_PASS:
				printf(" no files were found due to bad decryption password...\n");
				break;
		default:
				printf(" unknown errors..\n");
				break;
	}
  }

#else
  n = strlen(DECOMPRESS_CMD) + strlen(fmuPath) +strlen(decompPath)+10;
  cmd = (char*)calloc(n, sizeof(char));
  sprintf(cmd, "%s%s \"%s\" > NUL", DECOMPRESS_CMD, decompPath, fmuPath);
  err = system(cmd);
  free(cmd); // free
  if (err!=SEVEN_ZIP_NO_ERROR) {
    switch (err) {
      ERRORPRINT; fprintf(stderr,"#### 7z: %s","");
      case SEVEN_ZIP_WARNING: printf("warning\n"); break;
      case SEVEN_ZIP_ERROR: printf("error\n"); break;
      case SEVEN_ZIP_COMMAND_LINE_ERROR: printf("command line error\n"); break;
      case SEVEN_ZIP_OUT_OF_MEMORY: printf("out of memory\n"); break;
      case SEVEN_ZIP_STOPPED_BY_USER: printf("stopped by user\n"); break;
      default: ERRORPRINT; fprintf(stderr,"#### Unknown problem %s\n","");
    }
  }
#endif
  return EXIT_SUCCESS;
}

// function that returns the number of scalar variables contained in
// the ModelVariables entity, return -1 for error
int getNumberOfSV(ModelDescription* md) {
  int i;
  
  if (md->modelVariables) {
    for (i = 0; md->modelVariables[i]; i++);
    return i;
  }
  else
    return -1;
}

// function that gets the element type from a ScalarVariable defined
// in xmlparser.h
fmiScalarVariableType getElementType(ScalarVariable* sv){
	Elm elm;
	fmiScalarVariableType sv_type;
	elm = sv->typeSpec->type;
	
	switch(elm){
		case elm_Real: 		  sv_type = sv_real; break;
		case elm_Integer: 	  sv_type = sv_integer; break;
		case elm_Boolean: 	  sv_type = sv_boolean; break;
		case elm_String:      sv_type = sv_string; break;
		case elm_Enumeration: sv_type = sv_enum; break;
		
		default: ERRORPRINT; fprintf(stderr,"#### Unknown element type in getElementType() for ScalarVaraible %s...\n",getString(sv,att_name)); exit(EXIT_FAILURE);
	}
	return sv_type;
}

// function that allocates memory for element contained in scalar variables
void* allocateElmSV(fmiScalarVariable fmisv){
	switch(fmisv.type){
		case sv_real: return calloc(1, sizeof(fmiREAL));
		case sv_integer: return calloc(1, sizeof(fmiINTEGER));
		case sv_boolean: return calloc(1, sizeof(fmiBOOLEAN));
		case sv_string: return calloc(1, sizeof(fmiSTRING));
		case sv_enum: return calloc(1, sizeof(fmiENUM));
		default: ERRORPRINT; fprintf(stderr,"#### Unknown element type in allocateElmSV() for fmiScalarVariable: %s ...\n",fmisv.name); exit(EXIT_FAILURE);
	}
}


// function that  instantiates the element in scalar variable,
// e.g. Real, Integer, ect..
void instElmSV(ScalarVariable* sv, fmiScalarVariable fmisv){
	ValueStatus vs;
	double tmp_db;
	int tmp_i;
	const char *tmp_str;
	fmiBooleanXML tmp_bl;
	
	switch(fmisv.type){
		case sv_real:
		{
			tmp_str = getString(sv->typeSpec,att_declaredType);
			((fmiREAL *)fmisv.variable)->declType = (tmp_str!=NULL ? tmp_str : "");
			
			tmp_str = getString(sv->typeSpec,att_quantity);
			((fmiREAL *)fmisv.variable)->quantity = (tmp_str!=NULL ? tmp_str : "");
			
			tmp_str = getString(sv->typeSpec,att_unit);
			((fmiREAL *)fmisv.variable)->unit = (tmp_str!=NULL ? tmp_str : "");
			
			tmp_str = getString(sv->typeSpec,att_displayUnit);
			((fmiREAL *)fmisv.variable)->displayUnit = (tmp_str!=NULL ? tmp_str : "");
			
			tmp_bl = getBoolean(sv->typeSpec,att_relativeQuantity,&vs);
			((fmiREAL *)fmisv.variable)->relQuantity = (vs==valueDefined ? tmp_bl : 0);
			
			tmp_db = getDouble(sv->typeSpec,att_min,&vs);
			if(vs==valueDefined){
				((fmiREAL *)fmisv.variable)->min = tmp_db;
				((fmiREAL *)fmisv.variable)->defMin = 1;
			}
			else ((fmiREAL *)fmisv.variable)->defMin = 0;
			
			tmp_db = getDouble(sv->typeSpec,att_max,&vs);
			if(vs==valueDefined){
				((fmiREAL *)fmisv.variable)->max = tmp_db;
				((fmiREAL *)fmisv.variable)->defMax = 1;
			}
			else ((fmiREAL *)fmisv.variable)->defMax = 0;

			tmp_db = getDouble(sv->typeSpec,att_nominal,&vs);
			if(vs==valueDefined){
				((fmiREAL *)fmisv.variable)->nominal = tmp_db;
				((fmiREAL *)fmisv.variable)->defNorminal = 1;
			}
			else ((fmiREAL *)fmisv.variable)->defNorminal = 0;
			
			tmp_db = getDouble(sv->typeSpec,att_start,&vs);
			if(vs==valueDefined){
				((fmiREAL *)fmisv.variable)->start = tmp_db;
				((fmiREAL *)fmisv.variable)->defStart = 1;
			}
			else ((fmiREAL *)fmisv.variable)->defStart = 0;
			
			tmp_bl = getBoolean(sv->typeSpec,att_fixed,&vs);
			((fmiREAL *)fmisv.variable)->fixed = (vs==valueDefined ? tmp_bl : 0);
			
			break;
		}
		case sv_integer:
		{	
			tmp_str = getString(sv->typeSpec,att_declaredType);
			((fmiINTEGER *)fmisv.variable)->declType = (tmp_str!=NULL ? tmp_str : "");
			
			tmp_str = getString(sv->typeSpec,att_quantity);
			((fmiINTEGER *)fmisv.variable)->quantity = (tmp_str!=NULL ? tmp_str : "");
			
			tmp_i = getInt(sv->typeSpec,att_min,&vs);
			if(vs==valueDefined){
				((fmiINTEGER *)fmisv.variable)->min = tmp_i;
				((fmiINTEGER *)fmisv.variable)->defMin = 1;
			}
			else ((fmiINTEGER *)fmisv.variable)->defMin = 0;
			
			tmp_i = getInt(sv->typeSpec,att_max,&vs);
			if(vs==valueDefined){
				((fmiINTEGER *)fmisv.variable)->max = tmp_i;
				((fmiINTEGER *)fmisv.variable)->defMax = 1;
			}
			else ((fmiINTEGER *)fmisv.variable)->defMax = 0;

			
			tmp_i = getInt(sv->typeSpec,att_start,&vs);
			if(vs==valueDefined){
				((fmiINTEGER *)fmisv.variable)->start = tmp_i;
				((fmiINTEGER *)fmisv.variable)->defStart = 1;
			}
			else if(vs==valueMissing) ((fmiINTEGER *)fmisv.variable)->defStart = 0;
			else{
				printf("[%d]: #### Integer variable %s: start value illegal ...\n",__LINE__,fmisv.name);
				exit(EXIT_FAILURE);
			}
			tmp_bl = getBoolean(sv->typeSpec,att_fixed,&vs);
			((fmiINTEGER *)fmisv.variable)->fixed = (vs==valueDefined ? tmp_bl : 0);
			
			break;
		}
		case sv_boolean:
		{
			tmp_str = getString(sv->typeSpec,att_declaredType);
			((fmiBOOLEAN *)fmisv.variable)->declType = (tmp_str!=NULL ? tmp_str : "");
			tmp_bl = getBoolean(sv->typeSpec,att_start,&vs);
			if(vs==valueDefined){			
				((fmiBOOLEAN *)fmisv.variable)->start = tmp_bl;
				((fmiBOOLEAN *)fmisv.variable)->defStart = 1;
			}
			else if(vs==valueMissing) ((fmiBOOLEAN *)fmisv.variable)->defStart = 0;
			else{
				printf("[%d]: #### Boolean variable %s: start value illegal ...\n",__LINE__,fmisv.name);
				exit(EXIT_FAILURE);
			}
			tmp_bl = getBoolean(sv->typeSpec,att_fixed,&vs);
			((fmiBOOLEAN *)fmisv.variable)->fixed = (vs==valueDefined ? tmp_bl : 0);
			
			break;
		}
		case sv_string:
		{
			tmp_str = getString(sv->typeSpec,att_declaredType);
			((fmiSTRING *)fmisv.variable)->declType = (tmp_str!=NULL ? tmp_str : "");
			
			tmp_str = getString(sv->typeSpec,att_start);
			if(tmp_str){
				((fmiSTRING *)fmisv.variable)->start = tmp_str;
				((fmiSTRING *)fmisv.variable)->defStart = 1;
			}
			else{
				((fmiSTRING *)fmisv.variable)->start = "";
				((fmiSTRING *)fmisv.variable)->defStart = 0;
			}			
			
			tmp_bl = getBoolean(sv->typeSpec,att_fixed,&vs);
			((fmiSTRING *)fmisv.variable)->fixed = (vs==valueDefined ? tmp_bl : 0);
			break;
		}
		case sv_enum:
		{	
			tmp_str = getString(sv->typeSpec,att_declaredType);
			((fmiENUM *)fmisv.variable)->declType = (tmp_str!=NULL ? tmp_str : "");
			
			tmp_str = getString(sv->typeSpec,att_quantity);
			((fmiENUM *)fmisv.variable)->quantity = (tmp_str!=NULL ? tmp_str : "");
			
			tmp_i = getInt(sv->typeSpec,att_min,&vs);
			if(vs==valueDefined){
				((fmiENUM *)fmisv.variable)->min = tmp_i;
				((fmiENUM *)fmisv.variable)->defMin = 1;
			}
			else ((fmiENUM *)fmisv.variable)->defMin = 0;
			
			tmp_i = getInt(sv->typeSpec,att_max,&vs);
			if(vs==valueDefined){
				((fmiENUM *)fmisv.variable)->max = tmp_i;
				((fmiENUM *)fmisv.variable)->defMax = 1;
			}
			else ((fmiENUM *)fmisv.variable)->defMax = 0;

			
			tmp_i = getInt(sv->typeSpec,att_start,&vs);
			if(vs==valueDefined){
				((fmiENUM *)fmisv.variable)->start = tmp_i;
				((fmiENUM *)fmisv.variable)->defStart = 1;
			}
			else ((fmiENUM *)fmisv.variable)->defStart = 0;

			break;
		}
		
		default: ERRORPRINT; fprintf(stderr,"#### Unknown error in instElmSV() for fmiScalarVariable: %s...\n",fmisv.name); exit(EXIT_FAILURE);
	}
}

// functions that replaces the given character old with the character new in a string
void charReplace(char* flatName, unsigned int len, char old, char new){
	char* pntCh = NULL;
	
	pntCh = strchr(flatName,old);
	while(pntCh!=NULL){
		*pntCh = new;
		pntCh = strchr(flatName,old);
	}
	flatName[len] = '\0';
}

// function that returns the fmiVariability defined in moGenerator.h
fmiVariability getFMIVariability(ScalarVariable* sv){
	fmiVariability ret;
	Enu fmi_var;
	
	fmi_var = getVariability(sv);
	// var cases: enu_constant,enu_parameter,enu_discrete,enu_continuous
	switch(fmi_var){
		case enu_constant:   ret = constant; break;
		case enu_parameter:  ret = parameter; break;
		case enu_discrete:   ret = discrete; break;
		case enu_continuous: ret = continuous; break;
		
		default: ERRORPRINT; fprintf(stderr,"#### Unknown error in getFMIVariability() for fmiScalarVariable: %s...\n",getString(sv,att_name)); exit(EXIT_FAILURE);
	}
	return ret;
}

// function that returns the fmiCausality defined in moGenerator.h
fmiCausality getFMICausality(ScalarVariable* sv){
	fmiCausality ret;
	Enu fmi_cau;
	
	fmi_cau = getCausality(sv);
	// fmi_cau cases: enu_input,enu_output,enu_internal,enu_none
	switch(fmi_cau){
		case enu_input:    ret = input; break;
		case enu_output:   ret = output; break;
		case enu_internal: ret = internal; break;
		case enu_none:     ret = none; break;
		
		default: ERRORPRINT; fprintf(stderr,"#### Unknown error in getFMICausality() for fmiScalarVariable: %s...\n",getString(sv,att_name)); exit(EXIT_FAILURE);
	}
	return ret;
}

// function that returns the fmiAlias defined in moGenerator.h
fmiAlias getFMIAlias(ScalarVariable* sv){
	fmiAlias ret;
	Enu fmi_als;
	
	fmi_als = getAlias(sv);
	// fmi_als cases: enu_noAlias,enu_alias,enu_negatedAlias
	switch(fmi_als){
		case enu_noAlias:      ret = noalias; break;
		case enu_alias:        ret = alias; break;
		case enu_negatedAlias: ret = negatedAlias; break;
		
		default: ERRORPRINT; fprintf(stderr,"#### Unknown error in getFMIAlias() for fmiScalarVariable: %s...\n",getString(sv,att_name)); exit(EXIT_FAILURE);
	}
	return ret;
}

// function that instantiates the fmiScalarVariable list
void instScalarVariable(ModelDescription* md,fmiScalarVariable* list){
	int i;
	unsigned int len;

	if(md->modelVariables){
		for (i=0;md->modelVariables[i];i++){
			list[i].name = getName(md->modelVariables[i]);
			len = strlen(list[i].name);
			list[i].flatName = (char*)calloc(len+1, sizeof(char));
			strcpy(list[i].flatName,list[i].name);
			charReplace(list[i].flatName,len,'.','_');
			list[i].vr = getValueReference(md->modelVariables[i]);
			list[i].description = getDescription(md,md->modelVariables[i]);
			#ifdef _DEBUG_
			printf("#### descritpion of sv %s, %s, value reference: %d\n",list[i].name, list[i].description,list[i].vr );
			#endif

			list[i].var = getFMIVariability(md->modelVariables[i]);
			list[i].causality = getFMICausality(md->modelVariables[i]);
			list[i].alias = getFMIAlias(md->modelVariables[i]);
			list[i].type = getElementType(md->modelVariables[i]);
			#ifdef _DEBUG_
			printf("#### sv.typeSpec.type: %d\n",list[i].type);
			#endif
			
			list[i].variable = allocateElmSV(list[i]);
			instElmSV(md->modelVariables[i],list[i]);
			#ifdef _DEBUG_
				if(list[i].type==0)
				printf("#### %s startvalue: %lf\n", list[i].name,((fmiREAL*)list[i].variable)->start);
				if(list[i].type==1)
				printf("#### %s startvalue: %d\n", list[i].name,((fmiINTEGER*)list[i].variable)->start);
				if(list[i].type==2)
				printf("#### %s startvalue: %d\n", list[i].name,((fmiBOOLEAN*)list[i].variable)->start);
			#endif
		};
		return;
	}
	else{
		ERRORPRINT; fprintf(stderr,"#### instScalarVariable() failed: no modelVariable defined or memory error...%s\n","");
		exit(EXIT_FAILURE);
	}
}

// functions that returns the variable naming convention
fmiNamingConvention getNamingConvention(ModelDescription* md, Att att){
	ValueStatus vs;
	Enu enu;
	fmiNamingConvention nconv;
	
	enu = getEnumValue(md,att_variableNamingConvention,&vs);
	if (vs == valueIllegal){
		ERRORPRINT; fprintf(stderr,"#### Return value of getNamingConvention is illegal...%s\n","");
		exit(EXIT_FAILURE);
	}
	if(enu==enu_flat) nconv = flat;
	else nconv = structured;
	return nconv;
}

fmiDefaultExperiment *getDefaultExperiment(ModelDescription *md){
	ValueStatus vs;
	double tmp_db;
	fmiDefaultExperiment *defExp = (fmiDefaultExperiment *)calloc(1,sizeof(fmiDefaultExperiment));	;
	
	if(md->defaultExperiment){
		
		tmp_db = getDouble(md->defaultExperiment, att_startTime, &vs);
		defExp->startTime = (vs == valueDefined ? tmp_db : 0.0);
		
		tmp_db = getDouble(md->defaultExperiment, att_stopTime, &vs);
		defExp->stopTime = (vs == valueDefined ? tmp_db : 0.0);

		tmp_db = getDouble(md->defaultExperiment, att_tolerance, &vs);
		defExp->tolerance = (vs == valueDefined ? tmp_db : 0.00001);
	}
	else{
		defExp->startTime = 0.0;
		defExp->stopTime = 3.0;
		defExp->tolerance = 0.00001;
	}
	return defExp;
}

// function that instantiates the tree-like structure of fmu model description
void instFmuModelDescription(ModelDescription* md, fmuModelDescription* fmuMD, fmiModelVariable* fmuMV){
	fmuMD->fmiver = getFmiVersion(md);
	fmuMD->mn = getModelName(md);
	fmuMD->mid = getModelIdentifier(md);
	fmuMD->guid = getString(md, att_guid);
	fmuMD->description = getString(md,att_description);
	fmuMD->author = getString(md,att_author);
	fmuMD->mver = getString(md,att_version);
	fmuMD->genTool = getString(md,att_generationTool);
	fmuMD->genTime = getString(md,att_generationDateAndTime);
	fmuMD->nconv = getNamingConvention(md,att_variableNamingConvention);
	fmuMD->ncs = getNumberOfContStates(md);
	fmuMD->nei = getNumberOfEventIndicators(md);
	fmuMD->modelVariable = fmuMV;
	fmuMD->defaultExperiment = getDefaultExperiment(md);
}

// function that frees the allocated memory for scalar variable list
void freeScalarVariableLst(fmiScalarVariable* list,int nsv){
	int i;
	
	for(i=0;i<nsv;i++){
		free(list[i].variable);
	}
	free(list);
}

void printFileTest(){
	FILE *pfile = NULL;
	pfile = fopen("test.txt","w");
	fprintf(pfile,"\npackage FMUImport_%s\n","This is it");
	fclose(pfile);
	return;
}

// Modelica code generation for the external interface functions
void tmpcodegen(fmuModelDescription* fmuMD, const char* decompPath){
	char tmpstr[BUFFERSIZE];
	
	// Allocated memory needed to be freed and file needed to be closed
	FILE *pfile = NULL;
	FILE *pf = NULL;
	char *id = NULL;

	size_t len = strlen(fmuMD->mid)+strlen(decompPath);
	id = (char*) calloc(len+4, sizeof(char));
	strcpy(id,decompPath);
	strcat(id,fmuMD->mid);
	strcat(id,".mo");
	id[len+3]='\0';
	
	#ifdef _DEBUG_
	printf("#### %s id = %s\n",QUOTEME(__LINE__),id);
	#endif
	
	pfile = fopen(id,"w");
	if(!pfile){
		ERRORPRINT; fprintf(stderr,"#### Creating %s failed...\n",id);
		exit(EXIT_FAILURE);
	}
	else{
		fprintf(pfile,"\npackage FMUImport_%s\n",fmuMD->mid);
		pf = fopen(FMU_TEMPLATE,"r");
		if(!pf){
			ERRORPRINT; fprintf(stderr,"#### Opening %s failed...\n",FMU_TEMPLATE);
			exit(EXIT_FAILURE);
		}		
		while(!feof(pf)){
			fgets(tmpstr,BUFFERSIZE,pf);
			if(ferror(pf)){
				ERRORPRINT; fprintf(stderr,"#### Reading lines from %s failed...\n",FMU_TEMPLATE);
				exit(EXIT_FAILURE);
			}
			fputs(tmpstr,pfile);
		}
	}
	// Free memory
	fclose(pf);
	fclose(pfile);
	free(id);
}

// function that adds an element to an fmuOutputVar list;
void addOutputVariable(fmiScalarVariable* sv, fmuOutputVar** root, fmuOutputVar** nextVar, unsigned int* counter){
	int tmpAlias;
	if(sv->alias == noalias) tmpAlias = 1;
	if(sv->alias == alias) tmpAlias = 1;
	if(sv->alias == negatedAlias) tmpAlias = -1;
	
	if(*root == NULL){
		*root = (fmuOutputVar*)calloc(1, sizeof(fmuOutputVar));
		(*root)->name = sv->flatName;
		(*root)->vr = sv->vr;
		(*root)->aliasInd = tmpAlias;
		(*root)->next = NULL;
		*nextVar = *root;
	}
	else{
		(*nextVar)->next = (fmuOutputVar*)calloc(1, sizeof(fmuOutputVar));
		(*nextVar) = (*nextVar)->next;
		(*nextVar)->name = sv->flatName;
		(*nextVar)->vr = sv->vr;
		(*nextVar)->aliasInd = tmpAlias;
		(*nextVar)->next = NULL;
	}
	(*counter)++;
}

// function that appends tow strings and returns the resulted string
char* appendString(char const* head, char const* tail){
	int len;
	char* res = NULL;
	
	len = strlen(head)+strlen(tail)+1;
	res = (char*)calloc(len, sizeof(char));
	strcat(res,head);
	strcat(res,tail);
	res[len-1] = '\0';
	return res;
}

// Modelica code generation for the FMU block
void blockcodegen(fmuModelDescription* fmuMD, const char* decompPath, const char* fmudllpath){
	// Allocated memory needed to be freed
	char *id = NULL;
	FILE *pfile = NULL;
	fmiScalarVariable *tmpSV = NULL;
	
	// poiters for output variable linked listss
	fmuOutputVar* pntReal = NULL;
	fmuOutputVar* tmpReal = NULL;	
	fmuOutputVar* pntInteger = NULL;
	fmuOutputVar* tmpInteger = NULL;	
	fmuOutputVar* pntString = NULL;
	fmuOutputVar* tmpString = NULL;	
	fmuOutputVar* pntBoolean = NULL;
	fmuOutputVar* tmpBoolean = NULL;
	
	unsigned int noReal; // number of real variable
	unsigned int noInteger;	// number of integer variables
	unsigned int noString; // number of string variables
	unsigned int noBoolean; // number of boolean variables
	unsigned int noEnumeration; // number of enumeration variables
	int existPre; /* Whether there exists previous attribute */
	
	int j; /* counter */
	double StartTime = fmuMD->defaultExperiment->startTime;
	double StopTime = fmuMD->defaultExperiment->stopTime;;
	double Tolerance = fmuMD->defaultExperiment->tolerance;;
	
	// printing strings in modelica code
	const char *varDesc;
	char *y = appendString("y_",fmuMD->mid);
	char *nx = appendString("nx_",fmuMD->mid);
	char *der_x = appendString("der_x_",fmuMD->mid);
	char *out_x = appendString("out_x_",fmuMD->mid);
	char *out_der_x = appendString("out_der_x_",fmuMD->mid);
	char *x = appendString("x_",fmuMD->mid);
	char *nz = appendString("nz_",fmuMD->mid);
	char *z = appendString("z_",fmuMD->mid);
	char *prez = appendString("prez_",fmuMD->mid);
	char *zXprez = appendString("zXprez",fmuMD->mid);
	char *flagSE = appendString("flagSE_",fmuMD->mid);
	char *indSE = appendString("indSE_",fmuMD->mid);
	char *defaultVar = appendString("default_",fmuMD->mid);
	
	size_t len = strlen(fmuMD->mid)+strlen(decompPath)+4;
	id = (char *)calloc(len,sizeof(char));
	strcpy(id,decompPath);
	strcat(id,fmuMD->mid);
	strcat(id,".mo");
	id[len-1]='\0';
	
	pfile = fopen(id,"a+");
	if(!pfile){
		ERRORPRINT; fprintf(stderr,"#### Creating %s failed...\n",id);
		exit(EXIT_FAILURE);
	}
	else{
		fprintf(pfile,"\nblock FMUBlock \"%s model\"\n",fmuMD->mid);
		fprintf(pfile,"\tannotation(experiment(StartTime = %lf, StopTime = %lf, Tolerance = %lf));\n",StartTime,StopTime,Tolerance);
		// fprintf(pfile,"\tannotation(experiment(StartTime = %f, StopTime = %f));\n",StartTime,StopTime);

		if((fmuMD->ncs) > 0) fprintf(pfile,"\toutput Real y_%s[%d];\n",fmuMD->mid,fmuMD->ncs);
		fprintf(pfile,"\tconstant String dllPath = \"%s\";\n",fmudllpath);
		fprintf(pfile,"\tconstant String instName = \"%s\";\n",fmuMD->mid);
		fprintf(pfile,"\tconstant String guid = \"%s\";\n",fmuMD->guid);
		fprintf(pfile,"\tparameter Boolean logFlag = false;\n");
		fprintf(pfile,"\tparameter Boolean tolControl = true;\n");
		tmpSV = fmuMD->modelVariable->list_sv;
		#ifdef _DEBUG_
			printf("++++ fmuMD->modelVariable->nsv %d \n",fmuMD->modelVariable->nsv);
		#endif
	
		noReal = 0;
		noInteger = 0;
		noString = 0;
		noBoolean = 0;
		
		if(tmpSV>0){
			for(j = 0; j<fmuMD->modelVariable->nsv; j++){
				
				if(tmpSV[j].description != NULL) varDesc = tmpSV[j].description;
				else varDesc = "";
				
				#ifdef _DEBUG_
				printf("++++ Variable type is %d\n",tmpSV[j].type);
				printf("++++ Variable name is %s\n",tmpSV[j].name);
				#endif
				if(strncmp(tmpSV[j].name,"der(",4)!=0){ // check if the scalar variable is a derivative						
					switch(tmpSV[j].type){ // sorting by variable types
						case sv_real:{
							/* local variables for Real type varialbe definition */
							int bool_quantity, bool_unit, bool_displayUnit, bool_defMin, bool_defMax, bool_norminal, bool_defStart, bool_fixed;
						
							bool_quantity = (strcmp(((fmiREAL *)tmpSV[j].variable)->quantity,"") != 0) ? 1 : 0;
							bool_unit = (strcmp(((fmiREAL *)tmpSV[j].variable)->unit,"") != 0) ? 1 : 0;
							bool_displayUnit = (strcmp(((fmiREAL *)tmpSV[j].variable)->displayUnit,"") != 0) ? 1 : 0;
							bool_defMin = ((fmiREAL *)tmpSV[j].variable)->defMin;
							bool_defMax = ((fmiREAL *)tmpSV[j].variable)->defMax;
							bool_norminal = ((fmiREAL *)tmpSV[j].variable)->defNorminal;
							bool_defStart = ((fmiREAL *)tmpSV[j].variable)->defStart;
							bool_fixed = (((fmiREAL *)tmpSV[j].variable)->fixed == fmi_true) ? 1 : 0;
							
							if(((tmpSV[j].causality) != none) && ((tmpSV[j].causality) != internal)) fprintf(pfile,"\t%s",causalityNames[tmpSV[j].causality]);
							if((tmpSV[j].var) != continuous) fprintf(pfile," %s",variabilityNames[tmpSV[j].var]);
							fprintf(pfile," Real %s",tmpSV[j].flatName);
							existPre = 0;
							if(bool_quantity || bool_unit || bool_displayUnit || bool_defMin || bool_defMax || bool_norminal ||bool_defStart || bool_fixed) fprintf(pfile,"%s","(");
							if(bool_quantity){
								fprintf(pfile, "quantity = \"%s\"",((fmiREAL *)tmpSV[j].variable)->quantity);
								existPre = 1;
							}
							if(bool_unit){
								if(existPre) fprintf(pfile, ", unit = \"%s\"",((fmiREAL *)tmpSV[j].variable)->unit);
								else{
									fprintf(pfile, "unit = \"%s\"",((fmiREAL *)tmpSV[j].variable)->unit);
									existPre = 1;
								}
							}
							if(bool_displayUnit){
								if(existPre) fprintf(pfile, ", displayUnit = \"%s\"",((fmiREAL *)tmpSV[j].variable)->displayUnit);
								else{
									fprintf(pfile, "displayUnit = \"%s\"",((fmiREAL *)tmpSV[j].variable)->displayUnit);
									existPre = 1;
								}
							}
							if(bool_defStart){
								if(existPre) fprintf(pfile,", start = %lf",((fmiREAL*)tmpSV[j].variable)->start);
								else{
									fprintf(pfile,"start = %lf",((fmiREAL*)tmpSV[j].variable)->start);
									existPre = 1;
								}
							}
							if(bool_defMin){
								if(existPre) fprintf(pfile,", min = %lf",((fmiREAL*)tmpSV[j].variable)->min);
								else{
									fprintf(pfile,"min = %lf",((fmiREAL*)tmpSV[j].variable)->min);
									existPre = 1;
								}
							}
							if(bool_defMax){
								if(existPre) fprintf(pfile,", max = %lf",((fmiREAL*)tmpSV[j].variable)->max);
								else{
									fprintf(pfile,"max = %lf",((fmiREAL*)tmpSV[j].variable)->max);
									existPre = 1;
								}
							}
							if(bool_fixed){
								if(existPre) fprintf(pfile,", fixed = %s",startValueFixed[((fmiREAL*)tmpSV[j].variable)->fixed]);
								else{
									fprintf(pfile,"fixed = %s",startValueFixed[((fmiREAL*)tmpSV[j].variable)->fixed]);
									/* existPre = 1; */
								}
							}
							if(bool_quantity || bool_unit || bool_displayUnit || bool_defMin || bool_defMax || bool_norminal ||bool_defStart || bool_fixed) fprintf(pfile,"%s"," )");							
							if(strcmp(varDesc,"") != 0) fprintf(pfile," \"%s\"",varDesc);
							fprintf(pfile,"%s",";\n");	
							/*
							fprintf(pfile,"\t%s %s Real %s (start=%lf, fixed = %s) \"%s\";\n",causalityNames[tmpSV[j].causality],
							variabilityNames[tmpSV[j].var],tmpSV[j].flatName,((fmiREAL*)tmpSV[j].variable)->start,
							startValueFixed[((fmiREAL*)tmpSV[j].variable)->fixed],varDesc);
							*/
							if((tmpSV[j].var!=0) && (tmpSV[j].var!=1))	addOutputVariable(&tmpSV[j],&pntReal,&tmpReal,&noReal);
							break;
						}
						
						case sv_integer:{
							/* local variables for Integer type varialbe definition */
							int bool_quantity, bool_defMin, bool_defMax, bool_defStart, bool_fixed;							
							
							bool_quantity = (strcmp(((fmiINTEGER *)tmpSV[j].variable)->quantity,"") != 0) ? 1 : 0;
							bool_defMin = ((fmiINTEGER *)tmpSV[j].variable)->defMin;
							bool_defMax = ((fmiINTEGER *)tmpSV[j].variable)->defMax;
							bool_defStart = ((fmiINTEGER *)tmpSV[j].variable)->defStart;
							bool_fixed = (((fmiINTEGER *)tmpSV[j].variable)->fixed == fmi_true) ? 1 : 0;
							
							if(((tmpSV[j].causality) != none) && ((tmpSV[j].causality) != internal))fprintf(pfile,"\t%s",causalityNames[tmpSV[j].causality]);
							fprintf(pfile," Integer %s",tmpSV[j].flatName);
							existPre = 0;
							if(bool_quantity || bool_defMin || bool_defMax || bool_defStart || bool_fixed) fprintf(pfile," %s","(");
							if(bool_quantity){
								fprintf(pfile, "quantity = \"%s\"",((fmiINTEGER *)tmpSV[j].variable)->quantity);
								existPre = 1;
							}
							if(bool_defStart){
								if(existPre) fprintf(pfile,", start = %d",((fmiINTEGER*)tmpSV[j].variable)->start);
								else{
									fprintf(pfile,"start = %d",((fmiINTEGER*)tmpSV[j].variable)->start);
									existPre = 1;
								}
							}
							if(bool_defMin){
								if(existPre) fprintf(pfile,", min = %d",((fmiINTEGER*)tmpSV[j].variable)->min);
								else{
									fprintf(pfile,"min = %d",((fmiINTEGER*)tmpSV[j].variable)->min);
									existPre = 1;
								}
							}
							if(bool_defMax){
								if(existPre) fprintf(pfile,", max = %d",((fmiINTEGER*)tmpSV[j].variable)->max);
								else{
									fprintf(pfile,"max = %d",((fmiINTEGER*)tmpSV[j].variable)->max);
									existPre = 1;
								}
							}
							if(bool_fixed){
								if(existPre) fprintf(pfile,", fixed = %s",startValueFixed[((fmiINTEGER*)tmpSV[j].variable)->fixed]);
								else{
									fprintf(pfile,"fixed = %s",startValueFixed[((fmiINTEGER*)tmpSV[j].variable)->fixed]);
									/* existPre = 1; */
								}
							}
							if(bool_quantity || bool_defMin || bool_defMax || bool_defStart || bool_fixed) fprintf(pfile," %s",")");
							if(strcmp(varDesc,"") != 0) fprintf(pfile," \"%s\"",varDesc);
							fprintf(pfile,"%s",";\n");
							
							// fprintf(pfile,"\t%s %s Integer %s (start=%d, fixed = %s) \"%s\";\n",causalityNames[tmpSV[j].causality],
							// variabilityNames[tmpSV[j].var],tmpSV[j].flatName,((fmiINTEGER*)tmpSV[j].variable)->start,
							// startValueFixed[((fmiINTEGER*)tmpSV[j].variable)->fixed],varDesc);

							if((tmpSV[j].var!=0) && (tmpSV[j].var!=1))	addOutputVariable(&tmpSV[j],&pntInteger,&tmpInteger,&noInteger);
							break;
						}
						case sv_boolean:{
							int bool_defStart, bool_fixed;
							
							bool_defStart = ((fmiBOOLEAN *)tmpSV[j].variable)->defStart;
							bool_fixed = (((fmiBOOLEAN *)tmpSV[j].variable)->fixed == fmi_true) ? 1 : 0;
							
							if(((tmpSV[j].causality) != none) && ((tmpSV[j].causality) != internal)) fprintf(pfile,"\t%s",causalityNames[tmpSV[j].causality]);
							fprintf(pfile," Boolean %s",tmpSV[j].flatName);
							existPre = 0;
							if(bool_defStart || bool_fixed) fprintf(pfile," %s","(");
							if(bool_defStart){
								if(((fmiINTEGER*)tmpSV[j].variable)->start==1) fprintf(pfile," start = %s","true");
								else fprintf(pfile," start = %s","false");
								existPre = 1;
							}
							if(bool_fixed){
								if(existPre) fprintf(pfile,", fixed = %s",startValueFixed[((fmiBOOLEAN*)tmpSV[j].variable)->fixed]);
								else{
									fprintf(pfile,"fixed = %s",startValueFixed[((fmiBOOLEAN*)tmpSV[j].variable)->fixed]);
									existPre = 1;
								}
							}
							if(bool_defStart || bool_fixed) fprintf(pfile," %s",")");
							if(strcmp(varDesc,"") != 0) fprintf(pfile," \"%s\"",varDesc);
							fprintf(pfile,"%s",";\n");
							
							// fprintf(pfile,"\t%s %s Boolean %s (start=%s, fixed = %s) \"%s\";\n",causalityNames[tmpSV[j].causality],
							// variabilityNames[tmpSV[j].var],tmpSV[j].flatName,fmiBooleanValue[((fmiBOOLEAN*)tmpSV[j].variable)->start],
							// startValueFixed[((fmiBOOLEAN*)tmpSV[j].variable)->fixed],varDesc);
							
							if((tmpSV[j].var!=0) && (tmpSV[j].var!=1))	addOutputVariable(&tmpSV[j],&pntBoolean,&tmpBoolean,&noBoolean);
							break;
						}
						case sv_string:{
							int bool_defStart, bool_fixed;
							
							bool_defStart = ((fmiSTRING *)tmpSV[j].variable)->defStart;
							bool_fixed = (((fmiSTRING *)tmpSV[j].variable)->fixed == fmi_true) ? 1 : 0;
							if(((tmpSV[j].causality) != none) && ((tmpSV[j].causality) != internal)) fprintf(pfile,"\t%s",causalityNames[tmpSV[j].causality]);
							fprintf(pfile," String %s",tmpSV[j].flatName);
							existPre = 0;
							if(bool_defStart || bool_fixed) fprintf(pfile," %s","(");
							if(bool_defStart){
								fprintf(pfile," start = \"%s\"",((fmiSTRING*)tmpSV[j].variable)->start);
								existPre = 1;
							}
							if(bool_fixed){
								if(existPre) fprintf(pfile,", fixed = %s",startValueFixed[((fmiSTRING*)tmpSV[j].variable)->fixed]);
								else{
									fprintf(pfile,"fixed = %s",startValueFixed[((fmiSTRING*)tmpSV[j].variable)->fixed]);
									existPre = 1;
								}
							}
							if(bool_defStart || bool_fixed) fprintf(pfile," %s",")");
							if(strcmp(varDesc,"") != 0) fprintf(pfile," \"%s\"",varDesc);
							fprintf(pfile,"%s",";\n");
							
							// fprintf(pfile,"\t%s %s String %s (start=\"%s\", fixed = %s) \"%s\";\n",causalityNames[tmpSV[j].causality],
							// variabilityNames[tmpSV[j].var],tmpSV[j].flatName,((fmiSTRING*)tmpSV[j].variable)->start,
							// startValueFixed[((fmiSTRING*)tmpSV[j].variable)->fixed],varDesc);
							
							if((tmpSV[j].var!=0) && (tmpSV[j].var!=1))	addOutputVariable(&tmpSV[j],&pntString,&tmpString,&noString);
							break;
						}
						case sv_enum:{
							printf("[%d]: #### Here Ok...\n",__LINE__);
						}
						default: {}
					}
				}
			}
			
		}		
		else{
			ERRORPRINT; fprintf(stderr,"#### Memory error: %s is empty...\n","tmpSV");
			exit(EXIT_FAILURE);
		}
		
		#ifdef _DEBUG_
			printf("#### noReal: %d\n",noReal);
			printf("#### noInteger: %d\n",noInteger);
			printf("#### noString: %d\n",noString);
			printf("#### noBoolean: %d\n",noBoolean);
		#endif
		if(fmuMD->ncs>0){
			fprintf(pfile,"\tparameter Integer %s = %d;\n",nx,fmuMD->ncs);
			fprintf(pfile,"\tReal %s[%s];\n",der_x,nx);
			fprintf(pfile,"\tReal %s[%s];\n",out_x,nx);
			fprintf(pfile,"\tReal %s[%s];\n",out_der_x,nx);
			
			if (noReal>0){
				tmpReal = pntReal;
				fprintf(pfile,"\tReal realV[%d];\n",noReal);
				fprintf(pfile,"\tInteger realVR[%d] = {",noReal);
				if(noReal==1) fprintf(pfile,"%d};\n",tmpReal->vr);				
				else if(noReal>1){					
					for(j = 0; j<noReal-1; j++){
						fprintf(pfile,"%d, ",tmpReal->vr);						
						tmpReal = tmpReal->next;
					}
					fprintf(pfile,"%d};\n",tmpReal->vr);
				}				
			}
			
			if (noInteger>0){
				tmpInteger = pntInteger;
				fprintf(pfile,"\tInteger integerV[%d];\n",noInteger);
				fprintf(pfile,"\tInteger integerVR[%d] = {",noInteger);
				if(noInteger==1) fprintf(pfile,"%d};\n",tmpInteger->vr);
				else if(noInteger>1){
					for(j=0; j<noInteger-1; j++){
						fprintf(pfile,"%d, ",tmpInteger->vr);
						tmpInteger = tmpInteger->next;
					}
					fprintf(pfile,"%d};\n",tmpInteger->vr);
				}
			}
			if (noBoolean>0){
				tmpBoolean = pntBoolean;
				fprintf(pfile,"\tBoolean booleanV[%d];\n",noBoolean);
				fprintf(pfile,"\tInteger booleanVR[%d] = {",noBoolean);
				if(noBoolean==1) fprintf(pfile,"%d};\n",tmpBoolean->vr);
				else if(noBoolean>1){
					for(j=0; j<noBoolean-1; j++){
						fprintf(pfile,"%d, ",tmpBoolean->vr);
						tmpBoolean = tmpBoolean->next;
					}
					fprintf(pfile,"%d};\n",tmpBoolean->vr);
				}
			}
			if (noString>0){
				tmpString = pntString;
				fprintf(pfile,"\tString stringV[%d];\n",noString);
				fprintf(pfile,"\tInteger stringVR[%d] = {",noString);
				if(noString==1) fprintf(pfile,"%d};\n",tmpString->vr);
				else if(noString>1){
					for(j=0; j<noString-1; j++){
						fprintf(pfile,"%d, ",tmpString->vr);
						tmpString = tmpString->next;
					}
					fprintf(pfile,"%d};\n",tmpString->vr);
				}
			}

			fprintf(pfile,"\treplaceable Real %s[%s];\n",x,nx);
		}
		if(fmuMD->nei>0){
			fprintf(pfile,"\tparameter Integer %s = %d;\n",nz,fmuMD->nei);
			fprintf(pfile,"\tReal %s[%s];\n",z,nz);
			fprintf(pfile,"\tReal %s[%s];\n",prez,nz);
			fprintf(pfile,"\tReal %s[%s];\n",zXprez,nz);
			fprintf(pfile,"\tBoolean %s[%s] \"flag for state events\";\n",flagSE,nz);
			fprintf(pfile,"\tBoolean %s[%s] \"indicator for state events\";\n",indSE,nz);
		}
		fprintf(pfile,"\treplaceable parameter Real relTol = 0.0001;\n");   
		fprintf(pfile,"\tparameter Integer %s = 0;\n",defaultVar);   
		fprintf(pfile,"protected\n");
		fprintf(pfile,"\tfmuModelInst inst = fmuModelInst(fmufun, instName, guid, functions, logFlag);\n");
		fprintf(pfile,"\tfmuEventInfo evtInfo = fmuEventInfo();\n");
		fprintf(pfile,"\tfmuBoolean timeEvt = fmuBoolean(%s);\n",defaultVar);
		fprintf(pfile,"\tfmuBoolean stepEvt = fmuBoolean(%s);\n",defaultVar);
		fprintf(pfile,"\tfmuBoolean stateEvt = fmuBoolean(%s);\n",defaultVar);
		fprintf(pfile,"\tfmuBoolean interMediateRes = fmuBoolean(%s);\n",defaultVar);
		//fprintf(pfile,"\tfmuBoolean freeAll = fmuBoolean(default);\n");
		fprintf(pfile,"\tfmuFunctions fmufun = fmuFunctions(\"%s\",dllPath);\n",fmuMD->mid);
		fprintf(pfile,"\tfmuCallbackFuns functions = fmuCallbackFuns();\n");
		fprintf(pfile,"initial algorithm\n");
		fprintf(pfile,"\tfmuSetTime(fmufun, inst, time);\n");
		fprintf(pfile,"\tfmuInit(fmufun, inst, tolControl, relTol, evtInfo);\n");
		if(fmuMD->ncs>0){
			fprintf(pfile,"\t%s:=fmuGetContStates(fmufun, inst, %s);\n",x,nx);
			
			#ifdef _DEBUG_MODELICA
			fprintf(pfile,"\tprintVariable(%s, %s, \"%s, initial algorithm\");\n",x,nx,x);
			#endif
			
			fprintf(pfile,"algorithm \n");
			fprintf(pfile,"\t%s:=fmuGetDer(fmufun, inst, %s, %s);\n",der_x,nx,x);
			fputs("\tfmuSetTime(fmufun, inst, time);",pfile);
			fprintf(pfile,"\t%s:=%s;\n",y,der_x);
			
			fprintf(pfile,"equation\n");
			fprintf(pfile,"\tder(%s) = %s;\n",x,der_x);
			
			if(noReal>0){
				int *aliasInd = (int *)calloc(noReal,sizeof(int));
				tmpReal = pntReal;
				for(j = 0; j<noReal; j++){
					aliasInd[j] = tmpReal->aliasInd;					
					tmpReal = tmpReal->next;
				}
				
				fprintf(pfile,"\trealV = fmuGetRealVR(fmufun, inst, %d, realVR);\n",noReal);
				tmpReal = pntReal;
				fprintf(pfile,"\t{");
				if(noReal==1) fprintf(pfile,"%s} = realV;\n",tmpReal->name);				
				else if(noReal>1){					
					for(j = 0; j<noReal-1; j++){
						fprintf(pfile,"%s, ",tmpReal->name);						
						tmpReal = tmpReal->next;
					}
					fprintf(pfile,"%s} = realV*diagonal({",tmpReal->name);
					for(j = 0; j<noReal-1; j++) fprintf(pfile,"%d,",aliasInd[j]);
					fprintf(pfile,"%d});\n",aliasInd[noReal-1]);
				}
				free(aliasInd);
			}
			if(noInteger>0){
				int *aliasInd = (int *)calloc(noInteger,sizeof(int));
				tmpInteger = pntInteger;
				for(j = 0; j<noInteger; j++){
					aliasInd[j] = tmpInteger->aliasInd;					
					tmpInteger = tmpInteger->next;
				}
				
				fprintf(pfile,"\tintegerV = fmuGetIntegerVR(fmufun, inst, %d, integerVR);\n",noInteger);
				tmpInteger = pntInteger;
				fprintf(pfile,"\t{");
				if(noInteger==1) fprintf(pfile,"%s} = integerV;\n",tmpInteger->name);				
				else if(noInteger>1){					
					for(j = 0; j<noInteger-1; j++){
						fprintf(pfile,"%s, ",tmpInteger->name);						
						tmpInteger = tmpInteger->next;
					}
					fprintf(pfile,"%s} = integerV*diagonal({",tmpInteger->name);
					for(j = 0; j<noInteger-1; j++) fprintf(pfile,"%d,",aliasInd[j]);
					fprintf(pfile,"%d});\n",aliasInd[noInteger-1]);
				}
				free(aliasInd);
			}
			if(noBoolean>0){
				int *aliasInd = (int *)calloc(noBoolean,sizeof(int));				
				tmpBoolean = pntBoolean;
				for(j = 0; j<noReal; j++){
					aliasInd[j] = tmpBoolean->aliasInd;					
					tmpBoolean = tmpBoolean->next;
				}
				
				fprintf(pfile,"\tbooleanV = fmuGetBooleanVR(fmufun, inst, %d, booleanVR);\n",noBoolean);
				tmpBoolean = pntBoolean;
				fprintf(pfile,"\t{");
				if(noBoolean==1) fprintf(pfile,"%s} = booleanV;\n",tmpBoolean->name);				
				else if(noBoolean>1){					
					for(j = 0; j<noBoolean-1; j++){
						fprintf(pfile,"%s, ",tmpBoolean->name);						
						tmpBoolean = tmpBoolean->next;
					}
					fprintf(pfile,"%s} = booleanV;\n",tmpBoolean->name);
				}
				free(aliasInd);
			}
			if(noString>0){
				int *aliasInd = (int *)calloc(noString,sizeof(int));				
				tmpString = pntString;
				for(j = 0; j<noString; j++){
					aliasInd[j] = tmpString->aliasInd;					
					tmpString = tmpString->next;
				}
				
				fprintf(pfile,"\tstringV = fmuGetStringVR(fmufun, inst, %d, stringVR);\n",noString);
				tmpString = pntString;
				fprintf(pfile,"\t{");
				if(noString==1) fprintf(pfile,"%s} = stringV;\n",tmpString->name);				
				else if(noString>1){					
					for(j = 0; j<noString-1; j++){
						fprintf(pfile,"%s, ",tmpString->name);						
						tmpString = tmpString->next;
					}
					fprintf(pfile,"%s} = stringV;\n",tmpString->name);
				}
				free(aliasInd);
			}
			
			fprintf(pfile,"algorithm\n");
			fprintf(pfile,"\tfmuSetContStates(fmufun, inst, %s, %s);\n",nx,x);
			
			#ifdef _DEBUG_MODELICA
			fprintf(pfile,"\tprintVariable(time, 1, \"time\");\n");
			fprintf(pfile,"\tprintVariable(%s, %s, \"nx\");\n",x,nx);
			#endif
		}
		fprintf(pfile,"\tfmuCompIntStep(fmufun, inst, stepEvt);\n");
		if(fmuMD->nei>0){
			fprintf(pfile,"\t%s:=%s;\n",prez,z);
			fprintf(pfile,"\t%s:=fmuGetEventInd(fmufun, inst, %s);\n",z,nz);
			
			for(j=1;j<=fmuMD->nei;j++){
				fprintf(pfile,"\t%s[%d] := %s[%d]>0;\n",flagSE,j,z,j);
				fprintf(pfile,"\t%s[%d] := change(%s[%d]);\n",indSE,j,flagSE,j);
				fprintf(pfile,"\t%s[%d] := %s[%d] * %s[%d];\n",zXprez,j,z,j,prez,j);
			}
			//fprintf(pfile,"algorithm \n");
			fprintf(pfile,"\tfmuStateEvtCheck(stateEvt, %s, %s, %s);\n",nz,z,prez);
		}
		fprintf(pfile,"\tfmuEvtUpdate(fmufun, inst, evtInfo, timeEvt, stepEvt, stateEvt, interMediateRes);\n");
		if(fmuMD->ncs>0){
			fprintf(pfile,"\t%s:=fmuGetContStates(fmufun, inst, %s);\n",out_x,nx);
			fprintf(pfile,"\t%s:=fmuGetDer(fmufun, inst, %s, %s);\n",out_der_x,nx,out_x);
			fprintf(pfile,"equation\n");
			if(fmuMD->nei>0){
				fprintf(pfile,"\twhen %s then\n",indSE);
				for(j=1;j<=fmuMD->ncs;j++){
					fprintf(pfile,"\t\treinit(%s[%d], %s[%d]);\n",x,j,out_x,j);
					fprintf(pfile,"\t\treinit(%s[%d], %s[%d]);\n",der_x,j,out_der_x,j);
				}
				fprintf(pfile,"\tend when;\n");
			}
		}
		
		// fprintf(pfile,"equation\n");
		fprintf(pfile,"\twhen terminal() then\n");
		//fprintf(pfile,"\t\t freeAll:=fmuFreeAll(inst, fmufun, functions);\n");
		fprintf(pfile,"\tend when;\n");
		fputs("end FMUBlock;\n",pfile);
		fprintf(pfile,"end FMUImport_%s;\n",fmuMD->mid);
	}
	
	// Free memory
	free(id);
	free((void*)x);
	free((void*)nx);
	free((void*)out_x);
	free((void*)der_x);
	free((void*)out_der_x);
	free((void*)nz);
	free((void*)z);
	free((void*)prez);
	free((void*)zXprez);
	free((void*)y);
	free((void*)flagSE);
	free((void*)indSE);
	free((void*)defaultVar);
	
	// free(pntReal);
	// free(tmpReal);
	// free(pntInteger);
	// free(tmpInteger);
	// free(pntBoolean);
	// free(tmpBoolean);
	// free(pntString);
	// free(tmpString);
	fclose(pfile);
}

void printUsage() {
  printf("Usage: fmigenerator [--fmufile=NAME] [--outputdir=PATH]\n");
  printf("--fmufile	The FMU file that contains the model description to be imported.\n");
  printf("--outputdir	The output directory.");
}

int main(int argc, char *argv[]){

	// Declaration of variables
	size_t nsv;							// number of scalar variables
	fmiScalarVariable* list = NULL;		// list of fmiScalarVariable;
	fmiModelVariable* fmuMV = NULL; // pointer to model variables
	fmuModelDescription* fmuMD = NULL;  // pointer to the tree-like structure of paresed xml	
	char omPath[PATHSIZE]; // OpenModelica installation directory
	
	// Allocated memory needed to be freed or file needed to be closed
	FILE* logFile;
	
	const char * fmuname;	// name of the fmu file <fmuname>.fmu
	ModelDescription* md = NULL;            // handle to the parsed XML file
	const char * decompPath = NULL;			// name of the decompression path
	const char * xmlFile = NULL;			// model description xml file name
	const char * fmuDllPath = NULL;			// dll path
	fmuMD = (fmuModelDescription*)calloc(1,sizeof(fmuModelDescription));
	// end
	
	// Creating log file
	// errLogFile = fopen("fmuImportError.log","a+");
	// if(!errLogFile){
		// printf("#### %s, %s, %d, : Error: Creating file fmuImportError.log failed...\n",__DATE__,__TIME__,__LINE__);
		// exit(EXIT_FAILURE);
	// }
	printf("\n\n");
	// end
	if (argc < 2) {
		printUsage();
		ERRORPRINT; fprintf(stderr,"#### Wrong arguments passed to main entry...%s\n","");
		exit(EXIT_FAILURE);
	}
	else if (strcasecmp(argv[1], "-h") == 0) {
		printUsage();
		exit(EXIT_FAILURE);
	}
	else if (strcasecmp(argv[1], "--help") == 0) {
		printUsage();
		exit(EXIT_FAILURE);
	}
	// //	The path of FMU as an argument
	// if (argc<=1) {
		// printf("#### Please give the fmu file...\ngenerator.exe <fmupath>\n");
		// ERRORPRINT("#### Please give the fmu file like: generator.exe <fmupath>%s\n","");
		// freeElement(md);
		// return EXIT_FAILURE;
	// }
	// // end
	
	// Attaining the name of the FMU from the argument 
	// Decompressing the archive into an appropriate directory
	// Get the model description xml file name
	if (strncmp(argv[1], "--fmufile=", 10) == 0) {
		fmuname = getFMUname(argv[1] + 10);
	}
	if (argc > 2) {
		if (strncmp(argv[2], "--outputdir=", 12) == 0) {
		  decompPath = (char*) malloc(strlen(argv[2]) - 11);
		  strcpy((char*)decompPath, argv[2] + 12);
		  strcat((char*)decompPath, "/");
		  if (access(decompPath, F_OK) == -1) {
			free((char*) decompPath);
			decompPath = getDecompPath(omPath, fmuname);
		  }
		}
		else{
			printUsage();
			ERRORPRINT; fprintf(stderr,"#### Wrong argument for decompression path...%s\n","");
			exit(EXIT_FAILURE);
		}
	}
	else {
		decompPath = getDecompPath(omPath, fmuname);
	}
	if(decompPath==NULL){
		printf("#### path for decompression is not available: decompPath = %s\n","NULL");
		ERRORPRINT; fprintf(stderr,"#### path for decompression is not available: decompPath = %s\n","NULL");
		exit(EXIT_FAILURE);
	}
	
	#ifdef _DEBUG_
		printf("#### %s decompPath: %s\n",QUOTEME(__LINE__),decompPath);
	#endif
	decompress(argv[1] + 10,decompPath);
	xmlFile = getNameXMLfile(decompPath,MODEL_DESCRIPTION);
	if(xmlFile == NULL){
		printf("#### model description xml file is not available: xmlFile = %s\n","NULL");
		ERRORPRINT; fprintf(stderr,"#### model description xml file is not available: xmlFile = %s\n","NULL");
		exit(EXIT_FAILURE);
	}
	// end
	
	// Parsing the model description file as a tree-like structure
	md = parse(xmlFile);
	if(md == NULL){
		printf("#### parsing model description xml file occurred error: md = %s\n","NULL");
		ERRORPRINT; fprintf(stderr,"#### parsing model description xml file occurred error: md = %s\n","NULL");
		exit(EXIT_FAILURE);
	}
	//end
	
	// Creating the tree-like data structure for code generation
	nsv = getNumberOfSV(md);

	if(nsv>0){
	list = (fmiScalarVariable*)calloc(nsv,sizeof(fmiScalarVariable));
	instScalarVariable(md,list);
	}
	else{
		printf("#### There is no scalar variable in the model %s\n",fmuname);
		ERRORPRINT; fprintf(stderr,"#### There is no scalar variable in the model %s\n",fmuname);
		exit(EXIT_FAILURE);
	}
	
	fmuMV = (fmiModelVariable*)calloc(1,sizeof(fmiModelVariable));
	fmuMV->list_sv = list;
	fmuMV->nsv = nsv;
	
	instFmuModelDescription(md,fmuMD,fmuMV);
	// end
	
	# ifdef _DEBUG_
	printf("#### sizeof(fmiScalarVariable): %d, sizeof(list): %d\n",sizeof(fmiScalarVariable),sizeof(list));
	#endif
	fmuDllPath = getDllPath(decompPath,fmuMD->mid);

	// Code generation
	tmpcodegen(fmuMD,decompPath);
	//printFileTest();
	blockcodegen(fmuMD,decompPath,fmuDllPath);
	#ifdef PRINT_INFORMATION
	printf("#### ModelIdentifier fmuMD->mid = %s\n",fmuMD->mid);
	printf("#### fmuMD->guid = %s\n",fmuMD->guid);
    printf("#### NumberOfStates fmuMD->ncs = %d\n",fmuMD->ncs);
	printf("#### NumberOfEventIndicators fmuMD->nei = %d\n",fmuMD->nei);
	printf("#### NumberOfScalarVariables fmuMD->modelVariable->nsv = %d\n",fmuMD->modelVariable->nsv);
	#endif
	// end
	#ifdef _IMPORT_LOG_
	logFile = fopen("fmuImport.log","w");
	LOGPRINT("FMU decompression path: %s\n",decompPath);
	LOGPRINT("FMU name: %s\n",fmuname);
	LOGPRINT("FMU model identifier: %s\n",fmuMD->mid);
	LOGPRINT("Modelica model name: %s.mo\n",fmuMD->mid);
	#endif
	// Free allocated and close file memory	
	free((void*)xmlFile);
	free((void*)fmuDllPath);
	free((void*)fmuname);
	free((void*)decompPath);
	free(list);
	freeScalarVariableLst(list,nsv);
	freeElement((void*)md);
	// fclose(errLogFile);
	fclose(logFile);
	// end
  //system("PAUSE");	
  return EXIT_SUCCESS;
}