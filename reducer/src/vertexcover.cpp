#include "vertexcover.h"
#include "instance.h"
#include "mis_config.h"
#include "mis_log.h"
#include "exact_mis.h"
#include "../../3rdparty/pace-2019/lib/mis/exact_mis.h"
#include<algorithm>
#include<set>
#include<numeric>


bool vertex_cover_solution(Instance &instance) {
	std::set<int> in_vertex_cover;
	std::set<int> not_in_vertex_cover;

	std::vector<std::vector<int>> G(instance.g().n);

	for(size_t i = 0; i < instance.g().n; ++i){
		if(in_vertex_cover.count(i) || instance.g()[i].size() != 2)continue;
		int fi = *instance.g()[i].begin(), se = *(++instance.g()[i].begin());
		if(instance.g()[fi].contains(se)){
			in_vertex_cover.insert(fi);
			in_vertex_cover.insert(se);
			not_in_vertex_cover.insert(i);
			G[fi].push_back(se);
			G[se].push_back(fi);
		}
	}
	if(in_vertex_cover.size() + not_in_vertex_cover.size() != instance.g().n) return false;

	for (auto& v : G) std::sort(v.begin(), v.end());

	std::unordered_map <int, int> cc, icc;
	for (int i = 0; i < (int) instance.g().n; i++)  {
		if (G[i].empty()) continue;
		cc[i] = cc.size();
		icc[cc[i]] = i;
	}
	std::vector <std::vector <int>> H(cc.size());
	for (int i = 0; i < (int) G.size(); i++) for (int x : G[i]) H[cc[i]].push_back(cc[x]);
	const size_t Hsize = H.size();

	std::cout << "c TRYING TO VERTEX COVER" << std::endl;
	mis_log::instance()->restart_total_timer();
	MISConfig mis_config;

	mis_log::instance()->set_config(mis_config);
	mis_log::instance()->number_of_nodes = H.size();
	unsigned int num_edges = 0;
	for (auto &v : H) num_edges += v.size();
	mis_log::instance()->number_of_edges = num_edges;

	timer t;
	t.restart();
	std::vector<bool> MIS;
	// std::vector<int> deg(instance.g().n, 0);

	// for(size_t i = 0; i < instance.g().n; ++i){
	// 	deg[i] += G[i].size();
	// 	for(auto y : G[i])deg[y]++;
	// }

	MIS = getExactMISCombined(H, mis_config);

	// for(size_t i = 0; i < instance.g().n; ++i) assert(MIS[i] || deg[i]);

	std::cout << "c \t\tResult"        << std::endl;	
	std::cout << "c =========================================="                           << std::endl;
	std::cout << "c VC size:\t\t\t" << std::count(MIS.begin(), MIS.end(), false) << std::endl;

	// for(size_t i = 0; i < instance.g().n; ++i) if(!MIS[i]) instance.insert_D(i);
	for (size_t i = 0; i < Hsize; i++) if (!MIS[i]) instance.insert_D(icc[i]);
	return true;
}
