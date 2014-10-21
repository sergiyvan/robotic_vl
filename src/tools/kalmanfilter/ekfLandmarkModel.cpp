#include "ekfLandmarkModel.h"

#include "utils/math/rotationMatrix.h"


/*------------------------------------------------------------------------------------------------*/

EkfLandmarkModel::EkfLandmarkModel()
	: genericEKF(nullptr)
	, lastCorrectionTS(0)
	, cfgTTL(0)
	, cfgNoiseX(0)
	, cfgNoiseY(0)
	, msmtUpdate_pitchVariance(0)
	, msmtUpdate_yawVariance(0)
	, msmtUpdate_XVariance(0)
	, msmtUpdate_YVariance(0)
{
}
/*----------------------------------------------------------------------------*/
EkfLandmarkModel::EkfLandmarkModel(EkfLandmarkModel const& model)
: genericEKF(nullptr) {
	*this = model;
}
/*----------------------------------------------------------------------------*/

EkfLandmarkModel::~EkfLandmarkModel() {
	if (genericEKF != nullptr) {
		delete genericEKF;
	}
}

/*------------------------------------------------------------------------------------------------*/
void EkfLandmarkModel::init(const PositionRelative& perceptRelative, robottime_t _time, double _sigma)
{
	ASSERT(genericEKF == nullptr);
	// state
	arma::colvec state;
	state << perceptRelative.getX().value() << perceptRelative.getY().value() << arma::endr;
	/* state.print("state"); */


	genericEKF = new GenericEKF(state, _sigma);

	lastCorrectionTS = _time;
	ASSERT(genericEKF != nullptr);
}

/*------------------------------------------------------------------------------------------------*/
/**
 * Predict the current state with the information of the control data
 * @param control
 * @param dt
 */
void EkfLandmarkModel::predict(const arma::colvec3& control, Second dt)
{
	ASSERT(genericEKF != nullptr);
	// We need to set the following data to call predict(...)
	// - G: jocobian for state prediction
	// - R: noise matrix for process
	// - predicted_state

	// PREDICT
	// lot's of tmp vars

	arma::colvec2 translationControl;
	translationControl << control(0) << control(1) << arma::endr;
	/* translationControl.print("translationControl"); */

	arma::mat22 rotMat = getRotationMatrix22(-control(2) * radians);
	/* rotMat.print("rotMat"); */

	// R noise matrix
	arma::mat R = arma::zeros(2, 2);
	R(0, 0) = cfgNoiseX * Second(dt).value();
	R(1, 1) = cfgNoiseY * Second(dt).value();

	genericEKF->predictEKF(rotMat, -translationControl, R);
}

/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Do the correction step for the landmark perception.
 *
 * Only the math. Nothing gets set.
 *
 * @Note only call this if a ball was seen
 */
void EkfLandmarkModel::correctAsSpherical(const PositionRelative& percept, robottime_t _time, Centimeter cameraHeight)
{
	ASSERT(genericEKF != nullptr);
	// some useful vars
	// position of state
	Centimeter x = genericEKF->getState()(0) * centimeters;
	auto xx = x * x;
	Centimeter y = genericEKF->getState()(1) * centimeters;
	auto  yy = y * y;
	// distance of x and y
	Centimeter muNorm = sqrt((xx + yy).value()) * centimeters;
	auto muNorm3 = muNorm * muNorm * muNorm;
	// robot height
	Centimeter r = cameraHeight;
	auto r2 = r * r;

	// measurement_z
	arma::colvec measurement_z;

	double pitchInRad = atan2(r.value(), percept.getDistanceToMyself().value());
	double yawInRad   = atan2(percept.getY().value(), percept.getX().value());

	measurement_z << pitchInRad << yawInRad << arma::endr;

	// predicted measurement
	arma::colvec predicted_measurement = getPredictedMeasurement(cameraHeight);

	// H
	arma::mat H;
	H << (-((r * x) / (muNorm3 + r2 * muNorm))).value()
	  << (-((r * y) / (muNorm3 + r2 * muNorm))).value()
	  << arma::endr
	  << (- (y / (xx + yy))).value()
	  << (x / (xx + yy)).value()
	  << arma::endr;

	// Q noise matrix
	arma::mat Q = arma::zeros(2, 2);
	Q(0, 0) = msmtUpdate_pitchVariance;
	Q(1, 1) = msmtUpdate_yawVariance;
	arma::colvec diff = measurement_z - predicted_measurement;
	genericEKF->correctEKF(H, diff, Q);
	lastCorrectionTS = _time;
}

/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Do the correction step for the landmark perception.
 *
 * Only the math. Nothing gets set.
 *
 * @Note only call this if a ball was seen
 */
void EkfLandmarkModel::correct(const PositionRelative& percept, robottime_t _time)
{
	ASSERT(genericEKF != nullptr);

	// measurement_z
	arma::colvec measurement_z;
	measurement_z << percept.getX().value() << percept.getY().value() << arma::endr;

	// predicted measurement
	arma::colvec predicted_measurement;
	predicted_measurement << genericEKF->getState()(0)
	                      << genericEKF->getState()(1) << arma::endr;

	// H
	arma::mat H;
	H << 1 << 0 << arma::endr
	  << 0 << 1 << arma::endr;

	// Q noise matrix
	arma::mat Q = arma::zeros(2, 2);
	Q(0, 0) = msmtUpdate_XVariance;
	Q(1, 1) = msmtUpdate_YVariance;
	arma::colvec diff = measurement_z - predicted_measurement;
	genericEKF->correctEKF(H, diff, Q);
	lastCorrectionTS = _time;
}
/*------------------------------------------------------------------------------------------------*/
void EkfLandmarkModel::invalidate() {
	if (genericEKF != nullptr) {
		delete genericEKF;
		genericEKF = nullptr;
	}
	ASSERT(genericEKF == nullptr);
}

