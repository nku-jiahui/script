#include "AccGraph.hh"
#include <stdexcept>

using namespace std;

static long HelperDFS(VarGraph &bg, vtxidx_t u, vector<bool> &visited);

/*
 * Find a maximum bipartite matching using the Hungarian algorithm.
 * The return value is the size of the matching.
 */
size_t BipartiteGraphMatching(VarGraph &bg)
{
	size_t i, matchsize = 0u;
	vector<bool> fvisited;
	bg.resetmatches();
	
	for (i = 0u; i < bg.funcvertices().size(); ++i)
		fvisited.push_back(false);
	for (i = 0u; i < bg.accvertices().size(); ++i)
	{
		if (bg.accvertices().at(i).match == INVALID_VTXIDX)
		{	// `i` in "ACC" is unmatched
			fill(fvisited.begin(), fvisited.end(), false);
			if (HelperDFS(bg, i, fvisited))
				++matchsize;
		}
	}
	return matchsize;
}

/*
 * A helper function for DFS in the Hungarian algorithm.
 * `u` is the index of the vertex in "ACC", and
 * `visited` keeps track of the visited vertices in "FUNC".
 * If an augmenting path is found, the matching is updated and `1` is returned;
 * otherwise `0` is returned.
 */
static long HelperDFS(VarGraph &bg, vtxidx_t u, vector<bool> &visited)
{
	if (u >= bg.accvertices().size())
		throw out_of_range("HelperDFS: vertex index out of range");
	if (visited.size() != bg.funcvertices().size())
		throw logic_error("HelperDFS: `visited` size mismatch");
	for (vtxidx_t v : bg.accvertices().at(u).adjacent)
	{
		if (!visited.at(v))
		{
			visited.at(v) = true;
			if (bg.funcvertices().at(v).match == INVALID_VTXIDX
				|| HelperDFS(bg, bg.funcvertices().at(v).match, visited))
			{	// An augmenting path is found
				// Update the matching
				bg.accvertices().at(u).match = v;
				bg.funcvertices().at(v).match = u;
				return 1;
			}
		}
	}
	return 0;
}
