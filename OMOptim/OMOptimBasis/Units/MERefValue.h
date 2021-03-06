// $Id$
/**
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Open Source Modelica Consortium (OSMC),
 * c/o Linkpings universitet, Department of Computer and Information Science,
 * SE-58183 Linkping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR 
 * THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S ACCEPTANCE
 * OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3, ACCORDING TO RECIPIENTS CHOICE. 
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from OSMC, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 * Main contributor 2010, Hubert Thierot, CEP - ARMINES (France)

     @file MERefValue.h
     @brief Comments for file documentation.
     @author Hubert Thieriot, hubert.thieriot@mines-paristech.fr
     Company : CEP - ARMINES (France)
     http://www-cep.ensmp.fr/english/
     @version 

  */
#if !defined(_MEREFVALUE_H)
#define _MEREFVALUE_H

#include <QtCore/QObject>
#include <QtCore/QAbstractTableModel>
#include <QtCore/QTextStream>
#include <QtCore/QStringList>
#include "MOOptVector.h"
#include "Variable.h"


/** MERefValue is designed to contain value which could be either numerical or referential.
  * A referential value is a value which contains a variable name of a model.
  * Values concerned here are dimensioned values (\sa DimValue)
  */


template<class DimValue>
class MERefValue
{
public:
    MERefValue(QVariant value=QVariant(),int unit=0);
    virtual ~MERefValue(void);

    
    virtual unsigned nbUnits() const;
    QVariant value() const;
    virtual void setValue(QVariant,int iUnit=-1);
    virtual bool setValue(QVariant,QString unit);
        virtual void setValue(const MERefValue &);
        virtual void setValue(const DimValue &);
    bool setUnit(QString iUnit);
    void setUnit(int);
    QStringList units() const;
    QString unit() const;
    int iUnit() const;
    virtual QString unit(int iUnit) const;

        double numValue(MOOptVector *variables,int iUnit,bool &ok,QString modelName=QString()) const;
        DimValue numValue(MOOptVector *variables,bool &ok,QString modelName) const;
        DimValue numValue() const;

        QString reference() const ;
        bool isNum() const;
    
    
protected :
    int _unit;
    QVariant _value;
};

#include "MERefValue.cpp"

#endif
