#include "delaunay/linearinterpolator.h"

namespace WeGA {

typedef CGAL::Data_access< std::map<LinearInterpolator::Point,
                                    LinearInterpolator::Coord_t,
                                    LinearInterpolator::K::Less_xy_2 > > Value_access;

LinearInterpolator::LinearInterpolator()
{
}

void LinearInterpolator::addTrainingTuple(const vector<2>& in, const vector<2>& out)
{
    Point p(in[0], in[1]);
    screenCoordinatesX.insert(std::make_pair(p, out[0]));
    screenCoordinatesY.insert(std::make_pair(p, out[1]));

    delaunay.insert(p);
}

void LinearInterpolator::clear()
{
    delaunay.clear();
}

vector<2> LinearInterpolator::evaluate(const vector<2>& input) const
{
    Point p(input[0], input[1]);
    std::vector< std::pair< Point, Coord_t > > coords;
    CGAL::Triple<
        std::back_insert_iterator<std::vector<std::pair<CGAL::Point_2<CGAL::Epick>, double> > >,
        K::FT, bool> neigh = CGAL::natural_neighbor_coordinates_2(delaunay, p,std::back_inserter(coords));
    Coord_t norm;

    neigh = CGAL::natural_neighbor_coordinates_2(delaunay, p,std::back_inserter(coords));
    norm = neigh.second;

    Coord_t resX =  CGAL::linear_interpolation(coords.begin(), coords.end(), norm, Value_access(screenCoordinatesX));
    Coord_t resY =  CGAL::linear_interpolation(coords.begin(), coords.end(), norm, Value_access(screenCoordinatesY));
    vector<2> result;
    result[0] = resX * 0.5;
    result[1] = resY * 0.5;

    return result;
}

}
