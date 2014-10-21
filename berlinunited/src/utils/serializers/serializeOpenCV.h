/*
 * serializeOpenCV.h
 *
 *  Created on: Sep 16, 2014
 *      Author: dseifert
 */

#ifndef SERIALIZEOPENCV_H_
#define SERIALIZEOPENCV_H_

#include "ModuleFramework/Serializer.h"

#include <opencv/cv.h>


namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, cv::Point2f & p, const unsigned int version)
{
	ar & p.x;
	ar & p.y;
}

template<class Archive>
void serialize(Archive & ar, cv::Point3f & p, const unsigned int version)
{
	ar & p.x;
	ar & p.y;
	ar & p.z;
}

} // namespace serialization
} // namespace boost

REGISTER_SERIALIZATION(cv::Point2f, 1)
REGISTER_SERIALIZATION(cv::Point3f, 1)

#endif

