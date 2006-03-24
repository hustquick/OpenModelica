/*!  \file parser.g
* \author Ingemar Axelsson
* \brief Describes a grammar for notebooks.
* 
* This file contains a grammar describing a part of the notebook file 
* format. The parser mostly builds a ast. This ast is traversed by the 
* treeparser \see walker.g. 
*
* The parser has a lookahead of 2. This should be sufficient.
*
* \todo Check the grammar. It is certainly not completley correct, see
*  the mathematica documentation to find rules and attributes that is not
*  taken care of. This must be done eventually (Ingemar Axelsson)
*
* \todo It must be easy to make extensions to the grammar. Therefore a
*  correct grammar should be used. But I do not have time to verify that
*  it is completly correct. I stop when it works for most of the files.
*  (Ingemar Axelsson)
*/


header "pre_include_hpp"{

}

header "post_include_hpp"{

}

options
{
    language="Cpp";     //Generate C++ languages.
    genHashLines=false; //Do not generate hashlines.
}

class AntlrNotebookParser extends Parser;
options
{
    k=2;
    importVocab = notebookgrammar;
    buildAST=true;
}

document      
    :   expr
    ;

expr
    : (MODULENAME THICK)* exprheader
    | value
    | attribute
    ;

exprheader
    : NOTEBOOK^           LBRACK! expr (COMMA! rule)* RBRACK!
    | LIST^               LBRACK! (listbody)* (COMMA! listbody)* RBRACK!
    | LIST_SMALL^         LBRACK! (listbody)* (COMMA! listbody)* (COMMA! rule)* RBRACK!
    | CELL^          LBRACK! expr (COMMA! expr)? (COMMA! rule)* RBRACK!
    | CELLGROUPDATA^ LBRACK! expr (COMMA! (CELLGROUPOPEN|CELLGROUPCLOSED)) RBRACK!
    
    | TEXTDATA^      LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | STYLEBOX^      LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | STYLEDATA^     LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | SUPERSCRBOX^   LBRACK! expr COMMA! expr RBRACK! 
    | SUBSCRBOX^     LBRACK! expr COMMA! expr RBRACK!
    | SUBSUPERSCRIPTBOX^ LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | UNDERSCRIPTBOX^ LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | OVERSCRIPTBOX^ LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | UNDEROVERSCRIPTBOX^ LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | FRACTIONBOX^   LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | SQRTBOX^       LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | RADICALBOX^    LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | ROWBOX^        LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | GRIDBOX^       LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | FORMBOX^       LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | TAGBOX^        LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | COUNTERBOX^    LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | ADJUSTMENTBOX^ LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | BUTTONBOX^     LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | INTERPRETATIONBOX^  LBRACK! expr COMMA! expr RBRACK!
    | ANNOTATION^         LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | EQUAL^              LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | DIAGRAM^            LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | ICON^               LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | POLYGON^            LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | ELLIPSE^            LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | LINE^               LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | GRAYLEVEL^          LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | NOT_MATH_OLEDATE^   LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | RGBCOLOR^           LBRACK! NUMBER COMMA! NUMBER COMMA! NUMBER RBRACK!
    | FILENAME^           LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | BOXDATA^            LBRACK! expr (COMMA! expr)* (COMMA! rule)* RBRACK!
    | GRAPHICSDATA^	      LBRACK! QSTRING COMMA! QSTRING (COMMA! rule!)* RBRACK!
    | DIREXTEDINFINITY^   LBRACK! NUMBER RBRACK!
    | NOT_MATH_STARTMODELEDITOR!  LBRACK! RBRACK!
    | PARENTDIRECTORY!            LBRACK! RBRACK!
    ;

listbody
    : (expr|rule)
    ;

rule
    : RULE^          LBRACK! expr (COMMA! expr)  RBRACK!
    | RULE_SMALL^    LBRACK! expr (COMMA! expr)  RBRACK!
    | RULEDELAYED^   LBRACK! expr (COMMA! expr)  RBRACK!
    ;

