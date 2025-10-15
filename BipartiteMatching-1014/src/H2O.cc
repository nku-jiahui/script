#include "AccGraph.hh"
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>

using namespace std;

VarGraph varmatchgraph;
map<long, AccFuncMatch> accfuncmatches;

long ReadMatchesFromString(istream &fin);
vector<long> SelectSatisfiedMatched(void);

int main(int argc, char *argv[]) {
  try {
    if (argc < 2)
      throw runtime_error("Usage: " + string(argv[0]) + " <matchfile>");
    ifstream fin(argv[1]);
    if (!fin)
      throw runtime_error("Cannot open file " + string(argv[1]));

    // Read the "ACC"~"FUNC" matches from the stream
    long nmatches = ReadMatchesFromString(fin);

    fin.close();
    if (nmatches <= 0)
      throw runtime_error("No ACC~FUNC matches read");
    cout << "Read " << nmatches << " ACC~FUNC matches." << endl;
    cout << "The variable match graph has "
         << varmatchgraph.accvertices().size() << " ACC vertices and "
         << varmatchgraph.funcvertices().size() << " FUNC vertices." << endl;

    // Run the Hungarian matching algorithm
    size_t matchsize = BipartiteGraphMatching(varmatchgraph);

    cout << "The maximum matching has size " << matchsize << "." << endl;

    // Scan all "ACC"~"FUNC" matches and find the satisfied ones
    vector<long> satisfiedmatches = SelectSatisfiedMatched();

    cout << satisfiedmatches.size()
         << " ACC~FUNC matches are satisfied:" << endl;

    // Print all satisfied matches
    for (long idx : satisfiedmatches) {
      cout << "$" << idx << ' ' << accfuncmatches.at(idx).accname << " ~ "
           << accfuncmatches.at(idx).funcname << endl;

      // Print variable mappings for this match
      for (const auto &vp : accfuncmatches.at(idx).varsmatch) {
        vtxidx_t u = vp.first, v = vp.second;
        if (u < varmatchgraph.accvertices().size() &&
            v < varmatchgraph.funcvertices().size()) {
          cout << varmatchgraph.accvertices().at(u).name << " ~ "
               << varmatchgraph.funcvertices().at(v).name << endl;
        }
      }
    }
  } catch (const exception &e) {
    cerr << "**Error: " << e.what() << endl;
    return 1;
  }
  return 0;
}

/*
 * Read the "ACC"~"FUNC" matches from the input stream `fin`.
 * The return value is the number of matches read, or `-1` if error occurs.
 */
long ReadMatchesFromString(istream &fin) {
  regex
      // Match lines like "$1 acc1 ~ func1"
      rex_accfunc(R"(\$(\d+)\s+([\w.-]+)\s*~\s*([\w.-]+)\s*)"),
      // Match lines like "var.a ~ var.A"
      rex_varpair(R"(\s*([\w.-]+)\s*~\s*([\w.-]+)\s*)");
  smatch rex_matches;
  string line;
  long nmatches = 0;

  while (getline(fin, line)) {
    // Find the first non-white character
    auto it =
        find_if(line.begin(), line.end(), [](char c) { return !isspace(c); });
    // Empty lines and comments are ignored.
    if (it == line.end() || *it == '#')
      continue;

    if (regex_match(line, rex_matches, rex_accfunc)) {
      // A new "ACC"~"FUNC" match
      long matchidx = stol(rex_matches[1].str());
      string accname = rex_matches[2].str(), funcname = rex_matches[3].str();
      if (accfuncmatches.find(matchidx) != accfuncmatches.end())
        throw runtime_error("Duplicate match index: " + to_string(matchidx));
      // This is wrong! As `AccFuncMatch` has no default constructor.
      // accfuncmatches[matchidx] = AccFuncMatch(accname, funcname);
      accfuncmatches.emplace(matchidx, AccFuncMatch(accname, funcname));
      ++nmatches;
    } else if (regex_match(line, rex_matches, rex_varpair)) {
      // A variable pair in the current "ACC"~"FUNC" match
      if (accfuncmatches.empty())
        throw runtime_error("Variable pair without preceding"
                            " ACC~FUNC line");
      string varaccname = rex_matches[1].str(),
             varfuncname = rex_matches[2].str();
      vtxidx_t u = varmatchgraph.addvertex(varaccname, true),
               v = varmatchgraph.addvertex(varfuncname, false);
      varmatchgraph.addedge(u, v);
      accfuncmatches.rbegin()->second.varsmatch.push_back(make_pair(u, v));
    } else
      throw runtime_error("Unrecognized line: " + line);
  }
  return nmatches;
}

/*
 * After the Hungarian matching algorithm is done,
 * scan all "ACC"~"FUNC" matches and find the satisfied ones.
 */
vector<long> SelectSatisfiedMatched(void) {
  vector<long> satisfiedmatches;
  for (const auto &m : accfuncmatches) {
    bool issatisfied = true, flag1, flag2;
    for (const auto &vp : m.second.varsmatch) {
      vtxidx_t u = vp.first, v = vp.second;
      if (u >= varmatchgraph.accvertices().size() ||
          v >= varmatchgraph.funcvertices().size())
        throw out_of_range("SelectSatisfiedMatched: "
                           "vertex index out of range");
      flag1 = (varmatchgraph.accvertices().at(u).match == v);
      flag2 = (varmatchgraph.funcvertices().at(v).match == u);
      if (flag1 != flag2)
        throw logic_error("SelectSatisfiedMatched: "
                          "matching inconsistency for ACC~FUNC " +
                          to_string(m.first));
      if (!flag1) { // This variable pair is not matched
        issatisfied = false;
        break;
      }
    }
    if (issatisfied)
      // All of its variable pairs are matched
      satisfiedmatches.push_back(m.first);
  }
  return satisfiedmatches;
}
