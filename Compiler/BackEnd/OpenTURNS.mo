/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Open Source Modelica Consortium (OSMC)
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Link�ping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

encapsulated package OpenTURNS "
  file:        OpenTURNS.mo
  package:     OpenTURNS
  description: Connection to OpenTURNS, software for uncertainty propagation. The connection consists of two parts
  1. Generation of python script as input to OPENTURNS
  2. Generation and compilation of wrapper that calls the standard OpenModelica simulator and retrieves the final values 
  (after simulation has stopped at endTime) 

  RCS: $Id: OpenTURNS.mo 10958 2012-01-25 01:12:35Z wbraun $
  "

public 
import BackendDAE;
import DAE;
import Absyn;
import Env;

protected
import CevalScript;
import SimCode;
import Interactive;
import System;
import BackendDump;
import BackendDAEUtil;
import List;
import BackendVariable;
import ExpressionDump;
import ComponentReference;
import BackendDAEOptimize;
import Expression;
import Util;

public function generateOpenTURNSInterface "generates the dll and the python script for connections with OpenTURNS"
  input Env.Cache cache;
  input Env.Env env;
  input BackendDAE.BackendDAE inDaelow;
  input DAE.FunctionTree inFunctionTree;
  input Absyn.Path inPath;
  input Absyn.Program inProgram;
  input DAE.DAElist inDAElist;
  input String templateFile "the filename to the template file (python script)";
  output String scriptFile "the name of the generated file";
  
  protected
  String cname_str,fileNamePrefix,fileDir;
  list<String> libs;
  BackendDAE.BackendDAE dae,strippedDae;  
  SimCode.SimulationSettings simSettings;
algorithm
  cname_str := Absyn.pathString(inPath);
  fileNamePrefix := cname_str;
      
  simSettings := CevalScript.convertSimulationOptionsToSimCode(
    CevalScript.buildSimulationOptionsFromModelExperimentAnnotation(
      Interactive.setSymbolTableAST(Interactive.emptySymboltable,inProgram),inPath,fileNamePrefix)
  );
  // correlation matrix form (vector of records) currently not supported by OpenModelica backend, remove it .
  //print("enter dae:");
  //BackendDump.dump(inDaelow);
  strippedDae := stripCorrelationFromDae(inDaelow);
  
  strippedDae := BackendDAEUtil.getSolvedSystem(cache, env, strippedDae, NONE(), NONE(), NONE(),NONE());
  
  //print("strippedDae :");
  //BackendDump.dump(strippedDae);  
  (dae,libs,fileDir,_,_,_) := SimCode.generateModelCode(strippedDae,inProgram,inDAElist,inPath,cname_str,SOME(simSettings),Absyn.FUNCTIONARGS({},{}));
  
  //print("..compiling, fileNamePrefix = "+&fileNamePrefix+&"\n");  
  CevalScript.compileModel(fileNamePrefix , libs, fileDir, "", "");
  (scriptFile) := generatePythonScript(fileNamePrefix,templateFile,cname_str,inDaelow);
end generateOpenTURNSInterface; 

protected function generatePythonScript "generates the python script as input to OpenTURNS, given a template file name"
  input String fileNamePrefix;
  input String templateFile;
  input String modelName;
  input BackendDAE.BackendDAE inDaelow;
  output String pythonFileName;
  protected
  String templateFileContent;
  String distributions,correlationMatrix;
  String collectionDistributions,inputDescriptions;
  String pythonFileContent;
  list<tuple<String,String>> distributionVarLst;
algorithm
  //print("reached\n");
  templateFileContent := System.readFile(templateFile);
  
  //generate the different parts of the python file 
  (distributions,distributionVarLst) := generateDistributions(inDaelow);
  (correlationMatrix) := generateCorrelationMatrix(inDaelow,listLength(distributionVarLst),List.map(distributionVarLst,Util.tuple21));
  collectionDistributions := generateCollectionDistributions(inDaelow,distributionVarLst);
  inputDescriptions  := generateInputDescriptions(inDaelow);    
  //
  pythonFileName := System.stringReplace(templateFile,"template",modelName);
  
  // Replace template markers
  pythonFileContent := System.stringReplace(templateFileContent,"<%distributions%>",distributions);
  pythonFileContent := System.stringReplace(pythonFileContent,"<%correlationMatrix%>",correlationMatrix);
  pythonFileContent := System.stringReplace(pythonFileContent,"<%collectionDistributions%>",collectionDistributions);
  pythonFileContent := System.stringReplace(pythonFileContent,"<%inputDescriptions%>",inputDescriptions);  
  
  //Write file
  //print("writing python script to file "+&pythonFileName+&"\n");
  System.writeFile(pythonFileName,pythonFileContent);
    