value 
    : QSTRING
    | NUMBER
    | TRUE_
    | FALSE_
    | VALUERIGHT
    | VALUELEFT
    | VALUECENTER
    | VALUESMALLER
    | INHERITED
    | PAPERWIDTH
    | WINDOWWIDTH
    | TRADITIONALFORM
    | STANDARDFORM
    | INPUTFORM
    | OUTPUTFORM
    | DEFAULTINPUTFORMATTYPE
    | AUTOMATIC
    | NONESYM
    | NULLSYM!
    | ALLSYM
    ;

attribute
    :   FONTSLANT       
    |   FONTSIZE        
    |   FONTCOLOR       
    |   FONTWEIGHT      
    |   FONTFAMILY      
    |   FONTVARIATIONS  
    |   TEXTALIGNMENT   
    |   TEXTJUSTIFICATION
    |   INITIALIZATIONCELL
    |   FORMATTYPE_TOKEN
    |   PAGEWIDTH
    |	PAGEHEADERS
    |   PAGEHEADERLINES
    |   PAGEFOOTERS
    |   PAGEFOOTERLINES
    |   PAGEBREAKBELOW
    |   PAGEBREAKWITHIN
    |   BOXMARGINS
    |   BOXBASELINESHIFT
    |   LINESPACING
    |   HYPHENATION
    |   ACTIVE_TOKEN
    |   VISIBLE_TOKEN
    |   EVALUATABLE
    |   BUTTONFUNCTION
    |   BUTTONDATA
    |   BUTTONEVALUATOR      
    |   BUTTONSTYLE     
    |   CHARACHTERENCODING
    |   SHOWSTRINGCHARACTERS
    |   SCREENRECTANGLE 
    |   AUTOGENERATEDPACKAGE
    |   AUTOITALICWORDS
    |   INPUTAUTOREPLACEMENTS
    |   SCRIPTMINSIZE
    |   STYLEMEMULISTING
    |   COUNTERINCREMENTS
    |   COUNTERASSIGNMENTS
    |   PRIVATEEVALOPTIONS
    |   GROUPPAGEBREAKWITHIN
    |   DEFAULTFORMATTYPE
    |   NUMBERMARKS
    |   LINEBREAKADJUSTMENTS
    |   VISIOLINEFORMAT
    |   VISIOFILLFORMAT
    |   EXTENT
	|   NAMEPOSITION
    |   CELLTAGS
    |   CELLFRAME
    |   CELLFRAMECOLOR
    |   CELLFRAMELABELS
    |   CELLFRAMEMARGINS
	|   CELLFRAMELABELMARGINS
	|   CELLLABRLMARGINS
	|   CELLLABELPOSITIONING
    |   CELLMARGINS
    |   CELLDINGBAT
    |   CELLHORIZONTALSCROLL
    |   CELLOPEN
    |   CELLGENERATED
    |   SHOWCELLBRACKET
    |   SHOWCELLLABEL
    |	CELLBRACKETOPT
    |   EDITABLE     
    |   BACKGROUNT
    |   CELLGROUPINGRULES
    |   WINDOWSIZE     
    |   WINDOWMARGINS  
    |   WINDOWFRAME    
    |   WINDOWELEMENTS 
    |   WINDOWTITLE    
    |   WINDOWTOOLBARS 
    |   WINDOWMOVEABLE 
    |   WINDOWFLOATING 
    |   WINDOWCLICKSELECT
    |   STYLEDEFINITIONS
    |   FRONTENDVERSION
    |   SCREENSTYLEENV
    |   PRINTINGSTYLEENV
    |   PRINTINGOPTIONS
    |   PRINTINGCOPIES
    |   PRINTINGPAGERANGE
    |   PRIVATEFONTOPTIONS
    |   MAGNIFICATION
    |   GENERATEDCELL
    |   CELLAUTOOVRT
    |   IMAGESIZE
    |   IMAGEMARGINS     
    |   IMAGEREGION      
    |   IMAGERANGECACHE 
    |   IMAGECACHE
    |   NOT_MATH_MODELEDITOR  
    ;
