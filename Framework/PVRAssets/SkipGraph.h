/*!*********************************************************************************************************************
\file         PVRAssets/SkipGraph.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Implementation of a skip grah
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"

namespace pvr {
/*!***************************************************************************
 \brief      		Stores a pointer to the node's data and also uses a dynamic
					array to store pointer to nodes this node depends on and
					another to store pointers to nodes that are dependant on this node.
*****************************************************************************/
template<class T>
class SkipGraphNode
{
private:
	T m_pData;
	std::vector<SkipGraphNode*> m_apDependencies; // What I depend on
	std::vector<SkipGraphNode*> m_apDependents; // What depends on me

public:
	/*!***************************************************************************
	\brief      	Constructor.
	*****************************************************************************/
	SkipGraphNode() { }

	/*!***************************************************************************
	\brief      	Overloaded constructor.
	\param[in]		data    Pointer to a node
	*****************************************************************************/
	SkipGraphNode(const T& data) : m_pData(data) { }

	/*!***************************************************************************
	\brief      	Destructor.
	*****************************************************************************/
	~SkipGraphNode() { }

	/*!***************************************************************************
	\return			Return number of dependencies
	\brief      	Returns the number of dependencies referenced by this node.
	*****************************************************************************/
	uint32 getNumDependencies() const { return (uint32)m_apDependencies.size(); }

	/*!***************************************************************************
	\param[in]		id dependency id
	\return			Return a reference to dependency
	\brief      	Get given dependency.
	*****************************************************************************/
	SkipGraphNode& getDependency(uint32 id) const
	{
		assertion(id < (uint32)m_apDependencies.size(), "SkipGraph::getDependency id out of range");
		return *m_apDependencies[id];
	}


	/*!***************************************************************************
	\param[out]		pDependentNode dependency to add
	\return			Return true if dependency is added
	\brief      	Adds a dependency to this node.
	*****************************************************************************/
	bool addDependency(SkipGraphNode* pDependentNode)
	{
		uint32 ui(0);

		if (pDependentNode == this)
		{
			return false;
		}

		if (!pDependentNode)
		{
			return false;
		}

		//Check the dependency doesn't already exist
		for (ui = 0; ui < (uint32)m_apDependencies.size(); ++ui)
		{
			if (m_apDependencies[ui] == pDependentNode)
			{
				return true;
			}
		}

		//Add the dependency and also set this node as a dependent of the referenced node
		m_apDependencies.push_back(pDependentNode);
		pDependentNode->addDependent(this);

		return true;
	}

	/*!***************************************************************************
	\return			Return a reference to the data
	\brief      	Get the data associated with this node.
	*****************************************************************************/
	T& getData() { return m_pData; }

private:
	/*!***************************************************************************
	\param[out]		pDependancyNode dependent to this node
	\return			Return true if dependent added
	\brief      	Adds a dependent to this node.
	*****************************************************************************/
	bool addDependent(SkipGraphNode* pDependencyNode)
	{
		uint32 ui(0);
		if (!pDependencyNode) {	return false;	}

		//	Check the dependency doesn't already exist.
		for (ui = 0; ui < (uint32)m_apDependents.size(); ++ui)
		{
			if (m_apDependencies[ui] == pDependencyNode) {	return true;	}
		}

		//	Add the dependancy
		m_apDependents.push_back(pDependencyNode);
		return true;
	}
};

/*!***************************************************************************
 \brief     This class is the entry point for creating and accessing
			the elements of a skip graph. It uses a hash table to store
			the nodes of the structure and a hash value that allows
			fast searching of the skip graph.
*****************************************************************************/
template<class T>
class SkipGraphRoot
{
//-------------------------------------------------------------------------//
private:

	/*!***************************************************************************
	 \brief      	A struct to store data and a hash value generated from the
					data. The hash value allows faster searching of the skip graph.
	*****************************************************************************/
	struct HashElement
	{
	public:
		/*!***************************************************************************
		\param[in]			hash
		\param[in]			data
		\brief      	Overloaded constructor.
		*****************************************************************************/
		HashElement(size_t hash, const T& data) : m_hash(hash), m_skipGraphNode(data) {}

		/*!***************************************************************************
		\brief      	Constructor.
		*****************************************************************************/
		HashElement() {}

		/*!***************************************************************************
		\return			Return the hash
		\brief      	Get the element's hash value.
		*****************************************************************************/
		size_t getHash() const {	return m_hash;	}

		/*!***************************************************************************
		\return			Return the associated node
		\brief      	Get the node associated with this element.
		*****************************************************************************/
		SkipGraphNode<T>& getNode()	{	return m_skipGraphNode; 	}

		/*!***************************************************************************
		\return			Return a reference to the associated node
		\brief      	Get the node associated with this element.
		*****************************************************************************/
		const SkipGraphNode<T>& getNode() const {	return m_skipGraphNode; 	}

	private:
		size_t			 m_hash;
		SkipGraphNode<T> m_skipGraphNode;
	};

	std::vector<HashElement>		m_aHashTable;

//-------------------------------------------------------------------------//
public:

	/*!***************************************************************************
	 \param[in]	data The data of the node to be added
	 \return	Return true if the node was found or was created successfully.
	 \brief     Searches through the hash table to see if the added node already
				exists. If it doesn't, it creates a node.
	*****************************************************************************/
	bool addNode(const T& data)
	{
		size_t hashed = hash<string>()(data->toString());

		//	First, search the hash table to see
		//	if the node already exists.
		SkipGraphNode<T>* skipGraphNode(findNode(hashed));
		if (skipGraphNode == NULL)
		{
			//	The node wasn't found, so a new node needs to be
			//	created.
			m_aHashTable.push_back(HashElement(hashed, data));
			ptrdiff_t iArrayElement = m_aHashTable.size() - 1;

			//	Now point to the new instance.
			skipGraphNode = &m_aHashTable[iArrayElement].getNode();
		}
		return skipGraphNode ? true : false;
	}


