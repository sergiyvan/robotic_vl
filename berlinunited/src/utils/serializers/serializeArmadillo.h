/*
 * serializeArmadillo.h
 *
 *  Created on: Aug 18, 2014
 *      Author: dseifert
 */

#ifndef SERIALIZEARMADILLO_H_
#define SERIALIZEARMADILLO_H_

#include "ModuleFramework/Serializer.h"

template<class Archive, class MAT>
void serializeArmadillo(Archive & ar, MAT & mat) {
	for (uint c = 0; c < mat.n_cols; c++)
		for (uint r = 0; r < mat.n_rows; r++)
			ar & mat.at(r, c);
}

#endif

