#pragma once
#include "PVRAssets/PVRAssets.h"
#include "PVRCore/Math/Plane.h"
#include <deque>
// Describes the different types of way, used for tiling.
namespace WayTypes {
enum WayTypes
{
	Road,
	Parking,
	Building,
	Inner,
	Default,
};
}

// Describes the different types of road used by the system.
namespace RoadTypes {
enum RoadTypes
{
	Motorway,
	Trunk,
	Primary,
	Secondary,
	Other,
	Service,
	None
};
}

namespace BuildingType {
enum BuildingType
{
	Shop, Bar, Cafe, FastFood, Pub, College,
	Library, University, ATM, Bank, Restaurant,
	Doctors, Dentist, Hospital, Pharmacy,
	Cinema, Casino, Theatre, FireStation,
	Courthouse, Police, PostOffice, Toilets,
	PlaceOfWorship, PetrolStation, Parking,
	Other, PostBox, Veterinary, Embassy,
	HairDresser, Butcher, Optician, Florist,
	None //'None' needs to be last.
};
}

// Describes the sides of a 2D bounding box.
namespace Sides {
enum Sides
{
	Left,
	Top,
	Right,
	Bottom,
	NoSide,
};
}

namespace LOD {
enum Levels
{
	L0, L1, L2, L3, L4, Count, LabelLOD = L4, IconLOD = L3, AmenityLabelLOD = L3
};
}

// Stores the minimum and maximum latitude & longitude of the map.
struct Bounds
{
	glm::dvec2 min;
	glm::dvec2 max;
};

// Stores a key-value pair.
struct Tag
{
	std::string key;
	std::string value;
};

struct IntersectionData
{
	std::vector<uint64_t> nodes;
	std::vector<std::pair<uint32_t, glm::uvec2>> junctionWays;
	bool isBound;
};

struct BoundaryData
{
	bool consumed;
	uint32_t index;
};

/* A struct which stores node data.
Id - Unique node identification number,
index - Used for index drawing of nodes,
coords - Location in 2D space,
texCoords - Texture co-ordinates for AA lines,
wayIds - Ids of ways that this node belongs to,
tileBoundNode - Is this node on a tile boundary. */
struct Vertex
{
	uint64_t id;
	uint32_t index;
	glm::dvec2 coords;
	glm::vec2 texCoords;
	std::vector<uint64_t> wayIds;

	Vertex(uint64_t userId = 0,
	       glm::dvec2 userCoords = glm::dvec2(0, 0),
	       bool userTileBoundNode = false,
	       glm::vec2 userTexCoords = glm::vec2(-10000.f, -10000.f))
	{
		id = userId;
		coords = userCoords;
		texCoords = userTexCoords;
		index = 0;
	}

	Vertex& operator=(Vertex&& rhs)
	{
		id = rhs.id;
		index = rhs.index;
		coords = rhs.coords;
		texCoords = rhs.texCoords;
		wayIds = std::move(rhs.wayIds);
		return *this;
	}
	Vertex(Vertex&& rhs): id(rhs.id), index(rhs.index), coords(rhs.coords), texCoords(rhs.texCoords), wayIds(std::move(rhs.wayIds))
	{}

	Vertex& operator=(const Vertex& rhs)
	{
		id = rhs.id;
		index = rhs.index;
		coords = rhs.coords;
		texCoords = rhs.texCoords;
		wayIds = rhs.wayIds;
		return *this;
	}
	Vertex(const Vertex& rhs) : id(rhs.id), index(rhs.index), coords(rhs.coords), texCoords(rhs.texCoords), wayIds(rhs.wayIds)
	{}
};

struct LabelData
{
	std::string name;
	glm::dvec2 coords;
	float rotation;
	float scale;
	uint64_t id;
	float distToBoundary;
	float distToEndOfSegment;
};

struct IconData
{
	BuildingType::BuildingType buildingType;
	glm::dvec2 coords;
	float scale;
	uint64_t id;
};

struct AmenityLabelData : public LabelData
{
	IconData iconData;
};

struct RouteData
{
	glm::dvec2 point;
	float distanceToNext;
	float rotation;

	RouteData() : point(glm::dvec2(0.0)), distanceToNext(0.0f), rotation(0.0f) {}
};

// Element: Ordered series of nodes used to represent linear features or boundaries
class Way
{
public:
	uint64_t id;
	std::vector<uint64_t> nodeIds;
	double width;
	bool area;
	bool inner;
	bool isIntersection;
	bool isRoundabout;
	std::vector<Tag> tags;
	RoadTypes::RoadTypes roadType;
};

