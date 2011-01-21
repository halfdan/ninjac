/* 
 * File:   semant.h
 * Author: fate
 *
 * Created on 8. November 2010, 19:50
 */

#ifndef SEMANT_H
#define	SEMANT_H

#ifdef	__cplusplus
extern "C" {
#endif

Table **check(Absyn *fileTrees[], int numInFiles, boolean showSymbolTables);


#ifdef	__cplusplus
}
#endif

#endif	/* SEMANT_H */