	/*!***************************************************************************
	\brief      	Adds a node dependency.
	\param[in]		nodeData data of the node to add dependency to
	\param[in]		dependantData dependency data to add
	\return			Return true if the node dependency has been added
	*****************************************************************************/
	bool addNodeDependency(const T& nodeData, const T& dependantData)
	{
		SkipGraphNode<T>* pNode(NULL);
		SkipGraphNode<T>* pDependantNode(NULL);

		pNode = findNode(nodeData);
		if (!pNode) { return false; }

		pDependantNode = findNode(dependantData);
		if (!pDependantNode) {	return false;	}

		//	Nodes are not allowed to self reference
		if (pNode == pDependantNode) {	return false;  }
		pNode->addDependency(pDependantNode);
		return true;
	}

	/*!***************************************************************************
	 \brief     Get the total number of nodes in the skip graph.
	 \return	Return the total number of nodes
	*****************************************************************************/
	uint32 getNumNodes() const {	return (uint32)m_aHashTable.size();	}

	/*!***************************************************************************
	 \brief Returns a sorted list of dependencies for the specified node.
			The list is ordered with the leaf nodes at the front, followed by nodes
			that depend on them and so forth until the root node is reached and added
			at the end of the list.
	 \param[in]	aOutputArray	The dynamic array to store the sorted results in
	 \param[in]	nodeID	The ID of the root node for the dependency search
	*****************************************************************************/
	void RetreiveSortedDependencyList(std::vector<T>& aOutputArray,
	                                  const uint32 nodeID)
	{
		assertion(nodeID < (uint32)m_aHashTable.size(), "SkipGraph::RetreiveSortedDependencyList nodeId out of range");
		recursiveSortedListAdd(aOutputArray, m_aHashTable[nodeID].getNode());
	}

	/*!***************************************************************************
	 \brief Overloads operator[] to returns a handle to the node data for the specified ID.
	 \return Return handle to the node data
	*****************************************************************************/
	T& operator[](const uint32 ui32NodeID) {	return *(getNodeData(ui32NodeID));	}

	/*!***************************************************************************
	 \brief Overloads operator[] to returns a const handle to the node data for the specified ID.
	 \return Return handle to the node data
	*****************************************************************************/
	const T& operator[](const uint32 ui32NodeID) const {	return *(getNodeData(ui32NodeID));  	}

//-------------------------------------------------------------------------//
private:
	/*!***************************************************************************
	 \brief     Recursively adds node dependencies to aOutputArray.
				By doing so, aOutputArray will be ordered from leaf nodes to the root node
				that started the recursive chain.
	 \param[out] aOutputArray	The dynamic array to store the sorted results in
	 \param[in]	currentNode		The current node to process
	*****************************************************************************/
	void recursiveSortedListAdd(std::vector<T>& aOutputArray, SkipGraphNode<T>& currentNode)
	{
		uint32 ui(0);

		//	Recursively add dependancies first
		for (ui = 0; ui < currentNode.getNumDependencies(); ++ui)
		{
			recursiveSortedListAdd(aOutputArray, currentNode.getDependency(ui));
		}

		//	Then add this node to the array
		aOutputArray.push_back(currentNode.getData());
	}

	/*!***************************************************************************
	 \brief  Retrieve a handle to the specified node's data.
	 \param[in] 	nodeID		The node's ID
	 \return Return a handle to node's data, else return NULL if node not found
	*****************************************************************************/
	T* getNodeData(uint32 nodeID)
	{
		assertion(nodeID < (uint32)m_aHashTable.size(), "SkipGraph::getNodeData nodeId out of range");
		return &m_aHashTable[nodeID].getNode().getData();
	}

	/*!***************************************************************************
	 \brief Use the input hash to search the hash table and see if the
			node already exists. If it does, the function returns a pointer
		    to the node. If it doesn't, it returns NULL.
	 \param[in] hash	The hash value to search with
	 \return Return a handle to the found node, else return NULL
	*****************************************************************************/
	SkipGraphNode<T>* findNode(const size_t hash)
	{
		int i(0);
		int i32HashTableSize((int32)m_aHashTable.size());

		//	A NULL hash means the node has not initialized
		//	correctly.
		if (hash == 0) { return NULL; }

		//	NOTE:
		//	In the future, the hash table could be sorted from
		//	lowest hash value to highest so that binary search could
		//	be used to find a given node. It should be possible to
		//	use a bool (or some other mechanism) to toggle this form of search
		//	(if the skip graph is small, it might be faster to just use a brute
		//	force for loop to search through).
		for (i = 0; i < i32HashTableSize; ++i)
		{
			if (m_aHashTable[i].getHash()) {	return &m_aHashTable[i].getNode();	}
		}

		//	The element wasn't found, so return null.
		return NULL;
	}

	/*!***************************************************************************
	 \brief Use the input data to generate a hash and then search the hash table
			and see if the node already exists. If it does, the function returns a pointer.
			to the node. If it doesn't, it returns NULL.
	 \param[in] data Data to use as a source for the hash
	 \return Return a handle to the found node, else return NULL
	*****************************************************************************/
	SkipGraphNode<T>* findNode(const T& data)
	{
		return findNode(hash<string>()(data->toString()));
	}
};
}


