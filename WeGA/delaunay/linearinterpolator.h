#ifndef LINEARINTERPOLATOR_H
#define LINEARINTERPOLATOR_H

#include "interfaces.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <CGAL/Interpolation_traits_2.h>
#include <CGAL/natural_neighbor_coordinates_2.h>
#include <CGAL/interpolation_functions.h>

namespace WeGA {

class LinearInterpolator : public Approximator<2,2>
{
public:
    typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
    typedef CGAL::Delaunay_triangulation_2<K>             Delaunay_triangulation;
    typedef CGAL::Interpolation_traits_2<K>               Traits;
    typedef K::FT                                         Coord_t;
    typedef K::Point_2                                    Point;


    LinearInterpolator();

    void addTrainingTuple(const vector<2>&  in,
                          const vector<2>& out);
    void clear();
    vector<2> evaluate(const vector<2>& input) const;

    void update() {}
    void update(scalar) {}

protected:
    Delaunay_triangulation delaunay;
    std::map<Point, Coord_t, K::Less_xy_2> screenCoordinatesX;
    std::map<Point, Coord_t, K::Less_xy_2> screenCoordinatesY;
};

}

#endif // LINEARINTERPOLATOR_H
