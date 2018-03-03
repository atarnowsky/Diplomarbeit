#include "gazeapi.h"

// Inline helper functions ---

inline std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

namespace WeGA {

// TrackingResult ---

TrackingResult::TrackingResult() :
    pupil(vector2(0,0)),
    glint(vector2(0,0))
{
    for(int n = 0; n < 4; n++)
    {
        this->corners[n] = vector2(0,0);
        this->cornerStatus[n] = Missing;
    }
}

TrackingResult::TrackingResult(const vector2& pupil,
                               const vector2& glint,
                               vector2 corners[4],
                               CornerStatus cornerStatus[4]) :
    pupil(pupil),
    glint(glint)
{
    for(int n = 0; n < 4; n++)
    {
        this->corners[n] = corners[n];
        this->cornerStatus[n] = cornerStatus[n];
    }
}

vector2 TrackingResult::pupilPosition()  const
{
    return pupil;
}

vector2 TrackingResult::glintPosition()  const
{
    return glint;
}

vector2 TrackingResult::pupilGlintVector()  const
{
    return pupil - glint;
}

vector2 TrackingResult::cornerVector(CornerPosition position) const
{
    return corners[(int)position] - glint;
}

std::pair<vector2, TrackingResult::CornerStatus> TrackingResult::corner(CornerPosition position)  const
{
    assert(position < 5 && position >= 0);
    return std::make_pair(corners[position], cornerStatus[position]);
}

std::string TrackingResult::toString()
{
    std::stringstream buf;
    buf << glintPosition().x    << ";" << glintPosition().y    << ";"
        << pupilPosition().x    << ";" << pupilPosition().y    << ";"        
        << corners[0].x         << ";" << corners[0].y         << ";"
        << corners[1].x         << ";" << corners[1].y         << ";"
        << corners[2].x         << ";" << corners[2].y         << ";"
        << corners[3].x         << ";" << corners[3].y         << ";"
        << cornerStatus[0]      << ";" << cornerStatus[1]      << ";"
        << cornerStatus[2]      << ";" << cornerStatus[3];

    return buf.str();
}

TrackingResult TrackingResult::fromString(const std::string& string, unsigned int o)
{
    std::vector<std::string> splitted(split(string, ';'));
    assert(splitted.size()-o >= 16);

    vector2 corners[4] = {
        vector2(atof(splitted[4+o].c_str()), atof(splitted[5+o].c_str())),
        vector2(atof(splitted[6+o].c_str()), atof(splitted[7+o].c_str())),
        vector2(atof(splitted[8+o].c_str()), atof(splitted[9+o].c_str())),
        vector2(atof(splitted[10+o].c_str()), atof(splitted[11+o].c_str()))
    };

    CornerStatus status[4] = {
        (CornerStatus)atoi(splitted[12+o].c_str()),
        (CornerStatus)atoi(splitted[13+o].c_str()),
        (CornerStatus)atoi(splitted[14+o].c_str()),
        (CornerStatus)atoi(splitted[15+o].c_str())
    };

    return TrackingResult(
                vector2(atof(splitted[2+o].c_str()), atof(splitted[3+o].c_str())),
                vector2(atof(splitted[0+o].c_str()), atof(splitted[1+o].c_str())),
                corners, status);
}

// MappingResult ---

MappingResult::MappingResult()
{
    MappingResult(vector2(0,0));
}

MappingResult::MappingResult(const vector2& position) :
    pos(position)
{

}

vector2 MappingResult::position() const
{
    return pos;
}

vector2i MappingResult::scaledPosition(vector2i screenSize) const
{
    return vector2i(pos.x * screenSize.x, pos.y * screenSize.y);
}

std::string MappingResult::toString()
{
    std::stringstream buf;
    buf << pos.x << ";" << pos.y;
    return buf.str();
}

MappingResult MappingResult::fromString(const std::string& string, unsigned int o)
{
    std::vector<std::string> splitted(split(string, ';'));
    assert(splitted.size()-o >= 2);

    return MappingResult(
                vector2(atof(splitted[0+o].c_str()), atof(splitted[1].c_str())));
}

}
