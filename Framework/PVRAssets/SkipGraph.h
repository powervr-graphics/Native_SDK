/*!
\brief Implementation of a skip grah
\file PVRAssets/SkipGraph.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/AssetIncludes.h"

namespace pvr {
/// <summary>Stores a pointer to the node's data and also uses a dynamic array to store pointer to nodes this node
/// depends on and another to store pointers to nodes that are dependant on this node.</summary>
template<class T>
class SkipGraphNode
{
private:
	T _pData;
	std::vector<SkipGraphNode*> _apDependencies; // What I depend on
	std::vector<SkipGraphNode*> _apDependents; // What depends on me

public:
	/// <summary>Constructor.</summary>
	SkipGraphNode() { }

	/// <summary>Overloaded constructor.</summary>
	/// <param name="data">Pointer to a node</param>
	SkipGraphNode(const T& data) : _pData(data) { }

	/// <summary>Destructor.</summary>
	~SkipGraphNode() { }

	/// <summary>Returns the number of dependencies referenced by this node.</summary>
	/// <returns>Return number of dependencies</returns>
	uint32_t getNumDependencies() const { return static_cast<uint32_t>(_apDependencies.size()); }

	/// <summary>Get given dependency.</summary>
	/// <param name="id">dependency id</param>
	/// <returns>Return a reference to dependency</returns>
	SkipGraphNode& getDependency(uint32_t id) const
	{
		assertion(id < static_cast<uint32_t>(_apDependencies.size()), "SkipGraph::getDependency id out of range");
		return *_apDependencies[id];
	}


	/// <summary>Adds a dependency to this node.</summary>
	/// <param name="pDependentNode">dependency to add</param>
	/// <returns>Return true if dependency is added</returns>
	bool addDependency(SkipGraphNode* pDependentNode)
	{
		uint32_t ui(0);

		if (pDependentNode == this)
		{
			return false;
		}

		if (!pDependentNode)
		{
			return false;
		}

		//Check the dependency doesn't already exist
		for (ui = 0; ui < static_cast<uint32_t>(_apDependencies.size()); ++ui)
		{
			if (_apDependencies[ui] == pDependentNode)
			{
				return true;
			}
		}

		//Add the dependency and also set this node as a dependent of the referenced node
		_apDependencies.push_back(pDependentNode);
		pDependentNode->addDependent(this);

		return true;
	}

	/// <summary>Get the data associated with this node.</summary>
	/// <returns>Return a reference to the data</returns>
	T& getData() { return _pData; }

private:
	/// <summary>Adds a dependent to this node.</summary>
	/// <param name="pDependancyNode">dependent to this node</param>
	/// <returns>Return true if dependent added</returns>
	bool addDependent(SkipGraphNode* pDependencyNode)
	{
		uint32_t ui(0);
		if (!pDependencyNode) { return false; }

		//  Check the dependency doesn't already exist.
		for (ui = 0; ui < static_cast<uint32_t>(_apDependents.size()); ++ui)
		{
			if (_apDependencies[ui] == pDependencyNode) { return true;  }
		}

		//  Add the dependancy
		_apDependents.push_back(pDependencyNode);
		return true;
	}
};

/// <summary>This class is the entry point for creating and accessing the elements of a skip graph. It uses a hash
/// table to store the nodes of the structure and a hash value that allows fast searching of the skip graph.
/// </summary>
template<class T>
class SkipGraphRoot
{
//-------------------------------------------------------------------------//
private:

	/// <summary>A struct to store data and a hash value generated from the data. The hash value allows faster
	/// searching of the skip graph.</summary>
	struct HashElement
	{
	public:
		/// <summary>Overloaded constructor. Explicitly sets the Hash value and the Data</summary>
		/// <param name="hash">The hash of the value</param>
		/// <param name="data">The value</param>
		HashElement(size_t hash, const T& data) : _hash(hash), _skipGraphNode(data) {}

		/// <summary>Constructor. Does not initialize values.</summary>
		HashElement() {}

		/// <summary>Get the element's hash value.</summary>
		/// <returns>The hash of the value</returns>
		size_t getHash() const { return _hash; }

		/// <summary>Get the node associated with this element.</summary>
		/// <returns>Return the associated node</returns>
		SkipGraphNode<T>& getNode() { return _skipGraphNode;  }

		/// <summary>Get the node associated with this element.</summary>
		/// <returns>Return a reference to the associated node</returns>
		const SkipGraphNode<T>& getNode() const { return _skipGraphNode;  }

	private:
		size_t       _hash;
		SkipGraphNode<T> _skipGraphNode;
	};

	std::vector<HashElement>    _aHashTable;

//-------------------------------------------------------------------------//
public:

	/// <summary>Searches through the hash table to see if the added node already exists. If it doesn't, it creates a node.
	/// </summary>
	/// <param name="data">The data of the node to be added</param>
	/// <returns>Return true if the node was found or was created successfully.</returns>
	bool addNode(const T& data)
	{
		size_t hashed = hash<std::string>()(data->toString());

		//  First, search the hash table to see
		//  if the node already exists.
		SkipGraphNode<T>* skipGraphNode(findNode(hashed));
		if (skipGraphNode == NULL)
		{
			//  The node wasn't found, so a new node needs to be
			//  created.
			_aHashTable.push_back(HashElement(hashed, data));
			ptrdiff_t iArrayElement = _aHashTable.size() - 1;

			//  Now point to the new instance.
			skipGraphNode = &_aHashTable[iArrayElement].getNode();
		}
		return skipGraphNode ? true : false;
	}


	/// <summary>Adds a node dependency.</summary>
	/// <param name="nodeData">data of the node to add dependency to</param>
	/// <param name="dependantData">dependency data to add</param>
	/// <returns>Return true if the node dependency has been added</returns>
	bool addNodeDependency(const T& nodeData, const T& dependantData)
	{
		SkipGraphNode<T>* pNode(NULL);
		SkipGraphNode<T>* pDependantNode(NULL);

		pNode = findNode(nodeData);
		if (!pNode) { return false; }

		pDependantNode = findNode(dependantData);
		if (!pDependantNode) {  return false; }

		//  Nodes are not allowed to self reference
		if (pNode == pDependantNode) {  return false;  }
		pNode->addDependency(pDependantNode);
		return true;
	}

	/// <summary>Get the total number of nodes in the skip graph.</summary>
	/// <returns>Return the total number of nodes</returns>
	uint32_t getNumNodes() const {  return static_cast<uint32_t>(_aHashTable.size());  }

	/// <summary>Returns a sorted list of dependencies for the specified node. The list is ordered with the leaf nodes at
	/// the front, followed by nodes that depend on them and so forth until the root node is reached and added at the
	/// end of the list.</summary>
	/// <param name="aOutputArray">The dynamic array to store the sorted results in</param>
	/// <param name="nodeID">The ID of the root node for the dependency search</param>
	void RetreiveSortedDependencyList(std::vector<T>& aOutputArray,
	                                  const uint32_t nodeID)
	{
		assertion(nodeID < static_cast<uint32_t>(_aHashTable.size()), "SkipGraph::RetreiveSortedDependencyList nodeId out of range");
		recursiveSortedListAdd(aOutputArray, _aHashTable[nodeID].getNode());
	}

	/// <summary>Overloads operator[] to returns a handle to the node data for the specified ID.</summary>
	/// <param name="nodeId">The index of the node to retrieve</param>
	/// <returns>Return handle to the node data</returns>
	T& operator[](uint32_t nodeId) {  return *(getNodeData(nodeId));  }

	/// <summary>Overloads operator[] to returns a const handle to the node data for the specified ID.</summary>
	/// <param name="nodeId">The index of the node to retrieve</param>
	/// <returns>Return handle to the node data</returns>
	const T& operator[](uint32_t nodeId) const {  return *(getNodeData(nodeId)); }

//-------------------------------------------------------------------------//
private:
	/// <summary>Recursively adds node dependencies to aOutputArray. By doing so, aOutputArray will be ordered from leaf nodes
	/// to the root node that started the recursive chain.</summary>
	/// <param name="aOutputArray">The dynamic array to store the sorted results in</param>
	/// <param name="currentNode">The current node to process</param>
	void recursiveSortedListAdd(std::vector<T>& aOutputArray, SkipGraphNode<T>& currentNode)
	{
		uint32_t ui(0);

		//  Recursively add dependancies first
		for (ui = 0; ui < currentNode.getNumDependencies(); ++ui)
		{
			recursiveSortedListAdd(aOutputArray, currentNode.getDependency(ui));
		}

		//  Then add this node to the array
		aOutputArray.push_back(currentNode.getData());
	}

	/// <summary>Retrieve a handle to the specified node's data.</summary>
	/// <param name="nodeID">The node's ID</param>
	/// <returns>Return a handle to node's data, else return NULL if node not found</returns>
	T* getNodeData(uint32_t nodeID)
	{
		assertion(nodeID < static_cast<uint32_t>(_aHashTable.size()), "SkipGraph::getNodeData nodeId out of range");
		return &_aHashTable[nodeID].getNode().getData();
	}

	/// <summary>Use the input hash to search the hash table and see if the node already exists. If it does, the
	/// function returns a pointer to the node. If it doesn't, it returns NULL.</summary>
	/// <param name="hash">The hash value to search with</param>
	/// <returns>Return a handle to the found node, else return NULL</returns>
	SkipGraphNode<T>* findNode(const size_t hash)
	{
		int i(0);
		int i32HashTableSize(static_cast<int32_t>(_aHashTable.size()));

		//  A NULL hash means the node has not initialized
		//  correctly.
		if (hash == 0) { return NULL; }

		//  NOTE:
		//  In the future, the hash table could be sorted from
		//  lowest hash value to highest so that binary search could
		//  be used to find a given node. It should be possible to
		//  use a bool (or some other mechanism) to toggle this form of search
		//  (if the skip graph is small, it might be faster to just use a brute
		//  force for loop to search through).
		for (i = 0; i < i32HashTableSize; ++i)
		{
			if (_aHashTable[i].getHash()) { return &_aHashTable[i].getNode(); }
		}

		//  The element wasn't found, so return null.
		return NULL;
	}

	/// <summary>Use the input data to generate a hash and then search the hash table and see if the node already
	/// exists. If it does, the function returns a pointer. to the node. If it doesn't, it returns NULL.</summary>
	/// <param name="data">Data to use as a source for the hash</param>
	/// <returns>Return a handle to the found node, else return NULL</returns>
	SkipGraphNode<T>* findNode(const T& data)
	{
		return findNode(hash<std::string>()(data->toString()));
	}
};
}