end generatePythonScript;

protected function generateDistributions "generates distibution code for the python script.
Goes throuh all variables and collects a set of unique distributions, then generates python code.

Example of python code.
distributionE = Beta(0.93, 3.2, 2.8e7, 4.8e7)
distributionF = LogNormal(30000, 9000, 15000, LogNormal.MUSIGMA)
distributionL = Uniform(250, 260)
distributionI = Beta(2.5, 4.0, 3.1e2, 4.5e2)
LogNormal_u = LogNormal(30000, 9000, 15000, LogNormal.MUSIGMA)
Corresponding Modelica model.

model A
  parameter Distribution distributionE = Beta(0.93, 3.2, 2.8e7, 4.8e7);
  parameter Distribution distributionF = LogNormal(30000, 9000, 15000, LogNormal.MUSIGMA);
  parameter Distribution distributionL = Uniform(250, 260);
  parameter Distribution distributionI = Beta(2.5, 4.0, 3.1e2, 4.5e2);
  Real u(distribution = LogNormal(30000, 9000, 15000, LogNormal.MUSIGMA)); // distribution name becomes <instancename>+\"_\"+<distribution name> 
end A;

"
  input BackendDAE.BackendDAE dae;
  output String distributions;
  output list<tuple<String,String>> distributionVarLst;
  
protected
  list<BackendDAE.Var> varLst;
  list<DAE.Distribution> dists;  
  list<String> sLst;
  String header;
  BackendDAE.BackendDAE dae2;
algorithm
    //print("enter, dae:");
  //BackendDump.dump(dae);
  dae2 := BackendDAEOptimize.removeParameters(dae);
  //print("removed parameters, dae2:");
  //BackendDump.dump(dae2);
  
  varLst := BackendDAEUtil.getAllVarLst(dae2); 
  varLst := List.select(varLst,BackendVariable.varHasDistributionAttribute);
  dists := List.map(varLst,BackendVariable.varDistribution);
  (sLst,distributionVarLst) := List.map1_2(List.threadTuple(dists,List.map(varLst,BackendVariable.varCref)),generateDistributionVariable,dae2);
  header := "# "+&intString(listLength(sLst))+&" distributions from Modelica model\n";
  distributions := stringDelimitList(header::sLst,"\n");
end generateDistributions;

protected function generateDistributionVariable " generates a distribution variable for the python script "
   input tuple<DAE.Distribution,DAE.ComponentRef> tpl;
   input BackendDAE.BackendDAE dae;
   output String str;
   output tuple<String,String> distributionVar "the name of the python variable for the distribution and the description (Modelica variable name), used later for generating the collection"; 
algorithm
  (str,distributionVar) := matchcontinue(tpl,dae)
  local 
    String name,args,varName,distVar;
    DAE.Exp arr,sarr,e1,e2,e3,e2_1,e3_1;
    DAE.ComponentRef cr;
    list<DAE.Exp> expl1,expl2;
    
    
    case((DAE.DISTRIBUTION(DAE.SCONST(name),DAE.ARRAY(array=expl1),DAE.ARRAY(array=expl2)),cr),dae) equation
      // e.g. distributionL = Beta(0.93, 3.2, 2.8e7, 4.8e7)
      // TODO:  make sure that the arguments are in correct order by looking at the expl2 list containing strings of argument names
      args = stringDelimitList(List.map(expl1,ExpressionDump.printExpStr),",");
      varName = ComponentReference.crefModelicaStr(cr);
      distVar ="distribution"+&varName; 
      str = distVar+& " = " +& name +& "("+&args+&")\n";
    then (str,(varName,distVar));
    case((DAE.DISTRIBUTION(e1,e2,e3),cr),dae) equation
      ((e2_1,_)) = BackendDAEUtil.extendArrExp((e2,(NONE(),false)));
      ((e3_1,_)) = BackendDAEUtil.extendArrExp((e3,(NONE(),false)));
      false = Expression.expEqual(e2,e2_1); // Prevent infinte recursion
      false = Expression.expEqual(e3,e3_1);
      //print("extended arr="+&ExpressionDump.printExpStr(e2_1)+&"\n");
      //print("extended sarr="+&ExpressionDump.printExpStr(e3_1)+&"\n");
      (str,distributionVar) = generateDistributionVariable((DAE.DISTRIBUTION(e1,e2_1,e3_1),cr),dae);
    then (str,distributionVar);
  end matchcontinue;
end generateDistributionVariable;

