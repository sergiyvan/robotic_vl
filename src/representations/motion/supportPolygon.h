/*
 * supportPolygon.h
 *
 *  Created on: 18.10.2014
 *      Author: lutz
 */

#ifndef SUPPORTPOLYGON_H_
#define SUPPORTPOLYGON_H_

#include "platform/hardware/robot/motorIDs.h"
#include "tools/kinematicEngine/kinematicTreeSupportPolygon.h"
#include <armadillo>
#include <vector>
#include <utility>
#include <map>

class SupportPolygon {
public:

	typedef KinematicTreeSupportPolygon Edges;
	typedef std::map<MotorID, KinematicTreeSupportPolygon> SupportNodes;

	SupportPolygon();
	virtual ~SupportPolygon();

	void setEdges(Edges edges) {
		m_edges = edges;
	}

	Edges const& getEdges() const {
		return m_edges;
	}

	void setSupportNodes(SupportNodes supportNodes) {
		m_supportNodes = supportNodes;
	}

	SupportNodes const& getSupportNodes() const {
		return m_supportNodes;
	}

	void setSupportNodesPositions(std::map<MotorID, std::map<MotorID, arma::mat44>> supportNodesPositions) {
		m_supportNodesPositions = supportNodesPositions;
	}

	std::map<MotorID, std::map<MotorID, arma::mat44>> const& getSupportNodesPositions() const {
		return m_supportNodesPositions;
	}

private:
	// the overall support Polygon
	Edges m_edges;

	// moving kinematicNodes (servos) aka. supportNodes participating to create this support polygon
	SupportNodes m_supportNodes;

	// where the supportNodes were when they touched the ground with respect to all other supportEffectors
	std::map<MotorID, std::map<MotorID, arma::mat44>> m_supportNodesPositions;
};

#endif /* SUPPORTPOLYGON_H_ */