/*------------------------------------------------------------------------------------------------*/
bool EkfLandmarkModel::isValid() const {
	return genericEKF != nullptr;
}

void EkfLandmarkModel::setPosition(const PositionRelative& pos) {
	if (genericEKF == nullptr) return;
	arma::colvec state;
	state << pos.getX().value() << pos.getY().value() << arma::endr;
	genericEKF->setState(state);
}

/*------------------------------------------------------------------------------------------------*/

void EkfLandmarkModel::set(PositionRelative const& pos) {
	if (genericEKF != nullptr) {
		delete genericEKF;
	}
	if (pos.isValid()) {
		arma::colvec state;
		state << pos.getX().value() << pos.getY().value() << arma::endr;
		genericEKF = new GenericEKF(state);
	}
}

/*------------------------------------------------------------------------------------------------*/
void EkfLandmarkModel::updateParameters(Millisecond _cfgTTL,
                                        double _cfgProcX, double _cfgProcY,
	                                    double _cfgMeasPitch, double _cfgMeasYaw,
				                        double _cfgMeasX, double _cfgMeasY) {
	cfgTTL = _cfgTTL;
	cfgNoiseX = _cfgProcX;
	cfgNoiseY = _cfgProcY;
	msmtUpdate_pitchVariance = _cfgMeasPitch;
	msmtUpdate_yawVariance = _cfgMeasYaw;
	msmtUpdate_XVariance = _cfgMeasX;
	msmtUpdate_YVariance = _cfgMeasY;
}

/*------------------------------------------------------------------------------------------------*/

PositionRelative EkfLandmarkModel::getPosition() const {
	if (genericEKF == nullptr) {
		return PositionRelative();
	}

	ASSERT(genericEKF != nullptr);
	return PositionRelative(genericEKF->getState()(0) * centimeters,
	                        genericEKF->getState()(1) * centimeters);
}
arma::mat const& EkfLandmarkModel::getCovariance() const {
	ASSERT(genericEKF != nullptr);
	return genericEKF->getSigma();
}


Millisecond EkfLandmarkModel::getTTL() const {
	return cfgTTL;
}

arma::colvec EkfLandmarkModel::getPredictedMeasurement(Centimeter cameraHeight) const {
	ASSERT(genericEKF != nullptr);
	Centimeter x = genericEKF->getState()(0) * centimeters;
	Centimeter y = genericEKF->getState()(1) * centimeters;
	Centimeter muNorm = sqrt((x*x + y*y).value()) * centimeters;
	Centimeter r = cameraHeight;

	// predicted measurement
	arma::colvec predicted_measurement;
	predicted_measurement << atan2(r.value(), muNorm.value())
	                      << atan2(y.value(), x.value())
	                      << arma::endr;
	return predicted_measurement;
}
robottime_t EkfLandmarkModel::getLastCorrection() const {
	return lastCorrectionTS;
}
arma::colvec const& EkfLandmarkModel::getState() const {
	static arma::colvec2 r = arma::zeros(2, 1);
	if (genericEKF == nullptr) {
		WARNING("This shouldn't happen: ekfLandmarkModel::getState");
		return r;
	}

	ASSERT(genericEKF != nullptr);
	return genericEKF->getState();
}
arma::mat const& EkfLandmarkModel::getSigma() const {
	static arma::mat r = arma::eye(2, 2);
	if (genericEKF == nullptr) {
		WARNING("This shouldn't happen: ekfLandmarkModel::getSigma");
		return r;
	}
	ASSERT(genericEKF != nullptr);
	return genericEKF->getSigma();
}
arma::mat EkfLandmarkModel::calculateMeasurementCovS(Centimeter cameraHeight) const {
	ASSERT(genericEKF != nullptr);

	Centimeter x = genericEKF->getState()(0) * centimeters;
	auto xx = x * x;
	Centimeter y = genericEKF->getState()(1) * centimeters;
	auto  yy = y * y;
	// distance of x and y
	Centimeter muNorm = sqrt((xx + yy).value()) * centimeters;
	auto muNorm3 = muNorm * muNorm * muNorm;
	// robot height
	Centimeter r = cameraHeight;
	auto r2 = r * r;

	// H
	arma::mat H;
	H << (-((r * x) / (muNorm3 + r2 * muNorm))).value()
	  << (-((r * y) / (muNorm3 + r2 * muNorm))).value()
	  << arma::endr
	  << (- (y / (xx + yy))).value()
	  << (x / (xx + yy)).value()
	  << arma::endr;

	// Q noise matrix
	arma::mat Q = arma::zeros(2, 2);
	Q(0, 0) = msmtUpdate_pitchVariance;
	Q(1, 1) = msmtUpdate_yawVariance;

	return H * genericEKF->getSigma() * H.t() + Q;
}
EkfLandmarkModel& EkfLandmarkModel::operator=(EkfLandmarkModel const& model) {
	if (this == &model)
		return *this;

	if (genericEKF != nullptr) {
		delete genericEKF;
		genericEKF = nullptr;
	}
	if (model.genericEKF != nullptr) {
		genericEKF = new GenericEKF(*model.genericEKF);
	}
	lastCorrectionTS = model.lastCorrectionTS;
	cfgTTL = model.cfgTTL;
	cfgNoiseX = model.cfgNoiseX;
	cfgNoiseY = model.cfgNoiseY;
	msmtUpdate_pitchVariance = model.msmtUpdate_pitchVariance;
	msmtUpdate_yawVariance = model.msmtUpdate_yawVariance;
	msmtUpdate_XVariance = model.msmtUpdate_XVariance;
	msmtUpdate_YVariance = model.msmtUpdate_YVariance;

	return *this;
}