protected function generateCorrelationMatrix "
"
  input BackendDAE.BackendDAE dae;
  input Integer numDists "number of distributions == number of uncertain variables == size of correlation matrix";
  input list<String> uncertainVars;
  output String correlationMatrix;
protected
  array<DAE.Algorithm> algs; 
  DAE.Algorithm correlationAlg;
  String header;
algorithm 
  algs := BackendDAEUtil.getAlgorithms(dae);
  header := "RS = CorrelationMatrix("+&intString(numDists)+&")\n";
  SOME(correlationAlg) := Util.arrayFindFirstOnTrue(algs,hasCorrelationStatement);
  correlationMatrix := header +& generateCorrelationMatrix2(correlationAlg,uncertainVars);
end generateCorrelationMatrix;

protected function stripCorrelationFromDae "removes the correlation vector and it's corresponding algorithm section from the DAE.
This means that it must remove from three places, the variables, the equations and the algorithms"
  input BackendDAE.BackendDAE dae; 
  output BackendDAE.BackendDAE strippedDae;
protected
  BackendDAE.EqSystems eqs;
  BackendDAE.Shared shared;
  array<DAE.Algorithm> algs;
algorithm
  algs := BackendDAEUtil.getAlgorithms(dae);
  algs := stripCorrelationStatement(algs);
  BackendDAE.DAE(eqs,shared) := BackendDAEUtil.setAlgorithms(dae,algs);
  eqs := List.map(eqs,stripCorrelationVarsAndEqns);
  eqs := List.select(eqs,eqnSystemNotZero);
  strippedDae := BackendDAE.DAE(eqs,shared);
end stripCorrelationFromDae;

protected function eqnSystemNotZero "returns true if system contains variables and equations"
   input BackendDAE.EqSystem eqs; 
   output Boolean notZero;
protected
  BackendDAE.EquationArray eqns;
  BackendDAE.Variables vars;
 algorithm
   BackendDAE.EQSYSTEM(orderedVars = vars,orderedEqs=eqns) := eqs;
   notZero := BackendVariable.varsSize(vars) > 0 and BackendDAEUtil.equationSize(eqns) > 0;
 end eqnSystemNotZero;

protected function stripCorrelationStatement "help function"
  input array<DAE.Algorithm> algs;
  output array<DAE.Algorithm> outAlgs;
protected
  list<DAE.Algorithm> algLst;
algorithm
  algLst := arrayList(algs);
  (_,algLst) := List.splitOnTrue(algLst,hasCorrelationStatement);
  outAlgs := listArray(algLst);
end stripCorrelationStatement;
  
protected function stripCorrelationVarsAndEqns " help function "
  input BackendDAE.EqSystem eqsys;
  output BackendDAE.EqSystem outEqsys;
protected
  BackendDAE.Variables vars;
  BackendDAE.EquationArray eqns;
algorithm
  BackendDAE.EQSYSTEM(orderedVars=vars, orderedEqs = eqns)  := eqsys;
  vars := stripCorrelationVars(vars);
  eqns := stripCorrelationEqns(eqns); 
  outEqsys := BackendDAE.EQSYSTEM(vars,eqns,NONE(),NONE(),BackendDAE.NO_MATCHING());
end stripCorrelationVarsAndEqns;

protected function stripCorrelationEqns "help function "
  input BackendDAE.EquationArray eqns;
  output BackendDAE.EquationArray outEqns;
protected
  list<BackendDAE.Equation> eqnLst;
algorithm
  (_,eqnLst) := List.splitOnTrue(BackendDAEUtil.equationList(eqns),equationIsCorrelationBinding);
  outEqns := BackendDAEUtil.listEquation(eqnLst);
end stripCorrelationEqns;

protected function equationIsCorrelationBinding "help function"
  input BackendDAE.Equation eqn;
  output Boolean res;
algorithm
  res := matchcontinue(eqn)
  local 
    list<DAE.ComponentRef> crs;
    list<DAE.Exp> expl;
    
    case(BackendDAE.ALGORITHM(_,_,expl,_)) equation
      crs = List.flatten(List.map(expl,Expression.extractCrefsFromExp));
      true = List.reduce(List.map(crs,isCorrelationVarCref),boolAnd);
    then true;
    case(_) then false;
  end matchcontinue;
end equationIsCorrelationBinding;

protected function stripCorrelationVars " help function  "
  input BackendDAE.Variables vars;
  output BackendDAE.Variables outVars;
protected
  list<BackendDAE.Var> varLst;
algorithm
  (_,varLst) := List.splitOnTrue(BackendDAEUtil.varList(vars),isCorrelationVar);
  outVars := BackendDAEUtil.listVar(varLst); 
