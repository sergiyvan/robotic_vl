#ifndef EKFMOVABLEMODEL_H
#define EKFMOVABLEMODEL_H

#include "tools/kalmanfilter/genericEKF.h"
#include "tools/position.h"


/*----------------------------------------------------------------------------*/
class EkfMovableModel
{
public:
	EkfMovableModel(arma::colvec const& _state, arma::mat const& _R, arma::mat const& _Q,
	                double _friction);
	virtual ~EkfMovableModel() {}

	void predictEKF(PositionRelative _move, Degree _rot, Second _dt);
	void correctEKF(arma::mat const& _measurement, Centimeter cameraHeight);

	arma::mat const&    getSigma() const;
	arma::colvec const& getState() const;

	arma::mat const& getR() const;
	arma::mat const& getQ() const;


private:
	GenericEKF genericEKF;
	arma::mat R;
	arma::mat Q;
	double friction;
};

#endif

