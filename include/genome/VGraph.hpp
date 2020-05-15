#ifndef VGRAPH_HPP
#define VGRAPH_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <ARGraph.hpp>
#include <VG.hpp>
#include <Segment.hpp>
#include <ARGEdit.hpp>
#include <GFALoader.hpp>

namespace vflib
{
	class VGraph : public ARGraph<Segment, VGEdge>
	{
	private:
		std::unordered_map<std::string, VGPath> paths;

	public:
		VGraph(StreamGFALoader* loader):ARGraph(loader)
		{
            loader->GetPaths(paths);
		}

        inline const VGPath& operator[](const std::string& name) const { return paths.at(name); }
		
        inline const VGPath& GetPath(const std::string& name) const { return paths.at(name); }

        inline bool HasPath(const std::string& name) const 
        {

			auto res = paths.find(name);
            return res != paths.end();
        }

		void Print()
		{
			for (int node = 0; node < attr.size(); node++)
			{
				int i;
				std::cout << attr[node].str() << std::endl;
				for(i=0; i<in[node].size(); i++)
				{
					std::cout << "\t" << in_attr[node][i].str() << " " << attr[in[node][i]].str() << std::endl;
				}

				for(i=0; i < out[node].size(); i++)
				{
					std::cout << "\t" << out_attr[node][i].str() << " " << attr[out[node][i]].str() << std::endl;
				}
			}

			for(auto p : paths)
			{
				std::cout << "P: " << p.first << std::endl;
				std::cout << "\t" << p.second.str() << std::endl;
			}
        		
		}

	};
}

#endif //VGRAPH_HPP

