#include "AccGraph.hh"
#include <stdexcept>

using namespace std;

/*
 * Find the vertex with the given name in the given part.
 * Return the index of it or `INVALID_VTXIDX` if not found.
 */
vtxidx_t VarGraph::findvertex(const std::string &name, bool part) const
{
	const std::vector<VarVertex>& vertices
		= part ? accvertices_ : funcvertices_;
	vtxidx_t i;
	for (i = 0u; i < vertices.size(); ++i)
		if (vertices[i].name == name)
			return i;
	return INVALID_VTXIDX;
}

/*
 * Add a vertex with the given name in the given part and return its index.
 * If a vertex with the same name already exists in the given part, do nothing.
 */
vtxidx_t VarGraph::addvertex(const std::string &name, bool part)
{
	vtxidx_t idx = findvertex(name, part);
	if (idx != INVALID_VTXIDX)
		return idx;
	VarVertex v(part, name);
	if (part)
	{
		idx = accvertices_.size();
		accvertices_.push_back(v);
	}
	else
	{
		idx = funcvertices_.size();
		funcvertices_.push_back(v);
	}
	return idx;
}

/*
 * Add an edge between vertex `u` in "ACC" and vertex `v` in "FUNC".
 * The return value is reserved.
 * Note: if either `u` or `v` is invalid, exception will be thrown.
 */
long VarGraph::addedge(vtxidx_t u, vtxidx_t v)
{
	if (u >= accvertices_.size() || v >= funcvertices_.size())
		throw out_of_range("VarGraph::addedge: vertex index out of range");
	accvertices_[u].adjacent.insert(v);
	funcvertices_[v].adjacent.insert(u);
	return 0;
}

/*
 * Add an edge between the vertex with name `uname` in "ACC"
 * and the vertex with name `vname` in "FUNC".
 * If either vertex does not exist, it will be created.
 */
long VarGraph::addedge(const std::string &uname, const std::string &vname)
{
	vtxidx_t u = addvertex(uname, true),
		v = addvertex(vname, false);
	return addedge(u, v);
}

// Reset all matches in the graph and return 0
long VarGraph::resetmatches(void)
{
	for (auto &v : accvertices_)
		v.match = INVALID_VTXIDX;
	for (auto &v : funcvertices_)
		v.match = INVALID_VTXIDX;
	return 0l;
}