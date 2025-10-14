#ifndef __ACCGRAPH_HH__
#define __ACCGRAPH_HH__

#include <vector>
#include <set>
#include <utility>
#include <string>
#include <cstddef>
#include <climits>

typedef size_t vtxidx_t;
#define INVALID_VTXIDX SIZE_MAX

// Vertex in the bipartite graph representing a variable
class VarVertex {
public:
	bool part; // `true` for "ACC", `false` for "FUNC"
	std::string name;
	/*
	 * For a bipartite graph, the adjacent vertices are all in the other set.
	 */
	std::set<vtxidx_t> adjacent;
	/*
	 * If the vertex is matched, this is the index of the vertex it is matched to.
	 * If the vertex is unmatched, this is `INVALID_VTXIDX`.
	 * Note: as for a bipartite graph, the matched vertex must be in the other set.
	 */
	vtxidx_t match;
	// Constructor
	VarVertex(bool part, const std::string &name)
		: part(part), name(name), match(INVALID_VTXIDX) { }
};

// The graph that only contains variable vertices
class VarGraph {
private:
	std::vector<VarVertex> accvertices_;
	std::vector<VarVertex> funcvertices_;
public:
	std::vector<VarVertex>& accvertices() { return accvertices_; }
	std::vector<VarVertex>& funcvertices() { return funcvertices_; }
	vtxidx_t findvertex(const std::string &name, bool part) const;
	vtxidx_t addvertex(const std::string &name, bool part);
	long addedge(vtxidx_t u, vtxidx_t v);	// `u` is in "ACC", `v` is in "FUNC"
	long addedge(const std::string &uname, const std::string &vname);
	long resetmatches(void);
};

/*
 * The class that matches the variables in one "ACC" and one "FUNC"
 */
class AccFuncMatch {
public:
	std::string accname, funcname;
	/*
	 * The pairs of matched variable indices.
	 * The first is in "ACC", the second is in "FUNC"
	 */
	std::vector<std::pair<vtxidx_t, vtxidx_t>> varsmatch;
	// Constructor
	AccFuncMatch(const std::string &accname, const std::string &funcname)
		: accname(accname), funcname(funcname) { }
};


extern size_t BipartiteGraphMatching(VarGraph &bg);

#endif