// This is used to store road ways that have been converted into triangles
class ConvertedWay : public Way
{
public:
	std::vector<std::array<uint64_t, 3>> triangulatedIds;

	ConvertedWay(uint64_t userId = 0, bool userArea = false,
	             const std::vector<Tag>& userTags = std::vector<Tag> {}, RoadTypes::RoadTypes type = RoadTypes::None,
	             double roadWidth = 0, bool intersection = false, bool roundabout = false)
	{
		id = userId;
		area = userArea;
		tags = userTags;
		roadType = type;
		width = roadWidth;
		isIntersection = intersection;
		isRoundabout = roundabout;
	}
};

// Structure for storing data for an individual map tile.
struct Tile
{
	glm::dvec2 min;
	glm::dvec2 max;
	glm::vec2 screenMin;
	glm::vec2 screenMax;

	std::map<uint64_t, Vertex> nodes;
	std::vector<Way> areaWays;
	std::vector<Way> roadWays;
	std::vector<Way> parkingWays;
	std::vector<Way> buildWays;
	std::vector<Way> innerWays;
	std::vector<LabelData> labels[LOD::Count];
	std::vector<AmenityLabelData> amenityLabels[LOD::Count];
	std::vector<IconData> icons[LOD::Count];

	struct VertexData
	{
		glm::vec2 pos;
		glm::vec2 texCoord;

		VertexData(glm::vec2 position, glm::vec2 textureCoord = glm::vec2(1, 1))
			: pos(position), texCoord(textureCoord) {}
	};

	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

// Structure for storing data from OSM file.
struct OSM
{
	double lonTileScale;
	double latTileScale;
	uint32_t numCols;
	uint32_t numRows;
	glm::dvec2 minLonLat;
	glm::dvec2 maxLonLat;

	Bounds bounds;
	std::map<uint64_t, Vertex> nodes;
	std::vector<uint64_t> original_intersections;
	std::vector<LabelData> labels[LOD::Count];
	std::vector<AmenityLabelData> amenityLabels[LOD::Count];
	std::vector<IconData> icons[LOD::Count];
	std::set<std::string> uniqueIconNames;

	std::map<uint64_t, Way> originalRoadWays;
	std::map<uint64_t, ConvertedWay> convertedRoads;
	std::map<uint64_t, Way> parkingWays;
	std::map<uint64_t, Way> buildWays;
	std::map<uint64_t, Way> triangulatedRoads;

	std::vector<RouteData> route;

	std::vector<std::vector<Tile>> tiles;

	Way& getOriginalRoadWay(uint64_t wayId)
	{
		return originalRoadWays.find(wayId)->second;
	}
	const Way& getOriginalRoadWay(uint64_t wayId)const
	{
		return originalRoadWays.find(wayId)->second;
	}
	Way& getTriangulatedRoadWay(uint64_t wayId)
	{
		return triangulatedRoads.find(wayId)->second;
	}
	const Way& getTriangulatedRoadWay(uint64_t wayId)const
	{
		return triangulatedRoads.find(wayId)->second;
	}
	const Vertex& getNodeById(uint64_t nodeId) const
	{
		return nodes.find(nodeId)->second;
	}
	Vertex& getNodeById(uint64_t nodeId)
	{
		return nodes.find(nodeId)->second;
	}
	Tile& getTile(uint32_t x, uint32_t y)
	{
		return tiles[x][y];
	}
	Tile& getTile(glm::uvec2 tileCoords)
	{
		return tiles[tileCoords.x][tileCoords.y];
	}
	Vertex& insertOrOverwriteNode(Vertex&& node)
	{
		return (nodes[node.id] = std::move(node));
	}
	Vertex& createNode(uint64_t id)
	{
		Vertex& node = nodes[id];
		node.id = id;
		return node;
	}
	void cleanData();
};

// Remap an old co-ordinate into a new coordinate system.
template<typename T>
inline T remap(const T valueToRemap, const T oldmin, const T oldmax, const T newmin, const T newmax)
{
	return ((valueToRemap - oldmin) / (oldmax - oldmin)) * (newmax - newmin) + newmin;
}

typedef uint64_t NodeId;
typedef glm::dvec2 Vec2;
typedef double Real;

const double boundaryBufferX = 0.05;
const double boundaryBufferY = 0.05;

/*!*****************************************************************************
Class NavDataProcess This class handles the loading of OSM data from an XML file
and pre-processing (i.e. triangulation) the raw data into usable rendering data.
********************************************************************************/
class NavDataProcess
{
public:
	//Constructor takes a stream which the class uses to read the XML file.
	NavDataProcess(std::unique_ptr<pvr::Stream> stream)
	{
		_assetStream = std::move(stream);
	}

