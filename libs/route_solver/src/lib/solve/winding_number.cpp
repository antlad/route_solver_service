//
// Created by vlad on 14.11.16.
//

#include "route_solver/solve/winding_number.hpp"

namespace rs {

double isLeft(point2d P0, point2d P1, point2d P2)
{
	return ((P1.x() - P0.x()) * (P2.y() - P0.y())
			- (P2.x() - P0.x()) * (P1.y() - P0.y()));
}

int cn_PnPoly(point2d P, const std::vector<point2d>& V)
{
	int cn = 0;    // the  crossing number counter

	// loop through all edges of the polygon
	for (std::size_t i = 0; i < V.size(); i++)      // edge from V[i]  to V[i+1]
	{
		if (((V[i].y() <= P.y()) && (V[i + 1].y() > P.y()))     // an upward crossing
				|| ((V[i].y() > P.y()) && (V[i + 1].y() <= P.y())))   // a downward crossing
		{
			// compute  the actual edge-ray intersect x-coordinate
			double vt = (P.y() - V[i].y()) / (V[i + 1].y() - V[i].y());

			if (P.x() < V[i].x() + vt * (V[i + 1].x() - V[i].x()))   // P.x < intersect
			{
				++cn;
			}   // a valid crossing of y=P.y right of P.x
		}
	}

	return (cn & 1);    // 0 if even (out), and 1 if  odd (in)
}

int wn_PnPoly(point2d P, const std::vector<point2d>& V)
{
	int wn = 0;    // the  winding number counter

	// loop through all edges of the polygon
	for (std::size_t i = 0; i < V.size(); i++)     // edge from V[i] to  V[i+1]
	{
		if (V[i].y() <= P.y())// m_start y <= P.y
		{
			if (V[i + 1].y() > P.y())// an upward crossing
			{
				if (isLeft(V[i], V[i + 1], P) > 0)    // P left of  edge
				{
					++wn;
				}
			}// have  a valid up intersect
		}
		else// m_start y > P.y (no test needed)
		{
			if (V[i + 1].y() <= P.y())       // a downward crossing
			{
				if (isLeft(V[i], V[i + 1], P) < 0)    // P right of  edge
				{
					--wn;
				}
			}// have  a valid down intersect
		}
	}

	return wn;
}

}
