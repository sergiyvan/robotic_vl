#ifndef EKFLANDMARKMODEL_H
#define EKFLANDMARKMODEL_H

#include "tools/kalmanfilter/genericEKF.h"
#include "tools/position.h"


/*------------------------------------------------------------------------------------------------*/
class EkfLandmarkModel
{
public:
	EkfLandmarkModel();
	EkfLandmarkModel(EkfLandmarkModel const& model);
	virtual ~EkfLandmarkModel();

	void init(const PositionRelative& perceptRelative, robottime_t _time, double _sigma=50.);

	void predict(const arma::colvec3& control, Second dt);
	void correctAsSpherical(const PositionRelative& percept, robottime_t _time, Centimeter cameraHeight);
	void correct(const PositionRelative& percept, robottime_t _time);

	void invalidate();
	bool isValid() const;

	void set(PositionRelative const& pos);

	void updateParameters(Millisecond _cfgTTL, double _cfgProcX, double _cfgProcY,
	                      double _cfgMeasPitch, double _cfgMeasYaw,
						  double _cfgMeasX = 1., double _cfgMeasY = 1.);

	PositionRelative getPosition() const;
	arma::mat const& getCovariance() const;
	Millisecond getTTL() const;

	void setPosition(const PositionRelative& pos);

	arma::colvec getPredictedMeasurement(Centimeter cameraHeight) const;

	robottime_t getLastCorrection() const;

	arma::colvec const& getState() const;
	arma::mat const& getSigma() const;
	arma::mat calculateMeasurementCovS(Centimeter cameraHeight) const;

	EkfLandmarkModel& operator=(EkfLandmarkModel const& model);


private:

	GenericEKF* genericEKF;
	/* data */
	robottime_t lastCorrectionTS;

	// parameters
	Millisecond cfgTTL;
	double cfgNoiseX;
	double cfgNoiseY;
	double msmtUpdate_pitchVariance;
	double msmtUpdate_yawVariance;
	double msmtUpdate_XVariance;
	double msmtUpdate_YVariance;

};

#endif