	//These functions should be called before accessing the tile data to make sure the tiles have been initialised.
	pvr::Result loadAndProcessData();
	void initTiles(const glm::ivec2& screenDimensions); //Call after window width / height is known
	//Call this function at the end of the application to tidy up data stored by tiles.
	void releaseTileData();

	//Public accessor function to tiles.
	inline std::vector<std::vector<Tile>>& getTiles() { return _osm.tiles; }
	inline uint32_t getNumRows() const { return _osm.numRows; }
	inline uint32_t getNumCols() const { return _osm.numCols; }
	inline std::vector<RouteData>& getRouteData() { return _osm.route; }
	inline glm::dvec2 getBoundsMin() const { return _osm.bounds.min; }
	inline glm::dvec2 getBoundsMax() const { return _osm.bounds.max; }
	const OSM& getOsm() const { return _osm; }
private:
	// OSM data object.
	OSM _osm;
	glm::ivec2 _windowsDim;
	std::unique_ptr<pvr::Stream> _assetStream;

	//Raw data handling fuctions
	pvr::Result loadOSMData();
	glm::dvec2 lonLatToMetres(const glm::dvec2 origin, const glm::dvec2 point) const;
	void generateIcon(uint64_t* nodeIds, size_t numNodeIds, Tag* tags, size_t numTags, uint64_t id);
	void processLabels(glm::dvec2 mapWorldDim);
	void calculateRoute();


	//Linear mathematics functions
	std::array<glm::dvec2, 2> findPerpendicularPoints(const glm::dvec2 firstPoint, const glm::dvec2 secPoint, const double width, const int pointNum) const;
	std::array<glm::dvec2, 2> findPerpendicularPoints(const glm::dvec2 firstPoint, const glm::dvec2 secPoint, const glm::dvec2 thirdPoint, const double width) const;
	std::array<glm::dvec2, 2> circleIntersects(const glm::vec2 centre, const double r, const double m, const double constant) const;
	glm::dvec3 findIntersect(const glm::dvec2 minBounds, const glm::dvec2 maxBounds, const glm::dvec2 inPoint, const glm::dvec2 outPoint) const;
	glm::dvec2 calculateMidPoint(glm::dvec2 p1, glm::dvec2 p2, glm::dvec2 p3) const;

	//Road triangulation functions
	void triangulateAllRoads();
	void calculateIntersections();
	void convertToTriangleList();
	std::vector<uint64_t> triangulateRoad(const std::vector<uint64_t>& nodeIds, const double width);
	std::vector<uint64_t> tessellate(const std::vector<uint64_t>& nodeIds, Real width);

	//Polygon triangulation functions
	void triangulate(std::vector<uint64_t>& nodeIds, std::vector<std::array<uint64_t, 3>>& triangles) const;
	pvr::PolygonWindingOrder checkWinding(const std::vector<uint64_t>& nodeIds) const;
	pvr::PolygonWindingOrder checkWinding(const std::vector<glm::dvec2>& points) const;
	double calculateTriangleArea(const std::vector<glm::dvec2>& points) const;

	//Map tiling functions
	void initialiseTiles();
	void sortTiles();
	void fillLabelTiles(LabelData& label, uint32_t lod);
	void fillAmenityTiles(AmenityLabelData& label, uint32_t lod);
	void fillIconTiles(IconData& icon, uint32_t lod);

	void processLabelBoundary(LabelData& label, glm::uvec2& tileCoords);

	//Map tiling utility functions
	void insertWay(std::vector<Way>& insertIn, Way& way);
	void insert(const glm::uvec2& tileCoords, WayTypes::WayTypes type, Way* w = nullptr, uint64_t id = 0);

	glm::ivec2 findTile(const glm::dvec2& point) const;
	glm::ivec2 findTile2(glm::dvec2& point) const;
	bool isOutOfBounds(const glm::dvec2& point) const;
	bool isTooCloseToBoundary(const glm::dvec2& point) const;
	bool findMapIntersect(glm::dvec2& point1, glm::dvec2& point2) const;

	//Texture co-ordinate calculation functions
	std::array<uint64_t, 2> calculateEndCaps(Vertex& first, Vertex& second, double width);
	struct RoadParams;

	void clipRoad(
	  const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2,
	  const glm::uvec2& tileMin, const glm::uvec2& tileMax, const RoadParams& roadParams);

