#include "NavDataProcess.h"
#include "../../External/glm/gtx/intersect.hpp"
const float TexUVLeft = -1.f;
const float TexUVRight = 1.f;
const float TexUVUp = .25f;
const float TexUVCenter = (TexUVLeft + TexUVRight) * .5f;

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Initialisation of data, calls functions to load data from XML file and triangulate geometry.
***********************************************************************************************************************/
pvr::Result NavDataProcess::loadAndProcessData()
{
	// Set tile scaling parameters
	_osm.lonTileScale = 0.005;
	_osm.latTileScale = 0.005;

	pvr::Result result = loadOSMData();

	if (result != pvr::Result::Success)
	{
		return result;
	}

	initialiseTiles();
	calculateRoute();
	triangulateAllRoads();
	calculateIntersections();
	convertToTriangleList();

	return result;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Further initialisation - should be called after LoadAndProcess and once the window width/height is known.
This function fills the tiles with data which has been processed.
***********************************************************************************************************************/
void NavDataProcess::initTiles(const glm::ivec2& screenDimensions)
{
	_windowsDim = screenDimensions;
	processLabels(_osm.bounds.max - _osm.bounds.min);
	sortTiles();
	_osm.cleanData();
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Get map data and load into OSM object.
***********************************************************************************************************************/
pvr::Result NavDataProcess::loadOSMData()
{
#if defined(_WIN32) && defined(_DEBUG)
	// Enable memory-leak reports
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	pugi::xml_document mapData;
	std::vector<char> mapStream = _assetStream->readToEnd<char>();
	pugi::xml_parse_result result = mapData.load(mapStream.data(), static_cast<unsigned int>(mapStream.size()));

	Log(LogLevel::Debug, "XML parse result: %s", result.description());
	if (!result)
	{
		return pvr::Result::UnknownError;
	}

	// Get the bounds of the map
	_osm.maxLonLat = glm::vec2(mapData.root().child("osm").child("bounds").attribute("maxlon").as_double(), mapData.root().child("osm").child("bounds").attribute("maxlat").as_double());
	_osm.minLonLat = glm::vec2(mapData.root().child("osm").child("bounds").attribute("minlon").as_double(), mapData.root().child("osm").child("bounds").attribute("minlat").as_double());
	_osm.bounds.min = glm::dvec2(0, 0);
	_osm.bounds.max = lonLatToMetres(_osm.minLonLat, _osm.maxLonLat);

	// Collect the nodes
	pugi::xml_object_range<pugi::xml_named_node_iterator> nodes = mapData.root().child("osm").children("node");
	std::array<Tag, 50> tempTags;
	size_t numTags = 0;

	for (pugi::xml_named_node_iterator currentNode = nodes.begin(); currentNode != nodes.end(); ++currentNode)
	{
		if (!currentNode->attribute("visible").empty() && !currentNode->attribute("visible").as_bool()) // Skip node if not visible
		{
			continue;
		}

		glm::dvec2 coords;

		// Get ID, latitude and longitude
		uint64_t node_id = currentNode->attribute("id").as_ullong();
		Vertex& tempNode = _osm.createNode(node_id);

		tempNode.id = currentNode->attribute("id").as_ullong();
		coords = glm::dvec2(currentNode->attribute("lon").as_double(), currentNode->attribute("lat").as_double());
		tempNode.coords = lonLatToMetres(_osm.minLonLat, coords);

		if (coords.x < _osm.minLonLat.x)
		{
			tempNode.coords.x *= -1;
		}
		if (coords.y < _osm.minLonLat.y)
		{
			tempNode.coords.y *= -1;
		}

		// Get tags from XML.
		pugi::xml_object_range<pugi::xml_named_node_iterator> tags = currentNode->children("tag");
		numTags = 0;

		//Collect tags for this node.
		for (pugi::xml_named_node_iterator currentTag = tags.begin(); currentTag != tags.end(); ++currentTag)
		{
			uint32_t tag = static_cast<uint32_t>(numTags++);
			tempTags[tag].key = currentTag->attribute("k").as_string();
			tempTags[tag].value = currentTag->attribute("v").as_string();
		}

		generateIcon(&tempNode.id, 1, tempTags.data(), numTags, tempNode.id);

		debug_assertion(_osm.icons[LOD::IconLOD].size() >= _osm.amenityLabels[LOD::AmenityLabelLOD].size(), "There must be at least one amenity icon per amenity label");
	}
	if (_osm.nodes.empty())
	{
		return pvr::Result::UnknownError;
	}
	// Collect the ways
	pugi::xml_object_range<pugi::xml_named_node_iterator> ways = mapData.root().child("osm").children("way");

	for (pugi::xml_named_node_iterator currentWay = ways.begin(); currentWay != ways.end(); ++currentWay)
	{
		if (!currentWay->attribute("visible").empty() && !currentWay->attribute("visible").as_bool()) // Skip way if not visible
		{
			continue;
		}

		bool isArea = false;

		Way* tempWay = NULL;
		WayTypes::WayTypes wayType = WayTypes::Default;
		// Get ID
		uint64_t tempWayId = currentWay->attribute("id").as_ullong();

		// Get tags
		pugi::xml_object_range<pugi::xml_named_node_iterator> tags = currentWay->children("tag");

		for (pugi::xml_named_node_iterator currentTag = tags.begin(); currentTag != tags.end(); ++currentTag)
		{
			Tag tempTag;

			const char* key = currentTag->attribute("k").as_string();
			const char* value = currentTag->attribute("v").as_string();

			if ((strcmp(key, "highway") == 0) && (strcmp(value, "footway") != 0) && (strcmp(value, "bus_guideway") != 0) && (strcmp(value, "raceway") != 0) &&
			    (strcmp(value, "bridleway") != 0) && (strcmp(value, "steps") != 0) && (strcmp(value, "path") != 0) && (strcmp(value, "cycleway") != 0) &&
			    (strcmp(value, "proposed") != 0) && (strcmp(value, "construction") != 0) && (strcmp(value, "track") != 0) && (strcmp(value, "pedestrian") != 0))
			{
				wayType = WayTypes::Road;
			}
			else if ((strcmp(key, "amenity") == 0) && (strcmp(value, "parking") == 0))
			{
				wayType = WayTypes::Parking;
			}
			else if ((strcmp(key, "building") == 0) || (strcmp(key, "shop") == 0) || (strcmp(key, "landuse") == 0) && (strcmp(value, "retail") == 0))
			{
				wayType = WayTypes::Building;
			}
			else if ((strcmp(key, "area") == 0) && (strcmp(value, "yes") == 0))
			{
				isArea = true;
			}
		}
		Way tmp;

		if (wayType == WayTypes::Road)
		{
			tempWay = &_osm.originalRoadWays[tempWayId];
		}
		else if (wayType == WayTypes::Parking)
		{
			tempWay = &_osm.parkingWays[tempWayId];
		}
		else if (wayType == WayTypes::Building)
		{
			tempWay = &_osm.buildWays[tempWayId];
		}
		else
		{
			tempWay = &tmp;
		}

		tempWay->inner = false;
		tempWay->area = isArea;
		tempWay->isIntersection = false;
		tempWay->isRoundabout = false;
		tempWay->width = 0.0;

		// Get ID
		tempWay->id = tempWayId;

		for (pugi::xml_named_node_iterator currentTag = tags.begin(); currentTag != tags.end(); ++currentTag)
		{
			tempWay->tags.emplace_back();
			Tag& tempTag = tempWay->tags.back();
			tempTag.key = currentTag->attribute("k").as_string();
			tempTag.value = currentTag->attribute("v").as_string();
			tempWay->tags.push_back(tempTag);
		}


		// Get node IDs
		pugi::xml_object_range<pugi::xml_named_node_iterator> nodeIds = currentWay->children("nd");

		for (pugi::xml_named_node_iterator currentNodeId = nodeIds.begin(); currentNodeId != nodeIds.end(); ++currentNodeId)
		{
			tempWay->nodeIds.push_back(currentNodeId->attribute("ref").as_ullong());

			if ((wayType == WayTypes::Road) && !tempWay->area)
			{
				Vertex& currentNode = _osm.getNodeById(tempWay->nodeIds.back());
				currentNode.wayIds.push_back(tempWay->id);

				if (currentNode.wayIds.size() == 2)
				{
					_osm.original_intersections.push_back(currentNode.id);
				}
			}
		}

		//Add way to data structure based on type.
		switch (wayType)
		{
		case WayTypes::Road:
		{
			RoadTypes::RoadTypes type;
			tempWay->width = getRoadWidth(tempWay->tags, type);
			tempWay->roadType = type;
			tempWay->isRoundabout = isRoadRoundabout(tempWay->tags);

			std::string roadName = getAttributeName(tempWay->tags.data(), tempWay->tags.size());

			//Add a road name if none was available from the XML.
			if (roadName.empty())
			{
				static uint64_t uid = 0;
				Tag name;
				name.key = "name";
				name.value = pvr::strings::createFormatted("%dth Street", uid);
				tempWay->tags.push_back(name);
				uid++;
			}
			else if (!roadName.empty() && !tempWay->isRoundabout)
			{
				LabelData label;
				for (uint32_t i = 0; i < tempWay->nodeIds.size(); ++i)
				{
					label.coords = _osm.getNodeById(tempWay->nodeIds[i]).coords;
					label.name = roadName;
					label.scale = static_cast<float>(tempWay->width);
					label.id = tempWay->id;
					_osm.labels[LOD::LabelLOD].push_back(label);
				}
			}
			break;
		}
		case WayTypes::Parking:
		{
			generateIcon(tempWay->nodeIds.data(), tempWay->nodeIds.size(), tempWay->tags.data(), tempWay->tags.size(), tempWay->id);
			debug_assertion(_osm.icons[LOD::IconLOD].size() >= _osm.amenityLabels[LOD::AmenityLabelLOD].size(), "There must be at least one amenity icon per amenity label");
			break;
		}
		case WayTypes::Building:
		{
			generateIcon(tempWay->nodeIds.data(), tempWay->nodeIds.size(), tempWay->tags.data(), tempWay->tags.size(), tempWay->id);
			debug_assertion(_osm.icons[LOD::IconLOD].size() >= _osm.amenityLabels[LOD::AmenityLabelLOD].size(), "There must be at least one amenity icon per amenity label");
			break;
		}
		default:
			break;
		}
	}
	if (_osm.originalRoadWays.empty() && _osm.buildWays.empty() && _osm.parkingWays.empty())
	{
		return pvr::Result::UnknownError;
	}

	// Use relation data to sort inner ways
	pugi::xml_object_range<pugi::xml_named_node_iterator> relations = mapData.root().child("osm").children("relation");

	for (pugi::xml_named_node_iterator currentRelation = relations.begin(); currentRelation != relations.end(); ++currentRelation)
	{
		if (!currentRelation->attribute("visible").empty() && !currentRelation->attribute("visible").as_bool()) // Skip way if not visible
		{
			continue;
		}

		// Check tags to see if it describes a multipolygon
		pugi::xml_object_range<pugi::xml_named_node_iterator> tags = currentRelation->children("tag");
		bool multiPolygon = false;

		for (pugi::xml_named_node_iterator currentTag = tags.begin(); currentTag != tags.end(); ++currentTag)
		{
			std::string key = currentTag->attribute("k").as_string();
			std::string value = currentTag->attribute("v").as_string();

			if ((key == "type") && (value == "multipolygon"))
			{
				multiPolygon = true;
			}
		}

		if (!multiPolygon)
		{
			continue;
		}

		// Iterate through members to find outer way type
		pugi::xml_object_range<pugi::xml_named_node_iterator> members = currentRelation->children("member");

		WayTypes::WayTypes outerType = WayTypes::Default;
		for (pugi::xml_named_node_iterator currentMember = members.begin(); currentMember != members.end(); ++currentMember)
		{
			std::string type = currentMember->attribute("type").as_string();
			std::string role = currentMember->attribute("role").as_string();

			if ((type == "way") && (role == "outer"))
			{
				uint64_t wayId = currentMember->attribute("ref").as_ullong();

				if (_osm.parkingWays.find(wayId) != _osm.parkingWays.end())
				{
					outerType = WayTypes::Parking;
				}
				else if (_osm.buildWays.find(wayId) != _osm.buildWays.end())
				{
					outerType = WayTypes::Building;
				}
			}
		}

		// Iterate through members again to find inner ways
		for (pugi::xml_named_node_iterator currentMember = members.begin(); currentMember != members.end(); ++currentMember)
		{
			std::string type = currentMember->attribute("type").as_string();
			std::string role = currentMember->attribute("role").as_string();

			if ((type == "way") && (role == "inner"))
			{
				uint64_t wayId = currentMember->attribute("ref").as_ullong();

				const auto& parkingTemp = _osm.parkingWays.find(wayId);
				const auto& buildTemp = _osm.buildWays.find(wayId);

				if ((parkingTemp != _osm.parkingWays.end()) && (outerType == WayTypes::Parking))
				{
					parkingTemp->second.inner = true;
				}
				else if ((buildTemp != _osm.buildWays.end()) && (outerType == WayTypes::Building))
				{
					buildTemp->second.inner = true;
				}
			}
		}
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Iterates over available intersections and calculates a 'random' route through the available data set,
if no interesections are available no route will be calculated.
***********************************************************************************************************************/
void NavDataProcess::calculateRoute()
{
	Log(LogLevel::Information, "Calculating a simple route.");
	if (_osm.original_intersections.size() == 0)
	{
		Log(LogLevel::Information, "No Route Calculated - No intersections.");
		return;
	}

	uint32_t count = 0;
	//Hold previosuly visited IDs to prevent going back on ourselves.
	std::set<uint64_t> previousIntersectIDs;
	std::set<uint64_t> previousWayIDs;
	uint64_t nextID = _osm.original_intersections[0];
	uint64_t lastID = -1;
	std::vector<std::pair<uint64_t, glm::dvec2>> tempCoords;

	Log(LogLevel::Information, "Calculated intersections: %u", _osm.original_intersections.size());

	while (count < _osm.original_intersections.size())
	{
		bool nextJunctionFound = false;
		const Vertex& node = _osm.getNodeById(nextID);

		//Find the next way for the route.
		for (uint32_t i = 0; i < node.wayIds.size(); ++i)
		{
			Way way = _osm.originalRoadWays.find(node.wayIds[i])->second;

			//Make sure we have not visited this way in the past.
			if (previousWayIDs.find(way.id) == previousWayIDs.end())
			{
				previousWayIDs.insert(way.id);

				if (_osm.getNodeById(way.nodeIds.back()).wayIds.size() == 1 && _osm.getNodeById(way.nodeIds[0]).wayIds.size() == 1)
				{
					continue;
				}

				for (auto id : way.nodeIds)
				{
					glm::dvec2 coords = _osm.getNodeById(id).coords;

					//Check the node is not outside the map boundary.
					if (isOutOfBounds(coords) || isTooCloseToBoundary(coords))
					{
						if (tempCoords.size() > 0)
						{
							tempCoords.clear();
						}

						continue;
					}

					tempCoords.push_back(std::pair<uint64_t, glm::dvec2>(id, coords));

					if (!nextJunctionFound)
					{
						//Find the next node that is an intersection.
						for (uint32_t j = 0; j < _osm.original_intersections.size(); ++j)
						{
							if (id == _osm.original_intersections[j] && previousIntersectIDs.find(id) == previousIntersectIDs.end())
							{
								previousIntersectIDs.insert(id);
								nextID = id;
								nextJunctionFound = true;
								break;
							}
						}
					}

					if (nextJunctionFound)
					{
						if (way.nodeIds[0] == nextID && way.nodeIds.back() == node.id && way.nodeIds.size() > 2)
						{
							glm::dvec2 p1 = _osm.getNodeById(node.id).coords;
							glm::dvec2 p2 = _osm.getNodeById(way.nodeIds[0]).coords;
							glm::dvec2 p3 = _osm.getNodeById(way.nodeIds[way.nodeIds.size() / 2]).coords;

							float a1 = static_cast<float>(glm::atan(p1.y - p2.y, p1.x - p2.x));
							float a2 = static_cast<float>(glm::atan(p1.y - p3.y, p1.x - p3.x));

							if (glm::abs(a1 - a2) > 0.25f)
							{
								std::reverse(way.nodeIds.begin(), way.nodeIds.end());
								tempCoords.clear();

								for (uint32_t j = 0; j < way.nodeIds.size(); ++j)
								{
									tempCoords.push_back(std::pair<uint64_t, glm::dvec2>(way.nodeIds[j], _osm.getNodeById(way.nodeIds[j]).coords));
								}
							}
						}
						break;
					}

				}
			}
			//Add the found nodes to the route.
			if (nextJunctionFound)
			{
				for (uint32_t j = 0; j < tempCoords.size(); ++j)
				{
					if (lastID == tempCoords[j].first)
					{
						continue;
					}

					RouteData data;
					data.distanceToNext = 0.0f;
					data.point = tempCoords[j].second;

					_osm.route.push_back(data);
				}

				lastID = tempCoords.back().first;
				tempCoords.clear();
				break;
			}
			tempCoords.clear();
		}

		//If no junction was found end the route.
		if (!nextJunctionFound)
		{
			break;
		}

		count++;
	}
}

/*!*********************************************************************************************************************
\param  nodeIds The nodes IDs that make up this entity.
\param  tags The tags associated with this entity, which may contain the type of amenity / service and name.
\param  id The id to use to add this icon to the array of icons.
\brief  Check if the incoming entity is an amenity or service, if it is create an icon for it, and possibly a label if a name is present.
***********************************************************************************************************************/
void NavDataProcess::generateIcon(uint64_t* nodeIds, size_t numNodeIds, Tag* tags, size_t numTags, uint64_t id)
{
	static const uint32_t maxLineLen = 10;

	BuildingType::BuildingType type = getBuildingType(tags, numTags);
	if (type != BuildingType::None)
	{
		const std::string& name = getAttributeName(tags, numTags);
		bool nameEmpty = name.empty();

		if (_osm.uniqueIconNames.find(name) != _osm.uniqueIconNames.end() || (type == BuildingType::Other && nameEmpty))
		{
			return;
		}

		//Calculate the icons co-ordinates by averaging the nodes co-ordinates that make up the building.
		glm::dvec2 coord = glm::dvec2(0, 0);
		for (uint32_t i = 0; i < numNodeIds; ++i)
		{
			coord += _osm.getNodeById(nodeIds[i]).coords;
		}

		coord /= double(numNodeIds);

		IconData icon;
		icon.buildingType = type;
		icon.coords = coord;
		icon.scale = 0.005f;
		icon.id = id;
		_osm.icons[LOD::IconLOD].push_back(icon);

		//Check if this building has a name, if it does create a label for it.
		if (!nameEmpty)
		{
			_osm.uniqueIconNames.insert(name);

			AmenityLabelData label;
			label.scale = 0.003;
			// move the amenity label below the icon
			label.coords = coord - glm::dvec2(0, 1.2f * icon.scale);
			label.name = name;
			label.id = id;
			label.rotation = 0.0f;
			label.iconData = icon;

			//Split long names
			if (name.length() > maxLineLen)
			{
				std::size_t pos = name.find_first_of(' ', maxLineLen);
				pos = (pos == std::string::npos ? name.find_last_of(' ') : pos);

				if (pos != std::string::npos)
				{
					label.name.insert(pos + 1, "\n");
				}
			}

			_osm.amenityLabels[LOD::AmenityLabelLOD].push_back(label);
		}
	}
}

/*!*********************************************************************************************************************
\brief  Calculate actual label position based on the average of two nodes, also calculates the rotation that will be
applied to the text based on the slope of the road segment i.e. the line between the two nodes.
***********************************************************************************************************************/
void NavDataProcess::processLabels(glm::dvec2 mapWorldDim)
{
	for (int lod = 0; lod <= LOD::Count; ++lod)
	{
		auto& _osmlodlabels = _osm.labels[lod];
		if (_osmlodlabels.size() == 0)
		{
			continue;
		}
		static const float minDistLabels = 0.03f; //Minimum distance two labels can be apart, to prevent crowding / overlaps.
		std::vector<LabelData> temp;

		for (uint32_t i = 0; i < _osmlodlabels.size() - 1; ++i)
		{
			if (i > 0)
			{
				LabelData label;
				//Check labels came from the same way.
				if (_osmlodlabels[i].id == _osmlodlabels[i - 1].id)
				{
					if (glm::distance(_osmlodlabels[i].coords, _osmlodlabels[i - 1].coords) < 0.01)
					{
						continue;
					}

					label = _osmlodlabels[i];

					glm::dvec2 pos = (_osmlodlabels[i].coords + _osmlodlabels[i - 1].coords) / 2.0;
					label.distToEndOfSegment = static_cast<float>(glm::distance(pos, _osmlodlabels[i].coords));

					if (temp.size() > 0)
					{
						double dist = glm::distance(temp.back().coords, pos);
						if (dist < minDistLabels)
						{
							continue;
						}
					}

					//Remap co-ordinates into screen space to calculate the accurate angle of the line.
					glm::vec2 remappedPos1 = glm::vec2(-remap(_osmlodlabels[i - 1].coords, _osm.tiles[0][0].min,
					                                   _osm.tiles[getNumCols() - 1][getNumRows() - 1].max, -mapWorldDim * .5, mapWorldDim * .5));

					glm::vec2 remappedPos2 = glm::vec2(-remap(_osmlodlabels[i].coords, _osm.tiles[0][0].min,
					                                   _osm.tiles[getNumCols() - 1][getNumRows() - 1].max, -mapWorldDim * .5, mapWorldDim * .5));

					// Compute rotation for the label based on slope of line y / x
					float angle = -static_cast<float>(calculateAngleBetweenPoints(remappedPos1, remappedPos2));

					if (angle <= -glm::degrees(glm::half_pi<float>()))
					{
						angle += glm::degrees(glm::pi<float>());
					}
					else if (angle >= glm::degrees(glm::half_pi<float>()))
					{
						angle -= glm::degrees(glm::pi<float>());
					}

					// + 90 degrees ensures the labels are properly oriented with regard to the rotation of the roads
					angle += 90.0f;

					label.rotation = angle;
					label.coords = pos;
					temp.push_back(label);
				}
			}
		}

		_osmlodlabels.clear();
		_osmlodlabels.insert(_osmlodlabels.begin(), temp.begin(), temp.end());
	}
}

/*!*********************************************************************************************************************
\return The point in terms of x and y from the origin.
\param  origin  Longitude then latitude to use as origin.
\param  point Longitude then latitude of point to convert.
\brief  Convert longitude and latitude to x and y from a given origin.
***********************************************************************************************************************/
glm::dvec2 NavDataProcess::lonLatToMetres(const glm::dvec2 origin, const glm::dvec2 point) const
{
	glm::dvec2 coords;
	//Approx radius of Earth
	static const double radius = 6371.0;
	static const double pi = glm::pi<double>();

	// Determine the x coordinate
	double v = glm::sin((point.x * pi / 180.0 - origin.x * pi / 180.0) / 2.0);
	coords.x = 2.0 * radius * glm::asin(glm::sqrt(glm::cos(origin.y * pi / 180.0) * glm::cos(origin.y * pi / 180.0) * v * v));

	// Determine the y coordinate
	double u = sin((point.y * pi / 180.0 - origin.y * pi / 180.0) / 2.0);
	coords.y = 2.0 * radius * glm::asin(glm::sqrt(u * u));

	return coords;
}

/*!*********************************************************************************************************************
\brief  Determine min and max coordinates of individual tiles.
***********************************************************************************************************************/
void NavDataProcess::initialiseTiles()
{
	_osm.numCols = int(glm::ceil((_osm.maxLonLat.x - _osm.minLonLat.x) / _osm.lonTileScale));
	_osm.numRows = int(glm::ceil((_osm.maxLonLat.y - _osm.minLonLat.y) / _osm.latTileScale));
	double tileScaleX = _osm.bounds.max.x / double(_osm.numCols);
	double tileScaleY = _osm.bounds.max.y / double(_osm.numRows);

	for (uint32_t i = 0; i < _osm.numCols; ++i)
	{
		std::vector<Tile> tempCol;

		for (uint32_t j = 0; j < _osm.numRows; ++j)
		{
			Tile tempTile;

			tempTile.min.x = _osm.bounds.min.x + tileScaleX * i;
			tempTile.min.y = _osm.bounds.min.y + tileScaleY * j;

			tempTile.max.x = _osm.bounds.min.x + tileScaleX * (i + 1);
			tempTile.max.y = _osm.bounds.min.y + tileScaleY * (j + 1);

			tempCol.push_back(tempTile);
		}
		_osm.tiles.push_back(tempCol);
	}
}

/*!*********************************************************************************************************************
\param  newRoads Reference to temporary data shared between TriangulateAllRoads, CalculateIntersections and ConvertToTriangleList.
\brief  Convert all roads to triangles.
***********************************************************************************************************************/
void NavDataProcess::triangulateAllRoads()
{
	// Triangulate the roads
	for (auto wayIterator = _osm.originalRoadWays.begin(), end = _osm.originalRoadWays.end(); wayIterator != end; ++wayIterator)
	{
		if (wayIterator->second.area)
		{
			_osm.triangulatedRoads[wayIterator->first] = wayIterator->second;
		}
		else
		{
			/*Increases node density around sharp bends, which in turn increases the number of triangles produced by the triangulation function,
			which improves visual quality for sharp bends, at the cost of memory usage, initialisation time and potentially frame times.*/
			if (wayIterator->second.nodeIds.size() > 2)
			{
				wayIterator->second.nodeIds = tessellate(wayIterator->second.nodeIds, wayIterator->second.width);
			}

			_osm.triangulatedRoads[wayIterator->first] = wayIterator->second;
			_osm.triangulatedRoads[wayIterator->first].nodeIds = triangulateRoad(wayIterator->second.nodeIds, wayIterator->second.width);
		}
	}
}

void createNewWayWithIntersection(OSM& _osm, Way& newLineStrip, Way& newTriStrip, uint64_t newWayId)
{
	Vertex newNode0 = _osm.getNodeById(newTriStrip.nodeIds[0]); //Take copies to use for the new way
	Vertex newNode1 = _osm.getNodeById(newTriStrip.nodeIds[1]);
	newNode0.id = _osm.nodes.rbegin()->first + 1; // Generate new ids
	newNode1.id = _osm.nodes.rbegin()->first + 2;

	_osm.insertOrOverwriteNode(std::move(newNode0)); // Add the new vertices
	_osm.insertOrOverwriteNode(std::move(newNode1));

	newTriStrip.nodeIds[0] = newNode0.id; //Replace the old vertices with the new in the list
	newTriStrip.nodeIds[1] = newNode1.id;

	_osm.originalRoadWays[newWayId] = newLineStrip; // Add the ways we just created back into _osm
	_osm.triangulatedRoads[newWayId] = newTriStrip;
}

void breakUpAllIntersectionWays(OSM& _osm, uint64_t intersection_id)
{
	auto& intersection_vertex = _osm.getNodeById(intersection_id);
	for (auto it = intersection_vertex.wayIds.begin(); it != intersection_vertex.wayIds.end(); ++it)
	{
		for (auto it2 = it + 1; it2 != intersection_vertex.wayIds.end();)
		{
			if (*it == *it2)
			{
				auto pos = it2 - intersection_vertex.wayIds.begin();
				intersection_vertex.wayIds.erase(it2);
				it2 = intersection_vertex.wayIds.begin() + pos;
			}
			else
			{
				++it2;
			}
		}
	}

	bool isLoop;
	uint64_t wayToBreak_id = 0;

	while (wayToBreak_id != -1)
	{
		isLoop = false;
		wayToBreak_id = -1;

		auto originalsize = intersection_vertex.wayIds.size();
		// Determine if the junction involves only the start or end of a way
		for (size_t way_idx = 0; way_idx < intersection_vertex.wayIds.size() && wayToBreak_id == -1; ++way_idx)
		{
			uint64_t way_id = intersection_vertex.wayIds[way_idx];
			auto& oriway = _osm.getOriginalRoadWay(way_id);
			auto& triway = _osm.getTriangulatedRoadWay(way_id);

			bool hasIntersectionContinuity = (std::find(oriway.nodeIds.begin() + 1, oriway.nodeIds.end() - 1, intersection_id) != (oriway.nodeIds.end() - 1));

			// Only deal with loops AFTER chopping off leftover bits and pieces...
			// We will break loops to avoid problems, but later later...
			isLoop = (!hasIntersectionContinuity &&  oriway.nodeIds.front() == intersection_id && oriway.nodeIds.back() == intersection_id);
			if (hasIntersectionContinuity || isLoop)
			{
				wayToBreak_id = way_id;
			}
		}

		if (wayToBreak_id != -1) // If there is a junction part way through a road, or we have a loop to break up
		{
			Way& originalWay = _osm.originalRoadWays.find(wayToBreak_id)->second;
			Way& triangulatedWay = _osm.triangulatedRoads.find(wayToBreak_id)->second;

			// For all ways involved, the "original" way will actually contain the intersection node (id).
			// Find where in the way the intersection node is... (1->size -1, because we're partway through road)
			uint32_t originalIntersectIndex = 0;
			if (isLoop)
			{
				assertion(originalWay.nodeIds.front() == intersection_id &&
				          originalWay.nodeIds.back() == intersection_id &&
				          std::find(originalWay.nodeIds.begin() + 1, originalWay.nodeIds.end() - 1, intersection_id) == originalWay.nodeIds.end() - 1);
				assertion(originalWay.nodeIds.size() > 2);
				originalIntersectIndex = static_cast<uint32_t>(originalWay.nodeIds.size() / 2);
			}
			else
			{
				originalIntersectIndex = static_cast<uint32_t>((std::find(originalWay.nodeIds.begin() + 1, originalWay.nodeIds.end() - 1, intersection_id) - originalWay.nodeIds.begin()));
			}
			// Find the position of the (two?) intersection nodes in the triangulated way list
			uint32_t triangulatedIntersectIndex = originalIntersectIndex * 2;

			assertion(triangulatedIntersectIndex < triangulatedWay.nodeIds.size(), "Intersection index out of bounds.");

			uint64_t newWayId = _osm.originalRoadWays.rbegin()->first + 1;
			Way newNonTriangulatedRoad = originalWay; // Copy the way. We mainly need the data...
			Way newTriangulatedRoad = triangulatedWay; // The triangulated version as well.
			newNonTriangulatedRoad.id = newWayId;
			newTriangulatedRoad.id = newWayId;
			newNonTriangulatedRoad.nodeIds.clear();
			newTriangulatedRoad.nodeIds.clear();

			// Break the way in two : The piece before, and the piece after the intersection

			// Add the nodes AFTER the intersection (INCLUSIVE) to the new list...
			newNonTriangulatedRoad.nodeIds.assign(originalWay.nodeIds.begin() + originalIntersectIndex, originalWay.nodeIds.end());

			// ... so, erase them (EXCLUSIVE) from the original list...
			originalWay.nodeIds.erase(originalWay.nodeIds.begin() + originalIntersectIndex + 1, originalWay.nodeIds.end());
			// CAUTION - the intersection now belongs to BOTH.

			auto& newlastnode = _osm.getNodeById(originalWay.nodeIds.back());
			newlastnode.wayIds.push_back(newWayId);

			//For the "new" road that we just created, for each of its nodes, erase the "previous" way id, add the way id
			// we just created.
			for (uint32_t j = 1; j < newNonTriangulatedRoad.nodeIds.size(); ++j)
			{
				bool addOnlyOnce = true;
				std::vector<uint64_t>& wayIds = _osm.getNodeById(newNonTriangulatedRoad.nodeIds[j]).wayIds;
				for (int k = 0; k < (int)wayIds.size(); ++k)
				{
					if (wayIds[k] == originalWay.id)
					{
						wayIds.erase(wayIds.begin() + k);
						if (addOnlyOnce)
						{
							wayIds.push_back(newWayId);
							addOnlyOnce = true;
						}

					}
				}
			}

			// PHASE TWO - Same as just before: The triangulated road. (Break it in half, second half is the "newTriangulatedRoad".
			newTriangulatedRoad.nodeIds.insert(newTriangulatedRoad.nodeIds.end(), triangulatedWay.nodeIds.begin() + triangulatedIntersectIndex, triangulatedWay.nodeIds.end());
			triangulatedWay.nodeIds.erase(triangulatedWay.nodeIds.begin() + triangulatedIntersectIndex + 2, triangulatedWay.nodeIds.end());

			// So, now we replace the vertices of the second half, with new vertices (that still have the same coordinates, though).
			createNewWayWithIntersection(_osm, newNonTriangulatedRoad, newTriangulatedRoad, newWayId);
			if (isLoop) // If it is a loop, we will enter a short recursion to break its second, newly created, part.
			{
				_osm.original_intersections.push_back(originalWay.nodeIds[originalIntersectIndex]);
				breakUpAllIntersectionWays(_osm, originalWay.nodeIds[originalIntersectIndex]); //Add it to the back of the queue...
			}
		}
	}
}

void sortIntersectionWaysByAngle(OSM& _osm, std::vector<Way>& nonTriangulatedWays, std::vector<Way>& triangulatedWays, glm::dvec2 centrePoint)
{
	std::vector<Way> nonTriangulatedWaysCopy;
	std::vector<Way> triangulatedWaysCopy;
	using namespace std;
	swap(nonTriangulatedWays, nonTriangulatedWaysCopy);
	swap(triangulatedWays, triangulatedWaysCopy);

	// Basically, sort the ways on the intersection based on their angle. CCW.
	while (nonTriangulatedWaysCopy.size() > 1)
	{
		float angle = 100000.f;
		uint32_t wayNum = 0;

		for (uint32_t j = 0; j < nonTriangulatedWaysCopy.size(); ++j)
		{
			assertion(isVectorEqual(_osm.getNodeById(nonTriangulatedWaysCopy[j].nodeIds[0]).coords, centrePoint));

			glm::dvec2 nextPoint = _osm.getNodeById(nonTriangulatedWaysCopy[j].nodeIds[1]).coords;

			float currentAngle = static_cast<float>(glm::atan(nextPoint.y - centrePoint.y, nextPoint.x - centrePoint.x));

			if (currentAngle < 0) { currentAngle += glm::pi<float>() * 2.f; }

			if (currentAngle < angle)
			{
				angle = currentAngle;
				wayNum = j;
			}
		}
		triangulatedWays.push_back(triangulatedWaysCopy[wayNum]);
		nonTriangulatedWays.push_back(nonTriangulatedWaysCopy[wayNum]);
		triangulatedWaysCopy.erase(triangulatedWaysCopy.begin() + wayNum);
		nonTriangulatedWaysCopy.erase(nonTriangulatedWaysCopy.begin() + wayNum);
	}
	triangulatedWays.push_back(triangulatedWaysCopy[0]); // just adding the one left in the array...
	nonTriangulatedWays.push_back(nonTriangulatedWaysCopy[0]); // just adding the one left in the array...

}

void processIntersection(OSM& _osm, uint64_t intersection_id)
{
	const Vertex& intersection_vertex = _osm.getNodeById(intersection_id);
	assertion(intersection_vertex.wayIds.size() > 1, "Invalid intersection: Found intersection with only one incoming way.");

	bool endsOnly = true;
	std::vector<Way> nonTriangulatedWays;
	std::vector<Way> triangulatedWays;

	// Determine if the junction involves only the start or end of a way
	for (uint32_t j = 0; j < intersection_vertex.wayIds.size(); ++j)
	{
		nonTriangulatedWays.push_back(_osm.getOriginalRoadWay(intersection_vertex.wayIds[j]));
		triangulatedWays.push_back(_osm.getTriangulatedRoadWay(intersection_vertex.wayIds[j]));
	}
	for (uint32_t j = 0; j < nonTriangulatedWays.size(); ++j)
	{
		if (nonTriangulatedWays[j].nodeIds.front() != intersection_id)
		{
			std::reverse(nonTriangulatedWays[j].nodeIds.begin(), nonTriangulatedWays[j].nodeIds.end());
			std::reverse(triangulatedWays[j].nodeIds.begin(), triangulatedWays[j].nodeIds.end());
			assertion(nonTriangulatedWays[j].nodeIds.front() == intersection_id, "Invalid way in intersection: "
			          "Does not have the intersection node as first or last element.");
		}
	}


	glm::dvec2 centrePoint = intersection_vertex.coords;
	sortIntersectionWaysByAngle(_osm, nonTriangulatedWays, triangulatedWays, centrePoint);

	// Sweep CCW to actually create the intersections:
	Vertex tmpint = _osm.getNodeById(nonTriangulatedWays[0].nodeIds.front()); //Take (any) copy to use for the new intersection center
	tmpint.id = _osm.nodes.rbegin()->first + 1; // Generate new ids
	tmpint.coords = centrePoint;
	tmpint.texCoords = glm::vec2(TexUVCenter, TexUVUp);

	uint64_t intersectionCenterId = tmpint.id;

	_osm.insertOrOverwriteNode(std::move(tmpint)); // Add the new vertices

	std::vector<std::array<uint64_t, 3>> newIntersectionTriangles;

	static const Real Angle_Sine_For_Parallel_Line = .05; // If the absolute value of the sine of an angle is less than that, we consider the lines parallel
	static const Real Vertex_Max_Fudge_Distance = .001;// If the distance of two vertices are less than that, we are allowed to move them both to their centerpoint.


	for (uint32_t currentWayNum = 0; currentWayNum < triangulatedWays.size(); ++currentWayNum)
	{
		// Increase by one, circle back to zero
		const uint32_t nextWayNum = (currentWayNum < (triangulatedWays.size() - 1)) ? (currentWayNum + 1) : 0;

		auto& currentWay = triangulatedWays[currentWayNum];
		auto& nextWay = triangulatedWays[nextWayNum];
		assertion(currentWay.nodeIds.size() >= 4, "Road splitting code: Triangulated way has less than 4 vertices.");

		if (triangulatedWays.size() == 2)
		{
			glm::dvec2* current_ptr_0;
			const glm::dvec2* current_ptr_2;
			glm::dvec2* next_ptr_1;
			const glm::dvec2* next_ptr_3;
			int current_tmp = 0;
			int next_tmp = 1;

			// FIX: For vertices that are on top of each other
			while (true)
			{
				current_ptr_0 = &_osm.getNodeById(currentWay.nodeIds[current_tmp]).coords;
				current_ptr_2 = &_osm.getNodeById(currentWay.nodeIds[current_tmp + 2]).coords;
				if (current_tmp + 4 >= (int)currentWay.nodeIds.size()) { break; } // Nothing to do - can't look further...
				if (glm::length(*current_ptr_0 - *current_ptr_2) > 0.0001) // If they're far enough to get a direction, use it...
				{
					break;
				}
				current_tmp += 2;
			}
			while (true)
			{
				next_ptr_1 = &_osm.getNodeById(nextWay.nodeIds[next_tmp]).coords;
				next_ptr_3 = &_osm.getNodeById(nextWay.nodeIds[next_tmp + 2]).coords;
				if (next_tmp + 4 >= (int)nextWay.nodeIds.size()) { break; } // Nothing to do - can't look further...
				if (glm::length(*next_ptr_1 - *next_ptr_3) > 0.0001) // If they're far enough to get a direction, use it...
				{
					break;
				}
				next_tmp += 2;
			}

			glm::dvec2& current_0 = *current_ptr_0;
			const glm::dvec2& current_2 = *current_ptr_2;
			glm::dvec2& next_1 = *next_ptr_1;
			const glm::dvec2& next_3 = *next_ptr_3;

			Real sine = abs(vectorAngleSine(current_2, current_0, next_3, next_1));

			if (sine > Angle_Sine_For_Parallel_Line)
			{
				Vec2 point_X;
				assertion(rayIntersect(current_2, glm::normalize(current_0 - current_2), next_3, glm::normalize(next_1 - next_3), point_X), "Intersection error");
				next_1 = current_0 = point_X;
			}
			else if (glm::length(current_0 - next_1) < Vertex_Max_Fudge_Distance)
			{
				current_0 = next_1 = (current_0 + next_1) * .5;
			}
			//else {
			// do nothing: parallel, and not close enough to fudge together.
			// In order to handle this case, we would need to materialize a new segment (funnel) to connect the roads.
			//}
		}
		else if (triangulatedWays.size() >= 3)
		{

			bool intersection_point_found = false;
			glm::dvec2 intersection_point; // The point where the right side of the current road crosses with the left side of the current road
			newIntersectionTriangles.push_back(std::array<NodeId, 3> {intersectionCenterId, currentWay.nodeIds[0], currentWay.nodeIds[1]});

			// Iterate the nodes of the current and the next road (as far as required), but we need to have at least one segment left (hence the +1).
			for (uint32_t current_node_idx = 0, next_node_idx = 0;
			     current_node_idx + 1 < nonTriangulatedWays[currentWayNum].nodeIds.size() &&
			     next_node_idx + 1 < nonTriangulatedWays[nextWayNum].nodeIds.size() &&
			     !intersection_point_found
			     ;)
			{
				int32_t current_idx_0 = current_node_idx * 2 + 0;
				int32_t current_idx_2 = current_node_idx * 2 + 2;
				assertion(current_idx_2 < (int)currentWay.nodeIds.size());
				int32_t next_idx_1 = next_node_idx * 2 + 1;
				int32_t next_idx_3 = next_node_idx * 2 + 3;
				assertion(next_idx_3 < (int)nextWay.nodeIds.size());

				glm::dvec2& current_0 = _osm.getNodeById(currentWay.nodeIds[current_idx_0]).coords; // Current road, right point on intersection
				const glm::dvec2& current_2 = _osm.getNodeById(currentWay.nodeIds[current_idx_2]).coords; //Current road, right point, just before intersection



				glm::dvec2& next_1 = _osm.getNodeById(nextWay.nodeIds[next_idx_1]).coords; //Next road, left point, intersection
				const glm::dvec2& next_3 = _osm.getNodeById(nextWay.nodeIds[next_idx_3]).coords; //Next road, left point, just before intersection

				if (isVectorEqual(next_1, next_3))
				{
					++next_node_idx;
					continue;
				}
				if (isVectorEqual(current_0, current_2))
				{
					++current_node_idx;
					continue;
				}
				double dist_curr_next;

				Real sine = glm::abs(vectorAngleSine(current_0, current_2, next_1, next_3));
				Real  dist = glm::length(current_0 - next_1);



				if (sine > Angle_Sine_For_Parallel_Line) // Not parallel
				{
					assertion(rayIntersect(current_2, glm::normalize(current_0 - current_2), next_3, glm::normalize(next_1 - next_3), dist_curr_next, intersection_point), "PARALLEL ROADS!");
				}
				else if (dist <= Vertex_Max_Fudge_Distance) // Parallel, but close enough to fudge together
				{
					intersection_point = (current_0 + next_1) * .5;
					dist_curr_next = .5;
				}
				else // Ooops - can do nothing...
				{
					current_node_idx = 10000;
					next_node_idx = 10000;
					continue;
				}

				//This should probably be handled...
				// We check that the intersection points are actually valid, i.e. we don't "retract" the sides of the rode more than their lenght.
				// What we check for this is that either we extend the sides of the rode, or we retract them at most to zero length.
				// For "current", it is enough to check that the intersection distance from current_2 is >0.
				// For "next", we do a manual test (to avoid repeating the intersection test).
				bool isIntersectionValidForCurrent = (dist_curr_next >= 0.f);
				bool isIntersectionValidForNext = ((next_1 - next_3).x * (intersection_point.x - next_3.x) >= 0.f); // Will be positive if next1 and X are on the same side of next3.

				intersection_point_found = true;

				if (isIntersectionValidForCurrent && isIntersectionValidForNext)
				{
					for (int i = 0; i <= (int)current_node_idx; ++i)
					{
						_osm.getNodeById(currentWay.nodeIds[i * 2]).coords = intersection_point;
					}

					for (int i = 0; i <= (int)next_node_idx; ++i)
					{
						_osm.getNodeById(nextWay.nodeIds[i * 2 + 1]).coords = intersection_point;
					}
				}
				else
				{
					if (!isIntersectionValidForCurrent) { ++current_node_idx; intersection_point_found = false; }
					if (!isIntersectionValidForNext) { ++next_node_idx; intersection_point_found = false; }
				}
			}
		}
	}
	if (intersection_vertex.wayIds.size() > 2)
	{
		std::vector<std::pair<Tag*, size_t>> temp;
		bool roundabout = false;
		uint32_t oneWayCount = 0;
		double width = 0;

		for (Way& w : triangulatedWays)
		{
			temp.push_back(std::make_pair(w.tags.data(), w.tags.size()));
			if (w.isRoundabout)
			{
				roundabout = true;
			}
			if (w.width > width)
			{
				width = w.width;
			}
			if (isRoadOneWay(w.tags))
			{
				oneWayCount++;
			}
		}

		Tag t;
		t.key = "name";
		t.value = getIntersectionRoadName(temp);

		ConvertedWay intersection(_osm.originalRoadWays.rbegin()->first + 1, false, std::vector<Tag>() = { t }, getIntersectionRoadType(triangulatedWays));

		intersection.isIntersection = true;
		intersection.isRoundabout = roundabout;
		intersection.width = width;

		intersection.triangulatedIds = std::move(newIntersectionTriangles);

		_osm.convertedRoads[intersection.id] = intersection;
		_osm.originalRoadWays[intersection.id] = intersection; // To keep track of the new IDs
	}
}

/*!*********************************************************************************************************************
\brief  Calculate road intersections.
***********************************************************************************************************************/
void NavDataProcess::calculateIntersections()
{
	std::deque<uint64_t> processing_intersections(_osm.original_intersections.begin(), _osm.original_intersections.end());
	std::deque<uint64_t> processed_intersections; //All intersections that have been broken up to only contain way endpoints.
	while (!processing_intersections.empty())
	{

		uint64_t intersection_id = processing_intersections.front();
		auto& intersection_vertex = _osm.getNodeById(intersection_id);
		processing_intersections.pop_front();

		if (isOutOfBounds(intersection_vertex.coords)) { continue; }

		if (_osm.getNodeById(intersection_id).wayIds.size() < 2) { continue; }

		// We preprocess all ways and junctions, in such a way that each road segment between two junctions is a single way.
		// In other words, junctions are always at the ENDS of ways. This makes the next step so much easier...
		breakUpAllIntersectionWays(_osm, intersection_id);

		if (intersection_vertex.wayIds.size() > 1) { processed_intersections.push_back(intersection_id); }
	}

	//Processed intersections only contain roadways.
	while (!processed_intersections.empty())
	{
		uint64_t intersection_id = processed_intersections.front();
		processed_intersections.pop_front();
		processIntersection(_osm, intersection_id);
	}
}

/*!*********************************************************************************************************************
\param  newRoads Reference to temporary data shared between TriangulateAllRoads, CalculateIntersections and ConvertToTriangleList.
\brief  Convert triangles into an ordered triangle list.
***********************************************************************************************************************/
void NavDataProcess::convertToTriangleList()
{
	std::vector<std::array<uint64_t, 3>> triangles;
	// Finally sort into triangle lists and get outlines ready for tiling
	for (auto wayIterator = _osm.triangulatedRoads.begin(); wayIterator != _osm.triangulatedRoads.end(); ++wayIterator)
	{
		ConvertedWay convertedRoad(wayIterator->first, wayIterator->second.area, wayIterator->second.tags, wayIterator->second.roadType,
		                           wayIterator->second.width, wayIterator->second.isIntersection, wayIterator->second.isRoundabout);

		if (wayIterator->second.area) //Handle road areas
		{
			if (checkWinding(wayIterator->second.nodeIds) == pvr::PolygonWindingOrder::FrontFaceCW)
			{
				std::reverse(wayIterator->second.nodeIds.begin(), wayIterator->second.nodeIds.end());
			}

			triangulate(wayIterator->second.nodeIds, triangles);

			for (uint32_t i = 0; i < triangles.size(); ++i)
			{
				convertedRoad.triangulatedIds.push_back(triangles[i]);
			}
		}
		else
		{
			Way& way = _osm.originalRoadWays.find(wayIterator->first)->second;

			//Calculate end caps for roads which are dead ends.
			if (way.nodeIds.size() > 1)
			{
				//End of road segment.
				if (_osm.getNodeById(way.nodeIds.back()).wayIds.size() == 1)
				{
					Vertex& n1 = _osm.getNodeById(wayIterator->second.nodeIds.back());
					Vertex& n2 = _osm.getNodeById(wayIterator->second.nodeIds.back() - 1);

					//Check both nodes are within map limits to prevent artefacts.
					if (!isOutOfBounds(n1.coords) && !isOutOfBounds(n2.coords))
					{
						auto nodes = calculateEndCaps(n1, n2, wayIterator->second.width);

						wayIterator->second.nodeIds.push_back(nodes[0]);
						wayIterator->second.nodeIds.push_back(n2.id); //Need a repeated node to complete triangle list.
						wayIterator->second.nodeIds.push_back(nodes[1]);
					}
				}
				//Start of road segment.
				if (_osm.getNodeById(way.nodeIds[0]).wayIds.size() == 1)
				{
					Vertex& n1 = _osm.getNodeById(wayIterator->second.nodeIds[0]);
					Vertex& n2 = _osm.getNodeById(wayIterator->second.nodeIds[1]);

					//Check both nodes are within map limits to prevent artefacts.
					if (!isOutOfBounds(n1.coords) && !isOutOfBounds(n2.coords))
					{
						auto nodes = calculateEndCaps(n1, n2, wayIterator->second.width);

						wayIterator->second.nodeIds.insert(wayIterator->second.nodeIds.begin(), nodes[0]);
						wayIterator->second.nodeIds.insert(wayIterator->second.nodeIds.begin(), n2.id); //Need a repeated node to complete triangle list.
						wayIterator->second.nodeIds.insert(wayIterator->second.nodeIds.begin(), nodes[1]);
					}
				}
			}

			for (uint32_t i = 0; i < (wayIterator->second.nodeIds.size() - 2); ++i)
			{
				const Vertex& node0 = i % 2 == 0 ? _osm.getNodeById(wayIterator->second.nodeIds[i]) : _osm.getNodeById(wayIterator->second.nodeIds[i + 1]);
				const Vertex& node1 = i % 2 == 0 ? _osm.getNodeById(wayIterator->second.nodeIds[i + 1]) : _osm.getNodeById(wayIterator->second.nodeIds[i]);
				const Vertex& node2 = _osm.getNodeById(wayIterator->second.nodeIds[i + 2]);
				convertedRoad.triangulatedIds.push_back(std::array<uint64_t, 3> {node0.id, node1.id, node2.id});
			}
		}
		_osm.convertedRoads[convertedRoad.id] = convertedRoad;
	}
}

/*!*********************************************************************************************************************
\brief  Sort the ways into the tiles.
***********************************************************************************************************************/
void NavDataProcess::sortTiles()
{
	uint64_t id = 0;
	// Tile roads
	for (auto && wayMapEntry : _osm.convertedRoads)
	{
		auto& way = wayMapEntry.second;
		for (uint32_t i = 0; i < way.triangulatedIds.size(); ++i)
		{
			// pass all three of them in the fill tiles
			// need a seaprate clippig function for roads only
			const Vertex& vertex0 = _osm.getNodeById(way.triangulatedIds[i][0]);
			const Vertex& vertex1 = _osm.getNodeById(way.triangulatedIds[i][1]);
			const Vertex& vertex2 = _osm.getNodeById(way.triangulatedIds[i][2]);

			clipRoad(vertex0, vertex1, vertex2, id, way.tags, WayTypes::Road, way.area, way.roadType, way.width,
			         way.isIntersection, way.isRoundabout);
			id++;
		}
	}

	for (uint32_t lod = 0; lod < LOD::Count; ++lod)
	{
		//Labels
		for (uint32_t i = 0; i < _osm.labels[lod].size(); ++i)
		{
			fillLabelTiles(_osm.labels[lod][i], lod);
		}

		//Icons
		for (uint32_t i = 0; i < _osm.icons[lod].size(); ++i)
		{
			fillIconTiles(_osm.icons[lod][i], lod);
		}

		//Amenity Labels
		for (uint32_t i = 0; i < _osm.amenityLabels[lod].size(); ++i)
		{
			fillAmenityTiles(_osm.amenityLabels[lod][i], lod);
		}
	}

	// Tile car parking
	id = 0;
	std::vector<Way> innerWays;
	std::vector<std::array<uint64_t, 3>> triangles;
	for (auto && wayMapEntry : _osm.parkingWays)
	{
		auto& way = wayMapEntry.second;
		if (checkWinding(way.nodeIds) == pvr::PolygonWindingOrder::FrontFaceCW)
		{
			std::reverse(way.nodeIds.begin(), way.nodeIds.end());
		}

		if (way.inner)
		{
			innerWays.push_back(way);
			continue;
		}

		triangulate(way.nodeIds, triangles);


		for (uint32_t i = 0; i < triangles.size(); ++i)
		{
			const Vertex& vertex0 = _osm.getNodeById(triangles[i][0]);
			const Vertex& vertex1 = _osm.getNodeById(triangles[i][1]);
			const Vertex& vertex2 = _osm.getNodeById(triangles[i][2]);


			clipRoad(vertex0, vertex1, vertex2, id, way.tags, WayTypes::Parking, way.area, way.roadType, way.width,
			         way.isIntersection, way.isRoundabout);

			id++;
		}
	}

	// Tile buildings
	id = 0;
	for (auto && wayMapEntry : _osm.buildWays)
	{
		auto& way = wayMapEntry.second;
		if (checkWinding(way.nodeIds) == pvr::PolygonWindingOrder::FrontFaceCW)
		{
			std::reverse(way.nodeIds.begin(), way.nodeIds.end());
		}

		if (way.inner)
		{
			innerWays.push_back(way);
			continue;
		}

		triangulate(way.nodeIds, triangles);

		for (uint32_t i = 0; i < triangles.size(); ++i)
		{
			Vertex vertex0 = _osm.getNodeById(triangles[i][0]);
			Vertex vertex1 = _osm.getNodeById(triangles[i][1]);
			Vertex vertex2 = _osm.getNodeById(triangles[i][2]);

			clipRoad(vertex0, vertex1, vertex2, id, way.tags, WayTypes::Building, way.area, way.roadType, way.width,
			         way.isIntersection, way.isRoundabout);
			id++;
		}
	}

	// Tile inner ways
	id = 0;
	for (auto way : innerWays)
	{
		triangulate(way.nodeIds, triangles);

		for (uint32_t i = 0; i < triangles.size(); ++i)
		{
			Vertex vertex0 = _osm.getNodeById(triangles[i][0]);
			Vertex vertex1 = _osm.getNodeById(triangles[i][1]);
			Vertex vertex2 = _osm.getNodeById(triangles[i][2]);

			clipRoad(vertex0, vertex1, vertex2, id, way.tags, WayTypes::Inner, way.area, way.roadType, way.width,
			         way.isIntersection, way.isRoundabout);
			id++;
		}
	}
}

/*!*********************************************************************************************************************
\param  tileCoords   The tile co-ordinates where the data should be inserted.
\param  type     The type of way to insert (determines which vector the way will be inserted into).
\param  way      The way to insert.
\param  id       The node ID to insert.
\brief  Determine the correct array to insert the way / node id based on way type.
***********************************************************************************************************************/
void NavDataProcess::insert(const glm::uvec2& tileCoords, WayTypes::WayTypes type, Way* w, uint64_t id)
{
	switch (type)
	{
	case  WayTypes::Road:
	{
		if (w->area)
		{
			insertWay(_osm.tiles[tileCoords.x][tileCoords.y].areaWays, *w);
		}
		else
		{
			insertWay(_osm.tiles[tileCoords.x][tileCoords.y].roadWays, *w);
		}
		break;
	}
	case WayTypes::Parking:
	{
		insertWay(_osm.tiles[tileCoords.x][tileCoords.y].parkingWays, *w);
		break;
	}
	case WayTypes::Building:
	{
		insertWay(_osm.tiles[tileCoords.x][tileCoords.y].buildWays, *w);
		break;
	}
	case WayTypes::Inner:
	{
		insertWay(_osm.tiles[tileCoords.x][tileCoords.y].innerWays, *w);
		break;
	}
	default:
	{
		Log(LogLevel::Information, "Unrecognised way type.");
		break;
	}
	}
}

/*!*********************************************************************************************************************
\param  insertIn   The vector of ways in which to insert.
\param  way      The way to insert.
\brief  Insert a way (or a node ID) into a given array of ways.
***********************************************************************************************************************/
void NavDataProcess::insertWay(std::vector<Way>& insertIn, Way& way)
{
	if ((!insertIn.empty() && (insertIn.rbegin()->id == way.id)))
	{
		std::for_each(way.nodeIds.begin(), way.nodeIds.end(),
		[&](uint64_t nodeId) { insertIn.rbegin()->nodeIds.push_back(nodeId); });
	}
	else
	{
		insertIn.push_back(way);
	}
}

void NavDataProcess::processLabelBoundary(LabelData& label, glm::uvec2& tileCoords)
{
	glm::dvec2 min = _osm.tiles[tileCoords.x][tileCoords.y].min;
	glm::dvec2 max = _osm.tiles[tileCoords.x][tileCoords.y].max;

	glm::dvec2 leftBoundary = glm::dvec2(findIntersect(min, max, label.coords, label.coords - glm::dvec2(max.x * 2.0, 0)));
	glm::dvec2 rightBoundary = glm::dvec2(findIntersect(min, max, label.coords, label.coords + glm::dvec2(max.x * 2.0, 0)));
	glm::dvec2 topBoundary = glm::dvec2(findIntersect(min, max, label.coords, label.coords + glm::dvec2(0, max.y * 2.0)));
	glm::dvec2 bottomBoundary = glm::dvec2(findIntersect(min, max, label.coords, label.coords - glm::dvec2(0, max.y * 2.0)));
	double d1 = glm::distance2(leftBoundary, label.coords);
	double d2 = glm::distance2(rightBoundary, label.coords);
	double d3 = glm::distance2(topBoundary, label.coords);
	double d4 = glm::distance2(bottomBoundary, label.coords);

	label.distToBoundary = static_cast<float>(glm::sqrt(glm::min(glm::min(glm::min(d1, d2), d3), d4)));
}

/*!*********************************************************************************************************************
\brief  Fill tiles with Amenity label data.
***********************************************************************************************************************/
void NavDataProcess::fillAmenityTiles(AmenityLabelData& label, uint32_t lod)
{
	// Check if label is out of the map bounds
	if (isOutOfBounds(label.coords))
	{
		return;
	}

	glm::uvec2 tileCoords = findTile2(label.coords);
	processLabelBoundary(label, tileCoords);
	_osm.tiles[tileCoords.x][tileCoords.y].amenityLabels[lod].push_back(label);
}

/*!*********************************************************************************************************************
\brief  Fill tiles with Amenity label data.
***********************************************************************************************************************/
void NavDataProcess::fillLabelTiles(LabelData& label, uint32_t lod)
{
	// Check if label is out of the map bounds
	if (isOutOfBounds(label.coords))
	{
		return;
	}

	glm::uvec2 tileCoords = findTile2(label.coords);
	processLabelBoundary(label, tileCoords);

	_osm.tiles[tileCoords.x][tileCoords.y].labels[lod].push_back(label);
}

/*!*********************************************************************************************************************
\brief  Fill tiles with icon data.
***********************************************************************************************************************/
void NavDataProcess::fillIconTiles(IconData& icon, uint32_t lod)
{
	// Check if icon is out of the map bounds
	if (isOutOfBounds(icon.coords))
	{
		return;
	}

	glm::uvec2 tileCoords = findTile2(icon.coords);

	_osm.tiles[tileCoords.x][tileCoords.y].icons[lod].push_back(icon);
}

//The range of angles at which a bend should be tessellated - no need to tessellate almost flat road segments.
static const float lowerThreshold = 15.0f;

/*!*********************************************************************************************************************
\return std::vector<uint64_t> A vector of old + newly generated node IDs - line strip.
\param  oldNodes Node IDs of the incoming road linestrip.
\brief  Increases the complexity of the geometry to smooth out harsh corners - the min/max angle is configurable.
The algorithm iterates over the oldNodeIDs vector and selects 3 nodes (start, control, end) on each pass,
a bezier curve is then generated with x number of steps. Note that the algorithm may return the exact same nodes if the road is fairly straight.
***********************************************************************************************************************/
std::vector<uint64_t> NavDataProcess::tessellate(const std::vector<uint64_t>& oldNodeIDs, Real width)
{
	std::vector<uint64_t> newIds;
	/*How detailed the curve will be. A smaller number will increase the geometry's complexity.
	(This could negatively impact performance)*/

	glm::dvec2 lastPointOnCurve;
	bool middleNodeAdded = false;
	newIds.push_back(oldNodeIDs.front());

	for (uint32_t i = 1; i < (oldNodeIDs.size() - 1); ++i)
	{
		const Vertex& node0 = _osm.getNodeById(oldNodeIDs[i - 1]);
		Vertex node1 = _osm.getNodeById(oldNodeIDs[i]);
		const Vertex& node2 = _osm.getNodeById(oldNodeIDs[i + 1]);

		glm::dvec2 v1 = ((middleNodeAdded ? lastPointOnCurve : node0.coords) - node1.coords);
		glm::dvec2 v2 = node2.coords - node1.coords/*(middleNodeAdded ? (node1.coords + offset) : node1.coords)*/;
		Real lenv1 = glm::length(node0.coords - node1.coords), lenv2 = glm::length(node1.coords - node2.coords);

		Real segments_length = std::min(lenv1, lenv2);
		//Calculate angle between road segments (v1, v2).
		double angle = glm::degrees(glm::acos(glm::dot(glm::normalize(v1), glm::normalize(v2))));

		//Check angle is within thresholds and node1 is not outside of map boundary and it is not an intersection (to prevent artefacts).
		// Don't try to tessellate tiny segments.
		if (!isOutOfBounds(node1.coords) && (node1.wayIds.size() == 1) &&
		    angle > lowerThreshold && angle < 180 - lowerThreshold && segments_length > width * .40)
		{
			int numSteps_angle = 1 + (int)((1.f - (angle / (180.f))) * 9.f);

			middleNodeAdded = false;

			Vec2 normv1 = glm::normalize(v1);
			Vec2 normv2 = glm::normalize(v2);

			Real segmentFactorSize1 = std::min(0.45, .25 * width * numSteps_angle / lenv1); // We want it to be at most half the length of the road, ideally an entire road width.
			Real segmentFactorSize2 = std::min(0.45, .25 * width * numSteps_angle / lenv2); // We want it to be at most half the length of the road, ideally an entire road width.

			Real segmentFixedSize1 = segmentFactorSize1 * lenv1;
			Real segmentFixedSize2 = segmentFactorSize1 * lenv2;

			Real segmentSize = std::min(segmentFixedSize1, segmentFixedSize2);

			Real segmentFactorSize = std::min(segmentFactorSize1, segmentFactorSize2);

			Real widthFactor1 = segmentSize / width;
			static Real widthFactorMax = widthFactor1;
			static Real widthFactorMin = widthFactor1;
			widthFactorMax = std::max(widthFactor1, widthFactorMax);
			widthFactorMin = std::min(widthFactor1, widthFactorMin);


			//Compute the start and end locations for the bezier curve.
			glm::dvec2 startPos = node1.coords + (normv1 * segmentSize);
			glm::dvec2 endPos = node1.coords + (normv2 * segmentSize);
			// We will accept at most one added segment per width/5 of road
			// Ideally, we want to break stuff until all angles are < 15deg.

			// But, we want to be careful that we do not move too far back

			float numsteps_curve_length = static_cast<float>(5.f * segmentFactorSize / .45);
			int numSteps = std::min(numSteps_angle, int(numsteps_curve_length));

			Real stepValue = 1.0 / (1 + numSteps);


			for (float interpolant = 0.f; interpolant <= 1.0f; interpolant += static_cast<float>(stepValue))
			{
				//Calculate new point on the curve.
				glm::dvec2 a = glm::mix(startPos, node1.coords, interpolant);
				glm::dvec2 b = glm::mix(node1.coords, endPos, interpolant);
				glm::dvec2 newCoords = glm::mix(a, b, interpolant);

				Vertex newNode(node1);
				/*Copy the control node into the newIds when we reach the center
				of the curve (approx) only updating its position (to preserve intersections).*/
				if (interpolant >= 0.5 && !middleNodeAdded)
				{
					middleNodeAdded = true;
				}
				else //Create a new node
				{
					newNode.id = _osm.nodes.rbegin()->first + 1;
				}

				newNode.coords = lastPointOnCurve = newCoords;
				_osm.insertOrOverwriteNode(std::move(newNode));
				newIds.push_back(newNode.id);
			}
		}
		else
		{
			newIds.push_back(node1.id);
			middleNodeAdded = false;
		}
	}
	newIds.push_back(oldNodeIDs.back());
	return newIds;
}

/*!*********************************************************************************************************************
\return std::vector<uint64_t> A vector of node IDs representing triangle strips.
\param  nodeIds Node IDs of a road linestrip.
\brief  Triangluates a road line strip into a triangle strip.
***********************************************************************************************************************/
std::vector<uint64_t> NavDataProcess::triangulateRoad(const std::vector<uint64_t>& nodeIds, const double width)
{
	std::vector<uint64_t> newNodeIds;

	if (nodeIds.size() == 2)
	{
		uint64_t id = _osm.nodes.rbegin()->first + 1;
		const Vertex& node0 = _osm.getNodeById(nodeIds[0]);
		const Vertex& node1 = _osm.getNodeById(nodeIds[1]);

		// Find coordinates of new points
		std::array<glm::dvec2, 2> firstPerps = findPerpendicularPoints(node0.coords, node1.coords, width, 1);
		std::array<glm::dvec2, 2> secPerps = findPerpendicularPoints(node0.coords, node1.coords, width, 2);

		// Create new nodes
		Vertex newNode0(id, firstPerps[0], false, glm::vec2(TexUVLeft, TexUVUp));
		Vertex newNode1(++id, firstPerps[1], false, glm::vec2(TexUVRight, TexUVUp));
		Vertex newNode2(++id, secPerps[0], false, glm::vec2(TexUVLeft, TexUVUp));
		Vertex newNode3(++id, secPerps[1], false, glm::vec2(TexUVRight, TexUVUp));
		_osm.insertOrOverwriteNode(std::move(newNode0));
		_osm.insertOrOverwriteNode(std::move(newNode1));
		_osm.insertOrOverwriteNode(std::move(newNode2));
		_osm.insertOrOverwriteNode(std::move(newNode3));

		// Create triangles
		newNodeIds.push_back(newNode0.id);
		newNodeIds.push_back(newNode1.id);
		newNodeIds.push_back(newNode2.id);
		newNodeIds.push_back(newNode3.id);
	}
	else
	{
		{
			// Add first item
			uint64_t id = _osm.nodes.rbegin()->first + 1;
			std::array<glm::dvec2, 2> firstPerps = findPerpendicularPoints(_osm.getNodeById(nodeIds[0]).coords, _osm.getNodeById(nodeIds[1]).coords, width, 1);
			Vertex newNode0(id, firstPerps[0], false, glm::vec2(TexUVLeft, TexUVUp));
			Vertex newNode1(++id, firstPerps[1], false, glm::vec2(TexUVRight, TexUVUp));
			_osm.insertOrOverwriteNode(std::move(newNode0));
			_osm.insertOrOverwriteNode(std::move(newNode1));
			newNodeIds.push_back(newNode0.id);
			newNodeIds.push_back(newNode1.id);
		}

		for (uint32_t i = 1; i < (nodeIds.size() - 1); ++i)
		{
			uint64_t id = _osm.nodes.rbegin()->first + 1;
			const Vertex& node0 = _osm.getNodeById(nodeIds[i - 1]);
			const Vertex& node1 = _osm.getNodeById(nodeIds[i]);
			const Vertex& node2 = _osm.getNodeById(nodeIds[i + 1]);

			// Find coordinates of new points for the middle node
			std::array<glm::dvec2, 2> secPerps = findPerpendicularPoints(node0.coords, node1.coords, node2.coords, width);

			Vertex newNode2(++id, secPerps[0], false, glm::vec2(TexUVLeft, TexUVUp));
			Vertex newNode3(++id, secPerps[1], false, glm::vec2(TexUVRight, TexUVUp));

			_osm.insertOrOverwriteNode(std::move(newNode2));
			_osm.insertOrOverwriteNode(std::move(newNode3));

			newNodeIds.push_back(newNode2.id);
			newNodeIds.push_back(newNode3.id);


		}

		{
			// Add last item
			uint64_t id = _osm.nodes.rbegin()->first + 1;
			std::array<glm::dvec2, 2> thirdPerps;
			thirdPerps = findPerpendicularPoints(_osm.getNodeById(*(nodeIds.end() - 2)).coords, _osm.getNodeById(*(nodeIds.end() - 1)).coords, width, 2);

			Vertex newNode4(id++, thirdPerps[0], false, glm::vec2(TexUVLeft, TexUVUp));
			Vertex newNode5(id++, thirdPerps[1], false, glm::vec2(TexUVRight, TexUVUp));
			_osm.insertOrOverwriteNode(std::move(newNode4));
			_osm.insertOrOverwriteNode(std::move(newNode5));

			newNodeIds.push_back(newNode4.id);
			newNodeIds.push_back(newNode5.id);
		}
	}
	for (auto && newNodeId : newNodeIds)
	{
		debug_assertion(_osm.getNodeById(newNodeId).texCoords.x != -10000.f && _osm.getNodeById(newNodeId).texCoords.y != -10000.f, "TexCoord DEFAULT");
	}

	return newNodeIds;
}

/*!*********************************************************************************************************************
\return std::vector<std::vector<uint64_t>> A vector of triangle ways.
\param  nodeIds A vector of node IDs for a closed way (anti-clockwise wound).
\brief  Triangluates an anti-clockwise wound closed way.
***********************************************************************************************************************/
void NavDataProcess::triangulate(std::vector<uint64_t>& nodeIds, std::vector<std::array<uint64_t, 3>>& triangles) const
{
	triangles.clear();
	if (nodeIds.front() == nodeIds.back())
	{
		nodeIds.pop_back();
	}

	while (nodeIds.size() >= 3)
	{
		size_t size = nodeIds.size();

		for (uint32_t i = 0; i < nodeIds.size(); ++i)
		{
			Vertex prevNode;
			Vertex nextNode;
			Vertex currentNode = _osm.getNodeById(nodeIds[i]);
			std::vector<uint64_t> otherNodes = nodeIds;
			otherNodes.erase(otherNodes.begin() + i);

			if (i == 0)
			{
				prevNode = _osm.getNodeById(nodeIds.back());
				nextNode = _osm.getNodeById(nodeIds[1]);
				otherNodes.pop_back();
				otherNodes.erase(otherNodes.begin());
			}
			else if (i == (nodeIds.size() - 1))
			{
				prevNode = _osm.getNodeById(nodeIds[i - 1]);
				nextNode = _osm.getNodeById(nodeIds[0]);
				otherNodes.pop_back();
				otherNodes.erase(otherNodes.begin());
			}
			else
			{
				prevNode = _osm.getNodeById(nodeIds[i - 1]);
				nextNode = _osm.getNodeById(nodeIds[i + 1]);
				otherNodes.erase(otherNodes.begin() + i);
				otherNodes.erase(otherNodes.begin() + i - 1);
			}

			// Check if the vertex is interior or exterior
			if (checkWinding(std::vector<glm::dvec2> {prevNode.coords, currentNode.coords, nextNode.coords}) == pvr::PolygonWindingOrder::FrontFaceCW)
				continue;

			// Determine if any of the other points are inside the triangle
			bool pointInTriangle = false;
			for (uint32_t j = 0; j < otherNodes.size(); ++j)
			{
				glm::dvec2 prevPoint = prevNode.coords - currentNode.coords;
				glm::dvec2 nextPoint = nextNode.coords - currentNode.coords;
				glm::dvec2 currentPoint = _osm.getNodeById(otherNodes[j]).coords - currentNode.coords;
				double d = prevPoint.x * nextPoint.y - nextPoint.x * prevPoint.y;

				double currentWeight = (currentPoint.x * (prevPoint.y - nextPoint.y) + currentPoint.y *
				                        (nextPoint.x - prevPoint.x) + prevPoint.x * nextPoint.y - nextPoint.x * prevPoint.y) / d;

				double prevWeight = (currentPoint.x * nextPoint.y - currentPoint.y * nextPoint.x) / d;
				double nextWeight = (currentPoint.y * prevPoint.x - currentPoint.x * prevPoint.y) / d;

				if ((currentWeight > 0.0) && (currentWeight < 1.0) && (prevWeight > 0.0) && (prevWeight < 1.0) && (nextWeight > 0.0) && (nextWeight < 1.0))
				{
					pointInTriangle = true;
					break;
				}
			}

			if (pointInTriangle)
			{
				continue;
			}

			// Add the new triangle and remove the necessary point
			triangles.push_back(std::array<uint64_t, 3> {prevNode.id, currentNode.id, nextNode.id});
			nodeIds.erase(nodeIds.begin() + i);
			break;
		}

		if (size == nodeIds.size())
		{
			break;
		}
	}
}

/*!*********************************************************************************************************************
\return Array The newly generated nodes that create the end cap.
\param  Node first The first node at the start/end of the road segment.
\param  Node second The second node at the start/end of the road segment.
\param  Width This directly affects the cap size.
\brief  Calculates an end cap (i.e. the smoothed/curved end to a road segment when it does not connect to another road) for a given road segment.
***********************************************************************************************************************/
std::array<uint64_t, 2> NavDataProcess::calculateEndCaps(Vertex& first, Vertex& second, double width)
{
	//Calculate vector betweeen end nodes.
	glm::dvec2 v1 = first.coords - second.coords;
	//Calculate a perpendicular vector.
	glm::vec3 perp = glm::normalize(glm::cross(glm::vec3(v1, 0), glm::vec3(0, 0, 1)));
	//Project the perpendicular vector by width / 2
	v1 = glm::dvec2(perp) * (width / 2.0);

	first.texCoords.y = second.texCoords.y = 2 * TexUVUp;

	Vertex newNode1 = first;
	Vertex newNode2 = second;

	//Setup new nodes used for end cap.
	newNode1.coords -= v1;
	newNode1.texCoords.y = 4 * TexUVUp;
	newNode1.id = _osm.nodes.rbegin()->first + 1;
	_osm.insertOrOverwriteNode(std::move(newNode1));
	debug_assertion(newNode1.texCoords.x != -10000.f && newNode1.texCoords.y != -10000.f, "TexCoord DEFAULT");

	newNode2.coords -= v1;
	newNode2.texCoords.y = 4 * TexUVUp;
	newNode2.id = _osm.nodes.rbegin()->first + 1;
	_osm.insertOrOverwriteNode(std::move(newNode2));
	debug_assertion(newNode2.texCoords.x != -10000.f && newNode2.texCoords.y != -10000.f, "TexCoord DEFAULT");

	std::array<uint64_t, 2> retval;
	retval[0] = newNode1.id;
	retval[1] = newNode2.id;
	return retval;
}

void addNewVerticesFromClipping(
  const Vertex& quadVertex0, const Vertex& quadVertex1, const Vertex& triVertex2,
  glm::vec2 vec2to0, glm::vec2 vec1to2,
  float dist2to0, float dist1to2,
  float clipDistance20, float clipDistance12,
  Vertex* triFront, uint32_t& numTriFront,
  Vertex* triBack, uint32_t& numTriBack,
  const glm::vec2& planeNorm
)
{
	glm::vec2 new12 = (glm::vec2)quadVertex1.coords + clipDistance12 * vec1to2;
	glm::vec2 new20 = (glm::vec2)triVertex2.coords + clipDistance20 * vec2to0;
	glm::vec2 new12uv = glm::mix(quadVertex1.texCoords, triVertex2.texCoords, clipDistance12 / dist1to2);
	glm::vec2 new20uv = glm::mix(triVertex2.texCoords, quadVertex0.texCoords, clipDistance20 / dist2to0);

	Vertex v12 = Vertex(-1, new12, true, new12uv);
	Vertex v20 = Vertex(-1, new20, true, new20uv);

	//front
	bool triIsFront = (distanceToPlane((glm::vec2)triVertex2.coords, glm::dot(new12, planeNorm), planeNorm) > 0);
	Vertex* triangle = triIsFront ? triFront : triBack;
	Vertex* quad = triIsFront ? triBack : triFront;
	numTriFront = triIsFront ? 1 : 2;
	numTriBack = triIsFront ? 2 : 1;

	triangle[0] = triVertex2;
	triangle[1] = v20;
	triangle[2] = v12;

	quad[0] = quadVertex0;
	quad[1] = quadVertex1;
	quad[2] = v12;
	quad[3] = quadVertex0;
	quad[4] = v12;
	quad[5] = v20;
}


inline void debug_assert_vertices_greater_than(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2,
    glm::vec2 tile_min, bool vertical)
{
	if (vertical)
	{
		assertion(vertex0.coords.x >= tile_min.x - epsilon, "Vertex 0 min X");
		assertion(vertex1.coords.x >= tile_min.x - epsilon, "Vertex 1 min X");
		assertion(vertex2.coords.x >= tile_min.x - epsilon, "Vertex 2 min X");
	}
	else
	{
		assertion(vertex0.coords.y >= tile_min.y - epsilon, "Vertex 0 min Y");
		assertion(vertex1.coords.y >= tile_min.y - epsilon, "Vertex 1 min Y");
		assertion(vertex2.coords.y >= tile_min.y - epsilon, "Vertex 2 min Y");
	}
}
inline void debug_assert_vertices_less_than(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2,
    glm::vec2 tile_max, bool vertical)
{
	if (vertical)
	{
		assertion(vertex0.coords.x <= tile_max.x + epsilon, "Vertex 0 max X");
		assertion(vertex1.coords.x <= tile_max.x + epsilon, "Vertex 1 max X");
		assertion(vertex2.coords.x <= tile_max.x + epsilon, "Vertex 2 max X");
	}
	else
	{
		assertion(vertex0.coords.y <= tile_max.y + epsilon, "Vertex 0 max Y");
		assertion(vertex1.coords.y <= tile_max.y + epsilon, "Vertex 1 max Y");
		assertion(vertex2.coords.y <= tile_max.y + epsilon, "Vertex 2 max Y");

	}
}

void NavDataProcess::clipAgainst(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2, glm::vec2 planeOrigin,
                                 const glm::vec2& planeNorm, Vertex* triFront, Vertex* triBack, uint32_t& numTriFront, uint32_t& numTriBack)
{
	numTriFront = 0, numTriBack = 0;
	glm::vec2 vec0to1 = (glm::vec2)glm::normalize(vertex1.coords - vertex0.coords);
	glm::vec2 vec2to0 = (glm::vec2)glm::normalize(vertex0.coords - vertex2.coords);
	glm::vec2 vec1to2 = (glm::vec2)glm::normalize(vertex2.coords - vertex1.coords);
	float dist0to1 = static_cast<float>(glm::distance(vertex1.coords, vertex0.coords));
	float dist2to0 = static_cast<float>(glm::distance(vertex0.coords, vertex2.coords));
	float dist1to2 = static_cast<float>(glm::distance(vertex2.coords, vertex1.coords));
	bool rslt01, rslt12, rslt20;

	float clipDistance01 = 0;
	rslt01 = pvr::math::intersectLinePlane((glm::vec2)vertex0.coords, vec0to1, planeOrigin, planeNorm, clipDistance01);
	rslt01 = (rslt01 && (clipDistance01 > 0.f && clipDistance01 <= dist0to1)); //Lines not parallel, but do not cross.

	float clipDistance12 = 0;
	rslt12 = pvr::math::intersectLinePlane((glm::vec2)vertex1.coords, vec1to2, planeOrigin, planeNorm, clipDistance12);
	rslt12 = (rslt12 && (clipDistance12 > 0.f && clipDistance12 <= dist1to2)); //Lines not parallel, but do not cross.

	float clipDistance20 = 0;
	rslt20 = pvr::math::intersectLinePlane((glm::vec2)vertex2.coords, vec2to0, planeOrigin, planeNorm, clipDistance20);
	rslt20 = (rslt20 && (clipDistance20 > 0.f && clipDistance20 <= dist2to0)); //Lines not parallel, but do not cross.

	// Detecting the case where we're so "lucky" that one of the vertices is on the plane and the other two are on opposite sides...
	int num_intersections = (rslt01 + rslt12 + rslt20);
	//
	// n triangle must have either no intersection or 2 intersection
	assertion(num_intersections < 3, "INTERSECTION ERROR: Cannot have 3 intersections in line vs triangle.");

	// triangle must be oneside of the line
	if (num_intersections == 0)
	{
		float dot1 = glm::dot((glm::vec2)vertex0.coords - planeOrigin, planeNorm);
		float dot2 = glm::dot((glm::vec2)vertex1.coords - planeOrigin, planeNorm);
		float dot3 = glm::dot((glm::vec2)vertex2.coords - planeOrigin, planeNorm);
		assertion(glm::abs(dot1) >= epsilon || glm::abs(dot2) >= epsilon || glm::abs(dot3) >= epsilon,
		          "ClipRoads:Triangle vertices are all on the same line!");

		assertion(
		  (dot1 >= -epsilon &&
		   dot2 >= -epsilon &&
		   dot3 >= -epsilon) ||
		  (dot1 <= epsilon &&
		   dot2 <= epsilon &&
		   dot3 <= epsilon),
		  "ClipRoads:Triangle is not clipped, but on different sides of the plane");

		if (
		  dot1 > epsilon ||
		  dot2 > epsilon ||
		  dot3 > epsilon
		) // front?
		{
			numTriFront = 1;
			*triFront++ = vertex0;
			*triFront++ = vertex1;
			*triFront++ = vertex2;
		}
		else// back?
		{
			numTriBack = 1;
			*triBack++ = vertex0;
			*triBack++ = vertex1;
			*triBack++ = vertex2;
		}
		return;
	}
	// MUST BE: 2 true, 1 false. Creates a QUAD and a TRIANGLE
	else if (num_intersections == 2)
	{
		if (!rslt01) // Quad is: 0->1->NEW12->NEW20->0, triangle is 2->NEW20->NEW12
		{
			addNewVerticesFromClipping(
			  vertex0, vertex1, vertex2,
			  vec2to0, vec1to2,
			  dist2to0, dist1to2,
			  clipDistance20, clipDistance12,
			  triFront, numTriFront,
			  triBack, numTriBack,
			  planeNorm);
		}
		else if (!rslt12) // Quad is: 1->2->NEW20->NEW01, triangle is 0->NEW01->NEW20
		{
			addNewVerticesFromClipping(
			  vertex1, vertex2, vertex0,
			  vec0to1, vec2to0,
			  dist0to1, dist2to0,
			  clipDistance01, clipDistance20,
			  triFront, numTriFront,
			  triBack, numTriBack,
			  planeNorm);
		}
		else if (!rslt20) // Quad is: 2->0->NEW01->NEW12, triangle is 1->NEW12->NEW01
		{
			addNewVerticesFromClipping(
			  vertex2, vertex0, vertex1,
			  vec1to2, vec0to1,
			  dist1to2, dist0to1,
			  clipDistance12, clipDistance01,
			  triFront, numTriFront,
			  triBack, numTriBack,
			  planeNorm);

		}
	}
	else if (num_intersections == 1) //Wow... one of the vertices exactly on the plane, by chance?!
	{
		if (rslt01) // Vertex 2 is actually on the plane, 0 and 1 are on opposite sides of it. Triangles is: NEW->1->2 and NEW->2->0
		{

			glm::dvec2 newp_coords = vertex0.coords + (double)clipDistance01 * glm::dvec2(vec0to1);
			glm::vec2 newp_uvs = glm::mix(vertex0.texCoords, vertex1.texCoords, clipDistance01 / dist0to1);

			Vertex newVert = Vertex(-1, newp_coords, true, newp_uvs);

			//front
			bool vertex0isFront = (distanceToPlane((glm::vec2)vertex0.coords, glm::dot(glm::vec2(vertex2.coords), planeNorm), planeNorm) <= 0);
			numTriFront = numTriBack = 1;
			Vertex* tri0 = vertex0isFront ? triFront : triBack;
			Vertex* tri1 = vertex0isFront ? triBack : triFront;
			tri0[0] = newVert;
			tri0[1] = vertex1;
			tri0[2] = vertex2;
			tri1[0] = newVert;
			tri1[1] = vertex2;
			tri1[2] = vertex0;
		}
		else if (rslt12) // Triangles is: NEW->2->0 and NEW->0->1
		{
			glm::dvec2 newp_coords = vertex1.coords + (double)clipDistance12 * glm::dvec2(vec1to2);
			glm::vec2 newp_uvs = glm::mix(vertex1.texCoords, vertex2.texCoords, clipDistance12 / dist1to2);

			Vertex newVert = Vertex(-1, newp_coords, true, newp_uvs);

			//front
			bool vertex1isFront = (distanceToPlane((glm::vec2)vertex1.coords, glm::dot(glm::vec2(vertex0.coords), planeNorm), planeNorm) <= 0);
			numTriFront = numTriBack = 1;
			Vertex* tri1 = vertex1isFront ? triFront : triBack;
			Vertex* tri2 = vertex1isFront ? triBack : triFront;
			tri1[0] = newVert;
			tri1[1] = vertex2;
			tri1[2] = vertex0;
			tri2[0] = newVert;
			tri2[1] = vertex0;
			tri2[2] = vertex1;
		}
		else if (rslt20) // Triangles is: NEW->0->1 and NEW->1->2
		{
			glm::dvec2 newp_coords = vertex2.coords + (double)clipDistance20 * glm::dvec2(vec2to0);
			glm::vec2 newp_uvs = glm::mix(vertex2.texCoords, vertex0.texCoords, clipDistance20 / dist2to0);

			Vertex newVert = Vertex(-1, newp_coords, true, newp_uvs);

			//front
			bool vertex2isFront = (distanceToPlane((glm::vec2)vertex2.coords, glm::dot(glm::vec2(vertex1.coords), planeNorm), planeNorm) <= 0);
			numTriFront = numTriBack = 1;
			Vertex* tri2 = vertex2isFront ? triFront : triBack;
			Vertex* tri0 = vertex2isFront ? triBack : triFront;
			tri2[0] = newVert;
			tri2[1] = vertex0;
			tri2[2] = vertex1;
			tri0[0] = newVert;
			tri0[1] = vertex1;
			tri0[2] = vertex2;
		}
	}
}

struct NavDataProcess::RoadParams
{
	WayTypes::WayTypes wayType;
	uint64_t wayId;
	std::vector<Tag> wayTags;
	bool area;
	RoadTypes::RoadTypes roadType;
	double width;
	bool isIntersection;
	bool isRoundabout;
};

void NavDataProcess::recurseClipRoad(
  const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2,
  const glm::uvec2& minTileIndex, const glm::uvec2& maxTileIndex, const RoadParams& roadParams,
  bool isPlaneVertical
)
{
	// return if the triangle is degenerate
	if (
	  ((glm::abs(vertex0.coords.x - vertex1.coords.x) < epsilon) && (glm::abs(vertex0.coords.y - vertex1.coords.y) < epsilon)) ||
	  ((glm::abs(vertex0.coords.x - vertex2.coords.x) < epsilon) && (glm::abs(vertex0.coords.y - vertex2.coords.y) < epsilon)) ||
	  ((glm::abs(vertex1.coords.x - vertex2.coords.x) < epsilon) && (glm::abs(vertex1.coords.y - vertex2.coords.y) < epsilon))
	)
	{
		return;
	}

	debug_assertion((glm::abs(vertex0.coords.x - vertex1.coords.x) > epsilon) ||
	                (glm::abs(vertex0.coords.y - vertex1.coords.y) > epsilon), "");

	debug_assertion((glm::abs(vertex0.coords.x - vertex2.coords.x) > epsilon) ||
	                (glm::abs(vertex0.coords.y - vertex2.coords.y) > epsilon), "");

	debug_assertion((glm::abs(vertex1.coords.x - vertex2.coords.x) > epsilon) ||
	                (glm::abs(vertex1.coords.y - vertex2.coords.y) > epsilon), "");

	Vertex frontVertex[6];
	Vertex backVertex[6];
	uint32_t numFrontTriangles;
	uint32_t numBackTriangles;
	glm::uvec2 planeIdCoords((minTileIndex.x + maxTileIndex.x) / 2, (minTileIndex.y + maxTileIndex.y) / 2);//the id of the tile whose top-right is a point of the plane
	glm::vec2 planeOrigin = _osm.tiles[planeIdCoords.x][planeIdCoords.y].max;

	clipAgainst(vertex0, vertex1, vertex2, planeOrigin, isPlaneVertical ? glm::vec2(-1.0f, 0.0f) : glm::vec2(0.0f, -1.0f),
	            frontVertex, backVertex, numFrontTriangles, numBackTriangles);

	if (numFrontTriangles > 0) // CLIPS THE FIRST FRONT TRIANGLE IF IT EXISTS
		// At least one part of the resulting triangle is to the "front" of the plane, so:
		// Either the whole triangle was in front (so was not clipped), OR it was clipped
		// (either to a triangle or a quad). If this is not entered, the whole triangle is "back"
	{
		debug_assert_vertices_less_than(frontVertex[0], frontVertex[1], frontVertex[2], planeOrigin, isPlaneVertical);

		auto maxCoords = planeIdCoords;
		if (isPlaneVertical)
		{
			maxCoords.y = maxTileIndex.y;
		}
		else
		{
			maxCoords.x = maxTileIndex.x;
		}
		clipRoad(frontVertex[0], frontVertex[1], frontVertex[2],
		         minTileIndex,
		         maxCoords,
		         roadParams);
	}
	if (numFrontTriangles > 1) // CLIPS THE SECOND FRONT TRIANGLE IF IT EXISTS
		// The triangle was clipped, and the "quad" part of it was in front (so two triangles are in front)
		// If this is false, triangle was either not clipped, or it was clipped and the "quad" part
		// is in the "back"
	{
		debug_assert_vertices_less_than(frontVertex[3], frontVertex[4], frontVertex[5], planeOrigin, isPlaneVertical);
		auto maxCoords = planeIdCoords;
		if (isPlaneVertical)
		{
			maxCoords.y = maxTileIndex.y;
		}
		else
		{
			maxCoords.x = maxTileIndex.x;
		}

		clipRoad(frontVertex[3], frontVertex[4], frontVertex[5],
		         minTileIndex,
		         maxCoords,
		         roadParams);
	}
	if (numBackTriangles > 0)  // CLIPS THE FIRST BACK TRIANGLE IF IT EXISTS
		// Reverse of the 1st comment: whole triangle back, or was clipped. If false, whole tri front.
	{
		debug_assert_vertices_greater_than(backVertex[0], backVertex[1], backVertex[2], planeOrigin, isPlaneVertical);
		auto minCoords = planeIdCoords;
		if (isPlaneVertical)
		{
			minCoords.x += 1;
			minCoords.y = minTileIndex.y;
		}
		else
		{
			minCoords.y += 1;
			minCoords.x = minTileIndex.x;
		}

		clipRoad(backVertex[0], backVertex[1], backVertex[2],
		         minCoords,
		         maxTileIndex,
		         roadParams);
	}
	// Reverse of the 2st comment: Clipped, and quad was "back". If false, not clipped or tri back.
	if (numBackTriangles > 1)  // CLIPS THE SECOND BACK TRIANGLE IF IT EXISTS
	{
		debug_assert_vertices_greater_than(backVertex[3], backVertex[4], backVertex[5], planeOrigin, isPlaneVertical);
		auto minCoords = planeIdCoords;
		if (isPlaneVertical)
		{
			minCoords.x += 1;
			minCoords.y = minTileIndex.y;
		}
		else
		{
			minCoords.y += 1;
			minCoords.x = minTileIndex.x;
		}
		clipRoad(backVertex[3], backVertex[4], backVertex[5],
		         minCoords,
		         maxTileIndex, roadParams);
	}
}


void NavDataProcess::clipRoad(
  const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2,
  const glm::uvec2& minTileIndex, const glm::uvec2& maxTileIndex, const RoadParams& roadParams)
{
	if (
	  ((glm::abs(vertex0.coords.x - vertex1.coords.x) < epsilon) && (glm::abs(vertex0.coords.y - vertex1.coords.y) < epsilon)) ||
	  ((glm::abs(vertex0.coords.x - vertex2.coords.x) < epsilon) && (glm::abs(vertex0.coords.y - vertex2.coords.y) < epsilon)) ||
	  ((glm::abs(vertex1.coords.x - vertex2.coords.x) < epsilon) && (glm::abs(vertex1.coords.y - vertex2.coords.y) < epsilon))
	)
	{
		return;
	}
	// SELECT WHICH PLANE TO CLIP AGAINST
	if (minTileIndex.x == maxTileIndex.x) //If those are equal, we are working with a column.
	{
		if (minTileIndex.y == maxTileIndex.y)// We are in a single tile, so by definition there must be no more clipping : the triangle is completly inside a tile.
		{
			const auto& min = _osm.getTile(minTileIndex).min;
			const auto& max = _osm.getTile(maxTileIndex).max;
			assertion(
			  vertex0.coords.x < max.x + epsilon &&
			  vertex0.coords.x > min.x - epsilon &&
			  vertex0.coords.y < max.y + epsilon &&
			  vertex0.coords.y > min.y - epsilon &&
			  vertex1.coords.x < max.x + epsilon &&
			  vertex1.coords.x > min.x - epsilon &&
			  vertex1.coords.y < max.y + epsilon &&
			  vertex1.coords.y > min.y - epsilon &&
			  vertex2.coords.x < max.x + epsilon &&
			  vertex2.coords.x > min.x - epsilon &&
			  vertex2.coords.y < max.y + epsilon &&
			  vertex2.coords.y > min.y - epsilon, "vertices found outside tile boundaries");

			// add the triangle into the tile
			Way newWay;

			uint64_t nodeId = (_osm.nodes.rbegin()->first + 1);
			Tile& tile = _osm.tiles[minTileIndex.x][minTileIndex.y];
			{
				auto& tmp = tile.nodes[nodeId] = vertex0;
				tmp.id = nodeId;
				newWay.nodeIds.push_back(nodeId);
				_osm.nodes[nodeId++] = vertex0;
			}
			{
				auto& tmp = tile.nodes[nodeId] = vertex1;
				tmp.id = nodeId;
				newWay.nodeIds.push_back(nodeId);
				_osm.nodes[nodeId++] = vertex1;
			}
			{
				auto& tmp = tile.nodes[nodeId] = vertex2;
				tmp.id = nodeId;
				newWay.nodeIds.push_back(nodeId);
				_osm.nodes[nodeId++] = vertex2;
			}

			newWay.id = roadParams.wayId;
			newWay.tags = roadParams.wayTags;
			newWay.roadType = roadParams.roadType;
			newWay.area = roadParams.area;
			newWay.width = roadParams.width;
			newWay.isIntersection = roadParams.isIntersection;
			newWay.isRoundabout = roadParams.isRoundabout;
			// Add new node ID to way
			insert(minTileIndex, roadParams.wayType, &newWay, nodeId);
		}
		else //tileMin.y != tileMax.y : clip a single tile, and the rest of the row, from the column
		{
			recurseClipRoad(vertex0, vertex1, vertex2, minTileIndex, maxTileIndex, roadParams, false);
		}
	}
	else //tileMin.x != tileMax.x : Clip a column, and the rest of the field, from the grid
	{
		recurseClipRoad(vertex0, vertex1, vertex2, minTileIndex, maxTileIndex, roadParams, true);
	}
}

void NavDataProcess::clipRoad(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2, uint64_t wayId,
                              const std::vector<Tag>& wayTags, WayTypes::WayTypes wayType, bool area, RoadTypes::RoadTypes roadType,
                              double roadWidth, bool isIntersection, bool isRoundabout)
{
	if (
	  ((glm::abs(vertex0.coords.x - vertex1.coords.x) < epsilon) && (glm::abs(vertex0.coords.y - vertex1.coords.y) < epsilon)) ||
	  ((glm::abs(vertex0.coords.x - vertex2.coords.x) < epsilon) && (glm::abs(vertex0.coords.y - vertex2.coords.y) < epsilon)) ||
	  ((glm::abs(vertex1.coords.x - vertex2.coords.x) < epsilon) && (glm::abs(vertex1.coords.y - vertex2.coords.y) < epsilon))
	)
	{
		return;
	}
	const glm::ivec2& tile0 = findTile(vertex0.coords);
	const glm::ivec2& tile1 = findTile(vertex1.coords);
	const glm::ivec2& tile2 = findTile(vertex2.coords);

	glm::uvec2 minTileIndex = glm::max(glm::min(tile0, glm::min(tile1, tile2)), glm::ivec2(0, 0));
	glm::uvec2 maxTileIndex = glm::min(glm::max(tile0, glm::max(tile1, tile2)), glm::ivec2(_osm.numCols - 1, _osm.numRows - 1));



	RoadParams rp;
	rp.wayType = wayType;
	rp.wayId = wayId;
	rp.wayTags = wayTags;
	rp.area = area;
	rp.roadType = roadType;
	rp.width = roadWidth;
	rp.isIntersection = isIntersection;
	rp.isRoundabout = isRoundabout;

	if (vertex0.coords.x < (_osm.bounds.min.x - epsilon) || vertex1.coords.x < (_osm.bounds.min.x - epsilon) || vertex2.coords.x < (_osm.bounds.min.x - epsilon))
	{
		Vertex frontVertex[6];
		Vertex backVertex[6];
		uint32_t numFrontTriangles;
		uint32_t numBackTriangles;
		clipAgainst(vertex0, vertex1, vertex2, _osm.bounds.min, glm::vec2(1.0f, 0.0f), frontVertex, backVertex, numFrontTriangles, numBackTriangles);

		//Careful of the tile bounds: by using the same x in min and max, we are slicing off a column...
		if (numFrontTriangles > 0)
		{
			clipRoad(frontVertex[0], frontVertex[1], frontVertex[2], wayId, wayTags, wayType, area, roadType, roadWidth, isIntersection, isRoundabout);
		}
		if (numFrontTriangles > 1)
		{
			clipRoad(frontVertex[3], frontVertex[4], frontVertex[5], wayId, wayTags, wayType, area, roadType, roadWidth, isIntersection, isRoundabout);
		}
	}
	else if (vertex0.coords.x > (_osm.bounds.max.x + epsilon) || vertex1.coords.x > (_osm.bounds.max.x + epsilon) || vertex2.coords.x > (_osm.bounds.max.x + epsilon))
	{
		Vertex frontVertex[6];
		Vertex backVertex[6];
		uint32_t numFrontTriangles;
		uint32_t numBackTriangles;
		clipAgainst(vertex0, vertex1, vertex2, _osm.bounds.max, glm::vec2(-1.0f, 0.0f), frontVertex, backVertex, numFrontTriangles, numBackTriangles);

		//Careful of the tile bounds: by using the same x in min and max, we are slicing off a column...
		if (numFrontTriangles > 0)
		{
			clipRoad(frontVertex[0], frontVertex[1], frontVertex[2], wayId, wayTags, wayType, area, roadType, roadWidth, isIntersection, isRoundabout);
		}
		if (numFrontTriangles > 1)
		{
			clipRoad(frontVertex[3], frontVertex[4], frontVertex[5], wayId, wayTags, wayType, area, roadType, roadWidth, isIntersection, isRoundabout);
		}
	}
	else if (vertex0.coords.y < (_osm.bounds.min.y - epsilon) || vertex1.coords.y < (_osm.bounds.min.y - epsilon) || vertex2.coords.y < (_osm.bounds.min.y - epsilon))
	{
		Vertex frontVertex[6];
		Vertex backVertex[6];
		uint32_t numFrontTriangles;
		uint32_t numBackTriangles;
		clipAgainst(vertex0, vertex1, vertex2, _osm.bounds.min, glm::vec2(0.0f, 1.0f), frontVertex, backVertex, numFrontTriangles, numBackTriangles);

		//Careful of the tile bounds: by using the same x in min and max, we are slicing off a column...
		if (numFrontTriangles > 0)
		{
			clipRoad(frontVertex[0], frontVertex[1], frontVertex[2], wayId, wayTags, wayType, area, roadType, roadWidth, isIntersection, isRoundabout);
		}
		if (numFrontTriangles > 1)
		{
			clipRoad(frontVertex[3], frontVertex[4], frontVertex[5], wayId, wayTags, wayType, area, roadType, roadWidth, isIntersection, isRoundabout);
		}
	}
	else if (vertex0.coords.y > (_osm.bounds.max.y + epsilon) || vertex1.coords.y > (_osm.bounds.max.y + epsilon) || vertex2.coords.y > (_osm.bounds.max.y + epsilon))
	{
		Vertex frontVertex[6];
		Vertex backVertex[6];
		uint32_t numFrontTriangles;
		uint32_t numBackTriangles;
		clipAgainst(vertex0, vertex1, vertex2, _osm.bounds.max, glm::vec2(0.0f, -1.0f), frontVertex, backVertex, numFrontTriangles, numBackTriangles);

		//Careful of the tile bounds: by using the same x in min and max, we are slicing off a column...
		if (numFrontTriangles > 0)
		{
			clipRoad(frontVertex[0], frontVertex[1], frontVertex[2], wayId, wayTags, wayType, area, roadType, roadWidth, isIntersection, isRoundabout);
		}
		if (numFrontTriangles > 1)
		{
			clipRoad(frontVertex[3], frontVertex[4], frontVertex[5], wayId, wayTags, wayType, area, roadType, roadWidth, isIntersection, isRoundabout);
		}
	}
	else
	{
		clipRoad(vertex0, vertex1, vertex2, minTileIndex, maxTileIndex, rp);
	}
}

/*!*********************************************************************************************************************
\return Vector2 with a newly generated point.
\param  Vector2 p1 first point of triangle.
\param  Vector2 p2 second point of triangle.
\param  Vector2 p3 third point of triangle.
\brief  Find the center point of the triangle based in its height / 2.
***********************************************************************************************************************/
glm::dvec2 NavDataProcess::calculateMidPoint(glm::dvec2 p1, glm::dvec2 p2, glm::dvec2 p3) const
{
	//Get center of line.
	glm::dvec2 point = glm::dvec2((p1.x + p2.x) / 2.0, (p1.y + p2.y) / 2.0);
	//Vector from center of line to third point in triangle.
	glm::dvec2 v1 = p3 - point;
	//Calculate vector length, for normalisation and projection.
	double len = glm::length(v1);
	v1 /= len;
	//Project the new point along v1 by the projection length.
	return point += (v1 * (len / 2.0));
}

/*!*********************************************************************************************************************
\return Return true if the point is out of bounds.
\brief  Find if the point is out of the map bounds.
***********************************************************************************************************************/
bool NavDataProcess::isOutOfBounds(const glm::dvec2& point) const
{
	return ((point.x < _osm.bounds.min.x) || (point.y < _osm.bounds.min.y) || (point.x > _osm.bounds.max.x) || (point.y > _osm.bounds.max.y));
}

bool NavDataProcess::isTooCloseToBoundary(const glm::dvec2& point) const
{
	return ((point.x - boundaryBufferX < _osm.bounds.min.x) || (point.y - boundaryBufferY < _osm.bounds.min.y) ||
	        (point.x  + boundaryBufferX > _osm.bounds.max.x) || (point.y + boundaryBufferY > _osm.bounds.max.y));
}

/*!*********************************************************************************************************************
\return Return a vector with the tile coordinates.
\param  point Find the tile that this point belongs to.
\brief  Find the tile the given point belongs to.
***********************************************************************************************************************/
glm::ivec2 NavDataProcess::findTile2(glm::dvec2& point) const
{
	glm::uvec2 tileCoords;

	for (uint32_t i = 0; i < _osm.numCols; ++i)
	{
		if (point.x <= _osm.tiles[i][0].max.x)
		{
			if ((point.x == _osm.tiles[i][0].max.x) && (i != (_osm.numCols - 1)))
			{
				point.x -= 0.0000001;
			} // Move node off of tile border
			tileCoords.x = i;
			break;
		}
	}

	for (uint32_t i = 0; i < _osm.numRows; ++i)
	{
		if (point.y <= _osm.tiles[0][i].max.y)
		{
			if ((point.y == _osm.tiles[0][i].max.y) && (i != (_osm.numRows - 1)))
			{
				point.y -= 0.0000001;
			} // Move node off of tile border
			tileCoords.y = i;
			break;
		}
	}
	return tileCoords;
}


/*!*********************************************************************************************************************
\return Return a vector with the tile coordinates.
\param  point Find the tile that this point belongs to.
\brief  Find the tile the given point belongs to.
***********************************************************************************************************************/
glm::ivec2 NavDataProcess::findTile(const glm::dvec2& point) const
{
	glm::dvec2 tileSize = _osm.tiles[0][0].max - _osm.tiles[0][0].min;
	glm::dvec2 tileRatio = point / tileSize;
	glm::dvec2 tileFloorRatio(glm::floor(tileRatio));
	glm::ivec2 retval(tileFloorRatio);
	if (tileRatio.x == tileFloorRatio.x) { retval.x -= 1; } //If on tile boundary, return left/bottom tile
	if (tileRatio.y == tileFloorRatio.y) { retval.y -= 1; }
	return retval;
}

/*!*********************************************************************************************************************
\return Vector with point of intersection and side of tile that intersection occurs.
\param  minBounds Minimum x and y of rectangle.
\param  maxBounds Maximum x and y of rectangle.
\param  inPoint   The point inside the rectangle.
\param  outPoint  The point outside the rectangle.
\brief  Finds the point of intersection of: the line between two points, with a rectangle. Can only intersect once.
inPoint is inside the rectangle, outPoint is outside. Also gives the side of intersection.
***********************************************************************************************************************/
glm::dvec3 NavDataProcess::findIntersect(const glm::dvec2 minBounds, const glm::dvec2 maxBounds,
    const glm::dvec2 inPoint, const glm::dvec2 outPoint) const
{
	double m = (inPoint.y - outPoint.y) / (inPoint.x - outPoint.x);
	double c = inPoint.y - m * inPoint.x;

	if (outPoint.x < minBounds.x) // Check if intersect is with left side
	{
		double y = m * minBounds.x + c;

		if ((y >= minBounds.y) && (y <= maxBounds.y))
		{
			return glm::dvec3(minBounds.x, y, Sides::Left);
		}
	}

	if (outPoint.y > maxBounds.y) // Check if intersect is with top side
	{
		if (outPoint.x == inPoint.x)
		{
			return glm::dvec3(outPoint.x, maxBounds.y, Sides::Top);
		}

		double x = (maxBounds.y - c) / m;

		if ((x >= minBounds.x) && (x <= maxBounds.x))
		{
			return glm::dvec3(x, maxBounds.y, Sides::Top);
		}
	}

	if (outPoint.x > maxBounds.x) // Check if intersect is with right sight
	{
		double y = m * maxBounds.x + c;

		if ((y >= minBounds.y) && (y <= maxBounds.y))
		{
			return glm::dvec3(maxBounds.x, y, Sides::Right);
		}
	}

	if (outPoint.y < minBounds.y) // Check if intersect is with bottom side
	{
		if (outPoint.x == inPoint.x)
		{
			return glm::dvec3(outPoint.x, minBounds.y, Sides::Bottom);
		}

		double x = (minBounds.y - c) / m;

		if ((x >= minBounds.x) && (x <= maxBounds.x))
		{
			return glm::dvec3(x, minBounds.y, Sides::Bottom);
		}
	}

	Log(LogLevel::Error, "Could not find intersect point, empty vector returned");

	return glm::dvec3(0, 0, Sides::NoSide);
}

/*!*********************************************************************************************************************
\param  point1  Out of map bounds point.
\param  point2  Out of map bounds point.
\brief  Finds intersections with map bounds (if there are any) for a way that crosses a map without a node within bounds.
***********************************************************************************************************************/
bool NavDataProcess::findMapIntersect(glm::dvec2& point1, glm::dvec2& point2) const
{
	glm::dvec2 newPoint1;
	glm::dvec2 newPoint2;
	double m = (point1.y - point2.y) / (point1.x - point2.x);
	double c = point1.y - m * point1.x;
	double minX = glm::min(point1.x, point2.x);
	double maxX = glm::max(point1.x, point2.x);
	double minY = glm::min(point1.y, point2.y);
	double maxY = glm::max(point1.y, point2.y);
	bool mapIntersect = false;

	// Check if there is an intersection on the left side
	double y = m * _osm.bounds.min.x + c;
	if ((y >= _osm.bounds.min.y) && (y <= _osm.bounds.max.y) && (y > minY) && (y < maxY))
	{
		newPoint1 = glm::dvec2(_osm.bounds.min.x, y);
		mapIntersect = true;
	}

	// Check if there is an intersection on the top side
	double x = (_osm.bounds.max.y - c) / m;
	if ((x >= _osm.bounds.min.x) && (x <= _osm.bounds.max.x) && (x > minX) && (x < maxX))
	{
		newPoint2 = glm::dvec2(x, _osm.bounds.max.y);
		mapIntersect = true;
	}

	// Check if there is an intersection on the right side
	y = m * _osm.bounds.max.x + c;
	if ((y >= _osm.bounds.min.y) && (y <= _osm.bounds.max.y) && (y > minY) && (y < maxY))
	{
		newPoint1 = glm::dvec2(_osm.bounds.max.x, y);
		mapIntersect = true;
	}

	// Check if there is an intersection on the bottom side
	x = (_osm.bounds.min.y - c) / m;
	if ((x >= _osm.bounds.min.x) && (x <= _osm.bounds.max.x) && (x > minX) && (x < maxX))
	{
		newPoint2 = glm::dvec2(x, _osm.bounds.min.y);
		mapIntersect = true;
	}

	// If there is a map intersection, update the coordinates of the points
	if (mapIntersect)
	{
		glm::dvec2 vec1 = newPoint1 - point1;
		glm::dvec2 vec2 = newPoint2 - point1;

		point1 = (glm::length(vec1) < glm::length(vec2)) ? newPoint1 : newPoint2;
		point2 = (point1 == newPoint1) ? newPoint2 : newPoint1;
	}
	return mapIntersect;
}

/*!*********************************************************************************************************************
\return double The calculated area.
\param  vector An array of points (vec2s) that make up the polygon.
\brief  Calculates the area of a triangle from a series of given points.
***********************************************************************************************************************/
double NavDataProcess::calculateTriangleArea(const std::vector<glm::dvec2>& points) const
{
	double area = 0.0;

	for (uint32_t i = 0; i < points.size(); ++i)
	{
		glm::dvec2 currentPoint = points[i];
		glm::dvec2 nextPoint = (i == (points.size() - 1)) ? points[0] : points[i + 1]; // Start and end node of a closed way are the same

		area += ((nextPoint.x - currentPoint.x) * (nextPoint.y + currentPoint.y));
	}
	return area / 2.0;
}

/*!*********************************************************************************************************************
\return pvr::PolygonWindingOrder FrontFaceCW (clockwise) or FrontFaceCCW (anti-clockwise).
\param  nodeIds The node IDs of the closed way to check.
\brief  Checks the winding order of a closed way.
***********************************************************************************************************************/
pvr::PolygonWindingOrder NavDataProcess::checkWinding(const std::vector<uint64_t>& nodeIds) const
{
	std::vector<glm::dvec2> points;

	for (uint32_t i = 0; i < nodeIds.size(); ++i)
	{
		points.push_back(_osm.getNodeById(nodeIds[i]).coords);
	}

	double area = calculateTriangleArea(points);

	if (area <= 0.0)
	{
		return pvr::PolygonWindingOrder::FrontFaceCCW;
	}
	else
	{
		return pvr::PolygonWindingOrder::FrontFaceCW;
	}
}

/*!*********************************************************************************************************************
\return pvr::PolygonWindingOrder FrontFaceCW (clockwise) or FrontFaceCCW (anti-clockwise).
\param  points  The series of points to check.
\brief  Checks the winding order of a a series of points.
***********************************************************************************************************************/
pvr::PolygonWindingOrder NavDataProcess::checkWinding(const std::vector<glm::dvec2>& points) const
{
	double area = calculateTriangleArea(points);

	if (area <= 0.0)
	{
		return pvr::PolygonWindingOrder::FrontFaceCCW;
	}
	else
	{
		return pvr::PolygonWindingOrder::FrontFaceCW;
	}
}

/*!*********************************************************************************************************************
\return Return array of 2 new points. Left points (with respect to way direction) is given first.
\param  firstPoint  First point to use.
\param  secPoint  Second point to use.
\param  width   Desired width between the new nodes.
\param  pointNum  Point to return values for (1, 2).
\brief  Provides two points on the perpendicular line at distance "width" apart.
***********************************************************************************************************************/
std::array<glm::dvec2, 2> NavDataProcess::findPerpendicularPoints(const glm::dvec2 firstPoint, const glm::dvec2 secPoint, const double width, const int pointNum) const
{
	std::array<glm::dvec2, 2> points;
	std::array<glm::dvec2, 2> first;
	std::array<glm::dvec2, 2> sec;

	if (glm::abs(firstPoint.y - secPoint.y) <= epsilon) // To avoid division by zero
	{
		points[0] = (pointNum == 1) ? glm::dvec2(firstPoint.x, firstPoint.y + width / 2.0) : glm::dvec2(secPoint.x, secPoint.y + width / 2.0);
		points[1] = (pointNum == 1) ? glm::dvec2(firstPoint.x, firstPoint.y - width / 2.0) : glm::dvec2(secPoint.x, secPoint.y - width / 2.0);
	}
	else // All other cases give a valid gradient
	{
		double m = -(secPoint.x - firstPoint.x) / (secPoint.y - firstPoint.y);
		double c = (pointNum == 1) ? firstPoint.y - m * firstPoint.x : secPoint.y - m * secPoint.x;

		// You can use the new gradient and constant to find intersections with a circle with r = width / 2
		points = (pointNum == 1) ? circleIntersects(firstPoint, width / 2.0, m, c) : circleIntersects(secPoint, width / 2.0, m, c);
	}

	// Switch points around if necessary so we know element 0 holds the point that is to the left of the way
	if ((glm::atan(secPoint.y - firstPoint.y, secPoint.x - firstPoint.x) - glm::atan(points[0].y - firstPoint.y, points[0].x - firstPoint.x)) > 0)
	{
		std::reverse(points.begin(), points.end());
	}

	return points;
}

/*!*********************************************************************************************************************
\return Return array of 2 new points. Left points (with respect to way direction) is given first.
\param  firstPoint  First point to use.
\param  secPoint  Second point to use.
\param  thirdPoint  Third point to use.
\param  width   Desired width between the new nodes.
\brief  Provides two points on the perpendicular line at distance "width" apart for the middle point.
***********************************************************************************************************************/
std::array<glm::dvec2, 2> NavDataProcess::findPerpendicularPoints(const glm::dvec2 firstPoint, const glm::dvec2 secPoint, const glm::dvec2 thirdPoint, const double width) const
{
	std::array<glm::dvec2, 2> points;
	std::array<glm::dvec2, 2> first = findPerpendicularPoints(firstPoint, secPoint, width, 1);
	std::array<glm::dvec2, 2> sec1 = findPerpendicularPoints(firstPoint, secPoint, width, 2); // New points using line between first two given points
	std::array<glm::dvec2, 2> sec2 = findPerpendicularPoints(secPoint, thirdPoint, width, 1); // New points using line between second two given points
	std::array<glm::dvec2, 2> third = findPerpendicularPoints(secPoint, thirdPoint, width, 2);

	if (isVectorEqual(sec1[0], sec2[0]) && isVectorEqual(sec1[1], sec2[1])) // If the line section has no bend sec1 and sec2 will be equal
	{
		points = sec1;
	}
	else // Most of the time they will not be equal
	{
		bool p0 = rayIntersect(first[0], sec1[0] - first[0], third[0], sec2[0] - third[0], points[0]);
		bool p1 = rayIntersect(first[1], sec1[1] - first[1], third[1], sec2[1] - third[1], points[1]);
	}
	return points;
}

/*!*********************************************************************************************************************
\return Return the 2 points that intersect with the circle.
\param  centre    Centre of circle.
\param  r     Radius of circle.
\param  m     Gradient of line.
\param  constant  Constant of line.
\brief  Find the 2 intersections of a line with a circle (assumes they intersect).
***********************************************************************************************************************/
std::array<glm::dvec2, 2> NavDataProcess::circleIntersects(const glm::vec2 centre, const double r, const double m, const double constant) const
{
	double a = glm::pow(m, 2.0) + 1.0;
	double b = 2.0 * m * (constant - centre.y) - 2.0 * centre.x;
	double c = pow(centre.x, 2.0) + pow(constant - centre.y, 2.0) - pow(r, 2.0);

	double x1 = (-b + sqrt(pow(b, 2.0) - 4.0 * a * c)) / (2.0 * a);
	double x2 = (-b - sqrt(pow(b, 2.0) - 4.0 * a * c)) / (2.0 * a);

	return std::array<glm::dvec2, 2> { glm::dvec2(x1, m * x1 + constant), glm::dvec2(x2, m * x2 + constant) };
}

/*!*********************************************************************************************************************
\brief  Clears data no longer needed from the osm object.
***********************************************************************************************************************/
void OSM::cleanData()
{
	nodes.clear();
	originalRoadWays.clear();
	parkingWays.clear();
	buildWays.clear();
	convertedRoads.clear();
	original_intersections.clear();
	triangulatedRoads.clear();
	for (uint32_t lod = 0; lod < LOD::Count; ++lod)
	{
		labels[lod].clear();
		amenityLabels[lod].clear();
		icons[lod].clear();
	}
	uniqueIconNames.clear();
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Release all data held by tiles.
***********************************************************************************************************************/
void NavDataProcess::releaseTileData()
{
	//Clean up tile data.
	for (auto& tileCol : _osm.tiles)
	{
		for (auto& tile : tileCol)
		{
			tile.areaWays.clear();
			tile.buildWays.clear();
			tile.indices.clear();
			tile.innerWays.clear();
			tile.nodes.clear();
			tile.parkingWays.clear();
			tile.roadWays.clear();
			tile.vertices.clear();
			for (uint32_t lod = 0; lod < LOD::Count; ++lod)
			{
				tile.labels[lod].clear();
				tile.amenityLabels[lod].clear();
				tile.icons[lod].clear();
			}
		}
	}
}
