#ifndef CORRESPONDENCEFILTER_H
#define CORRESPONDENCEFILTER_H

#include "representations/camera/cameraMatrix.h"

#include "tools/position.h"
#include "debug.h"
#include "utils/math/Math.h"
#include "utils/math/Common.h"

#include <armadillo>


/*------------------------------------------------------------------------------------------------*/
// TODO clean up this file!!
/*------------------------------------------------------------------------------------------------*/
const std::string DisplacementID("cognition.modeling.mhkf.displacement.text");

/*------------------------------------------------------------------------------------------------*/
/**
 * @brief return the euclidian distance between two point
 *
 * @return euclidian distance
 */
inline Centimeter getDistance(Centimeter x1, Centimeter y1, Centimeter x2, Centimeter y2)
{
	Centimeter diffX = x1 - x2;
	Centimeter diffY = y1 - y2;
	return sqrt((diffX * diffX + diffY * diffY).value()) * centimeters;
}

inline Centimeter getDistance(PositionAbsolute p1, PositionAbsolute p2) {
	PositionAbsolute diff = p1 - p2;
	return sqrt((diff.getX() * diff.getX() + diff.getY() * diff.getY()).value()) * centimeters;
}

/*------------------------------------------------------------------------------------------------*/
/**
 * @brief return the euclidian distance between two point and assuming the values are angles
 *
 * @return euclidian distance
 */
inline Radian getAngleDistance(Radian x1, Radian y1, Radian x2, Radian y2)
{
	Radian diffX = Math::angleSignedDelta(x1, x2);
	Radian diffY = Math::angleSignedDelta(y1, y2);

	return sqrt( (diffX*diffX + diffY*diffY).value()) * radians;
}

/*------------------------------------------------------------------------------------------------*/
inline const PositionAbsolute& getBestMatch(
    const PositionRelative& perceptRelative,
    const PositionAbsolute& alternativeA,
    const PositionAbsolute& alternativeB,
    const arma::colvec& state)
{
	const PositionAbsolute perceptAbsolute = perceptRelative.translateToAbsolute(
	            PositionRobot(state(0) * centimeters, state(1) * centimeters, Degree(state(2) * radians)));

	Centimeter displacementA = getDistance(perceptAbsolute, alternativeA);
	Centimeter displacementB = getDistance(perceptAbsolute, alternativeB);

	// if B is a better match --> use B
	if (displacementB < displacementA) {
		return alternativeB;
	}
	// if A is a better match --> use A
	else {
		return alternativeA;
	}
}

/*------------------------------------------------------------------------------------------------*/
inline const PositionAbsolute& getBestMatch(
    const PositionRelative& perceptRelative,
    const std::vector<PositionAbsolute>& alternatives,
    const arma::colvec& state)
{
	const PositionAbsolute perceptAbsolute = perceptRelative.translateToAbsolute(
	            PositionRobot(state(0) * centimeters, state(1) * centimeters, Degree(state(2) * radians)));

	return *std::min_element(alternatives.begin(), alternatives.end(),
	[&](const PositionAbsolute & a1, const PositionAbsolute & a2)->bool {
		Centimeter displacementA = getDistance(perceptAbsolute, a1);
		Centimeter displacementB = getDistance(perceptAbsolute, a2);
		return displacementA < displacementB;
	});
}



/*------------------------------------------------------------------------------------------------*/
typedef struct {
	//MHUKF::Polar measurement;
	PositionAbsolute landmark;
	double displacement;
} MsmtLandmarkCorrespondence;

typedef std::vector<MsmtLandmarkCorrespondence> MsmtLandmakrCerrespondenceVec;


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Filter observation and corresponding landmarks.
 *
 * TODO: Does this really need to be a class. A simple function should be ok.
 */
class CorrespondenceFilter {
public:
	CorrespondenceFilter(const CameraMatrix& cameraMatrix)
		: theCameraMatrix(cameraMatrix)
	{}

	template <typename T>
	MsmtLandmakrCerrespondenceVec identifyObservations(
	                               const std::vector<T>& observations,
	                               const std::vector<PositionAbsolute>& landmarks,
	                               const arma::colvec& robot) const
	{


		// - calc displacement for each feature-obs pair
		// - determine best match
		MsmtLandmakrCerrespondenceVec result;

		return result;
		/*
		for (T& obsElem : observations) {
			// skip invalid observations
			if (!obsElem->isValid())
				continue;

			const PositionRelative tmpRel(theCameraMatrix.translateToRelative(
						obsElem->basePoint.x, obsIter->basePoint.y));
			const Polar tmp = tmpRel.getAsPolar();

			const MHUKF::Polar obs(tmp.getR(), Math::fromDegrees(tmp.getTheta()));

			//ERROR("=============================");
			//ERROR("Observation at %f/%f", obs.range, obs.bearing);
			DEBUG_TEXT(DisplacementID,
				"Observation at %f/%f", obs.range, obs.bearing);

			std::vector<MHUKF::Polar> displacements;
			std::vector<double> diff;

			for(PositionAbsolute& landmark : landmarks) {
				MHUKF::Polar displacement = getDisplacement(obs, robot, landmark);

				displacements.push_back(displacement);

				// TODO find something better. is eucidean distance better?
				double t = displacement.bearing * 300 + displacement.range;
				diff.push_back(t);

				DEBUG_TEXT(DisplacementID,
				        "\tlandmark at abs(%d/%d) has displacement (%f/%f) value: %f",
				        landmark.x,
				        landmark.y,
				        displacement.range,
				        displacement.bearing,
				        t);
			}

			// find landmark with smallest difference
			size_t indexOfMin = std::distance(diff.begin(),
			                                  std::min_element(diff.begin(), diff.end()));
			// add it ot result
			MsmtLandmarkCorrespondence corr;
			corr.measurement = obs;
			corr.landmark = landmarks.at(indexOfMin);
			corr.displacement = diff.at(indexOfMin);
			result.push_back(corr);
		}
		*/
	};

private:
	CameraMatrix theCameraMatrix;
};

#endif /* CORRESPONDENCEFILTER_H */