end stripCorrelationVars;

protected function isCorrelationVar "help function"
  input BackendDAE.Var var;
  output Boolean res;
algorithm
  res := matchcontinue(var)
  local 
    DAE.ComponentRef cr;
    
    case(var) equation
      cr = BackendVariable.varCref(var);
      true = isCorrelationVarCref(cr);      
    then true;
    case(_) then false;
  end matchcontinue;  
end isCorrelationVar;

protected function isCorrelationVarCref "help function"
 input DAE.ComponentRef cr;
 output Boolean res;
algorithm
  res := 0 == System.strcmp(ComponentReference.crefFirstIdent(cr),"correlation");  
end isCorrelationVarCref;
  
protected function hasCorrelationStatement " help function "
  input DAE.Algorithm alg;
  output Boolean res;
algorithm
  res := matchcontinue(alg)
  local 
    list<DAE.Statement> stmts;
    
    case(DAE.ALGORITHM_STMTS({})) then false;
    case(DAE.ALGORITHM_STMTS(DAE.STMT_ASSIGN_ARR(_,DAE.CREF_IDENT(ident="correlation"),_,_)::_)) then true;
    case(DAE.ALGORITHM_STMTS(_::stmts)) then hasCorrelationStatement(DAE.ALGORITHM_STMTS(stmts));
  end matchcontinue;
end hasCorrelationStatement;

protected function getCorrelationExp " help function "
  input DAE.Algorithm alg;
  output DAE.Exp res;
algorithm
  res := matchcontinue(alg)
  local 
    list<DAE.Statement> stmts;
    
    case(DAE.ALGORITHM_STMTS(DAE.STMT_ASSIGN_ARR(_,DAE.CREF_IDENT(ident="correlation"),res,_)::_)) then res;
    case(DAE.ALGORITHM_STMTS(_::stmts)) then getCorrelationExp(DAE.ALGORITHM_STMTS(stmts));
  end matchcontinue;
end getCorrelationExp;

protected function generateCorrelationMatrix2 "help function"
  input DAE.Algorithm correlationAlg;
  input list<String> uncertainVars;
  output String str;
protected 
  DAE.Exp exp;
  list<DAE.Exp> expl;
algorithm
  DAE.ARRAY(array=expl):= getCorrelationExp(correlationAlg);
  str := stringDelimitList(List.map1(expl,generateCorrelationMatrix3,uncertainVars),"\n");
end generateCorrelationMatrix2;

protected function generateCorrelationMatrix3 "help function"
  input DAE.Exp exp;
  input list<String> uncertainVars;
  output String str;
protected
  DAE.ComponentRef cr1,cr2;
  DAE.Exp val;
  String valStr;
  Integer p1,p2,plow,phigh;

algorithm
  DAE.CALL(path = Absyn.IDENT("Correlation"),expLst = {DAE.CREF(cr1,_),DAE.CREF(cr2,_),val}) := exp;
  valStr := ExpressionDump.printExpStr(val);
  p1 := List.position(ComponentReference.crefModelicaStr(cr1),uncertainVars);
  p2 := List.position(ComponentReference.crefModelicaStr(cr2),uncertainVars);
  plow := intMin(p1,p2);
  phigh := intMax(p1,p2);
  str := "RS["+&intString(plow)+&","+&intString(phigh)+&"] = "+&valStr;
end generateCorrelationMatrix3;   
  
protected function generateCollectionDistributions "
"
  input BackendDAE.BackendDAE dae;
  input list<tuple<String,String>> distributionVarLst;
  output String collectionDistributions;
protected
  String collectionVarName;
algorithm
  collectionVarName := "collectionMarginals";
  collectionDistributions := "# Initialization of the distribution collection:\n"
  +&collectionVarName+&" = DistributionCollection()\n"
  +&stringDelimitList(List.map1(distributionVarLst,generateCollectionDistributions2,(collectionVarName,dae)),"\n");   
end generateCollectionDistributions;

protected function generateCollectionDistributions2 "help function"
  input tuple<String,String> distVarTpl;
  input tuple<String,BackendDAE.BackendDAE> tpl;
  output String str;
algorithm
  str := Util.tuple21(tpl)+& ".add(Distribution(" +& Util.tuple22(distVarTpl) +& ",\"" +&Util.tuple21(distVarTpl)+&"\"))";  
end generateCollectionDistributions2;

protected function generateInputDescriptions "
"
  input BackendDAE.BackendDAE dae;
  output String inputDescriptions;
algorithm
  inputDescriptions := "# input descriptions currently not used, set above";   

end generateInputDescriptions;
    
end OpenTURNS;