	void clipRoad(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2, const uint64_t wayId,
	              const std::vector<Tag>& wayTags, WayTypes::WayTypes wayType, bool area = false, RoadTypes::RoadTypes roadType = RoadTypes::None,
	              double roadWidth = 0.f, bool isIntersection = false, bool isRoundabout = false);

	void clipAgainst(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2,
	                 glm::vec2 planeOrigin, const glm::vec2& planeNorm, Vertex* triFront, Vertex* triBack,
	                 uint32_t& numTriFront, uint32_t& numTriBack);

	void recurseClipRoad(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2,
	                     const glm::uvec2& tileMin, const glm::uvec2& tileMax, const RoadParams& roadParams, bool isPlaneVertical);
};
static const float epsilon = 0.00001f;

//Comparator functions
template<typename T>
inline bool isRealEqual(const T a, const T b) { return glm::abs(a - b) < epsilon; }
//Comparator functions
template<typename Vec2>
inline bool isVectorEqual(Vec2 a, Vec2 b) { return isRealEqual(a.x, b.x) && isRealEqual(a.y, b.y); }

inline bool compareX(const Vertex* const a, const Vertex* const b) { return a->coords.x < b->coords.x; }
inline bool compareY(const Vertex* const a, const Vertex* const b) { return a->coords.y < b->coords.y; }
inline bool compareRoadTypes(const Way& a, const Way& b) { return (int(a.roadType) < int(b.roadType)); }
inline bool compareID(const Vertex* const a, const Vertex* const b) { return a->id == b->id; }

/*!*********************************************************************************************************************
\return Find intersection point of 2 lines.
\param  p1  Point on line 1.
\param  d1  Direction vector of line 1.
\param  p2  Point on line 2.
\param  d2  Direction vector of line 2.
\brief  Find intersection point of 2 lines (assumes they intersect).
***********************************************************************************************************************/
template<typename Vec2>
inline bool rayIntersect(const Vec2& p0, const Vec2& d0, const Vec2& p1, const Vec2& d1, typename Vec2::value_type& outDistanceD0,
                         Vec2& outIntersectionPoint)
{
	typedef glm::detail::tvec3<typename Vec2::value_type, glm::precision::defaultp> Vec3;
	outIntersectionPoint = p0;
	outDistanceD0 = 0;
	if (isVectorEqual(p0, p1))
	{
		return true;
	}
	bool retval = pvr::math::intersectLinePlane(p0, d0, p1, -pvr::math::getPerpendicular(d1), outDistanceD0, epsilon);
	if (retval)
	{
		outIntersectionPoint = p0 + d0 * outDistanceD0;
	}
	else if (glm::length(glm::cross(Vec3(p0 - p1, 0), Vec3(d0, 0))) < epsilon)//COINCIDENT!
	{
		outDistanceD0 = 0.5;
		outIntersectionPoint = (p0 + p1) * .5;
		retval = true;
	}
	return retval;
}

/*!*********************************************************************************************************************
\return Find intersection point of 2 lines.
\param  p1  Point on line 1.
\param  d1  Direction vector of line 1.
\param  p2  Point on line 2.
\param  d2  Direction vector of line 2.
\brief  Find intersection point of 2 lines (assumes they intersect).
***********************************************************************************************************************/
template<typename Vec2>
inline bool rayIntersect(const Vec2& p0, const Vec2& d0, const Vec2& p1, const Vec2& d1, Vec2& outIntersectionPoint)
{
	typename Vec2::value_type dummy;
	return rayIntersect(p0, d0, p1, d1, dummy, outIntersectionPoint);
}


inline float distanceToPlane(const glm::vec2 pointToCheck, float planeDist, glm::vec2 planeNorm)
{
	return glm::dot(glm::vec2(planeNorm), pointToCheck) - planeDist;
}

inline float distanceToPlane(const glm::vec2 pointToCheck, glm::vec2 anyPlanePoint, glm::vec2 planeNorm)
{
	return glm::dot(glm::vec2(planeNorm), pointToCheck) - glm::dot(anyPlanePoint, planeNorm);
}

template<typename Vec2>
inline typename Vec2::value_type vectorAngleSine(const Vec2& d0, const Vec2& d1)
{
	using namespace glm;
	return glm::length(glm::cross(
	                     normalize(detail::tvec3<typename Vec2::value_type, glm::precision::defaultp>(d0, 0.)),
	                     normalize(glm::detail::tvec3<typename Vec2::value_type, glm::precision::defaultp>(d1, 0.))));
}

template<typename Vec2>
inline typename Vec2::value_type vectorAngleCosine(const Vec2& d0, const Vec2& d1)
{
	using namespace glm;
	return dot(normalize(d0), normalize(d1));
}
template<typename Vec2>
inline typename Vec2::value_type vectorAngleSine(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3)
{
	return vectorAngleSine(p1 - p0, p3 - p2);
}

template<typename Vec2>
inline typename Vec2::value_type vectorAngleCosine(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3)
{
	return vectorAngleCosine(p1 - p0, p3 - p2);
}

template<typename Vec2>
inline typename Vec2::value_type vectorAngleCosine(const Vec2& center, const Vec2& point0, const Vec2& point1)
{
	return vectorAngleCosine(point0 - center, point1 - center);
}

template<typename Vec2>
inline typename Vec2::value_type vectorAngleSine(const Vec2& center, const Vec2& point0, const Vec2& point1)
{
	return vectorAngleSine(point0 - center, point1 - center);
}

/*!*********************************************************************************************************************
\param  s String to sanitise.
\brief  Sanitises the incoming std::string.
***********************************************************************************************************************/
inline void cleanString(std::string& s)
{
	//Remove HTML escape character for '&'
	std::size_t pos;
	pos = s.find("&amp;", 0);
	while (pos != s.npos)
	{
		std::string sub1 = s.substr(0, pos);
		sub1.append(" & ");
		std::string sub2 = s.substr(pos + 5, s.length() - 1);
		s.clear();
		s.append(sub1);
		s.append(sub2);
		pos = s.find("&amp;", 0);
	}

	//Remove HTML escape character for quotation marks.
	pos = s.find("&quot;", 0);
	while (pos != s.npos)
	{
		std::string sub1 = s.substr(0, pos);
		sub1.append(" ");
		std::string sub2 = s.substr(pos + 6, s.length() - 1);
		s.clear();
		s.append(sub1);
		s.append(sub2);
		pos = s.find("&quot;", 0);
	}
}

/*!*********************************************************************************************************************
\return The width of the road.
\type The RoadType enum describing the type of the road
\brief  Use the type of a road to determine its width.
***********************************************************************************************************************/
inline double getRoadWidth(RoadTypes::RoadTypes type)
{
	static const float roadWidths[] = { .015, .014, .013, .012, .010, .008 };
	return roadWidths[type];
}
/*!*********************************************************************************************************************
\return Width of the road.
\param  tags  Tags for the road.
\brief  Use the tags of a road to determine its width.
***********************************************************************************************************************/
inline double getRoadWidth(const std::vector<Tag>& tags, RoadTypes::RoadTypes& outType)
{
	// Needs extension to include colour
	std::string roadType = "";
	for (Tag tag : tags)
	{
		if (tag.key == "highway")
		{
			roadType = tag.value;
			break;
		}
	}

	// Motorway, Trunk, Primary, Secondary, Other, Service
	if (roadType == "motorway")
	{
		outType = RoadTypes::Motorway;
	}
	else if ((roadType == "trunk") || (roadType == "motorway_link"))
	{
		outType = RoadTypes::Trunk;
	}
	else if ((roadType == "primary") || (roadType == "trunk_link"))
	{
		outType = RoadTypes::Primary;
	}
	else if ((roadType == "secondary") || (roadType == "primary_link") || (roadType == "tertiary_link"))
	{
		outType = RoadTypes::Secondary;
	}
	else if ((roadType == "service") || (roadType == "residential"))// || (roadType == "tertiary") || (roadType == "secondary_link"))
	{
		outType = RoadTypes::Service;
	}
	else
	{
		outType = RoadTypes::RoadTypes::Other;
	}

	return getRoadWidth(outType);
}

/*!*********************************************************************************************************************
\return bool True if the road is a roundabout otherwise false.
\param  tags Tags for the road.
\brief  Clears data no longer needed from the osm object.
***********************************************************************************************************************/
inline bool isRoadRoundabout(const std::vector<Tag>& tags)
{
	for (auto& tag : tags)
	{
		if (tag.key.compare("junction") == 0 && tag.value.compare("roundabout") == 0)
		{
			return true;
		}
	}
	return false;
}

/*!*********************************************************************************************************************
\return std::string  The unique ID to return.
\param  tags  A collection of tags to search.
\brief  Use the tags to find the unique ID.
***********************************************************************************************************************/
inline const std::string& getAttributeRef(const std::vector<Tag>& tags)
{
	static std::string value;
	for (auto& tag : tags)
	{
		if (tag.key.compare("ref") == 0)
		{
			value = tag.value;
			break;
		}
	}

	return value;
}

/*!*********************************************************************************************************************
\return std::string  Name of the road or empty std::string if no name is available.
\param  tags  A collection of tags to search.
\brief  Use the tags of a road to determine its name.
***********************************************************************************************************************/
inline const std::string& getAttributeName(const Tag* tags, size_t numTags)
{
	static std::string value;
	for (size_t i = 0; i < numTags; ++i)
	{
		const auto& tag = tags[i];
		if (tag.key.compare("name") == 0)
		{
			value = tag.value;
			break;
		}
	}

	cleanString(value);
	return value;
}

/*!*********************************************************************************************************************
\return std::string the road name for the intersection.
\param  std::vector<std::vector<Tag>> Reference to a vector of vectors which holds the tags for each way that makes up an intersection.
\brief  Finds the dominant road name for a given intersection.
***********************************************************************************************************************/
inline const std::string& getIntersectionRoadName(const std::vector<std::pair<Tag*, size_t>>& tags)
{
	std::map<std::string, uint32_t> nameCount;
	uint32_t currentCount = 0;
	static std::string name = "";

	for (auto& t : tags)
	{
		std::string name = getAttributeName(t.first, t.second);
		if (!name.empty())
		{
			nameCount[name]++;
		}
	}

	for (auto it = nameCount.begin(); it != nameCount.end(); ++it)
	{
		if (it->second > currentCount)
		{
			name = it->first;
			currentCount = it->second;
		}
	}
	cleanString(name);
	return name;
}


/*!*********************************************************************************************************************
\return RoadTypes the type of road.
\param  ways Reference to a vector of ways which make up the intersection.
\brief  Finds the dominant road type for a given intersection (used to colour the intersection based on the road type).
***********************************************************************************************************************/
inline RoadTypes::RoadTypes getIntersectionRoadType(const std::vector<Way>& ways)
{
	std::vector<Way> tempWays = ways;
	std::sort(tempWays.begin(), tempWays.end(), compareRoadTypes);

	uint32_t maxCount = 0;
	uint32_t tempCount = 0;
	RoadTypes::RoadTypes current = RoadTypes::Service;
	RoadTypes::RoadTypes temp = RoadTypes::Service;

	//Iterate through way and find which road type occurs the most.
	for (auto& way : tempWays)
	{
		current = std::min(current, way.roadType);
	}
	return current;
}

/*!*********************************************************************************************************************
\return bool True if the road is a roundabout otherwise false.
\param  tags Tags for the road.
\brief  Clears data no longer needed from the osm object.
***********************************************************************************************************************/
inline bool isRoadOneWay(const std::vector<Tag>& tags)
{
	for (auto& tag : tags)
	{
		if (tag.key.compare("oneway") == 0 && tag.value.compare("yes") == 0)
		{
			return true;
		}
	}
	return false;
}

inline bool initializeBuildingTypesMap(std::map<std::string, BuildingType::BuildingType>& strings)
{
	strings[""] = BuildingType::None;
	strings["supermarket"] = BuildingType::Shop;
	strings["convenience"] = BuildingType::Shop;
	strings["bar"] = BuildingType::Bar;
	strings["cafe"] = BuildingType::Cafe;
	strings["fast_food"] = BuildingType::FastFood;
	strings["pub"] = BuildingType::Pub;
	strings["college"] = BuildingType::College;
	strings["library"] = BuildingType::Library;
	strings["university"] = BuildingType::University;
	strings["atm"] = BuildingType::ATM;
	strings["bank"] = BuildingType::Bank;
	strings["restaurant"] = BuildingType::Restaurant;
	strings["doctors"] = BuildingType::Doctors;
	strings["dentist"] = BuildingType::Dentist;
	strings["hospital"] = BuildingType::Hospital;
	strings["pharmacy"] = BuildingType::Pharmacy;
	strings["cinema"] = BuildingType::Cinema;
	strings["casino"] = BuildingType::Casino;
	strings["theatre"] = BuildingType::Theatre;
	strings["fire_station"] = BuildingType::FireStation;
	strings["courthouse"] = BuildingType::Courthouse;
	strings["police"] = BuildingType::Police;
	strings["post_office"] = BuildingType::PostOffice;
	strings["toilets"] = BuildingType::Toilets;
	strings["place_of_worship"] = BuildingType::PlaceOfWorship;
	strings["fuel"] = BuildingType::PetrolStation;
	strings["parking"] = BuildingType::Parking;
	strings["post_box"] = BuildingType::PostBox;
	strings["veterinary"] = BuildingType::Veterinary;
	strings["pet"] = BuildingType::Veterinary;
	strings["embassy"] = BuildingType::Embassy;
	strings["hairdresser"] = BuildingType::HairDresser;
	strings["butcher"] = BuildingType::Butcher;
	strings["florist"] = BuildingType::Florist;
	strings["optician"] = BuildingType::Optician;
	return true;
}

inline BuildingType::BuildingType getBuildingType(Tag* tags, size_t numTags)
{

	static std::map<std::string, BuildingType::BuildingType> strings;
	static bool used_to_initialize = initializeBuildingTypesMap(strings);

	for (size_t i = 0; i < numTags; ++i)
	{
		auto& tag = tags[i];
		if (tag.key.compare("amenity") == 0 || tag.key.compare("shop") == 0)
		{
			auto it = strings.find(tag.value);
			if (it != strings.end())
			{
				return it->second;
			}
			break;
		}
	}
	return BuildingType::Other;
}

/*!*********************************************************************************************************************
\param  tile The tile to generate indices for.
\param  way The vector of ways to generate indices for.
\return uint32_t The number of indices added (used for offset to index IBO)
\brief  Generate indices for a given tile and way (i.e. road, area etc.).
***********************************************************************************************************************/
inline uint32_t generateIndices(Tile& tile, std::vector<Way>& way)
{
	uint32_t count = 0;
	for (uint32_t i = 0; i < way.size(); ++i)
	{
		for (size_t j = 0; j < way[i].nodeIds.size(); ++j)
		{
			tile.indices.push_back(tile.nodes.find(way[i].nodeIds[j])->second.index);
			count++;
		}
	}
	return count;
}

/*!*********************************************************************************************************************
\param  tile The tile to generate indices for.
\param  way The vector of outlines to generate indices for.
\return uint32_t The number of indices added (used for offset to index IBO)
\brief  Generate indices for a given tile and outline (i.e road, area etc.).
***********************************************************************************************************************/
inline uint32_t generateIndices(Tile& tile, std::vector<uint64_t>& outlines)
{
	uint32_t count = 0;
	for (uint32_t i = 0; i < outlines.size(); ++i)
	{
		tile.indices.push_back(tile.nodes.find(outlines[i])->second.index);
		count++;
	}
	return count;
}

/*!*********************************************************************************************************************
\param  tile The tile to generate indices for.
\param  way The vector of ways to generate indices for.
\param  type The road type to generate indices for.
\return uint32_t The number of indices added (used for offset to index IBO)
\brief  Generate indices for a given tile and way - specifically for road types (i.e motorway, primary etc.).
***********************************************************************************************************************/
inline uint32_t generateIndices(Tile& tile, std::vector<Way>& way, RoadTypes::RoadTypes type)
{
	uint32_t count = 0;
	for (uint32_t i = 0; i < way.size(); ++i)
	{
		if (way[i].roadType == type)
		{
			for (uint32_t j = 0; j < way[i].nodeIds.size(); ++j)
			{
				tile.indices.push_back(tile.nodes.find(way[i].nodeIds[j])->second.index);
				count++;
			}
		}
	}
	return count;
}

inline glm::dvec2 getMapWorldDimensions(NavDataProcess& navDataProcess, uint32_t& numCols, uint32_t& numRows)
{
	glm::dvec2 mapDim(navDataProcess.getTiles()[numCols - 1][numRows - 1].max - navDataProcess.getTiles()[0][0].min);
	double mapAspectRatio = mapDim.y / mapDim.x;

	double mapWorldDimX = (navDataProcess.getOsm().maxLonLat.x - navDataProcess.getOsm().minLonLat.x) * 64000; //MAGIC NUMBER: Gives you the order of magnitude of the map size.
	return glm::vec2(mapWorldDimX, mapWorldDimX * mapAspectRatio);
}

inline void remapItemCoordinates(NavDataProcess& navDataProcess, uint32_t& numCols, uint32_t& numRows, glm::dvec2& mapWorldDim)
{
	for (uint32_t col = 0; col < navDataProcess.getTiles().size(); ++col)
	{
		auto& tileCol = navDataProcess.getTiles()[col];
		for (uint32_t row = 0; row < tileCol.size(); ++row)
		{
			auto& tile = tileCol[row];

			// Set the min and max coordinates for the tile
			tile.screenMin = remap(tile.min, navDataProcess.getTiles()[0][0].min, navDataProcess.getTiles()[numCols - 1][numRows - 1].max,
			                       -mapWorldDim * .5, mapWorldDim * .5);

			tile.screenMax = remap(tile.max, navDataProcess.getTiles()[0][0].min, navDataProcess.getTiles()[numCols - 1][numRows - 1].max,
			                       -mapWorldDim * .5, mapWorldDim * .5);

			const float newMax = (float)glm::length(mapWorldDim);
			const float oldMax = (float)glm::length(navDataProcess.getTiles()[numCols - 1][numRows - 1].max);

			for (uint32_t lod = 0; lod < LOD::Count; ++lod)
			{
				//Max X extents and position
				for (uint32_t label_id = 0; label_id < tile.labels[lod].size(); ++label_id)
				{
					auto& label = tile.labels[lod][label_id];

					//Remap the position of the label.
					label.coords = remap(label.coords, navDataProcess.getTiles()[0][0].min,
					                     navDataProcess.getTiles()[numCols - 1][numRows - 1].max, -mapWorldDim * .5, mapWorldDim * .5);

					//Remap the previously calculated distance to the closest boundary and distance to end of the road segment - for culling.
					label.distToBoundary = remap(label.distToBoundary, 0.0f, oldMax, 0.0f, newMax);
					label.distToEndOfSegment = remap(label.distToEndOfSegment, 0.0f, oldMax, 0.0f, newMax);
				}

				//Max X extents and position
				for (uint32_t amenity_label_id = 0; amenity_label_id < tile.amenityLabels[lod].size(); ++amenity_label_id)
				{
					auto& label = tile.amenityLabels[lod][amenity_label_id];

					//Remap the position of the label.
					label.coords = remap(label.coords, navDataProcess.getTiles()[0][0].min,
					                     navDataProcess.getTiles()[numCols - 1][numRows - 1].max, -mapWorldDim * .5, mapWorldDim * .5);

					//Remap the position of the icon.
					label.iconData.coords = remap(label.iconData.coords, navDataProcess.getTiles()[0][0].min,
					                              navDataProcess.getTiles()[numCols - 1][numRows - 1].max, -mapWorldDim * .5, mapWorldDim * .5);

					//Remap the previously calculated distance to the closest boundary and distance to end of the road segment - for culling.
					label.distToBoundary = remap(label.distToBoundary, 0.0f, oldMax, 0.0f, newMax);
					label.distToEndOfSegment = remap(label.distToEndOfSegment, 0.0f, oldMax, 0.0f, newMax);
				}

				for (auto && icon : tile.icons[lod])
				{
					icon.coords = remap(icon.coords, navDataProcess.getTiles()[0][0].min,
					                    navDataProcess.getTiles()[numCols - 1][numRows - 1].max, -mapWorldDim * .5, mapWorldDim * .5);
				}
			}
		}
	}
}

inline double calculateAngleBetweenPoints(glm::dvec2 start, glm::dvec2 end)
{
	double dy = start.y - end.y;
	double dx = start.x - end.x;
	// switch x and y around here so that we are finding the angle in relation to the positive y axis
	double theta = glm::atan(dx, dy); // range [-PI, PI]
	theta *= 180.0f / glm::pi<double>(); // rads to degs, range [-180, 180]
	// range [0, 360)
	if (theta < 0)
	{
		theta = 360 + theta;
	}
	return theta;
}

/*!*********************************************************************************************************************
\brief  Converts pre-computed route into the appropriate co-ordinate space and calculates the routes total true distance
and partial distances between each node which is used later to animate the route.
***********************************************************************************************************************/
inline void convertRoute(glm::dvec2& mapWorldDim, uint32_t& numCols, uint32_t& numRows, NavDataProcess& navDataProcess, float& weight, float& rotation, float& totalRouteDistance)
{
	if (navDataProcess.getRouteData().size() == 0)
	{
		weight = 0.0f;
		rotation = 0.0f;
		Log(LogLevel::Information, "No route calculated.");
		return;
	}

	for (uint32_t i = 0; i < navDataProcess.getRouteData().size(); ++i)
	{
		navDataProcess.getRouteData()[i].point = -remap(navDataProcess.getRouteData()[i].point, navDataProcess.getTiles()[0][0].min,
		    navDataProcess.getTiles()[numCols - 1][numRows - 1].max, -mapWorldDim * .5, mapWorldDim * .5);

		if (i > 0)
		{
			glm::vec2 previousPoint = navDataProcess.getRouteData()[i - 1].point;
			glm::vec2 currentPoint = navDataProcess.getRouteData()[i].point;

			float partialDistance = glm::distance(currentPoint, previousPoint);
			navDataProcess.getRouteData()[i - 1].distanceToNext = partialDistance;
			totalRouteDistance += partialDistance; // The total 'true' distance of the path

			float angle = static_cast<float>(calculateAngleBetweenPoints(previousPoint, currentPoint));
			navDataProcess.getRouteData()[i - 1].rotation = angle;
		}
	}
}