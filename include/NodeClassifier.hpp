#ifndef NODECLASSIFIER_HPP
#define NODECLASSIFIER_HPP

#include <map>
#include <ARGraph.hpp>

namespace vflib
{

	/*
	* @class NodeClassifier
	* @brief Basic Node Classifier
	* @detials Classifies the nodes by different attribute values. 
	*	Nodes with the same attribute belongs to the same class
	*/
	template<typename Node, typename Edge>
	class NodeClassifier {
	protected:
		std::map<Node, uint32_t> classmap; //Map Of classes
		ARGraph<Node, Edge>* g1;
	private:
		/*
		* @fn GenerateClasses
		* @brief Generates the map of the classes
		*/
		void virtual GenerateClasses(ARGraph<Node, Edge>* g)
		{
			uint32_t c;
			uint32_t classes_count = 0;
			nodeID_t n;
			Node attr;
			uint32_t count = g->NodeCount();

			for (n = 0; n < count; n++)
			{
				attr = g->GetNodeAttr(n);
				if (classmap.count(attr))
				{
					c = classmap[attr];
				}
				else
				{
					c = classes_count++;
					classmap[attr] = c;

				}
			}
		}

	public:
		/*
		* @fn NodeClassifier 
		* @brief Used for first class generation
		*/
		NodeClassifier(ARGraph<Node, Edge>* g)
		{
			g1 = g;
			GenerateClasses(g);
		}

		/*
		* @fn NodeClassifier
		* @brief Used to inherit classes from another classifier
		*/
		NodeClassifier(ARGraph<Node, Edge>* g, const NodeClassifier<Node, Edge>& classifier)
		{
			g1 = g;
			classmap.insert(classifier.classmap.begin(), classifier.classmap.end());
			GenerateClasses(g);
		}

		/*
		* @fn CountClasses
		* @brief Returns the number of classes in the class map
		*/
		uint32_t CountClasses()
		{
			return classmap.size();
		}


		/*
		* @fn GetClasses
		* @brief Returns the vector of different classes
		*/
		std::vector<uint32_t> GetClasses()
		{
			Node attr;

			uint32_t count = g1->NodeCount();
			std::vector<uint32_t> classes(count);

			for (nodeID_t i = 0; i < count; i++)
			{
				attr = g1->GetNodeAttr(i);
				classes[i] = classmap[attr];
			}

			return classes;
		}

	};

}

#endif
